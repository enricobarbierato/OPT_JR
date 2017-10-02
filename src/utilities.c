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



#include "db.h"
#include "utilities.h"





void printOPT_JRPars(struct optJrParameters par )

{




	printf("N: %d\n", par.number);

	printf("Filename: %s\n", par.filename);

	printf("Candidates List Limit: %d\n", par.K);

	printf("DEBUG option: %d\n", par.debug);



	printf("Simulator: %d\n", par.simulator);

	printf("Global FO calculation: %d\n", par.globalFOcalculation);





}



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

	        printf("readFolder failure (%s)", path );

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

char * LundstromPredictor(sConfiguration *configuration, int nValue, char * appId, struct optJrParameters par)

{



struct Best best;





/* Get best configuration */

	    best = bestMatch(

	    		getConfigurationValue(configuration,"RESULTS_HOME"),

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
	    strcat(cmd, getConfigurationValue(configuration,"LUNDSTROM_HOME"));
	    strcat(cmd, ";");
	    strcat (cmd, "python run.py ");
	    strcat(cmd, parameters);
	    printf("cmd = %s\n", cmd);
	    return _run(cmd, par);

}


void Usage()
{

    	printf("Usage:\n");
    	printf("./optimize -f <filename.csv> -n <N> -k <Limit> -d <dY/y|N/n> -c <Y/y|N/n> -s <dagSim|lundstrom> -i <iterations>\n");
    	printf("where:\n");
    	printf("<filename.csv> is the csv file (including the input values) under $UPLOAD_HOME in wsi_config.xml;\n");
    	printf("<N> is the total number of cores;\n");
    	printf("<Limit> is the maximum number of considered candidates (if equal to 0, all the candidates are considered).\n");
    	printf("-d represents debug (on/off)");
    	printf("-i represents the maximum number of iterations");
    	printf("Example:\n");
    	printf("./OPT_JR -f=\"Test3.csv\" -n=220 -k=0 -d=Y -s=dagSim -i=10");
    	printf("/n OPT_JR is executed on a file Test3.csv, the total number of cores is 220, all the candidates are considered, the predictor used is dagSim, "
    			"the maximum number of iterations is 10\n");
    	exit(-1);
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





double elapsedTime(struct timeval  tv1, struct timeval tv2)
{
	return (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
	                     (double) (tv2.tv_sec - tv1.tv_sec);
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

int doubleCompare(double a, double b)

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

	 if (doubleCompare(a, b) == -1) return b;

	 else return a;

}







char * extractItem(const char *const string, const char *const left, const char *const right)

{

    char  *head;

	    char  *tail;

	    size_t length;

	    char  *result;



	    if ((string == NULL) || (left == NULL) || (right == NULL))

	        return NULL;

	    length = strlen(left);

	    head   = strstr(string, left);

	    if (head == NULL)

	        return NULL;

	    head += length;

	    tail  = strstr(head, right);

	    if (tail == NULL)

	        return tail;

	    length = tail - head;

	    result = malloc(1 + length);

	    if (result == NULL)

	        return NULL;

	    result[length] = '\0';



	    memcpy(result, head, length);

	    return result;









}











/*

 * 		Name:					bestMatch

 * 		Input parameters:		char * path, int nValue

 * 		Output parameters:		The best structure including the best match

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

char *_run(char * cmd, struct optJrParameters par)

{

	int BUFSIZE = 10240;
	char debugMsg[DEBUG_MSG];
	int outcome;



	char *buf = (char *)malloc(BUFSIZE);

	if (buf == NULL)

	{

		printf("Malloc failure: _run\n");

		exit(-1);

	}

	    FILE *fp;





	    if ((fp = popen(cmd, "r")) == NULL) {

	        printf("Fatal Error: _run: %s _cmd %s (%d)\n", strerror(errno), cmd, errno);

	        exit(-1);

	    }



	    while (fgets(buf, BUFSIZE, fp) != NULL) {



	        printf("OUTPUT: %s", buf);



	    }

	    outcome = pclose(fp);

	    if(outcome == -1)  {

	        printf("Fatal error: Command %s not found or exited with error status\n", cmd);
	        exit(-1);

	    } else sprintf(debugMsg, "_run has returned %d status\n", outcome);debugMessage(debugMsg, par);



	  return buf;

}







char * ls(char * pattern, struct optJrParameters par)

{
	char debugMsg[DEBUG_MSG];
	glob_t pglob;

	char *filename = (char *)malloc(1024);



	 int outcome = glob(pattern, GLOB_ERR, NULL, &pglob);



	 if (pglob.gl_pathc == 1)

	 	  {

	 		  sprintf(debugMsg, "ls: %s\n", pglob.gl_pathv[0]);debugMessage(debugMsg, par);

	 		  strcpy(filename, pglob.gl_pathv[0]);

	 		  globfree(&pglob);

	 		  return filename;

	 	  }

	 	  else

	 	  {

	 		  switch(outcome)

	 		  {

	 		  	  case GLOB_NOSPACE:

	               printf("Fatal error: ls: running out of memory: %s\n", pattern);

	               exit(-1);

	 	 	 	 	 break;

	 		  	  case GLOB_ABORTED:

	               printf("Fatal error: ls: a read error has occurred%s\n", pattern);

	               exit(-1);

	               break;

	 		  	  case GLOB_NOMATCH:

	               printf("Fatal error: ls: no matches found%s\n", pattern);

	               exit(-1);

	 		  	  default:

	    	  	   printf("Fatal error: ls: unknown error%s\n", pattern);

	    	  	   exit(-1);

	 		  }

	 	  }







	  return NULL;

}


char * extractRowMatchingPattern(char *text, char *pattern)
{

	char * line = (char *)malloc(1024);

	if (line == NULL)
	{
		printf("Malloc failure: extractRowMatchingPattern\n");
		exit(-1);
	}

	line = strstr(text, pattern);
	if (line == NULL)
	{
		printf("Fatal error: extractRowMatchingPattern: pattern %s was not found in predictor output file\n", pattern);
		exit(-1);
	}
	line = line + strlen(pattern) + 1;

	return line;
}


char * extractRowN(char *text, int row)
{
	int len = strlen(text);
	int iText, iLine = 0;
	char * line = (char *)malloc(BIG_LINE);

	if (line == NULL)
	{
		printf("Malloc failure: edxtractRowN\n");
		exit(-1);
	}

	int countRow = 0;

	iText = 0;

	strcpy(line, "");

	while ( countRow < row && iText < len)
	{
		iLine = 0;
		while(text[iText] != '\n' &&
				iText < strlen(text))
		{
			line[iLine++] = text[iText++];
		}
		countRow++;
		iText++;
	}

	if (row > countRow) return "stop";
	line[iLine] = '\0';

	if (line == NULL)
	{
		printf("Fatal error: extractRowN: returned string cannot be NULL\n");
		exit(-1);
	}
	return line;
}



char * extractWord(char * line, int pos)
{

	char *word = (char *)malloc(64);

	if (word == NULL)
	{
		printf("Malloc failure: extractWord\n");
		exit(-1);
	}

	int lineIndex = 0;
	int wordIndex = 0;
	int len = strlen(line);
	int countwords = 0;

	while (lineIndex <= len)
	{
		if (line[lineIndex] != '\t') word[wordIndex++] = line[lineIndex++];
		else
		{
			countwords++;
			if (countwords == pos)
			{
				word[wordIndex] = '\0';
				break;
			}
			wordIndex = 0;
			lineIndex++;
		}
	}

	if (word == NULL)
	{
		printf("Fatal error: extracWord: returned string is NULL\n");
		exit(-1);
	}
	return word;
}







void writeFile(const char *filepath, const char *data)

{

    FILE *fp = fopen(filepath, "w");

    if (fp != NULL)

    {

        fputs(data, fp);

        fclose(fp);

    }

}



char * replace(char * text, char *newLine)

{



	int lineCount;

	char line[BIG_LINE];

	char *newText = (char *)malloc(BIG_TEXT);

	if (newText == NULL)

	{

		printf("Malloc failure: replace\n");

		exit(-1);

	}



	lineCount = 1;



	strcpy(newText, "");

	strcpy(line, extractRowN(text, lineCount));

	while ( strcmp(line, "stop") != 0)

	{



		if (strstr(line, "Nodes") != NULL) strcat(newText, newLine);

		else strcat(newText, line);

		strcat(newText, "\n");

		lineCount++;

		strcpy(line, extractRowN(text, lineCount));

	}

	return newText;

}



struct optJrParameters parseCommandLine(char **args, int argc)

{



	if (argc != ARGS) Usage();



	struct optJrParameters par;



	for (int i = 0; i < ARGS; i++)
		if (strstr(args[i], FILENAME)) strcpy(par.filename, parseArg(args[i], FILENAME, STRING));

		else if (strstr(args[i], NUM_N)) par.number = atoi(parseArg(args[i], NUM_N, NUMBER));

				else if (strstr(args[i], LIST_LIMIT)) par.K = atoi(parseArg(args[i], LIST_LIMIT, NUMBER));

					else if (strstr(args[i], DEBUG)) par.debug = atoi(parseArg(args[i], DEBUG, YES_NO));


							else if (strstr(args[i], SIMULATOR))
							{
									if (strcmp(parseArg(args[i], FILENAME, STRING), "dagSim") == 0) par.simulator = DAGSIM;
															else par.simulator = LUNDSTROM;
							}

								else if (strstr(args[i], GLOBAL_FO_CALCULATION)) par.globalFOcalculation = atoi(parseArg(args[i], GLOBAL_FO_CALCULATION, YES_NO));
								else if (strstr(args[i], MAX_ITERATIONS))
									par.maxIterations = atoi(parseArg(args[i], MAX_ITERATIONS, NUMBER));


	return par;



}



char * parseArg(char * string, char * gap, int type)

{

	switch(type)

	{

		case NUMBER:

		case STRING:

			return string + strlen(gap);

			break;

		case YES_NO:

			if (strstr(string + strlen(gap), "y") || strstr(string + strlen(gap), "Y")) return "1";

			else if (strstr(string + strlen(gap), "n") || strstr(string + strlen(gap), "N")) return "0";

				else Usage();

			break;

		default:

			printf("Fatal Error: parseArg: option not recognized\n");

			exit(-1);

	}



	return NULL;



}


void debugBanner(char * string, struct optJrParameters par)
{
	if (par.debug == YES)
	printf("*************** %s *************** \n", string);
}

void debugMessage(char * string, struct optJrParameters par)
{

	if (par.debug == YES)
	printf("<debug message>: %s\n", string);

}


void debugInformational(char * string, struct optJrParameters par)
{
	if (par.debug == YES)
		printf("<debug information >: %s\n", string);
}



int read_line(FILE *in, char *buffer, size_t max)
{
  return fgets(buffer, max, in) == buffer;
}



char * readFile(char * filename)
{
	   struct stat sb;

	   stat(filename, &sb);

	   FILE *fp=fopen(filename, "r");

	   if (fp == NULL)
	   {
		   printf("Fatal error: readFile: could not open %s\n", filename);
		   exit(-1);
	   }

	   char *str=malloc(sb.st_size+1);

	   if (str == NULL)

	   {

		   printf("Malloc failure: readFile\n");

		   exit(-1);

	   }

	   fread(str, 1, sb.st_size, fp);



	   fclose(fp);

	   str[sb.st_size]=0;



	   return str;

}


