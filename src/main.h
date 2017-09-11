/*
 * main.h
 *
 *  Created on: Jun 4, 2017
 *      Author: work
 */

#ifndef MAIN_H_
#define MAIN_H_
#include "common.h"


#define NODES 0;

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

void findBound(MYSQL *conn, char *,  sList *);
void localSearch(MYSQL *conn, sList *, int, int);
void calculate_Nu(MYSQL *, char * , sList *, int);

void split (char str[], int *a, int *b);
int ObjFunctionComponent(sList * );







#endif /* MAIN_H_ */
