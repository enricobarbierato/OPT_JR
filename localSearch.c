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


#define MAX_ITERATIONS 100
#define PREDICTOR DAGSIM






/*
 * 		Name:					invokePredictor
 * 		Input parameters:		int nNodes, int nCores, char * memory, int datasize,  char *appId
 * 		Output parameters:		The output from Predictor predictor
 * 		Description:			This function works on one data log folder only, whose path can be found in wsi_config.xml file with the the keyword "FAKE".
 *
 * 								First, the data folder name is changed according to Nodes_Cores convention as for the input parameters,
 * 								the following lines build a command that executes Predictor tool.
 *
 */

char* invokePredictor(int nNodes, int currentCores, char * memory, int datasize,  char *appId)
{
	char parameters[1024];
	char cmd[1024];
	char mvCmd[1024];
	char path[1024];
	char lua[1024];
	char subfolder[1024];
	char *output = (char *)malloc(1024);
	/*
	 * Prepare the data
	 */

	char dir[1024];

	if (currentCores <= 0)
	{
		printf("Fatal Error: Cannot invoke predictor on negative  or zero number of cores: currentCores = %d\n", currentCores);
		exit(-1);
	}
	/* Prepare the folder by renaming the number of cores
	 This is possible because the variance between the log folders is small*/
	strcpy(path, parseConfigurationFile("FAKE", 1));
	strcpy(dir, readFolder(path));

	sprintf(mvCmd, "cd %s;mv %s %d_%d_%s_%d", path, dir, nNodes, currentCores, memory, datasize);

	_run(mvCmd);

	switch(PREDICTOR)
	{
		case LUNDSTROM:
			sprintf(parameters, "%d %d %s %d %s", nNodes, currentCores, memory, datasize, appId);
			sprintf(cmd, "cd %s;python run.py %s", parseConfigurationFile("LUNDSTROM_HOME", 1), parameters);
			break;
		case DAGSIM:
			sprintf(path, "%s/%d_%d_%s_%d/%s/logs", parseConfigurationFile("FAKE", 1), nNodes, currentCores, memory, datasize, appId);
			strcpy(subfolder, readFolder(path));
			sprintf(cmd, "ls %s/%s/*.lua", path, subfolder);
			strcpy(lua, _run(cmd));
			/* Remove /n from the lua filename */

			lua[strlen(lua)-1] = '\0';


			/* Update the number of nodes in the lua file
			 * calculated as nNodes * nCores
			 * line=$(cat $LUA$LUA_FILENAME|grep "Nodes = ")
        		sed -i "s/$line/Nodes = @@nodes@@;/g" $LUA$LUA_FILENAME
			 *
			 */

			sprintf(cmd, "line=$(cat %s|grep \"Nodes = \");sed -i \"s/$line/Nodes = %d;/g\" %s", lua, (nNodes*currentCores), lua);
			 _run(cmd);

			sprintf(cmd, "cd %s;./dagsim.sh %s|head -n1|awk '{print $3;}'", parseConfigurationFile("DAGSIM_HOME", 1), lua);

			break;
	}



	strcpy(output, _run(cmd));
	//printf("Cores: %d %s\n", nCores, output);

	return output;
}





/*
 * 		Name:				Bound
 * 		Input parameters:	int deadline, int nNodes, int nCores, int datasetSize, char *appId,
 * 		Output parameters:	int *R (initial Predictor estimate for the given cores, the bound (number of cores), and the time for the determined bound.
 * 		Description:		This function calculates the bound given a certain deadline and number of nodes, cores. Predictor method is invoked until an upper bound,
 * 							consisting of the number of nodes, is found (once that the time calculated by the predictor, a rollback is performed to
 * 							return the last "safe" number of core and time.
 *
 */
