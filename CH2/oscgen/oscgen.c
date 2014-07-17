/* a basic oscillator bank for additive synthesis */
/* USAGE: oscgen [-wN] [-t] [-v fac freq] outsndfile dur srate nchans amp freq wavetype nharms ampfac.dat freqfac.dat */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <portsf.h>
#include <breakpoints.h>
#include <wave.h>
#include <gtable.h>
#define DEFAULT_WIDTH 1024 // default oscillator table length
#define DEFAULT_NFRAMES 100 // default frames for buffer
#define DEFAULT_PHASE 0 /* default oscillator offset is 0  
                           set to 0.25 for triagle wave */

enum {ARG_PROGNAME, ARG_OUTFILE, ARG_DUR, ARG_SRATE, ARG_CHANS,
      ARG_AMP, ARG_FREQ, ARG_TYPE, ARG_NHARMS, 
      ARG_AMPFAC, ARG_FREQFAC, ARG_NARGS};

enum {WAVE_SINE=4, WAVE_SAWUP, WAVE_SQUARE, WAVE_SAWDOWN, WAVE_TRIANGLE};

main (int argc, char* argv[])
{
	/* declare variables */
	PSF_PROPS outprops; /* soundfile properties */
	double dur, amp, freq, val;
	double phase = DEFAULT_PHASE; 
	unsigned long width = DEFAULT_WIDTH; 
	double minval, maxval, maxamp; /* gather min/max values in breakpoint files */
	double ampfac, freqfac, ampadjust; /* oscillator bank values */
	double vibsig=0.0, vibfac=0.0, vibfreq=0.0; /* sets vibrato */ 
	int isvib = 0; /* flag for setting vibrato */
	int chans, srate,
	    noscs; /* number of oscillators in oscillator bank */
	int wavetype = -1;
	int i, j, i_out; /* for loop counters */
	psf_format format = PSF_FMT_UNKNOWN;
	unsigned long nbufs, outframes, remainder, nframes, framesread; /* buffer frame variables */
	unsigned long nharms;
	oscilt_tickfunc tickfunc = tabitick;	
	char line[40];

	/* initialize resources */
	int ofd = -1;
	int error = 0;
	FILE* fpamp = NULL;
	FILE* fpfreq = NULL;
	FILE* fpampfac = NULL;
	FILE* fpfreqfac = NULL;
	float* buffer = NULL;	
	BRKSTREAM* ampstream = NULL;
	BRKSTREAM* freqstream = NULL;
	OSCILT **oscs = NULL;
	OSCILT *vibosc = NULL; /* oscillator for vibrato */
	GTABLE** gtable = NULL;
	GTABLE* vibtable = NULL; /* oscillator lookup table for vibrato */
	double *oscamps = NULL,
	       *oscfreqs = NULL;
	unsigned long brkampSize = 0,
	              brkfreqSize = 0,
	              ampfacSize = 0,
	              freqfacSize = 0;

	printf("OSCGEN: basic oscillator bank for additive synthesis\n");

	/* check for command line options */
	if (argc>1)
	{
		while (argv[1][0] == '-')
		{
			switch (argv[1][1])
			{
				case ('\0'):
					printf("Error:   you did not specify an option.\n"
					       "options: -wN   | set width of lookup table "
					       "to N points (default: 1024 points)\n"
					       "         -t    | use truncating lookup "
					       "(default: interpolating lookup)\n"
					       "         -v fac freq   | include vibrato set at "
					       "a specified factor and frequency\n",
					        argv[1]);
					return 1;
				case('t'):
					if (argv[1][2] == '\0')
						tickfunc = tabtick;
					else
					{
						printf("Error:   %s is not a valid option.\n"
						       "options: -wN   | set width of lookup table "
						       "to N points (default: 1024 points)\n"
						       "         -t    | use truncating lookup "
						       "(default: interpolating lookup)\n"
						       "         -v fac freq   | include vibrato set at "
						       "a specified factor and frequency\n",
						        argv[1]);
						return 1;
					}
					break;
				case('v'):
					if (argv[1][2] != '\0')
					{
						printf("Error:   %s is not a valid option.\n"
						       "options: -wN   | set width of lookup table "
						       "to N points (default: 1024 points)\n"
						       "         -t    | use truncating lookup "
						       "(default: interpolating lookup)\n"
						       "         -v fac freq   | include vibrato set at "
						       "a specified factor and frequency\n",
						        argv[1]);
						return 1;
					}
					if (argc > 3)
					{
						vibfac = atof(argv[2]);	
						if (vibfac <= 0.0 || vibfac > 1.0)
						{
							printf("Error: vibrato factor out of range.\n"
							       "       0.0 < fac <= 1.0\n");
							return 1;
						}	
						vibfreq = atof(argv[3]);
						if (vibfreq <= 0.0)
						{
							printf("Error: vibrato frequency out of range.\n"
							       "       freq > 0.0\n");
							return 1;
						}
						isvib = 1;
						argc -= 2;
						argv += 2;
					}
					break;
				case('w'):
					if (argv[1][2] == '\0')
					{
						printf("Error: you did not specify a table lookup width.\n");
						return 1;
					}
					if (argv[1][2] >= '0' && argv[1][2] <= '9')
					{
						width = atoi(&argv[1][2]);
						if (width < 1)
						{
							printf("Error: table width must be at least 1\n");
							return 1;
						}
						break;
					}
				default:
					printf("Error:   %s is not a valid option.\n"
					       "options: -wN   | set width of lookup table "
					       "to N points (default: 1024 points)\n"
					       "         -t    | use truncating lookup "
					       "(default: interpolating lookup)\n"
					       "         -v fac freq   | include vibrato set at "
					       "a specified factor and frequency\n",
					        argv[1]);
					return 1;
			}
			argc--;
			argv++;
		}
	}

	/* check for correct number of arguments */
	if (argc!=ARG_NARGS)
	{
		printf("ERROR: insufficient number of arguments.\n\n"

		       "USAGE: oscgen [-wN] [-t] [-v fac freq] outsndfile dur srate nchans amp freq wavetype nharms ampfac.dat freqfac.dat\n"
		       "------------------------------------------------------------------------------------------------------------------\n"
		       "options:     -wN  set width of lookup table to N points (default: 1024 points)\n"
		       "                  (N >= 1)\n"
		       "             -t   use truncating lookup (default: interpolating lookup)\n"
		       "             -v   adds vibrato to the main oscillator\n"
		       "                  fac:  vibrato factor affects the amount of vibrato\n"
		       "                        (0.0 < fac <= 1.0)\n"
		       "                  freq: frequency of vibrato\n" 
		       "                        (freq > 0.0)\n"
		       "outsndfile:  output soundfile supported by portsf\n"
		       "             use any of .wav .aiff .aif .afc .aifc formats\n" 
		       "dur:         duration of soundfile (seconds)\n"
		       "srate:       soundfile sample rate (srate > 0)\n"
		       "nchans:      soundfile number of channels\n"
		       "amp:         amplitude value or breakpoint file\n"
		       "             (0.0 < amp <= 1.0)\n"
		       "freq:        frequency value or breakpoint file\n"
		       "             (frequency >= 0.0)\n"	
		       "wavetype:    square, triangle, sawtooth_up, sawtooth_down\n"
		       "nhamrs:      number of wavetype harmonics\n"
		       "ampfac.dat:  amplitude factor data file\n"
		       "             accepts a list of harmonic amplitude data\n"
		       "             which is stored in the oscillator bank\n"
		       "             (0.0 < ampfac <= 1.0)\n"
		       "freqfac.dat: frequency factor data file\n"
		       "             accepts a list of numbers which are\n"
		       "             multiplied by the fundamental frequency\n"
		       "             of each corresponding oscillator\n"
		       "             in the oscillator bank\n"
		       "             (freqfac > 0.0)\n"
		      );
		return 1;
	}

	/* open portsf */
	psf_init();

	/* get duration of the soundfile */
	dur = atof(argv[ARG_DUR]);
	if (dur <= 0.0)
	{
		printf("ERROR: duration must be positive.\n");
		return 1;
	}

	/* get the sample rate */
	srate = atoi(argv[ARG_SRATE]);
	if (srate <=0)
	{
		printf("ERROR: sample rate must be positive.\n");
		return 1;
	}
	outprops.srate = srate;

	/* get the number of channels */
	chans = atoi(argv[ARG_CHANS]);
	if (chans <= STDWAVE || chans > MC_WAVE_EX) 
	{
		printf("ERROR: portsf does not support %s channels\n",
		        argv[ARG_CHANS]);
		return 1;
	}	
	outprops.chans = chans;

	/* get the wavetype */
	int count = 0;
	while (argv[ARG_TYPE][count]!='\0')count++;	

	switch(count)
	{
		case(WAVE_SINE):
			if (!strcmp(argv[ARG_TYPE],"sine"))
				wavetype = WAVE_SINE;
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
			if (!strcmp(argv[ARG_TYPE],"sawup"))
				wavetype = WAVE_SAWUP;
			break;
		case(13):
			if (!strcmp(argv[ARG_TYPE],"sawdown"))
				wavetype = WAVE_SAWDOWN;
			break;
		default:
			wavetype = -1;
	}	
	if (wavetype < 0)
	{
		printf("ERROR:    %s is not a valid wavetype.\n"
		       "wavetype: square, triangle, sawup, sawdown\n",
		        argv[ARG_TYPE]); 
		return 1;
	}

	/* get the number of harmonics */
	nharms = atoi(argv[ARG_NHARMS]);
	if (nharms < 1)
	{
		printf("Error: the number of harmonics must be at least 1\n");
		return 1;
	}

  /* get the output soundfile extension */
	format = psf_getFormatExt(argv[ARG_OUTFILE]);
	if (format == PSF_FMT_UNKNOWN)
	{
		printf("ERROR: outfile extension unknown\n"
		       "       use any of .wav .aiff .aif .afc .aifc formats\n"
		      );
		return 1;
	}	
	outprops.format = format;

	/* create the output soundfile - will be 16-bit by default */
	outprops.samptype = PSF_SAMP_16;	
	outprops.chformat = PSF_STDWAVE;
	ofd = psf_sndCreate(argv[ARG_OUTFILE],&outprops,0,0,PSF_CREATE_RDWR);
	if (ofd<0)
	{
		printf("ERROR: unable to create soundfile: %s\n", argv[ARG_OUTFILE]);
		return 1; 
	}

	/* we have gathered a resource at this point
	   use goto upon hitting any errors */

	/* get amplitude value or breakpoint file */ 
	fpamp = fopen(argv[ARG_AMP],"r");
	if (fpamp == NULL)
	{
		/* if the user specified a non-existant breakpoint file
		   or didn't specify an ampitude value */
		if ( (argv[ARG_AMP][0] < '0' || argv[ARG_AMP][0] > '9') && 
		     (argv[ARG_AMP][0] != '.' && argv[ARG_AMP][0] != '-') || 
		     (argv[ARG_AMP][0] == '.' && (argv[ARG_AMP][1] < '0' || argv[ARG_AMP][1] > '9')) ||  
		     (argv[ARG_AMP][0] == '-' && (argv[ARG_AMP][1] < '0' || argv[ARG_AMP][1] > '9') && argv[ARG_AMP][1] != '.') )  
		{
			printf("Error: breakpoint file: \"%s\" does not exist\n",
			        argv[ARG_AMP]);
			error++;
			goto exit; 
		}
		/* the user specified an amplitude value */
		amp = atof(argv[ARG_AMP]);
		if (amp <= 0.0 || amp > 1.0)
		{
			printf("Error: amplitude value out of range.\n"
			       "       0 < amp <= 1.0\n"); 
			error++;
			goto exit;
		}
	}
	else
	{
		ampstream = bps_newstream(fpamp,outprops.srate,&brkampSize); 
		if (ampstream == NULL)
		{
			printf("Error: unable to obtain breakpoints from file \"%s\"\n",
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
		if (minval <= 0.0 || minval > 1.0 || maxval <= 0.0 || maxval > 1.0)
		{
			printf("Error: breakpoint amplitude values out of range in file \"%s\"\n"
			       "       0.0 < amp <= 1.0\n",
			        argv[ARG_AMP]);
			error++;
			goto exit; 
		}
		maxamp = maxval;
	}

	// get frequency value or breakpoint file	
	fpfreq = fopen(argv[ARG_FREQ],"r");			
	if (fpfreq == NULL)
	{
	if ( (argv[ARG_FREQ][0] < '0' || argv[ARG_FREQ][0] > '9') && 
	     (argv[ARG_FREQ][0] != '.' && argv[ARG_FREQ][0] != '-') || 
	     (argv[ARG_FREQ][0] == '.' && (argv[ARG_FREQ][1] < '0' || argv[ARG_FREQ][1] > '9')) ||  
	     (argv[ARG_FREQ][0] == '-' && (argv[ARG_FREQ][1] < '0' || argv[ARG_FREQ][1] > '9') && argv[ARG_FREQ][1] != '.')  )  
		{
			printf("Error: breakpoint file \"%s\" does not exist.\n",
			        argv[ARG_FREQ]);	
			error++;
			goto exit;
		}
		freq = atof(argv[ARG_FREQ]);
		if (freq <= 0.0)
		{
			printf("Error: frequency must be positive.\n");
			error++;
			goto exit;
		}
	}
	else
	{
		freqstream = bps_newstream(fpfreq,outprops.srate,&brkfreqSize);
		if (freqstream == NULL)
		{
			printf("Error: unable to obtain breakpoints from file \"%s\"\n",
			        argv[ARG_FREQ]);
			error++;
			goto exit;
		}
		if (bps_getminmax(freqstream,&minval,&maxval))
		{
			printf("Error reading range of breakpoint file \"%s\"\n",
			        argv[ARG_FREQ]);
			error++;
			goto exit;
		} 
		if (minval < 0.0 || maxval < 0.0)
		{
			printf("Error: breakpoint frequency values out of range in file \"%s\"\n"
			       "       freq >= 0.0\n",
			        argv[ARG_FREQ]);	
			error++;
			goto exit;
		}
	}

	/* check for valid amplitude factor data */
	fpampfac = fopen(argv[ARG_AMPFAC],"r");
	if (fpampfac == NULL)
	{
		printf("Error: data file \"%s\" does not exist.\n",
		        argv[ARG_AMPFAC]);
		error++;
		goto exit;
	}
	while (fgets(line,40,fpampfac))
	{
		ampfac = atof(line);
		if (ampfac <= 0.0 || ampfac > 1.0)
		{
			printf("Error: bad amplitude factor values in file \"%s\"\n"
			       "       0.0 < ampfac <= 1\n", argv[ARG_AMPFAC]); 
			error++;
			goto exit;
		}
		ampfacSize++;
	}
	/* reset file pointer for later use */
	rewind(fpampfac);
	
	/* check for valid frequency factor data */
	fpfreqfac = fopen(argv[ARG_FREQFAC],"r");
	if (fpfreqfac == NULL)
	{
		printf("Error: data file \"%s\" does not exist.\n",
		        argv[ARG_FREQFAC]);
		error++;
		goto exit;
	}
	while (fgets(line,40,fpfreqfac))
	{
		freqfac = atof(line);
		if (freqfac <= 0.0)
		{
			printf("Error: bad frequency factor values in file \"%s\"\n"
			       "       freqfac > 0.0\n", argv[ARG_FREQFAC]); 
			error++;
			goto exit;
		}
		freqfacSize++;
	}
	rewind(fpfreqfac);	

	if (ampfacSize != freqfacSize)
	{
		printf("Error: \"%s\" and \"%s\" must have\n"
		       "       the same number of data values\n",
		        argv[ARG_AMPFAC], argv[ARG_FREQFAC]);
		error++;
		goto exit;
	}
	noscs = ampfacSize;

	/* allocate space for buffer */
	nframes = DEFAULT_NFRAMES;
	buffer = (float*) malloc (outprops.chans * sizeof(float) * nframes);
	if (buffer == NULL)
	{
		puts("No memory!\n");
		error++;
		goto exit;
	}	

	/* calculate the number of output frames */
	outframes = (unsigned long) (dur * outprops.srate + 0.5);

	/* calculate the number of buffers */
	nbufs = (unsigned long) outframes/nframes;
	remainder = outframes - nbufs*nframes;
	if (remainder > 0)
		nbufs++; 

	/* create amp and freq arrays */
	oscamps = (double*) malloc (noscs * sizeof(double));
	if (oscamps == NULL)
	{
		puts("No memory!\n");
		error++;
		goto exit;
	}	

	/* create gtable arrays */
	gtable = (GTABLE **) malloc (noscs * sizeof(GTABLE *));	
	if (gtable == NULL)
	{
		printf("No memory!\n");
		error++;
		goto exit;
	}

	oscfreqs = (double*) malloc (noscs * sizeof(double));
	if (oscfreqs == NULL)
	{
		puts("No memory!\n");
		error++;
		goto exit;
	}

	/* create array of pointers to OSCILs */
	oscs = (OSCILT**) malloc (noscs * sizeof(OSCILT *));
	if (oscs == NULL)
	{
		puts("No memory!\n");
		error++;
		goto exit;
	}

	/* additive synthesis using five principle waveforms 
	   create and initialize wave tables */
	freqfac = 1.0;
	ampadjust = 0.0;
	switch (wavetype)
	{
		case (WAVE_SINE):
			for (i=0; i < noscs; i++)
			{
				gtable[i] = new_sine(width);
				if (gtable[i] == NULL)
				{
					printf("Error: unable to create sine wave table.\n");
					error++;
					goto exit;
				}
				ampfac = atof(fgets(line,40,fpampfac));
				oscamps[i] = ampfac;
				oscfreqs[i] = atof(fgets(line,40,fpfreqfac));
				ampadjust += ampfac;
			}
			break;
		case (WAVE_SQUARE):
			for (i=0; i < noscs; i++)
			{
				gtable[i] = new_square(width,nharms);
				if (gtable[i] == NULL)
				{
					printf("Error: unable to create square wave table.\n");
					error++;
					goto exit;
				}
				ampfac = atof(fgets(line,40,fpampfac));
				oscamps[i] = ampfac;
				oscfreqs[i] = atof(fgets(line,40,fpfreqfac));
				ampadjust += ampfac;
			}
			break;
		case (WAVE_TRIANGLE):
			for (i=0; i < noscs; i++)
			{
				gtable[i] = new_triangle(width,nharms);
				if (gtable[i] == NULL)
				{
					printf("Error: unable to create triangle wave table.\n");
					error++;
					goto exit;
				}
				ampfac = atof(fgets(line,40,fpampfac));
				oscamps[i] = ampfac;
				oscfreqs[i] = atof(fgets(line,40,fpfreqfac));
				ampadjust += ampfac;
			}
			phase = 0.25; /* default phase for triangle */
			break;
		case (WAVE_SAWUP):
		case (WAVE_SAWDOWN):
			for (i=0; i < noscs; i++)
			{
				if (wavetype == WAVE_SAWUP)
					gtable[i] = new_saw(width,nharms,SAW_UP);
		    else	
					gtable[i] = new_saw(width,nharms,SAW_DOWN);
				if (gtable[i] == NULL)
				{
					printf("Error: unable to create saw%s wave table.\n",
					        (wavetype==WAVE_SAWUP)?"up":"down");
					error++;
					goto exit;
				}
				ampfac = atof(fgets(line,40,fpampfac));
				oscamps[i] = ampfac;
				oscfreqs[i] = atof(fgets(line,40,fpfreqfac));
				ampadjust += ampfac;
			}
			break;
	}
	/* adjust amplitudes to add up to 1 */
	for (i=0; i < noscs; i++)
		oscamps[i] /= ampadjust;

	/* create and initialize each OSCILT */
	for (i=0; i < noscs; i++)
	{
		oscs[i] = new_oscilt(outprops.srate,gtable[i],phase);
		if (oscs[i] == NULL)
		{
			puts("No memory for oscillators.\n");
			error++;
			goto exit;
		}
	} 

	/* if specified, create and initialize
	   vibrato oscillator */
	if (isvib)
	{
		vibtable = new_sine(width);	
		if (vibtable == NULL)
		{
			printf("Error: unable to create vibrato wave table.\n");
			error++;
			goto exit;
		}
		vibosc = new_oscilt(outprops.srate,vibtable,phase);
		if (vibosc == NULL)
		{
			puts("No memory for vibrato oscillator.\n");
			error++;
			goto exit;
		}
	}

	printf("Creating soundfile...\n");

	/* process soundfile */
	framesread = 0;
	for (i=0; i < nbufs; i++)	
	{
		if ((i == (nbufs-1)) && remainder)
			nframes = remainder;
		/* update copy status after refreshing the buffer every 100 times */
		if ((i%100)==0)
			printf("%lu frames copied...  %d%%\r", framesread, (int)(100*framesread/outframes));
		/* clear update status when done */
		if (i==(nbufs-1))
			printf("                                                                       \r");
		for (j=0; j < nframes; j++)
		{
			long k;
			if (freqstream)
				freq = bps_tick(freqstream);
			if (ampstream)
				amp = bps_tick(ampstream);
			val = 0.0;
			if (isvib) /* add vibrato */
				vibsig = vibfac*tickfunc(vibosc,vibfreq);	
			for (k=0; k < noscs; k++)
				val += oscamps[k] * tickfunc(oscs[k], freq*oscfreqs[k] * (1 + vibsig));
			for (i_out=0; i_out < outprops.chans; i_out++)
				buffer[j*outprops.chans + i_out] = (float)(val * amp);
		}
		if (psf_sndWriteFloatFrames(ofd, buffer, nframes) != nframes)
		{
			printf("Error writing to outfile.\n");
			error++;
			break;
		}
		framesread += nframes;
	}

	printf("Done. %d error%s\n"
	       "File created:\t%s\n"
	       "Frames copied:\t%lu\n",
	        error, (error==1)?"":"s",
	        argv[ARG_OUTFILE], framesread
	       );	

	/* display outfile properties if successfully copied */
	if (!error)
		psf_sndInfileProperties(argv[ARG_OUTFILE], ofd, &outprops);

	// clean up resources
	exit:
	if (ofd>=0)
	{
		if (psf_sndClose(ofd))
		{
			printf("Error: unable to close soundfile: %s\n",
			        argv[ARG_OUTFILE]);
			error++;
		}
		else if (error)
		{
			printf("There was an error while processing the soundfile.\n"
			       "Deleting soundfile: %s\n", argv[ARG_OUTFILE]);
			if (remove(argv[ARG_OUTFILE]))
			{
				printf("Error: unable to delete %s\n", argv[ARG_OUTFILE]);
				error++;
			}
			else
				printf("%s successfully deleted\n", argv[ARG_OUTFILE]);
		}
	}
	if (ampstream)
		bps_freepoints(ampstream);
	if (freqstream)
		bps_freepoints(freqstream);
	if (buffer)
	{
		free(buffer);
		buffer = NULL;
	}
	if (fpamp)
	{
		if (fclose(fpamp))
		{
			printf("Error: unable to close breakpoint file: %s\n",
			        argv[ARG_AMP]);
		}
	}
	if (fpfreq)
	{
		if (fclose(fpfreq))
		{
			printf("Error: unable to close breakpoint file: %s\n",
			        argv[ARG_AMP]);
		}
	}
	if (fpampfac)
	{
		if (fclose(fpampfac))
		{
			printf("Error: unable to close data file: %s\n",
			        argv[ARG_AMPFAC]);
			error++;
		}
	}
	if (fpfreqfac)
	{
		if (fclose(fpfreqfac))
		{
			printf("Error: unable to close data file: %s\n",
			        argv[ARG_FREQFAC]); 
		}
		error++;
	}
	if (oscs)
	{
		free(oscs);
		oscs = NULL;
	}
	if (gtable)
		gtable_free(gtable);
	if (vibtable)
		gtable_free(&vibtable);
	if (oscamps)
	{
		free(oscamps);
		oscamps = NULL;
	}
	if (oscfreqs)
	{
		free(oscfreqs);
		oscfreqs = NULL;
	}
	if (vibosc)
	{
		free(vibosc);
		vibosc == NULL;
	}
	psf_finish();

	return error;
}
