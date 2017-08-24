/*
 * localSearch.c
 *
 *  Created on: Jun 27, 2017
 *      Author: work
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "localSearch.h"


#define MAX_ITERATIONS 10
#define PREDICTOR DAGSIM

#define GLOBAL_PRINT YES



sAux * approximatedLoop(sList *first_i )
{
	int nCoreMov;
	double DELTAVM_i;
	double DELTAVM_j;
	double DELTA_fo_App_i, DELTA_fo_App_j;
	sAux * scurrentAuxApproximated = NULL;
	sAux * sfirstAuxApproximated = NULL;
	sList *application_i,  *application_j;

	/* Initialize to the first element */
	/* (possibly redundant) */
	application_i = first_i;

	printf("Approximated iterated loop\n");
		while (application_i != NULL)
		{
			application_j = first_i;
			while (application_j != NULL)
			{
				if (strcmp(application_i->app_id, application_j->app_id)!= 0)
				{
					printf("\n\nComparing %s with %s\n", application_i->app_id, application_j->app_id);
					printf("-----------------------------------------------\n");


					nCoreMov = max(application_i->V, application_j->V);

					DELTAVM_i = nCoreMov/application_i->V;printf("app %s DELTAVM_i %lf\n", application_i->app_id, DELTAVM_i);
					DELTAVM_j = nCoreMov/application_j->V;printf("app %s DELTAVM_j %lf\n", application_j->app_id, DELTAVM_j);

					/* Change the currentCores, but rollback later */
					printf("app %s currentCores %d\n", application_i->app_id, (int)application_i->currentCores_d);
					printf("app %s currentCores %d\n", application_j->app_id, (int)application_j->currentCores_d);

					int deltaNCores_i=DELTAVM_i*application_i->V;
					int deltaNCores_j=DELTAVM_j*application_j->V;
					application_i->currentCores_d = application_i->currentCores_d + deltaNCores_i;
					application_j->currentCores_d = application_j->currentCores_d - deltaNCores_j;

					printf("Dopo mossa: app %s currentCores %d\n", application_i->app_id, (int)application_i->currentCores_d);
					printf("Dopo mossa: app %s currentCores %d\n", application_j->app_id, (int)application_j->currentCores_d);


					if (application_i->currentCores_d > 0 && application_j->currentCores_d > 0)
					{
						/* Set up the algorithm for FO evaluation */
						//application_i->mode= R_ALGORITHM;application_j->mode= R_ALGORITHM;

						/*
						* Call object function evaluation
						*/

						DELTA_fo_App_i = //application_i->alpha/(application_i->currentCores_d)-application_i->alpha/(application_i->currentCores_d - deltaNCores_i);

								ObjFunctionComponentApprox(application_i) - application_i->baseFO;

						printf("app %s DELTA_fo_App_i %lf\n", application_i->app_id, DELTA_fo_App_i);


						DELTA_fo_App_j = //application_j->alpha/(application_j->currentCores_d)-application_j->alpha/(application_j->currentCores_d + deltaNCores_j);
								ObjFunctionComponentApprox(application_j) - application_j->baseFO;
						printf("app %s DELTA_fo_App_j %lf\n", application_j->app_id, DELTA_fo_App_j);

						//printf("App %s Delta FO Approx  %lf\n", application_i->app_id, DELTA_fo_App_i);
						//printf("App %s Delta FO Approx  %lf\n", application_j->app_id, DELTA_fo_App_j);
						// DANILO Store delta complessivo e numeri core in lista di appoggio -> ENRICO DONE


						if ((int)(DELTA_fo_App_i + DELTA_fo_App_j) < 0 )
							addAuxParameters(&sfirstAuxApproximated,
									&scurrentAuxApproximated,
									application_i,
									application_j,
									application_i->currentCores_d,
									application_j->currentCores_d,
									DELTA_fo_App_i + DELTA_fo_App_j,
									DELTAVM_i,
									DELTAVM_j
									);

						// DANILO ripristina numero di core precedenti -> ENRICO DONE
						application_i->currentCores_d = application_i->currentCores_d - DELTAVM_i*application_i->V;
						application_j->currentCores_d = application_j->currentCores_d + DELTAVM_j*application_j->V;
					}
				}
			application_j = application_j->next;
			}
		application_i = application_i->next;
		}

	return sfirstAuxApproximated;
}


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
	//char mvCmd[1024];
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
	/* Consider always the same
	 This is possible because the variance between the log folders is small*/
	strcpy(path, parseConfigurationFile("FAKE", 1));
	strcpy(dir, readFolder(path));

	//sprintf(mvCmd, "cd %s;mv %s %d_%d_%s_%d", path, dir, nNodes, currentCores, memory, datasize);

	//_run(mvCmd);

	switch(PREDICTOR)
	{
		case LUNDSTROM:
			sprintf(parameters, "%d %d %s %d %s", nNodes, currentCores, memory, datasize, appId);
			sprintf(cmd, "cd %s;python run.py %s", parseConfigurationFile("LUNDSTROM_HOME", 1), parameters);
			break;
		case DAGSIM:
			sprintf(path, "%s/1_X_8G_500/%s/logs", parseConfigurationFile("FAKE", 1), appId);

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
			 //printf("%s \n", cmd);
			sprintf(cmd, "cd %s;./dagsim.sh %s|head -n1|awk '{print $3;}'", parseConfigurationFile("DAGSIM_HOME", 1), lua);
			//printf("%s dagSim was executed on %d nodes (%d x %d) \n", cmd, (nNodes*currentCores), nNodes, currentCores);
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

void  Bound(sList * pointer)
{

	printf("Bound evaluation \n");
	int predictorOutput;

	int BTime = 0;
	int BCores = 0;
	int STEP = pointer->V;
	pointer->currentCores_d=pointer->nCores_DB_d;
/*
	int a=pointer->currentCores_d ;
	int b= pointer->V;
*/
	pointer->currentCores_d =( (int) ( pointer->currentCores_d / pointer->V) ) * pointer->V;

	double nCores = pointer->currentCores_d;
	int nNodes = 1; // Tenporary fix


	// Danilo 27/7/2017
	predictorOutput = atoi(invokePredictor( nNodes, nCores, "8G", pointer->datasetSize, pointer->app_id));
	pointer->sAB.index = 0;
	pointer->sAB.vec[pointer->sAB.index].nCores = nCores;
	pointer->sAB.vec[pointer->sAB.index].R = predictorOutput;
	pointer->sAB.index++;
	// End Danilo


	BTime = predictorOutput;
	//nCores = ceil(nCores / pointer->V ) * pointer->V;

	//printf("Calculate Bound for %s: R %d D %d\n", appId, predictorOutput, deadline);
	if (predictorOutput > pointer->Deadline_d)
	while (predictorOutput > pointer->Deadline_d)
	{

		//printf("(up) time = %d Rnew =%d\n", time, BTime);

		nCores = nCores + STEP;

		printf("Bound evaluation, appid %s, Step %d, evaluating %lf\n", pointer->app_id, STEP, nCores);
		predictorOutput = atoi(invokePredictor( nNodes, nCores, "8G", pointer->datasetSize, pointer->app_id));
		BCores = nCores;
		BTime = predictorOutput;

		// Danilo 27/7/2017
		pointer->sAB.vec[pointer->sAB.index].nCores = nCores;
		pointer->sAB.vec[pointer->sAB.index].R = predictorOutput;
		pointer->sAB.index = pointer->sAB.index % HYP_INTERPOLATION_POINTS;
		// End Danilo

	}
	else
		while (predictorOutput < pointer->Deadline_d)
		{
				BCores = nCores;
				BTime = predictorOutput;
				nCores = nCores - STEP;
			    printf("Bound evaluation, appid %s, evaluating %lf\n", pointer->app_id, nCores);

				if (nCores <= 0)
				{
					printf("nCOres is currently 0. Cannot invoke Predictor\n");
					exit(-1);
				}
				predictorOutput = atoi(invokePredictor( nNodes, nCores, "8G", pointer->datasetSize, pointer->app_id));

				pointer->sAB.vec[pointer->sAB.index].nCores = nCores;
				pointer->sAB.vec[pointer->sAB.index].R = predictorOutput;
				pointer->sAB.index = pointer->sAB.index % HYP_INTERPOLATION_POINTS;

				//printf("(down) time = %d Rnew =%d\n", time, BTime);
		}

	pointer->R_d = BTime;
	pointer->bound_d = BCores;
	printf("D = %lf R = %lf  bound = %lf\n", pointer->Deadline_d, pointer->R_d, pointer->bound_d);


}

float computeAlpha(sAlphaBetaManagement sab, float Beta)
{
	return sab.vec[1].nCores * (sab.vec[1].R - Beta);
}

float computeBeta(sAlphaBetaManagement sAB)
{
/*
	printf("R %lf nCores%d\n", sAB.vec[0].R, sAB.vec[0].nCores);
	printf("R %lf nCores%d\n", sAB.vec[1].R, sAB.vec[1].nCores);
	printf("%lf\n, ", 1.0 * sAB.vec[1].nCores / (sAB.vec[0].nCores - sAB.vec[1].nCores));
	printf("%lf\n", (1.0 * sAB.vec[0].nCores)/sAB.vec[1].nCores * sAB.vec[0].R - sAB.vec[1].R);
*/


	return ((double) sAB.vec[1].nCores) / (sAB.vec[0].nCores - sAB.vec[1].nCores) * (((double) sAB.vec[0].nCores)/sAB.vec[1].nCores * sAB.vec[0].R - sAB.vec[1].R);
}

int ObjFunctionGlobal(sList * pointer)
{
	int sum = 0;

	while (pointer != NULL)
	{
		sum = sum + ObjFunctionComponent(pointer);
		pointer = pointer->next;
	}

	if (sum == 0)
	{
		printf("Warning in ObjFunctionGlobal: sum equal to zero\n");
		//exit(-1);
	}

	return sum;
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


	pointer->R_d = atof(invokePredictor( 1, (int)pointer->currentCores_d, "8G", pointer->datasetSize, pointer->app_id));
	//printf("ObjFunctionComponent: App_id %s w %f R %d D %d nCores %d newCores %d\n",pointer->app_id, pointer->w, pointer->R, pointer->D, pointer->cores, pointer->newCores);

	/* Determine how the obj function needs to be calculated */
	switch(pointer->mode)
	{
		case R_ALGORITHM:
				printf("W %lf R_d %lf D %lf\n", pointer->w, pointer->R_d, pointer->Deadline_d);
				if (pointer->R_d > pointer->Deadline_d)
					output = pointer->w * (pointer->R_d - pointer->Deadline_d);
				else output = 0;
				printf("Compute FO for app %s currentCores_d %d  R %lf FO=%lf\n", pointer->app_id, (int)pointer->currentCores_d, pointer->R_d, output);
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


int ObjFunctionComponentApprox(sList * pointer)
{


	double output;

	if (pointer == NULL)
	{
		printf("ObjFunctionComponentApprox failure: NULL pointer\n");
		exit(-1);
	}


	pointer->R_d = pointer->alpha/pointer->currentCores_d + pointer->beta;  //atof(invokePredictor( 1, (int)pointer->currentCores_d, "8G", pointer->datasetSize, pointer->app_id));
	//printf("ObjFunctionComponent: App_id %s w %f R %d D %d nCores %d newCores %d\n",pointer->app_id, pointer->w, pointer->R, pointer->D, pointer->cores, pointer->newCores);

	/* Determine how the obj function needs to be calculated */

	printf("W %lf R_d %lf D %lf\n", pointer->w, pointer->R_d, pointer->Deadline_d);
	if (pointer->R_d > pointer->Deadline_d)
		output = pointer->w * (pointer->R_d - pointer->Deadline_d);
	else output = 0;
		printf("Compute FO for app %s currentCores_d %d  R %lf FO=%lf\n", pointer->app_id, (int)pointer->currentCores_d, pointer->R_d, output);

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
void findBound(MYSQL *conn, char *db,  sList *pointer)
{

	char statement[256];



	/* Retrieve nCores from the DB
	 *
	 **/
        sprintf(statement,
                        "select num_cores_opt from %s.OPTIMIZER_CONFIGURATION_TABLE where application_id='%s' and dataset_size=%d and deadline=%lf;"
                        , db, pointer->app_id, pointer->datasetSize, pointer->Deadline_d);

    pointer->nCores_DB_d = executeSQL(conn, statement);

    Bound(pointer);
    /*
	Bound(deadline, nNodes_d, pointer->nCores_d,
			pointer->datasetSize,
			pointer->app_id,
			&(pointer->R_bound_d),
			&(pointer->bound_d),
			pointer->V);
			*/

	pointer->mode = R_ALGORITHM;pointer->baseFO = ObjFunctionComponent(pointer);


}


/*
 * 		Name:					localSearch (WORK IN PROGRESS !)
 * 		Input parameters:		sList * application_i
 * 		Output parameters:		TBD
 * 		Description:			Localsearch algorithm as per functional analysis
 *
 */
void localSearch(sList * application_i, int n, int MAX_PROMISING_CONFIGURATIONS)
{
	sList * application_j, *first_i = application_i;
	sAux *firstAux = NULL, *currentAux = NULL;
	sAux *sfirstAuxApproximated = NULL;


	int nCoreMov;
	double DELTAVM_i, DELTAVM_j;
	double DELTA_fo_App_i, DELTA_fo_App_j;
	sAux * minAux;


	char *app_id_i, *app_id_j;

		app_id_i = (char *)malloc(1024);
		if (app_id_i == NULL)
		{
			printf("Malloc failure in invokePredictor\n");
			exit(-1);
		}
		app_id_j = (char *)malloc(1024);
		if (app_id_j == NULL)
		{
			printf("Malloc failure in invokePredictor\n");
			exit(-1);
		}

	printf("\n\nLocalsearch\n");
	if (GLOBAL_PRINT == YES) printf("Global obj function %d\n", ObjFunctionGlobal(first_i));



int index = 0;
for (int i = 1; i <= MAX_ITERATIONS; i++){
	printf("ITERATION %d\n", i);

	//sfirstAuxApproximated = approximatedLoop(first_i,  sfirstAuxApproximated);
	sfirstAuxApproximated = approximatedLoop(first_i  );

	printf("\n\n ************************************** Ex-iteration loop ***********************************************************************\n");

		while (sfirstAuxApproximated != NULL )
		{


			strcpy(app_id_i, sfirstAuxApproximated->app1->app_id);
			strcpy(app_id_j, sfirstAuxApproximated->app2->app_id);
			application_i = searchApplication(first_i, app_id_i);
			application_j = searchApplication(first_i, app_id_j);



			printf("\n\nComparing %s with %s\n", application_i->app_id, application_j->app_id);
			printf("-----------------------------------------------\n");


			nCoreMov = max(application_i->V, application_j->V);

			DELTAVM_i = nCoreMov/application_i->V;printf("app %s DELTAVM_i %lf\n", application_i->app_id, DELTAVM_i);
			DELTAVM_j = nCoreMov/application_j->V;printf("app %s DELTAVM_j %lf\n", application_j->app_id, DELTAVM_j);

			/* Change the currentCores, but rollback later */
			printf("app %s currentCores %d\n", application_i->app_id, (int)application_i->currentCores_d);
			printf("app %s currentCores %d\n", application_j->app_id, (int)application_j->currentCores_d);

			application_i->currentCores_d = application_i->currentCores_d + DELTAVM_i*application_i->V;
			application_j->currentCores_d = application_j->currentCores_d - DELTAVM_j*application_j->V;

			printf("Dopo mossa: app %s currentCores %d\n", application_i->app_id, (int)application_i->currentCores_d);
			printf("Dopo mossa: app %s currentCores %d\n", application_j->app_id, (int)application_j->currentCores_d);

			if (application_i->currentCores_d > 0 && application_j->currentCores_d > 0)
			{
				/* Set up the algorithm for FO evaluation */
				application_i->mode= R_ALGORITHM;application_j->mode= R_ALGORITHM;

				/*
				* Call object function evaluation
				*/

				DELTA_fo_App_i = ObjFunctionComponent(application_i) - application_i->baseFO; printf("app %s DELTA_fo_App_i %lf\n",
						application_i->app_id, DELTA_fo_App_i);
				DELTA_fo_App_j = ObjFunctionComponent(application_j) - application_j->baseFO;printf("app %s DELTA_fo_App_j %lf\n",
						application_j->app_id, DELTA_fo_App_j);


				// DANILO Store delta complessivo e numeri core in lista di appoggio -> ENRICO DONE

				if ((int)(DELTA_fo_App_i + DELTA_fo_App_j) < 0 )
					addAuxParameters(&firstAux,
							&currentAux,
							application_i,
							application_j,
							application_i->currentCores_d,
							application_j->currentCores_d,
							DELTA_fo_App_i + DELTA_fo_App_j,
							DELTAVM_i,
							DELTAVM_j
							);

				// DANILO ripristina numero di core precedenti -> ENRICO DONE
				application_i->currentCores_d = application_i->currentCores_d - DELTAVM_i*application_i->V;
				application_j->currentCores_d = application_j->currentCores_d + DELTAVM_j*application_j->V;
			}
			sfirstAuxApproximated = sfirstAuxApproximated->next;

		}

	readAuxList(firstAux);

	// DANILO accedo alla lista di appoggio cercando delta fo minore
	//TODO: valutare le prime n della lista firstAUX ordinata per valore delta totale FO (app_i+app_j)
	//minAux = findMinDelta(firstAux);

	//if (minAux == NULL)
	minAux = firstAux; // The list is now sorted, so the smallest element is the first;

	if ((firstAux ==NULL || index == MAX_PROMISING_CONFIGURATIONS))
	{
		printf("Auxiliary list empty or MAX_PROMISING_CONFIGURATIONS has been reached. Optimization terminated.\n");// Se non c'è minimo vuol dire che non c'è migliorante usciamo dal ciclo
		return;
	}
	index++;
	// DANILO effettuo assegnamento del numero di core alle applicazioni
	commitAssignment(first_i, minAux->app1->app_id, minAux->delta_i); // application i
	commitAssignment(first_i, minAux->app2->app_id, -minAux->delta_j); // application j

	if (GLOBAL_PRINT == YES) printf("Global obj function %d\n", ObjFunctionGlobal(first_i));

	// Modificare in modo da ricalcolare solo FO per le applicazioni i e j (usare le copie sopra, senza richiamare dagSim
	initialize(first_i);

	printf("Destroy Aux list\n");

	// DESTROY Auxiliary lists and prepare it for a new run

	freeAuxList(sfirstAuxApproximated);
	freeAuxList(firstAux);
	firstAux = NULL;
	currentAux = NULL;

	/* Prepare applications list for a new run */
	//application_i = first_i;

	// DANILO faccio somma di numero di core assegnati e confronto con N
/*
	if (!checkTotalCores(first_i, n))
	{
			printf("Total cores (new assignment) not equal to original N (%d)\n", n );

	}
	*/
	printf("****************************************************\n");
}

}




/*
 *
 */


void initialize(sList * application_i)
{
	while (application_i != NULL)
	{

			application_i->mode = R_ALGORITHM;
			application_i->baseFO = ObjFunctionComponent(application_i);
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
void calculate_Nu(MYSQL *conn, char * uniqueFilename, sList *first, int N)
{

	sList * current = first;
	int rows = 0;
	char * app_id;
	double w1;
	double chi_c_1;
	double csi_1;
	double index_d;
	double csi;


	app_id = (char *)malloc(MAX_APP_LENGTH);
	if (app_id == NULL)
	{
	          printf("app_id: malloc_failure in process\n");
	          exit(-1);
    }


	/* Calculate nu_1 */
	double tot = 0;
	while (current != NULL)
	{
		if (rows == 0) /* First row only */
		{
			w1 = current->w;
			chi_c_1 = current->chi_C;
			csi_1 = getCsi(current->M/current->m, current->V/current->v);
			//printf("first app: %s %lf %lf %lf\n", current->app_id, w1, chi_c_1, csi_1);
		}
		else /*Any other row */
		{
		       csi = getCsi(current->M/current->m, current->V/current->v);
		       tot = tot + sqrt((current->w/w1)*(current->chi_C/chi_c_1)*(csi_1/csi));
		       //printf("Other rows: %s %lf %lf %lf\n", current->app_id, current->w, csi, tot);
		}
		rows++;
		current = current->next;
	}

	double nu_1 = N/(1 + tot);
	//printf("nu_1=%lf\n", nu_1);

	rows = 0;

	while (first != NULL)
	{
	    csi = getCsi(first->M/first->m, first->V/first->v);

	    if (rows == 0) index_d = nu_1; else
	    /* Any other application than the first */
	        index_d = nu_1*sqrt((first->w/w1)*(first->chi_C/chi_c_1)*(csi_1/csi));
	    first->nu_d = index_d;
	    first->currentCores_d = index_d;

	    //printf("%lf %lf %lf %lf\n",current->w, index_d, nu_1, sqrt((current->w/w1)*(current->chi_C/chi_c_1)*(csi_1/csi)));


	        /* Update the db table */
	        //DBinsertrow(conn, uniqueFilename, app_id, indexF);

	        /*
	           Calculate
	           1) the bound (new number of cores)
	           2) the bound (time)
	        */

	     findBound(conn, parseConfigurationFile("OptDB_dbName", XML), first);

	        /* Compute alpha and beta for H Interpolation
	         *
	         */
	     first->beta = computeBeta(first->sAB);
	     first->alpha = computeAlpha(first->sAB, first->beta);

	        printf("App %s alpha = %lf beta = %lf\n", first->app_id, first->alpha, first->beta);
	        printf(" R %lf nCores %d\n", first->sAB.vec[0].R, first->sAB.vec[0].nCores);
	        printf(" R %lf nCores %d\n", first->sAB.vec[1].R, first->sAB.vec[1].nCores);

	        //current->currentCores_d = current->bound_d;

	        printRow(first);
	      first = first->next;
	      rows++;
	   }

}

