#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>

#define MAXLINE 1024

int i;
struct hostent *host;
struct in_addr ipv4addr;

//use this to check if string is IP address
int isValidIpAddress(char ipAddress[])
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    if (result != 0)
        return 1;
    else
        return 0;
}

//convert from IP to address
char *fromIpToAddress(char* ip)
{
    char *message = "Official name: ";
    char firstloglocation[MAXLINE];
    inet_pton(AF_INET, ip, &ipv4addr);
    if ((host = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET)) == NULL)
    {
        herror("Error");
        return "Error: IP address is invalid\n";
    }
    else
    {
        strcpy(firstloglocation, message);
        message = strcat(firstloglocation, host->h_name);
        strcat(message, "\nAlias name:\n");
        for (i = 0; host->h_aliases[i] != NULL; i++)
        {
            strcpy(firstloglocation, message);
            message = strcat(firstloglocation, host->h_aliases[i]);
            strcat(message, "\n");
        }
        return message;
    }
}

//convert form address to ip
char *fromAddressToIp(char address[])
{
    char *message = "Official IP: \n";
    char firstloglocation[MAXLINE];
    char secondloglocation[MAXLINE];
    if ((host = gethostbyname(address)) == NULL)
    {
        herror("Error");
        return "Error\n";
    }
    else
    {
        for (i = 0; host->h_addr_list[i] != NULL; i++)
        {
            strcpy(firstloglocation, message);
            message = strcat(firstloglocation, inet_ntoa(*(struct in_addr *)host->h_addr_list[i]));
            strcat(message, "\n");
        }
        strcat(message, "Alias IP address: \n");

        for (i = 0; host->h_aliases[i] != NULL; i++)
        {
            strcpy(secondloglocation, message);
            message = strcat(secondloglocation, inet_ntoa(*(struct in_addr *)host->h_aliases[i]));
            strcat(message, "\n");
        }
        return message;
    }
}

int main(int argc, char const *argv[])
{
    int i;

    if (argc != 2)
    {
        printf("error, too many or too few arguments\n");
        return 1;
    }
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr, cliaddr;

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(atoi(argv[1]));
    //servaddr.sin_port = htons(PORT);
    bzero(&(servaddr.sin_zero), 8); /* zero the rest of the structure */

    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *)&servaddr,
             sizeof(struct sockaddr)) == -1)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        int len, n;
        len = sizeof(struct sockaddr_in);
        //receive request form client, save in buffer
        n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                     MSG_WAITALL, (struct sockaddr *)&cliaddr,
                     &len);
        if (n < 0)
        {
            perror("Error: ");
            close(sockfd);
            return 0;
        }
        buffer[n - 1] = '\0';
        printf("Recieve form Client : %s\n", buffer);
        //check if requested string is a ip address
        if (isValidIpAddress(buffer))
        {
            printf("IP to address: Processing ...\n");
            n = sendto(sockfd, (const char *)fromIpToAddress("216.58.199.14"),
                       strlen(fromIpToAddress("216.58.199.14")),
                       MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
                       len);
        }
        else
        {
            printf("Address to IP: Processing ...\n");
            n = sendto(sockfd, (const char *)fromAddressToIp("google.com"),
                       strlen(fromAddressToIp("google.com")),
                       MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
                       len);
        }
        if (n < 0)
        {
            perror("Error: ");
            close(sockfd);
            return 0;
        }
        printf("Result sent.\n");
    }
    close(sockfd);
    return 0;
}