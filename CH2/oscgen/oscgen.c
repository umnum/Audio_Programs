/* a basic oscillator bank for additive synthesis */
/* USAGE: oscgen outsndfile dur srate nchans amp freq wavetype noscs */
#include <stdio.h>
#include <stdlib.h>
#include <portsf.h>
#include <breakpoints.h>
#include <wave.h>

enum {ARG_PROGNAME, ARG_OUTFILE, ARG_DUR, ARG_SRATE, ARG_NCHANS,
      ARG_AMP, ARG_FREQ, ARG_TYPE, ARG_NOSCS, ARG_NARGS};

enum {WAVE_SQUARE, WAVE_TRIANGLE, WAVE_SAWUP, WAVE_SAWDOWN};

main (int argc, char* argv[])
{
	//TODO declare variables

	//TODO initialize resources

	printf("OSCGEN: basic oscillator bank for additive synthesis\n");

	//TODO gather/check command line arguments

	//TODO switch statement for selecting wavetype
	//     additive synthesis algorithm for each wavetype 

	//TODO allocate memory for resources

	//TODO process soundfile

	//TODO update status

	//TODO clean up resources
}
