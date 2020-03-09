import sys
import os
from os import path
from struct import *
from time import sleep
from socket import *


def check_start():
    if len(sys.argv) < 5 or len(sys.argv) > 6:
        print("Error, Invalid number of args")
        exit(1)

    elif (sys.argv[1] != "flip1" and sys.argv[1] != "flip2" and sys.argv[1] != "flip3"):
        print("Error, Invalid Server")
        exit(1)

    elif (int(sys.argv[2]) > 65535 or int(sys.argv[2]) < 1024) or \
            (sys.argv[3] == "-l" and (int(sys.argv[4]) > 65535 or int(sys.argv[4]) < 1024)) or \
            (sys.argv[3] == "-g" and (int(sys.argv[5]) > 65535 or int(sys.argv[5]) < 1024)):

        print("Error, Invalid control port number")
        exit(1)

    elif (sys.argv[3] != "-l" and sys.argv[3] != "-g"):
        print("Error, Invalid command")
        exit(1)


def get_dir_list(new_socket):
    dir_list = new_socket.recv(4)
    dir_list = unpack("I", dir_list)
    rec = str(new_socket.recv(dir_list[0]), encoding="UTF-8").split("\x00")

    for items in rec:
        print(items)


def get_server_file(new_socket, file):
    buffer = rec_server_msg(new_socket)

    if path.isfile(file):
        file = file.split(".")[0] + "_copy.txt"
    with open(file, "w") as new_file:
        new_file.write(buffer)

def send_message(new_socket, message):
    out_msg = bytes(message, encoding="UTF-8")
    new_socket.sendall(out_msg)

def send_number(new_socket, num):
    out_num = pack('i', num)
    new_socket.send(out_num)


def send_to_server(new_socket, cmd, port_num):
    send_message(new_socket, cmd + "\0")
    send_number(new_socket, port_num)


def rec_server_msg(new_socket):
    ds = new_socket.recv(4)
    ds = unpack("I", ds)
    return rec_full_msg(new_socket, ds[0])


def rec_full_msg(new_socket, n):
    rec = ""

    while len(rec) < n:
        packet = str(new_socket.recv(n - len(rec)), encoding="UTF-8")
        if not packet:
            return None
        rec += packet
    return rec



def connect_server(host_name, port_num):
    s = socket.socket(AF_INET, SOCK_STREAM)
    s.connect((host_name, port_num))
    return s


# def file_list(data_socket):
#     file_name = data_socket.recv(100)
#     while file_name != "done":
#         print(file_name)
#         file_name = data_socket.recv(100)


def receive_file(s):
    f = open(sys.argv[4], "w")
    buffer = s.recv(1000)
    while "__done__" not in str(buffer):
        f.write(str(buffer))
        buffer = s.recv(1000)





# def get_file(new_socket, )


# def get_list(data_socket):
#     file_name = data_socket.recv(100)
#     while file_name != "done":
#         print(file_name)
#         file_name = data_socket.recv(100)





# def get_address():
#     s = socket(AF_INET, SOCK_STREAM)
#     s.connect(("8.8.8.8"), 80)
#     return s.getsockname()[0]





# def get_info(client_socket):
#     if sys.argv[3] == "-l":
#         print("File List:")
#         port_num = 4
#     elif sys.argv[3] == "-g":
#         print("Requesting file {}".format(sys.argv[4]))
#         port_num = 5
#
#     client_socket.send(sys.argv[port_num])
#     client_socket.recv(1024)
#
#     if sys.argv[3] == "-l":
#         client_socket.send("l")
#     else:
#         client_socket.send("g")
#
#     client_socket.recv(1024)
#     client_socket.send(get_address())
#     response = client_socket.recv(1024)
#
#     if response == "bad":
#         print("Invalid Command")
#         exit(1)
#
#     if sys.argv[3] == "-g":
#         client_socket.send(sys.argv[4])
#         response = client_socket.recv(1024)
#         if response != "File found":
#             print("File not found")
#             return
#     data_socket = connect_server()
#
#     if sys.argv[3] == "-l":
#         get_list(data_socket)
#     elif sys.argv[3] == "-g":
#         recieve_file(data_socket)
#
#     data_socket.close()


def main():
    check_start()
    server_name = sys.argv[1] + ".engr.oregonstate.edu"
    server_port = int(sys.argv[2])
    new_socket = connect_server(server_name, server_port)


    command = sys.argv[3]
    port_number = sys.argv[-1]


    send_to_server(new_socket, command, port_number)

    if command == "-l":
        data_sock = connect_server(server_name, port_number)
        print("Receiving Directory from {}. {}".format(sys.argv[1], port_number))
        get_dir_list(data_sock)
        data_sock.close()

    if command == "-g":
        send_number(new_socket, len(sys.argv[4]))

        send_message(new_socket, sys.argv[4] + "\0")

        message = rec_server_msg(new_socket)

        if message == "FILE NOT FOUND":
            print("{}: {} says {}".format(sys.argv[4], sys.argv[1], port_number))
        elif message == "FILE FOUND":
            print("Receiving File")

            data_sock = connect_server(sys.argv[1], port_number)
            get_server_file(data_sock, sys.argv[4])
            print("Transfer Complete")

            data_sock.close()
    new_socket.close()



if __name__ == '__main__':
    main()


