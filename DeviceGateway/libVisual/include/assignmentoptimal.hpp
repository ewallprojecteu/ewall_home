#ifndef ASSIGNMENTOPTIMAL
#define ASSIGNMENTOPTIMAL

#include <iostream> //for std::cerr, std::endl
#include <cstdlib> //for malloc(), free()

#define MAXABS_64F ( 1.7976931348623158e+308 )

#define CHECK_FOR_INF
// #define ONE_INDEXING

void assignmentoptimal(int *assignment, double *cost, double *distMatrix, int nOfRows, int nOfColumns);

#endif /*ASSIGNMENTOPTIMAL*/
