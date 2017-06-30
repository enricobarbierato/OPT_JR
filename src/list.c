#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"


void addResult(sResult ** first, sResult ** current,  int index, double nu, char * app_id)
{


	  sResult *new = (sResult*) malloc(sizeof(sResult));
	  if (new == NULL)
	  {
		  printf("addResults: Fatal Error: malloc failure\n");
		  exit(-1);
	  }

	  new->app_id = (char *)malloc(1024);
	  if (new->app_id == NULL)
	  {
		  printf("addResults(app_id): Fatal Error: malloc failure\n");
	  }
	  strcpy(new->app_id, app_id);
	  new->index = index;
      new->nu = nu;
      new->next = NULL;

	  if (*first == NULL) *first = new;
	  else (*current)->next = new;
	  *current = new;
}

void addParameters(sList ** first, sList ** current,  char * app_id, double w, double w1, double chi_0, double chi_C, double chi_c_1, double m, double M, double V, double v, double D, double csi, double csi_1)
{


	  sList *new = (sList*) malloc(sizeof(sList));
	  if (new == NULL)
	  {
		  printf("addParameters: Fatal Error: malloc failure\n");
		  exit(-1);
	  }
	  	    new->app_id = (char *)malloc(1024);
	  	    if (new->app_id == NULL)
	  	    {
	  	    	printf("addParameters: malloc failure\n");
	  	    	exit(-1);
	  	    }
	  	    strcpy(new->app_id, app_id);
		    new->w = w;
		    new->w1 = w1;
		    new->chi_0 = chi_0;
		    new->chi_C = chi_C;
		    new->chi_c_1 = chi_c_1;
		    new->m = m;
		    new->M = M;
		    new->V = V;
		    new->v = v;
		    new->D = D;
		    new->csi = csi;
		    new->csi_1 = csi_1;

		 new->next = NULL;

	  if (*first == NULL) *first = new;
	  else (*current)->next = new;
	  *current = new;
}



void readResult(sResult *pointer)
{
while (pointer!=NULL)
	{
		printf("nu_%d = %lf\n", pointer->index, pointer->nu);
		//if (pointer->previous!=NULL) printf("(prev. %lf) ", pointer->previous->T);
		pointer = pointer->next;
	}
	printf("\n");
}

void readList(sList *pointer)
{
	if (pointer == NULL) return;


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

    printf("w1 = %lf w = %lf chi_0 = %lf chi_c_1 = %lf m = %lf M = %lf V = %lf v = %lf D = %lf\n ",
    		pointer->w1, pointer->w,  pointer->chi_0, pointer->chi_c_1, pointer->m, pointer->M, pointer->V, pointer->v, pointer->D);
}

void searchResult(sResult *pointer, char * app_id)
{
	while (pointer!=NULL)
	{
		if (strcmp(pointer->app_id, app_id) == 0)
		{
			printf("%lf\n", pointer->nu);
			break;
		}
		else pointer = pointer->next;
	}
}

void freeResultList(sResult * pointer)
{
	sResult * next;
	while (pointer != NULL)
	{
		next = pointer->next;
		free(pointer);
		pointer = next;
	}
}
sList * returnARow(sList ** first )
{
	if (*first == NULL) return NULL;

	sList * next = *first;
	*first = (*first)->next;

	return(next);
}

