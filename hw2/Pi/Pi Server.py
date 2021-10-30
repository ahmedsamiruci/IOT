from os import error
import socket
import sys
import json
from threading import Thread, Event
from time import sleep

sock = None
connection = None

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

                dataObj = json.loads(data)
                print("Json data {0}".format(dataObj))

                if data:
                    #continue;
                    print('sending data back to the client')
                    connection.send(data.encode())
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
    

def AppDispatch():

    print("App Dispatch Started\n")
    
    while True:

        try:
            print("AppDispatch")
            sleep(5)
        except KeyboardInterrupt:
            global sock
            global connection
            print('receive keyboard interrupt, terminate the program')
            cleanup()


def mainProgram():

    try:
        tcpThreadObj = Thread(target=threadWrapper, args=(tcpThread,))
        AppThreadObj = Thread(target=AppDispatch)

        AppThreadObj.start()
        tcpThreadObj.start()

    except KeyboardInterrupt:
        print('receive keyboard interrupt, terminate the program')
        cleanup()

    


if __name__ == '__main__':
    mainProgram()