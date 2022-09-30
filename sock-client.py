import socket
from threading import Thread



def client_program(host=None, port=1043):

    if host is None:
        host = socket.gethostname()

    port = int(port)

    client_socket = socket.socket()  # instantiate

    try:
        client_socket.connect((host, port))  # connect to the server
        #print(client_socket.recv(1024).decode())

        message = input(" -> ")  # take input

        try:
            while message.lower().strip() != 'bye':
                client_socket.send(message.encode())  # send message

                data= client_socket.recv(1024).decode()
                print("Server >> Client:", data)

                message = input(" -> ")

        except ConnectionResetError:
            print("Server closed the connection.")

        client_socket.close()  # close the connection

    except ConnectionRefusedError:
        print("Server not found.")


if __name__ == '__main__':
    import sys

    args = ()

    if 1 < len(sys.argv) < 4:
        args = sys.argv[1:]

    main_thread = Thread(target=client_program, daemon=True, args=args)
    main_thread.start()
    #main_thread.join()
    # заглушка роботи циклом
    while True:
        pass