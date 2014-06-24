/* generate exponential attack or decay breakpoint data */
/* usage: expbrk duration npoints startval endval */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char**argv)
{
	int i, npoints; // number of breakpoints
	double startval, endval; // start and end values of attack/decay
	double dur, // duration of attack/decay 
	       start, end, step, thisstep;
	double fac, valrange, offset;
	const double verysmall = 1.0e-4;

	/* check user input */
	if (argc!=5)
	{
		fprintf(stderr,"USAGE: expbrk duration npoints startval endval\n");
		return 1;		
	}	
	dur = atof(argv[1]);
	if (dur <= 0.0)
	{
		fprintf(stderr,"ERROR: duration must be positive!\n");
		return 1;
	}
	npoints = atoi(argv[2]);
	if (npoints <= 0)
	{
		fprintf(stderr,"ERROR: npoints must be positive!\n"); 
		return 1;
	}	
	step = dur/npoints;
	
	startval = atof(argv[3]);
	endval = atof(argv[4]);
	valrange = endval - startval;	

	if (valrange == 0.0)
		fprintf(stderr,"Warning: start and end values are the same.\n");

	if (startval>endval)
	{
		start = 1.0;
		end = verysmall;
		offset = endval;
		valrange = -valrange;
	}		
	else
	{
		start = verysmall;
		end = 1.0;
		offset = startval;
	}

	thisstep = 0.0;

	fac = pow(end/start,1.0/npoints);

	for (i=0; i<npoints; i++)
	{
		fprintf(stdout,"%0.4lf\t%0.8lf\n",thisstep,offset+(start*valrange));
		start*=fac;
		thisstep+=step;
	}

	fprintf(stderr,"Done.\n");
	return 0;
}
