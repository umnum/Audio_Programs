/* sfprop: reports the format inforation of a soundfile
	 usage: sfprop infile.wav */ 

#include <stdio.h>
#include <stdlib.h>
#include <portsf.h>
#include <math.h>

enum {ARG_PROGNAME, ARG_INFILE, ARG_NARGS};

int main (int argc, char**argv)
{
	int ifd=-1;
	int error=0;
	PSF_CHPEAK* peaks=NULL;	
	PSF_PROPS props;
	int size;
	char* samptype;
	char* format;
	char* chformat;

	if (argc!=ARG_NARGS)
	{
		printf("ERROR: insufficient number of arguments.\n"
					 "USAGE: sfprop infile.wav.\n");
		return 1;
	}

	/* start up portsf */
	if (psf_init())
	{
		printf("ERROR: unable to start portsf\n");
		return 1;
	}

	/* open infile */
	ifd = psf_sndOpen(argv[ARG_INFILE],&props,0);
	if (ifd<0)
	{
		printf("ERROR: unable to open \"%s\"\n",argv[ARG_INFILE]);
		return 1;
	}
	
	/**** from this point resources from infile have been gathered
		    make sure to delete resources when done. ****/ 	

	/* get sample size of infile */
	size = psf_sndSize(ifd);
	if (size < 0)
	{
		printf("ERROR: unable to obtain the size of \"%s\"\n",
					 argv[ARG_INFILE]);
		error++;
		goto exit;
	}

	/* allocate memory for peak values */
	peaks = (PSF_CHPEAK*)malloc(props.chans*sizeof(PSF_CHPEAK));
	if (peaks==NULL)
	{		
		printf("Not enough memory!\n");
		error++;
		goto exit;
	}
	
	/* find infile sample type */
	switch (props.samptype)
	{
		case(PSF_SAMP_8):
			samptype = "8-bit integer";
			break;
		case(PSF_SAMP_16):
			samptype = "16-bit integer";
			break;
		case(PSF_SAMP_24):
			samptype = "24-bit integer";
			break;
		case(PSF_SAMP_32):
			samptype = "32-bit integer";	
			break;
		case(PSF_SAMP_IEEE_FLOAT):
			samptype = "32-bit float";
			break;
		default:
			samptype = "unknown";
			break;
	}

	/* find infile format */ 
	switch (props.format)
	{
		case(PSF_STDWAVE):
			format = "WAV";
			break;
		case(PSF_WAVE_EX):
			format = "WAVFORMATEXTENSIBLE"; 
			break;
		case(PSF_AIFF):
			format = "AIFF";
			break;
		case(PSF_AIFC):
			format = "AIFC";
			break;
		default:
			format = "unknown";
	}

	/* find channel format */ 
	switch (props.chformat)
	{
		case(STDWAVE):
			chformat = "standard wave";
			break;
		case(MC_STD):
			chformat = "standard";
			break;
		case(MC_MONO):
			chformat = "mono";
			break;
		case(MC_STEREO):
			chformat = "stereo";
			break;
		case(MC_QUAD):
			chformat = "quad";
			break;
		case(MC_LCRS):
			chformat = "lrcs";
			break;
		case(MC_BFMT):
			chformat = "bfmt";
			break;
		case(MC_DOLBY_5_1):
			chformat = "dolby 5.1";
			break;
		case(MC_WAVE_EX):
			chformat = "wave format extensible";
			break;
		case(MC_SURR_5_0):
			chformat = "surround sound 5.0";
			break;
		case(MC_SURR_7_1):
			chformat = "surround sound 7.1";
			break;
		default:
			chformat = "unknown";
	}

	printf("\n%s properties:"
				 "\n--------------------------------\n"
				 "frame size:      %d\n"
				 "sample rate:     %d\n"
				 "channels:        %d\n"
				 "sample type:     %s\n"
				 "sample format:   %s\n"
				 "channel format:  %s\n",
				 argv[ARG_INFILE], size,
				 props.srate, props.chans, 
				 samptype, format, chformat);

	if (psf_sndReadPeaks(ifd, peaks, NULL) > 0)
	{
		long i;
		double peaktime;
		double peakDB;
		printf("PEAKS:\n");
		for (i=0; i<props.chans; i++)
		{
			peaktime = (double)peaks[i].pos/props.srate;
			if (peaks[i].val == 0.0)
				peaks[i].val = 1.0e-4;
			peakDB = log10(peaks[i].val);
			printf("\tCH %ld:\t%.4f\t(%.4f dB) at %.4f secs\n",
						 i+1, peaks[i].val, peakDB, peaktime); 
		}
	}
	
	exit:
	if (ifd>=0)
		psf_sndClose(ifd);
	if (peaks)
		free(peaks);
	psf_finish();

	return error;
} 
