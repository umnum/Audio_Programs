#include <stdio.h>
#include <stdlib.h>
#include <wave.h>
#include <math.h>

/** wave oscil function definitions **/

/* a combined OSCIL creation and initialization function */
OSCIL* new_oscil(unsigned long srate)
{
	OSCIL* p_osc;	
	p_osc = (OSCIL*)malloc(sizeof(OSCIL));
	if (p_osc==NULL)
		return NULL;
	p_osc->twopiovrsr = TWOPI / (double) srate;
	p_osc->curfreq = 0.0;
	p_osc->curphase = 0.0;
	p_osc->incr = 0.0;

	return p_osc;
} 

/* sample generation for OSCIL type */
double sinetick(OSCIL* p_osc, double freq)
{
	double val;
	
	val = sin(p_osc->curphase);
	if (p_osc->curfreq!=freq)
	{
		p_osc->curfreq = freq;
		p_osc->incr = p_osc->twopiovrsr * freq;
	}
	p_osc->curphase += p_osc->incr;
	if (p_osc->curphase >= TWOPI)
		p_osc->curphase -= TWOPI;
	if (p_osc->curphase < 0.0)
		p_osc->curphase += TWOPI;
	
	return val;
}
