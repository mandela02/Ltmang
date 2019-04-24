#include <stdio.h> /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <errno.h>


enum
{
	encoder,
	decoder
};

/* Receive and echo message to client */
void *
echo(void *);

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

void receiveFile(int conn_sock)
{
	FILE *file_out;
	char command[BUFSIZ];
	memset(command, 0, strlen(command));
	char name[BUFSIZ];
	memset(name, 0, strlen(name));
	char fileData[BUFSIZ];
	memset(fileData, 0, strlen(fileData));
	char payload[BUFSIZ];
	memset(payload, 0, strlen(payload));
	char *fr_name = "output/";
	char firstloglocation[BUFSIZ];
	memset(firstloglocation, 0, strlen(firstloglocation));
	int opcode;
	int length;
	int key;
	int clientCommand;
	printf("wait for file name ...\n");
	recv(conn_sock, name, BUFSIZ, 0);
	strcpy(firstloglocation, fr_name);
	fr_name = strcat(firstloglocation, name);
	file_out = fopen(fr_name, "wb");
	if (!file_out)
	{
		printf("Error: cannot open file. make sure \"output\" folder is exist!\n");
		send(conn_sock, "3000", sizeof("3000"), 0);
	}

	else
	{
		send(conn_sock, "OK", sizeof("OK"), 0);
		//encode or decode
		recv(conn_sock, command, BUFSIZ, 0);
		send(conn_sock, "Success command\n", sizeof("Success command\n"), 0);
		process(command, sizeof(command), &opcode, &length, &payload);
		key = atoi(payload);
		if (key >= 26)
		{
			key = (key % 26);
		}
		clientCommand = opcode ? encoder : decoder;
		//receive file
		length = 100;
		while (length != 0)
		{
			memset(fileData, 0, strlen(fileData));
			memset(payload, 0, strlen(payload));
			recv(conn_sock, fileData, BUFSIZ, 0);
			send(conn_sock, "Success data\n", sizeof("Success data\n"), 0);
			process(fileData, sizeof(fileData), &opcode, &length, &payload);
			if (length != 0)
				fputs(payload, file_out);
		}
		fclose(file_out);
		if (clientCommand == encoder)
			caesarDecoder(fr_name, key, conn_sock);
		else if (clientCommand == decoder)
			caesarEncoder(fr_name, key, conn_sock);
		printf("done!\n");
	}
}

void removeSubstring(char *s, const char *toremove)
{
	while (s = strstr(s, toremove))
		memmove(s, s + strlen(toremove), 1 + strlen(s + strlen(toremove)));
}

void sendFile(FILE *fp, int conn_sock)
{
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
	filelen = ftell(fp);
	rewind(fp);
	payload = (char *)malloc((1) * sizeof(char));
	int byte_sent;

	for (i = 0; i < filelen; i++)
	{
		byte_sent = fread(payload, 1, 1, fp);
		//fseek(fp, 0, 1);
		strcat(clientMsg, opcode);

		if (byte_sent < 10)
		{
			sprintf(length, "0%d", byte_sent);
		}
		else
			sprintf(length, "%d", byte_sent);

		strcat(clientMsg, length);
		strcat(clientMsg, payload);
		send(conn_sock, clientMsg, sizeof(clientMsg), 0);
		recv(conn_sock, trash, BUFSIZ - 1, 0);
		memset(clientMsg, 0, strlen(clientMsg));
		memset(length, 0, strlen(length));
	}
	strcpy(clientMsg, "2000");
	send(conn_sock, clientMsg, sizeof(clientMsg), 0);
	recv(conn_sock, trash, BUFSIZ - 1, 0);
	if (payload)
		free(payload);
	printf("Complete!\n");
}

