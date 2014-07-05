#include <stdio.h>


/***** BREAKPOINT type definition *****/

typedef struct breakpoint
{
	double time;
	double value;
}	BREAKPOINT;


/***** BRKSTREAM type definition *****/

typedef struct breakpoint_stream
{
	BREAKPOINT* points;
	BREAKPOINT leftpoint, rightpoint;
	unsigned long npoints;
	double curpos;
	double incr;
	double width;
	double height;
	unsigned long ileft, iright;
	int more_points;	
} BRKSTREAM;


/***** breakpoint function prototypes *****/

/* gather breakpoint data from a file */
BREAKPOINT *get_breakpoints(FILE *fp, long *psize);

/* return the breakpoint with the largest value */
BREAKPOINT maxpoint(const BREAKPOINT *points, long npoints);

/* return the breakpoint with the smallest value */
BREAKPOINT minpoint(const BREAKPOINT* points, long npoints);

/* scan an array of breakpoint values and check if they are within the inputed range
   return 1 if in range, return 0 if out of range */
int inrange(const BREAKPOINT* points, double minval, double maxval, unsigned long npoints); 

/* find a value for a given time using an array of breakpoints */
double val_at_brktime(const BREAKPOINT* points, unsigned long npoints, unsigned long *pointnum, double time);

/* create a stream of breakpoint values from a breakpoint file */ 
BRKSTREAM* bps_newstream(FILE* fp, unsigned long srate, unsigned long* size);

/* frees any internal memory in the breakpoint stream */
void bps_freepoints(BRKSTREAM* stream);

/* grab the current value from the breakpoint stream */
double bps_tick(BRKSTREAM* stream);

/* get the minimum and maximum value in a breakpoint stream
   return 0 for success, return 1 for faliure */
int bps_getminmax(BRKSTREAM* stream, double* min, double* max);
