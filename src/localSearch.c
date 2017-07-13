/*
 * localSearch.c
 *
 *  Created on: Jun 27, 2017
 *      Author: work
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "localSearch.h"

#define STEP 1
#define MAX_ITERATIONS 2

//#define FAKE_LUNDSTROM 1



/*
 * 		Name:					invokeLundstrom
 * 		Input parameters:		int nNodes, int nCores, char * memory, int datasize,  char *appId
 * 		Output parameters:		The output from Lundstrom predictor
 * 		Description:			This function works on one data log folder only, whose path can be found in wsi_config.xml file withe the keyword "FAKE".
 *
 * 								First, the data folder name is changed according to Nodes_Cores convention as for the input parameters,
 * 								the following lines build a command that executes lundstrom tool.
 *
 */

char* invokeLundstrom(int nNodes, int nCores, char * memory, int datasize,  char *appId)
{
	char parameters[128];
	char cmd[128];
	char mvCmd[64];
	char path[64];
	/*
	 * Prepare the data
	 */

	char dir[16];

	strcpy(path, parseConfigurationFile("FAKE", 1));
	strcpy(dir, readFolder(path));

	sprintf(mvCmd, "cd %s;mv %s %d_%d_%s_%d", path, dir, nNodes, nCores, memory, datasize);

	sprintf(parameters, "%d %d %s %d %s", nNodes, nCores, memory, datasize, appId);
	sprintf(cmd, "cd %s;python run.py %s", parseConfigurationFile("LUNDSTROM_HOME", 1), parameters);

	printf("mvCmd = %s\n", mvCmd);
	printf("cmd = %s\n", cmd);

	_run(mvCmd);


	return _run(cmd);
}



/*
 * 		Name:					fakeLundstrom
 * 		Input parameters:		int mode, int count, int nNodes, int nCores, char * memory, int datasize,  char *appId
 * 		Output parameters:		simulation of the output produced by Lundstrom predictor
 * 		Description:			According to the "mode" and "count" variables, the function simulates the output Lundstrom using
 * 								hard coded values.
 *
 */

char* fakeLundstrom(int mode, int count, int nNodes, int nCores, char * memory, int datasize,  char *appId)
{
	char *time = (char *)malloc(16);

switch(mode)
{
case DOWN:
	switch(count)
	{
		case 0:
					strcpy(time, "38000");
					break;
			case 1:
					strcpy(time, "39000");
					break;
			case 2:
					strcpy(time, "40000");
					break;
			default:
					printf("fakeLundstrom out of range parameter (count %d)\n", count);
					break;
	}
	break;
case UP:
	switch(count)
		{
		case 0:
				strcpy(time, "42000");
				break;
		case 1:
				strcpy(time, "41000");
				break;
		case 2:
				strcpy(time, "40000");
				break;
		default:
				printf("fakeLundstrom out of range parameter (count %d)\n", count);
				break;

		}
		break;
}

	return time;
}


/*
 * 		Name:				Bound
 * 		Input parameters:	int mode, int deadline, int nNodes, int nCores, int datasetSize, char *appId,
 * 		Output parameters:	int *R (initial Lundstrom estimate for the given cores, the bound (number of cores), and the time for the determined bound.
 * 		Description:		This function calculates the bound given a certain deadline and number of nodes, cores. Lundstrom method is invoked until an upper bound,
 * 							consisting of the number of nodes, is found (once that the time calculated by the predictor, a rollback is performed to
 * 							return the last "safe" number of core and time.
 *
 */
