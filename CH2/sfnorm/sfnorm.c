/* sfnorm.c: normalize soundfile */
#include <stdio.h>
#include <stdlib.h>
#include <portsf.h>
#include <math.h>
#define max(x,y) ((x) > (y) ? (x) : (y))

enum {ARG_PROGNAME, ARG_INFILE, ARG_OUTFILE, ARG_BUFF, ARG_DB, ARG_NARGS};

int main(int argc, char**argv)
{
	PSF_PROPS props;
	long framesread, totalread; 
	unsigned long blocksize;
	DWORD nFrames;	
	int size; 
	double dbval, thispeak, inpeak = 0.0;
	float ampfac, scalefac;
	
	/* init all resource vals to default states */ 
	int ifd=-1, ofd=-1;
	int error=0;
	psf_format outformat = PSF_FMT_UNKNOWN; 
	PSF_CHPEAK* peaks = NULL;
	float* buffer = NULL;

	printf ("SFNORM: normalize soundfile.\n");

	if (argc!=ARG_NARGS)
	{
		printf("insufficient arguments.\n"
					 "USAGE:\tsfnorm infile outfile buffer dBval\n" 
					 "dBval must be <= 0\n"); 
		return 1;
	}
	
	/* startup portsf */
	if(psf_init())
	{
		printf("ERROR: unable to start portsf\n");
		return 1;
	}

	/* initialize buffer */
	nFrames = (DWORD)atoi(argv[ARG_BUFF]);
	if (nFrames < 1)
	{
		printf("ERROR: buffer size must be at least 1\n");
		return 1;
	}

	/* initialize dBval */
	dbval = atof(argv[ARG_DB]);
	if (dbval > 0.0)
	{
		printf("ERROR: dBval cannot be positive.\n");
		return 1;
	} 
	/* convert from decibels to amplitude */
	ampfac = pow(10.0, dbval/20.0);

	/* open infile */ 
	ifd = psf_sndOpen(argv[ARG_INFILE], &props, 0);
	if (ifd<0)
	{
		printf("ERROR: unable to open infile \"%s\"\n",argv[ARG_INFILE]);
		return 1;
	}

	/*** we now have a resource, so we use goto hereafter
	     on hitting any error ***/

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
		printf("ERROR: sfnorm does not support 8-bit format.\n");
		error++;
		goto exit;
	}

	/* display infile properties to user */
	if(!psf_sndInfileProperties(argv[ARG_INFILE],ifd,&props))
	{
		error++;
		goto exit;
	}	

	/* check if outfile extension is one we know about */	
	outformat = psf_getFormatExt(argv[ARG_OUTFILE]);
	if (outformat == PSF_FMT_UNKNOWN)
	{
		printf("Outfile name \"%s\" has unknown format.\n"
					 "Use any of .wav .aiff .aif .afc .aifc\n",
					 argv[ARG_OUTFILE]);
		error++;
		goto exit;
	}
	props.format = outformat;

	/* allocate space for sample frames */
	buffer= (float*)malloc(props.chans*sizeof(float)*nFrames);
	if (buffer==NULL)
	{
		puts("No memory!\n");
		error++;
		goto exit;
	}

	/* and allocate space for PEAK info */
	peaks = (PSF_CHPEAK*)malloc(props.chans*sizeof(PSF_CHPEAK));
	if (peaks==NULL)
	{
		puts("No memory!\n");
		error++;
		goto exit;
	}

	/* get peak info: scan file if required */
	/* inpeak has been initialized to 0 */ 
	if (psf_sndReadPeaks(ifd,peaks,NULL)>0)
	{
		/* get peak info */
		long i;
		for (i=0; i<props.chans; i++)
		{
			inpeak = max(peaks[i].val,inpeak);
		}
	}
	else
	{
		/* scan file and rewind */
		framesread = psf_sndReadFloatFrames(ifd, buffer, nFrames); 
		blocksize = framesread*props.chans;
		while (framesread > 0)
		{
			blocksize = (unsigned long) framesread*props.chans;
			thispeak = maxsamp(buffer, blocksize);
			inpeak = max(thispeak,inpeak);
			framesread = psf_sndReadFloatFrames(ifd, buffer, nFrames);
		}

		/* rewind */
		if(psf_sndSeek(ifd,0,PSF_SEEK_SET))
		{
			printf("ERROR: cannot reset infile\n");
			error++;
			goto exit;
		}	
	}
	if (inpeak==0.0)
	{
		printf("infile is silent! Outfile not created.\n");
		goto exit;
	}

	scalefac = (float)ampfac/inpeak;

	/* create outfile */
	ofd = psf_sndCreate(argv[ARG_OUTFILE], &props, 0, 0, PSF_CREATE_RDWR);
	if (ofd<0)
	{
		printf("ERROR: unable to create outfile \"%s\"\n",argv[ARG_OUTFILE]);
		error++;
		goto exit;
	}

	printf("copying...\n");

	int update=0; 
	int i;
	int j;

	totalread = 0; /* running count of sample frames */

	/* nFrames loop to do copy, report any errors */
	framesread = psf_sndReadFloatFrames(ifd,buffer,nFrames);
	while (framesread>0)
	{
		update++;
		
		/* update copy status after refreshing the buffer every 100 times */
		if (update%100==0)
			printf("%ld samples copied...  %ld%\r",totalread,100*totalread/size);

		totalread+=framesread;
	
		/* normalize soundfile */ 	
		for (j=0; j<nFrames; j++)
			for (i=0; i<props.chans; i++)	
				buffer[props.chans*j+i] *= scalefac; 

		if(psf_sndWriteFloatFrames(ofd,buffer,nFrames)<0)
		{
			printf("Error writing to outfile.\n");
			error++;
			break;
		}

		framesread = psf_sndReadFloatFrames(ifd,buffer,nFrames);

	}

	if(framesread<0)
	{
		printf("Error reading infile. Outfile is incomplete.\n");
		error++;
	}
	else
		printf("Done. %ld sample frames copied to %s\n",
						totalread, argv[ARG_OUTFILE]);

	/* report PEAKS to user */
	if (psf_sndReadPeaks(ofd,peaks,NULL)>0)
	{
		long i;
		double peaktime;
		double peakDB;
		printf("Peak information:\n");
		for (i=0; i<props.chans; i++)
		{
			peaktime = (double)peaks[i].pos/props.srate; 
			if (peaks[i].val == 0.0)
				peaks[i].val = 1.0e-4;
			peakDB = log10(peaks[i].val);
			printf("CH %ld:\t%.4f\t(%.4f dB) at %.4f secs\n",
							i+1, peaks[i].val, peakDB, peaktime);	
		}
	}
	
	/* do all the cleanup */
	exit:
	if (ifd>=0)
		psf_sndClose(ifd);
	if (ofd>=0)
		psf_sndClose(ofd);
	if (buffer)
		free(buffer);
	if (peaks)
		free(peaks);
	psf_finish();
	 	
	return error;
}
