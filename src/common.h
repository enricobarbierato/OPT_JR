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

#define FIRST 0
#define SECOND 1
#define THIRD 2



struct Best
{
	int nNodes;
	int nCores;
	char datasize[16];
	char method[16];

};


/* Templates */

int doubleCompare(double, double, double);
double getCsi(double , double , double );
char * parseConfigurationFile(char *, int);
struct Best bestMatch(char *, int);
char * extractWord(char * , int );
char * _run(char * );
char * getfield(char* , int);
void Usage();
void howAmIInvoked(char **, int );
char * LundstromPredictor(int , char * );

#endif /* SRC_COMMON_H_ */
