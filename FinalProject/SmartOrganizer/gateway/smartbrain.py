import tagBle
import datetime
import time
import json
import sys
import signal
from threading import Thread, Event, Lock
import serverLink as srv


slotEvtList = []
tempEvtList = []


def fetchSchedule():
    with open("schedule.json", 'r') as f:
        schedule = json.load(f)
        return schedule

def updateSchedule(schedule):
    print("Update Schedule .................")
    with open("schedule.json", 'w') as f:
        json.dump(schedule, f, indent=4)
    

def getCurrDay():
    with open("timeMock.json", "r") as f:
        timeMock = json.load(f)
        if timeMock['mode'] == 'test':
            return timeMock['day']
        else:
            t = datetime.datetime.now()
            return t.strftime("%a")

def getCurrSlot():
    with open("timeMock.json", "r") as f:
        timeMock = json.load(f)
        if timeMock['mode'] == 'test':
            return timeMock['slot']
        else:
            t = datetime.datetime.now()
            return t.strftime("%p")

def getCurrHr():
    with open("timeMock.json", "r") as f:
        timeMock = json.load(f)
        if timeMock['mode'] == 'test':
            return timeMock['hr']
        else:
            t = datetime.datetime.now()
            return t.strftime("%I")

# def getTime():
#     t = datetime.datetime.now()
#     return t.strftime("%a:%p:%I")

def inWindow(schedule, hr):
    #convert input to int
    if type(hr) == str:
        hr = int(hr)
        
    if (hr in range(schedule["AM_Window"][0], schedule["AM_Window"][1] + 1 )) or (hr in range(schedule["PM_Window"][0], schedule["PM_Window"][1] + 1)):
        return True
    else:
        return False


def insertEvent(event, data):
    global slotEvtList
    global tempEvtList
    
    evt = {"day": getCurrDay(), "slot": getCurrSlot(), "Hr":getCurrHr(), "slotEvt": data}
    if event == 'slot':    
        print('Slot event: day[{0}], slot[{1}], hr[{2}], data[{3}]'.format(evt['day'],evt['slot'],evt['Hr'],evt['slotEvt']))
        slotEvtList.append(evt)
    else:
        evt['temp'] = evt.pop('slotEvt')
        print('Temp event: day[{0}], slot[{1}], hr[{2}], data[{3}]'.format(evt['day'],evt['slot'],evt['Hr'],evt['temp']))
        tempEvtList.append(evt)

    #push the event to the server
    srv.sendEventData(evt)

def findOpenEvent(day, slot):
    for evt in slotEvtList:
        if day.capitalize() == evt['day'].capitalize() and slot == evt['slot']:
            packSlotDay = (evt['slotEvt'].split(',')[0]).split('-')[0]
            packSlot = (evt['slotEvt'].split(',')[0]).split('-')[1]
            evtType = evt['slotEvt'].split(',')[1]
            print("find matching event, pack evt info: slotDay[{0}], slot[{1}], evt[{2}]".format(packSlotDay, packSlot, evtType))
            
            if (evtType == 'OPEN') and (packSlot == slot) and (day.capitalize() == packSlotDay.capitalize()):
                return True
            else:
                print('Not matching evt!')
    
    return False

def medicineTakenAction(day, slot):
    print("[TODO] Medicine is taken")
    srv.pushMedTaken()

def medicineMissedAction(day, slot):
    print("Medicine is missed!!")
    srv.pushMissingDosage()

def generateReminder(schedule, day, slot):
    schedule[day][slot]['reminders'] = schedule[day][slot]['reminders'] - 1
    print(" Remaining reminders after this one is [{0}]".format(schedule[day][slot]['reminders']))
    print("[TODO] generate Reminder")
    srv.pushReminder()


def updateSlotStatus(schedule, day, slot):
    if schedule[day][slot]['status'] == '':
        # check for the openning event
        if findOpenEvent(day, slot):
            # Update the schedule entry that medicine is taken
            schedule[day][slot]['status'] = 'taken'
            medicineTakenAction(day,slot)
            return True
        else:
            # check on reminder left for this slot
            if schedule[day][slot]['reminders'] == 0:
                schedule[day][slot]['status'] = 'missed'
                medicineMissedAction(day,slot)
                return True
            else:
                print('updateSlotStatus: generate reminder: {0}'.format(schedule[day][slot]['reminders']))
                generateReminder(schedule, day, slot)
                return True

    else:
        #print('updateSlotStatus: schedule is ready with: {0}'.format(schedule[day][slot]['status']))
        return False

def generateTempAlarm(temp):
    print('[TODO] Generate Temp Alarm')
    srv.pushTempAlarm(temp)

def checkTemp(schedule):
    if schedule['tempAlarm'] == "":
        for tempEvt in tempEvtList:
            if int(tempEvt['temp']) > schedule['tempThreshold']:
                schedule['tempAlarm'] = 'high'
                generateTempAlarm(int(tempEvt['temp']))
                return True

def reminderThread():

    try:
        while True:
            # Get current time
            day = getCurrDay()
            slot = getCurrSlot()
            hr = getCurrHr()
            print('current data: day[{0}], slot[{1}], hr[{2}]'.format(day,slot,hr))

            # Get Schedule
            bUpdateSchedule = False
            schedule = fetchSchedule()

            # check schedule to see if there is a medicine slot
            if (day in schedule):
                # If time within slot time and there is a medicine slot
                if  inWindow(schedule, hr) and (slot in schedule[day]):
                    bUpdateSchedule = updateSlotStatus(schedule, day, slot)
                else:
                    print('No Medicine at this time')

            
            # Check on Temp Alarms
            if checkTemp(schedule):
                updateSchedule(schedule)

            # Update Schdule
            if bUpdateSchedule:
                updateSchedule(schedule)

            time.sleep(10)
    finally:
        print('Finally reminderThread!')


def main():
    print("The main smartOrganizer brain")

    try:
        dvcThreadObj = Thread(target=tagBle.catchSmartDevice, args=(insertEvent,))
        reminderThreadObj = Thread(target=reminderThread)

        dvcThreadObj.start()
        time.sleep(30)
        reminderThreadObj.start()
    
    finally:
        print('Finally main!')

if __name__ == "__main__":
    main()
    

