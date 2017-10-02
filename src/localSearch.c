/*
 * localSearch.c *
 *  Created on: Jun 27, 2017
 *      Author: work
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "localSearch.h"
#include "db.h"


#define GLOBAL_PRINT YES


sListPointers * fixInitialSolution(sList *applications,  struct optJrParameters par)
{
	sList * first;
	int allocatedCores;
	sListPointers * first_LP = NULL;
	int loopExit = 0;
	sListPointers *auxPointer;
	int residualCores;
	char debugMsg[DEBUG_MSG];
	int N = par.number;

	allocatedCores = 0; // TODO To be changed into INT ->DONE

	first = applications;

	while (first != NULL)
	{
		//int currentcores1 =first->currentCores_d;
		//double currentcores2=max(((int)(first->currentCores_d / first->V)) * first->V,first->V);


		first->currentCores_d = max(((int)(first->currentCores_d / first->V)) * first->V,first->V);
		if (first->currentCores_d > first->bound)
			first->currentCores_d = first->bound;
		else
			{
				sprintf(debugMsg, "adding %s to ListPointers\n", first->session_app_id);debugMessage(debugMsg, par);
				addListPointers(&first_LP, first);
			}

		// Danilo Application (suffering) insert in the new list
		// TODO Handle insert in such a way the list is sorted by weight -> DONE

		allocatedCores+= first->currentCores_d;
		sprintf(debugMsg, "fixInitialSolution FIXING CORES %s %d\n", first->session_app_id, first->currentCores_d);debugMessage(debugMsg, par);
		first = first->next;
	}
	//readListPointers(first_LP);

	sprintf(debugMsg,"fixInitialSolution: allocatedCores %d\n", allocatedCores);debugMessage(debugMsg, par);

	auxPointer = first_LP;



	residualCores = N - allocatedCores;
	int addedCores;


	while (!loopExit&& (residualCores>0))
	{

		if (auxPointer == NULL) loopExit = 1;
		else
		{
			// cores assignment

			int potentialDeltaCores=((int)(residualCores / auxPointer->app->V) )* auxPointer->app->V;

			//addedCores = MIN(, auxPointer->app->bound_d);

			if ((auxPointer->app->currentCores_d + potentialDeltaCores) > auxPointer->app->bound){
				addedCores = auxPointer->app->bound - auxPointer->app->currentCores_d ;
				auxPointer->app->currentCores_d = auxPointer->app->bound;


			}
			else{
				auxPointer->app->currentCores_d = auxPointer->app->currentCores_d + potentialDeltaCores;
				addedCores=potentialDeltaCores;
			}

			if (auxPointer->app->currentCores_d == 0)
			{
				printf("\nFatal Error: FixInitialSolution: app %s has %d cores after fix\n", auxPointer->app->session_app_id, auxPointer->app->currentCores_d);
				exit(-1);
			}
			if (addedCores > 0)
			{
				//auxPointer->app->currentCores_d+= addedCores;

				sprintf(debugMsg,"adding cores to App %s, %d \n", auxPointer->app->session_app_id, addedCores);debugMessage(debugMsg, par);

				sprintf(debugMsg," application_id %s new cores %d moved cores %d\n", auxPointer->app->session_app_id, (int)auxPointer->app->currentCores_d, addedCores);debugMessage(debugMsg, par);

				residualCores = residualCores - addedCores;
			}
			auxPointer = auxPointer->next;
		}

		if (residualCores == 0) loopExit = 1;
	}
	readList(applications, par);

	return first_LP;
}


sAux * approximatedLoop(sList *first_i, int *iteration, struct optJrParameters par )
{
	char debugMsg[DEBUG_MSG];

	if (first_i == NULL)
	{
		printf("Error: approximatedLoop: null pointer\n");
		exit(-1);
	}

	int record = 0;
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

	sprintf(debugMsg, "Approximated iterated loop\n");debugInformational(debugMsg, par);
		while (application_i != NULL)
		{
			application_j = first_i;
			while (application_j != NULL)
			{
				if (strcmp(application_i->session_app_id, application_j->session_app_id)!= 0)
				{
					sprintf(debugMsg,"\n\nComparing %s with %s\n", application_i->session_app_id, application_j->session_app_id);debugMessage(debugMsg, par);
					sprintf(debugMsg, " ");debugBanner(debugMsg, par);


					nCoreMov = max(application_i->V, application_j->V);

					DELTAVM_i = nCoreMov/application_i->V;
					sprintf(debugMsg, "app %s DELTAVM_i %lf\n", application_i->session_app_id, DELTAVM_i);debugMessage(debugMsg, par);
					DELTAVM_j = nCoreMov/application_j->V;
					sprintf(debugMsg, "app %s DELTAVM_j %lf\n", application_j->session_app_id, DELTAVM_j);debugMessage(debugMsg, par);

					/* Change the currentCores, but rollback later */
					sprintf(debugMsg,"\n app %s currentCores %d\n", application_i->session_app_id, (int)application_i->currentCores_d);debugMessage(debugMsg, par);
					sprintf(debugMsg,"app %s currentCores %d\n", application_j->session_app_id, (int)application_j->currentCores_d);debugMessage(debugMsg, par);

					int deltaNCores_i=DELTAVM_i*application_i->V;
					int deltaNCores_j=DELTAVM_j*application_j->V;
					application_i->currentCores_d = application_i->currentCores_d + deltaNCores_i;
					application_j->currentCores_d = application_j->currentCores_d - deltaNCores_j;

					sprintf(debugMsg,"\n   After cores exchange: app %s currentCores %d\n", application_i->session_app_id, (int)application_i->currentCores_d);debugMessage(debugMsg, par);
					sprintf(debugMsg,"   After cores exchange: app %s currentCores %d\n", application_j->session_app_id, (int)application_j->currentCores_d);debugMessage(debugMsg, par);


					if (application_i->currentCores_d > 0 && application_j->currentCores_d > 0)
					{

						/* Set up the algorithm for FO evaluation */
						//application_i->mode= R_ALGORITHM;application_j->mode= R_ALGORITHM;

						/*
						* Call object function evaluation
						*/

						DELTA_fo_App_i = //application_i->alpha/(application_i->currentCores_d)-application_i->alpha/(application_i->currentCores_d - deltaNCores_i);

								ObjFunctionComponentApprox(application_i, par) - application_i->baseFO;

						sprintf(debugMsg,"\napp %s DELTA_fo_App_i %lf\n", application_i->session_app_id, DELTA_fo_App_i);debugMessage(debugMsg, par);


						DELTA_fo_App_j = //application_j->alpha/(application_j->currentCores_d)-application_j->alpha/(application_j->currentCores_d + deltaNCores_j);
								ObjFunctionComponentApprox(application_j, par) - application_j->baseFO;
						sprintf(debugMsg,"\n app %s DELTA_fo_App_j %lf\n", application_j->session_app_id, DELTA_fo_App_j);debugMessage(debugMsg, par);

						//printf("App %s Delta FO Approx  %lf\n", application_i->app_id, DELTA_fo_App_i);
						//printf("App %s Delta FO Approx  %lf\n", application_j->app_id, DELTA_fo_App_j);
						// TODO DANILO Store global delta complessivo and number of cores into auxiloiary list -> ENRICO DONE


						//if ((DELTA_fo_App_i + DELTA_fo_App_j < 0) ||(DELTA_fo_App_i + DELTA_fo_App_j >= 0))
						if ((DELTA_fo_App_i + DELTA_fo_App_j < 0))
						{
							//printf("LOCALSEARCH adding negative DELTA (%lf)\n", DELTA_fo_App_i + DELTA_fo_App_j);
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
							/* Update the number of insertions */
							++record;
						}


					}
					// TODO DANILO Restore previous number of cores -> ENRICO DONE
					application_i->currentCores_d = application_i->currentCores_d - DELTAVM_i*application_i->V;
					application_j->currentCores_d = application_j->currentCores_d + DELTAVM_j*application_j->V;
				}
			application_j = application_j->next;
			}
		application_i = application_i->next;
		}

	*iteration = record;
	return sfirstAuxApproximated;
}



