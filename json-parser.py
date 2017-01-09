import json

with open('santa2016.json') as data_file:
    data = json.load(data_file)

dest = []

for dest_json in data['destinations']:
    dest.append({
        'city': dest_json['city'],
        'location': dest_json['location']
        })

print(json.dumps(dest, indent=2))
