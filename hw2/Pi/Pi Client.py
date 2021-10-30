import socket
import json
import time
import random
import sys

client_socket = None

def cleanup():
    global client_socket
    print('receive keyboard interrupt, terminate the program')
    client_socket.close()
    sys.exit()


def client_program():
    global client_socket

    host = socket.gethostname()  # as both code is running on same pc
    port = 10000  # socket server port number

    client_socket = socket.socket()  # instantiate
    client_socket.connect((host, port))  # connect to the server

    #text = "{\"time\": 6495,\"val\": 106 }"
    value = {'time': 6495, 'val': 106}
    
    while True:
        
        try:
            dataToSend = json.dumps(value)
            print("Send the following data {0} , type+{1}".format(dataToSend, type(dataToSend)))
            client_socket.send(dataToSend.encode())  # send message
            data = client_socket.recv(1024).decode()  # receive response

            print('Received from server: ' + data)  # show in terminal

            #Convert from string to JSON Object
            rxValue = json.loads(data)
            print("rxValue = {0}".format(rxValue))

            time.sleep(2)

            #client_socket.close()  # close the connection

            value['time'] = random.randint(2,5000)
            value['val'] = random.randint(0,500)

        except KeyboardInterrupt:
            print('receive keyboard interrupt, terminate the program')
            cleanup()
        

if __name__ == '__main__':
    client_program()