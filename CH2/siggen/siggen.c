/* signal generator for a simple waveform */
/* usage: siggen [-sN] [-oN] outsndfile wavetype [pwval] duration srate nchans amp freq */ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <portsf.h>
#include <wave.h>
#include <breakpoints.h>
#define NFRAMES 100 // size of buffer

enum {ARG_PROGNAME, ARG_OUTFILE, ARG_TYPE,
      ARG_PWMOD, ARG_DUR=3, ARG_SRATE, ARG_NCHANS, 
      ARG_AMP, ARG_FREQ, ARG_NARGS};

enum {WAVE_SINE=4, WAVE_PWMOD, WAVE_SQUARE, WAVE_TRIANGLE=8,
      WAVE_SAWUP=11, WAVE_SAWDOWN=13};

int main (int argc, char**argv)
{
	PSF_PROPS outprops;
	unsigned long srate;
	double amp, freq, pwval, dur, peakdiff;
	unsigned long nbufs, outframes, remainder, nframes;
	psf_format outformat = PSF_FMT_UNKNOWN;
	int wavetype=-1;
	float wavevalue;
	int nchans;
	int ispwval=0; /* flagged if pwval is a value and not a breakpoint file */
	char* arg_pwmod; /* retains the name of the pwval breakpoint file */
	double minval, maxval; /* used for breakpoint amplitude values */	
	tickfunc tick; /* point to the specified wavetype function */
	char option; /* stores command line options */
	psf_stype samptype = PSF_SAMP_16; /* outfile is set to 16-bit by default */	
	double phase=0.0; /* fraction between 0 and 1 which sets the phase offset
	                 default values match the direction of the sine wave
	                 (i.e starting at zero, going positive)  */

	/* init resource values */
	int ofd=-1;
	int error=0;
	float* outbuf = NULL;  /* buffer for outfile */
	OSCIL* p_osc = NULL;  /* waveform oscillator */
	BRKSTREAM* ampstream = NULL; /* breakpoint stream of amplitude values */
	BRKSTREAM* freqstream = NULL; /* breakpoint stream of frequency values */
	BRKSTREAM* pwmodstream = NULL; /* breakpoint stream of pwmod values */
	FILE* fpamp = NULL; /* amplitude breakpoint file */
	FILE* fpfreq = NULL; /* frequency breakpoint file */	
	FILE* fppwmod = NULL; /* pulse width modulation breakpoint file */
	unsigned long brkampSize = 0; /* number of amplitude breakpoints */
	unsigned long brkfreqSize = 0; /* number of frequency breakpoints */
	unsigned long brkpwmodSize = 0; /* number of pulse wave mod breakpoints */
	
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
				case('o'):
					if (argv[1][2]=='\0')
					{
						printf("ERROR: you did not specify an offset\n"
						       "USAGE: -oN   (0 <= N <= 1)\n"
						      );
						return 1;
					}
					if (!(argv[1][2] >= 'a' && argv[1][2]<='z'))
					{
						phase = atof(&argv[1][2]);
						if (phase < 0.0)
							phase = 0.0;
						if (phase > 1.0)
							phase = 1.0;
						break;
					}
				default:
					printf("ERROR: %s is not a valid option\n"
					       "options: -sN  select the format of the output sound file\n" 
					       "              N = 16 (16-bit), 24 (24-bit), or 32 (32-bit)\n"
					       "         -oN  select the offset of the oscillator\n"
					       "              0 <= N <= 1\n",
					        argv[1]);
					return 1;
			}
			argc--;
			argv++;
		}
	}

	if (argc!=ARG_NARGS)
		if (argc==(ARG_NARGS+1))
		{
			/* check if pwval is specified */
			fppwmod = fopen(argv[ARG_PWMOD], "r");
			if (fppwmod==NULL)
			{
				if (argv[ARG_PWMOD][0] >='a' && argv[ARG_PWMOD][0] <= 'z') 
				{
					printf("Error: breakpoint file \"%s\" does not exist.\n",
					        argv[ARG_PWMOD]);
					return 1;
				}
				pwval = atof(argv[ARG_PWMOD]);
				if (pwval < 1 || pwval > 99)
				{
					printf("Error: pwval is out of range.\n"
					       "       1 <= pwval <= 99\n");
					return 1;
				}
				/* we have gathered a pwval value */
				ispwval=1;
			}
			else
		  {
				/* breakpoint file exists, gather breakpoint values */
				pwmodstream = bps_newstream(fppwmod,atof(argv[ARG_SRATE+1]),&brkpwmodSize);
				if (pwmodstream==NULL)
				{
					printf("ERROR: unable to obtain breakpoints from \"%s\"\n",
									argv[ARG_PWMOD]);
					error++;
					goto exit;
				}
			}
			/* make sure enum ARG values are aligned with command-line arguments */	
			int argnum = ARG_PWMOD;
			arg_pwmod = argv[ARG_PWMOD];
			while (argnum != (ARG_NARGS+1))
				argv[argnum] = argv[++argnum];
		}
		else
		{
			printf("ERROR: insufficient number of arguments.\n"
						 "USAGE: siggen [-sN] [-oN] outsndfile wavetype [pwval] duration srate nchans amp freq\n"
						 "       -sN:       select the format of the output sound file (16-bit by default)\n"
						 "                  N = 16 (16-bit), 24 (24-bit), or 32 (32-bit)\n"
			       "       -oN:       select the offset of the oscillator\n"
			       "                  0 <= N <= 1\n"
			       "                  if you set N < 0, then N is set to 0\n"
		         "                  if you set N > 1, then N is set to 1\n"
						 "       wavetype:  sine, triangle, square, pwmod, sawtooth_up, sawtooth_down\n" 
						 "       pwval:     pulse wave percentage value or breakpoint file\n"
			       "                  modulation range is from 1%% to 99%%\n"
			       "                  a normal square wave is 50%%\n" 
						 "                  pwval must be selected only when the\n"
						 "                  pwmod wavetype has been selected.\n"
						 "       duration:  duration of outfile (seconds)\n"
						 "       srate:     required sample rate of outfile\n"
						 "       amp:       amplitude value or breakpoint file\n"
						 "                  (0 < amp <= 1.0)\n"
						 "       freq:      frequency value or breakpoint file\n"
						 "                  (freq > 0 )\n"
						);
			return 1;
		}

	/** at this point we may have gathered resources 
	    we henceforth use goto upon hitting any errors **/

	/* define outfile format */	
	srate = atof(argv[ARG_SRATE]);		
	if (srate<=0)
	{
		printf("ERROR: sample rate must be positive.\n");
		error++;
		goto exit;
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
		error++;
		goto exit;
	}
	outprops.chans = nchans;	

	/* get the wavetype */
	int count = 0;
	while (argv[ARG_TYPE][count]!='\0') count++;
	switch (count)
	{
		case(WAVE_SINE):
			if (!strcmp(argv[ARG_TYPE],"sine")) 
				wavetype = WAVE_SINE;
			break;
		case(WAVE_PWMOD):
			if (!strcmp(argv[ARG_TYPE],"pwmod"))
				wavetype = WAVE_PWMOD;
			break;
		case(WAVE_SQUARE):
			if (!strcmp(argv[ARG_TYPE],"square"))
				wavetype = WAVE_SQUARE;
			break;
		case(WAVE_TRIANGLE):
			if (!strcmp(argv[ARG_TYPE],"triangle"))
				wavetype = WAVE_TRIANGLE;
			break;
		case(WAVE_SAWUP):
			if (!strcmp(argv[ARG_TYPE],"sawtooth_up"))
				wavetype = WAVE_SAWUP;
			break;
		case(WAVE_SAWDOWN):
			if (!strcmp(argv[ARG_TYPE],"sawtooth_down"))
				wavetype = WAVE_SAWDOWN;
			break;	
		default:
			wavetype = -1;		
	}
	if (wavetype<0)
	{
		printf("ERROR: you have chosen an unknown wavetype.\n"
		       "wavetypes: sine, square, pwmod, triangle, sawtooth_up, sawtooth_down\n"
		      );
		error++;
		goto exit;
	}
	
	/* if user specifies pwval, make sure pwmod wavetype is selected */
	if (pwmodstream||ispwval)
		if (wavetype!=WAVE_PWMOD)
		{
			printf("ERROR: if you select a value or breakpoint file for pwval,\n"
			       "       you must select the pwmod wavetype.\n");
			error++;
			goto exit;
		}

	/* select waveform function */
	switch (wavetype)
	{
		case(WAVE_SINE):
			tick = sinetick;
			phase += 0.0;
			break;
		case(WAVE_TRIANGLE):
			tick = tritick;
			phase += 0.75;
			break;
		case(WAVE_SQUARE):
			tick = sqtick;
			phase += 0.0;
			break;
		case(WAVE_PWMOD):
			if (pwmodstream==NULL && !ispwval)
			{
				printf("ERROR: you didn't specify a pulse wave value "
				       "or breakpoint file.\n");
				error++;
				goto exit;
			}
			phase += 0.0;
			break;
		case(WAVE_SAWUP):
			tick = sawutick;
			phase += 0.5;
			break;
		case(WAVE_SAWDOWN):
			tick = sawdtick;
			phase += 0.5;
	}

	/* get the time duration of the soundfile */
	dur = atof(argv[ARG_DUR]);
	if (dur<=0.0)
	{
		printf("ERROR: time duration must be positive.\n");
		error++;
		goto exit;
	}

	/* open breakpoint file, or set constant amplitude */ 
	fpamp = fopen(argv[ARG_AMP],"r"); 
	if (fpamp==NULL)
	{
		if (argv[ARG_AMP][0] >= 'a' && argv[ARG_AMP][0] <= 'z')
		{
			printf("ERROR: breakpoint file \"%s\" does not exist.\n",
			        argv[ARG_AMP]);
			error++;
			goto exit;
		}
		amp = atof(argv[ARG_AMP]);
		if (amp<=0.0 || amp>1.0)
		{
			printf("ERROR: amplitude value out of range.\n"
			       "       0.0 < amp <= 1.0\n"
			      );
			error++;
			goto exit;
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

	/* open breakpoint file, or set constant frequency */
	fpfreq = fopen(argv[ARG_FREQ],"r");
	if (fpfreq==NULL)
	{
		if (argv[ARG_FREQ][0] >= 'a' && argv[ARG_FREQ][0] <='z')
		{
			printf("ERROR: breakpoint file \"%s\" does not exist.\n",
			        argv[ARG_FREQ]); 
			error++;
			goto exit;
		}
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
		freqstream = bps_newstream(fpfreq,outprops.srate,&brkfreqSize);
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
	p_osc = new_oscilp(outprops.srate, phase);

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
			if (pwmodstream)
				pwval = bps_tick(pwmodstream);
			if (pwmodstream || ispwval )
				wavevalue = (float)(amp * pwmtick(p_osc,freq,pwval));
			else
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
	if ((peakdiff>.001) || (peakdiff<-.001))
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
	if (pwmodstream)
		bps_freepoints(pwmodstream);
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
	if (fppwmod)
		if (fclose(fppwmod))
		{
			printf("Error closing breakpoint file \"%s\"\n",
			        arg_pwmod);
		}
	psf_finish();

	return error;	
}
