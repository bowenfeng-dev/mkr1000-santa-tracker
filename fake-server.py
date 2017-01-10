from flask import Flask
import json
import time

app = Flask(__name__)

fake_start_time = 0  # initialized to first departure time from json
real_start_time = 0  # set to start time

all_destinations = None

current_info = {
    'status': 'OK',
    'language': 'en',
    'now': None,            # Will be set to fake time
    'timeOffset': 120000,
    'fingerprint': '3b8835bc354c6d5018344b289b833402f7079844',
    'refresh': 51449,
    'switchOff': False,
    'clientSpecific': {
        'DisableEarth': False,
        'DisableTracker': False,
        'DisableWikipedia': False,
        'DisablePhotos': False,
        'HighResolutionPhotos': False,
        'EarthAltitudeMultiplier': 1
    },
    'routeOffset': 0,
    'destinations': None    # Will only have destinations up to two towns ahead
}


@app.route('/info')
def info():
    return app.response_class(json.dumps(current_info), content_type='application/json')


@app.route('/start')
def start():
    global real_start_time
    real_start_time = real_now()


@app.route('/reset')
def reset():
    global real_start_time
    real_start_time = 0
    current_info['destinations'] = [all_destinations[0]]


def real_now():
    return int(time.time() * 100)


def fake_now():
    return real_now() - real_start_time + fake_start_time


def arrival(d):
    return d['arrival']


def load_json():
    with open('santa2016.json') as data_file:
        data = json.load(data_file)

    global all_destinations, fake_start_time
    all_destinations = sorted(data['destinations'], key=arrival)
    fake_start_time = all_destinations[0]['departure']
    reset()

    print(u'{0} destinations loaded, fake_start_time={1}'.format(len(all_destinations), fake_start_time))


if __name__ == '__main__':
    load_json()
    app.run(host='0.0.0.0')

