#include <stdio.h>


/***** BREAKPOINT type definition *****/

typedef struct breakpoint
{
	double time;
	double value;
}	BREAKPOINT;


/***** breakpoint function prototypes *****/

/* gather breakpoint data from a file */
BREAKPOINT *get_breakpoints(FILE *fp, long *psize);

/* return the breakpoint with the largest value */
BREAKPOINT maxpoint(const BREAKPOINT *points, long npoints);
