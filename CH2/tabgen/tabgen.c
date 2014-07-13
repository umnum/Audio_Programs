/* TABGEN: a table lookup oscillator generator */
/* USAGE:  tabgen [-wN] [-t] outfile dur srate nchans amp freq type nharms */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <portsf.h>
#include <wave.h>
#include <gtable.h>
#include <breakpoints.h>

enum {ARG_PROGNAME, ARG_OUTFILE, ARG_DUR, ARG_SR, ARG_CHANS,
      ARG_AMP, ARG_FREQ, ARG_TYPE, ARG_NHARMS, ARG_NARGS};

enum {WAVE_SINE=4, WAVE_SAWUP, WAVE_SQUARE, WAVE_SAWDOWN, WAVE_TRIANGLE, };

int main (int argc, char**argv)
{
	//TODO declare variables
	PSF_PROPS outprops;
	psf_format outformat = PSF_FMT_UNKNOWN; 
	double dur, freq, amp;
	int srate, nharms;
	int wavetype = -1;
	int chans;
	long int width = 1024;
	int istrunc = 0;
	unsigned long nbufs, outframes, remainder, nframes, framesread;

	//TODO init resources
	int ofd = -1;
	FILE *fpamp = NULL;
	FILE *fpfreq = NULL;
	float* buffer = NULL;
	BRKSTREAM* ampstream = NULL;
	BRKSTREAM* freqstream = NULL;
	OSCILT* p_osc = NULL;	

	//TODO check command line arguments, check options
	
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
						istrunc = 1;	
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
		       "Usage: tabgen [-wN] [-t] outfile dur srate nchans amp freq type nharms\n");
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
	if (wavetype < 0 )
	{
		printf("Error:    %s is not a valid wave type.\n"
		       "wavetype: sine, square, triangle, sawup, sawdown\n",
		        argv[ARG_TYPE]); 
		return 1;
	}

	//TODO allocate memory for resources

	//TODO select wavetype

	//TODO process soundfile

	//TODO update status

	//TODO clean resources
}
