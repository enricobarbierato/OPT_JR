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
#include "db.h"

#include "list.h"
#include "performance.h"
#define  XML 1
#define  NOXML 0




int main(int argc, char **argv)
{

    double w, w1;
    double chi_0;
    double chi_C, chi_c_1;
    double m;
    double M;
    double V;
    double v;
    double D, D1;
    double csi, csi_1;
    double nu_1;
    char * app_id;
    char * app_id1;
    char * St;
    int DatasetSize;
    int R1;
    int R;
    int bound1;


    double N;
    char line[1024];
    int rows = 1;
    double tot = 0;
    int newTotal = 0;

    //howAmIInvoked(argv, argc);


    if (argc < 3) Usage();


    char *folder = parseConfigurationFile("UPLOAD_HOME", XML);
    char *filename = strcat(folder, "/");
    filename = strcat(folder, argv[1]);

    N = atof(argv[2]);



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
    app_id = (char *)malloc(1024);
    if (app_id == NULL)
    {
          printf("app_id: malloc_failure in main\n");
          exit(-1);
    }
    app_id1 = (char *)malloc(1024);
    if (app_id1 == NULL)
    {
        printf("app_id1: malloc_failure in main\n");
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
        	if (rows > 1)
        	{
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

        		csi = getCsi(M/m, V/v, precision);
         		addParameters(&first, &current, app_id, w, w1, chi_0, chi_C, chi_c_1, m, M, V, v, D, csi, csi_1, St, DatasetSize);
         		//printf("%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf\n", w, w1, chi_0, chi_C, chi_c_1, m, M, V, v, D, csi, csi_1);
        	    tot = tot + sqrt((w/w1)*(chi_C/chi_c_1)*(csi_1/csi));

        	}
        	else
        	{
        		strcpy(app_id1, getfield(tmp, _APP_ID));tmp = strdup(line);
        		w1 = 	atof(getfield(tmp, _W));tmp = strdup(line);
        		chi_0 = atof(getfield(tmp, _CHI_0));tmp = strdup(line);
        		chi_c_1 = atof(getfield(tmp, _CHI_C));tmp = strdup(line);
        		M = 	atof(getfield(tmp, _M));tmp = strdup(line);
        		m = 	atof(getfield(tmp, _m));tmp = strdup(line);
        		V = 	atof(getfield(tmp, _V));tmp = strdup(line);
        		v = 	atof(getfield(tmp, _v));tmp = strdup(line);
        		D1 = 	atoi(getfield(tmp, _D));tmp = strdup(line);
        		csi_1 = getCsi(M/m, V/v, precision);tmp = strdup(line);
        		strcpy(St, getfield(tmp, _St));tmp = strdup(line);
        		DatasetSize = 	atoi(getfield(tmp, _Dsz));

        		//printf("%lf %lf %lf %lf %lf %lf %lf\n", w1, chi_c_1, m, M, V, v, D, csi_1);
        	}

        	rows++;
        	free(tmp);
        }

    }

    /*
      * Connect to the db
    */
        MYSQL *conn = DBopen(
        					parseConfigurationFile("OptDB_IP", XML),
							parseConfigurationFile("OptDB_user", XML),
							parseConfigurationFile("OptDB_pass", XML),
							parseConfigurationFile("OptDB_dbName", XML)
							);
        if (conn == NULL) DBerror(conn, "open_db: Opening the database");

    /*
     * Calculate nu_1
     */
    nu_1 = N/(1 + tot);
    DBinsertrow(conn, argv[1], app_id1, nu_1);


    /*
       	TEST LOCALSEARCH (first application only)
     */
    /* Invoke Lundstrom the first time */
    R = atoi(fakeLundstrom(0, 0, V/v, 1, "8G", DatasetSize, app_id1));

    split(localSearch(0, conn, parseConfigurationFile("OptDB_dbName", XML), app_id, DatasetSize, D), &bound1, &R1);
    newTotal+= ObjFunctionComponent(w1, R1, D1);

    /*
     * Calculate nu_i, store each value in a db table and invoke localsearch for any other application
     */
    rows = 2;

    //sResult *rFirst= NULL, *rCurrent=NULL;
    while ((current=returnARow(&first))!=NULL)
    {
    	strcpy(app_id, current->app_id);
        w = 	current->w;
        chi_0 = current->chi_0;
        chi_C = current->chi_C;
        M = 	current->M;
        m = 	current->m;
        V = 	current->V;
        v = 	current->v;
        D = 	current->D;


        csi = getCsi(M/m, V/v, 1/1000);

        float nu_i = nu_1*sqrt((w/w1)*(chi_C/chi_c_1)*(csi_1/csi));
        //addResult(&rFirst, &rCurrent, rows, nu_1*sqrt((w/w1)*(chi_C/chi_c_1)*(csi_1/csi)), current->app_id);

        DBinsertrow(conn, argv[1], app_id, nu_i);


        /*
           LOCALSEARCH: calculate the bound
        */

        split(localSearch(1, conn, parseConfigurationFile("OptDB_dbName", XML), app_id, DatasetSize, D), &current->bound, &current->R);
        newTotal+= ObjFunctionComponent(current->w, current->R, current->D);
        rows++;
     }

    printf("New ObjFun = %d", newTotal);
    fclose(stream);

    free(current);
    free(app_id);

    DBclose(conn);

    // This return code is tested by the caller
    // Any value different than 0 will fire an exception
    return 0;

}




