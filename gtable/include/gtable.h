
/*** type definition of lookup table
     GTABLE  with guard point ***/

typedef struct t_gtable
{
	double* table; /* ptr to array containing the waveform */
	unsigned long length; /* excluding guard point */
} GTABLE;


/*** function prototypes ***/

/* GTABLE creation function for sine wave */
GTABLE* new_sine(unsigned long length);

/* the GTABLE destruction function */
void gtable_free(GTABLE** gtable);
