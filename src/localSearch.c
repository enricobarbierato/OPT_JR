/*
 * localSearch.c
 *
 *  Created on: Jun 27, 2017
 *      Author: work
 */
#include <stdio.h>
#include <stdlib.h>
#include "localSearch.h"


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



char * Bound(int mode, int deadline, int nNodes, int nCores, int datasetSize, char *appId, int step)
{
	int bound = nCores;
	int time, R;
	int fake = 0;
	char * output = (char *)malloc(64);

	// mode is a Temporary variable: it says to FakeLjundstrom to take a value greater (9) or lower (1) than D

	//time = atoi(invokeLundstrom( nNodes, nCores, "8G", datasetSize, appId));
	time = atoi(fakeLundstrom(mode, fake++, nNodes, nCores, "8G", datasetSize, appId));step = 1;

//printf("R %d D %d\n", time, deadline);
	if (time > deadline)
	while (time > deadline)
	{
		R = time;
		nCores = nCores + step;
		//time = atoi(invokeLundstrom( nNodes, nCores, "8G", datasetSize, appId));
		time = atoi(fakeLundstrom(0, fake++, nNodes, nCores, "8G", datasetSize, appId));
		bound = nCores;

	}
	else
		while (time < deadline)
			{
				R = time;
				nCores = nCores - step;
				//time = atoi(invokeLundstrom( nNodes, nCores, "8G", datasetSize, appId));
				time = atoi(fakeLundstrom(1, fake++, nNodes, nCores, "8G", datasetSize, appId));
				bound = nCores;
			}

	//printf("deadline = %d Lundstrom %d upperBound %d\n", deadline, time, bound);
	sprintf(output, "%d %d", bound, R);
	return output;
}

int ObjFunctionComponent(double w, int R, int D)
{
	printf("Got: %f %d %d\n",w,R, D);
	return w * (R - D);
}


char * localSearch(int mode, MYSQL *conn, char *db, char * appId, int datasetSize, int deadline)
{
	char statement[256];
	char *output = (char *)malloc(64);
	int nCores;
	double R_n, R_nf;
	int bound;
	//int lowerBound;




/*
 * 		TEMPORARY FIX - NEEDS TO BE CHANGED!
 */
	int step = 20;
	int nNodes = 6;

	/* Retrieve the number of cores
	sprintf(statement,
			"select num_cores_opt from %s.OPTIMIZER_CONFIGURATION_TABLE where application_id='%s' and dataset_size=%d and deadline=%d;"
			, db, appId, datasetSize, deadline);*/

	//nCores = executeSQL(conn, statement);

	// TEST
	nCores =12;


	/* Calculate the bound */
	//lowerBound = Bound(deadline, nNodes, nCores, datasetSize, appId, step, 0);
	return Bound(mode, deadline, nNodes, nCores, datasetSize, appId, step);

}
