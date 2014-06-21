/* tfork outfile.txt dur freq srate slope */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifndef M_PI
#define M_Pi (3.141592654)
#endif

int main (int argc, char**argv)
{
	int	i, nsamps, srate;
	double samp, freq, slope, dur;
	double k, x, a;
	double twopi = 2*M_PI;
	double theta;
	double maxsamp = 0.0;

	enum {ARG_NAME, ARG_OUTFILE, ARG_DUR, ARG_FREQ, ARG_SR, ARG_SLOPE, ARG_NARGS}; 
	FILE *fp = NULL;
	
	if (argc!=ARG_NARGS)
	{
		fprintf(stderr,"ERROR: insufficient number of arguments.\n");
		fprintf(stderr,"USAGE: tfork outfile.txt dur freq srate slope\n");
		return 1;
	} 
	
	dur = atof(argv[ARG_DUR]);
	if (dur<=0.0)
	{
		fprintf(stderr,"ERROR: duration must be positive.\n");
		return 1;
	}	

	freq = atof(argv[ARG_FREQ]);
	if (freq<=0.0)
	{
		fprintf(stderr,"ERROR: frequency must be positive.\n");
		return 1;
	}
	
	srate = atoi(argv[ARG_SR]);
	if (srate<44100)
	{
		fprintf(stderr,"ERROR: sample rate must be at least 44.1K\n");
		return 1;
	}

	slope = atof(argv[ARG_SLOPE]);
	if (slope <= 0.0)
	{
		fprintf(stderr,"ERROR: slope must be positive.\n");
		return 1;
	}

	fp = fopen(argv[ARG_OUTFILE],"w"); 
	if (fp==NULL)
	{
		fprintf(stderr,"Error creating output file %s\n",argv[ARG_OUTFILE]);
		return 1;
	}

	nsamps = (int)(dur*srate);
	theta = twopi*freq/srate;

	k = dur/nsamps;
	a = exp(-k/slope);
	x = 1.0;

	for (i=0; i<nsamps; i++)
	{
		samp=sin(theta*i);
		x*=a;
		samp*=x;
		fprintf(fp,"%.8lf\n",samp);
	}	
	fclose(fp);
	fprintf(stderr,"done\n");		
	
	return 0;
}
