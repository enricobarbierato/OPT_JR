/*
 * prototypes.h
 *
 *  Created on: Jun 4, 2017
 *      Author: work
 */

#ifndef SRC_COMMON_H_
#define SRC_COMMON_H_

#define PRODUCT 2
#define COUPLE 1

#define FIRST_APP 0
#define OTHER_APPS 1

#define MAX_APP_LENGTH 1024
#include "db.h"
#include "list.h"


#define  XML 1
#define  NOXML 0


struct Best
{
	int nNodes;
	int nCores;
	char datasize[16];
	char method[16];

};


/* Templates */
char * ls(char *);
void writeFile(const char *, const char *);
char * replace(char * , char *);
char * extractRowN(char *, int );
int read_line(FILE *, char *, size_t );
char * readFile(char * );
MYSQL_ROW retrieveTimeFromDBCash(MYSQL *conn, char *sessionId, char *appId, int datasize, int ncores );
char * MPI_PrepareCmd(char * , char * , char *, char * , int);
double max(double, double);
int doubleCompare(double, double);
double getCsi(double , double );
char * parseConfigurationFile(char *, int);
struct Best bestMatch(char *, int);
char * extractWord(char * , int );
char * _run(char * );
char * getfield(char* , int);
void Usage();
void howAmIInvoked(char **, int );
char * LundstromPredictor(sConfiguration *, int , char * );
char * MPI_prepareOutput(int index);
double elapsedTime(struct timeval , struct timeval );

#endif /* SRC_COMMON_H_ */
