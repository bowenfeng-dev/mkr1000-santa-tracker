
#include <SPI.h>
#include <WiFi101.h>
#include <Adafruit_NeoPixel.h>
#include "JsonStreamingParser.h"
#include "JsonListener.h"

#define LED_PIN 6
#define LED_NUM 30
#define BRIGHTNESS 50

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_NUM, LED_PIN, NEO_GRB + NEO_KHZ800);

char ssid[] = "YOUR_SSID";     //  your network SSID (name)
char pass[] = "YOUR_PWRD";  // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
IPAddress server(192, 168, 1, 120);  // numeric IP for RPI server
//char server[] = "rpi3.local";    // name address for RPI server
char endpoint[] = "/santa";
int port = 2412;

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiClient client;

class Led {
  public:
    String name;
    int distance;
    int presents;
    boolean on;
};

Led leds[30];

class LedSwitcher: public JsonListener {
  public:
    void whitespace(char c) {}
    void startDocument() {}
    void key(String key) {
      Serial.println(key);
      currentKey = key;
    }
    void value(String value) {
      Serial.println(value);
      if (currentKey == "i") {
        ledIndex = value.toInt();
      } else if (currentKey == "p") {
        presents = value.toInt();
      } else if (currentKey == "d") {
        distance = value.toInt();
      } else {
        name = value;
      }
    }
    void endArray() {}
    void endObject() {
      Serial.println("End of Object");
      Serial.print(ledIndex);
      Serial.print(":");
      Serial.print(name.c_str());
      Serial.print(",");
      Serial.print(presents);
      Serial.print(",");
      Serial.print(distance);

      leds[ledIndex].on = true;
      leds[ledIndex].name = name;
      leds[ledIndex].presents = presents;
      leds[ledIndex].distance = distance;
    }
    void endDocument() {}
    void startArray() {}
    void startObject() {}

    int lastLed() {
      return ledIndex;
    }

  private:
    String currentKey;
    int ledIndex;
    int presents;
    int distance;
    String name;
};

LedSwitcher ledSwitcher;

void connectToWifi() {
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("Connected to wifi");
  printWifiStatus();
}

boolean connectToSantaServer() {
  Serial.println("Starting connection to server...");
  return client.connect(server, port);
}

void ensureConnected() {
  if (!client.connected()) {
    while (!connectToSantaServer()) {
      Serial.println("Failed to connect to server. Retry in 5 seconds");
      delay(5000);
    }
    Serial.println("connected to server");
  }
}

void fetchSantaInfo() {
  ensureConnected();

  // Make a HTTP request:
  client.print("GET ");
  client.print(endpoint);
  client.println(" HTTP/1.1");
  client.print("Host: ");
  client.println(server);
  client.println("Connection: close");
  client.println();

  int bytes = 0;
  boolean isBody = false;
  JsonStreamingParser parser;
  parser.setListener(&ledSwitcher);

  Serial.println();
  Serial.println("Received response:");
  Serial.println();
  
  while (client.connected()) {
    while (client.available()) {
      char c = client.read();
      ++bytes;
      //Serial.write(c);
      if (isBody || c == '[') {
        isBody = true;
        parser.parse(c);
      }
    }
  }

  Serial.println();
  Serial.println();
  Serial.println("Disconnecting from server.");
  client.stop();

  Serial.print("Received: ");
  Serial.print(bytes);
  Serial.println(" Bytes.");
}

void flashLastLed() {
  strip.setPixelColor(ledSwitcher.lastLed(), strip.Color(0, 0, 0));
  strip.show();
  delay(500);
  strip.setPixelColor(ledSwitcher.lastLed(), strip.Color(255, 0, 0));
  strip.show();
  delay(500);
}

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  
  strip.setBrightness(BRIGHTNESS);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'.
  
  connectToWifi();
  fetchSantaInfo();

  for (int i = 0; i < 30; ++i) {
    if (leds[i].on) {
      strip.setPixelColor(i, strip.Color(255, 0, 0));
    } else {
      strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
  }
  strip.show();
}

void loop() {
  fetchSantaInfo();
  delay(1000);
  for (int i = 0; i < 10; ++i) {
    flashLastLed();
  }
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

