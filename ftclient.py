import sys
import os
import socket
import struct
from os import path
from time import sleep

"""Function
Checks to see if the program has the correct amount of
parameters when entered in the console. If it does not
it will quit out of the program.
"""


def check_start():
    if len(sys.argv) < 5 or len(sys.argv) > 6:
        print("Error, Invalid number of args")
        exit(1)

    elif sys.argv[1] != "flip1" and sys.argv[1] != "flip2" and sys.argv[1] != "flip3":
        print("Error, Invalid Server")
        exit(1)

    elif (int(sys.argv[2]) > 65535 or int(sys.argv[2]) < 1024) or \
            (sys.argv[3] == "-l" and (int(sys.argv[4]) > 65535 or int(sys.argv[4]) < 1024)) or \
            (sys.argv[3] == "-g" and (int(sys.argv[5]) > 65535 or int(sys.argv[5]) < 1024)):

        print("Error, Invalid control port number")
        exit(1)

    elif sys.argv[3] != "-l" and sys.argv[3] != "-g":
        print("Error, Invalid command")
        exit(1)


"""Function
Gets contents of the directory and splits them. 
Prints each of them line by line. 
"""


def get_dir_list(new_socket):
    dir_list = new_socket.recv(4)
    dir_list = struct.unpack("I", dir_list)
    file_list = str(new_socket.recv(dir_list[0]), encoding="UTF-8").split("\x00")

    for file in enumerate(file_list):
        print(file)

"""Function
Receives file and then writes the contents to a new
file.
"""


def get_server_file(new_socket, file):
    buffer = rec_server_msg(new_socket)

    if path.isfile(file):
        file = file.split(".")[0] + "_copy.txt"
    with open(file, "w") as new_file:
        new_file.write(buffer)


def send_message(new_socket, message):
    msg = bytes(message, encoding="UTF-8")
    new_socket.sendall(msg)


def send_number(new_socket, num):
    sent_num = struct.pack('i', num)
    new_socket.send(sent_num)


def send_to_server(new_socket, cmd, port_num):
    send_message(new_socket, cmd + "\0")
    send_number(new_socket, port_num)


def rec_server_msg(new_socket):
    b_rec = new_socket.recv(4)
    b_rec = struct.unpack("I", b_rec)
    return rec_full_msg(new_socket, b_rec[0])


"""Function
Works in conjunction with the above function
to receive the message from the server.
"""


def rec_full_msg(new_socket, n):
    full_msg = ""

    while len(full_msg) < n:
        msg = str(new_socket.recv(n - len(full_msg)), encoding="UTF-8")
        if not msg:
            break
        full_msg += msg
    return full_msg


"""Function
Starts the server. Code taken and modularized 
from CS 372 Lecture 15.
"""
def start_server(host_name, port_num):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host_name, port_num))
    return s


def main():
    global file_name
    check_start()
    server_name = sys.argv[1] + ".engr.oregonstate.edu"
    server_port = int(sys.argv[2])
    data_port = int(sys.argv[-1])
    command = sys.argv[3]

    new_socket = start_server(server_name, server_port)

    send_to_server(new_socket, command, data_port)

    if command == "-l":
        sleep(1)
        data_sock = start_server(server_name, data_port)
        print("Receiving Directory List:")
        get_dir_list(data_sock)
        data_sock.close()

    if command == "-g":
        file_name = sys.argv[4]
        send_number(new_socket, len(file_name))
        send_message(new_socket, file_name + "\0")
        message = rec_server_msg(new_socket)

        if message == "NOT FOUND":
            print("{}: {} says {}".format(sys.argv[4], sys.argv[1], data_port))
        elif message == "FOUND":
            print("Downloading File")
            data_sock = start_server(sys.argv[1], data_port)

            get_server_file(data_sock, file_name)
            print("File Transfer Completed Successfully")

            data_sock.close()
    new_socket.close()


if __name__ == '__main__':
    main()


