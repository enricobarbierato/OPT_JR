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
case 0:
	switch(count)
	{
	case 0:
			strcpy(time, "40000");
			break;
	case 1:
			strcpy(time, "39000");
			break;
	case 2:
			strcpy(time, "38000");
			break;
	default:
			printf("fakeLundstrom out of range parameter (count %d)\n", count);
			break;
	}
	break;
case 1:
	switch(count)
		{
		case 0:
				strcpy(time, "39000");
				break;
		case 1:
				strcpy(time, "40000");
				break;
		case 2:
				strcpy(time, "41000");
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

	int time;
	int fake = 0;


	// mode is a Temporary variable: it says to FakeLjundstrom to take a value greater (9) or lower (1) than D

	*bound = nCores;
	//time = atoi(invokeLundstrom( nNodes, nCores, "8G", datasetSize, appId));
	time = atoi(fakeLundstrom(mode, fake++, nNodes, nCores, "8G", datasetSize, appId));
	*R = time;

//printf("R %d D %d\n", time, deadline);
	if (time > deadline)
	while (time > deadline)
	{
		*Rnew = time;
		nCores = nCores + STEP;
		//time = atoi(invokeLundstrom( nNodes, nCores, "8G", datasetSize, appId));
		time = atoi(fakeLundstrom(0, fake++, nNodes, nCores, "8G", datasetSize, appId));
		*bound = nCores;

	}
	else
		while (time < deadline)
			{
				*Rnew = time;
				nCores = nCores - STEP;
				//time = atoi(invokeLundstrom( nNodes, nCores, "8G", datasetSize, appId));
				time = atoi(fakeLundstrom(1, fake++, nNodes, nCores, "8G", datasetSize, appId));
				*bound = nCores;
				printf("%d %d\n", time, deadline);
			}

	//printf("deadline = %d Lundstrom %d upperBound %d\n", deadline, time, bound);

	/* Return a string including:
	 *
	 * 1) Current number of Nodes;
	 * 2) New number of Cores (bound);
	 * 3) Lundstrom's output for the bound;
	 */

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



	printf("ObjFunctionComponent: %s %f %d %d\n",pointer->app_id, pointer->w, pointer->R, pointer->D);

	/* Determine how the obj function needs to be calculated */
	switch(pointer->mode)
	{
		case FIRST:

				if (pointer->R > pointer->D)
					output = pointer->w * (pointer->R - pointer->D);
				else output = 0;
			break;
		case SECOND:
				if (pointer->cores < pointer->newCores)
					output = pointer->w * (pointer->R - pointer->D);
					 else output = 0;
			break;
		case THIRD:
			break;
		default:
			printf("ObjFunctionComponent: unknown case within Switch statement: mode %d\n", pointer->mode);
			exit(-1);
			break;
	}



	return output;
}

/*
 * 		Name:					findBound
 * 		Input parameters:		MYSQL *conn, char *db, int mode,  int deadline,
 * 		Output parameters:		Updated fields R, bound and Rnew (see "Bound" function for more details)
 * 		Description:			Initially, this function queries the lookup table to find the number of cores, calculated by OPT_IC earlier,
 * 								given a deadline, an application id and a dataset size.
 * 								Secondly, it invokes the Bound function.
 *
 */
void findBound(MYSQL *conn, char *db, int mode,  int deadline, sList **pointer)
{


	int nNodes = 6; // Temporary value
	int nCores = 12; // Temporary value
	/* Retrieve nCores from the DB
	 *
	 *
        sprintf(statement,
                        "select num_cores_opt from %s.OPTIMIZER_CONFIGURATION_TABLE where application_id='%s' and dataset_size=%d and deadline=%d;"
                        , db, appId, datasetSize, deadline);

        nCores = executeSQL(conn, statement);
	 *
	 */


	Bound(mode, deadline, nNodes, nCores,
			(*pointer)->datasetSize,
			(*pointer)->app_id,
			&((*pointer)->R),
			&((*pointer)->bound),
			&((*pointer)->Rnew));


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

//for (int i = 0; i < MAX_ITERATIONS; i++)
	while (application_i != NULL)
	{
		application_j = first;
		while (application_j != NULL)
		{
			if (strcmp(application_i->app_id, application_j->app_id)!= 0)
			{
				nCoreMov = max(application_i->v, application_i->v);

				DELTA_i = application_i->v/nCoreMov;
				DELTA_j = application_j->v/nCoreMov;

				application_i->newCores = application_i->cores + DELTA_i*application_i->cores;
				application_j->newCores = application_j->cores - DELTA_i*application_i->cores;
				application_i->mode= SECOND;
				application_j->mode= SECOND;

				DELTA_fo_App_i = ObjFunctionComponent(application_i);
				DELTA_fo_App_j = ObjFunctionComponent(application_j);

				printf("AppId_i %s 			   DELTA_i %lf application_i->newCores %d DELTA_fo_App_i  %lf\n",
						application_i->app_id, DELTA_i,    application_i->newCores,    DELTA_fo_App_i);

				printf("AppId_j %s 			   DELTA_j %lf application_j->newCores %d DELTA_fo_App_j  %lf\n",
						application_j->app_id, DELTA_j,    application_j->newCores,    DELTA_fo_App_j);


				// Update the lists with values used
			}
			application_j = application_j->next;
		}
		application_i = application_i->next;
	}

	free(application_j);
	free(first);
}


