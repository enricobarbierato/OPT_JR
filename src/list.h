/*
 * list.h
 *
 *  Created on: 13 mar 2016
 *      Author: Enrico
 */

#ifndef LIST_H_
#define LIST_H_

#define HYP_INTERPOLATION_POINTS  2





struct Result
{
	int index;
	char * app_id;
    double nu;
	struct Result *next;
};
typedef struct Result sResult;

struct lastSimulatorRun
{
	int nCores;
	double R;

};
typedef struct lastSimulatorRun slastSimulatorRun;

struct AlphaBetaManagement
{
	slastSimulatorRun vec[HYP_INTERPOLATION_POINTS];
	int index;
};
typedef struct AlphaBetaManagement sAlphaBetaManagement;

struct List
{
	/* Way to invoke the algorithm */
	int mode; /* How the objective function is calculated */

	/* CSV file parameters related to the single application */
	char * app_id;
    double w;					/* Weight application  */
    double chi_0;
    double chi_C, chi_c_1;
    double m;
    double M;
    double V;
    double v;
    double Deadline_d;					/* Deadline */
    double csi, csi_1;
    char * stage;
    int datasetSize;

    /* Calculated values */

    double nu_d;
    double currentCores_d;		/* Initialized to nu_i */
    double nCores_d;			/* Initialized to the value from look-up table */
    double bound_d;				/* Bound (number of cores) */
    double R_d;					/* Value of R as per the predictor */
    double R_bound_d;			/* Bound (R) */
    int baseFO;					/* base FO value (used to calculate the delta) */

    float alpha;				/* First parameter for Hyperbolic interpolation */
    float beta;					/* Second parameter for Hyperbolic interpolation */
    sAlphaBetaManagement sAB;

	struct List *next;
};
typedef struct List sList;




// The element corresponding to the event




struct ListPointers
{
	sList *applicazione;
	struct ListPointers *next;
};

typedef struct ListPointers sListPointers;





struct aux
{
	char * app1; // ToDO: sostituire con il puntatore all'applicazione
	char * app2;// ToDO: sostituire con il puntatore all'applicazione
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
void addParameters(int, sList ** , sList ** ,  char *, double , double ,  double , double , double , double , double , double , double , double , double, char *, int  );

void freeAuxList(sAux * pointer);
void readAuxList(sAux *);
void printAuxRow(sAux *);
void freeAuxList(sAux * );
void addAuxParameters(sAux ** , sAux ** ,  char * , char * , int , int , double, double, double);
void commitAssignment(sList *, char *,  double );
sAux * findMinDelta(sAux * );
int checkTotalCores(sList * pointer, double N);
sList * searchApplication(sList * , char *);
void addListPointers(sListPointers ** , sListPointers ** ,  sList *);
void printRow(sList *);
void readListPointers(sListPointers *);

sList * returnARow(sList **  );
void readResult(sResult *);

void addResult(sResult ** , sResult ** ,  int , double, char * );

#endif /* LIST_H_ */
