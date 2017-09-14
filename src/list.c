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



sList * searchApplication(sList * first, char *appId)
{
	while (first != NULL)
	{
		if (strcmp(first->app_id, appId) == 0) return first;
		first = first->next;
	}
	return NULL;
}

/*
 * 		Name:					readList
 * 		Input parameters:		sList *pointer
 * 		Output parameters:		Pointer to the first application
 * 		Description:			This function prints the information about all the applications in the list. It is used for debug only.
 *
 */


void readList(sList *pointer)
{
	printf("\n\nApplications list content:\n");


	while (pointer!=NULL)
	{
		printRow(pointer);
		//if (pointer->previous!=NULL) printf("(prev. %lf) ", pointer->previous->T);
		pointer = pointer->next;
	}
	printf("\n");
}

void printRow(sList *pointer)
{

    /*printf("session_pp_id =%s app_id = %s  w = %lf chi_0 = %lf chi_c_1 = %lf m = %lf M = %lf \n V = %lf v = %lf D = %d R = %lf "
    		" bound = %lf nu = %lf currentcores = %lf nCores = %lf \n\n ",
			pointer->session_app_id,
			pointer->app_id,
			pointer->w,
			pointer->chi_0,
			pointer->chi_C,
			pointer->m,
			pointer->M,
			pointer->V,
			pointer->v,
			pointer->Deadline_d,
			pointer->R_d,
			pointer->bound_d,
			pointer->nu_d,
			pointer->currentCores_d,
			pointer->nCores_DB_d);
			*/
	printf("session_app_id %s app_id %s  weight %d nu %lf iterations to find the bound %d currentcores = %d nCores from DB = %d \n\n",
			pointer->session_app_id, pointer->app_id, pointer->w, pointer->nu_d, pointer->boundIterations, pointer->currentCores_d, (int)pointer->nCores_DB_d);
}

