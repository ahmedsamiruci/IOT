import tagBle
import datetime
import time
from threading import Thread, Event, Lock


# schedule = {
#                 "days": ["Sat", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri"],
#                 "slots": ["AM", "PM"],
#                 "alarms": "on"
#             }

slotEvtList = []
tempEvtList = []

#   day     AM Slot [valid       #Reminders     status]       PM Slot [valid       #Reminders     status]
#   Sat              yes/no        2/1/0        missed/taken
schedule = {
    "reminderTimeout": 30,
    "alarms" : "on",
    "AM_Window": [8,10],
    "PM_Window": [4,6],
    "Sat": {
        "AM": {
            "reminders" : 2,
            "status" : 'NA'
        },
        "PM": {
            "reminders" : 2,
            "status" : 'NA'
        }
    },
    "Sun": {
        "AM": {
            "reminders" : 2,
            "status" : 'NA'
        },
        "PM": {
            "reminders" : 2,
            "status" : 'NA'
        }
    },
    "Mon": {
        "AM": {
            "reminders" : 2,
            "status" : 'NA'
        },
        "PM": {
            "reminders" : 2,
            "status" : 'NA'
        }
    },
    "Tue": {
        "AM": {
            "reminders" : 2,
            "status" : 'NA'
        },
        "PM": {
            "reminders" : 2,
            "status" : 'NA'
        }
    },
    "Wed": {
        "AM": {
            "reminders" : 2,
            "status" : 'NA'
        },
        "PM": {
            "reminders" : 2,
            "status" : 'NA'
        }
    },
    "Thu": {
        "AM": {
            "reminders" : 2,
            "status" : 'NA'
        },
        "PM": {
            "reminders" : 2,
            "status" : 'NA'
        }
    },
    "Fri": {
        "AM": {
            "reminders" : 2,
            "status" : 'NA'
        },
        "PM": {
            "reminders" : 2,
            "status" : 'NA'
        }
    }
}

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
    return t.strftime("%a:%p:%I")

def inWindow(hr):
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
    for evt in evtList:
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

def generateReminder(day, slot):
    print("[TODO] generate Reminder")


def updateSlotStatus(day, slot):
    if schedule[day][slot]['status'] == 'NA':
        # check for the openning event
        if findOpenEvent(day, slot):
            # Update the schedule entry that medicine is taken
            schedule[day][slot]['status'] = 'taken'
            medicineTakenAction(day,slot)
            return 'ready'
        else:
            # check on reminder left for this slot
            if schedule[day][slot]['reminders'] == 0:
                schedule[day][slot]['status'] = 'missed'
                medicineMissedAction(day,slot)
                return 'ready'
            else:
                return 'reminder'
    else:
        return 'ready'

def eventCallback(event, data):
    #print("smartbrain CB: evt:{0}, data:{1}".format(event, data))
    insertEvent(event, data)

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
        # check schedule to see if there is a medicine slot
        if (day in schedule):
            # If time within slot time and there is a medicine slot
            if  inWindow(hr) and (slot in schedule[day]):
                status = UpdateSlotStatus(day, slot)
                if status == 'reminder':
                    generateReminder(day, slot)

        time.sleep(10)




def main():
    print("The main smartOrganizer brain")
    dvcThreadObj = Thread(target=tagBle.catchSmartDevice, args=(eventCallback,))
    reminderThreadObj = Thread(target=reminderThread)

    dvcThreadObj.start()
    reminderThreadObj.start()

if __name__ == "__main__":
    main()

