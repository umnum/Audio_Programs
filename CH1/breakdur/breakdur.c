/* extract breakpoints from infile.txt,
   display the duration and maximum value of breakpoint file */ 
/* usage: breakdur infile.txt */
#include <stdio.h>
#include <stdlib.h>
#include <breakpoints.h>

int main(int argc, char *argv[])
{

	long size;
	double dur;
	BREAKPOINT point, *points;		
	FILE *fp;
	
	printf("breakdur: find duration of breakpoint file.\n"); 
	if (argc<2)
	{
		printf("USAGE: breakdur infile.txt\n");
		return 0;
	}	
	fp = fopen(argv[1],"r");
	if (fp==NULL)
	{
		printf("There was an error opening the file.\n");
		return 0;
	}	
	size = 0;
	points=NULL;
	points = get_breakpoints(fp,&size);
	if (points==NULL)
	{
		printf("No breakpoints read.\n");
		fclose(fp);
		return 1;
	} 	
	if (size<2)
	{
		printf("Error: At least two breakpoints are required.\n");
		free(points);
		points=NULL;
		fclose(fp);	
		return 1;
	}
	if (points[0].time != 0.0)
	{
		printf("Error in breakpoint data: first time must be 0.0\n");	
		free(points);
		points=NULL;
		fclose(fp);
		return 1;
	}
	printf("Read %ld breakpoints\n", size); 
	dur = points[size-1].time;
	printf("duration: %f seconds\n", dur);  
	point = maxpoint(points, size); 
	printf("maximum value %f at %f seconds\n", point.value, point.time);
	free(points);
	fclose(fp);
	return 0;
} 
