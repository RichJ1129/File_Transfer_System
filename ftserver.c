//
// Created by Richard Joseph on 2/24/20.
//

#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <ctype.h>
#include <fcntl.h>
#include <vector>
#include <dirent.h>


struct sockaddr_in serv_addr;
struct sockaddr_in cli_addr;


/* Function
 * Prints error message that is passed to it. Obtained
 * from linux source in readme file.*/

void error(char *msg) {
    perror(msg);
    exit(0);
}

char *getContents(char *fileName) {
    char *source = NULL;

    FILE *myFile = fopen(fileName, "r");

    if(myFile == NULL) {
        error("Unable to open the file");
    }

    if (myFile != NULL) {
        if (fseek(myFile, 0L, SEEK_END) == 0) {
            long bufferSize = ftell(myFile);

            if(bufferSize == -1) {
                error("Invalid File");
            }

            source = (char*) malloc(sizeof(char) * (bufferSize + 1));

            if (fseek(myFile, 0L, SEEK_SET) != 0) {
                error("Unable to read");
            }

            size_t newLength = fread(source, sizeof(char), bufferSize, myFile);

            if (ferror(myFile) != 0){
                fputs("Error reading file", stderr);
            }
            else {
                source[newLength++] = "\0";
            }
        }
    }

    fclose(myFile);
    return source;
}

void checkStart(int argc) {
    if (argc < 2) {
        fprintf(stderr, "Error, 2 argument expected\n");
    }
}


int dirContent(char *filePath[]) {
    DIR *d;
    struct dirent *dir;
    int size = 0;
    int numFiles = 0;

    d = opendir(".");

    if (d) {
        int i = 0;
        while ((dir = readdir(d)) != NULL) {
            if(dir->d_type == DT_REG) {
                filePath[i] = dir->d_name;
                size += strlen(filePath[i]);
                i++;
            }
        }
        numFiles = i - 1;
    }
    closedir(d);

    return size + numFiles;
}

int openSocket() {
    int socketNew;

    //socket(int domain, int type, int protocol)
    socketNew = socket(AF_INET, SOCK_STREAM, 0);

    if (socketNew < 0) {
        error("Error opening the socket");
    }

    return socketNew;
}


void bindSocket(char *port[], int socket) {
    int portNum;

    portNum = atoi(port[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portNum);

    if (bind(socket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    }
}

int acceptConnection(int socket) {
    socklen_t clilen;
    int socketConnection;

    clilen = sizeof(cli_addr);

    socketConnection = accept(socket, (struct sockaddr *) &cli_addr, &clilen);

    if (socketConnection < 0) {
        error("Error with the accept");
    }

    return socketConnection;
}

void sendData(int newSocket) {
    char buffer[1000];
    memset(buffer, 0, sizeof(buffer));

    sleep(3);

    int ch = 0;

    int fd = open("PP.txt", O_RDONLY);

    while (true) {
        int bytesRead = read(fd, buffer, sizeof(buffer) - 1);
        if (bytesRead == 0) {
            break;
        }


        if(bytesRead < 0) {
            fprintf(stderr, "error\n");
            return;
        }

        void *p = buffer;

        while(bytesRead > 0) {
            int bytesWrite = send(newSocket, p, sizeof(buffer), 0);
            if (bytesWrite < 0) {
                return;
            }
            bytesRead -= bytesWrite;
            p = (char*)p + bytesWrite;
        }
        memset(buffer, 0, sizeof(buffer));
    }
    memset(buffer, 0, sizeof(buffer));
    strcpy(buffer, "__done__");
    send(newSocket, buffer, sizeof(buffer),0);
    close(newSocket);

    printf("The file was received");
}

void sendDirectory(int data_socket, char ** files, int numFiles) {
    sleep(3);

    for (int i = 0; i < numFiles; i++){
        send(data_socket, files[i], 100,0);
    }
    // send done message when done
    char * done_message = "done";
    send(data_socket, done_message, strlen(done_message),0);
    // close socket and free address information
    close(data_socket);
}


void messageClient(int newSocket, char *buffer) {
    ssize_t n;
    ssize_t size = strlen(buffer) + 1;
    ssize_t total = 0;

    while (total < size) {
        n = write(newSocket, buffer, size - total);
        total += n;

        if(n < 0) {
            error("Message not able to be sent");
        } else if (n == 0) {
            total = size - total;
        }
    }
}


void numberClient(int newSocket, int num) {
    ssize_t n = 0;

    n = write(newSocket, &num, sizeof(int));

    if (n < 0) {
        error("Number unable to be sent");
    }
}

void sendFile(int newSocket, char *fileName) {
    char *contents;

    contents = getContents(fileName);

    numberClient(newSocket, strlen(contents));

    messageClient(newSocket, contents);
}

void recMessage(int newSocket, char *buffer, size_t size) {
    char temp[size + 1];

    ssize_t n;
    size_t total = 0;

    while(total < size) {
        n = read(newSocket, temp + total, size - total);
        total += n;

        if (n < 0) {
            error("Error with receiving message");
        }
    }
    strncpy(buffer, temp, size);
}


int recNum(int newSocket) {
    int num;
    ssize_t n = 0;

    n = read(newSocket, &num, sizeof(int));

    if(n < 0) {
        error("Unable to receive number");
    }

    return num;
}


int handleReq(int newSocket, int *dataPort) {
    char command[3] = "\0";

    recMessage(newSocket, command, 3);
    *dataPort = recNum(newSocket);

    if (strcmp(command, "-l") == 0) {
        return 1;
    }

    if (strcmp(command, "-g") == 0) {
        return 2;
    }

    return 0;
}

int createServer(int portNum) {
    int sockfd;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        return -1;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(portNum);
    server.sin_addr.s_addr = INADDR_ANY;
    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    if(bind(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0){
        error("ERROR: unable to bind");
    }

    if(listen(sockfd, 10) < 0){
        error("ERROR: unable to listen");
    }

    return sockfd;
}

int main(int argc, char *argv[]) {
    int sockfd;
    int newSocket;
    int dataSock;
    int serverPort;
    int pid;

    checkStart(argc);

    serverPort = atoi(argv[1]);
    sockfd = createServer(serverPort);
    printf("Server is open on %d \n", serverPort);

    while (1) {
       newSocket = accept(sockfd, NULL, NULL);
       if (new)
    }



    return 0;
}
