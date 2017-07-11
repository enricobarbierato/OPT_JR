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
	/* Way to make calculation */
	int mode; /* How the objective function is calculated */

	/* CSV file parameters related to the single application */
	char * app_id;
    double w;					/* Weight application (different from the first) */
    //double w1;					/* Weight first application */
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

    float nu;

    /* Calculated values */
    int bound;
    int R;
    int Rnew;
    int cores;
    int newCores;

	struct List *next;
};
typedef struct List sList;
// The element corresponding to the event


/*
 * Function templates
 */
void freeList(sList * pointer);
void readList(sList *);
void searchResult(sResult *, char *);
void printRow(sList *);
void freeResultList(sResult * );
void addParameters(int, sList ** , sList ** ,  char *, double , double ,  double , double , double , double , double , double , int , double , double, char *, int  );
sList * returnARow(sList **  );
void readResult(sResult *);

void addResult(sResult ** , sResult ** ,  int , double, char * );

#endif /* LIST_H_ */
