#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include<pthread.h>

#include "util.h"


#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 3000 /*port*/
#define MESSAGE_SIZE 2048

int main(int argc, char **argv) {

    char username[32];
    char password[32];

    int is_my_turn;

    char client_message[MESSAGE_SIZE];
    char server_message[MESSAGE_SIZE];

    int clientSocket;
    struct sockaddr_in serverAddr;
    socklen_t addr_size;

    // Create the socket. 
    clientSocket = socket(PF_INET, SOCK_STREAM, 0);
    //Configure settings of the server address
    //Address family is Internet 
    serverAddr.sin_family = AF_INET;
    //Set port number, using htons function 
    serverAddr.sin_port = htons(7799);
    //Set IP address to localhost
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
    //Connect the socket to the server using the address
    addr_size = sizeof serverAddr;

    if(connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size)<0){
        printf("Connection failed. \n");
        return -1;
    }
    else printf("Connected to server. \n");

    //Game started

    // Login
    while(1){
        //empty message
        memset(server_message, 0, sizeof(server_message));
        memset(client_message, 0, sizeof(client_message));
        //empty username
        while(1){
            memset(username, 0, sizeof(username));
            printf("Username     : ");
            get_input(username);
            if(contain_white_space_or_empty(username)==1){
                printf("Username should not have space!\n"); 
                continue;
            }
            if(contain_white_space_or_empty(username)==2){
                printf("Username should not be empty!\n"); 
                continue;
            }
            break;
        }

        while(1){
            memset(password, 0, sizeof(password));
            printf("Password     : ");
            get_input(password);
            if(contain_white_space_or_empty(password)==1){
                printf("Password should not have space!\n"); 
                continue;
            }
            if(contain_white_space_or_empty(password)==2){
                printf("Password should not be empty!\n"); 
                continue;
            }
            break;
        }
        // LOGIN username password
        strcpy(client_message, "LOGIN ");
        strcat(client_message, username);
        strcat(client_message, " ");
        strcat(client_message, password);
        strcat(client_message, "");

        if(send(clientSocket , client_message , strlen(client_message) , 0) < 0){
            printf("Send failed\n");
        }

        //Read the client_message from the server into the server_message
        if(recv(clientSocket, server_message, 2048, 0) < 0){
            printf("Receive failed\n");
        }
        //Print the received client_message
        if(DEBUG) printf("Data received: \"%s\"\n", server_message);
        if(strcmp("NOUSER",server_message)==0) printf("Can't find that user!\n");
        if(strcmp("WRONGPASS",server_message)==0) printf("Wrong password!\n");
        if(strcmp("OK",server_message)==0) break;

    }

    //Gameplay
    while(1){
        //empty message
        memset(server_message, 0, sizeof(server_message));
        memset(client_message, 0, sizeof(client_message));
        printf("%s\'s command     : ",username);
        get_input(client_message);

        if(DEBUG) printf("Sent data: \"%s\"\n", client_message);

        if(send(clientSocket , client_message , strlen(client_message) , 0) < 0){
            printf("Send failed\n");
        }

        //Read the client_message from the server into the server_message
        if(recv(clientSocket, server_message, MESSAGE_SIZE, 0) < 0){
            printf("Receive failed\n");
        }
        
        //Print the received client_message
        printf("Server reply: %s\n", server_message);
        if(strcmp("WIN",server_message)==0){
            printf("Congratulation! YOU WIN!\n");
            break;
        }

        if(strcmp("LOSE",server_message)==0){
            printf("Too bad, you lose!\n");
            break;
        }
        if(strcmp("EXIT",client_message)==0) break;
    }
    close(clientSocket);

    return 0;
}