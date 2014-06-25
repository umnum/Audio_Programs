/* sfpan: changes the stereo panning position on a mono soundfile */
/* usage: sfpan infile outfile panpos */
#include <stdio.h>
#include <stdlib.h>
#include <portsf.h>
#define NFRAMES 100

enum {ARG_PROGNAME, ARG_INFILE, ARG_OUTFILE, ARG_PANPOS, ARG_NARGS};

int main(int argc, char**argv)
{
	double panpos;
	int framesread;
	PANPOS thispos;
	PSF_PROPS inprops, outprops;
	psf_format outformat = PSF_FMT_UNKNOWN;

	/* init all resourse vals to default states */
	int ifd=-1, ofd=-1;
	int error=0;
	float* inbuffer = NULL;
	float* outbuffer = NULL;

	if (argc!=ARG_NARGS)
	{
		printf("ERROR: insufficient number of arguments.\n"
		       "USAGE: sfpan infile outfile panpos\n"); 
		return 1;
	}

	panpos = atof(argv[ARG_PANPOS]);
	if ( (panpos < -1.0) || (panpos > 1.0) )
	{
		printf("ERROR: panpos value out of range -1 to 1\n");
		return 1;
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

	thispos = simplepan(panpos);
	while ((framesread = psf_sndReadFloatFrames(ifd,inbuffer,NFRAMES)) > 0)
	{
		int i, out_i; 
		for (i=0, out_i=0; i < framesread; i++)
		{
			outbuffer[out_i++] = (float)(inbuffer[i]*thispos.left);	
			outbuffer[out_i++] = (float)(inbuffer[i]*thispos.right);
		}
		if (psf_sndWriteFloatFrames(ofd,outbuffer,framesread) != framesread)
		{
			printf("Error writing to outfile.\n");
			error++;
			break;
		}
	}	
	
	printf("Done.\n");

	exit:
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
