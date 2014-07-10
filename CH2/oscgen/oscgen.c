/* a basic oscillator bank for additive synthesis */
/* USAGE: oscgen outsndfile dur srate nchans amp freq wavetype noscs */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <portsf.h>
#include <breakpoints.h>
#include <wave.h>
#define NFRAMES 100 // default frames for buffer

enum {ARG_PROGNAME, ARG_OUTFILE, ARG_DUR, ARG_SRATE, ARG_CHANS,
      ARG_AMP, ARG_FREQ, ARG_TYPE, ARG_NOSCS, ARG_NARGS};

enum {WAVE_SQUARE, WAVE_TRIANGLE, WAVE_SAWUP, WAVE_SAWDOWN};

main (int argc, char* argv[])
{
	//TODO declare variables
	PSF_PROPS outprops; /* soundfile properties */
	double dur, amp, freq, peakdiff; 
	double minval, maxval;
	int chans, srate, noscs;
	int wavetype = -1;
	psf_format format = PSF_FMT_UNKNOWN;

	//TODO initialize resources
	int ofd = -1;
	int error = 0;
	FILE* fpamp = NULL;
	FILE* fpfreq = NULL;
	float* buffer = NULL;	
	OSCIL* p_osc = NULL; 
	BRKSTREAM* ampstream = NULL;
	BRKSTREAM* freqstream = NULL;
	OSCIL **oscs = NULL;
	double *oscamps = NULL,
	       *oscfreqs = NULL;
	unsigned long brkampSize = 0,
	              brkfreqSize = 0;

	printf("OSCGEN: basic oscillator bank for additive synthesis\n");

	/* check for correct number of arguments */
	if (argc!=ARG_NARGS)
	{
		printf("ERROR: insufficient number of arguments.\n"
		       "USAGE: oscgen outsndfile dur srate nchans amp freq wavetype noscs\n"
		       "outsndfile: output soundfile supported by portsf\n"
		       "            use any of .wav .aiff .aif .afc .aifc formats\n" 
		       "dur:        duration of soundfile (seconds)\n"
		       "srate:      soundfile sample rate (srate > 0)\n"
		       "nchans:     soundfile number of channels\n"
		       "amp:        amplitude value or breakpoint file\n"
		       "            (0 < amp <= 1)\n"
		       "freq:       frequency value or breakpoint file\n"
		       "            (frequency >= 0)\n"	
		       "wavetype:   square, triangle, sawtooth_up, sawtooth_down\n"
		       "noscs:      number of oscillators in the oscillator bank\n"
		      );
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
		case(6):
			if (!strcmp(argv[ARG_TYPE],"square"))
				wavetype = WAVE_SQUARE;
			break;
		case(8):
			if (!strcmp(argv[ARG_TYPE],"triangle"))
				wavetype = WAVE_TRIANGLE;
			break;
		case(11):
			if (!strcmp(argv[ARG_TYPE],"sawtooth_up"))
				wavetype = WAVE_SAWUP;
				break;
		case(13):
			if (!strcmp(argv[ARG_TYPE],"sawtooth_down"))
				wavetype = WAVE_SAWDOWN;
			break;
		default:
			wavetype = -1;
	}	
	if (wavetype < 0)
	{
		printf("ERROR:    %s is not a valid wavetype.\n"
		       "wavetype: square, triangle, sawtooth_up, sawtooth_down\n",
		        argv[ARG_TYPE]); 
		return 1;
	}

	/* get the number of oscillators */
	noscs = atoi(argv[ARG_NOSCS]);
	if (noscs < 1)
	{
		printf("ERROR: you must choose at least one oscillator.\n");
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

	//TODO allocate memory for resources
	
	/* allocate space for buffer */
	buffer = (float*) malloc (outprops.chans * sizeof(float) * NFRAMES);
	if (buffer == NULL)
	{
		puts("No memory!\n");
		error++;
		goto exit;
	}	

	/* create amp and freq arrays */
	oscamps = (double*) malloc (noscs * sizeof(double));
	if (oscamps == NULL)
	{
		puts("No memory!\n");
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
	oscs = (OSCIL**) malloc (noscs * sizeof(OSCIL *));
	if (oscs == NULL)
	{
		puts("No memory!\n");
		error++;
		goto exit;
	}

	//TODO initialize arrays... 
	//     switch statement for selecting wavetype
	//     additive synthesis algorithm for each wavetype 

	/* create each OSCIL */
	int i;
	for (i=0; i < noscs; i++)
	{
		oscs[i] = new_oscil(outprops.srate);
		if (oscs[i] == NULL)
		{
			puts("No memory for oscillators.\n");
			error++;
			goto exit;
		}
	} 

	//TODO process soundfile

	//TODO update status

	// clean up resources
	exit:
	if (ofd>=0)
	{
		if (psf_sndClose(ofd))
		{
			printf("Error: unable to close soundfile: %s\n",
			        argv[ARG_OUTFILE]);
		}
		else if (error)
		{
			printf("There was an error while processing the soundfile.\n"
			       "Deleting soundfile: %s\n", argv[ARG_OUTFILE]);
			if (remove(argv[ARG_OUTFILE]))
				printf("Error: unable to delete %s\n", argv[ARG_OUTFILE]);
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
	if (p_osc)
	{
		free(p_osc);
		p_osc = NULL;
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
	if (oscs)
	{
		free(oscs);
		oscs = NULL;
	}
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
	psf_finish();

	return error;
}
