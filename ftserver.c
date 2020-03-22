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
#include <ctype.h>
#include <fcntl.h>
#include <dirent.h>

/* Function
 * Prints error message that is passed to it. Obtained
 * from linux source in readme file.*/

void error(char *msg) {
    perror(msg);
    exit(0);
}

/* Function
 * Checks to see if the program has the correct amount of
arguments when entered in the console. If it does not
it will quit out of the program.*/

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


char *getContents(char *fileName) {
    char *source = NULL;

    FILE *myFile = fopen(fileName, "r");

    if (myFile == NULL) {
        error("Unable to open the file");
    }

    if (myFile != NULL) {
        if (fseek(myFile, 0L, SEEK_END) == 0) {
            long bufferSize = ftell(myFile);

            if(bufferSize == -1) {
                error("Invalid File");
            }

            source = malloc(sizeof(char) * (bufferSize + 1));

            if (fseek(myFile, 0L, SEEK_SET) != 0) {
                error("Unable to read");
            }

            size_t newLength = fread(source, sizeof(char), bufferSize, myFile);

            if (ferror(myFile) != 0){
                fputs("Error reading file", stderr);
            }
            else {
                source[newLength++] = '\0';
            }
        }
    }

    fclose(myFile);
    return source;
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
    int optional = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optional, sizeof(optional));

    if(bind(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0){
        error("Unable to bind");
    }

    if(listen(sockfd, 10) < 0){
        error("Unable to listen");
    }

    return sockfd;
}

int main(int argc, char *argv[]) {
    int sockfd;
    int newsockfd;
    int datasockfd;
    int server_port_number;
    int pid;

    checkStart(argc);

    server_port_number = atoi(argv[1]);
    sockfd = createServer(server_port_number);
    printf("Server is open on %d \n", server_port_number);

    //Infinite loop until user presses CTRL + C
    while (1) {
       newsockfd = accept(sockfd, NULL, NULL);
       if (newsockfd < 0) {
           error("The socket was not able to accept");
       }

       pid = fork();

       if (pid < 0) {
           error("Error");
       }

       if (pid == 0){
           close(sockfd);
           int command = 0;
           int data_port_number;
           int newsock;

           printf("Connecting from flip2.\n");
           command = handleReq(newsockfd, &data_port_number);

           if (command == 0) {
               error("Error: Command is invalid.");
           }

           if (command == 1) {
               char *path[100];
               int i = 0;
               int length = 0;

               printf("List directory requested\n");

               length = dirContent(path);
               newsock = createServer(data_port_number);

               datasockfd = accept(newsock, NULL, NULL);

               if (datasockfd < 0) {
                   error("Unable to create socket");
               }

               numberClient(datasockfd, length);


               while (path[i] != NULL) {
                   messageClient(datasockfd, path[i]);
                   i++;
               }

               close(newsock);
               close(datasockfd);
               exit(0);
           }

           if(command == 2) {
               int i = recNum(newsockfd);
               char fileName[255] = "\0";

               recMessage(newsockfd, fileName, i);

               printf("File %s has been requested \n", fileName);

               if (access(fileName, F_OK) == -1) {
                   printf("File not found");
                   char errorMessage[] = "NOT FOUND";
                   numberClient(newsockfd, strlen(errorMessage));
                   messageClient(newsockfd, errorMessage);

                   close(newsockfd);
                   close(datasockfd);
                   exit(1);
               } else {
                   char message[] = "FOUND";
                   numberClient(newsockfd, strlen(message));
                   messageClient(newsockfd, message);
               }


               printf("Sending file %s", fileName);

               newsock = createServer(data_port_number);
               datasockfd = accept(newsock, NULL, NULL);

               if (datasockfd < 0) {
                   error("ERROR");
               }


               sendFile(datasockfd, fileName);

               close(newsock);
               close(datasockfd);
               exit(0);
           }
           exit(0);
       }
    }

    return 0;
}
