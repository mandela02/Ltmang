#include <stdio.h>          /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define BACKLOG 2   /* Number of allowed connections */
#define BUFF_SIZE 1024

char *resultMessage(char buffer[], int len)
{
	int pos_1 = 1, pos_2 = 0, isError = 0;
	char number[BUFF_SIZE];
	char alpha[BUFF_SIZE];
	number[0] = '\n';
	for (int i = 0; i < len - 1; i++)
	{
		char ch = buffer[i];
		//check if character is number
		if (ch >= '0' && ch <= '9')
		{
			number[pos_1] = ch;
			pos_1++;
		}
		//check if character is alphabet;
		else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
		{
			alpha[pos_2] = ch;
			pos_2++;
		}
		//character is special symbol
		else
		{
			isError = 1;
		}
	}
	number[pos_1] = '\n';
	number[pos_1 + 1] = '\0';
	alpha[pos_2] = '\0';
	//if requested string have any special symbol
	if (isError == 1)
	{
		return "Error\n";
	}
	//sent result back
	else
	{
		return strcat(number, alpha);
	}
}

int main(int argc, char const *argv[])
{

	if (argc != 2)
	{
		printf("error, too many or too few arguments\n");
		return 1;
	}
	int sockfd;
	char buffer[BUFF_SIZE];
	struct sockaddr_in servaddr, cliaddr;

	int listen_sock, conn_sock; /* file descriptors */
	char recv_data[BUFF_SIZE];
	int bytes_sent, bytes_received;
	struct sockaddr_in server; /* server's address information */
	struct sockaddr_in client; /* client's address information */
	int sin_size;
	
	//Step 1: Construct a TCP socket to listen connection request
	if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){  /* calls socket() */
		perror("\nError: ");
		return 0;
	}
	
	//Step 2: Bind address to socket
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[1]));		 /* Remember htons() from "Conversions" section? =) */
	server.sin_addr.s_addr = htonl(INADDR_ANY);  /* INADDR_ANY puts your IP address automatically */   
	if(bind(listen_sock, (struct sockaddr*)&server, sizeof(server))==-1){ /* calls bind() */
		perror("\nError: ");
		return 0;
	}     
	
	//Step 3: Listen request from client
	if(listen(listen_sock, BACKLOG) == -1){  /* calls listen() */
		perror("\nError: ");
		return 0;
	}
	
	//Step 4: Communicate with client
	while(1){
		//accept request
		sin_size = sizeof(struct sockaddr_in);
		if ((conn_sock = accept(listen_sock,( struct sockaddr *)&client, &sin_size)) == -1) 
			perror("\nError: ");
  
		printf("You got a connection from %s\n", inet_ntoa(client.sin_addr) ); /* prints client's IP */
		
		//start conversation
		while(1){
			//receives message from client
			bytes_received = recv(conn_sock, recv_data, BUFF_SIZE-1, 0); //blocking
			if (bytes_received <= 0){
				printf("\nConnection closed");
				break;
			}
			else{
				recv_data[bytes_received] = '\0';
				printf("\nReceive: %s ", recv_data);
			}
			char result[BUFF_SIZE];
			strcpy(result, resultMessage(recv_data, strlen(recv_data)));
			int result_len = strlen(resultMessage(recv_data, strlen(recv_data)));

			//echo to client
			bytes_sent = send(conn_sock, result, result_len, 0); /* send to the client welcome message */
			if (bytes_sent <= 0){
				printf("\nConnection closed");
				break;
			}
		}//end conversation
		close(conn_sock);	
	}
	close(listen_sock);
	return 0;
}
