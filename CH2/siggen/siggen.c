/* signal generator for a simple waveform */
/* usage: siggen outsndfile wavetype duration srate nchans amp freq */ 
#include <stdio.h>
#include <stdlib.h>
#include <portsf.h>
#include <wave.h>
#define NFRAMES 100 // size of buffer

enum {ARG_PROGNAME, ARG_OUTFILE, ARG_TYPE,
      ARG_DUR, ARG_SRATE, ARG_NCHANS, 
      ARG_AMP, ARG_FREQ, ARG_NARGS};

enum {WAVE_SINE, WAVE_TRIANGLE, WAVE_SQUARE,
      WAVE_SAWUP, WAVE_SAWDOWN, WAVE_NTYPES};

int main (int argc, char**argv)
{
	PSF_PROPS outprops;
	unsigned long srate;
	double amp, freq, dur, peakdiff;
	unsigned long nbufs, outframes, remainder, nframes;
	psf_format outformat = PSF_FMT_UNKNOWN;
	int wavetype=-1;
	float wavevalue;
	int nchans;
	tickfunc tick;
	/* init resource values */
	int ofd=-1;
	int error=0;
	float* outbuf = NULL;  /* buffer for outfile */
	OSCIL* p_osc = NULL;  /* sinewave oscillator */
	
	printf("SIGGEN: generate a simple sine wave oscillator\n");

	if (argc!=ARG_NARGS)
	{
		printf("ERROR: insufficient number of arguments.\n"
		       "USAGE: siggen outsndfile wavetype duration srate nchans amp freq\n"
		       "       wavetype:  sine, triangle, square, sawtooth_up, sawtooth_down\n" 
		       "       duration:  duration of outfile (seconds)\n"
		       "       srate:     required sample rate of outfile\n"
		       "       amp:       amplitude (0 < amp <= 1.0)\n"
		       "       freq:      frequency (freq > 0 )\n"
		      );
		return 1;
	}

	/* define outfile format - this sets 16-bit format */	
	srate = atof(argv[ARG_SRATE]);		
	if (srate<=0)
	{
		printf("ERROR: sample rate must be positive.\n");
		return 1;
	}
	outprops.srate = srate;
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
	/* get number of channels */
	nchans = atoi(argv[ARG_NCHANS]);
	if (nchans > MC_WAVE_EX)
	{
		printf("ERROR: portsf does not support %d channels\n",
		        nchans); 
		return 1;
	}
	outprops.chans = nchans;	

	/* get the wavetype */
	int count = 0;
	while (argv[ARG_TYPE][count]!='\0') count++;
	switch (count)
	{
		case(4):
			if ( (argv[ARG_TYPE][0]=='s')&&(argv[ARG_TYPE][1]=='i'&&
					 (argv[ARG_TYPE][2]=='n')&& argv[ARG_TYPE][3]=='e')  ) 
				wavetype = WAVE_SINE;
			break;
		case(6):
			if ( (argv[ARG_TYPE][0]=='s')&&(argv[ARG_TYPE][1]=='q')&&
			     (argv[ARG_TYPE][2]=='u')&&(argv[ARG_TYPE][3]=='a')&&
			     (argv[ARG_TYPE][4]=='r')&&(argv[ARG_TYPE][5]=='e')   )
				wavetype = WAVE_SQUARE;
			break;
		case(8):
			if ( (argv[ARG_TYPE][0]=='t')&&(argv[ARG_TYPE][1]=='r')&&
			     (argv[ARG_TYPE][2]=='i')&&(argv[ARG_TYPE][3]=='a')&&
			     (argv[ARG_TYPE][4]=='n')&&(argv[ARG_TYPE][5]=='g')&&
			     (argv[ARG_TYPE][6]=='l')&&(argv[ARG_TYPE][7]=='e')   )
				wavetype = WAVE_TRIANGLE;
			break;
		case(11):
			if ( (argv[ARG_TYPE][0]=='s')&&(argv[ARG_TYPE][1]=='a')&&
			     (argv[ARG_TYPE][2]=='w')&&(argv[ARG_TYPE][3]=='t')&&
			     (argv[ARG_TYPE][4]=='o')&&(argv[ARG_TYPE][5]=='o')&&
			     (argv[ARG_TYPE][6]=='t')&&(argv[ARG_TYPE][7]=='h')&&
			     (argv[ARG_TYPE][8]=='_')&&(argv[ARG_TYPE][9]=='u')&&
			     (argv[ARG_TYPE][10]=='p')                            )
				wavetype = WAVE_SAWUP;
			break;
		case(13):
			if ( (argv[ARG_TYPE][0]=='s')&&(argv[ARG_TYPE][1]=='a')&&
			     (argv[ARG_TYPE][2]=='w')&&(argv[ARG_TYPE][3]=='t')&&
			     (argv[ARG_TYPE][4]=='o')&&(argv[ARG_TYPE][5]=='o')&&
			     (argv[ARG_TYPE][6]=='t')&&(argv[ARG_TYPE][7]=='h')&&
			     (argv[ARG_TYPE][8]=='_')&&(argv[ARG_TYPE][9]=='d')&&
			     (argv[ARG_TYPE][10]=='o')&&(argv[ARG_TYPE][11]=='w')&&
			     (argv[ARG_TYPE][12]=='n')                              )
				wavetype = WAVE_SAWDOWN;
			break;	
		default:
			wavetype = -1;		
	}
	if (wavetype<0)
	{
		printf("ERROR: you have chosen an unknown wavetype.\n"
		       "wavetypes: sine, square, triangle, sawtooth_up, sawtooth_down\n"
		      );
		return 1;
	}
	
	/* select waveform function */
	switch (wavetype)
	{
		case(WAVE_SINE):
			tick = sinetick;
			break;
		case(WAVE_TRIANGLE):
			tick = tritick;
			break;
		case(WAVE_SQUARE):
			tick = sqtick;
			break;
		case(WAVE_SAWUP):
			tick = sawutick;
			break;
		case(WAVE_SAWDOWN):
			tick = sawdtick;
	}

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
	outbuf = (float*)malloc(sizeof(float)*nframes*outprops.chans);

	/* calculate the number of buffers */
	nbufs = outframes/nframes;	
	remainder = outframes - nbufs*nframes;
	if (remainder > 0)
		nbufs++;	

	printf("Creating soundfile...\n");

	
	/* process soundfile */	
	long i,j,i_out;
	for (i=0; i < nbufs; i++)
	{ 
		if ((i == (nbufs-1)) && remainder)
			nframes = remainder;
		for (j=0; j < nframes; j++)
		{
			wavevalue = (float)(amp * tick(p_osc,freq));
			for (i_out=0; i_out < outprops.chans; i_out++)
				outbuf[j*outprops.chans+i_out] = wavevalue; 
		}
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
