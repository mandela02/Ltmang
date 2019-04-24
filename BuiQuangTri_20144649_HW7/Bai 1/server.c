#include <stdio.h> /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <errno.h>

#define SERVER_PORT 12345

#define TRUE 1
#define FALSE 0

enum
{
	encoder,
	decoder
};

#ifndef INFTIM
#define INFTIM (-1)
#endif

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
		if (key > 25)
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
	int len, rc, on = 1;
	int listen_sd = -1, new_sd = -1;
	int desc_ready, end_server = FALSE, compress_array = FALSE;
	int close_conn;
	char buffer[80];
	struct sockaddr_in addr;
	int timeout;
	struct pollfd fds[200];
	int nfds = 1, current_size = 0, i, j;

	if ((listen_sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket() failed");
		exit(-1);
	}
	// Allow socket descriptor to be reuseable
	rc = setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
	if (rc < 0)
	{
		perror("setsockopt() failed");
		close(listen_sd);
		exit(-1);
	}

	/* Set socket to be nonblocking. All of the sockets for      */
	rc = ioctl(listen_sd, FIONBIO, (char *)&on);
	if (rc < 0)
	{
		perror("ioctl() failed");
		close(listen_sd);
		exit(-1);
	}
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[1]));	 /* Remember htons() from "Conversions" section? =) */
	addr.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY puts your IP address automatically */
	/* Bind the socket                                           */
	if (bind(listen_sd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		perror("\nError: ");
		return 0;
	}
	/* Set the listen back log                                   */
	if (listen(listen_sd, 32) == -1)
	{
		perror("\nError: ");
		return 0;
	}
	/* Initialize the pollfd structure                           */
	memset(fds, 0, sizeof(fds));
	/* Set up the initial listening socket                        */
	fds[0].fd = listen_sd;
	fds[0].events = POLLIN;
	/* Loop waiting for incoming connects or for incoming data   */
	/* on any of the connected sockets.                          */
	do
	{
		/* Call poll() and wait for it to complete.      */
		printf("Waiting on poll()...\n");
		rc = poll(fds, nfds, INFTIM);
		/* Check to see if the poll call failed.                   */
		if (rc < 0)
		{
			perror("  poll() failed");
			break;
		}
		/* Check to see if timeout                 */
		if (rc == 0)
		{
			printf("  poll() timed out.  End program.\n");
			break;
		}
		/* One or more descriptors are readable.  Need to          */
		/* determine which ones they are.                          */
		current_size = nfds;
		for (i = 0; i < current_size; i++)
		{
			/* Loop through to find the descriptors that returned    */
			/* POLLIN and determine whether it's the listening       */
			/* or the active connection.                             */

			if (fds[i].revents == 0)
				continue;
			/* If revents is not POLLIN, it's an unexpected result,  */
			/* log and end the server.                               */
			if (fds[i].revents != POLLIN)
			{
				printf("  Error! revents = %d\n", fds[i].revents);
				end_server = TRUE;
				break;
			}
			if (fds[i].fd == listen_sd)
			{
				printf("  Listening socket is readable\n");

				do
				{
					new_sd = accept(listen_sd, NULL, NULL);
					if (new_sd < 0)
					{
						if (errno != EWOULDBLOCK)
						{
							perror("  accept() failed");
							end_server = TRUE;
						}
						break;
					}

					printf("  New incoming connection - %d\n", new_sd);
					fds[nfds].fd = new_sd;
					fds[nfds].events = POLLIN;
					nfds++;

				} while (new_sd != -1);
			}

			else
			{
				/* Listening descriptor is readable.                   */
				printf("  Descriptor %d is readable\n", fds[i].fd);
				close_conn = FALSE;
				receiveFile(fds[i].fd);
				close_conn = TRUE;
				if (close_conn)
				{
					close(fds[i].fd);
					fds[i].fd = -1;
					compress_array = TRUE;
				}
			}
		}

		if (compress_array)
		{
			compress_array = FALSE;
			for (i = 0; i < nfds; i++)
			{
				if (fds[i].fd == -1)
				{
					for (j = i; j < nfds; j++)
					{
						fds[j].fd = fds[j + 1].fd;
					}
					i--;
					nfds--;
				}
			}
		}

	} while (end_server == FALSE); /* End of serving running.    */

	for (i = 0; i < nfds; i++)
	{
		if (fds[i].fd >= 0)
			close(fds[i].fd);
	}
}
