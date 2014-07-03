/* signal generator for a simple sine wave */
/* usage: siggen outsndfile duration srate amp freq */ 
#include <stdio.h>
#include <stdlib.h>
#include <portsf.h>
#include <wave.h>
#define NFRAMES 100 // size of buffer

enum {ARG_PROGNAME, ARG_OUTFILE, ARG_DUR, ARG_SRATE,
      ARG_AMP, ARG_FREQ, ARG_NARGS};

int main (int argc, char**argv)
{
	PSF_PROPS outprops;
	unsigned long srate;
	double amp, freq, dur, peakdiff;
	unsigned long nbufs, outframes, remainder, nframes;
	psf_format outformat = PSF_FMT_UNKNOWN;

	/* init resource values */
	int ofd=-1;
	int error=0;
	float* outbuf = NULL;  /* buffer for outfile */
	OSCIL* p_osc = NULL;  /* sinewave oscillator */
	
	printf("SIGGEN: generate a simple sine wave oscillator\n");

	if (argc!=ARG_NARGS)
	{
		printf("ERROR: insufficient number of arguments.\n"
		       "USAGE: siggen outsndfile duration srate amp freq\n"
		      );
		return 1;
	}

	/* define outfile format - this sets 16-bit mono format */	
	srate = atof(argv[ARG_SRATE]);		
	if (srate<=0)
	{
		printf("ERROR: sample rate must be positive.\n");
		return 1;
	}
	outprops.srate = srate;
	outprops.chans = 1;
	outprops.samptype = PSF_SAMP_16;
	outprops.chformat = PSF_STDWAVE;
	/* get outfile extension */
	outformat = psf_getFormatExt(argv[ARG_OUTFILE]);
	if (outformat==PSF_FMT_UNKNOWN)
	{
		printf("Outfile name \"%s\" has unknown format.\n"
		       "Use any of .wav .aiff .aif .afc .aifc\n",
		        argv[ARG_OUTFILE]
		      );
		error++;
		goto exit;
	}
	outprops.format = outformat;

	/* get the time duration of the soundfile */
	dur = atof(argv[ARG_DUR]);
	if (dur<=0.0)
	{
		printf("ERROR: time duration must be positive.\n");
		return 1;
	}

	/* get the peak amplitude of the soundfile */
	amp = atof(argv[ARG_AMP]);
	if (amp<=0.0)
	{
		printf("ERROR: amplitude must be positive.\n");
		return 1;
	}

	/* get the frequency value */
	freq = atof(argv[ARG_FREQ]);
	if (freq<=0.0)
	{
		printf("ERROR: frequency must be positive.\n");
		return 1;
	}

	/* start portsf */
	psf_init();

	ofd = psf_sndCreate(argv[ARG_OUTFILE],&outprops,0,0,PSF_CREATE_RDWR); 
	if (ofd<0)
	{
		printf("ERROR: unable to create outfile: %s\n",argv[ARG_OUTFILE]);
		return 1;
	}

	/** at this point we have gathered resources 
	    we henceforth use goto upon hitting any errors **/

	/* initialize sinewave oscillator */
	p_osc = new_oscil(outprops.srate);

	/* calculate the number of frames for the soundfile */
	outframes = (unsigned long) (dur * outprops.srate + 0.5);

	/* allocate a buffer for outfile */
	nframes = NFRAMES;
	outbuf = (float*)malloc(sizeof(float)*nframes);

	/* calculate the number of buffers */
	nbufs = outframes/nframes;	
	remainder = outframes - nbufs*nframes;
	if (remainder > 0)
		nbufs++;	

	printf("Creating soundfile...\n");

	/* process soundfile */	
	long i,j;
	for (i=0; i < nbufs; i++)
	{ 
		if ((i == (nbufs-1)) && remainder)
			nframes = remainder;
		for (j=0; j < nframes; j++)
			outbuf[j] = (float)(amp * sinetick(p_osc,freq)); 
		if (psf_sndWriteFloatFrames(ofd,outbuf,nframes)!=nframes)
		{
			printf("Error writing to outfile\n");
			error++;
			break;
		}
	}

	/* make sure peak amplitude roughly matches the
	   user requested amplitude */
	peakdiff = amp-psf_sndPeakValue(ofd,&outprops);
	if ((peakdiff>.0001) || (peakdiff<-.0001))
	{
		printf("ERROR: unable to generate the correct peak\n"
		       "       amplitude for %s\n", argv[ARG_OUTFILE]); 	
		error++;
	}

	printf("Done.\t%d error%s\n"
	       "Outfile created: %s\n",
	        error, (error==1)?"":"s", argv[ARG_OUTFILE]);

	/* display outfile properties */
	psf_sndInfileProperties(argv[ARG_OUTFILE],ofd,&outprops);	

	/* cleanup resources */ 
	exit:
	if (ofd>=0)	
		if (psf_sndClose(ofd))
			printf("Error: unable to close outfile: %s\n",argv[ARG_OUTFILE]);
		else if (error)
		{
			printf("There was an error while processing the outfile.\n"
			       "Deleting outfile: %s ...\n", argv[ARG_OUTFILE]);
			if (remove(argv[ARG_OUTFILE]))
				printf("Error: failed to delete %s\n", argv[ARG_OUTFILE]);
			else
				printf("%s successfully deleted.\n", argv[ARG_OUTFILE]);
		}
	if (outbuf)
		free(outbuf);
	if (p_osc)
		free(p_osc);
	psf_finish();

	return error;	
}
