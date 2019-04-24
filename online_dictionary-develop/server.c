#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <mysql/mysql.h>
#include <mysql/my_global.h>

#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 3000 /*port*/
#define LISTENQ 8 /*maximum number of client connections */

//ket noi db
static char *host = "localhost";
static char *user = "root";
static char *pass = "Crobat12345";
static char *dbname = "english";

unsigned int port = 3306;
static char *unix_socket = NULL;
unsigned int flag = 0;

int main (int argc, char **argv)
{
	int listenfd, connfd, n;
	pid_t childpid;
	socklen_t clilen;
	char buf[MAXLINE];
	struct sockaddr_in cliaddr, servaddr;
	char buf2[MAXLINE];

	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char sql[MAXLINE];

	conn = mysql_init(NULL);

	//connect database
	if (mysql_real_connect(conn, host, user, pass, dbname, port, unix_socket, flag) == NULL)
	{
		printf("MySQL client version: %s\n", mysql_get_client_info());
		exit(1);
	}

	//creation of the socket
	listenfd = socket (AF_INET, SOCK_STREAM, 0);

	//preparation of the socket address 
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	bind (listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	listen (listenfd, LISTENQ);

	printf("%s\n","Server running...waiting for connections.");

	for ( ; ; ) {

		clilen = sizeof(cliaddr);
		connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);
		printf("%s\n","Received request...");

		while ( (n = recv(connfd, buf, MAXLINE,0)) > 0)  {
			char* request[2];
			int i = 0;
			char* p = 	strtok(buf, ":");
			while(p!=NULL){
				request[i++] = p;
				p = strtok(NULL, "\0");
			}
			strcpy(buf2, "");

			if (strcmp(request[0], "SEARCH") == 0)
			{
				puts("SEARCH");
				
				strcpy(sql,"SELECT distinct(word) FROM entries WHERE word LIKE \'");
				strcat(sql, request[1]);
				strcat(sql, "%\' LIMIT 50");

				if (mysql_query(conn, sql))
				{
					exit(1);
				}

				res = mysql_store_result(conn);
				while(row = mysql_fetch_row(res)){
					strcat(buf2, row[0]);
					strcat(buf2, " ");
				}
				//strcat(buf2, "\0");

				send(connfd, buf2, strlen(buf2) + 1, 0);
			} else if (strcmp(request[0], "VIEW") == 0)
			{
				puts("VIEW");
				strcpy(sql,"SELECT distinct(word), definition FROM entries WHERE word = \'");
				strcat(sql, request[1]);
				strcat(sql, "\'");

				if (mysql_query(conn, sql))
				{
					exit(1);
				}

				res = mysql_store_result(conn);
				while(row = mysql_fetch_row(res)){
					strcat(buf2, row[1]);
					strcat(buf2, "\n");
				}
				//strcat(buf2, "\0");

				send(connfd, buf2, strlen(buf2) + 1, 0);
			} else if (strcmp(request[0], "ADD") == 0)
			{
				puts("ADD");
				send(connfd, request[1], strlen(request[1]) + 1, 0);
			}else if (strcmp(request[0], "EDIT") == 0)
			{
				puts("EDIT");
				send(connfd, request[1], strlen(request[1]) + 1, 0);
			} else{
				send(connfd, buf, n, 0);
			}
		}

		if (n < 0) {
			perror("Read error"); 
			exit(1);
		}
		close(connfd);

	}
//close listening socket
	close (listenfd); 
	mysql_close(conn);

}

