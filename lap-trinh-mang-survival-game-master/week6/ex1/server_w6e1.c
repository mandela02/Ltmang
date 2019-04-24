/*
* @Author: nam
* @Date:   2018-10-18 12:50:55
 * @Last Modified by: nam
 * @Last Modified time: 2018-11-02 19:45:27
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "myfunc.h"

#define BACKLOG 5
#define MAX 1024

int main(int argc, char const *argv[])
{
    // valid number of argument
    if (argc != 2)
    {
        printf("Usage: ./server PortNumber\n\n");
        return 0;
    }

    message_t message;
    FILE *f_recv;
    int listen_sock, conn_sock, request, key;
    char buff[MAX], *reply;
    int bytes_sent, bytes_received;
    struct sockaddr_in server;
    struct sockaddr_in client;
    socklen_t sin_size;
    int pid;

    // Construct a TCP socket to listen connection request
    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("\nError: ");
        return 0;
    }

    // Bind address to socket
    memset(&server, '\0', sizeof server);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1]));
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listen_sock, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        perror("\nError: ");
        return 0;
    }

    // Listen request from client
    if (listen(listen_sock, BACKLOG) == -1)
    {
        perror("\nError: ");
        return 0;
    }

    puts("Server up and running...\n");

    // Communicate with client
    while (1)
    {
        // accept request
        sin_size = sizeof(struct sockaddr_in);
        if ((conn_sock = accept(listen_sock, (struct sockaddr *)&client, &sin_size)) == -1)
            perror("\nError: ");

        printf("\nYou got a connection from %s\n", inet_ntoa(client.sin_addr)); /* prints client's IP */

        // start conversation on other fork
        pid = fork();
        if (pid < 0)
        {
            perror("Error");
            return 1;
        }

        // on child process
        if (pid == 0)
        {
            // receive request from client
            myrecv(conn_sock, &message);
            printout(message);
            
            // request = 0 => encode
            // request = 1 => decode
            request = message.opcode;
            key = atoi(message.payload);

            if (NULL == (f_recv = fopen("temp_recv", "wb")))
            {
                printf("Can not open file temp_recv");
                exit(0);
            }

            while (1)
            {
                myrecv(conn_sock, &message);
                printout(message);
                if (message.opcode == 2 && message.length > 0)
                {
                    fwrite(message.payload, message.length, 1, f_recv);
                }
                else if (message.opcode == 2 && message.length == 0)
                {
                    puts("File received successfully");
                    break;
                }
            }
            fclose(f_recv);
        }
        else
        {
            // on parent process
            close(conn_sock);
        }
    }
    close(listen_sock);
    return 0;
}
