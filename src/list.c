#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <sys/time.h>

#include "list.h"
#include "common.h"


/*
 * 		Name:					addParameters
 * 		Input parameters:		int nApp, sList ** first, sList ** current,  char * app_id, double w, double chi_0, double chi_C, double chi_c_1, double m, double M, double V, double v, int D, double csi,
		double csi_1, char * StageId, int datasetSize
 * 		Output parameters:		Uodated pointers to the first and current element of the list
 * 		Description:			This function adds all the information regarding an application into a list
 *
 */
void addParameters(sList ** first,   sList ** current, char *session_app_id, char * app_id, double w, double chi_0, double chi_C, double m, double M, double V, double v, double Deadline_d, double csi,
		char * StageId, int datasetSize)
{


	  sList *new = (sList*) malloc(sizeof(sList));
	  if (new == NULL)
	  {
		  printf("addParameters: Fatal Error: malloc failure\n");
		  exit(-1);
	  }

	  new->w = w;
	  new->app_id = (char *)malloc(1024);
	  if (new->app_id == NULL)
	  {
	  	  	    printf("addParameters: malloc failure\n");
	  	  	    exit(-1);
	  }
	  strcpy(new->app_id, app_id);

	  new->session_app_id = (char *)malloc(1024);
	  if (new->session_app_id == NULL)
	  {
	  	  	printf("addParameters: malloc failure\n");
	  	  	exit(-1);
	  }
	  strcpy(new->session_app_id, session_app_id);

	  new->chi_0 = chi_0;
	  new->chi_C = chi_C;
	  new->m = m;
	  new->M = M;
	  new->V = V;
	  new->v = v;
	  new->Deadline_d = Deadline_d;
	  new->csi = csi;
	  new->boundIterations = 0;


	  new->stage = (char *)malloc(1024);
	  if (new->stage == NULL)
	  {
		  printf("addParameters: malloc failure (stage)\n");
		  exit(-1);
	  }
	  strcpy(new->stage, StageId);
	  new->datasetSize = datasetSize;

	  /* Initialize the parameter that will be calculated later */
	  new->R_d = 0;
	  new->baseFO = -1;
	  new->bound = 0;

	  new->next = NULL;

	  /*
	  if (*first == NULL) *first = new;
	  else (*current)->next = new;
	  *current = new;
	   */

	  if (*first == NULL) *first = new;
	 	 	 	  else
	 	 	 		  if (doubleCompare((*first)->w, w) == 1)
	 	 	 		  {
	 	 	 			  new->next = *first;
	 	 	 			  *first = new;
	 	 	 		  }
	 	 	 		  	 else
	 	 	 			 {
	 	 	 		  		sList * previous = *first;
	 	 	 		  	sList * current = (*first)->next;

	 	 	 				 while (current != NULL && current->w < w)
	 	 	 				 {
	 	 	 					 previous = current;
	 	 	 					 current = current->next;
	 	 	 				 }

	 	 	 				 previous->next = new;
	 	 	 				 new->next = current;
	 	 	 			 }



}



sList * searchApplication(sList * first, char *session_appId)
{
	while (first != NULL)
	{
		if (strcmp(first->session_app_id, session_appId) == 0) return first;
		first = first->next;
	}
	if (first == NULL)
	{
		printf("Fatal Error: searchApplication: could not find session application id %s\n", session_appId);
		exit(-1);
	}

	return NULL;
}

/*
 * 		Name:					writeList
 * 		Input parameters:		sList *pointer
 * 		Output parameters:		Pointer to the first application
 * 		Description:			This function prints the information about all the applications in the list. It is used for debug only.
 *
 */


void writeList(MYSQL *conn, char * dbName, sList *pointer, struct optJrParameters par)
{
	char debugMsg[DEBUG_MSG];
	char sqlStatement[512];

	sprintf(debugMsg, "\n\nApplications list content:\n");debugInformational(debugMsg, par);


	while (pointer!=NULL)
	{
		sprintf(sqlStatement, "insert %s.OPT_SESSIONS_RESULTS_TABLE "
				"values('%s', '%s',%d, %d)",
				dbName,
				strcat(par.filename,pointer->session_app_id),
				pointer->app_id,
				pointer->currentCores_d,
				pointer->vm
			);
		if (mysql_query(conn, sqlStatement))
		{
				char error[512];
				sprintf(error, " %s", sqlStatement);
				DBerror(conn, error);
		}
		pointer = pointer->next;
	}
	sprintf(debugMsg, "\n");debugMessage(debugMsg, par);
}

