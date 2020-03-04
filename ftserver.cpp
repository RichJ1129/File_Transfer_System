//
// Created by Richard Joseph on 2/24/20.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

struct sockaddr_in serv_addr;
struct sockaddr_in cli_addr;


/* Function
 * Prints error message that is passed to it. Obtained
 * from linux source in readme file.*/


void error(char *msg) {
    perror(msg);
    exit(0);
}


void checkStart(int argc) {
    if (argc < 2) {
        fprintf(stderr, "Error, no port provided\n");
    }
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


int main(int argc, char *argv[]) {
    int newSocket;
    int socketConnect;

    checkStart(argc);
    newSocket = openSocket();
    bzero((char *) &serv_addr, sizeof(serv_addr));
    bindSocket(argv, newSocket);
    listen(newSocket, 5);
    socketConnect = acceptConnection(newSocket);



//    newsockfd = accept(sockfd,
//                       (struct sockaddr *) &cli_addr,
//                       &clilen);
//    if (newsockfd < 0)
//        error("ERROR on accept");
//
//    FILE *dataFile;
//    dataFile = fopen("PP.txt", "r");
//
//
    close(socketConnect);
    close(newSocket);

    return 0;
}