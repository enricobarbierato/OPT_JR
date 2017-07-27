#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>


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
void addParameters(int nApp, sList ** first, sList ** current,  char * app_id, double w, double chi_0, double chi_C, double chi_c_1, double m, double M, double V, double v, double Deadline_d, double csi,
		double csi_1, char * StageId, int datasetSize)
{


	  sList *new = (sList*) malloc(sizeof(sList));
	  if (new == NULL)
	  {
		  printf("addParameters: Fatal Error: malloc failure\n");
		  exit(-1);
	  }


	  if (nApp == 0) new->chi_c_1 = chi_c_1;


	  new->w = w;
	  new->app_id = (char *)malloc(1024);
	  if (new->app_id == NULL)
	  {
	  	  	    printf("addParameters: malloc failure\n");
	  	  	    exit(-1);
	  }
	  strcpy(new->app_id, app_id);
	  new->chi_0 = chi_0;
	  new->chi_C = chi_C;

	  new->m = m;
		    new->M = M;
		    new->V = V;
		    new->v = v;
		    new->Deadline_d = Deadline_d;
		    new->csi = csi;
		    new->csi_1 = csi_1;
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

		 new->bound_d = 0;

		 new->next = NULL;

	  if (*first == NULL) *first = new;
	  else (*current)->next = new;
	  *current = new;
}


/*
 * 		Name:					readList
 * 		Input parameters:		sList *pointer
 * 		Output parameters:		Pointer to the first application
 * 		Description:			This function prointd the information about all the applications in the list. It is used for debug only.
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
/*
    printf("app_id = %s  w = %lf chi_0 = %lf chi_c_1 = %lf m = %lf M = %lf \n V = %lf v = %lf D = %d R = %lf "
    		" bound = %lf nu = %lf currentcores = %lf nCores = %lf \n\n ",
    		pointer->app_id,
			pointer->w,
			pointer->chi_0,
			pointer->chi_c_1,
			pointer->m,
			pointer->M,
			pointer->V,
			pointer->v,
			pointer->Deadline_d,
			pointer->R_d,
			pointer->bound_d,
			pointer->nu_d,
			pointer->currentCores_d,
			pointer->nCores_d);
			*/
	printf("app_id %s  nu %d currentcores = %d nCores = %d \n\n", pointer->app_id, (int)pointer->nu_d, (int)pointer->currentCores_d, (int)pointer->nCores_d);
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
 * 		Name:					freeList
 * 		Input parameters:		sList *pointer
 * 		Output parameters:		Pointer to the first application
 * 		Description:			It releases the allocated memory for the list
 *
 */
void freeList(sList * pointer)
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
 * 		Name:					returnARow
 * 		Input parameters:		sList *pointer
 * 		Output parameters:		Pointer to the first application
 * 		Description:			This function returns, one by one, the applications of the list one by one
 *
 */

sList * returnARow(sList ** first )
{
	if (*first == NULL) return NULL;

	sList * next = *first;
	*first = (*first)->next;

	return(next);
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

	while (pointer != NULL)
	    {
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
	int min = INT_MAX;
	sAux *minAux = NULL;

	if (pointer == NULL)
	{
		printf("pointer to auxiliary struct is null in findMinDelta\n");
		exit(-1);
	}
	while (pointer != NULL)
	{
		if (pointer->deltaFO < min)
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
	double tot = 0;

	while (pointer!= NULL)
	{
		printf("app %s currentCores %d", pointer->app_id, (int)pointer->currentCores_d);
		tot = tot + pointer->currentCores_d;
		pointer = pointer->next;
	}
	printf("\nTOTALE CORES :%d\n", (int)tot);
	return doubleCompare(tot, N) == 0;
}




/*
 * 		Name:					addAuxParameters
 * 		Input parameters:		sAux ** first, sAux ** current,  char * app_id1, char * app_id2, int contr1, int contr2, double delta
 * 		Output parameters:		Updated pointers to the first and current element of the list
 * 		Description:			This function adds all the information regarding the localSearch deltafo calculation
 *
 */
void addAuxParameters(sAux ** first, sAux ** current,  char * app_id1, char * app_id2, int contr1, int contr2, double delta, double delta_i, double delta_j)
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


	  /*
	   * Applications
	   */
	  new->app1 = (char *)malloc(1024);
	  if (new->app1 == NULL)
	  {
	  	  	    printf("addAuxParameters (char *): malloc failure\n");
	  	  	    exit(-1);
	  }
	  strcpy(new->app1, app_id1);

	  new->app2 = (char *)malloc(1024);
	  if (new->app2 == NULL)
	  {
	  	  	printf("addAuxParameters (char *): malloc failure\n");
	  	  	exit(-1);
	  }
	  strcpy(new->app2, app_id2);

	  new->newCoreAssignment1 = contr1;
	  new->newCoreAssignment2 = contr2;
	  new->deltaFO = delta;
	  new->delta_i = delta_i;
	  new->delta_j = delta_j;
	  new->next = NULL;

	  if (*first == NULL) *first = new;
	  else (*current)->next = new;
	  *current = new;
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

		pointer = pointer->next;
	}
	printf("\n");
}

void printAuxRow(sAux *pointer)
{

    printf("app_id1 = %s  app_id2 = %s newCoresAssignment1 = %d newCoresAssignment2 = %d Totdelta = %lf delta1 = %lf delta2 = %lf\n\n ",
    		pointer->app1, pointer->app2, (int)pointer->newCoreAssignment1, (int)pointer->newCoreAssignment2,
			pointer->deltaFO, pointer->delta_i, pointer->delta_j);
}




