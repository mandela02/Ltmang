/*
 * @Author: nam 
 * @Date: 2018-11-02 13:02:40 
 * @Last Modified by: nam
 * @Last Modified time: 2018-11-02 19:45:14
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "myfunc.h"

#define MAX 1024

int main(int argc, char const *argv[])
{
	// valid number of argument
	if (argc != 3)
	{
		printf("Usage: ./client IPAddress PortNumber\n\n");
		return 0;
	}

	message_t message;
	int key, client_sock, request;
	char filename[MAX], buff[MAX + 3], payload[MAX];
	struct sockaddr_in server_addr;
	int bytes_sent, bytes_received;
	char file_buff[100];
	int file_read_byte;
	FILE *f_send;

	// Construct socket
	client_sock = socket(AF_INET, SOCK_STREAM, 0);

	// Specify server address
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);

	// Request to connect server
	if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
	{
		printf("\nError!Can not connect to sever! Client exit imediately! ");
		return 0;
	}

	// Communicate with server

	// show menu
	printf(
		"\n---------------------------------------------\n"
		"FILE ENCODE/DECODE\n"
		"---------------------------------------------\n"
		"1. Encode\n"
		"2. Decode\n"
		"Your choice (1-2, other to quit): ");

	// user choice of request
	scanf("%d", &request);
	getchar();
	if (request != 1 && request != 2)
		return 0;
	request--;

	// user input file to open
	printf("File path: ");
	scanf("%[^\n]%*c", filename);
	if (NULL == (f_send = fopen(filename, "rb")))
	{
		printf("Can not open file \"%s\"!\n", filename);
		exit(0);
	}

	// user input key
	while (1)
	{
		printf("Key: ");
		scanf("%d", &key);
		getchar();
		if (key < 0 || key > 25)
		{
			puts("Key must be a number between 0 and 25!");
			continue;
		}
		break;
	}

	// print key to payload
	memset(&payload, '\0', sizeof payload);
	sprintf(payload, "%d", key);

	// send encode/decode request and key to server
	message = new_message(request, 0, payload);
	mysend(client_sock, &message);

	// send file to server
	while (0 < (file_read_byte = fread(file_buff, 1, 99, f_send)))
	{
		memset(&message, '\0', sizeof(message_t));
		message = new_message(2, file_read_byte, file_buff);
		mysend(client_sock, &message);
		sleep(1);
	}

	// send read file ending
	message = new_message(2, 0, "");
	mysend(client_sock, &message);

	// Close socket
	close(client_sock);
	return 0;
}
