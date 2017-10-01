/*
 * prototypes.h
 *
 *  Created on: Jun 4, 2017
 *      Author: work
 */

#ifndef SRC_COMMON_H_
#define SRC_COMMON_H_

#define DEBUG_MSG 512

#define PRODUCT 2
#define COUPLE 1

#define WHOLE_DAGSIM 0
#define RESIDUAL_DAGSIM 1

#define FIRST_APP 0
#define OTHER_APPS 1

#define MAX_APP_LENGTH 1024
#include "db.h"
#include "list.h"

#define DAGSIM 0
#define LUNDSTROM 1

#define ARGS 8 // Command line

#define FILENAME "-f="
#define NUM_N "-n="
#define LIST_LIMIT "-k="
#define DEBUG "-d="
#define MAX_ITERATIONS "-i="
#define SIMULATOR "-s="
#define GLOBAL_FO_CALCULATION "-g"

#define NUMBER 0
#define STRING 1
#define YES_NO 2

#define NO 0
#define YES 1

struct Best
{
	int nNodes;
	int nCores;
	char datasize[16];
	char method[16];

};

//-f="Test1.csv" --n=220  -k=0 -d=y -c=Y -s=dagSim -g=Y


/* Templates */

void addCacheParameters(sPredictorCash ** , sPredictorCash ** ,  char * , int , int , double );
void addListPointers(sListPointers ** ,  sList *);
void addStatistics(sStatistics ** , sStatistics ** , int , int, double );
void addParameters(sList ** ,  sList **, char *, char *, double , double  , double , double , double , double , double , double , double, char *, int  );
sAux * approximatedLoop(sList *, int *, struct optJrParameters );
void addAuxParameters(sAux ** , sAux ** ,  sList * , sList * , int , int , double, double, double);

struct Best bestMatch(char *, int);
void  Bound(sConfiguration *, MYSQL *conn, sList *, struct optJrParameters, int);

void  calculate_Nu(sConfiguration *, MYSQL *, sList *,  struct optJrParameters);
float computeBeta(sAlphaBetaManagement );
float computeAlpha(sAlphaBetaManagement , float );
void commitAssignment(sList *, char *,  double, struct optJrParameters );
int checkTotalCores(sList * pointer, double N);

void debugMessage(char * string, struct optJrParameters par);
void debugInformational(char * string, struct optJrParameters par);
int doubleCompare(double, double);
void DBerror(MYSQL *, char * );
MYSQL * DBopen(char * , char * , char *, char * );
void DBclose(MYSQL *conn);
void DBinsertrow(MYSQL * , char *, char *, float );
void debugBanner(char * string, struct optJrParameters par);
MYSQL_ROW executeSQL(MYSQL *, char *, struct optJrParameters);

double elapsedTime(struct timeval , struct timeval );
char * extractWord(char * , int );
char * extractItem(const char *const string, const char *const left, const char *const right);
char * extractRowN(char *, int );
char * extractRowMatchingPattern(char *text, char *pattern);

sListPointers * fixInitialSolution(sList *applications,  struct optJrParameters);
void findBound(sConfiguration *, MYSQL *conn, char *,  sList *, struct optJrParameters);
void freeStatisticsList(sStatistics * );
void freeParametersList(sList * pointer);
void freeApplicationList(sListPointers * pointer);
void freeAuxList(sAux * pointer);
sAux * findMinDelta(sAux * );

void howAmIInvoked(char **, int );

char * getfield(char* , int);
char *getConfigurationValue(sConfiguration *pointer, char * variable);
double getCsi(double , double );

void initialize(sConfiguration * configuration, MYSQL *conn, sList * application_i, struct optJrParameters par);
char* invokePredictor(sConfiguration * , MYSQL *, int , int , char * , int ,  char *, char *, char *, struct optJrParameters, int);

void localSearch(sConfiguration *, MYSQL *conn, sList *, int, int, struct optJrParameters);
char * ls(char *, struct optJrParameters);
char * LundstromPredictor(sConfiguration *, int , char * , struct optJrParameters);

double max(double, double);

double ObjFunctionGlobal(sConfiguration *, MYSQL *conn, sList *, struct optJrParameters);
int   ObjFunctionComponent(sConfiguration * ,MYSQL *, sList *,  struct optJrParameters );
int   ObjFunctionComponentApprox(sList *, struct optJrParameters );

char * parseConfigurationFile(char *, int);
struct optJrParameters parseCommandLine(char **args, int argc);
char * parseArg(char * string, char * gap, int type);
void printConfigurationFile(sConfiguration *pointer);
void printCacheParameters(sPredictorCash * );
void printRow(sList *, struct optJrParameters);
void printAuxRow(sAux *, struct optJrParameters);
void printRow(sList *, struct optJrParameters);
void printOPT_JRPars(struct optJrParameters par );

sConfiguration * readConfigurationFile();
void readStatistics(sStatistics *, struct optJrParameters);
void writeList(MYSQL *conn, char *,sList *, struct optJrParameters);
void readList(sList *, struct optJrParameters);
void readAuxList(sAux *, struct optJrParameters);
void readListPointers(sListPointers *, struct optJrParameters);
char *readFolder(char *);
char * replace(char * , char *);
int read_line(FILE *, char *, size_t );
char * readFile(char * );
void readSolution(sList *pointer);
MYSQL_ROW retrieveTimeFromDBCash(MYSQL *conn, char *sessionId, char *appId, int datasize, int ncores );
char * _run(char *,  struct optJrParameters );

double searchCacheParameters(sPredictorCash * , char * , int , int );
void split (char str[], int *a, int *b);
sList * searchApplication(sList * , char *);

void Usage();

void writeFile(const char *, const char *);



#endif /* SRC_COMMON_H_ */
