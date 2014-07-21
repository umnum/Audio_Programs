#include <stdio.h>
#include "tinyAudioLib.h"
#include "portsf.h"

#define FRAME_BLOCK_LEN 512

void main (int argc, char**argv)
{
	float buf[FRAME_BLOCK_LEN * 2]; /* buffer space for stereo
	                                   (and mono) */
	
	int sfd; /* audio file descriptor */
	PSF_PROPS props; /* struct filled by psf_sndOpen(), 
	                    containing audio file info */
	
	long nread; /* number of frames actually read */

	if (argc != 2) /* needs a command line argument */
	{
		printf("Error: bad command line. Syntax is:\n\n");
		printf("player filename\n");
		return;
	}	

	psf_init(); /* initialize portsf library */
	sfd = psf_sndOpen(argv[1], &props, 0); /* open an audio file
	                                          using portsf lib */
	if (sfd < 0) /* error condition */
	{
		printf("An error occured opening audio file\n");
		goto end;
	}
	if (props.chans > 2)
	{
		printf("Invalid number of channels in audio file\n");
		goto end;
	}	

	/*======== ENGINE =========*/
	do
	{
		nread = psf_sndReadFloatFrames(sfd, buf, FRAME_BLOCK_LEN);
		if (props.chans == 2) /* stereo */
			outBlockInterleaved(buf, FRAME_BLOCK_LEN);
		else /* mono */
			outBlockMono(buf, FRAME_BLOCK_LEN);
	} while (nread == FRAME_BLOCK_LEN);
	/*====== ENGINE END =======*/
	end:
	printf("Finished!\n");
	psf_sndClose(sfd);
	psf_finish();
}