void readList(sList *pointer, struct optJrParameters par)
{
	char debugMsg[DEBUG_MSG];

	sprintf(debugMsg, "\n\nApplications list content:\n");debugInformational(debugMsg, par);


	while (pointer!=NULL)
	{
		printRow(pointer, par);
		//if (pointer->previous!=NULL) printf("(prev. %lf) ", pointer->previous->T);
		pointer = pointer->next;
	}
	sprintf(debugMsg, "\n");debugMessage(debugMsg, par);
}


void readSolution(sList *pointer)
{

	while (pointer!=NULL)
	{
		printf("%s %d\n", pointer->session_app_id, pointer->currentCores_d);
		pointer = pointer->next;
	}

}


void printRow(sList *pointer, struct optJrParameters par)
{

	char debugMsg[DEBUG_MSG];

	sprintf(debugMsg, "session_app_id %s app_id %s  weight %d nu %lf iterations to find the bound %d currentcores = %d nCores from DB = %d \n\n",
			pointer->session_app_id, pointer->app_id, pointer->w, pointer->nu_d, pointer->boundIterations, pointer->currentCores_d, (int)pointer->nCores_DB_d);debugMessage(debugMsg, par);

}

void commitAssignment(sList *pointer, char *session_appId,  double DELTA, struct optJrParameters par)
{
	char debugMsg[DEBUG_MSG];

	while (pointer != NULL)
		if (strcmp(pointer->session_app_id, session_appId) == 0) break;
		else pointer = pointer->next;

	if (pointer == NULL)
	{
		printf("Application %s not found in the list\n", session_appId);
		exit(-1);
	}

	/* Check that currentCores_d is positive: ignore if otherwise
			 *
	*/
	if ((int)pointer->currentCores_d + DELTA*pointer->V <= 0)
	{
		printf("Negative or zero value for currentCores was not committed\n");
		return;
	}


	pointer->currentCores_d = pointer->currentCores_d + DELTA*pointer->V;
	sprintf(debugMsg, "Committed %s currentCores = %d\n", pointer->session_app_id, (int)pointer->currentCores_d);debugMessage(debugMsg, par);


}



/*
 * 		Name:					freeApplicationList
 * 		Input parameters:		sList *pointer
 * 		Output parameters:		Pointer to the first application
 * 		Description:			It releases the allocated memory for the list
 *
 */
void freeApplicationList(sListPointers * pointer)
{
	sListPointers * tmp;


		while (pointer != NULL)
		    {
		       tmp = pointer;
		       pointer = pointer->next;
		       free(tmp);
		    }
}

/*
 * 		Name:					freeParametersList
 * 		Input parameters:		sList *pointer
 * 		Output parameters:		Pointer to the first application
 * 		Description:			It releases the allocated memory for the list
 *
 */
void freeParametersList(sList * pointer)
{
	sList * tmp;

	while (pointer != NULL)
			    {
			       tmp = pointer;
			       pointer = pointer->next;
			       free(tmp);
			    }

}




/*
 * 		Name:					freeAuxList
 * 		Input parameters:		sAux *pointer
 * 		Output parameters:		Pointer to the first application
 * 		Description:			It releases the allocated memory for the list
 *
 */
void freeAuxList(sAux * pointer)
{
	sAux * tmp;
	while (pointer != NULL)
			    {
			       tmp = pointer;
			       pointer = pointer->next;
			       free(tmp);
			    }

}


void freeStatisticsList(sStatistics * pointer)
{
	sStatistics * tmp;

	while (pointer != NULL)
			    {
			       tmp = pointer;
			       pointer = pointer->next;
			       free(tmp);
			    }

}


/*
 * 		Name:					findMinDelta
 * 		Input parameters:		sAux *pointer
 * 		Output parameters:		Minimum Delta
 * 		Description:			It retrieves the minimum delta
 *
 */
sAux * findMinDelta(sAux * pointer)
{
	double min = DBL_MAX;
	sAux *minAux = NULL;


	while (pointer != NULL)
	{
		if (doubleCompare(pointer->deltaFO, min) == -1)
			{
				min = pointer->deltaFO;
				minAux = pointer;
			}
		pointer = pointer->next;
	}
	return minAux;
}
/*
 *
 */
int checkTotalCores(sList * pointer, double N)
{
	int tot = 0;


	while (pointer!= NULL)
	{
		printf("app %s currentCores %d", pointer->session_app_id, (int)pointer->currentCores_d);
		tot = tot + pointer->currentCores_d;
		pointer = pointer->next;
	}
	printf("\nTOTALE CORES :%d out of %lf\n", tot, N);
	return doubleCompare(tot, N) == 0;
}




