import RPi.GPIO as GPIO
import board
#import pwmio
import time
from threading import Thread, Event, Lock, Timer

pinsType = GPIO.BCM
redPin = 13
greenPin = 19
bluePin = 26
buzzPin = 5

# pinsType = GPIO.BOARD
# redPin = board.D33
# greenPin = board.D35
# bluePin = board.D37
# buzzPin = board.D29

# pinsType = GPIO.BOARD
# redPin = board.D13
# greenPin = board.D19
# bluePin = board.D26
# buzzPin = board.D5

initialized = False
buzzPWM = None
rPWM = None
gPWM = None
bPWM = None


def setLedStatus(ledName, status, delay = 0):
    if delay > 0:
        t = Timer(delay, setLedStatus, [ledName, 'off'])
        t.start()

    if ledName == 'red':
        global rPWM
        changeLeds(rPWM, status)
    elif ledName == 'green':
        global gPWM
        changeLeds(gPWM, status)
    elif ledName == 'blue':
        global bPWM
        changeLeds(bPWM, status)

def changeLeds(LED, status):
    if status == 'on':
        LED.ChangeFrequency(1000)
        LED.start(100)
    elif status == 'off':
        LED.stop()
    elif status == 'slow1':
        LED.ChangeFrequency(1)
        LED.start(50)
    elif status == 'slow2':
        LED.ChangeFrequency(4)
        LED.start(50)


def init():
    global initialized
    if not initialized:
        GPIO.cleanup([redPin, greenPin, bluePin, buzzPin])
        GPIO.setmode(pinsType)

        #buzzer pin
        GPIO.setup(buzzPin, GPIO.OUT)
        GPIO.setup(redPin, GPIO.OUT)
        GPIO.setup(bluePin, GPIO.OUT)
        GPIO.setup(greenPin, GPIO.OUT)

        global rPWM
        global gPWM
        global bPWM

        # rPWM = pwmio.PWMOut(redPin, duty_cycle=0, frequency = 100)
        # gPWM = pwmio.PWMOut(greenPin, duty_cycle=0, frequency = 100)
        # bPWM = pwmio.PWMOut(bluePin, duty_cycle=0, frequency = 100)
        rPWM = GPIO.PWM(redPin, 100)
        gPWM = GPIO.PWM(greenPin, 100)
        bPWM = GPIO.PWM(bluePin, 100)

        initialized = True

def soundBuzzerThread(nTimes, delay):
    for idx in range(0,nTimes):
        GPIO.output(buzzPin, GPIO.LOW)
        time.sleep(delay)
        GPIO.output(buzzPin, GPIO.HIGH)
        time.sleep(delay)

    GPIO.output(5, GPIO.LOW)

def soundBuzzer(nTimes, delay):
    buzzThread = Thread(target=soundBuzzerThread, args=(nTimes,delay,))
    buzzThread.start()


def main():
    global rPWM
    global gPWM
    global bPWM

    #rPWM = pwmio.PWMOut(board.D13, duty_cycle=0, frequency = 100)
    #gPWM = pwmio.PWMOut(board.D19, duty_cycle=0, frequency = 100)
    #bPWM = pwmio.PWMOut(board.D26, duty_cycle=0, frequency = 100)
    
    # GPIO.cleanup()
    # #GPIO.setmode(GPIO.BOARD)
    # GPIO.setup(5, GPIO.OUT)
    # delay = 0.2
    # # for idx in range(0,3):
    # #     GPIO.output(5, GPIO.LOW)
    # #     time.sleep(delay)
    # #     GPIO.output(5, GPIO.HIGH)
    # #     time.sleep(delay)

    # # GPIO.output(5, GPIO.LOW)

    init()

    setLedStatus('red', 'on')
    #soundBuzzer(4, 0.2)

if __name__ == "__main__":
    try: 
        main()
    finally:
        GPIO.cleanup()