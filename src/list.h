/*
 * list.h
 *
 *  Created on: 13 mar 2016
 *      Author: Enrico
 */

#ifndef LIST_H_
#define LIST_H_

#define HYP_INTERPOLATION_POINTS  2

struct optJrParameters
{
	char filename[1024];
	int debug;
	int cache;
	int globalFOcalculation;
	int K;
	int simulator;
	int number;
	int maxIterations;
};


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


struct Configuration
{
	char * variable;
	char *value;
	struct Configuration *next;
};
typedef struct Configuration sConfiguration;

struct List
{
	/* Way to invoke the algorithm */
	int mode; /* How the objective function is calculated */

	/* profile parameters related to the single application */
	char * session_app_id;
	char * app_id;
    int w;					/* Weight application  */
    double term_i;
    double chi_0;
    double chi_C;
    double m;
    double M;
    double V;
    double v;
    double Deadline_d;
    double csi;
    char * stage;
    int datasetSize;

    /* Calculated values */

    double nu_d;				/* nu value */
    int  currentCores_d;		/* Initialized to nu_i */
    int  nCores_DB_d;			/* Initialized to the value from look-up table */
    int bound;					/* Bound (number of cores) */
    double R_d;					/* Value of R as per the predictor */
    double R_bound_d;			/* Bound (R) */
    double baseFO;				/* base FO value (used to calculate the delta) */

    float alpha;				/* First parameter for Hyperbolic interpolation */
    float beta;					/* Second parameter for Hyperbolic interpolation */
    sAlphaBetaManagement sAB;

    int boundIterations;		/* Metrics */

    int vm;						/* Read from OPTIMIZER_CONFIGURATION_TABLE */

	struct List *next;
};
typedef struct List sList;


struct PredictorCash
{
	char app_id[1024];
	int ncores;
	int datasize;
	double output;
	struct PredictorCash *next;
};
typedef struct PredictorCash sPredictorCash;

// The element corresponding to the event




struct ListPointers
{
	sList *app;
	struct ListPointers *next;
};

typedef struct ListPointers sListPointers;

struct Statistics
{
	int iteration;
	int size;
	double FO_Total;
	struct Statistics *next;
};
typedef struct Statistics sStatistics;




struct aux
{
	sList * app1; // ToDO: replace to the pointer to the application -> DONE
	sList * app2;// ToDO: replace to the pointer to the application -> DONE
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



#endif /* LIST_H_ */
