import requests

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
        response = requests.post(serverUrl+'/tempAlarm', json={'temp':temp})
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
    #getSchedule()
    pushTempAlarm(30)

if __name__ == "__main__":
    main()