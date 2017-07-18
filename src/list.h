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
    double delta_fo;

    double nu;

    /* Calculated values */



    double currentCores;		/* Initialized to nu_i */
    double nCores;				/* Initialized to the value from look-up table */
    double bound;				/* Bound (number of cores) */
    int R;						/* Bound (time) */

	struct List *next;
};
typedef struct List sList;
// The element corresponding to the event

struct aux
{
	char * app1;
	char * app2;
	int newCoreAssignment1;
	int newCoreAssignment2;
	double deltaFO;
	double delta_i, delta_j;
	struct aux *next;
};
typedef struct aux sAux;
/*
 * Function templates
 */
void freeList(sList * pointer);
void readList(sList *);
void printRow(sList *);
void freeResultList(sResult * );
void addParameters(int, sList ** , sList ** ,  char *, double , double ,  double , double , double , double , double , double , int , double , double, char *, int  );

void freeAuxList(sAux * pointer);
void readAuxList(sAux *);
void printAuxRow(sAux *);
void freeAuxList(sAux * );
void addAuxParameters(sAux ** , sAux ** ,  char * , char * , int , int , double, double, double);
void commitAssignment(sList *, char *,  double );
sAux * findMinDelta(sAux * );
int checkTotalCores(sList * pointer, double N);

sList * returnARow(sList **  );
void readResult(sResult *);

void addResult(sResult ** , sResult ** ,  int , double, char * );

#endif /* LIST_H_ */