int caesarEncoder(char *fr_name, int key, int conn_sock)
{
	char trash[BUFSIZ];
	FILE *file_in;
	FILE *file_out;
	char ch;
	file_in = fopen(fr_name, "rb");
	char *encoder = "_encode";
	char firstloglocation[BUFSIZ];
	memset(firstloglocation, 0, strlen(firstloglocation));
	strcpy(firstloglocation, fr_name);
	char *encodeFileName;
	removeSubstring(firstloglocation, ".txt");

	encodeFileName = strcat(firstloglocation, encoder);

	char fileToRemove[BUFSIZ];
	memset(fileToRemove, 0, strlen(fileToRemove));
	strcpy(fileToRemove, encodeFileName);

	file_out = fopen(encodeFileName, "wb+");
	while ((ch = fgetc(file_in)) != EOF)
	{
		if (ch >= 'a' && ch <= 'z')
		{
			ch = ch - key;
			if (ch < 'a')
			{
				ch = ch + 'z' - 'a' + 1;
			}
		}
		else if (ch >= 'A' && ch <= 'Z')
		{
			ch = ch - key;
			if (ch < 'A')
			{
				ch = ch + 'Z' - 'A' + 1;
			}
		}
		fprintf(file_out, "%c", ch);
	}
	removeSubstring(encodeFileName, "output/");
	send(conn_sock, encodeFileName, BUFSIZ, 0);
	sendFile(file_out, conn_sock);
	fclose(file_out);
	fclose(file_in);
	remove(fr_name);
	remove(fileToRemove);
	printf("%s __ %s __ DONE! \n", fr_name, fileToRemove);
}

int caesarDecoder(char *fr_name, int key, int conn_sock)
{
	char trash[BUFSIZ];
	FILE *file_in;
	FILE *file_out;
	char ch;
	file_in = fopen(fr_name, "rb");
	char *decoder = "_decode";
	char firstloglocation[BUFSIZ];
	memset(firstloglocation, 0, strlen(firstloglocation));

	strcpy(firstloglocation, fr_name);
	char *decodeFileName;
	removeSubstring(firstloglocation, ".txt");
	//removeSubstring(firstloglocation, "_encode");
	//removeSubstring(firstloglocation, "_decode");

	decodeFileName = strcat(firstloglocation, decoder);

	char fileToRemove[BUFSIZ];
	memset(fileToRemove, 0, strlen(fileToRemove));
	strcpy(fileToRemove, decodeFileName);

	file_out = fopen(decodeFileName, "wb+");
	while ((ch = fgetc(file_in)) != EOF)
	{
		if (ch >= 'a' && ch <= 'z')
		{
			ch = ch + key;

			if (ch > 'z')
			{
				ch = ch - 'z' + 'a' - 1;
			}
		}
		else if (ch >= 'A' && ch <= 'Z')
		{
			ch = ch + key;

			if (ch > 'Z')
			{
				ch = ch - 'Z' + 'A' - 1;
			}
		}
		fprintf(file_out, "%c", ch);
	}
	removeSubstring(decodeFileName, "output/");
	send(conn_sock, decodeFileName, BUFSIZ, 0);
	sendFile(file_out, conn_sock);
	fclose(file_out);
	fclose(file_in);
	remove(fr_name);
	remove(fileToRemove);
	printf("%s __ %s __ DONE! \n", fr_name, fileToRemove);
}

int main(int argc, char const *argv[])
{
	if (argc != 2)
	{
		printf("error, too many or too few arguments\n");
		printf("Correct format is /.server YourPort\n");
		return 1;
	}
	int listen_fd, *conn_fd, *new_sock;

	struct sockaddr_in server;  /* server's address information */
	struct sockaddr_in *client; /* client's address information */
	int sin_size;
	pthread_t tid;

	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{ /* calls socket() */
		perror("\nError: ");
		return 0;
	}
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[1]));		/* Remember htons() from "Conversions" section? =) */
	server.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY puts your IP address automatically */

	if (bind(listen_fd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		perror("\nError: ");
		return 0;
	}

	if (listen(listen_fd, BACKLOG) == -1)
	{
		perror("\nError: ");
		return 0;
	}

	sin_size = sizeof(struct sockaddr_in);
	client = malloc(sin_size);
	while (1)
	{
		conn_fd = malloc(sizeof(int));
		if ((*conn_fd = accept(listen_fd, (struct sockaddr *)client, &sin_size)) == -1)
			perror("\nError: ");

		if (pthread_create(&tid, NULL, &echo, conn_fd) < 0)
		{
			perror("could not create thread");
			return 1;
		}
	}
	return 0;
}

void *echo(void *arg)
{
	int conn_fd;
	conn_fd = *((int *)arg);
	pthread_detach(pthread_self());
	receiveFile(conn_fd);
	free(arg);
}
