/*
 * localSearch.h
 *
 *  Created on: Jun 27, 2017
 *      Author: work
 */

#ifndef SRC_LOCALSEARCH_H_
#define SRC_LOCALSEARCH_H_

#include "common.h"
#include "db.h"
#define LOWER 0
#define UPPER 1


char* invokeLundstrom(int , int , char * , int ,  char *);

char *readFolder(char *);
#endif /* SRC_LOCALSEARCH_H_ */
