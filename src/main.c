/*
 * io.c
 *
 *  Created on: 10 Mar 2017
 *      Author: Enrico
 */



#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include "db.h"

#include "list.h"
#include "main.h"



#define MIN(a, b) (((a) < (b)) ? (a) : (b))



sListPointers * fixInitialSolution(sList *applications, int N)
{
	sList * first;
	int allocatedCores;
	sListPointers * first_LP = NULL;
	int loopExit = 0;
	sListPointers *auxPointer;
	int residualCores;


	allocatedCores = 0; // TODO To be changed into INT ->DONE

	first = applications;

	while (first != NULL)
	{
		int currentcores1 =first->currentCores_d;
		double currentcores2=max(((int)(first->currentCores_d / first->V)) * first->V,first->V);


		first->currentCores_d = max(((int)(first->currentCores_d / first->V)) * first->V,first->V);
		if (first->currentCores_d > first->bound)
			first->currentCores_d = first->bound;
		else
			{
				printf("adding %s to ListPointers\n", first->app_id);
				addListPointers(&first_LP, first);
			}

		// Danilo Application (suffering) insert in the new list
		// TODO Handle insert in such a way the list is sorted by weight -> DONE

		allocatedCores+= first->currentCores_d;
		printf("***fixInitialSolution FIXING CORES*** %s %d\n", first->app_id, first->currentCores_d);
		first = first->next;
	}
	//readListPointers(first_LP);

	printf("fixInitialSolution: allocatedCores %d\n", allocatedCores);

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
				printf("\nFatal Error: FixInitialSolution: app %s has %d cores after fix\n", auxPointer->app->app_id, auxPointer->app->currentCores_d);
				exit(-1);
			}
			if (addedCores > 0)
			{
				//auxPointer->app->currentCores_d+= addedCores;

				printf("adding cores to App %s, %d \n", auxPointer->app->app_id, addedCores);

				printf(" applicationid %s new cores %d moved cores %d\n", auxPointer->app->app_id, (int)auxPointer->app->currentCores_d, addedCores);

				residualCores = residualCores - addedCores;
			}
			auxPointer = auxPointer->next;
		}

		if (residualCores == 0) loopExit = 1;
	}
	readList(applications);

	return first_LP;
}

