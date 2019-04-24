
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

enum
{
    false,
    true
};

int i;
struct hostent *host;
struct in_addr ipv4addr;

int isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    if (result != 0)
        return true;
    else
        return false;
}

void fromIpToAddress(char *ip)
{
    inet_pton(AF_INET, ip, &ipv4addr);
    if ((host = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET)) == NULL)
    {
        herror("Error: ");
    }
    else
    {
        printf("Official name: %s\n", host->h_name);
        printf("Alias name: ");
        for (i = 0; host->h_aliases[i] != NULL; i++)
        {
            printf(" %s\n", host->h_aliases[i]);
        }
        printf("\n");
    }
}

void fromAddressToIp(char *address)
{
    if ((host = gethostbyname(address)) == NULL)
    {
        herror("Error: ");
    }
    else
    {
        printf("Official IP address: ");
        for (i = 0; host->h_addr_list[i] != NULL; i++)
        {
            printf("%s", inet_ntoa(*(struct in_addr *)host->h_addr_list[i]));
        }
        printf("\n");
        printf("Alias IP address: ");
        for (i = 0; host->h_aliases[i] != NULL; i++)
        {
            printf("%s\n", inet_ntoa(*(struct in_addr *)host->h_aliases[i]));
        }
        printf("\n");
    }
}

main(int argc, char const *argv[])
{
    int i;

    if (argc != 2)
    {
        printf("error, too many or too few arguments\n");
        return 1;
    }
    if (isValidIpAddress(argv[1]))
    {
        fromIpToAddress(argv[1]);
    }
    else
    {
        fromAddressToIp(argv[1]);
    }

    return 0;
}