void  Bound(int deadline, int nNodes, int nCores, int datasetSize, char *appId, double *R, double *bound, int STEP)
{

	int predictorOutput;

	int BTime = 0;
	int BCores = 0;



	predictorOutput = atoi(invokePredictor( nNodes, nCores, "8G", datasetSize, appId));

	BTime = predictorOutput;

	//printf("Calculate Bound for %s: R %d D %d\n", appId, predictorOutput, deadline);
	if (predictorOutput > deadline)
	while (predictorOutput > deadline)
	{
		BCores = nCores;
		BTime = predictorOutput;
		//printf("(up) time = %d Rnew =%d\n", time, BTime);
		nCores = nCores + STEP;

		predictorOutput = atoi(invokePredictor( nNodes, nCores, "8G", datasetSize, appId));

	}
	else
		while (predictorOutput < deadline)
		{
				BCores = nCores;
				BTime = predictorOutput;
				nCores = nCores - STEP;
				if (nCores <= 0)
				{
					printf("nCOres is currently 0. Cannot invoke Predictor\n");
					exit(-1);
				}
				predictorOutput = atoi(invokePredictor( nNodes, nCores, "8G", datasetSize, appId));


				//printf("(down) time = %d Rnew =%d\n", time, BTime);
		}

	*R = BTime;
	*bound = BCores;
	printf("D = %d R = %lf  bound = %lf\n", deadline, *R, *bound);


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

				pointer->R_d = atof(invokePredictor( 1, (int)pointer->currentCores_d, "8G", pointer->datasetSize, pointer->app_id));
				printf("app %s currentCores_d %d  R %lf\n", pointer->app_id, (int)pointer->currentCores_d, pointer->R_d);
				if (pointer->R_d > pointer->Deadline_d)
					output = pointer->w * (pointer->R_d - pointer->Deadline_d);
				else output = 0;
			break;
			/*
		case CORES_ALGORITHM:
			printf("Cores Algorithm\n");
				if (pointer->currentCores > pointer->newCores) output = 0;
				else output = pointer->w * (pointer->Rnew - pointer->D);

			break;
		case NCORES_ALGORITHM:
			printf("NCores Algorithm\n");
				if (pointer->newCores >pointer->bound) output = 0;
				else output = pointer->w * pointer->R - pointer->R;
			break;
			*/
		default:
			printf("ObjFunctionComponent: unknown case within Switch statement: mode %d\n", pointer->mode);
			exit(-1);
			break;
	}



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
void findBound(MYSQL *conn, char *db,   int deadline, sList *pointer)
{

	char statement[256];
	int nNodes_d = 1; // Temporary value


	/* Retrieve nCores from the DB
	 *
	 **/
        sprintf(statement,
                        "select num_cores_opt from %s.OPTIMIZER_CONFIGURATION_TABLE where application_id='%s' and dataset_size=%d and deadline=%d;"
                        , db, pointer->app_id, pointer->datasetSize, deadline);

        pointer->nCores_d = executeSQL(conn, statement);

	Bound(deadline, nNodes_d, pointer->nCores_d,
			pointer->datasetSize,
			pointer->app_id,
			&(pointer->R_d),
			&(pointer->bound_d),
			pointer->V);



}


/*
 * 		Name:					localSearch (WORK IN PROGRESS !)
 * 		Input parameters:		sList * application_i
 * 		Output parameters:		TBD
 * 		Description:			Localsearch algorithm as per functional analysis
 *
 */
