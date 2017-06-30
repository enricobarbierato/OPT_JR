/*
 * utilities.c
 *
 *  Created on: 06 apr 2016
 *      Author: Enrico
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include <limits.h>

#include "utilities.h"

void howAmIInvoked(char** argv, int argc)
{
	int i;

	for (i = 0; i < argc; i++)
		printf("%s\n", argv[i]);
}







void Usage()
{
    	printf("Usage:\n");
    	printf("./optimize <csv_filename> <N> nNodes nCores Dataset param app_id\n");
    	printf("./optimize <csv_filename> <N> nNodes*nCores Dataset param app_id\n");
    	printf("./optimize <csv_filename> <N>\n");
    	exit(-1);
    }

char * extractWord(char * source, int position)
{

	char *dest = malloc(16);
	int  i = 0;

	if (position == 2) source = strstr(source, "_") + 1;
	while (source[i] != '_')
	{
		dest[i] = source[i];
		i++;
	}

	dest[i] = '\0';
	return dest;

}

char * getfield(char* line, int num)
{



    char* tok;
    if ((num < 1) || (num > PARAMETERS)) printf("getfield: num %d out of bound\n", num);
    	else for (tok = strtok( line, ","); tok && *tok; tok = strtok(NULL, ",\n"))
    		if (!--num) return tok;

    return NULL;
}


/*
 * doubleCompare
 * Compare two double, returning:
 *
 * 0  if a = b
 * -1 if a < b
 * 1  if a > b
 */
int doubleCompare(double a, double b, double epsilon)
{


	  if (((a - epsilon) < b) &&
	      ((a + epsilon) > b))
	   {
	    return 0;
	   }
	  else
	   {
	    if (a > b) return 1; else return -1;
	   }

}

double getCsi(double a, double b, double epsilon)
{
	 if (doubleCompare(a, b, epsilon) == -1) return a;
	 else return b;
}


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


struct Best bestMatch(char * path, int nNodesInput, int nCoresInput, int modality)
{
	int dir_count = 0;
	    struct dirent* dent;
	    DIR* srcdir = opendir(path);
	    int nNodes, nCores;
	    int min = INT_MAX;
	    int diff;
	    int savenCores;
	    int savenNodes;
	    struct Best best;


	    if (srcdir == NULL)
	    {
	        perror("opendir");
	        exit(-1);
	    }

	    while((dent = readdir(srcdir)) != NULL)
	    {
	        struct stat st;

	        if(strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0)
	            continue;

	        if (fstatat(dirfd(srcdir), dent->d_name, &st, 0) < 0)
	        {
	            perror(dent->d_name);
	            continue;
	        }

	        if (S_ISDIR(st.st_mode))
	        {
	        	nNodes = atoi(extractWord(dent->d_name, 1));
	        	nCores = atoi(extractWord(dent->d_name, 2));
	        	if (modality == COUPLE) diff = abs(nNodesInput - nNodes);
	        	else diff = abs(nNodesInput - nNodes * nCores);
	        	if (min > diff)
	        	{
	        		min = diff;
	        		savenNodes = nNodes;
	        		savenCores = nCores;
	        	}
	        	dir_count++;
	        }
	    }
	    closedir(srcdir);

	    best.nNodes = savenNodes;
	    best.nCores = savenCores;

        return best;
}


int _run(char * cmd)
{
	FILE *fp;
	  char path[1035];

	  /* Open the command for reading. */
	  fp = popen(cmd, "r");
	  if (fp == NULL) {
	    printf("Failed to run command\n" );
	    exit(1);
	  }

	  /* Read the output a line at a time - output it. */
	  while (fgets(path, sizeof(path)-1, fp) != NULL) {
	    printf("%s", path);
	  }

	  /* close */
	  pclose(fp);

	  return 0;
}




