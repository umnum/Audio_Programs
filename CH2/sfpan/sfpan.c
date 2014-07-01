/* sfpan: changes the stereo panning position on a mono soundfile 
          by using values in a breakpoint file */
/* usage: sfpan infile outfile panpos */
#include <stdio.h>
#include <stdlib.h>
#include <portsf.h>
#include <breakpoints.h>
#define NFRAMES 100

enum {ARG_PROGNAME, ARG_INFILE, ARG_OUTFILE, ARG_BRKFILE, ARG_NARGS};

int main(int argc, char**argv)
{
	PANPOS pos;
	int framesread;
	unsigned long size;
	PSF_PROPS inprops, outprops;
	psf_format outformat = PSF_FMT_UNKNOWN;
	double timeincr, sampletime;

	/* init all resource vals to default states */
	int ifd=-1, ofd=-1;
	int error=0;
	float* inbuffer = NULL;
	float* outbuffer = NULL;
	FILE* fp = NULL;
	BREAKPOINT* points = NULL;

	if (argc!=ARG_NARGS)
	{
		printf("ERROR: insufficient number of arguments.\n"
		       "USAGE: sfpan infile outfile posfile.brk\n"
		       "       posfile.brk is a breakpoint file\n"
		       "       with values in range -1.0 <= pos <= 1.0\n"
		       "       where -1.0 = full Left, 0 = Center, +1.0 = full Right\n" 
		      );
		return 1;
	}

	/* read breakpoint file and verify it */
	fp = fopen(argv[ARG_BRKFILE],"r");
	if (fp==NULL)
	{
		printf("ERROR: unable to open breakpoint file \"%s\"\n", argv[ARG_BRKFILE]);
		return 1;
	}
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
	/* we require breakpoints to start from 0 */
	if (points[0].time!=0.0)
	{
		printf("Error in breakpoint data: first time must be 0.0\n");
		error++;
		goto exit;
	}
	/* check if breakpoint values are in range */
	if(!inrange(points,-1,1,size))
	{
		printf("Error in breakpoint data: values out of range -1 to +1\n");
		error++;
		goto exit;
	}

	/* start up portsf */
	if (psf_init())
	{
		printf("ERROR: unable to start portsf.\n");
		return 1;
	}

	/* open infile */
	ifd = psf_sndOpen(argv[ARG_INFILE],&inprops,0);
	if (ifd<0)
	{
		printf("ERROR: unable to open \"%s\"\n",argv[ARG_INFILE]);
		return 1;
	}

	/* we have a resource, we use goto hereafter 
	   upon hitting any errors */

	/* check if infile is 8-bit */
	if (inprops.samptype==PSF_SAMP_8)
	{
		printf("ERROR: portsf does not support 8-bit soundfiles.\n");
		error++;
		goto exit;
	} 

	/* check if infile is in mono */
	if (inprops.chans!=1)
	{
		printf("ERROR: infile must be mono.\n");
		error++;
		goto exit;
	}


	/* properties of infile and outfile will be the same except
	   infile is mono and outfile is stereo */
	outprops = inprops;
	outprops.chans = 2;

	ofd = psf_sndCreate(argv[ARG_OUTFILE],&outprops,0,0,PSF_CREATE_RDWR);
	if (ofd<0)
	{
		printf("ERROR: unable to create \"%s\"\n",argv[ARG_OUTFILE]);
		error++;
		goto exit;
	} 

	/* check if outfile extension is one we know about */
	outformat = psf_getFormatExt(argv[ARG_OUTFILE]);
	if (outformat==PSF_FMT_UNKNOWN)
	{
		printf("Outfile name \"%s\" has unknown format.\n"
		       "Use any of .wav .aiff .aif .afc .aifc\n",
		        argv[ARG_OUTFILE]);
		error++;
		goto exit;
	} 

	/* allocate space for input frame buffer */
	inbuffer = (float*)malloc(sizeof(float)*inprops.chans*NFRAMES);
	/* allocate space for output frame buffer */
	outbuffer = (float*)malloc(sizeof(float)*outprops.chans*NFRAMES);

	/* init time position counter for reading envelope */
	timeincr = 1.0 / inprops.srate;
	sampletime = 0.0;	
	unsigned long pointnum=1;

	while ((framesread = psf_sndReadFloatFrames(ifd,inbuffer,NFRAMES)) > 0)
	{
		int i, out_i; 
		double stereopos;

		for (i=0, out_i=0; i < framesread; i++) 
		{
			stereopos = val_at_brktime(points,size,&pointnum,sampletime);
			pos = constpower(stereopos);
			outbuffer[out_i++] = (float)(inbuffer[i]*pos.left);	
			outbuffer[out_i++] = (float)(inbuffer[i]*pos.right);
			sampletime += timeincr;
		}
		if (psf_sndWriteFloatFrames(ofd,outbuffer,framesread) != framesread)
		{
			printf("Error writing to outfile.\n");
			error++;
			break;
		}
	}	
	
	printf("Done.\n");

	/* clean up resources */
	exit:
	if (fp)
		fclose(fp);
	if (points)
		free(points);
	if (ifd)
		psf_sndClose(ifd);
	if (ofd)
		psf_sndClose(ofd);
	if (inbuffer)
		free(inbuffer);
	if (outbuffer)
		free(outbuffer);
	psf_finish();	

	return error;
}
