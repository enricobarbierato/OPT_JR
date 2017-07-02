/*
 * performance.h
 *
 *  Created on: Jun 4, 2017
 *      Author: work
 */

#ifndef PERFORMANCE_H_
#define PERFORMANCE_H_
#include "common.h"

const int MAX_LINE_LENGTH = 1024;


const int _APP_ID = 1;
const int _W = 2;
const int _CHI_0 = 3;
const int _CHI_C = 4;
const int _M = 5;
const int _m = 6;
const int _V = 7;
const int _v = 8;
const int _D = 9;
const int _St = 10;
const int _Dsz = 11;

char * localSearch(int, MYSQL *conn, char *, char * , int , int);
void split (char str[], int *a, int *b);
int ObjFunctionComponent(double, int , int );
char* fakeLundstrom(int , int , int , int , char * , int ,  char *);

const double precision = 1/1000;



#endif /* PERFORMANCE_H_ */
