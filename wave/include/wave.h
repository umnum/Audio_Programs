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


