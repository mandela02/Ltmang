#include <stdio.h>
#include <mysql/mysql.h>
#include <stdlib.h>
#include <mysql/my_global.h>
#include <string.h>

//ket noi db
static char *host = "localhost";
static char *user = "root";
static char *pass = "12345678";
static char *dbname = "dictionary";

unsigned int port = 3306;
static char *unix_socket = NULL;
unsigned int flag = 0;

int main (int argc, char **argv)
{
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char sql[1000];

	conn = mysql_init(NULL);

	if (mysql_real_connect(conn, host, user, pass, dbname, port, unix_socket, flag) == NULL)
	{
		printf("MySQL client version: %s\n", mysql_get_client_info());
		exit(1);
	}
	else
	{
		strcpy(sql,"SELECT distinct(word) FROM entries WHERE word LIKE \'%");
		strcat(sql, "mus");
		strcat(sql, "%\'");
		if (mysql_query(conn, sql))
		{
			exit(1);
		}

		res = mysql_store_result(conn);
		while(row = mysql_fetch_row(res)){
			printf("%s\n", row[0]);
		}
	}
	mysql_close(conn);
}