void  Bound(int mode, int deadline, int nNodes, int nCores, int datasetSize, char *appId, int *R, int *bound, int *Rnew)
{

	int lundtsromOutput;

	int BTime = 0;
	int BCores = 0;


	// mode is a Temporary variable: it says to FakeLjundstrom to take a value greater (0) or lower (1) than D

	*bound = nCores;
	//
#ifdef FAKE_LUNDSTROM
	int fake = 0;
	lundtsromOutput = atoi(fakeLundstrom(mode, fake++, nNodes, nCores, "8G", datasetSize, appId));
#else
	lundtsromOutput = atoi(invokeLundstrom( nNodes, nCores, "8G", datasetSize, appId));
#endif
	*R = lundtsromOutput;

	printf("INIZIO R %d D %d\n", lundtsromOutput, deadline);
	if (lundtsromOutput > deadline)
	while (lundtsromOutput > deadline)
	{

		BTime = lundtsromOutput;
		//printf("(up) time = %d Rnew =%d\n", time, BTime);
		nCores = nCores + STEP;
#ifdef FAKE_LUNDSTROM
		lundtsromOutput = atoi(fakeLundstrom(UP, fake++, nNodes, nCores, "8G", datasetSize, appId));
#else
		lundtsromOutput = atoi(invokeLundstrom( nNodes, nCores, "8G", datasetSize, appId));
#endif
		BCores = nCores;

	}
	else
		while (lundtsromOutput < deadline)
		{
				if (nCores == 0)
				{
					printf("nCOres is currently 0. Cannot invoke Lundstrom\n");
					exit(-1);
				}
				nCores = nCores - STEP;
#ifdef FAKE_LUNDSTROM
				lundtsromOutput = atoi(fakeLundstrom(DOWN, fake++, nNodes, nCores, "8G", datasetSize, appId));
#else
				lundtsromOutput = atoi(invokeLundstrom( nNodes, nCores, "8G", datasetSize, appId));
#endif
				BCores = nCores;
				BTime = lundtsromOutput;
				//printf("(down) time = %d Rnew =%d\n", time, BTime);
		}

	*Rnew = BTime;
	*bound = BCores;
	printf("D = %d R = %d Rnew = %d bound = %d\n", deadline, *R, *Rnew, *bound);


}

/*
 * 		Name:						ObjFunctionComponent
 * 		Input parameters:			The pointer to the applications list
 * 		Output parameters:			The contribution to the calculation of the objective function
 * 		Description:				Currently, only two methods are supported. Note that the algorithm's choice is stored in the "mode" field
 * 									of the application structure.
 *
 */
int ObjFunctionComponent(sList * pointer)
{


	double output;

	if (pointer == NULL)
	{
		printf("ObjFunctionComponent failure: NULL pointer\n");
		exit(-1);
	}



	//printf("ObjFunctionComponent: App_id %s w %f R %d D %d nCores %d newCores %d\n",pointer->app_id, pointer->w, pointer->R, pointer->D, pointer->cores, pointer->newCores);

	/* Determine how the obj function needs to be calculated */
	switch(pointer->mode)
	{
		case R_ALGORITHM:
			printf("R Algorithm\n");
				if (pointer->R > pointer->D)
					output = pointer->w * (pointer->R - pointer->D);
				else output = 0;
			break;
		case CORES_ALGORITHM:
			printf("Cores Algorithm\n");
				if (pointer->cores > pointer->newCores) output = 0;
				else output = pointer->w * (pointer->Rnew - pointer->D);

			break;
		case NCORES_ALGORITHM:
			printf("NCores Algorithm\n");
				if (pointer->newCores >pointer->bound) output = 0;
				else output = pointer->w * atoi(fakeLundstrom(1, 0, 2, pointer->newCores, "8G", pointer->datasetSize, pointer->app_id)) - pointer->R;
			break;
		default:
			printf("ObjFunctionComponent: unknown case within Switch statement: mode %d\n", pointer->mode);
			exit(-1);
			break;
	}
printf("FO output: %lf\n", output);


	return output;
}

/*
 * 		Name:					findBound
 * 		Input parameters:		MYSQL *conn, char *db, int mode,  int deadline,sList *pointer
 * 		Output parameters:		Updated fields R, bound and Rnew (see "Bound" function for more details)
 * 		Description:			Initially, this function queries the lookup table to find the number of cores, calculated by OPT_IC earlier,
 * 								given a deadline, an application id and a dataset size.
 * 								Secondly, it invokes the Bound function.
 *
 */
void findBound(MYSQL *conn, char *db, int mode,  int deadline, sList *pointer)
{


	int nNodes = 1; // Temporary value
	int nCores =1;// Temporary value


	// Temporary value

	/* Retrieve nCores from the DB
	 *
	 *
        sprintf(statement,
                        "select num_cores_opt from %s.OPTIMIZER_CONFIGURATION_TABLE where application_id='%s' and dataset_size=%d and deadline=%d;"
                        , db, appId, datasetSize, deadline);

        nCores = executeSQL(conn, statement);
	 *
	 */
	pointer->cores = nCores;

	Bound(mode, deadline, nNodes, nCores,
			pointer->datasetSize,
			pointer->app_id,
			&(pointer->R),
			&(pointer->bound),
			&(pointer->Rnew));


}


/*
 * 		Name:					localSearch (WORK IN PROGRESS !)
 * 		Input parameters:		sList * application_i
 * 		Output parameters:		TBD
 * 		Description:			Localsearch algorithm as per functional analysis
 *
 */
