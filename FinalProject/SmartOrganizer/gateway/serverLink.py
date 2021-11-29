import requests
import sys

serverUrl = "http://127.0.0.1:5000"

def getSchedule():
    try:
        response = requests.get(serverUrl+'/schedule')
        print(response.json())
    
    except requests.exceptions.HTTPError as errh:
        print(errh)
    except requests.exceptions.ConnectionError as errc:
        print(errc)
    except requests.exceptions.Timeout as errt:
        print(errt)
    except requests.exceptions.RequestException as err:
        print(err)

def pushTempAlarm(temp):
    try:
        response = requests.post(serverUrl+'/Alarm', json={'evt':'temp', 'temp':temp})
        print(response.json())
        
    except requests.exceptions.HTTPError as errh:
        print(errh)
    except requests.exceptions.ConnectionError as errc:
        print(errc)
    except requests.exceptions.Timeout as errt:
        print(errt)
    except requests.exceptions.RequestException as err:
        print(err)

def pushReminder():
    try:
        response = requests.post(serverUrl+'/Alarm', json={'evt':'reminder'})
        print(response.json())
        
    except requests.exceptions.HTTPError as errh:
        print(errh)
    except requests.exceptions.ConnectionError as errc:
        print(errc)
    except requests.exceptions.Timeout as errt:
        print(errt)
    except requests.exceptions.RequestException as err:
        print(err)

def pushMissingDosage():
    try:
        response = requests.post(serverUrl+'/Alarm', json={'evt':'missed'})
        print(response.json())
        
    except requests.exceptions.HTTPError as errh:
        print(errh)
    except requests.exceptions.ConnectionError as errc:
        print(errc)
    except requests.exceptions.Timeout as errt:
        print(errt)
    except requests.exceptions.RequestException as err:
        print(err)

def main():
    print("Welcome ServerLink!!")
    print ('Number of arguments: {0} arguments:'.format(len(sys.argv)))
    for arg in sys.argv:
        print(arg)


    if sys.argv[1] == 'fetch':
        getSchedule()
    elif sys.argv[1] == 'missed':
        pushMissingDosage()
    elif sys.argv[1] == 'reminder':
        pushReminder()
    elif sys.argv[1] == 'temp':
        pushTempAlarm(30)

if __name__ == "__main__":
    main()