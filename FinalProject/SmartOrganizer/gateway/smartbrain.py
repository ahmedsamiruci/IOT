import tagBle
import datetime
from threading import Thread, Event, Lock


schedule = {
                "days": ["Sat", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri"],
                "slots": ["AM", "PM"],
                "alarms": "on"
            }

slotEvtList = []
tempEvtList = []


def getCurrDay():
    t = datetime.datetime.now()
    return t.strftime("%a")

def getCurrSlot():
    t = datetime.datetime.now()
    return t.strftime("%p")

def getCurrHr():
    t = datetime.datetime.now()
    return t.strftime("%I")

def getTime():
    t = datetime.datetime.now()
    return t.strftime("%a:%I:%p")

def isWithinSchedule(schdl):
    for day in schdl["days"]:
        # Check if the day exist in schedule
        if getCurrDay() == day:
            # check for the day time slot
            if (getCurrSlot() == schdl['slots'][0]) or (getCurrSlot() == schdl['slots'][1]):
                return True
    return False


def insertEvent(event, data):
    global slotEvtList
    global tempEvtList
    
    evt = [getTime(), data]
    if event == 'slot':    
        print('insert slot event, time: {0}, evt: {1}'.format(evt[0],evt[1]))
        slotEvtList.append(evt)

    else:
        print('insert temp event, time: {0}, evt: {1}'.format(evt[0],evt[1]))
        tempEvtList.append(evt)


def configReminders(schedule):
    now = datetime.datetime.now()


def eventCallback(event, data):
    print("smartbrain CB: evt:{0}, data:{1}".format(event, data))
    insertEvent(event, data)

def main():
    print("The main smartOrganizer brain")
    dvcThreadObj = Thread(target=tagBle.catchSmartDevice, args=(eventCallback,))
    dvcThreadObj.start()


if __name__ == "__main__":
    main()

