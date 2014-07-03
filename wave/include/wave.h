/* wave.h header files */
#ifndef M_PI
#define m_PI (3.1415926535897932)
#endif
#define TWOPI (2.0 * M_PI) 

/*** type definition for OSCIL ***/
typedef struct t_oscil 
{
	double twopiovrsr; /* to hold a constant value 2PI/sr */
	double curfreq;
	double curphase;
	double incr;	
}	OSCIL;

/*** wave function prototypes ***/

/* a combined OSCIL creation and initialization function */
OSCIL* new_oscil(unsigned long srate);

/* sample generation for OSCIL type */
double sinetick(OSCIL* p_osc, double freq);
