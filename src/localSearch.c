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
 * 		Name:
 * 		Input parameters:
 * 		Output parameters:
 * 		Description:
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
 * 		Name:
 * 		Input parameters:
 * 		Output parameters:
 * 		Description:
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
		}
		break;
}

	return time;
}


/*
 * 		Name:
 * 		Input parameters:
 * 		Output parameters:
 * 		Description:
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
 * 		Name:
 * 		Input parameters:
 * 		Output parameters:
 * 		Description:
 *
 */
int ObjFunctionComponent(sList * pointer)
{

	float wF;
	double output;

	if (pointer == NULL)
	{
		printf("ObjFunctionComponent failure: NULL pointer\n");
		exit(-1);
	}

	/* Determine the application: is it the first or any other? */
	switch(pointer->forWhom)
	{
		case FIRST_APP:
			wF = pointer->w1;
			break;
		case OTHER_APPS:
			wF = pointer->w;
			break;
		default:
			printf("ObjFunctionComponent: unknown case within Switch statement: forWhom %d\n", pointer->forWhom);
			exit(-1);
			break;
	}

	printf("ObjFunctionComponent: %f %d %d\n",wF,pointer->R, pointer->D);

	/* Determine how the obj function needs to be calculated */
	switch(pointer->mode)
	{
		case FIRST:
				if (pointer->R > pointer->D)
					output = wF * (pointer->R - pointer->D);
				else output = 0;
			break;
		case SECOND:
				if (pointer->cores > pointer->newCores)
					output = wF * (pointer->R - pointer->D);
					 else output = 0;
			break;
		case THIRD:
			break;
		default:
			printf("ObjFunctionComponent: unknown case within Switch statement: mode %d\n", pointer->forWhom);
			exit(-1);
			break;
	}



	return output;
}

/*
 * 		Name:
 * 		Input parameters:
 * 		Output parameters:
 * 		Description:
 *
 */
void localSearch(MYSQL *conn, char *db, int mode,  char * appId, int datasetSize, int nCores, int deadline, int *R, int *bound, int *Rnew)
{


	int nNodes = 6; // Temporary value
	/* Retrieve nCores from the DB
	 *
	 *
        sprintf(statement,
                        "select num_cores_opt from %s.OPTIMIZER_CONFIGURATION_TABLE where application_id='%s' and dataset_size=%d and deadline=%d;"
                        , db, appId, datasetSize, deadline);

        nCores = executeSQL(conn, statement);
	 *
	 */


	Bound(mode, deadline, nNodes, nCores, datasetSize, appId,  &(*R), &(*bound), &(*Rnew));


}
