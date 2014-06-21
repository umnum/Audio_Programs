/* ADEnvelope attackDuration decayDuration peakLevel points infile.txt*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char* argv[])
{
	FILE* fp;
	float verysmall = 1.0e-4; 
	float step;
	float duration;
	float begin;
	float end;
	float value;
	float fac;
	int err;
	int i;

	if (argc!=6)
	{
		fprintf(stderr,"ERROR: you must enter the correct number of arguments.\n");
		fprintf(stderr,"USAGE: ADEnvelope attackDuration decayDuration peakLevel points infile.txt\n");
		return 1;
	}		
	float attackDuration = atof(argv[1]);
	if (attackDuration <= 0.0)
	{
		fprintf(stderr,"ERROR: attack duration must be positive.\n");
		return 1;
	}
	float decayDuration = atof(argv[2]);
	if (decayDuration <= 0.0)
	{
		fprintf(stderr,"ERROR: decay duration must be positive.\n");
		return 1;
	}
	float peakLevel = atof(argv[3]);	
	if (peakLevel<=verysmall)
	{
		fprintf(stderr,"ERROR: peak level is not large enough.\n");
		return 1;
	}
	long points = atol(argv[4]);
	if (points<3)
	{
		fprintf(stderr,"ERROR: there must be at least three points.\n");
		return 1;
	}
		
	fp = fopen(argv[5],"w");
/*
	if (fp==NULL);
	{
		fprintf(stderr,"ERROR: unable to open \"%s\"\n", argv[5]);
		return 1;
	} */
	duration = attackDuration + decayDuration;
	for (i=0, step=0.0; step<attackDuration; i++, step+=duration/points);
	if (i<2)
	{
		fprintf(stderr,"ERROR: attack duration is not long enough.\n");
		return 1;
	}
	if ((points-i)<2)
	{
		fprintf(stderr,"ERROR: delay duration is not long enough.\n");
		return 1;
	}

	begin = verysmall;
	end = 1.0;
	fac = pow(end/begin,1.0/i);
	for (value=verysmall, step=0.0; step<attackDuration; step+=duration/points, value=begin*peakLevel)
	{
		err = fprintf(fp,"%0.4f\t%0.8f\n",step,value);	
		if (err<0)
		{
			fprintf(stderr,"ERROR: unable to write to \"%s\"\n",argv[5]);	
			return 1;
		}
		begin*=fac;
	}	
	begin = 1.0;
	end = verysmall;
	fac = pow(end/begin,1.0/(points-i));
	for (value=peakLevel; step<duration; step+=duration/points, value=begin*peakLevel) 
	{
		err = fprintf(fp,"%0.4f\t%0.8f\n",step,value); 
		if (err<0)
		{
			fprintf(stderr,"ERROR: unable to write to \"%s\"\n", argv[5]);
			return 1;
		}
		begin*=fac;
	}	
	fclose(fp);	
	return 0;
}
