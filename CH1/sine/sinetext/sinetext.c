/* generate sine audio data in text format */
/* usage: sinetext > output.txt */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifndef M_PI
#define M_PI (3.141592654)
#endif

int main (int argc, char**argv)
{
	int i, nsamples=50; // generate 50 samples 
	double sample;
	double twopi = 2*M_PI;
	double theta = twopi/nsamples;	

	for (i=0; i<nsamples;	i++)
	{
		sample = sin(theta*i);
		fprintf(stdout,"%.8lf\n",sample);
	}	
	fprintf(stderr,"done\n");
	return 0;
}

