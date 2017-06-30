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

#ifndef SRC_COMMON_H_
#define SRC_COMMON_H_

#define PRODUCT 2
#define COUPLE 1

struct Best
{
	int nNodes;
	int nCores;
};


/* Templates */

int doubleCompare(double, double, double);
double getCsi(double , double , double );
char * parseConfigurationFile(char *, int);
struct Best bestMatch(char *, int, int, int);
char * extractWord(char * , int );
int _run(char * );
char * getfield(char* , int);
void Usage();
void howAmIInvoked(char **, int );

#endif /* SRC_COMMON_H_ */
