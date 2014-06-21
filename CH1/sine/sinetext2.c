/* sinetext2 nsamps freq srate */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifndef M_PI
#define M_PI (3.141592654)
#endif

int main(int argc, char**argv)
{
	int i, nsamps, srate;
	double samp, freq;
	double twopi = 2.0*M_PI;
	double theta;

	enum {ARG_NAME, ARG_NSAMPS, ARG_FREQ, ARG_SR, ARG_NARGS};	
	
	if(argc!=ARG_NARGS)
	{
		fprintf(stderr,"ERROR: insufficient number of arguments.\n");
		fprintf(stderr,"USAGE: sinetext2 nsamps freq srate\n");
		return 1;
	}
	
	nsamps = atoi(argv[ARG_NSAMPS]);
	if(nsamps<=2)
	{
		fprintf(stderr,"ERROR: insufficient number of samples.\n");
		return 1;
	}	

	freq = atof(argv[ARG_FREQ]);
	if(freq<=0.0)
	{
		fprintf(stderr,"ERROR: frequency must be positive.\n");
		return 1;	
	}
	
	srate = atoi(argv[ARG_SR]);
  if(srate<44100)
	{
		fprintf(stderr,"ERROR: sample rate must be at least 44.1k.\n");
		return 1;
	}
  	
	theta = twopi*freq/srate;	
	
	for (i=0; i<nsamps; i++)
	{
		samp = sin(theta*i);
		fprintf(stdout,"%.8lf\n",samp);	
	}
	
	fprintf(stderr,"done\n");
	return 0;
}