/*
 * 		Name:					addAuxParameters
 * 		Input parameters:		sAux ** first, sAux ** current,  char * app_id1, char * app_id2, int contr1, int contr2, double delta
 * 		Output parameters:		Updated pointers to the first and current element of the list
 * 		Description:			This function adds all the information regarding the localSearch deltafo calculation
 *
 */
void addAuxParameters(sAux ** first, sAux ** current,  sList * app1, sList * app2, int contr1, int contr2, double delta, double delta_i, double delta_j)
{
	if (contr1 < 0 || contr2 < 0)
	{
		printf("addAuxParameters: an application has a number of core <= 0\n");
		return;
	}

	  sAux *new = (sAux*) malloc(sizeof(sAux));
	  if (new == NULL)
	  {
		  printf("addAuxParameters: Fatal Error: malloc failure\n");
		  exit(-1);
	  }


	  new->app1 = app1;
	  new->app2 = app2;
	  new->newCoreAssignment1 = contr1;
	  new->newCoreAssignment2 = contr2;
	  new->deltaFO = delta;
	  new->delta_i = delta_i;
	  new->delta_j = delta_j;
	  new->next = NULL;

/*
	  if (*first == NULL) *first = new;
	  else (*current)->next = new;
	  *current = new;*/

	  if (*first == NULL) *first = new;
	 	 	 	  else
	 	 	 		  if (doubleCompare((*first)->deltaFO, delta) == 1)
	 	 	 		  {
	 	 	 			  new->next = *first;
	 	 	 			  *first = new;
	 	 	 		  }
	 	 	 		  	 else
	 	 	 			 {
	 	 	 		  		sAux * previous = *first;
	 	 	 		  		sAux * current = (*first)->next;

	 	 	 				 while (current != NULL && doubleCompare(current->deltaFO, delta) == -1)
	 	 	 				 {
	 	 	 					 previous = current;
	 	 	 					 current = current->next;
	 	 	 				 }

	 	 	 				 previous->next = new;
	 	 	 				 new->next = current;
	 	 	 			 }
}




/*
 * 		Name:					addStatistics
 * 		Input parameters:		sStatistics ** first, sStatistics ** current, int iteration, int how_many
 * 		Output parameters:		current pointer to the list
 * 		Description:			This function adds a new statistics (iteration_id, number of candidates found by hyperbolic approximation
 *
 */
void addStatistics(sStatistics ** first, sStatistics ** current, int iteration, int how_many, double total)
{


	  sStatistics *new = (sStatistics*) malloc(sizeof(sStatistics));
	  if (new == NULL)
	  {
		  printf("addAuxParameters: Fatal Error: malloc failure\n");
		  exit(-1);
	  }



	  new->iteration = iteration;
	  new->size = how_many;
	  new->FO_Total = total;
	  new->next = NULL;


	  if (*first == NULL) *first = new;
	  else (*current)->next = new;
	  *current = new;


}

void addConfiguration(sConfiguration ** first, sConfiguration ** current, char * variable, char * value)
{


	  sConfiguration *new = (sConfiguration*) malloc(sizeof(sConfiguration));
	  if (new == NULL)
	  {
		  printf("addConfiguration: Fatal Error: malloc failure\n");
		  exit(-1);
	  }

	  new->variable = (char *)malloc(256);
	  if (new->variable == NULL)
	  {
		  printf("Malloc failure: addConfiguration\n");
		  exit(-1);
	  }
	  new->value = (char *)malloc(256);
	  if (new->value == NULL)
	  {
	  		printf("Malloc failure: addConfiguration\n");
	  		exit(-1);
	  }

	  strcpy(new->variable, variable);
	  strcpy(new->value, value);
	  new->next = NULL;


	  if (*first == NULL) *first = new;
	  else (*current)->next = new;
	  *current = new;
}

sConfiguration * readConfigurationFile()
{
	FILE * fp;
	    char * line = NULL;
	    size_t len = 0;
	    ssize_t read;
	    char var[256], val[256],* configurationFile;

	    sConfiguration *first = NULL;
	    sConfiguration *current = NULL;

	    configurationFile = getenv("WSI_CONFIG_FILE");

	    if (configurationFile == NULL)
	    {
	    	printf("Fatal error: WSI_CONFIG_FILE environment variable was not defined.\n");
	    	exit(-1);
	    }

	    fp = fopen(configurationFile , "r");
	    if (fp == NULL)
	    {
	    	printf("Fatal Error: parseConfigurationFile: configuration file not found in home directory: (%s)\n", configurationFile);
	    	exit(-1);
	    }



	    while ((read = getline(&line, &len, fp)) != -1)
	    {
	       if (line != NULL)
	    	if (strstr(line, "entry key=") != NULL)
	    	{
	    		strcpy(var, extractItem(line, "\"", "\""));
	    		strcpy(val, extractItem(line, ">", "<"));

	    		addConfiguration(&first, &current, var, val);
	    	}

	    }

	    return first;
}



