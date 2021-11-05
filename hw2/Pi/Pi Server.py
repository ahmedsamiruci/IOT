#!/user/bin/env python

from os import error
import socket
import sys
import json
from threading import Thread, Event, Lock
#from time import sleep
import time
import random
import re
import RPi.GPIO as GPIO
import board
import pwmio
import serial


testModeEnabled = False

sensorVal = None
ser = None
espPin = 23
sock = None
connection = None
evt = Event()
valList = []
ledPins = {"red":13, "green": 19, "blue":26}
rPWM = None
gPWM = None
bPWM = None


def cleanup():
    global sock
    global connection
    print('receive keyboard interrupt, terminate the program')
    connection.close()
    sock.close()
    sys.exit()


def handleRxData(MSG):
    print('handleRxData:-> {0}'.format(MSG))
    #Check if there are multiple records in the message
    #Split the records in a list
    MSG = re.sub(r'(}{)', r'}@{', MSG)
    records = MSG.split('@')

    for idx, rec in enumerate(records):
        #delete the invalid rows row
        if rec[-1] != '}':
            del records[idx]
        # valid record
        else: 
            dataObj = json.loads(rec)
            valList.append(dataObj)

    #signal AppThread
    #print("signal AppThread")
    evt.set()

    return True

def isRecord(MSG):
    if 0 < MSG.find('}'):
        return True
    else:
        return False

def get_ip_address():
 ip_address = '';
 s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
 s.connect(("8.8.8.8",80))
 ip_address = s.getsockname()[0]
 s.close()
 return ip_address


def tcpThread():
    print("TCP Thread start successfully")
    global sock
    # Create a TCP/IP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # Bind the socket to the port
    server_address = (get_ip_address(), 10000)
    print('starting up server on {0} port {1}'.format(server_address, 10000))
    sock.bind(server_address)

    # Listen for incoming connections
    sock.listen(1)

    try:

        while True:
            global connection
            # Wait for a connection
            print('waiting for a connection')
            connection, client_address = sock.accept()
            
            #Set timeout for socket operations
            connection.settimeout(10)
            print('tcp is alive')

            print('connection from: {0}'.format(client_address))
            
            changeLeds(rPWM, 'slow2')
            #get connection time
            tic = time.perf_counter()
            timerRun = True
            
            # Wait for data from the client
            totalData = b''
            while True:
                toc = time.perf_counter()
                if timerRun and ((toc - tic) >= 2):
                    print("Time elapsed = {0} seconds, Turn ON LED".format(toc-tic))
                    timerRun = False                    
                    changeLeds(rPWM, 'on')
                    
                data = connection.recv(512)
                if data == b'':
                    print("connection dropped")
                    #sock.shutdown(socket.SHUT_RDWR)
                    break


                #print('data received, with type {0}, data={1}'.format(type(data), data))
                totalData = totalData + data


                #print('TotalData, with type {0}, data={1}'.format(type(totalData), totalData))
                if isRecord(totalData.decode('utf-8')):
                    handleRxData(totalData.decode('utf-8'))
                    #clear total data
                    totalData = b''
                else:
                    print('wait for more data from {0}'.format(client_address))
                    
    except KeyboardInterrupt:
        print('receive keyboard interrupt, terminate the program')
        cleanup()      
    finally:
        print("error happened to the connection, exit the connection")
        print("Oops!", sys.exc_info()[0], "occurred.")
        # Clean up the connection
        connection.close()
        #clear the values list
        valList.clear()
        #turn off leds
        changeLeds(rPWM, 'slow1')
        changeLeds(bPWM, 'off')
        changeLeds(gPWM, 'off')


def threadWrapper(tcpThread):
    def wrapper():
        while True:
            try:
                tcpThread()
            except BaseException as e:
                print('{!r}; wait for 5 sec then restarting thread'.format(e))
                time.sleep(5)
            else:
                print('exited normally!!!!')

    return wrapper()


def readSensor(lock):

    if testModeEnabled:
        return {"Pi_Val" : random.randint(0,200)}
    else:
        global sensorVal
        #TODO Raise Semaphore
        lock.acquire()
        value = {"Pi_Val" : sensorVal}
        #TODO Release Semaphore
        lock.release()
        return value
 