void commitAssignment(sList *pointer, char *appId,  double DELTA)
{

	while (pointer != NULL)
		if (strcmp(pointer->app_id, appId) == 0) break;
		else pointer = pointer->next;

	if (pointer == NULL)
	{
		printf("Application %s not found in the list\n", appId);
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
	printf("Committed %s currentCores = %d\n", pointer->app_id, (int)pointer->currentCores_d);


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
	sListPointers * next;

	while (pointer != NULL)
	    {
		//if (pointer->app != NULL) free(pointer->app);
	       next = pointer;
	       pointer = pointer->next;
	       if (next != NULL) free(next);
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
	sList * next;

	while (pointer != NULL)
	    {
	       next = pointer;
	       pointer = pointer->next;
	       if (next != NULL) free(next);
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
	sAux * next;
	if (pointer == NULL) return;

	while (pointer != NULL)
	    {
	       next = pointer;
	       pointer = pointer->next;
	       if (next != NULL) free(next);
	    }
}


void freeStatisticsList(sStatistics * pointer)
{
	sStatistics * next;

	if (pointer == NULL)
	{
		printf("Warning: Statistics list is empty\n");
		return;
	}
	printf("Statistics \n");
	while (pointer != NULL)
	    {
		//if (pointer->app != NULL) free(pointer->app);
	       next = pointer;
	       pointer = pointer->next;
	       if (next != NULL) free(next);
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
		printf("app %s currentCores %d", pointer->app_id, (int)pointer->currentCores_d);
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
 * 		Name:					addCacheParameters
 * 		Input parameters:		sPredictorCash ** first, sPredictorCash ** current,  int nCores, int datasize, double output
 * 		Output parameters:		Updated pointers to the first and current element of the list
 * 		Description:			This function adds all the information regarding the localSearch deltafo calculation
 *
 */
void addCacheParameters(sPredictorCash ** first, sPredictorCash ** current,  char * app_id, int nCores, int datasize, double output)
{

	sPredictorCash *new = (sPredictorCash*) malloc(sizeof(sPredictorCash));
	  if (new == NULL)
	  {
		  printf("addsPredictorCashParameters: Fatal Error: malloc failure\n");
		  exit(-1);
	  }
	  strcpy(new->app_id, app_id);
	  new->ncores = nCores;
	  new->datasize = datasize;
	  new->output = output;
	  new->next = NULL;

/*
	  if (*first == NULL) *first = new;
	  else (*current)->next = new;
	  *current = new;*/

	  if (*first == NULL) *first = new;
	 	 	 	  else
	 	 	 		  if (doubleCompare((*first)->output, output) == 1)
	 	 	 		  {
	 	 	 			  new->next = *first;
	 	 	 			  *first = new;
	 	 	 		  }
	 	 	 		  	 else
	 	 	 			 {
	 	 	 		  		 sPredictorCash * previous = *first;
	 	 	 		  		 sPredictorCash * current = (*first)->next;

	 	 	 				 while (current != NULL && doubleCompare(current->output, output) == -1)
	 	 	 				 {
	 	 	 					 previous = current;
	 	 	 					 current = current->next;
	 	 	 				 }

	 	 	 				 previous->next = new;
	 	 	 				 new->next = current;
	 	 	 			 }
}

void printCacheParameters(sPredictorCash * pointer)
{
	while (pointer != NULL)
	{
		printf("app_id %s nCores %d datasize %d  output %lf\n", pointer->app_id, pointer->ncores, pointer->datasize, pointer->output);
		pointer = pointer->next;
	}
}

double searchCacheParameters(sPredictorCash * pointer, char * app_id, int nCores, int datasize)
{
	while (pointer != NULL)
		if (strcmp(pointer->app_id, app_id) == 0 && pointer->ncores == nCores && pointer->datasize == datasize)
			return(pointer->output);
		else pointer = pointer->next;

	/* The parameter was not found */
	return -1;

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

void printOutput(sList * pointer)
{
	FILE *fp;
	char filename[1024];



	struct timeval tv;
	gettimeofday(&tv,NULL);


	strcpy(filename, parseConfigurationFile("OUTPUT_FILE", 1));
	sprintf(filename, "%s/opt_jr_output.%lf.csv", parseConfigurationFile("OUTPUT_FILE", 1), (double)tv.tv_sec);

	fp=fopen(filename,"w+");

	fprintf(fp, "#AppId, currentCores\n");
	while (pointer!= NULL)
	{
		fprintf(fp,"\n%s,%d",pointer->app_id, pointer->currentCores_d);
		pointer = pointer->next;
	}



	fclose(fp);
}

void readStatistics(sStatistics *pointer)
{
	printf("******************************\n");
	printf("\n\nStatistics list content:\n");
	printf("******************************\n");

	printf("Iteration   List Size  Total FO\n");
	while (pointer!=NULL)
	{
		printf("%d %d %lf\n", pointer->iteration, pointer->size, pointer->FO_Total);
		pointer = pointer->next;
	}
	printf("\n");
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


void readListPointers(sListPointers *pointer)
{
	printf("\n\nListPointers list content:\n");


	while (pointer!=NULL)
	{
		printRow(pointer->app);

		pointer = pointer->next;
	}
	printf("\n");
}



/*
 * 		Name:					readAuxList
 * 		Input parameters:		sAux *pointer
 * 		Output parameters:		Pointer to the first application
 * 		Description:			This function prints the information about all the applications in the list. It is used for debug only.
 *
 */


void readAuxList(sAux *pointer)
{
	printf("\n\nAuxiliary list content:\n");


	while (pointer!=NULL)
	{
		printAuxRow(pointer);
		//printf("%lf\n", pointer->deltaFO);

		pointer = pointer->next;
	}
	printf("\n");
}

void printAuxRow(sAux *pointer)
{
	printRow(pointer->app1);
	printRow(pointer->app2);
    printf("newCoresAssignment1 = %d newCoresAssignment2 = %d Totdelta = %lf delta1 = %lf delta2 = %lf\n\n ",
    		(int)pointer->newCoreAssignment1, (int)pointer->newCoreAssignment2,
			pointer->deltaFO, pointer->delta_i, pointer->delta_j);
}




