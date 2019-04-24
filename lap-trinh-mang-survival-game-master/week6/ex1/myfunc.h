#ifndef MYFUNC_H_
#define MYFUNC_H_

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX 1024

typedef struct
{
    int opcode;
    int length;
    char payload[MAX];
} message_t;

int mysend(int client_sock, message_t *mess)
{
    char buff[MAX + 3];
    int bytes_sent;
    sprintf(buff, "%1d%02d%s", mess->opcode, mess->length, mess->payload);
    if (0 >= (bytes_sent = send(client_sock, buff, strlen(buff), 0)))
    {
        printf("\nConnection closed!\n");
        return 0;
    }
    printf(">>>>>send %s\n", buff);
    return 1;
}

int myrecv(int conn_sock, message_t *mess)
{
	memset(mess, '\0', sizeof(message_t));
    char buff[MAX + 3];
    int bytes_received;
    if (0 >= (bytes_received = recv(conn_sock, buff, MAX - 1, 0)))
    {
        printf("\nConnection closed\n");
        return 0;
    }
    buff[bytes_received] = '\0';
    sscanf(buff, "%1d%02d%*s", &mess->opcode, &mess->length);
    strcpy(mess->payload, &buff[3]);
    printf(">>>>>recv %s\n", buff);
    return 1;
}

message_t new_message(int opcode, int length, char *payload)
{
    message_t mess;
    mess.opcode = opcode;
    mess.length = length;
    strcpy(mess.payload, payload);
    return mess;
}

void printout(message_t message)
{
    printf("\n>>>>>opcode %d\n"
           ">>>>>length %d\n"
           ">>>>>payload %s\n\n",
           message.opcode, message.length, message.payload);
}

#endif