int main(int argc, char **argv)
{

    double w;
    double chi_0;
    double chi_C;
    double m;
    double M;
    double V;
    double v;
    double D;
    double csi;
    char * session_app_id;
    char * app_id;
    char * St;
    int DatasetSize;
    double N;
    char line[1024];

    int rows = 1;

    struct timeval  tv_initial_main,
    				tv_initial_nu,
    				tv_final_nu,
    				tv_initial_fix,
    				tv_final_fix,
    				tv_initial_locals,
    				tv_final_locals,
    				tv_final_main;
    /*
     * Check Usage
     */

    if (argc < 4) Usage();

    /* Connect to the db */
    MYSQL *conn = DBopen(
            			parseConfigurationFile("OptDB_IP", XML),
    					parseConfigurationFile("OptDB_user", XML),
    					parseConfigurationFile("OptDB_pass", XML),
    					parseConfigurationFile("OptDB_dbName", XML)
    					);
    if (conn == NULL) DBerror(conn, "open_db: Opening the database");

    // Calculate the time taken
    gettimeofday(&tv_initial_main, NULL);


    gettimeofday(&tv_initial_nu, NULL);
    /*
     * Find where the file has been uploaded and determine absolute file path
     */
    char *folder = parseConfigurationFile("UPLOAD_HOME", XML);
    char *filename = strcat(folder, "/");
    filename = strcat(folder, argv[1]);

    /*
     * Read total cores available
     */
    N = atof(argv[2]);
    int MAX_PROMISING_CONFIGURATIONS = atoi(argv[3]);



    FILE* stream = fopen(filename, "r");
    if (stream == NULL)
    {
    	printf("FATAL ERROR: could not find or open %s\n", filename);
    	exit(-1);
    }


    /*
     * Initialize Vars
     */
    sList *first = NULL, *current = NULL;
    session_app_id = (char *)malloc(MAX_APP_LENGTH);
    if (session_app_id == NULL)
    {
          printf("sessoion_app_id: malloc_failure in main\n");
          exit(-1);
    }

    app_id = (char *)malloc(MAX_APP_LENGTH);
    if (app_id == NULL)
    {
          printf("app_id: malloc_failure in main\n");
          exit(-1);
    }

    St = (char *)malloc(1024);
        if (St == NULL)
        {
            printf("app_id1: malloc_failure in main\n");
            exit(-1);
        }

    /*
      * Read the file and load all the parameters in a list.
      * Calculate nu_1
    */
    while (fgets(line, MAX_LINE_LENGTH, stream))
    {
        char* tmp = strdup(line);

        if ((strlen(line)==0) || (strstr(line, "#")==NULL)) // Skip if it's comment or empty line
        {
        	strcpy(session_app_id, getfield(tmp, _SESSION_APP_ID));tmp = strdup(line);
        	strcpy(app_id, getfield(tmp, _APP_ID));tmp = strdup(line);
        	w = 	atof(getfield(tmp, _W));tmp = strdup(line);
        	chi_0 = atof(getfield(tmp, _CHI_0));tmp = strdup(line);
        	chi_C = atof(getfield(tmp, _CHI_C));tmp = strdup(line);
        	M = 	atof(getfield(tmp, _M));tmp = strdup(line);
        	m = 	atof(getfield(tmp, _m));tmp = strdup(line);
        	V = 	atof(getfield(tmp, _V));tmp = strdup(line);
        	v = 	atof(getfield(tmp, _v));tmp = strdup(line);
        	D = 	atoi(getfield(tmp, _D));tmp = strdup(line);
        	strcpy(St, getfield(tmp, _St));tmp = strdup(line);
        	DatasetSize = 	atoi(getfield(tmp, _Dsz));
        	csi = getCsi(M/m, V/v);
        	/* Add application parameters to the List */
        	addParameters(&first, &current, session_app_id, app_id, w, chi_0, chi_C, m, M, V, v, D, csi, St, DatasetSize);

        	rows++;
        	free(tmp);
        }

    }




    /*
     * For each application:
     * -	Calculate nu_i (for other application different than the first),
     * -	Store each value in a db table
     * -	Find the bounds
     */
    calculate_Nu(conn, argv[1], first, N);
/*
    while (first!=NULL)
    {
    	printf("%s w(%d) nu(%lf) %d\n", first->app_id, first->w, first->nu_d, first->currentCores_d);
    	first=first->next;
    }
*/

    gettimeofday(&tv_final_nu, NULL);

    /* Calculate baseFO for erach application */
    initialize(conn, first);

    gettimeofday(&tv_initial_fix, NULL);

    sListPointers *firstPointer = fixInitialSolution(first, N);
    gettimeofday(&tv_final_fix, NULL);

    /* Invoke localSearch */
    gettimeofday(&tv_initial_locals, NULL);
    localSearch(conn, first, N, MAX_PROMISING_CONFIGURATIONS);
    gettimeofday(&tv_final_locals, NULL);

    printf("Final solution\n");
    readList(first);

    /* De-allocate resources and close connection */
    fclose(stream);
    //
    freeParametersList(first);
    freeApplicationList(firstPointer);
    free(app_id);
    DBclose(conn);

    gettimeofday(&tv_final_main, NULL);

    //printOutput(first);
    printf("FixInitial step elapsed time: %lf\n", elapsedTime(tv_initial_fix, tv_final_fix));
    printf("FIndbounds (including Nu computation) elapsed time: %lf\n", elapsedTime(tv_initial_nu, tv_final_nu));
    printf("LocalSearch step elapsed time: %lf\n", elapsedTime(tv_initial_locals, tv_final_locals));
    printf("Overall elapsed time: %lf\n", elapsedTime(tv_initial_main, tv_final_main));



    // This return code is tested by the caller
    // Any value different than 0 will fire an exception
    return 0;

}




