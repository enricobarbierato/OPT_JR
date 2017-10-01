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
    struct optJrParameters par;
    char debugMsg[DEBUG_MSG];

    par = parseCommandLine(argv, argc);

    int rows = 1;

    struct timeval  tv_initial_main,
    				tv_initial_nu,
    				tv_final_nu,
					tv_initial_init,
					tv_final_init,
    				tv_initial_fix,
    				tv_final_fix,
    				tv_initial_locals,
    				tv_final_locals,
    				tv_final_main;


    sConfiguration *configuration = readConfigurationFile();

    /* Connect to the db */
    MYSQL *conn = DBopen(
            			getConfigurationValue(configuration, "OptDB_IP"),
    					getConfigurationValue(configuration, "OptDB_user"),
    					getConfigurationValue(configuration, "OptDB_pass"),
    					getConfigurationValue(configuration, "OptDB_dbName")
    					);
    if (conn == NULL) DBerror(conn, "open_db: Opening the database");

    // Calculate the time taken
    gettimeofday(&tv_initial_main, NULL);


    gettimeofday(&tv_initial_nu, NULL);
    /*
     * Find where the file has been uploaded and determine absolute file path
     */
    char *folder = getConfigurationValue(configuration, "UPLOAD_HOME");
    char *filename = strcat(folder, "/");
    filename = strcat(folder, par.filename);

    /*
     * Read total cores available
     */
    N = par.number;
    int MAX_PROMISING_CONFIGURATIONS = par.K;



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
          printf("session_app_id: malloc_failure in main\n");
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
    calculate_Nu(configuration, conn, first,  par);
/*
    while (first!=NULL)
    {
    	printf("%s w(%d) nu(%lf) %d\n", first->app_id, first->w, first->nu_d, first->currentCores_d);
    	first=first->next;
    }
*/

    gettimeofday(&tv_final_nu, NULL);

    gettimeofday(&tv_initial_init, NULL);
    /* Calculate baseFO for erach application */
    initialize(configuration, conn, first, par);
    gettimeofday(&tv_final_init, NULL);

    gettimeofday(&tv_initial_fix, NULL);

    sListPointers *firstPointer = fixInitialSolution(first, par);
    gettimeofday(&tv_final_fix, NULL);

    /* Invoke localSearch */
    gettimeofday(&tv_initial_locals, NULL);
    localSearch(configuration, conn, first, N, MAX_PROMISING_CONFIGURATIONS, par);
    gettimeofday(&tv_final_locals, NULL);

    debugInformational("Final solution\n", par);
    writeList(conn, getConfigurationValue(configuration,"OptDB_dbName"),first, par);

    /* De-allocate resources and close connection */
    fclose(stream);
    //
    freeParametersList(first);
    freeApplicationList(firstPointer);
    free(app_id);
    DBclose(conn);

    gettimeofday(&tv_final_main, NULL);

    //printOutput(first);
    sprintf(debugMsg, "FixInitial step elapsed time: %lf\n", elapsedTime(tv_initial_fix, tv_final_fix));debugMessage(debugMsg, par);
    sprintf(debugMsg, "Findbounds (including Nu computation) elapsed time: %lf\n", elapsedTime(tv_initial_nu, tv_final_nu));debugMessage(debugMsg, par);
    sprintf(debugMsg, "Initialization elapsed time %lf\n", elapsedTime(tv_initial_nu, tv_final_nu));debugMessage(debugMsg, par);
    sprintf(debugMsg, "LocalSearch step elapsed time: %lf\n", elapsedTime(tv_initial_locals, tv_final_locals));debugMessage(debugMsg, par);
    sprintf(debugMsg, "Overall elapsed time: %lf\n", elapsedTime(tv_initial_main, tv_final_main));debugMessage(debugMsg, par);



    // This return code is tested by the caller
    // Any value different than 0 will fire an exception
    return 0;

}




