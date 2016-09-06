/*
 * thumctl.c
 * Source code for thumctl utility program
 * Must link with libTHUM.so
 * $Revision: 1.4 $
 * $Name:  $
 * $Date: 2006/09/17 23:12:48 $
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <asm/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <linux/hiddev.h>

#include "THUM.h"

int main (int argc, char **argv) {
	

/* parse args */
	opterr = 0;
	
	int opt;
	int verbose = THUM_FALSE;

	int H = 0, 
		V = 0, 
		v = 0,
		f = 0,
		c = 0,
		t = 0,
		h = 0,
		d = 0,
		s = 0,
		s2 = 0,
		s3 = 0,
		x = 0,
		i = 0,
		u = 0,
		p = 0;

	double tValue = 0.0,
		   hValue = 0.0,
		   dValue = 0.0,
		   sValue = 0.0,
		   s2Value = 0.0,
		   s3Value = 0.0,
		   xValue = 0.0,
		   iValue = 0.0,
		   uValue = 0.0;

	char tempUnit = 'C';
	
	while( (opt = getopt(argc, argv, "HVvfcthds23xiup:")) != -1){
		switch(opt){
			case 'H':
				/* print usage */
				H = 1;
				break;
			case 'V':
				/* print version */
				V = 1;
				break;
			case 'v':
				/* toggle verbosity on */
				v = 1;
				break;
			case 'f':
				/* set degrees f */
				f = 1;
				break;
			case 'c':
				/* set degrees c */
				c = 1;
				break;
			case 't':
				/* display temp */
				t = 1;
				break;
			case 'h':
				/* display humidity */
				h = 1;
				break;
			case 'd':
				/* display dew point */
				d = 1;
				break;
			case 's':
				/* display switch vale */
				s = 1;
				break;
			case '2':
				/* display switch2 vale */
				s2 = 1;
				break;
			case '3':
				/* display switch3 vale */
				s3 = 1;
				break;
			case 'x':
				/* display external temp */
				x = 1;
				break;
			case 'i':
				/* display IR reading */
				i = 1;
				break;
			case 'u':
				/* display ultrasonic reading */
				u = 1;
				break;
			default:
				fprintf(stderr, "Unknown option specified\n");
				return 1;
		}
	}	

	/* If -H or -V is supplied, display requested information and exit */
	if(H == 1){
		printf("Usage:\n");
		printf("%s <options> where <options> is one or more of the following:\n", argv[0]);
		printf("\t-H  print this message\n");
		printf("\t-V  print THUM library version\n");
		printf("\t-v  use verbose output\n");
		printf("\t-f  display temperatures in degrees F\n");
		printf("\t-c  display temperatures in degrees C\n");
		printf("\t-t  take a temperature reading\n");
		printf("\t-h  take a relative humudity reading\n");
		printf("\t-d  take a dew point reading\n");
		printf("\t-s  take a switch reading\n");
		printf("\t-2  take a switch reading from switch 2\n");
		printf("\t-3  take a switch reading from switch 3\n");
		printf("\t-x  take an external temperature reading\n");
		printf("\t-i  take an IR proximity reading\n");
		printf("\t-u  take an ultrasonic reading\n");
		printf("\n");
		return 0;
	}

	if(V == 1){
		char ver[THUM_MAXSTRLEN];
		GetVersion(ver);
		printf("thumctl version %s\n", ver);
		return 0;
	}
    
	/* Otherwise perform requested tests */
	
	if(v == 1){
		verbose = THUM_TRUE;
		printf("Enabling verbose output\n");
		SetVerbose(1);
	}

	/* Initialize device */
    /* Removing as each Read*() function does this anyway... */
    /*
	if(verbose){
		printf("Finding and initializing THUM\n");
	}
	if(FindHID() == THUM_FALSE){
		fprintf(stderr, "Couldn't find the THUM\n");
		return -1;
	}
	if(verbose){
		printf("Found and initialized the THUM\n");
	}
    */

	/* if no args specified, read everything using degrees C */
	if(argc == 1){
		c = t = h = d = s = x = i = u = 1;
	}
	
	if(f == 1){
		if(verbose){
			printf("Setting temperature unit to F\n");
		}
		if(SetTempUnit('F') != THUM_ERROR_SUCCESS){
			fprintf(stderr, "Error setting temperature unit\n");
		}
		tempUnit = 'F';
	}
	
	if(c == 1){
		if(verbose){
			printf("Setting temperature unit to C\n");
		}
		if(SetTempUnit('C') != THUM_ERROR_SUCCESS){
			fprintf(stderr, "Error setting temperature unit\n");
		}
		tempUnit = 'C';
	}

	if(t == 1){
		if(verbose){
			printf("Taking internal temperature reading\n");
		}
		if(ReadTemp() != THUM_ERROR_SUCCESS){
			fprintf(stderr,"Reading failed\n");
			return 1;
		}
		tValue = GetTemp();
	}

	if(h == 1){
		if(verbose){
			printf("Taking relative humidity reading\n");
		}
		if(ReadRH() != THUM_ERROR_SUCCESS){
			fprintf(stderr,"Reading failed\n");
			return 1;
		}
		hValue = GetRH();
	}

	if(d == 1){
		if(verbose){
			printf("Taking dew point reading\n");
		}
		if(ReadDewPt() != THUM_ERROR_SUCCESS){
			fprintf(stderr,"Reading failed\n");
			return 1;
		}
		dValue = GetDewPt();
	}
	
	if(s == 1){
		if(verbose){
			printf("Taking switch reading\n");
		}
		if(ReadSwitch() != THUM_ERROR_SUCCESS){
			fprintf(stderr,"Reading failed\n");
			return 1;
		}
		sValue = GetSwitch();
	}
		
	if(s2 == 1){
		if(verbose){
			printf("Taking switch2 reading\n");
		}
		if(ReadSwitch2() != THUM_ERROR_SUCCESS){
			fprintf(stderr,"Reading failed\n");
			return 1;
		}
		s2Value = GetSwitch2();
	}

	if(s3 == 1){
		if(verbose){
			printf("Taking switch3 reading\n");
		}
		if(ReadSwitch3() != THUM_ERROR_SUCCESS){
			fprintf(stderr,"Reading failed\n");
			return 1;
		}
		s3Value = GetSwitch3();
	}

	if(x == 1){
		if(verbose){
			printf("Taking external temperature reading\n");
		}
		if(ReadExternalTempOnly() != THUM_ERROR_SUCCESS){
			fprintf(stderr,"Reading failed\n");
			return 1;
		}
		xValue = GetExternalTempOnly();
	}
		
	if(i == 1){
		if(verbose){
			printf("Taking infrared reading\n");
		}
		if(ReadIRprox() != THUM_ERROR_SUCCESS){
			fprintf(stderr,"Reading failed\n");
			return 1;
		}
		iValue = GetIRprox();
	}
	
	if(u == 1){
		if(verbose){
			printf("Taking ultrasonic reading\n");
		}
		if(ReadUltrasonic() != THUM_ERROR_SUCCESS){
			fprintf(stderr,"Reading failed\n");
			return 1;
		}
		uValue = GetUltrasonic();
	}

	/* Print a header */
	time_t tt;
    struct tm *tod;
    time(&tt);
    tod = localtime(&tt);
	
	printf("%d/%d/%d ", tod->tm_mon+1, tod->tm_mday, tod->tm_year+1900);
    if(tod->tm_hour < 10){
		printf("0");
    }
    printf("%d:", tod->tm_hour);
    if(tod->tm_min < 10){
        printf("0");
    }
    printf("%d:", tod->tm_min);
    if(tod->tm_sec < 10){
        printf("0");
    }
    printf("%d ", tod->tm_sec);

	if(t == 1){
		printf("%f %c ", tValue, tempUnit);
	}
	if(h == 1){
		printf("%f ", hValue);
	}
	if(d == 1){
		printf("%f %c ", dValue, tempUnit);
	}
	if(s == 1){
		printf("%f ", sValue);
	}
	if(s2 == 1){
		printf("%f ", s2Value);
	}
	if(s3 == 1){
		printf("%f ", s3Value);
	}
	if(x == 1){
		printf("%f %c ", xValue, tempUnit);
	}
	if(i == 1){
		printf("%f ", iValue);
	}
	if(u == 1){
		printf("%f ", uValue);
	}
	printf("\n");


	/* done */
	return 0;
}