char* invokePredictor(sConfiguration * configuration, MYSQL *conn, int nNodes, int currentCores, char * memory, int datasize,  char *sessionId, char *appId, char *stage, struct optJrParameters par, int flagDagsim)
{
	char parameters[1024];
	char cmd[1024];
	char path[1024];
	char lua[1024];
	char subfolder[1024];
	char *output1 = (char *)malloc(64);
	char statement[1024];
	char debugMsg[DEBUG_MSG];
	char dbName[64];
	char dir[1024];


	if (output1 == NULL)
	{
		printf("Malloc failure: invokePredictor: output1\n");
		exit(-1);
	}



	sprintf(debugMsg, "invokePredictor\n");debugInformational(debugMsg, par);

	/* Consider always the same folder and lua file (replacing the number of nodes)
	 This is possible because the variance between the log folders is small*/

	strcpy(path, getConfigurationValue(configuration, "RESULTS_HOME"));
	strcpy(dir, readFolder(path));

	//sprintf(mvCmd, "cd %s;mv %s %d_%d_%s_%d", path, dir, nNodes, currentCores, memory, datasize);

	//_run(mvCmd);

	switch(par.simulator)
	{
		case LUNDSTROM:
			sprintf(parameters, "%d %d %s %d %s", nNodes, currentCores, memory, datasize, appId);
			sprintf(cmd, "cd %s;python run.py %s", getConfigurationValue(configuration, "LUNDSTROM_HOME"), parameters);
			break;
		case DAGSIM:

			/* Check if there is an output from dagSim already cached in the DB */


			/*sprintf(statement, "select value from %s.PREDICTOR_CACHE_TABLE where session_application_id = '%s' and "
											"application_id=\'%s\' and "
											"dataset_size=%d and phi_mem=\'8G\' and num_cores_opt = %d;",
											getConfigurationValue(configuration,"OptDB_dbName"),
											sessionId,
											appId,
											datasize,
											currentCores);*/


			strcpy(dbName, getConfigurationValue(configuration,"OptDB_dbName"));
			sprintf(statement, "select %s.PREDICTOR_CACHE_TABLE.val \nfrom %s.PREDICTOR_CACHE_TABLE,"
					"%s.APPLICATION_PROFILE_TABLE \nwhere %s.PREDICTOR_CACHE_TABLE.is_residual = %d and "
							"%s.PREDICTOR_CACHE_TABLE.dataset_size = "
					"%s.APPLICATION_PROFILE_TABLE.dataset_size and\n 	"
					"%s.PREDICTOR_CACHE_TABLE.application_id=\'%s\' and %s.PREDICTOR_CACHE_TABLE.dataset_size=%d  "
					"and %s.PREDICTOR_CACHE_TABLE.num_cores = %d;\n",
					dbName,
					dbName,
					dbName,
					dbName,
					flagDagsim,
					dbName,
					dbName,
					dbName,appId, dbName,datasize, dbName,currentCores);


			MYSQL_ROW row = executeSQL(conn, statement, par);

			if (row == NULL)
			{
				sprintf(debugMsg, "Last SQL statement returned 0 rows. Invoking predictor...\n");
				debugMessage(debugMsg, par);

				/* Replaced FAKE variable with RESULTS_HOME */
				//sprintf(path, "%s/%s/logs", getConfigurationValue(configuration, "RESULTS_HOME"), appId);
				sprintf(path, "%s/%s/%s/logs", getConfigurationValue(configuration, "RESULTS_HOME"), readFolder(path), appId);


				strcpy(subfolder, readFolder(path));
				sprintf(cmd, "%s/%s/", path, subfolder);
				sprintf(cmd, "%s*.lua", cmd);
				strcpy(lua, ls(cmd, par));


				char pattern[64];

				sprintf(pattern, "Nodes = %d",(nNodes*currentCores));
				//writeFile(lua, replace(readFile(lua), pattern));
				writeFile("/tmp/temp.lua", replace(readFile(lua), pattern));

				double systemTime;
				double stageTime;

				/* Using /tmp/temp.lua instead of lua variable, so the original file is never overwritten */
				sprintf(cmd, "cd %s;./dagsim.sh %s -s > /tmp/outputDagsim.txt", getConfigurationValue(configuration, "DAGSIM_HOME"), "/tmp/temp.lua");
				sprintf(debugMsg, "Executing predictor: %s\n", cmd);debugMessage(debugMsg, par);
				_run(cmd, par);
				switch(flagDagsim)
				{
					case RESIDUAL_DAGSIM:
						systemTime = atof(extractWord(extractRowN(readFile("/tmp/outputDagsim.txt"),3), 3));
						stageTime = atof(extractWord(extractRowN(extractRowMatchingPattern(readFile("/tmp/outputDagsim.txt"), stage),1), 4));
						sprintf(output1, "%lf", (systemTime - stageTime));
						sprintf(debugMsg, "Residual time: %s\n", output1);debugMessage(debugMsg, par);
						break;
					case WHOLE_DAGSIM:
						strcpy(output1, extractWord(extractRowN(readFile("/tmp/outputDagsim.txt"),1),3));
						break;
					default:
						printf("FATAL ERROR: invokePredictor: unknown case %d\n", flagDagsim);
						exit(-1);
						break;
				}


				if (doubleCompare(atof(output1), 0) == 0)
				{
					printf("Fatal Error: invokePredictor: dagSim output was zero (%s)\n", lua);
					exit(-1);
				}


				/* Update the db cash table with a new value */
				char statement[1024];

				sprintf(statement,"insert %s.PREDICTOR_CACHE_TABLE values('%s', %d, %d, '%s', %d, %lf);",
												getConfigurationValue(configuration, "OptDB_dbName"),
												appId,
												datasize,
												nNodes*currentCores,
												stage,
												flagDagsim,
												atof(output1)
												);

				if (mysql_query(conn, statement))
				{
					char error[512];
					sprintf(error, " %s", statement);
					DBerror(conn, error);
				}

			}
			else
				{
					double out = atof(row[0]);
					sprintf(debugMsg,"Dagsim output retrieved from DB cash: %s %lf\n", appId, out);debugMessage(debugMsg, par);
					sprintf(output1, "%lf", out);
				}

		}




	//printf("Cores: %d %s\n", nCores, output);

	return output1;
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

void  Bound(sConfiguration *configuration, MYSQL *conn, sList * pointer, struct optJrParameters par, int flagDagsim)
{


	double predictorOutput;
	char debugMsg[DEBUG_MSG];
	double BTime = 0;
	int BCores = 0;
	int STEP = pointer->V;
	int nCores;
	int nNodes = 1; // Temporary fix

	pointer->currentCores_d = pointer->nCores_DB_d;
/*
	int a=pointer->currentCores_d ;
	int b= pointer->V;
*/

	//To Do Call X0 this expression -> DONE
	// To Do max (X0, pointer->V)  ->DONE
	//pointer->currentCores_d = ( (int) ( pointer->currentCores_d / pointer->V) ) * pointer->V;
	int X0 = ((int) ( pointer->currentCores_d / pointer->V) ) * pointer->V;
	pointer->currentCores_d = max( X0, pointer->V);
	//if (pointer->currentCores_d > 0 && pointer->currentCores_d != pointer->V)//Togliere condizione
//	{
		nCores = pointer->currentCores_d;


		predictorOutput = atoi(invokePredictor(configuration, conn, nNodes, nCores, "*", pointer->datasetSize, pointer->session_app_id,
							pointer->app_id, pointer->stage, par, flagDagsim));
		sprintf(debugMsg,"Bound evaluation for %s predictorOutput = %lf (deadline is %lf) cores %d\n",  pointer->session_app_id, predictorOutput, pointer->Deadline_d, nCores);debugMessage(debugMsg, par);
		// Danilo 27/7/2017
		pointer->sAB.index = 0;
		pointer->sAB.vec[pointer->sAB.index].nCores = nCores;
		pointer->sAB.vec[pointer->sAB.index].R = predictorOutput;
		pointer->sAB.index++;
		// End Danilo


		BTime = predictorOutput;
		//To do nCores = ceil(nCores / pointer->V ) * pointer->V;

		//printf("Calculate Bound for %s: R %d D %d\n", appId, predictorOutput, deadline);
		if (doubleCompare(predictorOutput, pointer->Deadline_d) == 1)
			while (predictorOutput > pointer->Deadline_d)
			{
				if (nCores ==0)
				{
					printf("Warning Bound (| if case): nCores is currently 0 for app. Cannot invoke Predictor\n");
					nCores= pointer->V;
					//leave the while loop
					break;
				}
				//printf("(up) time = %d Rnew =%d\n", time, BTime);

				nCores = nCores + STEP;
				predictorOutput = atof(invokePredictor(configuration, conn, nNodes, nCores, "8G", pointer->datasetSize, pointer->session_app_id, pointer->app_id, pointer->stage,par, WHOLE_DAGSIM));
				sprintf(debugMsg,"Bound evaluation for %s predictorOutput = %lf (deadline is %lf) cores %d\n",  pointer->session_app_id, predictorOutput,pointer->Deadline_d, nCores);debugMessage(debugMsg, par);

				BCores = nCores;
				BTime = predictorOutput;

				// Danilo 27/7/2017
				pointer->sAB.vec[pointer->sAB.index].nCores = nCores;
				pointer->sAB.vec[pointer->sAB.index].R = predictorOutput;
				pointer->sAB.index = (pointer->sAB.index +1) % HYP_INTERPOLATION_POINTS;



				// End Danilo

				pointer->boundIterations++;

			}
		else
			while (doubleCompare(predictorOutput, pointer->Deadline_d) == -1)
			{
				BCores = nCores;
				BTime = predictorOutput;
				nCores = nCores - STEP;



				if (nCores <0)
				{
					printf("nCores is currently 0. Cannot invoke Predictor\n");
					exit(-1);
				}

				if (nCores ==0)
				{
					//printf("Warning Bound (< if case): nCores is currently 0 for app. Cannot invoke Predictor\n");
					nCores= pointer->V;
					//leave the while loop
					break;
				}
				predictorOutput = atof(invokePredictor(configuration, conn, nNodes, nCores, "8G", pointer->datasetSize, pointer->session_app_id, pointer->app_id, pointer->stage, par, WHOLE_DAGSIM));
				sprintf(debugMsg,"Bound evaluation for %s predictorOutput = %lf (deadline is %lf) cores %d\n",  pointer->session_app_id, predictorOutput, pointer->Deadline_d, nCores);debugMessage(debugMsg, par);

				pointer->sAB.vec[pointer->sAB.index].nCores = nCores;
				pointer->sAB.vec[pointer->sAB.index].R = predictorOutput;
				pointer->sAB.index = pointer->sAB.index % HYP_INTERPOLATION_POINTS;

				//printf("(down) time = %d Rnew =%d\n", time, BTime);
				pointer->boundIterations++;
			}
	//}
	/* Update the record with bound values */

	pointer->currentCores_d = BCores;
	pointer->R_d = BTime;
	pointer->bound = BCores;
	sprintf(debugMsg,"\n\nSession_app_id %s APP_ID %s D = %lf R = %lf  bound = %d\n\n", pointer->session_app_id, pointer->session_app_id, pointer->Deadline_d, pointer->R_d, pointer->bound);debugMessage(debugMsg, par);


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
	printf("%lf\n, ".0 * sAB.vec[1].nCores / (sAB.vec[0].nCores - sAB.vec[1].nCores));
	printf("%lf\n", (1.0 * sAB.vec[0].nCores)/sAB.vec[1].nCores * sAB.vec[0].R - sAB.vec[1].R);
*/


	return ((double) sAB.vec[1].nCores) / (sAB.vec[0].nCores - sAB.vec[1].nCores) * (((double) sAB.vec[0].nCores)/sAB.vec[1].nCores * sAB.vec[0].R - sAB.vec[1].R);
}

double ObjFunctionGlobal(sConfiguration * configuration, MYSQL *conn, sList * pointer, struct optJrParameters par)
{
	char debugMsg[DEBUG_MSG];
	double sum = 0;

	if (pointer == NULL)
	{
		printf("Warning: NULL pointer in ObjFunctionGlobal\n");
	}
	while (pointer != NULL)
	{
		sum = sum + ObjFunctionComponent(configuration, conn, pointer, par);
		pointer = pointer->next;
	}

	if (doubleCompare(sum, 0) == 0)
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
 * 									of ethe application structure.
 *
 */

int ObjFunctionComponent(sConfiguration *configuration, MYSQL *conn, sList * pointer, struct optJrParameters par)
{

	char debugMsg[DEBUG_MSG];
	double output;

	if (pointer == NULL)
	{
		printf("ObjFunctionComponent failure: NULL pointer\n");
		exit(-1);
	}


	pointer->R_d = atof(invokePredictor( configuration, conn, 1, pointer->currentCores_d, "8G", pointer->datasetSize, pointer->session_app_id, pointer->app_id, pointer->stage, par, RESIDUAL_DAGSIM));
	//printf("ObjFunctionComponent: App_id %s w %f R %d D %d nCores %d newCores %d\n",pointer->app_id, pointer->w, pointer->R, pointer->D, pointer->cores, pointer->newCores);

	/* Determine how the obj function needs to be calculated */
	switch(pointer->mode)
	{
		case R_ALGORITHM:
				sprintf(debugMsg,"ObjFunctionComponent W %d R_d %lf D %lf\n", pointer->w, pointer->R_d, pointer->Deadline_d);debugMessage(debugMsg, par);
				if (pointer->R_d > pointer->Deadline_d)
					output = pointer->w * (pointer->R_d - pointer->Deadline_d);
				else output = 0;
				sprintf(debugMsg,"Compute FO for app %s currentCores_d %d  R %lf FO=%lf\n", pointer->session_app_id, (int)pointer->currentCores_d, pointer->R_d, output);debugMessage(debugMsg, par);
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


int ObjFunctionComponentApprox(sList * pointer, struct optJrParameters par)
{

	char debugMsg[DEBUG_MSG];
	double output;

	if (pointer == NULL)
	{
		printf("ObjFunctionComponentApprox failure: NULL pointer\n");
		exit(-1);
	}


	pointer->R_d = pointer->alpha/pointer->currentCores_d + pointer->beta;  //atof(invokePredictor( 1, (int)pointer->currentCores_d, "8G", pointer->datasetSize, pointer->app_id));
	//printf("ObjFunctionComponent: App_id %s w %f R %d D %d nCores %d newCores %d\n",pointer->app_id, pointer->w, pointer->R, pointer->D, pointer->cores, pointer->newCores);

	/* Determine how the obj function needs to be calculated */

	sprintf(debugMsg,"W %d R_d %lf D %lf\n", pointer->w, pointer->R_d, pointer->Deadline_d);debugMessage(debugMsg, par);
	if (pointer->R_d > pointer->Deadline_d)
		output = pointer->w * (pointer->R_d - pointer->Deadline_d);
	else output = 0;
		sprintf(debugMsg,"Compute FO for app %s currentCores_d %d  R %lf FO=%lf\n", pointer->session_app_id, (int)pointer->currentCores_d, pointer->R_d, output);debugMessage(debugMsg, par);

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
void findBound(sConfiguration *configuration, MYSQL *conn, char *db,  sList *pointer, struct optJrParameters par)
{
	char debugMsg[DEBUG_MSG];
	char statement[256];

    sprintf(debugMsg,"\n findBound %s %s\n", pointer->session_app_id, pointer->app_id);debugMessage(debugMsg, par);

	///Retrieve nCores from the DB
    sprintf(statement,
                        "select num_cores_opt, num_vm_opt from %s.OPTIMIZER_CONFIGURATION_TABLE where application_id='%s' and dataset_size=%d and deadline=%lf;"
                        , db, pointer->app_id, pointer->datasetSize, pointer->Deadline_d);

    MYSQL_ROW row = executeSQL(conn, statement, par);
    if (row == NULL)
    {
    	printf("Fatal error: no matches found on OPTIMIZER_CONFIGURATION_TABLE.\nCHeck that OPT_IC was run over the specific application.\n");
    	exit(-1);
    }

    pointer->nCores_DB_d = atoi(row[0]);
    pointer->vm = atoi(row[1]);

    Bound(configuration, conn, pointer, par, WHOLE_DAGSIM);
    sprintf(debugMsg,"A bound for %s has been calculated\n", pointer->session_app_id);
    debugMessage(debugMsg, par);


}


/*
 * 		Name:					localSearch
 * 		Input parameters:		sList * application_i
 * 		Output parameters:		TBD
 * 		Description:			Localsearch algorithm as per functional analysis
 *
 */
void localSearch(sConfiguration * configuration, MYSQL *conn, sList * application_i, int n, int MAX_PROMISING_CONFIGURATIONS, struct optJrParameters par)
{
	sList * application_j, *first_i = application_i;
	sAux *firstAux = NULL, *currentAux = NULL;
	sAux *sfirstAuxApproximated = NULL;
	char debugMsg[DEBUG_MSG];

	int nCoreMov;
	int stop = 0;
	double DELTAVM_i, DELTAVM_j;
	double DELTA_fo_App_i, DELTA_fo_App_j;
	sAux * minAux;

#ifdef GLOBAL_PRINT
	sStatistics *firstS = NULL, *currentS = NULL;
#endif
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






int index = 0;
#ifdef GLOBAL_PRINT
double TotalFO;
#endif

int how_many;


for (int iteration = 1; iteration <= par.maxIterations; iteration++)
{
	sprintf(debugMsg, "ITERATION %d \n", iteration);debugBanner(debugMsg, par);

	//sfirstAuxApproximated = approximatedLoop(first_i,  sfirstAuxApproximated);
	sfirstAuxApproximated = approximatedLoop(first_i, &how_many, par );
	if (sfirstAuxApproximated == NULL) sprintf(debugMsg, "Empty Candidates list\n");debugInformational(debugMsg, par);

	sprintf(debugMsg, " Ex-iteration loop\n");debugBanner(debugMsg, par);

		// To Do: consider only the first MAX_PROMISING_CONFIGURATIONS of the list
		while (sfirstAuxApproximated != NULL)
		{
			sprintf(debugMsg, "Browsing auxApproximated list");debugBanner(debugMsg, par);
			/* Consider only the first MAX_PROMISING_CONFIGURATIONS list members */
			if (index > 0 && index == MAX_PROMISING_CONFIGURATIONS)
			{
				sprintf(debugMsg,"LocalSearch: MAX_PROMISING_CONFIGURATIONS was reached, leaving sfirstAuxApproximated loop\n");debugMessage(debugMsg, par);
				break;
			}

			strcpy(app_id_i, sfirstAuxApproximated->app1->session_app_id);
			strcpy(app_id_j, sfirstAuxApproximated->app2->session_app_id);
			application_i = searchApplication(first_i, app_id_i);
			application_j = searchApplication(first_i, app_id_j);



			sprintf(debugMsg, "\n\nComparing %s with %s\n", application_i->session_app_id, application_j->session_app_id);
			sprintf(debugMsg, " ");debugBanner(debugMsg, par);

			nCoreMov = max(application_i->V, application_j->V);

			DELTAVM_i = nCoreMov/application_i->V;sprintf(debugMsg, "app %s DELTAVM_i %lf\n", application_i->session_app_id, DELTAVM_i);debugMessage(debugMsg, par);
			DELTAVM_j = nCoreMov/application_j->V;sprintf(debugMsg, "app %s DELTAVM_j %lf\n", application_j->session_app_id, DELTAVM_j);debugMessage(debugMsg, par);

			/* Change the currentCores, but rollback later */
			sprintf(debugMsg,"app %s currentCores %d\n", application_i->session_app_id, (int)application_i->currentCores_d);debugMessage(debugMsg, par);
			sprintf(debugMsg,"app %s currentCores %d\n", application_j->session_app_id, (int)application_j->currentCores_d);debugMessage(debugMsg, par);

			application_i->currentCores_d = application_i->currentCores_d + DELTAVM_i*application_i->V;
			application_j->currentCores_d = application_j->currentCores_d - DELTAVM_j*application_j->V;

			sprintf(debugMsg,"After cores exchange: app %s currentCores %d\n", application_i->session_app_id, (int)application_i->currentCores_d);debugMessage(debugMsg, par);
			sprintf(debugMsg,"After cores exchange: app %s currentCores %d\n", application_j->session_app_id, (int)application_j->currentCores_d);debugMessage(debugMsg, par);

			if (application_i->currentCores_d > 0 && application_j->currentCores_d > 0)
			{
				/* Set up the algorithm for FO evaluation */
				application_i->mode= R_ALGORITHM;application_j->mode= R_ALGORITHM;

				/*
				* Call object function evaluation
				*/

				 DELTA_fo_App_i = ObjFunctionComponent(configuration, conn, application_i, par) - application_i->baseFO; sprintf(debugMsg, "app %s DELTA_fo_App_i %lf\n",
				                                                application_i->session_app_id, DELTA_fo_App_i);debugMessage(debugMsg, par);
				 DELTA_fo_App_j = ObjFunctionComponent(configuration, conn, application_j, par) - application_j->baseFO;sprintf(debugMsg, "app %s DELTA_fo_App_j %lf\n",
				                                                application_j->session_app_id, DELTA_fo_App_j);debugMessage(debugMsg, par);





				// DANILO Store total delta complessivo and number of cores into auxiliary list -> ENRICO DONE

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


			}
			// DANILO restore previous number of cores -> ENRICO DONE
			application_i->currentCores_d = application_i->currentCores_d - DELTAVM_i*application_i->V;
			application_j->currentCores_d = application_j->currentCores_d + DELTAVM_j*application_j->V;
			sfirstAuxApproximated = sfirstAuxApproximated->next;

		}

	readAuxList(firstAux, par);

	// TODO DANILO retrieve from the auxiliary list the element with the smallest delta_fo -> DONE
	// TODO: consider only the first n elements of the list (sorted by total FO given by (app_i+app_j)) ->DONE
	//minAux = findMinDelta(firstAux);
	//if (minAux == NULL)
	minAux = firstAux; // The list is now sorted, so the smallest element is the first;



	sprintf(debugMsg, " ");debugBanner(debugMsg, par);
#ifdef GLOBAL_PRINT

		TotalFO = ObjFunctionGlobal(configuration, conn, first_i, par);
		sprintf(debugMsg,"\n\nGlobal obj function %lf\n", TotalFO);debugMessage(debugMsg, par);
		/* Update Statistics */
		addStatistics(&firstS, &currentS, iteration, how_many, TotalFO);
#endif


	index++;

	if (firstAux ==NULL)
	{
		sprintf(debugMsg, "Information: LocalSearch Auxiliary list empty.\n");debugInformational(debugMsg, par);
		//if (sfirstAuxApproximated) freeAuxList(sfirstAuxApproximated);
		currentAux = NULL;
		stop = 1;
		readSolution(first_i);
	}
	else
	{
		// TODO DANILO assign number of cores to the applications -> DONE
		commitAssignment(first_i, minAux->app1->session_app_id, minAux->delta_i, par); // application i
		commitAssignment(first_i, minAux->app2->session_app_id, -minAux->delta_j, par); // application j
	}

	sprintf(debugMsg, "Information: LocalSearch: Destroy Aux list\n");debugInformational(debugMsg, par);

	// DESTROY Auxiliary lists and prepare it for a new run

	if (sfirstAuxApproximated) freeAuxList(sfirstAuxApproximated);
	if (firstAux) freeAuxList(firstAux);

	firstAux = NULL;
	currentAux = NULL;

	if (stop) break;

	// TODO Modify to recalculate only FO for apps i,j (use the above copies without invoke dagSim)
	initialize(configuration, conn, first_i, par);

}

#ifdef GLOBAL_PRINT
readStatistics(firstS, par);

if (firstS) freeStatisticsList(firstS);

#endif
}




/*
 *
 */


void initialize(sConfiguration * configuration, MYSQL *conn, sList * application_i, struct optJrParameters par)
{
	char debugMsg[DEBUG_MSG];

	sprintf(debugMsg, "Information: INITIALIZE baseFo for all the applications\n");debugInformational(debugMsg, par);
	while (application_i != NULL)
	{

			application_i->mode = R_ALGORITHM;
			application_i->baseFO = ObjFunctionComponent(configuration, conn, application_i, par);
			sprintf(debugMsg,"\n\n INITIALIZE BASE FO for APP %s baseFO =%lf\n\n", application_i->session_app_id, application_i->baseFO);debugMessage(debugMsg, par);
			application_i = application_i->next;
	}
}





/*
 * 		Name:					calculate_Nu
 * 		Input parameters:		MYSQL *conn, char * finsertuniqueFilename, sList *current, double nu_1, double w1, double csi_1, double chi_c_1
 * 		Output parameters:		none
 * 		Description:			Given nu_1 and other measures related to the first applications:
 * 								- it computes the nu indices for all the other applications:
 * 								- it updates the DB;
 * 								- it calculates the bound for each application
 *
 */
void calculate_Nu(sConfiguration * configuration, MYSQL *conn, sList *first, struct optJrParameters par)
{
	char debugMsg[DEBUG_MSG];
	sList * current = first;
	int rows = 0;
	char * app_id;
	int w1;
	double chi_c_1;
	double csi_1;
	int N = par.number;
	double csi;

	int minCapacity= 0;


	sprintf(debugMsg, "Calculate nu indices and bounds for each application");debugBanner(debugMsg, par);

	//TO DO sum di V_i -> DONE
	while (current != NULL)
	{
		minCapacity+= current->V;
		current = current->next;
	}
	current = first;

	N = N - minCapacity;

	if (current == NULL)
	{
		printf("Fatal error: calculate_nu: current cannot be NULL\n");
		exit(-1);
	}

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
			//printf("Calculate_nu first app: %s w1 %d chi_c %lf chi_c_1%lf\n", current->app_id, w1, chi_c_1, csi_1);
		}
		else /*Any other row */
		{
		       csi = getCsi(current->M/current->m, current->V/current->v);
		       current->term_i = sqrt((current->w/w1)*(current->chi_C/chi_c_1)*(csi_1/csi));
		       tot = tot + current->term_i;
		      // printf("Calculate_nu  Other rows: %s w %d csi %lf tot %lf\n", current->app_id, current->w, csi, tot);
		}
		rows++;
		current = current->next;
	}

	double nu_1 = N/(1 + tot);
	//printf("nu_1=%lf\n", nu_1);

	rows = 0;
	tot = 0;
	current = first;
	double term_j;

	while (current != NULL)
	{
		findBound(configuration, conn, getConfigurationValue(configuration, "OptDB_dbName"), current, par);
		if (rows > 0)
		{
			csi = getCsi(current->M/current->m, current->V/current->v);
			term_j = sqrt((current->w/w1)*(current->chi_C/chi_c_1)*(csi_1/csi));
			tot = tot+ term_j;

		} else rows++;
	    current = current->next;
	}

	rows = 0;
	    while (first != NULL)
	    {
	    	if (rows == 0) first->nu_d = nu_1;
	    	else
	    	{
	    		first->nu_d = (first->term_i/(1 + tot))*N;
	    		//printf("\nTERM app %s %lf tot %lf\n", first->app_id, first->term_i, (1 + tot) );
	    	}
	    	//printf("NU_i%lf nu1 %lf\n", first->nu_d, nu_1);
	    	first->currentCores_d = first->nu_d;



	    	first->beta = computeBeta(first->sAB);
	    	first->alpha = computeAlpha(first->sAB, first->beta);

	        //printf("App %s alpha = %lf beta = %lf\n", first->app_id, first->alpha, first->beta);
	        //printf(" R %lf nCores %d\n", first->sAB.vec[0].R, first->sAB.vec[0].nCores);
	        //printf(" R %lf nCores %d\n", first->sAB.vec[1].R, first->sAB.vec[1].nCores);

	        //current->currentCores_d = current->bound_d;

	        printRow(first, par);

	        first = first->next;
	        rows++;
	   }
	    sprintf(debugMsg,"end calculate nu");debugBanner(debugMsg, par);
}



