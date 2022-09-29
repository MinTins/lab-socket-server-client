##!/usr/bin/python           # This is server.py file                                                                                                                                                                           


import socket               # Import socket module
from threading import Thread

def on_new_client(clientsocket,addr):
    try:
        clientsocket.send("Success connection.".encode())

        while True:
            inp_msg = clientsocket.recv(1024).decode()
            print(addr, ' >> ', inp_msg)
            
            if inp_msg.lower() == "who":
                out_msg = "Test get info"

            elif inp_msg.lower() == "exit":

                break

            else:
                out_msg = "0"

            #out_msg = ""

            clientsocket.send(out_msg.encode())

    except (ConnectionResetError, ConnectionAbortedError):
        print("Closed connection on", addr)

    clientsocket.send("Connection closed.".encode())
    clientsocket.close()
    print("Connection closed", addr)


def main(host=None, port=5000):

    if host is None:
        host = socket.gethostname()

    s = socket.socket()

    port = int(port)

    print('Server started!')
    print('Waiting for clients...')

    s.bind((host, port))        # Bind to the port
    s.listen(5)                 # Now wait for client connection.

    while True:
       c, addr = s.accept()     # Establish connection with client.
       print('Got connection from', addr)
       Thread(target=on_new_client,args=(c,addr)).start()

    s.close()


if __name__ == "__main__":
    import sys

    args = ()

    if 1 < len(sys.argv) < 4:
        args = sys.argv[1:]

    a = Thread(target=main, args=args, daemon=True).start()

    while True:
        pass