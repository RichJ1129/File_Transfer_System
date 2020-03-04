import socket
import sys
import os

def main():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    host = socket.gethostbyname(sys.argv[1])
    port = 30000

    s.connect((host, port))

    with open('received_file.txt', 'wb') as f:
        print('file opened')
        while True:
            print('receiving data...')
            data = s.recv(512)
            print('data=%s', (data))
            if not data:
                break
        # write data to a file
            f.write(data)

            f.close()
    print('Successfully get the file')
    s.close()
    print('connection closed')

if __name__ == '__main__':
    main()


