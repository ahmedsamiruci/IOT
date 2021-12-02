from flask import Flask,jsonify,request,json
import requests
import datetime
import time

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
   
    try:
        if param == 'temp':
            response = requests.post('https://maker.ifttt.com/trigger/TempAlarm/with/key/d6oHDDX89YYY0X6Mg9hG0S')
            print(response)
        elif param == 'missed':
            response = requests.post('https://maker.ifttt.com/trigger/MissDosage/with/key/d6oHDDX89YYY0X6Mg9hG0S')
            print(response)
        elif param == 'reminder':  
            response = requests.post('https://maker.ifttt.com/trigger/MedReminder/with/key/d6oHDDX89YYY0X6Mg9hG0S')
            print(response)          
        elif param == 'taken':  
            response = requests.post('https://maker.ifttt.com/trigger/MedTaken/with/key/d6oHDDX89YYY0X6Mg9hG0S')
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


@app.route('/Alarm', methods=['POST'])
def Alarm():
    if request.headers['Content-Type'] == 'application/json':
        print("received data: {0}, data type: {1}".format(request.json, type(request.json)))
        # Push notification to IFTTT
        pushNotification(request.json['evt'])
        return jsonify({"reply": "got Alarm Message"})

@app.route('/Events', methods=['POST'])
def rxEventData():
    if request.headers['Content-Type'] == 'application/json':
        print("received data: {0}, data type: {1}".format(request.json, type(request.json)))
        #save event data in the report
        with open("userReport.json", 'a') as report:
            t = datetime.datetime.now()
            eventData = {'timestamp': t.strftime("%m/%d/%Y, %H:%M:%S")}
            #eventData.update(request.json)
            eventData['day'] = request.json.get('day')
            eventData['slot'] = request.json.get('slot')
            eventData['slotEvt'] = request.json.get('slotEvt')
            eventData['temp'] = request.json.get('temp')
            report.write(json.dumps(eventData))
            report.write('\n')
        
        return jsonify({"reply":"got event Data"})

    else:
        return jsonify({"reply":"Error in data-type"})

if __name__ == "__main__":
    app.run(debug=True, host="0.0.0.0")
