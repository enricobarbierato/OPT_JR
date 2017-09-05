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


void  Bound(sList *pointer);
void calculate_Nu(MYSQL *, char * , sList *, int);
char* invokeLundstrom(int , int , char * , int ,  char *);
float computeBeta(sAlphaBetaManagement );
float computeAlpha(sAlphaBetaManagement , float );
void initialize(sList *);
double ObjFunctionGlobal(sList *);
int ObjFunctionComponent(sList * );
int ObjFunctionComponentApprox(sList * );
sAux * approximatedLoop(sList *, int * );

char *readFolder(char *);
#endif /* SRC_LOCALSEARCH_H_ */
