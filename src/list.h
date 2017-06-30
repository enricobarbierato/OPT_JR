/*
## Copyright 2017 Enrico Barbierato  <enrico.barbierato@polimi.it>
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.
*/


#ifndef LIST_H_
#define LIST_H_


struct Result
{
	int index;
	char * app_id;
    double nu;
	struct Result *next;
};
typedef struct Result sResult;



struct List
{
	char * app_id;
    double w, w1;
    double chi_0;
    double chi_C, chi_c_1;
    double m;
    double M;
    double V;
    double v;
    double D;
    double csi, csi_1;
	struct List *next;
};
typedef struct List sList;
// The element corresponding to the event


/*
 * Function templates
 */

void readList(sList *);
void searchResult(sResult *, char *);
void printRow(sList *);
void freeResultList(sResult * );
void addParameters(sList ** , sList ** ,  char *, double , double , double , double , double , double , double , double , double , double , double , double  );
sList * returnARow(sList **  );
void readResult(sResult *);

void addResult(sResult ** , sResult ** ,  int , double, char * );

#endif /* LIST_H_ */
