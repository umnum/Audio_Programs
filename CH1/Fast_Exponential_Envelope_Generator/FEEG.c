/* FEEG points levelBegin levelEnd releaseTime */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
const float sampleRate = 44100;
float coeff;
float currentLevel;
float currentTime;
float time;
void init(float, float, float);
inline void calculateEnvelope(int);

int main(int argc, char**argv)
{
	if (argc!=5)
	{
		fprintf(stderr,"ERROR: insufficient arguments\n");
		fprintf(stderr,"USAGE: FEEG points levelBegin levelEnd releaseTime\n");
		return 1;
	}
	int points = atoi(argv[1]);
	if (points <= 0)
	{
		fprintf(stderr,"ERROR: value for sample points must be positive.\n");
		return 1; 
	}
	float begin = atof(argv[2]);
	if (begin==0.0)
	{
		fprintf(stderr,"ERROR: beginning level value cannot be zero.\n");
		return 1;	
	}
	float end = atof(argv[3]);
	if (end==0.0)
	{
		fprintf(stderr,"ERROR: end level value cannot be zero.\n");
		return 1;
	}
	time = atof(argv[4]);	
	if (time<=0.0)
	{
		fprintf(stderr,"ERROR: time must have a positive value.\n");
		return 1;
	}
	if ((end-begin)==0.0)
		fprintf(stderr,"Warning: beginning and end level values are the same.\n");

	init(begin,end,time);
	calculateEnvelope(points);
	
	return 0;
}

void init(float levelBegin, float levelEnd, float releaseTime)
{
	currentTime = 0.0; 	
	currentLevel = levelBegin;
	coeff = (log(levelEnd)-log(levelBegin))/(releaseTime*sampleRate);
}

inline void calculateEnvelope(int samplePoints)
{
	int i;
	for (i=0; i<samplePoints; i++)
	{
		fprintf(stdout,"%0.4f\t%0.8f\n",currentTime,currentLevel); 
		currentTime += time/samplePoints; 
		currentLevel += coeff*currentLevel;
	}	
}

