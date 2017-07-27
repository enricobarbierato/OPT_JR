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





int main(int argc, char **argv)
{

    double w, w1;
    double chi_0;
    double chi_C, chi_c_1;
    double m;
    double M;
    double V;
    double v;
    double D;
    double csi, csi_1;
    double nu_1;
    char * app_id;
    char * app_id1;
    char * St;
    int DatasetSize;

    double N;

    char line[1024];

    int rows = 1;
    double tot = 0;


    /*
     * Check Usage
     */
    if (argc < 3) Usage();


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
    app_id = (char *)malloc(MAX_APP_LENGTH);
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
        	strcpy(app_id, getfield(tmp, _APP_ID));tmp = strdup(line);
        	if (rows > 1) /* Any other application than the first */
        	{
        		w = 	atof(getfield(tmp, _W));tmp = strdup(line);
        		chi_0 = atof(getfield(tmp, _CHI_0));tmp = strdup(line);
        		chi_C = atof(getfield(tmp, _CHI_C));tmp = strdup(line);
        	}
        	else /* First application */
        	{
        		w1 = 	atof(getfield(tmp, _W));tmp = strdup(line); w=w1;
        		chi_0 = atof(getfield(tmp, _CHI_0));tmp = strdup(line);
        		chi_c_1 = atof(getfield(tmp, _CHI_C));tmp = strdup(line);
        	}

        	/* Parameters common for all the applications */
        	M = 	atof(getfield(tmp, _M));tmp = strdup(line);
        	m = 	atof(getfield(tmp, _m));tmp = strdup(line);
        	V = 	atof(getfield(tmp, _V));tmp = strdup(line);
        	v = 	atof(getfield(tmp, _v));tmp = strdup(line);

        	if (rows > 1) /* Any other application than the first */
        	{
        			csi = getCsi(M/m, V/v);
        			tot = tot + sqrt((w/w1)*(chi_C/chi_c_1)*(csi_1/csi));
        	}
        	else csi_1 = getCsi(M/m, V/v); /* First application */

        	D = 	atoi(getfield(tmp, _D));tmp = strdup(line);
        	strcpy(St, getfield(tmp, _St));tmp = strdup(line);
        	DatasetSize = 	atoi(getfield(tmp, _Dsz));
        	//printf("app_id %s w %lf w1 %lf chi_0 %lf chi_C %lf chi_c_1 %lf\n m %lf M %lf V %lf v %lf D %lf csi  %lf csi_1 %lf\n",
        	  //      app_id,   w,    w1,    chi_0,    chi_C,    chi_c_1,    m,    M,    V,    v,    D,    csi,     csi_1);

        	/* Add application parameters to the List */
        	addParameters(rows, &first, &current, app_id, w, chi_0, chi_C, chi_c_1, m, M, V, v, D, csi, csi_1, St, DatasetSize);
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


        /* Calculate in advance nu_1 (first application) */
     nu_1 = N/(1 + tot);

    /*
     * For each eapplication:
     * -	Calculate nu_i (for other application different than the first),
     * -	Store each value in a db table
     * -	Find the bound
     */
    process(conn, argv[1], first, nu_1, w1, csi_1, chi_c_1);


    /* Invoke localSearch */
    localSearch(first, N);

    readList(first);

    /* De-allocate resources and close copnnection */
    fclose(stream);
    freeList(first);
    free(app_id);
    DBclose(conn);

    // This return code is tested by the caller
    // Any value different than 0 will fire an exception
    return 0;

}




