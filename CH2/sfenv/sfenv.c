/* sfenv.c: apply an amplitude envelope to a mono soundfle */ 
/* USAGE: sfenv insndfile infile.brk outsndfile */ 
#include <stdio.h>
#include <stdlib.h>
#include <portsf.h>
#include <breakpoints.h>
#include <math.h>
#define NFRAMES 100

enum {ARG_PROGNAME, ARG_INSNDFILE, ARG_INBRKFILE, ARG_OUTSNDFILE, ARG_NARGS};

int main(int argc, char**argv)
{
	PSF_PROPS inprops, outprops;
	long framesread, /* number of frames copied to buffer */ 
	     totalread; /* running count of sample frames */
	BREAKPOINT* points;
	unsigned long size; /* number of breakpoints */
	psf_format outformat = PSF_FMT_UNKNOWN;
	double timeincr, sampletime;

	/* init all resource vals to default states */ 
	int ifd=-1, ofd=-1; 
	int error=0;
	float* buffer = NULL;
	FILE* fp = NULL;

	printf ("SFENV: apply an amplitude envelope to a mono soundfile.\n");

	if (argc!=ARG_NARGS)
	{
		printf("ERROR:\tinsufficient arguments.\n"
					 "USAGE:\tsfenv insndfile infile.brk outsndfile\n"
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
	ifd = psf_sndOpen(argv[ARG_INSNDFILE], &inprops, 0);
	if (ifd<0)
	{
		printf("ERROR: unable to open infile \"%s\"\n",argv[ARG_INSNDFILE]);
		return 1;
	}

	/* we now have a resource, so we use goto hereafter
		 on hitting any error */

	/* check if infile uses 8-bit samples*/ 
	if (inprops.samptype==PSF_SAMP_8)
	{
		printf("ERROR: sfenv does not support 8-bit format.\n");
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
	if(!psf_sndInfileProperties(argv[ARG_INSNDFILE],ifd,&inprops))
	{
		error++;
		goto exit;
	}	

	/* open input breakpoint file */
	fp = fopen(argv[ARG_INBRKFILE],"r");
	if (fp==NULL)
	{
		printf("ERROR: unable to open breakpoint file: %s\n",argv[ARG_INBRKFILE]);
		error++;
		goto exit;
	}

	/* get breakpoint data */
	points = get_breakpoints(fp,&size);
	if (points==NULL)
	{
		printf("ERROR: no breakpoints read.\n");
		error++;
		goto exit;
	}	
	if (size<2)
	{
		printf("ERROR: at least two breakpoints required.\n");
		error++;
		goto exit;
	}
	/* make sure the first breakpoint starts at 0.0 */
	if (points[0].time!=0.0)
	{
		printf("Error in breakpoint data: first time must be 0.0\n");
		error++;
		goto exit;
	}

	/* outfile properties are identicle to infile properties */
	outprops = inprops;

	/* create outfile */
	ofd = psf_sndCreate(argv[ARG_OUTSNDFILE],&outprops,0,0,PSF_CREATE_RDWR);
	if (ofd<0)
	{
		printf("ERROR: unable to create \"%s\"\n", argv[ARG_OUTSNDFILE]);
		error++;
		goto exit;
	}

	/* check if oufile extension is one we know about */
	outformat = psf_getFormatExt(argv[ARG_OUTSNDFILE]); 
	if (outformat==PSF_FMT_UNKNOWN)
	{
		printf("Outfile name \"%s\" has unknown format.\n"
		       "Use any of .wav .aiff .aif .afc .aifc\n",
		       argv[ARG_OUTSNDFILE]
		      );
		error++;
		goto exit;
	}

	/* allocate memory for read write buffer */
	buffer= (float*)malloc(NFRAMES * sizeof(float));
	if (buffer==NULL)
	{
		puts("No memory!\n");
		error++;
		goto exit;
	}

	printf("copying outfile...\n");

	/* initialize running count of sample frames */
	totalread = 0; 
	
	/* initialize time position counter for reading envelope */
	timeincr = 1.0/inprops.srate;
	sampletime = 0.0;
	unsigned long pointnum = 1;

	/* loop every time NFRAMES are copied, report any errors */
	while ((framesread = psf_sndReadFloatFrames(ifd,buffer,NFRAMES))>0)
	{
		int i;

		totalread+=framesread;

		/* extract breakpoint values */
		double thisamp;

		for (i=0; i < framesread; i++, sampletime += timeincr)
		{
			thisamp = val_at_brktime(points,size,&pointnum,sampletime); 
			buffer[i] = (float)(buffer[i] * thisamp);
		}
		if (psf_sndWriteFloatFrames(ofd,buffer,framesread) != framesread)
		{
			printf("Error writing to outfile.\n");
			error++;
			break;	
		}
	}

	printf("\nDone: %d errors\n"
				 "soundfile created: %s\n"
				 "samples copied: %lu\n",
					error, argv[ARG_OUTSNDFILE], totalread 
				);

	/* do all the cleanup */
	exit:
	if (ifd>=0)
		psf_sndClose(ifd);
	if (ofd>=0)
		psf_sndClose(ofd);
	if (fp)
		if(fclose(fp))
			printf("sfenv: failed to close breakpoint file: %s\n",argv[ARG_INBRKFILE]);
	if (buffer)
		free(buffer);
	psf_finish();
	 	
	return error;
}
