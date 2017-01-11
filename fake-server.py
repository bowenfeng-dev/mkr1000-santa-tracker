from flask import Flask, request
import json
import time

app = Flask(__name__)

fake_start_time = 0  # initialized to first arrival time from json
real_start_time = 0  # set to start time
speed_factor = 100  # fake clock speed

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
    if real_start_time != 0:
        advance_fake_time()
    return app.response_class(json.dumps(current_info), content_type='application/json')


@app.route('/start')
def start():
    global real_start_time, speed_factor
    real_start_time = real_now()
    speed_factor = int(request.args.get('speed', '100'))
    print(u'fake clock stated at speed {0}'.format(speed_factor))
    return 'ok'


@app.route('/reset')
def reset():
    global real_start_time
    real_start_time = 0
    current_info['destinations'] = all_destinations[:3]
    return 'ok'


def index_of_current_destination(ts):
    for i, dest in enumerate(all_destinations):
        if dest['departure'] > ts:
            return i
    return 0


def current_destinations():
    index = index_of_current_destination(fake_now()) + 3
    return all_destinations[:index]


def advance_fake_time():
    current_info['now'] = fake_now()
    current_info['destinations'] = current_destinations()


def real_now():
    return int(time.time() * 100)


def fake_now():
    return (real_now() - real_start_time) * speed_factor + fake_start_time


def arrival(d):
    return d['arrival']


def load_json():
    with open('santa2016.json') as data_file:
        data = json.load(data_file)

    global all_destinations, fake_start_time
    all_destinations = sorted(data['destinations'], key=arrival)
    fake_start_time = all_destinations[1]['arrival']
    reset()

    print(u'{0} destinations loaded, fake_start_time={1}'.format(len(all_destinations), fake_start_time))


if __name__ == '__main__':
    load_json()
    app.run(host='0.0.0.0', port=1224)

