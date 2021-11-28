import tagBle
import datetime
import time
import json
from threading import Thread, Event, Lock


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
        
    if (hr in range(schedule["AM_Window"][0], schedule["AM_Window"][1])) or (hr in range(schedule["PM_Window"][0], schedule["PM_Window"][1])):
        return True
    else:
        return False


def insertEvent(event, data):
    global slotEvtList
    global tempEvtList
    
    evt = {"day": getCurrDay(), "slot": getCurrSlot(), "Hr":getCurrHr(), "data": data}
    if event == 'slot':    
        print('Slot event: day[{0}], slot[{1}], hr[{2}], data[{3}]'.format(evt['day'],evt['slot'],evt['Hr'],evt['data']))
        slotEvtList.append(evt)

    else:
        print('Temp event: day[{0}], slot[{1}], hr[{2}], data[{3}]'.format(evt['day'],evt['slot'],evt['hr'],evt['data']))
        tempEvtList.append(evt)

def findOpenEvent(day, slot):
    for evt in slotEvtList:
        if day == evt['day'] and slot == evt['slot']:
            packSlotDay = (evt['data'].split(',')[0]).split('-')[0]
            packSlot = (evt['data'].split(',')[0]).split('-')[1]
            evtType = evt['data'].split(',')[1]
            print("find matching event, pack evt info: slotDay[{0}], slot[{1}], evt[{2}]".format(packSlotDay, packSlot, evtType))
            
            if (evtType == 'OPEN') and (packSlot == slot) and (day.capitalize() == packSlotDay.capitalize()):
                return True
            else:
                print('Not matching evt!')
    
    return False

def medicineTakenAction(day, slot):
    print("[TODO] Medicine is taken")

def medicineMissedAction(day, slot):
    print("[TODO] Medicine is missed!!")

def generateReminder(schedule, day, slot):
    schedule[day][slot]['reminders'] = schedule[day][slot]['reminders'] - 1
    print(" Remaining reminders after this one is [{0}]".format(schedule[day][slot]['reminders']))
    print("[TODO] generate Reminder")


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


    # if no medicine event generate a reminder
        # decrement the no of remaining reminders   -> reminder counter
        # repeat the check afte reminder Timeout    -> reminder timeout
    # if there is opening event
        # cancel the alarm and consider medicine as taken -> marker for taken medicine
# else: 
    # generate alarm for the missed dose
    # mark this slot as missing

def reminderThread():
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
                print('time is not within the window ({0})', inWindow(schedule, hr))


        # Update Schdule
        if bUpdateSchedule:
            updateSchedule(schedule)

        time.sleep(10)




def main():
    print("The main smartOrganizer brain")
    dvcThreadObj = Thread(target=tagBle.catchSmartDevice, args=(insertEvent,))
    reminderThreadObj = Thread(target=reminderThread)

    dvcThreadObj.start()
    reminderThreadObj.start()

if __name__ == "__main__":
    main()

