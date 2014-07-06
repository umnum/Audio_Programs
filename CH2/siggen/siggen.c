/* signal generator for a simple waveform */
/* usage: siggen [-sN] outsndfile wavetype duration srate nchans amp freq */ 
#include <stdio.h>
#include <stdlib.h>
#include <portsf.h>
#include <wave.h>
#include <breakpoints.h>
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
	double minval, maxval;	
	tickfunc tick;
	char option;
	psf_stype samptype;	

	/* init resource values */
	int ofd=-1;
	int error=0;
	float* outbuf = NULL;  /* buffer for outfile */
	OSCIL* p_osc = NULL;  /* waveform oscillator */
	BRKSTREAM* ampstream = NULL; /* breakpoint stream of amplitude values */
	BRKSTREAM* freqstream = NULL; /* breakpoint stream of frequency values */
	FILE* fpamp = NULL;
	FILE* fpfreq = NULL;	
	unsigned long brkampSize = 0;
	
	printf("SIGGEN: generate a simple waveform\n");

	/* check for command-line options */
	if (argc>1)
	{
		while (argv[1][0] == '-')
		{
			option = argv[1][1];
			switch (option)
			{
				case('\0'):
					printf("ERROR:   you did not specify an option\n"
					       "options: -sN  select the format of the output sound file\n" 
					       "              N = 16 (16-bit), 24 (24-bit), or 32 (32-bit)\n"
					      );
					return 1;
				case('s'):
					if (argv[1][2]=='\0')
					{
						printf("ERROR: you did not specify a format\n"
						       "formats: 16 (16-bit), 24 (24-bit), or 32 (32-bit)\n",
						        &argv[1][2]);
						return 1;
					}
					samptype = atoi(&argv[1][2]);
					if (samptype == 16)
					{
						samptype = PSF_SAMP_16;
						break;
					}	
					if (samptype == 24)
					{
						samptype = PSF_SAMP_24;
						break;
					}
					if (samptype == 32)
					{
						samptype = PSF_SAMP_32;
						break;
					}
					printf("ERROR:   %s is not a valid format\n"
					       "formats: 16 (16-bit), 24 (24-bit), or 32 (32-bit)\n",
					        &argv[1][2]);
					return 1;
				default:
					printf("ERROR: %s is not a valid option\n"
					       "options: -sN  select the format of the output sound file\n" 
					       "              N = 16 (16-bit), 24 (24-bit), or 32 (32-bit)\n",
					        argv[1]);
					return 1;
			}
			argc--;
			argv++;
		}
	}

	if (argc!=ARG_NARGS)
	{
		printf("ERROR: insufficient number of arguments.\n"
		       "USAGE: siggen [-sN] outsndfile wavetype duration srate nchans amp freq\n"
		       "       -sN:       select the format of the output sound file\n"
		       "                  N = 16 (16-bit), 24 (24-bit), or 32 (32-bit)\n"
		       "       wavetype:  sine, triangle, square, sawtooth_up, sawtooth_down\n" 
		       "       duration:  duration of outfile (seconds)\n"
		       "       srate:     required sample rate of outfile\n"
		       "       amp:       amplitude value or breakpoint file\n"
		       "                  (0 < amp <= 1.0)\n"
		       "       freq:      frequency value or breakpoint file\n"
		       "                  (freq > 0 )\n"
		      );
		return 1;
	}

	/* define outfile format */	
	srate = atof(argv[ARG_SRATE]);		
	if (srate<=0)
	{
		printf("ERROR: sample rate must be positive.\n");
		return 1;
	}
	outprops.srate = srate;
	outprops.samptype = samptype;
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

	/* open breakpoint file, or set constant amplitude */ 
	fpamp = fopen(argv[ARG_AMP],"r"); 
	if (fpamp==NULL)
	{
		amp = atof(argv[ARG_AMP]);
		if (amp<=0.0 || amp>1.0)
		{
			printf("ERROR: amplitude value out of range.\n"
			       "       0.0 < amp <= 1.0\n"
			      );
			return 1;
		}
	}
	else
	{
		ampstream = bps_newstream(fpamp,outprops.srate,&brkampSize);
		if (ampstream==NULL)
		{
			printf("ERROR: unable to obtain breakpoints from \"%s\"\n",
							argv[ARG_AMP]);
			error++;
			goto exit;
		}
		if (bps_getminmax(ampstream,&minval,&maxval))
		{
			printf("Error reading range of breakpoint file \"%s\"\n",
			        argv[ARG_AMP]);
			error++;
			goto exit;
		}
		if (minval < 0.0 || minval > 1.0 || maxval < 0.0 || maxval > 1.0)
		{
			printf("Error: amplitude values out of range in file \"%s\"\n"
			       "       0.0 < amp <= 1.0\n",
			        argv[ARG_AMP]);
			error++;
			goto exit;
		}
	}

	/** at this point we may have gathered resources 
	    we henceforth use goto upon hitting any errors **/

	/* open breakpoint file, or set constant frequency */
	fpfreq = fopen(argv[ARG_FREQ],"r");
	if (fpfreq==NULL)
	{
		freq = atof(argv[ARG_FREQ]);
		if (freq<=0.0)
		{
			printf("ERROR: frequency must be positive.\n");
			error++;
			goto exit;
		}
	}
	else
	{
		freqstream = bps_newstream(fpfreq,outprops.srate,&brkampSize);
		if (freqstream==NULL)
		{
			printf("ERROR: unable to obtain breakpoint from \"%s\"\n",
			        argv[ARG_FREQ]);
			error++;
			goto exit;
		}
	}

	/* start portsf */
	psf_init();

	ofd = psf_sndCreate(argv[ARG_OUTFILE],&outprops,0,0,PSF_CREATE_RDWR); 
	if (ofd<0)
	{
		printf("ERROR: unable to create outfile: %s\n",argv[ARG_OUTFILE]);
		error++;
		goto exit;
	}

	/* initialize waveform oscillator */
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
			if (ampstream)
				amp = bps_tick(ampstream);	
			if (freqstream)
				freq = bps_tick(freqstream);
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
	if (ampstream==NULL)
		peakdiff = amp-psf_sndPeakValue(ofd,&outprops);
	else
		peakdiff = maxval-psf_sndPeakValue(ofd,&outprops);
	if ((peakdiff>.0001) || (peakdiff<-.0001))
	{
		printf("ERROR: unable to generate the correct peak\n"
		       "       amplitude for %s\n", argv[ARG_OUTFILE]); 	
		printf("amp = %lf\tpeakamp = %lf\n", amp, psf_sndPeakValue(ofd,&outprops));
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
	{
		free(outbuf);
		outbuf=NULL;
	}
	if (p_osc)
	{
		free(p_osc);
		p_osc=NULL;
	}
	if (ampstream)
		bps_freepoints(ampstream);
	if (freqstream)
		bps_freepoints(freqstream);
	if (fpamp)
		if (fclose(fpamp))
		{
			printf("Error closing breakpoint file \"%s\"\n",
			        argv[ARG_AMP]);
		}
	if (fpfreq)
		if (fclose(fpfreq))
		{
			printf("Error closing breakpoint file \"%s\"\n",
			        argv[ARG_FREQ]);
		}
	psf_finish();

	return error;	
}
