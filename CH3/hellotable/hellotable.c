/* hellotable: wavetable synthesizer, input/output specified by user */
#include <stdio.h>
#include <math.h>

#define SAMPLING_RATE 44100
#define PI 3.14159265 

#define TABLE_LEN 512
#define SINE 0 
#define SQUARE 1
#define SAW 2
#define TRIANGLE 3 

float table[TABLE_LEN];

#ifdef REALTIME /* uses the Tiny Audio Library */
#include "tinyAudioLib.h"
#elif defined(BINARY_RAW_FILE)
FILE* file;
#elif defined(WAVE_FILE) /* uses portsf library */
#include "portsf.h"
PSF_PROPS props;
int ofd;
#endif

/** outSample function prototype **/
void outSample(float);

/** cleanup function prototype **/
void cleanup(void);

/** init function prototype **/
void init(void);

/** wavetable function prototypes **/
void fill_sine(void);
void fill_square(void);
void fill_saw(void);
void fill_triangle(void);

void main()
{
	int waveform;
	const float frequency, duration;

	printf("Type the frequency of the output in Hz, "
	       "and press ENTER: ");
	scanf("%f", &frequency);

	printf("Type the duration of the tone in seconds, "
	       "and press ENTER: ");
	scanf("%f", &duration);
	
	wrong_waveform:
	printf("Type a number from 0 to 3 corresponding "
	       " to the waveform you intend to choose\n"
	       "(sine = 0, square = 1, saw = 2, triangle = 3)\n");
	scanf("%d", &waveform);	
	if (waveform < 0 || waveform > 3)
	{
		printf("Wrong number for waveform, try again!\n");
		goto wrong_waveform;
	}	

	/* ---------- FILL THE TABLE ------------*/
	switch (waveform)
	{
		case (SINE):
			printf("You have chosen the SINE wave\n");
			fill_sine();
			break;
		case (SQUARE):
			printf("You have chosen the SQUARE wave\n");
			fill_square();
			break;
		case (SAW):
			printf("You have chosen the SAW wave\n");
			fill_saw();
			break;
		case (TRIANGLE):
			printf("You have chosen the TRIANGLE wave\n");
			fill_triangle();
			break;
		default: /* Impossible! */
			printf("\nWrong wave!! Ending program.\n");
			return;
	}

	init();
	/*----------- SYNTHESIS ENGINE START -------------*/
	{
		int j;
		double sample_increment = frequency * TABLE_LEN / SAMPLING_RATE;	
		double phase = 0;
		float sample;

		for (j=0; j < duration * SAMPLING_RATE; j++)
		{
			sample = table[(long)phase];
			outSample(sample);
			phase += sample_increment;
			if (phase > TABLE_LEN)
				phase -= TABLE_LEN;
		}
	}
	/*----------- SYNTHESIS ENGINE END ----------------*/
	cleanup();
	printf("End of process.\n");
}


/** wavetable function definitions **/

/* fills the table with a single cycle of a sine wave */
void fill_sine()
{
	int j;
	for (j=0; j < TABLE_LEN; j++)
		table[j] = (float) sin(2 * PI * j / TABLE_LEN);	
}

/* fills the table with a single cycle of a square wave */
void fill_square()
{
	int j;
	for (j=0; j < TABLE_LEN/2; j++)
		table[j] = 1;
	for (j=TABLE_LEN/2; j < TABLE_LEN; j++)
		table[j] = -1;	
}

/* fills the table with a single cycle of a saw wave */
void fill_saw()
{
	int j;
	for (j=0; j < TABLE_LEN; j++)
		table[j] = 1 - (2 * (float) j / (float) TABLE_LEN);
}

/* fills the table with a single cycle of a triangle wave */
void fill_triangle()
{
	int j;
	for (j=0; j < TABLE_LEN/2; j++)
		table[j] = 2 * (float) j / (TABLE_LEN/2) - 1; 
	for (j=TABLE_LEN/2; j < TABLE_LEN; j++)
		table[j] = 1 - (2 * (float) (j-TABLE_LEN/2)/(float) (TABLE_LEN/2));
}

/* write samples to a user-controlled device */
void outSample(float sample)
{
	#ifdef REALTIME /* uses Tiny Audio Library */
	outSampleMono(sample);
	#elif defined(BINARY_RAW_FILE)
	short isample = (short) (sample * 32000);
	fwrite(&isample,sizeof(short),1,file);
	#elif defined(WAVE_FILE) /* uses portsf library */
	psf_sndWriteFloatFrames(ofd,&sample,1);
	#else /* standard output */
	printf("%f\n",sample);
	#endif	
}

/* open user specified input/output device */ 
void init()
{
	#ifdef REALTIME /* uses Tine Audio Library */
	tinyInit();
	#elif defined(BINARY_RAW_FILE)
	file = fopen("hellotable.raw","wb");
	#elif defined(WAVE_FILE) /* uses portsf library */
	props.srate = 44100;
	props.chans = 1;
	props.samptype = PSF_SAMP_16;
	props.format = PSF_STDWAVE;
	props.chformat = STDWAVE;
	psf_init();
	ofd = psf_sndCreate("hellotable.wav",&props,0,0,PSF_CREATE_RDWR);
	#else /* standard output */
	printf(". . . nothing to initialize . . .\n");
	#endif	
}

/* cleanup resources */
void cleanup()
{
	printf(". . . cleaning up . . .\n");
	#ifdef REALTIME /* uses Tine Audio Library */
	tinyExit();
	#elif defined(BINARY_RAW_FILE)
	fclose(file);
	#elif defined(WAVE_FILE) /* uses portsf library */
	{
		int err1, err2;
		err1 = psf_sndClose(ofd);
		err2 = psf_finish();
		if (err1 || err2)
			printf("Warning! An error occurred "
			       "writing WAVE_FILE file.\n");
	}
	#else /* standard output */
	printf("nothing to clean up . . .\n");	
	#endif	
}
