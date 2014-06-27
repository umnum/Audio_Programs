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

/* scan an array of breakpoint values and check if they are within the inputed range
   return 1 if in range, return 0 if out of range */
int inrange(const BREAKPOINT* points, double minval, double maxval, unsigned long npoints); 

/* find a value for a given time using an array of breakpoints */
double val_at_brktime(const BREAKPOINT* points, unsigned long npoints, double time);
