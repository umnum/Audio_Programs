#include <stdio.h>
#include <stdlib.h>
#include <breakpoints.h>
#define NPOINTS 64 // maxpoint

/***** breakpoint functions *****/ 

/* extract breakpoint values from a text file */ 
BREAKPOINT* get_breakpoints(FILE *fp, long *psize)
{
	/* number of values exctracted from printf */ 
	int got;

	long npoints=0, size=64;

	double lasttime=0.0;
	BREAKPOINT *points = NULL;
	char line[80];	

	if (fp==NULL)
		return NULL;
	points = (BREAKPOINT*)(malloc(sizeof(BREAKPOINT)*size));
	if (points==NULL)
		return NULL; 

	/* extract each breakpoint from the file */ 
	while (fgets(line,80,fp))
	{
		got = sscanf(line,"%lf%lf",&points[npoints].time,&points[npoints].value); 
		if (got<0)
			continue; // skip initial white space
		if (got==0)
		{
			printf("Line %ld has non-numeric data.\n",npoints+1);
			break;
		}
		if (got==1)
		{
			printf("Incomplete breakpoint found at %ld\n",npoints+1);
			break;
		}
		if (points[npoints].time < lasttime) 
		{
			printf("Data error at point %ld: time not increasing.\n",npoints+1);
			break;	
		}

		lasttime=points[npoints].time;
		
		/* if allocated memory is full, expand the size of points */ 
		if (++npoints == size)
		{
			BREAKPOINT *tmp;
			size+=NPOINTS;
			tmp = (BREAKPOINT*)(realloc(points, sizeof(BREAKPOINT)*size));
			if (tmp==NULL)
			{
				npoints=0;
				free(points);
				points=NULL;
				break;
			}		
			points=tmp;
		}
	}
	if(npoints)
		*psize=npoints; 
	return points;
}

/* return the breakpoint with the largest value */
BREAKPOINT maxpoint(const BREAKPOINT *points, long npoints)
{
	int i;
	BREAKPOINT point; 
	point.time = points[0].time;
	point.value = points[0].value;
	for (i = 0; i < npoints; i++)
	{
		if (point.value < points[i].value)
		{
			point.value = points[i].value; 
			point.time = points[i].time;
		}
	}
	return point;
}