char *getConfigurationValue(sConfiguration *pointer, char * variable)
{
	int found = 0;

	while (!found && pointer != NULL)
	{
		if (strcmp(pointer->variable, variable) == 0) found = 1;
		else pointer = pointer->next;
	}
	if (!found)
	{
		printf("variable %s not found!\n", variable);
		exit(-1);

	}
	else return pointer->value;
}


void readStatistics(sStatistics *pointer, struct optJrParameters par)
{
	char debugMsg[DEBUG_MSG];

	sprintf(debugMsg, " ");debugBanner(debugMsg, par);
	sprintf(debugMsg, "\n\nStatistics list content:\n");debugMessage(debugMsg, par);
	sprintf(debugMsg, " ");debugBanner(debugMsg, par);

	printf("Iteration   List Size  Total FO\n");
	while (pointer!=NULL)
	{
		sprintf(debugMsg, "%d %d %lf\n", pointer->iteration, pointer->size, pointer->FO_Total);debugMessage(debugMsg, par);

		pointer = pointer->next;
	}
	sprintf(debugMsg, "\n");debugMessage(debugMsg, par);
}

void addListPointers(sListPointers ** first,   sList *application)
{


	  sListPointers *new = (sListPointers*) malloc(sizeof(sListPointers));
	  if (new == NULL)
	  {
		  printf("addListPointers: Fatal Error: malloc failure\n");
		  exit(-1);
	  }

	  new->app= application;
	  new->next = NULL;

	  /*
	  if (*first == NULL) *first = new;
	  else (*current)->next = new;
	  *current = new;
	  */


	  if (*first == NULL) *first = new;
	 	 	 	 	  else
	 	 	 	 		  if (doubleCompare((*first)->app->w, application->w) == 1)
	 	 	 	 		  {
	 	 	 	 			  new->next = *first;
	 	 	 	 			  *first = new;
	 	 	 	 		  }
	 	 	 	 		  	 else
	 	 	 	 			 {
	 	 	 	 		  		sListPointers * previous = *first;
	 	 	 	 		  		sListPointers * current = (*first)->next;

	 	 	 	 				 while (current != NULL && doubleCompare(current->app->w, application->w) == -1)
	 	 	 	 				 {
	 	 	 	 					 previous = current;
	 	 	 	 					 current = current->next;
	 	 	 	 				 }

	 	 	 	 				 previous->next = new;
	 	 	 	 				 new->next = current;
	 	 	 	 			 }

}


void readListPointers(sListPointers *pointer, struct optJrParameters par)
{
	char debugMsg[DEBUG_MSG];

	sprintf(debugMsg, "\n\nListPointers list content:\n");debugMessage(debugMsg, par);


	while (pointer!=NULL)
	{
		printRow(pointer->app, par);
		pointer = pointer->next;
	}
	sprintf(debugMsg, "\n");debugMessage(debugMsg, par);
}



/*
 * 		Name:					readAuxList
 * 		Input parameters:		sAux *pointer
 * 		Output parameters:		Pointer to the first application
 * 		Description:			This function prints the information about all the applications in the list. It is used for debug only.
 *
 */


void readAuxList(sAux *pointer, struct optJrParameters par)
{
	char debugMsg[DEBUG_MSG];

	sprintf(debugMsg, "\n\nAuxiliary list content:\n");debugMessage(debugMsg, par);


	while (pointer!=NULL)
	{
		printAuxRow(pointer, par);
		//printf("%lf\n", pointer->deltaFO);

		pointer = pointer->next;
	}
	sprintf(debugMsg, "\n");debugMessage(debugMsg, par);
}

void printAuxRow(sAux *pointer, struct optJrParameters par)
{
	char debugMsg[DEBUG_MSG];
	printRow(pointer->app1, par);
	printRow(pointer->app2, par);
    sprintf(debugMsg, "newCoresAssignment1 = %d newCoresAssignment2 = %d Totdelta = %lf delta1 = %lf delta2 = %lf\n\n ",
    		(int)pointer->newCoreAssignment1, (int)pointer->newCoreAssignment2,
			pointer->deltaFO, pointer->delta_i, pointer->delta_j);debugMessage(debugMsg, par);
}




