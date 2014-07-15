/* TABGEN: a table lookup oscillator generator */
/* USAGE:  tabgen [-wN] [-t] outfile dur srate nchans amp freq type nharms */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <portsf.h>
#include <wave.h>
#include <gtable.h>
#include <breakpoints.h>
#define DEFAULT_NFRAMES 100 // frames in buffer
#define DEFAULT_WIDTH 1024  // table oscillator size

enum {ARG_PROGNAME, ARG_OUTFILE, ARG_DUR, ARG_SR, ARG_CHANS,
      ARG_AMP, ARG_FREQ, ARG_TYPE, ARG_NHARMS, ARG_NARGS};

enum {WAVE_SINE=4, WAVE_SAWUP, WAVE_SQUARE, WAVE_SAWDOWN, WAVE_TRIANGLE};

int main (int argc, char**argv)
{
	/* declare variables */
	PSF_PROPS outprops; // properties of output soundfile
	psf_format outformat = PSF_FMT_UNKNOWN; 
	double dur, freq, amp, peakdiff;
	float val;
	double minval, maxval, maxamp;
	int srate, nharms;
	unsigned long i, j;
	int i_out;
	int wavetype = -1;
	int chans;
	unsigned long width = DEFAULT_WIDTH;
	unsigned long nbufs, outframes, remainder, nframes, framesread;
	oscilt_tickfunc tickfunc = tabitick; // default table lookup is interpolating

	/* init resources */
	int ofd = -1;
	int error = 0;
	FILE *fpamp = NULL; // amplitude breakpoint file
	FILE *fpfreq = NULL; // frequency breakpoint file
	float* buffer = NULL;
	BRKSTREAM* ampstream = NULL; // breakpoint amplitude values
	BRKSTREAM* freqstream = NULL; // breakpoint frequency values
	GTABLE* gtable = NULL; // wave table with guard point
	OSCILT* p_osc = NULL; // oscillator using table lookup	
	unsigned long brkampSize = 0, // number of breakpoints
	              brkfreqSize = 0;
	
	printf("TABGEN: a table lookup oscillator generator.\n");

	/* check for command line options */
	if (argc > 1)
	{
		while (argv[1][0]=='-')
		{
			switch(argv[1][1])
			{
				case('\0'):
					printf("Error:   you did not specify an option.\n"
					       "options: -wN  set width of lookup table "
					       "to N points (default: 1024 points)\n"
					       "         -t   use truncating lookup "
					       "(default: interpolating lookup)\n");
					return 1;
				case('t'):
					if (argv[1][2] == '\0')
						tickfunc = tabtick;	
					else
					{
						printf("Error:   %s is not a valid option.\n"
						       "options: -wN  set width of lookup table "
						       "to N points (default: 1024 points)\n"
						       "         -t   use truncating lookup "
						       "(default: interpolating lookup)\n",
						        argv[1]);
						return 1;	
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
						       "options: -wN  set width of lookup table "
						       "to N points (default: 1024 points)\n"
						       "         -t   use truncating lookup "
						       "(default: interpolating lookup)\n",
						        argv[1]);
						return 1;	
			}
			argc--;
			argv++;
		}
	}

	/* check for sufficient command line input */
	if (argc != ARG_NARGS)
	{
		printf("Error: insufficient number of arguments.\n"
		       "Usage: tabgen [-wN] [-t] outfile dur srate nchans amp freq type nharms\n"
		       "       -wN:      w option sets the width of the lookup table to N points\n"
		       "                 N >= 1\n"
		       "       -t:       t option selects truncating table lookup\n"
		       "                 default is interpolating table lookup\n"
		       "       outfile:  output soundfile\n"
		       "                 set to 16-bit format\n"
		       "                 use any of .wav .aiff .aif .afc .aifc formats\n"
		       "       dur:      duration of soundfile (seconds)\n"
		       "       srate:    sample rate\n"
		       "       nchans:   number of channels\n"
		       "       amp:      amplitude value or breakpoint file\n"
		       "                 0.0 < amp <= 1.0\n"
		       "       freq:     frequency value or breakpoint file\n"
		       "                 freq >= 0.0\n"
		       "       type:     oscillator wave type\n"
		       "                 sine, square, triangle, sawup, sawdown\n"
		       "       nharms:   number of wave harmonics that are\n"
		       "                 added together in oscillator bank\n" 
		       "                 nharms >= 1\n"
		      );
		return 1;
	} 

	/* check duration input */
	dur = atof(argv[ARG_DUR]);
	if (dur <= 0.0)
	{
		printf("Error: duration must be positive.\n");
		return 1;
	}

	/* check sample rate input */
	srate = atoi(argv[ARG_SR]);
	if (srate <= 0)
	{
		printf("Error: sample rate must be positive.\n");
		return 1;		
	}
	outprops.srate = srate;

	/* check number of channels input */
	chans = atoi(argv[ARG_CHANS]);
	if (chans <= STDWAVE || chans > MC_WAVE_EX)
	{ 
		printf("Error: portsf does not support %d channels.\n",
		        chans);
		return 1;
	}
	outprops.chans = chans;

	/* check the output soundfile extension */
	outformat = psf_getFormatExt(argv[ARG_OUTFILE]);
	if (outformat == PSF_FMT_UNKNOWN)
	{
		printf("Error: outfile extension unknown.\n"
		       "       use any of .wav .aiff .aif .afc .aifc formats\n");
		return 1; 
	}	
	outprops.format = outformat;

	/* check for number of harmonics */
	nharms = atoi(argv[ARG_NHARMS]);
	if (nharms <= 0)
	{
		printf("Error: you must enter a positive number for harmonics.\n");
		return 1;
	} 
	
	/* get the wavetype */
	int count = 0;
	while (argv[ARG_TYPE][count] != '\0') count++;

	switch (count)
	{
		case (WAVE_SINE):
			if (!strcmp(argv[ARG_TYPE],"sine"))
				wavetype = WAVE_SINE;
			break;
		case (WAVE_SAWUP):
			if (!strcmp(argv[ARG_TYPE],"sawup"))
				wavetype = WAVE_SAWUP;
			break;
		case (WAVE_SQUARE):
			if (!strcmp(argv[ARG_TYPE],"square"))
				wavetype = WAVE_SQUARE;
			break;
		case (WAVE_SAWDOWN):
			if (!strcmp(argv[ARG_TYPE],"sawdown"))
				wavetype = WAVE_SAWDOWN;
			break;	
		case (WAVE_TRIANGLE):
			if (!strcmp(argv[ARG_TYPE],"triangle"))
				wavetype = WAVE_TRIANGLE;
			break;
		default:
			wavetype = -1;
	} 
	if (wavetype < 0) 
	{
		printf("Error:    %s is not a valid wave type.\n"
		       "wavetype: sine, square, triangle, sawup, sawdown\n",
		        argv[ARG_TYPE]); 
		return 1;
	}

	/* start portsf */
	psf_init();

	/* create output soundfile - set to 16-bit by default */
	outprops.samptype = PSF_SAMP_16;
	outprops.chformat = PSF_STDWAVE;
	ofd = psf_sndCreate(argv[ARG_OUTFILE],&outprops,0,0,PSF_CREATE_RDWR);
	if (ofd < 0)
	{
		printf("Error: unable to create soundfile \"%s\"\n",
		        argv[ARG_OUTFILE]);
		return 1;
	}

	/* resources have been gathered at this point
	   use goto upon hitting any errors */

	/* get amplitude value or breakpoint file */
	fpamp = fopen(argv[ARG_AMP],"r");		
	if (fpamp == NULL)
	{
		/* if the user specified a non-existant breakpoint file
		   or didn't specify a breakpoint value */
		if ( (argv[ARG_AMP][0] < '0' || argv[ARG_AMP][0] > '9') &&
		   (argv[ARG_AMP][0] != '.' && argv[ARG_AMP][0] != '-') ||
		   (argv[ARG_AMP][0] == '.' && (argv[ARG_AMP][1] < '0' || argv[ARG_AMP][1] > '9')) ||
		   (argv[ARG_AMP][0] == '-' && (argv[ARG_AMP][1] < '0' || argv[ARG_AMP][1] > '9') && argv[ARG_AMP][1] != '.') )
		{
			printf("Error: breakpoint file \"%s\" does not exist.\n",
			        argv[ARG_AMP]);
			error++;
			goto exit;
		}
		/* the user specified a breakpoint value */
		amp = atof(argv[ARG_AMP]);
		if (amp <= 0.0 || amp > 1.0)
		{
			printf("Error: amplitude value out of range.\n"
			       "       0.0 < amp <= 1.0\n");
			error++;
			goto exit;
		}
	}
	else
	{
		/* gather breakpoint values */
		ampstream = bps_newstream(fpamp,outprops.srate,&brkampSize);
		if (ampstream == NULL)
		{
			printf("Error reading breakpoint values from file \"%s\".\n",
			        argv[ARG_AMP]);
			error++;
			goto exit;			
		}
		if (bps_getminmax(ampstream,&minval,&maxval))
		{
			printf("Error: unable to read breakpoint range "
			       "from file \"%s\".\n", argv[ARG_AMP]);
			error++;
			goto exit;
		}
		if (minval <= 0.0 || minval > 1.0 || maxval <= 0.0 || maxval > 1.0)
		{
			printf("Error: breakpoint amplitude values out of range in file \"%s\"\n"
			       "       0.0 < amp <= 1.0\n", argv[ARG_AMP]);
			error++;
			goto exit; 
		}
		maxamp = maxval; // retain the maximum breakpoint amplitude value	
	}

	/* get frequency value or breakpoint file */
	fpfreq = fopen(argv[ARG_FREQ],"r");
	if (fpfreq == NULL)
	{
		if ( (argv[ARG_FREQ][0] < '0' || argv[ARG_FREQ][0] > '9') &&
		   (argv[ARG_FREQ][0] != '.' && argv[ARG_FREQ][0] != '-') ||
		   (argv[ARG_FREQ][0] == '.' && (argv[ARG_FREQ][1] < '0' || argv[ARG_FREQ][1] > '9')) ||
		   (argv[ARG_FREQ][0] == '-' && (argv[ARG_FREQ][1] < '0' || argv[ARG_FREQ][1] > '9') && argv[ARG_FREQ][1] != '.') )
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
			printf("Error reading breakpoing values from file \"%s\".\n",
			        argv[ARG_FREQ]);
			error++;
			goto exit;
		}	
		if (bps_getminmax(freqstream,&minval,&maxval))
		{
			printf("Error: unable to read breakpoint range "
			       "from file \"%s\".\n", argv[ARG_FREQ]);
			error++;
			goto exit;
		}
		if (minval < 0.0 || maxval < 0.0)
		{
			printf("Error: breakpoint frequency values out of range in file \"%s\"\n"
			       "       freq >= 0.0\n", argv[ARG_FREQ]);
			error++;
			goto exit;
		}
	}

	/* create an outfile buffer */	
	nframes = DEFAULT_NFRAMES;
	buffer = (float *) malloc (sizeof(float) * nframes * outprops.chans);
	if (buffer == NULL)
	{
		printf("No memory!\n");
		error++;
		goto exit;
	}

	/* calculate the total number of outfile frames */
	outframes = (unsigned long) (dur * outprops.srate + 0.5);	

	/* calculate the number of buffers used in outfile */
	nbufs = outframes / nframes;
	remainder = outframes - nframes * nbufs; 
	if (remainder > 0)
		nbufs++;

	/* allocate space for new wavetype table oscillator 
	   and initialize oscillator */
	switch (wavetype)
	{
		case (WAVE_SINE):
			gtable = new_sine(width); 
			if (gtable == NULL)
			{
				printf("Error: unable to create a sine wave table.\n");
				error++;
				goto exit;
			}
			p_osc = new_oscilt(outprops.srate,gtable,0); 
			break;
		case (WAVE_SQUARE):
			gtable = new_square(width,nharms);
			if (gtable == NULL)
			{
				printf("Error: unable to create a square wave table.\n");
				error++;
				goto exit;
			}
			p_osc = new_oscilt(outprops.srate,gtable,0); 
			break;
		case (WAVE_TRIANGLE):
			gtable = new_triangle(width,nharms);
			if (gtable == NULL)
			{
				printf("Error: unable to create a triangle wave table.\n");
				error++;
				goto exit;
			}
			p_osc = new_oscilt(outprops.srate,gtable,0.25); 
			break;
		case (WAVE_SAWUP):
		case (WAVE_SAWDOWN):
			if (wavetype==WAVE_SAWUP)
				gtable = new_saw(width,nharms,SAW_UP);
			else 
				gtable = new_saw(width,nharms,SAW_DOWN);
			if (gtable == NULL)
			{
				printf("Error: unable to create a saw%s wave table.\n",
				        (wavetype==SAW_UP)?"up":"down");
				error++;
				goto exit;
			}	
			p_osc = new_oscilt(outprops.srate,gtable,0);
			break;
	}	

	printf("Creating soundfile...\n");

	/* process soundfile */
	framesread = 0;
	for (i=0; i < nbufs; i++)	
	{
		/* update outfile copy status after the buffer
		   is refreshed every 100 times */
		if(i%100)
		{
			printf("%lu frames copied... %d%%\r",
			        framesread, (int)(framesread*100/outframes));
		} 
		/* clear update status when done */
		if (i==(nbufs-1))
			printf("                                        \n");

		if ((i == (nbufs-1)) && remainder)	
			nframes = remainder;
		for (j=0; j < nframes; j++)
		{
			if (ampstream)
				amp = bps_tick(ampstream);
			if (freqstream)
				freq = bps_tick(freqstream);
			val = tickfunc(p_osc,freq) * amp;	
			for (i_out=0; i_out < outprops.chans; i_out++)
				buffer[j*outprops.chans + i_out] = val;
		}
		if (psf_sndWriteFloatFrames(ofd,buffer,nframes) != nframes)
		{
			printf("Error: unable to write frames to outfile.\n");
			error++;
			break;
		}
		framesread += nframes;
	}

	/* make sure peak amplitude roughly matches the
	   user requested amplitude */
	if (ampstream)
		peakdiff = maxamp-psf_sndPeakValue(ofd,&outprops);
	else	
		peakdiff = amp-psf_sndPeakValue(ofd,&outprops);
	if ((peakdiff>.001) || (peakdiff<-.001))
	{
		printf("ERROR: unable to generate the correct peak\n"
		       "       amplitude for %s\n", argv[ARG_OUTFILE]);
		error++;
	}

	printf("Done. %d error%s\n"
	       "soundfile created: %s\n"
	       "number of frames:  %lu\n",
	        error, (error==1)?"":"s",
	        argv[ARG_OUTFILE], framesread);

	/* display soundfile properties if successfully copied */
	if (!error)
		psf_sndInfileProperties(argv[ARG_OUTFILE],ofd,&outprops);

	/* clean resources */
	exit:
	if (ofd >= 0)
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
				printf("%s successfully deleted.\n", argv[ARG_OUTFILE]); 
		}
	}
	if (fpamp)
	{
		if (fclose(fpamp))
		{
			printf("Error: unable to close breakpoint file \"%s\"\n",
			        argv[ARG_AMP]);	
		}
	} 
	if (fpfreq)
	{
		if (fclose(fpfreq))
		{
			printf("Error: unable to close breakpoint file \"%s\"\n",
			        argv[ARG_FREQ]);
		}
	}
	if (buffer)
	{
		free(buffer);
		buffer = NULL;
	}
	if (ampstream)
	{
		bps_freepoints(ampstream);
		ampstream = NULL;
	}
	if (freqstream)
	{
		bps_freepoints(freqstream);
		freqstream = NULL;
	}
	if (gtable)
		gtable_free(&gtable);
	if (p_osc)
	{
		free(p_osc);
		p_osc = NULL;
	}
	psf_finish();

	return error;
}
