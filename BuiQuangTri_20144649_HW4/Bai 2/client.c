#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>

void sendFile(char fs_name[], int client_sock, int len)
{

    char buff[BUFSIZ];
    char trash[BUFSIZ];
    int fd;
    int bytes_sent = 0;
    struct stat file_stat;
    ssize_t bytes_received;
    int offset;
    char file_size[256];

    fd = open(fs_name, O_RDONLY);

    if (fd == -1)
    {
        fprintf(stderr, "Error opening file --> %s", strerror(errno));
    }
    else
    {
        if (fstat(fd, &file_stat) < 0)
        {
            fprintf(stderr, "Error fstat --> %s", strerror(errno));
        }


        if (file_stat.st_size > 1000000000) 
        {
            printf("file to large to transfer!!\n");
        }

        else
        {

            sprintf(file_size, "%d", file_stat.st_size);
            printf("\nFile size : %d\n", file_stat.st_size);
            /* Sending file size */
            bytes_sent = send(client_sock, file_size, sizeof(file_size), 0);
            if (bytes_sent <= 0)
            {
                printf("\nConnection closed!\n");
            }
            recv(client_sock, trash, BUFSIZ - 1, 0);

            //send file name
            int bytes_sent = send(client_sock, fs_name, len, 0);
            if (bytes_sent <= 0)
            {
                printf("\nConnection closed!\n");
            }
            recv(client_sock, trash, BUFSIZ - 1, 0);

            printf("[Client] Sending %s to the Server... \n", fs_name);

            bzero(buff, BUFSIZ);
            int fs_block_sz;
            offset = 0;

            while (fs_block_sz = sendfile(client_sock, fd, &offset, BUFSIZ) > 0)
            {
                printf("sending\n");
            }
            printf("Ok File %s from Client was Sent!\n", fs_name);

            bytes_received = recv(client_sock, buff, BUFSIZ - 1, 0);
            if (bytes_received <= 0)
            {
                printf("\nError!Cannot receive data from sever!\n");
                //break;
            }
            buff[bytes_received] = '\0';
            printf("Reply from server: %s", buff);
        }
    }
}

int isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    if (result != 0)
        return 1;
    else
        return 0;
}

int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        printf("error, too many or too few arguments\n");
        return 1;
    }
    //check if input id is valid
    if (isValidIpAddress(argv[1]) == 0)
    {
        printf("Not a valid ip address\n");
        return 1;
    }

    int client_sock;
    struct sockaddr_in server_addr; /* server's address information */
    struct stat file_stat;
    int len;
    char fs_name[BUFSIZ];

    //Step 1: Construct socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);

    //Step 2: Specify server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    //Step 3: Request to connect server
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
    {
        printf("\nError!Can not connect to sever! Client exit NOW! ");
        return 0;
    }

    //Step 4: Communicate with server
    while (1)
    {
        printf("\nInsert file to send:");
        fgets(fs_name, BUFSIZ, stdin);
        if (strcmp(fs_name, "\n") == 0)
        {
            printf("Closing now\n");
            close(client_sock);
            exit(0);
        }
        int msg_len = strlen(fs_name);
        fs_name[msg_len - 1] = '\0';
        len = sizeof(fs_name);
        sendFile(fs_name, client_sock, len);
    }

    //Step 4: Close socket
    close(client_sock);
    return 0;
}
