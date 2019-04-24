#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

void process(char *clientMsg, int sizeMsg, int *opcode, int *length, char *payload)
{
	int i, size = 0;
	char lengthMsg[2];
	char opcodeMsg[1];
	opcodeMsg[0] = clientMsg[0];
	*opcode = atoi(opcodeMsg);
	lengthMsg[0] = clientMsg[1];
	lengthMsg[1] = clientMsg[2];
	*length = atoi(lengthMsg);
	for (i = 3; i < sizeMsg - 1; i++)
	{
		payload[size] = clientMsg[i];
		size++;
	}
}

void sendFile(FILE *fp, int client_sock)
{

	ssize_t read;
	char opcode[1] = "2";
	char *payload;
	char length[BUFSIZ];
	memset(length, 0, strlen(length));
	char clientMsg[BUFSIZ];
	memset(clientMsg, 0, strlen(clientMsg));
	int i;
	char trash[BUFSIZ];
	long filelen;
	fseek(fp, 0, SEEK_END);
	filelen = ftell(fp); // Get the current byte offset in the file
	rewind(fp);
	payload = (char *)malloc((1) * sizeof(char));
	int byte_sent;
	for (i = 0; i < filelen; i++)
	{
		byte_sent = fread(payload, 1, 1, fp);
		strcat(clientMsg, opcode);
		if (byte_sent < 10)
		{
			sprintf(length, "0%d", byte_sent);
		}
		else
			sprintf(length, "%d", byte_sent);
		strcat(clientMsg, length);
		strcat(clientMsg, payload);
		send(client_sock, clientMsg, sizeof(clientMsg), 0);
		recv(client_sock, trash, BUFSIZ - 1, 0);
		memset(clientMsg, 0, strlen(clientMsg));
		memset(length, 0, strlen(length));
	}
	strcpy(clientMsg, "2000");
	send(client_sock, clientMsg, sizeof(clientMsg), 0);
	recv(client_sock, trash, BUFSIZ - 1, 0);
	if (payload)
		free(payload);
	printf("Send to server complete\n");
}

void receiveFile(int client_sock)
{
	FILE *file_out;
	char fileData[BUFSIZ];
	char payload[BUFSIZ];
	memset(fileData, 0, strlen(fileData));
	memset(payload, 0, strlen(payload));
	int opcode;
	int length;
	int key;
	int clientCommand;
	//receive file
	char fr_name[BUFSIZ];
	recv(client_sock, fr_name, BUFSIZ, 0);
	file_out = fopen(fr_name, "wb");
	//receive file
	length = 100;
	while (length != 0)
	{
		memset(fileData, 0, strlen(fileData));
		memset(payload, 0, strlen(payload));
		recv(client_sock, fileData, BUFSIZ, 0);
		send(client_sock, "Success data\n", sizeof("Success data\n"), 0);
		process(fileData, sizeof(fileData), &opcode, &length, &payload);
		if (length != 0)
			fputs(payload, file_out);
	}
	fclose(file_out);
	printf("Receive file complete!\n");
}

int main(int argc, char const *argv[])
{
	char trash[BUFSIZ];
	int client_sock;
	struct sockaddr_in server_addr; /* server's address information */
	int msg_len, bytes_sent, bytes_received;

	//Step 1: Construct socket
	client_sock = socket(AF_INET, SOCK_STREAM, 0);

	//Step 2: Specify server address
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);

	//Step 3: Request to connect server
	if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
	{
		printf("\nError!Can not connect to sever! Client exit immediately! ");
		return 0;
	}

	//Step 4: Communicate with server
	struct stat file_stat;
	int len;
	char fs_name[BUFSIZ];
	int choice, key;
	char keyCaesar[BUFSIZ];
	char size[BUFSIZ];
	char nameMsg[BUFSIZ];
	memset(nameMsg, 0, strlen(nameMsg));
	char welcomeMsg[BUFSIZ];
	memset(welcomeMsg, 0, strlen(welcomeMsg));

	FILE *fp;
	puts(welcomeMsg);
	printf("\n-----------------------\n");
	printf("Caesar Cipher  PROGRAM\n");
	printf("\n-----------------------\n");
	printf(" 0. Encode\n 1. Decode \n other. Exit\n");
	printf("Your choice (0 or 1, other to exit): ");
	scanf("%d", &choice);
	char clientMsg[BUFSIZ];
	switch (choice)
	{
	//encode
	case 0:
	case 1:
		if (choice == 0)
			printf("\nInsert file to encode:");
		if (choice == 1)
			printf("\nInsert file to decode:");

		scanf("%s", fs_name);
		len = sizeof(fs_name);
		fp = fopen(fs_name, "rb");
		if (fp == NULL)
		{
			printf("Error! Could not open file\n");
			break;
		}
		send(client_sock, fs_name, sizeof(fs_name), 0);
		recv(client_sock, nameMsg, BUFSIZ, 0);
		if (strcmp(nameMsg, "3000") == 0)
			printf("something wrong with server. try again later!");
		else
		{
			printf("\nInsert key:");
			scanf("%d", &key);
			sprintf(keyCaesar, "%d", key);
			if (sizeof(key) < 10)
				sprintf(clientMsg, "%d0%d%d", choice, sizeof(key), key);
			else
				sprintf(clientMsg, "%d%d%d", choice, sizeof(key), key);
			printf("MESS: %s\n", clientMsg);
			//send encode message
			send(client_sock, clientMsg, sizeof(clientMsg), 0);
			recv(client_sock, trash, BUFSIZ - 1, 0);
			//send file
			sendFile(fp, client_sock);
			fclose(fp);
			sleep(1);
			//receive processed file
			receiveFile(client_sock);
		}
		memset(nameMsg, 0, strlen(nameMsg));

		break;

	default:
		exit(1);
		break;
	}

	//Step 4: Close socket
	close(client_sock);
	return 0;
}