def EspThread(lock):
    global sensorVal
    
    while True:
        tic = time.perf_counter()
        # Raise the ESP Interrupt pin
        #print("----> raise pin\n")
        GPIO.output(espPin, GPIO.HIGH)
        
        data = ser.readline().decode('utf-8')
        print("ESP Rspn: {0}\n".format(data))
        GPIO.output(espPin, GPIO.LOW)
        if data.startswith("val="):
            toc = time.perf_counter()
            print("[{0}] ESP Value = {1}, ".format(toc-tic, int(data.split("val=")[1])))
            #TODO: Raise semaphore
            lock.acquire()
            sensorVal = int(data.split("val=")[1])
            #TODO: Release semaphore
            lock.release()
    
 
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
    if avg_reading['ESP'] <= avg_reading['Pi']:
        print('Turn on the ESP LED')
        ledStatus['ESP_LED'] = 1
        changeLeds(bPWM, 'on')
        changeLeds(gPWM, 'off')
    else:
        print('Turn on Pi LED')
        ledStatus['Pi_LED'] = 1
        changeLeds(bPWM, 'off')
        changeLeds(gPWM, 'on')

    return ledStatus

def sendData(dictObj):
    global connection
    # Convert the dictonary to string
    msgStr = json.dumps(dictObj)
    print('Send to Client: {0}'.format(msgStr))
    #Send Data to Connected socket
    connection.send(msgStr.encode())


def AppThread(lock):

    print("App Thread Started\n")
    
    def AppDispatch():
        while True:
            evt.wait()
            print('AppThread Signalled')
            for dic in valList:
                if not 'Pi_Val' in dic:
                    dic.update(readSensor(lock))
                    print('update item in the list= {0}, list len {1}'.format(dic, len(valList)))

            #Check if the list has 8 pairs (2 seconds window)
            if len(valList) >= 8:
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


def changeLeds(LED, status):
    
    if status == 'on':
        LED.duty_cycle = 1000
        LED.frequency = 1000
    elif status == 'off':
        LED.duty_cycle = 0
    elif status == 'slow1':
        LED.duty_cycle = 1000
        LED.frequency = 1
    elif status == 'slow2':
        LED.duty_cycle = 1000
        LED.frequency = 4
    
       

def configLeds():
    global rPWM
    global bPWM
    global gPWM
    
    #GPIO.setmode(GPIO.BCM)
    
    greenLed = 19 #board.D19
    redLed = 13 #board.D13
    blueLed = 26 #board.D26
    
    #GPIO.setup(ledPins['green'], GPIO.OUT)
    #GPIO.setup(ledPins['blue'], GPIO.OUT)
    
    rPWM = pwmio.PWMOut(board.D13, duty_cycle=0, frequency = 100)
    gPWM = pwmio.PWMOut(board.D19, duty_cycle=0, frequency = 100)
    bPWM = pwmio.PWMOut(board.D26, duty_cycle=0, frequency = 100)
    
    #print("Turn On All Leds")
    
    #Turn On All Leds
    changeLeds(rPWM, 'slow1')
    changeLeds(gPWM, 'off')
    changeLeds(bPWM, 'off')


def configESP82Comm():
    global ser
    ser = serial.Serial(
        port='/dev/ttyS0', 
        baudrate = 921600,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS,
        timeout=1
        )
    
    GPIO.setup(espPin, GPIO.OUT)
    GPIO.output(espPin, GPIO.LOW)
    
    
def mainProgram():
    
    #Configure GPIOs For LEDs
    configLeds()
    
    #Configure communication with ESP8266 to get sensor readings
    configESP82Comm()
    
    try:
        # creating a lock
        lock = Lock()
        
        tcpThreadObj = Thread(target=threadWrapper, args=(tcpThread,))
        AppThreadObj = Thread(target=AppThread, args=(lock,))
        EspThreadObj = Thread(target=EspThread, args=(lock,))

        AppThreadObj.start()
        tcpThreadObj.start()
        EspThreadObj.start()

    except KeyboardInterrupt:
        print('receive keyboard interrupt, terminate the program')
        cleanup()

    


if __name__ == '__main__':
    mainProgram()