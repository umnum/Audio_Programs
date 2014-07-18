/* wave.h */
#ifndef M_PI
#define m_PI (3.1415926535897932)
#endif
#define TWOPI (2.0 * M_PI) 
#define UPDATE_FREQ ( ( p_osc->curfreq!=freq) ? \
                        p_osc->curfreq = freq, \
                        p_osc->incr = p_osc->twopiovrsr * freq : 0 )

/*** type definition for OSCIL ***/
typedef struct t_oscil 
{
	double twopiovrsr; /* to hold a constant value 2PI/sr */
	double curfreq;
	double curphase;
	double incr;	
}	OSCIL;

/*** define a pointer to a tick function ***/
typedef double (*tickfunc)(OSCIL* osc, double freq);


/*** wave function prototypes ***/

/* a combined OSCIL creation and initialization function */
OSCIL* new_oscil(unsigned long srate);

/* a combined OSCIL creation and initialization function 
   with phase offset (0 <= phase <= 1) */
OSCIL* new_oscilp(unsigned long srate, double phase);

/* tick function for a sine waveform */
double sinetick(OSCIL* p_osc, double freq);

/* tick function for a square waveform */
double sqtick(OSCIL* p_osc, double freq);

/* tick function for a square waveform
with dynamic pulse wave modulation */
double pwmtick(OSCIL* p_osc, double freq, double pwmod);

/* tick function for a downward sawtooth waveform */
double sawdtick(OSCIL* p_osc, double freq);

/* tick function for an upward sawtooth waveform */
double sawutick(OSCIL* p_osc, double freq);

/* tick function for a triangle waveform */
double tritick(OSCIL* p_osc, double freq);
