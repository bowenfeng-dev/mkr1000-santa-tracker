import json

with open('santa2016.json') as data_file:
    data = json.load(data_file)

dest = []

for dest_json in data['destinations']:
    dest.append({
        'arrival': dest_json['arrival'],
        'departure': dest_json['departure'],
        'city': dest_json['city'],
        'location': dest_json['location']
        })


def arrival(d):
    return d['arrival']

print(json.dumps(sorted(dest, key=arrival), indent=2))
