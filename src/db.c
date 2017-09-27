/*
 * db.c
 *
 *  Created on: Apr 10, 2017
 *      Author: work
 */
#include <stdio.h>
#include <string.h>
#include "db.h"


/*
 * Standard error procedure for DB operations
 */
void DBerror(MYSQL *conn, char * msg)
{
	printf("DBError: %s %s\n", msg, mysql_error(conn));
	mysql_close(conn);
	exit(-1);
}


MYSQL_ROW executeSQL(MYSQL *conn, char *statement, struct optJrParameters par)
{
	MYSQL_RES *result;
	char error[256];
	char debugMsg[1024];

	if (mysql_query(conn, statement))
	{
		char error[512];
		sprintf(error,"SQL failure executeSQL on %s", statement);
		DBerror(conn, error);
	}


	result = mysql_store_result(conn);
	sprintf(debugMsg, "statement %s\n", statement);debugMessage(debugMsg, par);
	if (result == NULL)
	{
		sprintf(error, "Failure: ExecuteSQL: statement was %s\n", statement);
		DBerror(conn, error);
		return NULL;
	}
	else
		return(mysql_fetch_row(result));

}
/*
 * Open a DB connection
 */
MYSQL * DBopen(char * host, char * login, char * passw, char *dbName)
{


MYSQL *con = mysql_init(NULL);

  if (con == NULL)
  {
      fprintf(stderr, "DBError: %s\n", mysql_error(con));
      exit(1);
  }

  if (mysql_real_connect(con, host, login, passw,
          dbName, 0, NULL, 0) == NULL) DBerror(con, "mysql_real_connect");

  return con;
}

/*
 * Close the Db connection
 */
void DBclose(MYSQL *conn)
{
	mysql_close(conn);
}


