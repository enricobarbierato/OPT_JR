/*
 * db.h
 *
 *  Created on: Apr 10, 2017
 *      Author: work
 */

#ifndef SRC_DB_H_
#define SRC_DB_H_
#include <my_global.h>
#include <mysql.h>

void DBerror(MYSQL *, char * );
MYSQL * DBopen(char * , char * , char *, char * );
void DBclose(MYSQL *conn);
void DBinsertrow(MYSQL * , char *, char *, float );
int executeSQL(MYSQL *, char *);


#endif /* SRC_DB_H_ */
