#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <wave.h>
#include <gtable.h> 


/**** gtable function definitions ***/

/* GTABLE creation function for sine wave */
GTABLE* new_sine(unsigned long length)
{
	unsigned long i;
	double step;
	GTABLE* gtable = NULL;
	 
	if (length == 0)
		return NULL;
	gtable = (GTABLE* ) malloc (sizeof(GTABLE));
	if (gtable == NULL)
		return NULL;
	gtable->table = (double* ) malloc ((length + 1) * sizeof(double));
	if (gtable->table == NULL)
	{
		 free(gtable);
		 return NULL;
	}
	gtable->length = length;
	step = TWOPI / length; /* make sine wave */
	for (i=0; i < length; i++)
		gtable->table[i] = sin(step * i);
	gtable->table[i] = gtable->table[0]; /* guard point */
	return gtable;
 }

/* the GTABLE destruction function */
void gtable_free(GTABLE** gtable)
{
	if (gtable && *gtable && (*gtable)->table)
	{
		free ((*gtable)->table);
		free (*gtable);
		*gtable = NULL;
	}
}
