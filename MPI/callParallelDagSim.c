#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <stdlib.h>

const int MAX_STRING = 1000;

void callDagSim(int, int, char * ); 
char * parseConfigurationFile(char *, int );

int main(int argc, char *argv[]) {
	int comm_sz; /* Number of processes */
        int my_rank; /*My process rank */

        MPI_Init(NULL, NULL);
        MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	callDagSim(comm_sz, my_rank, argv[1]);

	MPI_Finalize();	
	
	return 0; 
}

void callDagSim(int comm_sz, int my_rank, char *file){

	char outMsg[MAX_STRING];
	char cmd[MAX_STRING];


        if (my_rank != 0) {
		sprintf(file, "%s1.lua", file);
		sprintf(cmd,"%s/dagsim.sh %s > /tmp/output1",parseConfigurationFile("DAGSIM_HOME", 1), file);
		int status = system(cmd);
		sprintf(outMsg, "dagSim process end %d of %d!",
                        my_rank, comm_sz);

                MPI_Send(outMsg, strlen(outMsg)+1, MPI_CHAR, 0, 0,
                         MPI_COMM_WORLD);
        }
        else {
                printf("Process 0 run \n");
                printf("Running dagSim process %d of %d!\n", my_rank,
                        comm_sz);
		sprintf(file, "%s0.lua", file);
		sprintf(cmd,"%s/dagsim.sh %s > /tmp/output0",parseConfigurationFile("DAGSIM_HOME", 1), file);
		int status = system(cmd);
               

                for (int q = 1; q < comm_sz; q++) {
                        MPI_Recv(outMsg, MAX_STRING, MPI_CHAR, q, 0,
                                MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        printf("%s\n", outMsg);
                }
        }



}

/*
 * 		Name:					parseConfigurationFile
 * 		Input parameters:		char *variable, int xml
 * 		Output parameters:		The value (char *) of a variable
 * 		Description:			It fetches the value of a variable in wsi_config.xml file
 *
 */
char * parseConfigurationFile(char *variable, int xml)
{
FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int found = 0;
    char * configurationFile = malloc(64);
    char * newString;

    strcpy(configurationFile, getenv("HOME"));
    if (xml == 0) strcat(configurationFile, "/.ws_properties");
    else
    	{
    		configurationFile = getenv("WSI_CONFIG_FILE");

    		if (configurationFile == NULL)
    		{
    			printf("Fatal error: WSI_CONFIG_FILE environment variable was not defined.\n");
    			exit(-1);
    		}

    	}

    fp = fopen(configurationFile , "r");
    if (fp == NULL)
    {
    	if (xml == 0) printf(".ws_properties "); else printf("wsi_config.xml");
    	printf(" configuration file not found in home directory: (%s)\n", configurationFile);
    	free(configurationFile);
        exit(-1);
    }

    while ((read = getline(&line, &len, fp)) != -1)
    {
    	if (strstr(line, variable) != NULL)
    	{
    		found = 1;
    		break;
    	}
    }

    if (!found)
    {
    	printf("Could not find %s environment variable.", variable);
    	exit(-2);
    }


    	int len1 = strlen(line);

    	/*
    	 * Remove \n from the string
    	 */
    	char *newLine = malloc(1024);
    	strncpy(newLine, line, strlen(line)-1);
    	len1 = strlen(newLine);

    	fclose(fp);
    	if (line) free(line);

    	if (xml == 0) newString = strstr(newLine, "=");
    	else
    		{
    		newString = strstr(newLine, ">") + 1;
    		int pos = strstr(newString, "<") - newString -1;
    		newString[pos+1] = '\0';
    		return newString;
    		}

    return(newString+1);
}

