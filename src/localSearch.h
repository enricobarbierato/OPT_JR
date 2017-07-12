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

#define DOWN 0
#define UP 1


void  Bound(int , int , int , int , int , char *,  int *, int *, int *);
void process(MYSQL *, char * , sList *, double, double, double, double);
char* invokeLundstrom(int , int , char * , int ,  char *);

char *readFolder(char *);
#endif /* SRC_LOCALSEARCH_H_ */