void localSearch(sList * application_i)
{
	sList * application_j, *first = application_i;
	int nCoreMov;
	double DELTA_i, DELTA_j;
	double DELTA_fo_App_i, DELTA_fo_App_j;


	printf("\n\nLocalsearch\n");
for (int i = 0; i < MAX_ITERATIONS; i++)
	while (application_i != NULL)
	{
		application_j = first;
		while (application_j != NULL)
		{
			if (strcmp(application_i->app_id, application_j->app_id)!= 0)
			{
				printf("Comparing %s with %s\n", application_i->app_id, application_j->app_id);
				printf("-----------------------------------------------\n");
				nCoreMov = max(application_i->V, application_j->V);

				DELTA_i = nCoreMov/application_i->V;
				DELTA_j = nCoreMov/application_j->V;

				application_i->newCores = application_i->cores + DELTA_i*application_i->V;
				application_j->newCores = application_j->cores - DELTA_j*application_j->V;
				application_i->mode= CORES_ALGORITHM;
				application_j->mode= CORES_ALGORITHM;

				/*
				 * Call object function evaluation
				 */
				DELTA_fo_App_i = ObjFunctionComponent(application_i);
				DELTA_fo_App_j = ObjFunctionComponent(application_j);

				/*
				 * Keep track of the application with whom the comparison was made
				 */
				application_i->app_id_j = (char *)malloc(1024);
				if (application_i->app_id_j == NULL)
				{
					printf("malloc failure in localSearch\n");
					exit(-1);
				}
				strcpy(application_i->app_id_j, application_j->app_id);
				application_i->delta_fo = DELTA_fo_App_i + DELTA_fo_App_j;


				printf("AppId_i %s DELTA_i %lf application_i->newCores %d DELTA_fo_App_i  %lf\n",
						application_i->app_id, DELTA_i,    application_i->newCores,    DELTA_fo_App_i);

				printf("AppId_j %s DELTA_j %lf application_j->newCores %d DELTA_fo_App_j  %lf\n",
						application_j->app_id, DELTA_j,    application_j->newCores,    DELTA_fo_App_j);


				// Update the lists with values used
			}
			application_j = application_j->next;
		}

		application_i = application_i->next;
	}


}

/*
 * 		Name:					process
 * 		Input parameters:		MYSQL *conn, char * uniqueFilename, sList *current, double nu_1, double w1, double csi_1, double chi_c_1
 * 		Output parameters:		none
 * 		Description:			Given nu_1 and other measures related to the first applications:
 * 								- it computes the nu indices for all the other applications:
 * 								- it updates the DB;
 * 								- it calculates the bound for each application
 *
 */
void process(MYSQL *conn, char * uniqueFilename, sList *current, double nu_1, double w1, double csi_1, double chi_c_1)
{

	//double chi_0;
	double chi_C;
	double M;
	double m;
	double v;
	double V;
	double D;
	int rows = 1;
	char * app_id;
	double w;

	double indexF;
	double csi;


	app_id = (char *)malloc(MAX_APP_LENGTH);
	if (app_id == NULL)
	{
	          printf("app_id: malloc_failure in process\n");
	          exit(-1);
    }

	printf("Calculates the indices and update the list\n");

	while (current != NULL)
	    {
	    	strcpy(app_id, current->app_id);

	    	if (rows > 1 ) w = 	current->w;/* Any other application than the first */

	    	//chi_0 = current->chi_0;
	        chi_C = current->chi_C;
	        M = 	current->M;
	        m = 	current->m;
	        V = 	current->V;
	        v = 	current->v;
	        D = 	current->D;

	        csi = getCsi(M/m, V/v);

	        if (rows == 1) indexF = nu_1; else indexF = nu_1*sqrt((w/w1)*(chi_C/chi_c_1)*(csi_1/csi));
	        current->nu = indexF;

	        DBinsertrow(conn, uniqueFilename, app_id, indexF);

	        current->cores = indexF;

	        /*
	           Calculate
	           1) the bound (new number of cores)
	           2) New Lundstrom value (Rnew)
	        */

#ifdef FAKE_LUNDSTROM
	        if (strcmp(current->app_id, "Q26") == 0) findBound(conn, parseConfigurationFile("OptDB_dbName", XML), DOWN, D, current);
	        else findBound(conn, parseConfigurationFile("OptDB_dbName", XML), UP, D, current);
#else
	        findBound(conn, parseConfigurationFile("OptDB_dbName", XML), UP, D, current);
	        //printf("%d %d %d\n", current->R, current->Rnew, current->bound);
	        current->mode = R_ALGORITHM;
#endif

	        current = current->next;
	        rows++;
	     }
}

