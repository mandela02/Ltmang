#include <stdio.h> /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#define BACKLOG 2    /* Number of allowed connections */
#define MAXLINE 2048 /* Number of allowed connections */

const char nameOfFile[100][BUFSIZ];
int indexNum = 0;

void addToArray(char *fr_name)
{
    strcpy(nameOfFile[indexNum], fr_name);

    indexNum++;
}
void receiveFile(int conn_sock)
{
    FILE *received_file;
    ssize_t len;
    socklen_t sock_len;
    int file_size;
    int remain_data = 0;
    char size[BUFSIZ];
    char name[BUFSIZ];
    int i;
    int isExist = 0;
    char *fr_name = "output/";

    char firstloglocation[MAXLINE];

    recv(conn_sock, size, BUFSIZ, 0);
    file_size = atoi(size);
    send(conn_sock, "Success size\n", sizeof("Success size\n"), 0);

    recv(conn_sock, name, BUFSIZ, 0);
    strcpy(firstloglocation, fr_name);
    fr_name = strcat(firstloglocation, name);
    send(conn_sock, "Success name\n", sizeof("Success name\n"), 0);
    for (i = 0; i <= indexNum; i++)
    {
        if (strcmp(nameOfFile[i], name) == 0)
            isExist = 1;
    }
    if (isExist != 0)
        send(conn_sock, "File already exist on server\n", sizeof("File already exist on server\n"), 0);
    else
    {
        receiveData(fr_name, conn_sock, file_size);
        addToArray(name);
    }
}

void receiveData(char *fr_name, int conn_sock, int file_size)
{
    int bytes_received, bytes_sent;
    char recv_data[BUFSIZ];
    int remain_data = 0;

    FILE *fr = fopen(fr_name, "w");
    if (fr == NULL)
    {
        printf("File %s Cannot be opened file on server.\n", fr_name);
    }
    else
    {
        remain_data = file_size;

        int fr_block_sz = 0;
        while ((fr_block_sz = recv(conn_sock, recv_data, BUFSIZ, 0)) > 0 && (remain_data > 0))
        {
            int write_sz = fwrite(recv_data, sizeof(char), fr_block_sz, fr);
            remain_data -= fr_block_sz;
            fprintf(stdout, "Receive %d bytes and we hope :- %d bytes\n", fr_block_sz, remain_data);
            if (remain_data == 0)
                break;
        }

        printf("Ok received from client!\n\n\n");
        fclose(fr);

        bytes_sent = send(conn_sock, "Success\n", sizeof("Success\n"), 0); /* send to the client welcome message */
        if (bytes_sent <= 0)
        {
            printf("\nConnection closed");
        }
    }
}

int main(int argc, char const *argv[])
{

    if (argc != 2)
    {
        printf("error, too many or too few arguments\n");
        return 1;
    }

    int listen_sock, conn_sock; /* file descriptors */
    int bytes_sent, bytes_received;
    struct sockaddr_in server; /* server's address information */
    struct sockaddr_in client; /* client's address information */
    int sin_size;
    char recv_name[BUFSIZ];

    //Step 1: Construct a TCP socket to listen connection request
    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    { /* calls socket() */
        perror("\nError: ");
        return 0;
    }

    //Step 2: Bind address to socket
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1]));     /* Remember htons() from "Conversions" section? =) */
    server.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY puts your IP address automatically */
    if (bind(listen_sock, (struct sockaddr *)&server, sizeof(server)) == -1)
    { /* calls bind() */
        perror("\nError: ");
        return 0;
    }

    //Step 3: Listen request from client
    if (listen(listen_sock, BACKLOG) == -1)
    { /* calls listen() */
        perror("\nError: ");
        return 0;
    }

    //Step 4: Communicate with client

    char opcode[BUFSIZ];

    while (1)
    {
        //accept request
        sin_size = sizeof(struct sockaddr_in);
        if ((conn_sock = accept(listen_sock, (struct sockaddr *)&client, &sin_size)) == -1)
            perror("\nError: ");

        printf("You got a connection from %s\n", inet_ntoa(client.sin_addr)); /* prints client's IP */

        while (1)
        {
            //start conversation
            recv(conn_sock, trash, BUFSIZ - 1, 0);
            send(client_sock, opcode, sizeof(opcode), 0);

            receiveFile(conn_sock);
        }
        close(conn_sock);
    }
    close(listen_sock);
    return 0;
}
