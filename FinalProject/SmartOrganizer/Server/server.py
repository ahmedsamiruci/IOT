from flask import Flask,jsonify,request,json
import requests

schedule = {
    "reminderTimeout": 30,
    "tempAlarm": "",
    "tempThreshold": 25,
    "AM_Window": [
        8,
        10
    ],
    "PM_Window": [
        4,
        6
    ],
    "Sat": {
        "AM": {
            "reminders": 2,
            "status": ""
        },
        "PM": {
            "reminders": 2,
            "status": ""
        }
    },
    "Sun": {
        "AM": {
            "reminders": 2,
            "status": ""
        },
        "PM": {
            "reminders": 2,
            "status": ""
        }
    },
    "Mon": {
        "AM": {
            "reminders": 2,
            "status": ""
        },
        "PM": {
            "reminders": 2,
            "status": ""
        }
    },
    "Tue": {
        "AM": {
            "reminders": 2,
            "status": ""
        },
        "PM": {
            "reminders": 2,
            "status": ""
        }
    },
    "Wed": {
        "AM": {
            "reminders": 2,
            "status": ""
        },
        "PM": {
            "reminders": 2,
            "status": ""
        }
    },
    "Thu": {
        "AM": {
            "reminders": 2,
            "status": ""
        },
        "PM": {
            "reminders": 2,
            "status": ""
        }
    },
    "Fri": {
        "AM": {
            "reminders": 2,
            "status": ""
        },
        "PM": {
            "reminders": 2,
            "status": ""
        }
    }
}


app = Flask(__name__)

def pushNotification(param):
    if param == 'temp':
        try:
            response = requests.post('https://maker.ifttt.com/trigger/TempAlarm/with/key/d6oHDDX89YYY0X6Mg9hG0S')

            print(response)

        except requests.exceptions.HTTPError as errh:
            print(errh)
        except requests.exceptions.ConnectionError as errc:
            print(errc)
        except requests.exceptions.Timeout as errt:
            print(errt)
        except requests.exceptions.RequestException as err:
            print(err)

@app.route('/schedule',methods=['GET'])
def getSchedule():
    return jsonify(schedule)


@app.route('/tempAlarm', methods=['POST'])
def tempAlarm():
    if request.headers['Content-Type'] == 'application/json':
        data = json.dumps(request.json)
        print("received data: {0}".format(data))
        # Push notification to IFTTT
        pushNotification("temp")
        return jsonify({"reply": "got Alarm Message"})

if __name__ == "__main__":
    app.run(debug=True)
