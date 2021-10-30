from os import error
import socket
import sys
import json
from threading import Thread, Event
from time import sleep
import random

sock = None
connection = None
evt = Event()
valList = []

def cleanup():
    global sock
    global connection
    print('receive keyboard interrupt, terminate the program')
    connection.close()
    sock.close()
    sys.exit()

def tcpThread():
    print("TCP Thread start successfully")
    global sock
    # Create a TCP/IP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # Bind the socket to the port
    server_address = (socket.gethostbyname(socket.gethostname()), 10000)
    print('starting up server on {0} port {1}'.format(server_address, 10000))
    sock.bind(server_address)

    # Listen for incoming connections
    sock.listen(1)

    while True:
        global connection
        # Wait for a connection
        print('waiting for a connection')
        connection, client_address = sock.accept()
        
        #Set timeout for socket operations
        connection.settimeout(5)
        print('tcp is alive')
        try:
            print('connection from: {0}'.format(client_address))
            # Wait for data from the client
            while True:
                data = connection.recv(512).decode()
                print('data received, with type {0}, data={1}'.format(type(data), data))

                if data:
                    print('handle recieved data')
                    dataObj = json.loads(data)
                    valList.append(dataObj)
                    #signal AppThread
                    print("signal AppThread")
                    evt.set()
                    #connection.send(data.encode())
                else:
                    print('no more data from {0}'.format(client_address))
                    break
        except KeyboardInterrupt:
            print('receive keyboard interrupt, terminate the program')
            cleanup()      
        finally:
            print("error happened to the connection, exit the connection")
            # Clean up the connection
            connection.close()
            #clear the values list
            valList.clear()


def threadWrapper(tcpThread):
    def wrapper():
        while True:
            try:
                tcpThread()
            except BaseException as e:
                print('{!r}; restarting thread'.format(e))
            else:
                print('exited normally!!!!')

    return wrapper()
    

def readSensor():
    # ToDO: read the actual value of the sensor
    return {"Pi_Val" : random.randint(2,500)}
 
def calcAvg():
    avg_reading = {'ESP': 0, 'Pi': 0}
    
    for dic in valList:
        avg_reading['ESP'] += dic['ESP_Val']
        avg_reading['Pi'] += dic["Pi_Val"]

    avg_reading['ESP'] /= 8
    avg_reading['Pi'] /= 8

    return avg_reading

def UpdateLed(avg_reading):
    #ToDO: include the actual LED on PI
    ledStatus = {'ESP_LED':0, 'Pi_LED':0}
    if avg_reading['ESP'] >= avg_reading['Pi']:
        print('Turn on the ESP LED')
        ledStatus['ESP_LED'] = 1
    else:
        print('Turn on Pi LED')
        ledStatus['Pi_LED'] = 1

    return ledStatus

def sendData(dictObj):
    global connection
    # Convert the dictonary to string
    msgStr = json.dumps(dictObj)
    print('Send to Client: {0}'.format(msgStr))
    #Send Data to Connected socket
    connection.send(msgStr.encode())


def AppThread():

    print("App Thread Started\n")
    
    def AppDispatch():
        while True:
            evt.wait()
            print('AppThread Signalled')
            valList[-1].update(readSensor())
            print('update item in the list= {0}, list len {1}'.format(valList[-1], len(valList)))

            #Check if the list has 8 pairs (2 seconds window)
            if len(valList) == 8:
                #calculate the average for both ESP and Pi
                avg_reading = calcAvg()
                print("[ESP Avg = {0}] while [Pi Avg = {1}]".format(avg_reading['ESP'], avg_reading['Pi']))
                #clear the list
                valList.clear()
                
                #Apply Led and get the LED Value
                ledStatus = UpdateLed(avg_reading)

                #Send the value to ESP
                sendData(ledStatus)

            evt.clear()

        
    try:
        AppDispatch()
    except KeyboardInterrupt:
        print('receive keyboard interrupt, terminate the program')
        cleanup()


def mainProgram():

    try:
        tcpThreadObj = Thread(target=threadWrapper, args=(tcpThread,))
        AppThreadObj = Thread(target=AppThread)

        AppThreadObj.start()
        tcpThreadObj.start()

    except KeyboardInterrupt:
        print('receive keyboard interrupt, terminate the program')
        cleanup()

    


if __name__ == '__main__':
    mainProgram()