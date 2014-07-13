
/*** type definition of lookup table
     GTABLE  with guard point ***/

typedef struct t_gtable
{
	double* table; /* ptr to array containing the waveform */
	unsigned long length; /* excluding guard point */
} GTABLE;

/*** type definition of OSCILT derived from OSCIL ***/

typedef struct t_tab_oscil
{
	OSCIL osc;
	const GTABLE* gtable;
	double dtablen;
	double sizeovrsr;	
} OSCILT;

/*** define a pointer to an oscilt tick function */
typedef double (*oscilt_tickfunc)(OSCILT* osc, double freq);


/*** function prototypes ***/

/* GTABLE creation function for sine wave */
GTABLE* new_sine(unsigned long length);

/* the GTABLE destruction function */
void gtable_free(GTABLE** gtable);

/* OSCILT creation function */
OSCILT* new_oscilt(double srate, const GTABLE* gtable, double phase);

/* truncating GTABLE tick function */
double tabtick(OSCILT* p_osc, double freq);

/* interpolating GTABLE tick function */
double tabitick(OSCILT* p_osc, double freq);

/* fills table with triangle wave */
GTABLE* new_triangle(unsigned long length, unsigned long nharms);

/* creates a fully allocate but empty table */
GTABLE* new_gtable(unsigned long length);
