/* generate a simple tuning fork, store audio data as a raw file */
/* tforkraw outfile.raw dur freq srate amp isfloat */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifndef M_PI
#define M_PI (3.141592654)
#endif

enum {ARG_NAME, ARG_OUTFILE, ARG_DUR, ARG_HZ, ARG_SR, ARG_AMP, ARG_TYPE, ARG_NARGS};
enum samptype {RAWSAMP_SHORT, RAWSAMP_FLOAT}; 

int byte_order()
{
	int one = 1;
	char* endptr = (char *)&one;
	return (*endptr); 
}

const char* endianness[2] = {"big_endian","little_endian"};

int main(int argc, char**argv)
{
	unsigned int i, nsamps;
	unsigned int maxframe = 0;		
	unsigned int samptype, endian, bitreverse;
	double samp, dur, freq, srate, amp, step; 
	double start, end, fac, maxsamp;
	double twopi = 2*M_PI;
	double theta;
	FILE *fp = NULL;
	short ssamp;
	float fsamp;

	if (argc!=ARG_NARGS)
	{
		fprintf(stderr,"USAGE: tforkraw outfile.raw dur freq srate amp isfloat\n");
		return 1;
	}

	dur = atof(argv[ARG_DUR]);
	if (dur<=0.0)
	{
		fprintf(stderr,"ERROR: duration must be positive.\n");
		return 1;
	}		

	freq = atof(argv[ARG_HZ]);
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
	if (amp>1.0)
		fprintf(stderr,"Warning: amplitude of %lf may be too large\n",amp);

	samptype = (unsigned int)atoi(argv[ARG_TYPE]);
	if (samptype>1)
	{
		fprintf(stderr,"ERROR: sample type can only be 0 or 1\n");
		return 1;		
	}

	fp = fopen(argv[ARG_OUTFILE],"wb");
	if (fp==NULL)
	{
		fprintf(stderr,"ERROR: unable to open file \"%s\"\n",argv[ARG_OUTFILE]);
		return 1;
	}

	nsamps = (int)(dur*srate);
	theta = twopi*freq/srate;
	step = dur/nsamps;
	start = 1.0;
	end = 1.0e-4;
	maxsamp = 0.0;
	fac = pow(end/start,1.0/nsamps);
	endian = byte_order();
	
	printf("Writing %d %s samples...\n",nsamps,endianness[endian]);

	if (samptype==RAWSAMP_SHORT)
	{
		for (i=0; i<nsamps; i++)
		{
			samp = amp*sin(theta*i);
			samp *= start;
			start *= fac;  
			ssamp = (short)(samp*32767.0);
			if (fwrite(&ssamp,sizeof(short),1,fp) != 1)
			{
				printf("Error writing data to file.\n");
				return 1;
			}
			if (fabs(samp) > maxsamp)
			{
				maxsamp = fabs(samp);
				maxframe = i;
			}
		}
	}
	else
	{
		for (i=0; i<nsamps; i++)
		{
			samp = amp*sin(theta*i);
			samp *= start;
			start *= fac;
			fsamp = (float) samp;	
			if (fwrite(&fsamp,sizeof(float),1,fp) != 1)
			{
				printf("Error writing data to file.\n");
				return 1;
			}
			if (fabs(samp)>maxsamp)
			{
				maxsamp = fabs(samp);
				maxframe = i;
			}
		}
	}
	
	fclose(fp);
	printf("Done. Maximum sample value = %.8lf at frame %d\n", maxsamp, maxframe); 
	return 0;
}
