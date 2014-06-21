/* tfork2 outfile.txt dur freq srate amp */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifndef M_PI
#define M_PI (3.141592654)
#endif

int main(int argc, char**argv)
{
	int i, nsamps, srate;
	double samp, freq, amp, maxamp, dur;
	double start, end, fac, theta; 
	double twopi = 2*M_PI;
	FILE *fp = NULL;
	enum {ARG_NAME, ARG_OUTFILE, ARG_DUR, ARG_FREQ, ARG_SR, ARG_AMP, ARG_NARGS};
	
	if (argc!=ARG_NARGS)
	{
		fprintf(stderr,"ERROR: insufficient number of arguments.\n");
		fprintf(stderr,"USAGE: tfork2 outfile.txt dur freq srate amp\n");
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

	amp = atof(argv[ARG_AMP]);
	if (amp<0.0)
	{
		fprintf(stderr,"ERROR: amplitude must be a positive.\n");
		return 1;
	}
	if (amp>1.0)
		fprintf(stderr,"Warning: amplitude may be too large.\n");
	
	nsamps = (int)(dur*srate);
	theta = twopi*freq/srate; 

	fp = fopen(argv[ARG_OUTFILE],"w");
	if (fp==NULL)
	{
		fprintf(stderr,"ERROR: unable to open file \"%s\"\n",argv[ARG_OUTFILE]);
		return 1;
	}

	start = 1.0;
	end = 1.0e-4;
	maxamp = 0.0;
	fac = pow(end/start,1.0/nsamps);	

	for (i=0; i<nsamps; i++)
	{
		samp=amp*sin(theta*i);
		samp*=start;
		start*=fac;	
		fprintf(fp,"%.8lf\n",samp);
		if (fabs(samp)>maxamp)
			maxamp=fabs(samp);
	}

	fclose(fp);
	fprintf(stderr,"Done. Maximum sample value = %.8lf\n",maxamp);

	return 0;
}
