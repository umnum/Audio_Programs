/* mtable [-r] [-c] [-a] nFrom nTo [output.txt] */ 
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
  /* default flags for command line options */  
  int isrow=0, // creates a row lable if '-r' option is set   
			iscol=0, // creates a column lable if '-c' option is set  
			isapp=0, // appends to a specified output file if '-a' option  is set   
			iserr=0; // set to a negative value if there was an error writing to the output file   

	int nFrom, nTo, // multiplication table range of values  
			i, j;
 
	/* pointer to optional output file */ 
  FILE* fp;

	/* there are command line arguments */ 	
  while (argc > 1) 
  {
		/* test for command line options */ 
    if (argv[1][0] == '-') 
    { 
      if (argv[1][1] == 'r')
      { 
        isrow = 1; 
      }
      else if (argv[1][1] == 'c')
      {
        iscol = 1; 
      }
      else if (argv[1][1] == 'a') 
      { 
       isapp = 1;
      }
			else 
			{
				printf("ERROR: unrecognized option %s\n", argv[1]);
				return 1; 
			}
      argc--;
      argv++; 
    }
		else
			break; 
	} 

	/* test for the correct number of command line arguments */ 
	if (argc < 3)
	{ 
		printf ("ERROR:  There is an insufficient number of arguments.\n"); 
		printf ("USAGE: mtables [-r] [-c] [-a] nFrom nTo [output.txt]\n"); 
		return 1; 
	} 
  if (argc > 4)
	{
		printf ("ERROR:  There are too many arguments.\n");
		printf ("USAGE: mtables [-r] [-c] [-a] nFrom nTo [output.txt]\n"); 
		return 1;
	}
	
	int f1 = atoi(argv[1]);
  int f2 = atoi(argv[2]);

	/* test for the correct type of command line arguments */ 
	if (f1==0 || f2==0)
	{
		printf ("ERROR:  \"%s\" is not a valid argument\n", (f1)?argv[2]:argv[1]);
		printf ("USAGE: mtables [-r] [-c] [-a] nFrom nTo [output.txt]\n"); 
		return 1; 
	}  
	if (argc == 4)
	{
		if (argv[3][0] == '-')
		{
			printf("ERROR: \"%s\" is not a valid filename. Cannot start filenames with '-'.\n", argv[3]);
			printf ("USAGE: mtables [-r] [-c] [-a] nFrom nTo [output.txt]\n"); 
			return 1;
		}
	}

  // nFrom gets the smallest value, nTo gets the largest 
  nFrom = (f1<f2)?f1:f2;
  nTo = (f1>f2)?f1:f2;

	/* if an output file is specified, open the file */ 
  fp = NULL; 
  if (argc == 4)
  {
	  if (isapp)
		{ /* append to the output file */ 
		  fp = fopen(argv[3], "a");
			if (fp==NULL)
			{
				printf ("WARNING: Unable to create file %s\n", argv[3]); 
				perror ("");  
			}
		}
		else
		{ /* write to the output file */ 
			fp = fopen(argv[3], "w");
			if (fp==NULL)
			{
				printf ("WARNING: Unable to create file %s\n", argv[3]); 
				perror ("");  
			}
		}  
	}

	/* create a column lable for -c option */ 
  if (iscol)
	{
		if (isrow)
			printf("     ");
		for (i=1; i<=9; i++)
		{
			printf("%5d", i); 
		}
		printf("\n");
		if (isrow)
			printf("     "); 
		for (i=1; i<=9; i++) 
		{
			printf("-----");
		}
		printf("\n");
  } 

	for (i=nFrom; i<=nTo; i++)
	{
		if (fp)
		{
			for (j=1; j<=9; j++)
			{
				iserr = fprintf(fp, "%5d", i*j);	
			}
			iserr = fprintf(fp, "%c", '\n');
			if (iserr < 0)
				break; 
		} 
		
		/* create a row lable for -r option */ 
		if (isrow)
		{
			printf("%-5d", i);  
		}
		for (j=1; j<=9; j++) 
		{ 
				printf("%5d", i*j);
		} 	
		printf("\n");
	} 

	if (iserr < 0)
		perror("There was an error reading the file.\n");
	if (fp)
		fclose(fp);

	return 0;
} 
