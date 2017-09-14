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


double executeSQL(MYSQL *conn, char *statement)
{
	MYSQL_RES *result;
	MYSQL_ROW rowdata;
	char errorMsg[256];

	if (conn == NULL)
	  {
	      fprintf(stderr, "DBError: %s %s\n", statement, mysql_error(conn));
	      exit(1);
	  }

	/* Check if the query has returned at least a row */
	if (mysql_query(conn, statement))
			DBerror(conn, "SQL failure executeSQL)\n");

	result = mysql_store_result(conn);
	if (result)
	{
		rowdata = mysql_fetch_row(result);
		if (rowdata)  return atof(rowdata[0]);
	}

return(-1);
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

/*
 * Insert a row in opt table
 */
void DBinsertrow(MYSQL * conn, char *id, char *app_id, float nu)
{
	char * values = (char *)malloc(2048);
	char nuS[__SIZEOF_FLOAT__];

	strcpy(values, "INSERT INTO OPT_SESSIONS_RESULTS VALUES('");
	strcat(values, id);
	strcat(values, "','");
	strcat(values, app_id);
	strcat(values, "',");
	sprintf(nuS, "%g", nu);
	strcat(values, nuS);
	strcat(values, ");");

printf("%s\n", values);
	//if (mysql_query(conn, values))
	//	DBerror(conn, "DBinsertrow failure");

}
