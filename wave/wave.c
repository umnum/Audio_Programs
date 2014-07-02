#include <wave.h>
#include <math.h>

/** wave oscil function definitions **/

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