void localSearch(sList * application_i, int n)
{
	sList * application_j, *first_i = application_i;
	sAux *firstAux = NULL, *currentAux = NULL;
	int nCoreMov;
	double DELTA_i, DELTA_j;
	double DELTA_fo_App_i, DELTA_fo_App_j;
	sAux * minAux;
	double prev_d;
	double previousFO_i, previousFO_j;


	printf("\n\nLocalsearch\n");


for (int i = 1; i <= MAX_ITERATIONS; i++){
	while (application_i != NULL)
	{
		previousFO_i = 0;
		previousFO_j = 0;
		application_j = first_i;
		while (application_j != NULL)
		{
			if (strcmp(application_i->app_id, application_j->app_id)!= 0)
			{
				printf("Comparing %s with %s\n", application_i->app_id, application_j->app_id);
				printf("-----------------------------------------------\n");


				nCoreMov = max(application_i->V, application_j->V);

				DELTA_i = nCoreMov/application_i->V;printf("app %s DELTA_i %lf\n", application_i->app_id, DELTA_i);
				DELTA_j = nCoreMov/application_j->V;printf("app %s DELTA_j %lf\n", application_j->app_id, DELTA_j);

				/* Change the currentCores, but rollback later */
				printf("app %s currentCores %d\n", application_i->app_id, (int)application_i->currentCores_d);
				printf("app %s currentCores %d\n", application_j->app_id, (int)application_j->currentCores_d);
				application_i->currentCores_d = application_i->currentCores_d + DELTA_i*application_i->V;
				application_j->currentCores_d = application_j->currentCores_d - DELTA_j*application_j->V;
				if (	(int)application_i->currentCores_d - DELTA_i*application_i->V  > 0 &&
						(int)application_j->currentCores_d - DELTA_j*application_j->V  > 0)
				{
					printf("Dopo mossa: app %s currentCores %d\n", application_i->app_id, (int)application_i->currentCores_d);
					printf("Dopo mossa: app %s currentCores %d\n", application_j->app_id, (int)application_j->currentCores_d);

					/* Set up the algorithm for FO evaluation */
					application_i->mode= R_ALGORITHM;application_j->mode= R_ALGORITHM;

					/*
					 * Call object function evaluation
					 */

					DELTA_fo_App_i = previousFO_i - ObjFunctionComponent(application_i);
					DELTA_fo_App_j = previousFO_j - ObjFunctionComponent(application_j);

					// DANILO Store delta complessivo e numeri core in lista di appoggio -> ENRICO DONE

					if ((int)(DELTA_fo_App_i + DELTA_fo_App_j) < 0 )
						addAuxParameters(&firstAux,
								&currentAux,
								application_i->app_id,
								application_j->app_id,
								application_i->currentCores_d,
								application_j->currentCores_d,
								DELTA_fo_App_i + DELTA_fo_App_j,
								DELTA_i,
								DELTA_j
								);

					// DANILO ripristina numero di core precedenti -> ENRICO DONE
					application_i->currentCores_d = application_i->currentCores_d - DELTA_i*application_i->V;
					application_j->currentCores_d = application_j->currentCores_d + DELTA_j*application_j->V;
				}
				}
				application_j = application_j->next;
			}

			application_i = application_i->next;
		}

		readAuxList(firstAux);

		// DANILO accedo alla lista di appoggio cercando delta fo minore
	minAux = findMinDelta(firstAux);
	if (minAux == NULL)
	{
		printf("Auxiliari list empty. Optimization terminated.\n");// Se non c'è minimo vuol dire che non c'è migliorante usciamo dal ciclo
		return;
	}
	printf("FO complessivo alla iterazione %d = %lf\n", i, minAux->deltaFO);

	// DANILO effettuo assegnamento del numero di core alle applicazioni
	commitAssignment(first_i, minAux->app1, minAux->delta_i); // application i
	commitAssignment(first_i, minAux->app2, -minAux->delta_j); // application j





	printf("Destroy Aux list\n");
	// DESTROY Auxiliary list and prepare it for a new run
	freeAuxList(firstAux);
	firstAux = NULL;
	currentAux = NULL;

	/* Prepare applications list for a new run */
	application_i = first_i;

	// DANILO faccio somma di numero di core assegnati e confronto con N

		if (!checkTotalCores(first_i, n))
		{
				printf("Total cores (new assignment) not equal to original N (%d)\n", n );

		}
	/*
		 * Check if the improvement was significant or not
		 */


		prev_d = minAux->deltaFO;
		printf("****************************************************\n");
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
	double Deadline_d;
	int rows = 1;
	char * app_id;
	double w;

	double index_d;
	double csi;


	app_id = (char *)malloc(MAX_APP_LENGTH);
	if (app_id == NULL)
	{
	          printf("app_id: malloc_failure in process\n");
	          exit(-1);
    }



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
	        Deadline_d = 	current->Deadline_d;

	        csi = getCsi(M/m, V/v);

	        if (rows == 1) index_d = nu_1; else index_d = nu_1*sqrt((w/w1)*(chi_C/chi_c_1)*(csi_1/csi));
	        current->nu_d = index_d;




	        /* Update the db table */
	        //DBinsertrow(conn, uniqueFilename, app_id, indexF);

	        /*
	           Calculate
	           1) the bound (new number of cores)
	           2) the bound (time)
	        */

	        findBound(conn, parseConfigurationFile("OptDB_dbName", XML), Deadline_d, current);
	        current->currentCores_d = index_d;
	        //current->currentCores_d = current->bound_d;

	        printRow(current);
	        current = current->next;
	        rows++;
	     }
}

