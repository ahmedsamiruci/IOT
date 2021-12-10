from bluepy.btle import Scanner, DefaultDelegate, UUID, Peripheral, BTLEDisconnectError
import hwControls as hw

statusChar = None
evtChar = None
evtCb = None

def defaultEvtCallback(event, data):
    print('default eventCb: evt:{0}, data:{1}'.format(event, data))     

class ScanDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)

    def handleDiscovery(self, dev, isNewDev, isNewData):
        if isNewDev:
            print( "Discovered device {0}".format( dev.addr))
        elif isNewData:
            print ("Received new data from {0}".format( dev.addr))

class NotifyDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)       

    def handleNotification(self, cHandle, data):
        #print("Got notification from handle {0}".format(cHandle))
        #print('Notification Data:{0}'.format(data))
        rxData = data.decode("utf-8").split(':')
        evtCb(rxData[0], rxData[1])

def scanSmartDvc():
    print("Search for SmartOrganizer device...")
    hw.setLedStatus('blue', 'slow2')
    scanner = Scanner() #.withDelegate(ScanDelegate())
    devices = scanner.scan(5.0)

    for dev in devices:    
        for (adtype, desc, value) in dev.getScanData():
            if value == "SmartOrganizer":
                print ("{0} = {1}".format(desc, value))
                return dev   
    return None


def printDvcChar(device):
    dvcChars = device.getCharacteristics()
    print("found {0} chars".format(len(dvcChars)))
    print("----------------")
    for idx, char in enumerate(dvcChars):
        print("Char[{0}]:\tuuid = {1}, handle = {2}".format(idx,char.uuid.getCommonName(), char.getHandle()))
            
    print("----------------")


def connectSmartDvc(scanDvc):
    STATUS_CHAR_UUID = "a84c0884-4514-11ec-81d3-0242ac130003"
    EVT_CHAR_UUID = "a84c094c-4514-11ec-81d3-0242ac130003"

    print("Initiate connection to SmartOrganizer...")
    
    smartDvc = Peripheral(scanDvc)
    smartDvc.setDelegate(NotifyDelegate())
    #print("Reading Chars...")
    #printDvcChar(smartDvc)
    statusChar = smartDvc.getCharacteristics(uuid=STATUS_CHAR_UUID)[0]
    evtChar = smartDvc.getCharacteristics(uuid=EVT_CHAR_UUID)[0]
    evtNotifyHandle = evtChar.getHandle() + 1
    notifySetup = b"\x01\x00"
    smartDvc.writeCharacteristic(evtNotifyHandle, notifySetup)     

    return smartDvc

    # except BTLEDisconnectError as e:
    #     print("connectSmartDvc: {!r}".format(e))
    #     raise BTLEDisconnectError
    # except BaseException as e:
    #     print('{!r}; Exception in connectSmartDvc'.format(e))


def catchSmartDevice(_evtCb=None):
    global evtCb
    if _evtCb != None:
        #print("catchSmartDevice: Set Callback Function")
        evtCb = _evtCb
    else:
       # print("catchSmartDevice: set DeafultCb")
        evtCb = defaultEvtCallback
    
    hw.init()
    smartOrganizer = scanSmartDvc()
    if smartOrganizer != None:
        print("Found a smartOrganizer device")
        
        try:
            connectedDvc = None
            connectedDvc = connectSmartDvc(smartOrganizer)
            print("Connected to SmartOrganizer device")
            hw.setLedStatus('blue', 'on')
            while True:
                connectedDvc.waitForNotifications(5)

        except BTLEDisconnectError as e:
            print("catchSmartDevice: {!r}".format(e))
            catchSmartDevice()
        except BaseException as e:
            print('{!r}; Exception in catchSmartDevice'.format(e))
        finally:
                if connectedDvc is not None: 
                    connectedDvc.disconnect()
    else:
        print("Couldn't find SmartOrganizer!")
        hw.setLedStatus('blue', 'off')



def main():
    print("Main function of tagBLE.py module")
    catchSmartDevice()


if __name__ == "__main__":
    main()