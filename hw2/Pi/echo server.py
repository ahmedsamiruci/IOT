import socket
import sys


# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Bind the socket to the port
server_address = (socket.gethostbyname(socket.gethostname()), 10000)
print('starting up on {0} port {1}'.format(server_address, 10000))
sock.bind(server_address)

# Listen for incoming connections
sock.listen(1)

while True:
    # Wait for a connection
    print('waiting for a connection')
    connection, client_address = sock.accept()

    try:
        print('connection from: {0}'.format(client_address))

        # Receive the data in small chunks and retransmit it
        while True:
            data = connection.recv(16)
            print('received {0}'.format(data))
            if data:
                print('sending data back to the client')
                connection.sendall(data)
            else:
                print('no more data from {0}'.format(client_address))
                break
            
    finally:
        # Clean up the connection
        connection.close()