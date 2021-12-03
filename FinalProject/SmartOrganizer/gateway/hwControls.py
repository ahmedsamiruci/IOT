import RPi.GPIO as GPIO
import board
import pwmio
import time
from threading import Thread, Event, Lock


ledPins = {"red":13, "green": 19, "blue":26}

buzzPWM = None
rPWM = None
gPWM = None
bPWM = None


def setLedStatus(ledName, status):
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

def init():
    GPIO.cleanup()
    #buzzer pin
    GPIO.setup(5, GPIO.OUT)

    global rPWM
    global gPWM
    global bPWM

    rPWM = pwmio.PWMOut(board.D13, duty_cycle=0, frequency = 100)
    gPWM = pwmio.PWMOut(board.D19, duty_cycle=0, frequency = 100)
    bPWM = pwmio.PWMOut(board.D26, duty_cycle=0, frequency = 100)

def soundBuzzerThread(nTimes, delay):
    for idx in range(0,nTimes):
        GPIO.output(5, GPIO.LOW)
        time.sleep(delay)
        GPIO.output(5, GPIO.HIGH)
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

    changeLeds(rPWM, 'slow1')
    soundBuzzer(4, 0.2)

if __name__ == "__main__":
    main()