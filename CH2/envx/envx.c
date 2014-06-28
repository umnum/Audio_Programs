/* envx.c: extract amplitude envelope from mono soundfile */ 
/* USAGE: envx infile outfile */ 
#include <stdio.h>
#include <stdlib.h>
#include <portsf.h>
#include <breakpoints.h>
#include <math.h>
#define NFRAMES 100

enum {ARG_PROGNAME, ARG_INFILE, ARG_OUTFILE, ARG_NARGS};

int main(int argc, char**argv)
{
	PSF_PROPS props;
	long framesread, totalread; 
	int size; 

	/* init all resource vals to default states */ 
	int ifd=-1; 
	int error=0;
	float* buffer = NULL;
	FILE* fp = NULL;

	printf ("ENVX: extract amplitude envelope from mono soundfile.\n");

	if (argc!=ARG_NARGS)
	{
		printf("insufficient arguments.\n"
					 "USAGE:\tenvx infile outfile\n");
		return 1;
	}

	/* startup portsf */
	if(psf_init())
	{
		printf("ERROR: unable to start portsf\n");
		return 1;
	}


	/* open infile */ 
	ifd = psf_sndOpen(argv[ARG_INFILE], &props, 0);
	if (ifd<0)
	{
		printf("ERROR: unable to open infile \"%s\"\n",argv[ARG_INFILE]);
		return 1;
	}

	/* we now have a resource, so we use goto hereafter
		 on hitting any error */

	/* get number of frames from infile */
	size = psf_sndSize(ifd);
	if(size<0)
	{
		printf("ERROR: unable to obtain the size of \"%s\"\n",argv[ARG_INFILE]);
		error++;
		goto exit;
	}
	
	/* check if infile uses 8-bit samples*/ 
	if (props.samptype==PSF_SAMP_8)
	{
		printf("ERROR: envx does not support 8-bit format.\n");
		error++;
		goto exit;
	}

	/* check if infile is mono */
	if (props.chans!=1)
	{
		printf("ERROR: infile must be mono.\n");
		error++;
		goto exit;
	}

	/* display infile properties */
	if(!psf_sndInfileProperties(argv[ARG_INFILE],ifd,&props))
	{
		error++;
		goto exit;
	}	

	/* create outfile */
	fp = fopen(argv[ARG_OUTFILE],"w");
	if (fp==NULL)
	{
		printf("ERROR: cannot open outfile: %s\n",argv[ARG_OUTFILE]);
		error++;
		goto exit;
	}

	/* allocate memory for infile buffer */
	buffer= (float*)malloc(props.chans*sizeof(float)*NFRAMES);
	if (buffer==NULL)
	{
		puts("No memory!\n");
		error++;
		goto exit;
	}

	printf("creating breakpoint file...\n");

	int update=0; 

	/* running count of sample frames */
	totalread = 0; 
		
	/* loop every time NFRAMES are copied, report any errors */
	while ((framesread = psf_sndReadFloatFrames(ifd,buffer,NFRAMES))>0)
	{
		update++;
		
		/* update infile reading status after refreshing the buffer every 100 times */
		if (update%100==0)
			printf("\r%ld samples read...  %ld%\r",totalread,100*totalread/size);

		totalread+=framesread;
	
		//TODO <<-- Process sound file and breakpoint file here -->>

	}

	if(framesread<0)
	{
		printf("Error reading infile. Outfile is incomplete.\n");
		error++;
	}
	else
		printf("Done.\n"
		       "sample frames read: %ld\n"
		       "breakpoint file created: %s\n",
						totalread, argv[ARG_OUTFILE]);

	/* do all the cleanup */
	exit:
	if (ifd>=0)
		psf_sndClose(ifd);
	if (fp)
		fclose(fp);
	if (buffer)
		free(buffer);
	psf_finish();
	 	
	return error;
}
