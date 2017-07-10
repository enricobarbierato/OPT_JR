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


/*
 * 		Name:					readFolder
 * 		Input parameters:		A path to a folder
 * 		Output parameters:		The name of subfolder contained in the folder corresponding to the folder in "path"
 * 		Description:			This function returns, the first subFolder in the folder corresponding to "path"
 *
 */
char * readFolder(char *  path)
{
	 struct dirent *de;


	    DIR *dr = opendir(path);

	    if (dr == NULL)
	    {
	        printf("readFolder failure" );
	        exit(-1);
	    }


	    while ((de = readdir(dr)) != NULL)
	    {
	    	if (de->d_type == 4) // folder
	    	{
	    		if (
	    				strcmp(de->d_name, ".") != 0 &&
						strcmp(de->d_name, "..")

	    			)

	    		return de->d_name;
	    	}
	    }

	    closedir(dr);
	    return NULL;
}

/*
 * 		Name:					LundstromPredictor
 * 		Input parameters:		int nValue, char * appId
 * 		Output parameters:		The output from Lundstrom predictor
 * 		Description:			This function invokes Lundstrom when, instead of giving the tuple (nNodes, nCores, Memory, Datasize) the tuple (nNodes*nCores, Memory, Datasize) is provided
 * 								Currently is not used
 *
 */
char * LundstromPredictor(int nValue, char * appId)
{

struct Best best;


/* Get best configuration */
	    best = bestMatch(
	    		parseConfigurationFile("RESULTS_HOME", 1),
	    		nValue);

	    char cmd[1024];
	    char parameters[64];
	    char _nNodes[8];
	    char _nCores[8];

	    sprintf(_nNodes, "%d", best.nNodes);
	    sprintf(_nCores, "%d", best.nCores);

	    strcpy(parameters, _nNodes);
	    strcat(parameters, " ");

		strcat(parameters, _nCores);
		strcat(parameters, " ");

		strcat(parameters, best.datasize);
		strcat(parameters, " ");

		strcat(parameters, best.method);
		strcat(parameters, " ");
		strcat(parameters, appId);

	    strcpy(cmd, "cd ");
	    strcat(cmd, parseConfigurationFile("LUNDSTROM_HOME", 1));
	    strcat(cmd, ";");
	    strcat (cmd, "python run.py ");
	    strcat(cmd, parameters);
	    printf("cmd = %s\n", cmd);

	    return _run(cmd);

}






void Usage()
{
    	printf("Usage:\n");
    	printf("./optimize <csv_filename> <N>\n");
    	exit(-1);
    }



/*
 * 		Name:					extractWord
 * 		Input parameters:		char * source, int position
 * 		Output parameters:		A word
 * 		Description:			This function extracts a word in a line at a certain position (1st word, 2nd word, etc.). The words are separated by a space
 * 								Note: This function is possibly redundant. Currently it is used to extract information from a folder such as 2_4_8G_500
 *
 */

char * extractWord(char * source, int position)
{

	char *dest = malloc(16);
	int  i = 0, j=0;
	int cont;
	char sep;


	sep = '_';
	for (cont=1; cont<= position; cont++)
	{
		while (source[i] != sep && i < strlen(source))
		{
			if (cont == position) dest[j++] = source[i];
			i++;
		}
		i++;
	}

	dest[j] = '\0';

	return dest;

}


/*
 * 		Name:					getfield
 * 		Input parameters:		char * source, int num
 * 		Output parameters:		A word
 * 		Description:			it extracts values from the csv file
 *
 */
char * getfield(char* line, int num)
{



    char* tok;
    if ((num < 1) || (num > PARAMETERS)) printf("getfield: num %d out of bound\n", num);
    	else for (tok = strtok( line, ","); tok && *tok; tok = strtok(NULL, ",\n"))
    		if (!--num) return tok;

    return NULL;
}


/*
 * 		Name:					doubleCompare
 * 		Input parameters:		double a, double b
 * 		Output parameters:		0  if a = b
 * 								-1 if a < b
 * 								1  if a > b
 * 		Description:			Compare two doubles according to a certain precision (epsilon)
 *
 */

double doubleCompare(double a, double b)
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


/*
 * 		Name:					max
 * 		Input parameters:		double a, double b
 * 		Output parameters:		a double
 * 		Description:			It takes the max between two doubles
 *
 */
double max(double a, double b)
{
	if (doubleCompare(a,b) == 1) return(a);
	else return b;
}


/*
 * 		Name:					getCsi
 * 		Input parameters:		double a, double b
 * 		Output parameters:		a double
 * 		Description:			It takes the max between two doubles
 * 		NOTE: This function is possibly redundant
 *
 */
double getCsi(double a, double b)
{
	 if (doubleCompare(a, b) == -1) return a;
	 else return b;
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


/*
 * 		Name:					bestMatch
 * 		Input parameters:		char * path, int nValue
 * 		Output parameters:		A best structure including the best match
 * 		Description:			Given nNodes * nCores, it provides the best match in the data log repository
 * 		NOTE: This function is not optimized - no need to use a struct. Needs some fixing
 * 		NOTE: This function is not currently used
 *
 */
struct Best bestMatch(char * path, int nValue)
{
	int dir_count = 0;
	    struct dirent* dent;
	    DIR* srcdir = opendir(path);
	    int nNodes, nCores;
	    int min = INT_MAX;
	    int diff;
	    int savenCores;
	    int savenNodes;
	    char datasize[16];
	    char method[16];

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
	        	//diff = abs(nValue - nNodes * nCores);
	        	diff = abs(nValue - nCores);
	        	if (min > diff)
	        	{
	        		min = diff;
	        		savenNodes = nNodes;
	        		savenCores = nCores;
	        		strcpy(datasize, extractWord(dent->d_name, 3));
	        		strcpy(method, extractWord(dent->d_name, 4));
	        	}
	        	dir_count++;
	        }
	    }
	    closedir(srcdir);

	    best.nNodes = savenNodes;
	    best.nCores = savenCores;
	    strcpy(best.datasize, datasize);
	    strcpy(best.method, method);


        return best;
}


/*
 * 		Name:					_run
 * 		Input parameters:		char * cmd
 * 		Output parameters:		The output provided by the executed command
 * 		Description:			This function executes a command ("cmd")
 *
 */
char * _run(char * cmd)
{
	FILE *fp;
	  char *path = (char *)malloc(1024);

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

	  return path;
}




