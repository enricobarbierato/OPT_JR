/*
 * localSearch.h
 *
 *  Created on: Jun 27, 2017
 *      Author: work
 */

#ifndef SRC_LOCALSEARCH_H_
#define SRC_LOCALSEARCH_H_

#include "common.h"
#include "list.h"
#include "db.h"

#define R_ALGORITHM 0
#define CORES_ALGORITHM 1
#define NCORES_ALGORITHM 2

#define NO 0
#define YES 1

#define LUNDSTROM 0
#define DAGSIM 1


void  Bound(sConfiguration *, MYSQL *conn, sList *);
void  calculate_Nu(sConfiguration *, MYSQL *, char * , sList *, int);
char* invokeLundstrom(int , int , char * , int ,  char *);
float computeBeta(sAlphaBetaManagement );
float computeAlpha(sAlphaBetaManagement , float );
void  initialize(sConfiguration *, MYSQL *conn, sList *);
double ObjFunctionGlobal(sConfiguration *, MYSQL *conn, sList *);
int   ObjFunctionComponent(sConfiguration * ,MYSQL *, sList * );
int   ObjFunctionComponentApprox(sList * );
sAux * approximatedLoop(sList *, int * );

char *readFolder(char *);
#endif /* SRC_LOCALSEARCH_H_ */
