/* envx.c: extract amplitude envelope from mono soundfile */ 
/* USAGE: envx [-wN] infile outfile */ 
#include <stdio.h>
#include <stdlib.h>
#include <portsf.h>
#include <breakpoints.h>
#include <math.h>
#define DEFAULT_WINDOW_MSECS 15

enum {ARG_PROGNAME, ARG_INFILE, ARG_OUTFILE, ARG_NARGS};

int main(int argc, char**argv)
{
	PSF_PROPS inprops;
	long framesread, /* number of frames copied to buffer */ 
	     totalread; /* running count of sample frames */
	unsigned long npoints; /* number of breakpoints generated */
	unsigned long winsize;
	char flag;
	double brktime; /* holds the time for the current breakpoint time */
	double windur = DEFAULT_WINDOW_MSECS;

	/* init all resource vals to default states */ 
	int ifd=-1; 
	int error=0;
	float* inbuffer = NULL;
	FILE* fp = NULL;

	printf ("ENVX: extract amplitude envelope from mono soundfile.\n");

	/* check for any options in command-line arguments */
	if (argc>1)
	{
		while (argv[1][0]=='-')
		{
			flag = argv[1][1];
			switch(flag)
			{
			/* TODO handle any flag arguments here */
				case('\0'):
					printf("ERROR: missing flag name.\n");
					return 1;
				case('w'):
					windur = atof(&argv[1][2]);
					if (windur <= 0.0)
					{
						printf("ERROR: bad value for window duration\n"
						       "       must be positive\n");
						return 1;	
					}
					break;
				default:
					break;
			}
			argc--;
			argv++;
		}
	}

	if (argc!=ARG_NARGS)
	{
		printf("ERROR:\tinsufficient arguments.\n"
					 "USAGE:\tenvx [-wN] insndfile outfile.brk\n"
		       "-wN: set extraction window size to N msecs.\n"
		       "(default: 15)\n"
		      );
		return 1;
	}

	/* startup portsf */
	if(psf_init())
	{
		printf("ERROR: unable to start portsf\n");
		return 1;
	}


	/* open infile */ 
	ifd = psf_sndOpen(argv[ARG_INFILE], &inprops, 0);
	if (ifd<0)
	{
		printf("ERROR: unable to open infile \"%s\"\n",argv[ARG_INFILE]);
		return 1;
	}

	/* we now have a resource, so we use goto hereafter
		 on hitting any error */

	/* check if infile uses 8-bit samples*/ 
	if (inprops.samptype==PSF_SAMP_8)
	{
		printf("ERROR: envx does not support 8-bit format.\n");
		error++;
		goto exit;
	}

	/* check if infile is mono */
	if (inprops.chans!=1)
	{
		printf("ERROR: infile has %d channels, must be mono.\n",inprops.chans);
		error++;
		goto exit;
	}

	/* display infile properties */
	if(!psf_sndInfileProperties(argv[ARG_INFILE],ifd,&inprops))
	{
		error++;
		goto exit;
	}	

	/* create output breakpoint file */
	fp = fopen(argv[ARG_OUTFILE],"w");
	if (fp==NULL)
	{
		printf("ERROR: unable to create breakpoint file: %s\n",argv[ARG_OUTFILE]);
		error++;
		goto exit;
	}

	/* set buffersize to the required envelope window size */
	windur /= 1000.0; /* convert to secs */
	winsize = (unsigned long)(windur * inprops.srate); /* number of frames */

	/* allocate memory for infile buffer */
	inbuffer= (float*)malloc(winsize * sizeof(float));
	if (inbuffer==NULL)
	{
		puts("No memory!\n");
		error++;
		goto exit;
	}

	printf("creating breakpoint file...\n");

	/* initialize running count of sample frames */
	totalread = 0; 
	/* initialize breakpoint time */
	brktime = 0.0;	
	/* initialize count of breakpoints */
	npoints = 0;

	/* loop every time winsize number of frames are copied, report any errors */
	while ((framesread = psf_sndReadFloatFrames(ifd,inbuffer,winsize))>0)
	{

		totalread+=framesread;
	
		/* extract breakpoint value */
		double amp;
		amp = maxsamp(inbuffer,framesread);
		
		/* write breakpoint time and value to outfile */
		if (fprintf(fp,"%f\t%f\n",brktime,amp)<2)
		{
			printf("Failed to write to breakpoint file: %s\n", argv[ARG_OUTFILE]);
			error++;
			goto exit;
		}

		npoints++;

		brktime += windur;
	}

	if(framesread<0)
	{
		printf("\nError reading infile. Outfile is incomplete.\n");
		error++;
	}
	else
		printf("\nDone: %d errors\n"
		       "breakpoint file created: %s\n"
		       "breakpoints written: %lu\n",
						error, argv[ARG_OUTFILE], npoints
		      );

	/* do all the cleanup */
	exit:
	if (ifd>=0)
		psf_sndClose(ifd);
	if (fp)
		if(fclose(fp))
			printf("\nenvx: failed to close breakpoint file: %s\n",argv[ARG_OUTFILE]);
		else if (error)
		{
			printf("\nThere was an error while processing the breakpoint file.\n"
			       "Deleting outfile: %s ...\n", argv[ARG_OUTFILE]);
			if (remove(argv[ARG_OUTFILE]))
				printf("Error: failed to delete %s\n", argv[ARG_OUTFILE]);
			else
				printf("%s successfully deleted.\n",argv[ARG_OUTFILE]);
		}
	if (inbuffer)
		free(inbuffer);
	psf_finish();
	 	
	return error;
}
