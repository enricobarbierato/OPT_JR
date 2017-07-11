#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"


/*
 * 		Name:					addParameters
 * 		Input parameters:		int nApp, sList ** first, sList ** current,  char * app_id, double w, double chi_0, double chi_C, double chi_c_1, double m, double M, double V, double v, int D, double csi,
		double csi_1, char * StageId, int datasetSize
 * 		Output parameters:		Uodated pointers to the first and current element of the list
 * 		Description:			This function adds all the information regarding an application into a list
 *
 */
void addParameters(int nApp, sList ** first, sList ** current,  char * app_id, double w, double chi_0, double chi_C, double chi_c_1, double m, double M, double V, double v, int D, double csi,
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
		    new->D = D;
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
		 new->R = 0;
		 new->Rnew = 0;
		 new->bound = 0;

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

    printf("app_id = %s  w = %lf chi_0 = %lf chi_c_1 = %lf m = %lf M = %lf V = %lf v = %lf D = %d R = %d Rnew = %d bound = %d nu = %lf\n ",
    		pointer->app_id, pointer->w,  pointer->chi_0, pointer->chi_c_1, pointer->m, pointer->M, pointer->V, pointer->v, pointer->D, pointer->R, pointer->Rnew, pointer->bound, pointer->nu);
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

