/* generate breakpoint data using an exponential decay envelope */
/* usage: expdecay dur T steps > outfile */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define ARG_NARGS 4

int main(int argc, char **argv)
{
	int i,nsteps; // number of breakpoints 
	double step, x, a, k, 
	       T; // decay constant, affects the rate of decay
	double dur; // duration of decay

	if(argc != ARG_NARGS)
	{
		printf("usage: expdecay dur T steps > outfile.txt\n");
		return 1;
	}

	dur = atof(argv[1]);
	T = atof(argv[2]);
	nsteps = atoi(argv[3]);

	k = dur/nsteps;
	a = exp(-k/T);
	x = 1.0; // decay value starts at one, decays to zero	

	step = 0.0;
	for (i=0; i<nsteps; i++)
	{
		printf("%.4lf\t%.8lf\n",step,x);
		x*=a;
		step+=k;
	}
	return 0;
}
