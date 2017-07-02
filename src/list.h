/*
 * list.h
 *
 *  Created on: 13 mar 2016
 *      Author: Enrico
 */

#ifndef LIST_H_
#define LIST_H_


struct Result
{
	int index;
	char * app_id;
    double nu;
	struct Result *next;
};
typedef struct Result sResult;



struct List
{
	char * app_id;
    double w, w1;
    double chi_0;
    double chi_C, chi_c_1;
    double m;
    double M;
    double V;
    double v;
    int D;
    double csi, csi_1;
    char * stage;
    int datasetSize;
    int bound;
    int R;
    int newR;
    int DELTA;
	struct List *next;
};
typedef struct List sList;
// The element corresponding to the event


/*
 * Function templates
 */

void readList(sList *);
void searchResult(sResult *, char *);
void printRow(sList *);
void freeResultList(sResult * );
void addParameters(sList ** , sList ** ,  char *, double , double , double , double , double , double , double , double , double , int , double , double, char *, int  );
sList * returnARow(sList **  );
void readResult(sResult *);

void addResult(sResult ** , sResult ** ,  int , double, char * );

#endif /* LIST_H_ */
