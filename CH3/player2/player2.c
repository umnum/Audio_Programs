#include <stdio.h>
#include <math.h>
#include "tinyAudioLib.h"
#include "portsf.h"

void SYNTAX()
{
	printf("Syntax is: \nplayer2 [-tTIME] [-dDUR] filename\n");
}

#define FRAME_BLOCK_LEN 512
void main (int argc, char **argv)
{
	float buf[FRAME_BLOCK_LEN * 2]; /* buffer space for stereo
	                                   (and mono) */ 
	int sfd; /*audio file descriptor */
	int opened = 0; /* flag telling if audio file has been opened */
	PSF_PROPS props; /* struct filled by psf_sndOpen(),
	                    containing audio file info */
	long counter; /* counter of frames read */
	long length; /* length of file in frames */
	long endpoint; /* end point of playing */
	extern int arg_index; /* from crack.c */
	extern char *arg_option; /* from crack.c */
	extern int crack(int argc, char **argv, char *flags, int ign);
	int flag, timflag=9, durflag=0; /* flags */
	long nread; /* number of frames actually read */
	double startime, dur;

	while (flag = crack(argc, argv, "t|d|T|D|", 0))
	{
		switch (flag)
		{
			case 't':
			case 'T':
				if (*arg_option)
				{
					timflag = 1;
					startime = atof(arg_option);
				}
				else
				{
					printf("Error: -t flag set without "
								 "specifying a start time in seconds.\n");
					SYNTAX();
					return;
				}
				break;
			case 'd':
			case 'D':
				if (*arg_option)
				{
					durflag = 1;
					dur = atof(arg_option);
				}
				else
				{
					printf("Error: -d flag set without "
					       "specifying a duration in seconds.\n");
					SYNTAX();
					return;
				}
				break;
			case EOF:
				return;
		}
	}

	if (argc < 2) /* needs a command line argument */
	{
		printf("Error: Bad command line arguments\n");
		SYNTAX();
		return;
	}

	psf_init(); /* initialize portsf library */
	sfd = psf_sndOpen(argv[arg_index], &props, 0); /* open an audio file
	                                                  using portsf lib */ 
	if (sfd < 0) /* error condition */
	{
		printf("An error occurred opening audio file.\n");
		goto end;	
	}

	printf("file \'%s\' opened. . .\n", argv[arg_index]);
	printf("sampling rate: %d\n", props.srate);
	printf("number of chans: %d\n", props.chans);
	length = psf_sndSize(sfd);
	printf("duration: %f\n", (float) length / (float) props.srate);

	if (timflag)
		counter = (long) (startime * props.srate); /* length in frames */
	else
		counter = 0; /* beginning of file */
	
	if (durflag)
	{
		endpoint = (long) (dur * props.srate + counter);
		endpoint = (endpoint < length) ? endpoint : length;
	}
	else
	{
		endpoint = length; 
		dur = (double) (endpoint - counter) / (double) props.srate;	
	}	

	if (props.chans > 2)
	{
		printf("Invalid number of channels in audio file, "
		       "max 2 chans allowed.\n");
		goto end;
	}
	
	psf_sndSeek(sfd, counter, PSF_SEEK_SET); /* begin position at the
	                                            appropriate point */
	printf("Playing the file from the time position  0.3%lf to 0.3%lf "
	       "seconds. . .\n", startime, dur);

	/* ========= ENGINE =========*/
	do 
	{
		nread = psf_sndReadFloatFrames(sfd, buf, FRAME_BLOCK_LEN);
		if (props.chans == 2) /* stereo */
			outBlockInterleaved(buf, FRAME_BLOCK_LEN);
		else /* mono */
			outBlockMono(buf, FRAME_BLOCK_LEN);
		counter += FRAME_BLOCK_LEN;
	} while (counter < endpoint);
	/* ======== ENGINE END =========*/
	end:
	printf("Finished!\n");
	psf_sndClose(sfd);
	psf_finish();
}
