/*
 * THUM.c
 * libTHUM.so source file
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
#include <linux/hiddev.h>
#include <math.h>
#include <dirent.h>
#include <string.h>
#include <ftw.h>

#include "THUM.h"

/* output codes should be set as follows in client code:
 *  F1  3, F2 0 will reset the device
 *  F1  0, F2 1 will configure for reading temperature
 *  F1  1, F2 1 will configure for reading RH
 *  F1  4, F2 1 will configure for reading IR prox
 *  F1  5, F2 1 will configure for reading LM35
 *  F1  6, F2 1 will configure for reading switch
 *  F1  7, F2 1 will configure for reading ultrasonic
 *  F1  8, F2 1 will configure for reading Dallas onboard
 *  F1  9, F2 1 will configure for reading Dallas external
 *  F1 10, F2 1 will configure for reading switch 2
 *  F1 11, F2 1 will configure for reading switch 3
 */

/* GLOBALS */
int fd = -1;               /* file descriptor */
int verbose = 0;           /* verbosity flag */
int DeviceDetected = THUM_FALSE;

char   tempUnit = 'C';     /* 'F' or 'C' */
double tempF;              /* Temperature in deg F */
double tempC;              /* Temperature in deg C */
double dewPtF;             /* Dew Point in deg F */
double dewPtC;             /* Dew Point in deg C */
double rh;                 /* Relative humidity */
double irProx;             /* IR proximity sensor value */
double ultrasonicProx;     /* Ultrasonic proximity sensor value */
double tempOnlyF;          /* Temperature from temp only sensor in deg F */
double tempOnlyC;          /* Temperature from temp only sensor in deg C */
double externalTempOnlyF;  /* Temperature from external temp only sensor in deg F */
double externalTempOnlyC;  /* Temperature from external temp only sensor in deg C */
double switchVal;          /* value from external switch */
double switch2Val;          /* value from external switch2 */
double switch3Val;          /* value from external switch3 */

unsigned short devID = 0x0100; /* ID of current device */

/* FUNCTIONS */

/* Set the hid device to communicate with THUM
 * PRE: strlen(dev) < THUM_MAXHIDDEVLEN
 */
/* hiddev is autodetected now  -- this should be removed soon*/
/*
long SetHiddev(char *dev){
	if(strlen(dev) > THUM_MAXHIDDEVLEN){
		fprintf(stderr, "Specified hiddev string too long\n");
		return THUM_ERROR_PARAMOUTOFRANGE;
	}
	strcpy(hiddev, dev);
	return THUM_ERROR_SUCCESS;
}
*/

/*
 * Tells the unit to take an internal temperature reading
 * NOTE: CalculateTemps MUST be called before CalculateRH as the RH calculation
 *       depends on the temperature.
 * NOTE: the pauses (::Sleep(1000)) are needed, but the 1000 is just a guess
 * RETURNS: THUM_ERROR_SUCCESS upon success, the error code from first error
 *          encountered otherwise upon failure
 */
long ReadTemp(void){
	unsigned char byte1, byte2;

    long retval = THUM_ERROR_SUCCESS;

	if (DeviceDetected==THUM_FALSE){
            DeviceDetected=FindHID();
    }

    if(DeviceDetected==THUM_TRUE){

        retval = WriteToDevice(0, 1);
        if(retval != THUM_ERROR_SUCCESS){
        	return retval;
        }
        sleep(1);
        retval = ReadFromDevice(&byte1, &byte2);
        if (retval != THUM_ERROR_SUCCESS){
        	return retval;
        }
        retval = CalculateTemps(byte1, byte2);
        if (retval != THUM_ERROR_SUCCESS){
        	return retval;
        }

    } else {
        retval = THUM_ERROR_THUMNOTFOUND;
    }
    return retval;
}

/*
 * Tells the unit to take an RH reading
 * NOTE: CalculateTemps MUST be called before CalculateRH as the RH calculation
 *       depends on the temperature.
 * NOTE: the pauses are needed, but the duration is just a guess
 * RETURNS: THUM_ERROR_SUCCESS upon success, the error code from first error
 *          encountered otherwise upon failure
 */
long ReadRH(void){
	unsigned char byte1, byte2;

    long retval = THUM_ERROR_SUCCESS;
    if (DeviceDetected==THUM_FALSE){
            DeviceDetected=FindHID();
    }

    if(DeviceDetected==THUM_TRUE){
		/* CalculateRH() uses tmepC */
		char oldTempUnit = tempUnit; /* save previous value */
		tempUnit = 'C';

		/* read temp in degrees C */
        retval = WriteToDevice(0, 1);
        if(retval != THUM_ERROR_SUCCESS){
			tempUnit = oldTempUnit;
        	return retval;
        }
        sleep(1);
        retval = ReadFromDevice(&byte1, &byte2);
        if (retval != THUM_ERROR_SUCCESS){
			tempUnit = oldTempUnit;
        	return retval;
        }
        retval = CalculateTemps(byte1, byte2);
        if (retval != THUM_ERROR_SUCCESS){
			tempUnit = oldTempUnit;
        	return retval;
        }

        /* read the RH */
        retval = WriteToDevice(1, 1);
        if(retval != THUM_ERROR_SUCCESS){
			tempUnit = oldTempUnit;
        	return retval;
        }
        sleep(1);
        retval = ReadFromDevice(&byte1, &byte2);
        if(retval != THUM_ERROR_SUCCESS){
			tempUnit = oldTempUnit;
        	return retval;
        }
        retval = CalculateRH(byte1, byte2);
        if(retval != THUM_ERROR_SUCCESS){
			tempUnit = oldTempUnit;
        	return retval;
        }
		
		tempUnit = oldTempUnit;

    } else {
        retval = THUM_ERROR_THUMNOTFOUND;
    }
    return retval;
}

/*
 * Tells the unit to take a dew point reading
 * NOTE: CalculateTemps MUST be called before CalculateRH as the RH calculation
 *       depends on the temperature.
 * NOTE: the pauses are needed, but the duration is just a guess
 * RETURNS: THUM_ERROR_SUCCESS upon success, the error code from first error
 *          encountered otherwise upon failure
 */
long ReadDewPt(void){
	unsigned char byte1, byte2;

    long retval = THUM_ERROR_SUCCESS;
    if (DeviceDetected==THUM_FALSE){
            DeviceDetected=FindHID();
    }

    if(DeviceDetected==THUM_TRUE){
		/* CalculateRH() uses tmepC */
		char oldTempUnit = tempUnit; /* save previous value */
		tempUnit = 'C';

		/* read temp in degrees C */
        retval = WriteToDevice(0, 1);
        if(retval != THUM_ERROR_SUCCESS){
			tempUnit = oldTempUnit;
        	return retval;
        }
        sleep(1);
        retval = ReadFromDevice(&byte1, &byte2);
        if (retval != THUM_ERROR_SUCCESS){
			tempUnit = oldTempUnit;
        	return retval;
        }
        retval = CalculateTemps(byte1, byte2);
        if (retval != THUM_ERROR_SUCCESS){
			tempUnit = oldTempUnit;
        	return retval;
        }

        /* read the RH */
        retval = WriteToDevice(1, 1);
        if(retval != THUM_ERROR_SUCCESS){
			tempUnit = oldTempUnit;
        	return retval;
        }
        sleep(1);
        retval = ReadFromDevice(&byte1, &byte2);
        if(retval != THUM_ERROR_SUCCESS){
			tempUnit = oldTempUnit;
        	return retval;
        }
        retval = CalculateRH(byte1, byte2);
        if(retval != THUM_ERROR_SUCCESS){
			tempUnit = oldTempUnit;
        	return retval;
        }

        /* calculate the dew point */
        CalculateDewPt();

		tempUnit = oldTempUnit;
    } else {
        retval = THUM_ERROR_THUMNOTFOUND;
    }
    return retval;
}

/*
 * Resets the device
 * RETURNS: THUM_ERROR_SUCCESS upon success, the error reported by
 *         WriteToDevice() upon failure
 */
long Reset(void){
       long retval = THUM_ERROR_SUCCESS;
       retval = WriteToDevice(3,0);
       return retval;
}

/*
 * Gets the temperature unit (F or C)
 * RETURNS: the value of tempUnit: 'C' or 'F'
 */
char GetTempUnit(void){
	return tempUnit;
}

/*
 * Sets the temperature unit (to F or C)
 * PRE: unit must be F or C for this function to succeed
 * RETURNS: THUM_ERROR_SUCCESS upon success, THUM_ERROR_BADTEMPUNIT upon
 *         failure
 */
long SetTempUnit(char unit){
       long retval = THUM_ERROR_SUCCESS;
       if(unit == 'F' || unit == 'f'){
               tempUnit = 'F';
       } else if(unit == 'C' || unit == 'c'){
               tempUnit = 'C';
       } else {
               retval = THUM_ERROR_BADTEMPUNIT;
       }
       return retval;
}

/*
 * Gets the termperature from the unit
 * RETURNS: the value of tempF or tempC depending on tempUnit
 */
double GetTemp(void){
       if(tempUnit == 'F'){
               return tempF;
       } else {
               return tempC;
       }
}

/*
 * Gets the relative humidity from the unit
 * RETURNS: the value of rh
 */
double GetRH(void){
       return rh;
}

/*
 * Gets the dew point from the unit
 * RETURNS: the value of dewPt
 */
double GetDewPt(void){
       if(tempUnit == 'F'){
               return dewPtF;
       } else {
               return dewPtC;
       }
}

/* sets the ID of the device to communicate with
 * sDevID defaults to 0x0100.  Use this function to change it.
 */
void SetDevID(unsigned short id){
       devID = id;
}

// RETURNS: the ID of the device currently being communicated with
unsigned short GetDevID(void){
       return devID;
}


/*
 * callback for ftw() function used in FindHID()
 * This function is called for each directory entry found by ftw()
 * see 'man ftw' for more details
 */
int find_hiddev(const char *name, const struct stat *status, int type){
	char sep = '/';
	char baseName[THUM_MAXHIDDEVLEN] = "hiddev";
	
	if(type == FTW_NS) return 0; // not processable
	if(type == FTW_F){ // file
		if(rindex(name, sep) != NULL){
			char *filename = rindex(name, sep) + 1; // is this safe?
			if(strncmp(filename, baseName, strlen(baseName)) == 0){
				if(verbose){
					printf("Checking for THUM device at %s\n", name);
				}
				if(isTHUM(name) == THUM_TRUE){
					if(verbose){
						printf("Found THUM device at %s\n", name);
					}
					return THUM_TRUE;
				}
			}
		}
	}
	return 0;
}

/* find and initializes the THUM HID Device in /dev
 * RETURNS THUM_TRUE upon success, THUM_FALSE upon failure
 */
int FindHID(void){
	char devDir[THUM_MAXHIDDEVLEN] = "/dev";
	if(ftw(devDir, find_hiddev, 1) == THUM_TRUE){
		return THUM_TRUE;
	} else {
		return THUM_FALSE;
	}
}


/* Attempts to initializes the THUM HID Device located at devName
 * RETURNS THUM_TRUE upon success, THUM_FALSE upon failure
 */
int isTHUM(const char *devName){
	int retval;
	struct hiddev_devinfo device_info;
	
    /* find the THUM */
	if ((fd = open(devName, O_RDONLY)) < 0) {
    	perror("Could not open HID device");
     	return THUM_FALSE;
    }
    retval = ioctl(fd, HIDIOCGDEVINFO, &device_info);
    if(retval == -1){
		perror("Could not get HID device info\n");
        return THUM_FALSE;
    }
   
    if(device_info.vendor != THUM_VENDORID || 
	   device_info.product != THUM_PRODID  ||
	   device_info.version != devID){
        fprintf(stderr, "Could not find the THUM on %s\n", devName);
        return THUM_FALSE;
    } else {
        char nameBuff[1024];
        retval = ioctl(fd, HIDIOCGNAME(sizeof(nameBuff)), nameBuff);
        if(retval == -1){
			perror("Could not get Device name\n");
            return THUM_FALSE;
        }
		if(verbose){
	        printf("Found %s on %s\n", nameBuff, devName);
		}
    }
  
    retval = ioctl(fd, HIDIOCINITREPORT,0);
    if(retval == -1){
		perror("Could not initialize device report\n");
        return THUM_FALSE;
    }
	return THUM_TRUE;
}

/*
 * Writes two bytes to the device.  The two bytes are specified in byte1 and byte2.
 * RETURNS: THUM_ERROR_SUCCESS upon success, the first error code encountered
 *          upon failure
 */
long WriteToDevice(unsigned char byte1, unsigned char byte2){
	long retval = 0;
	struct hiddev_usage_ref uref;
	struct hiddev_report_info rinfo;

/* send info for usage 0*/
	memset(&uref, 0, sizeof(uref));
	uref.report_type = HID_REPORT_TYPE_OUTPUT; /* INPUT, OUTPUT, or FEATURE */
	uref.report_id = 0;
	uref.field_index=0;
	uref.usage_index=0;
	uref.usage_code = THUM_USAGE_OUTPUT_F1;
	uref.value=byte1;
	retval = ioctl(fd, HIDIOCSUSAGE, &uref);
	if(retval == -1){
		return retval;
	}

/* send info for usage 1*/
	memset(&uref, 0, sizeof(uref));
	uref.report_type = HID_REPORT_TYPE_OUTPUT; /* INPUT, OUTPUT, or FEATURE */
	uref.report_id = 0;
	uref.field_index=0;
	uref.usage_index=1;
	uref.usage_code = THUM_USAGE_OUTPUT_F2;
	uref.value=byte2;
	retval = ioctl(fd, HIDIOCSUSAGE, &uref);
	if(retval == -1){
		return retval;
	}

/* now send the report */
	memset(&rinfo, 0, sizeof(rinfo));
	rinfo.report_type=HID_REPORT_TYPE_OUTPUT;
	rinfo.report_id=0;
	rinfo.num_fields=1;
	retval = ioctl(fd, HIDIOCSREPORT, &rinfo);
	if(retval == -1){
		return retval;
	}

	return THUM_ERROR_SUCCESS;
}

/*
 * Reads two bytes from the device into byte1 and byte2
 * RETURNS: THUM_ERROR_SUCCESS upon success, the first error code encountered
 *          upon failure
 */
long ReadFromDevice(unsigned char* byte1, unsigned char* byte2){
	long retval = 0;
	struct hiddev_usage_ref uref;

/* Get Byte0 */
	memset(&uref, 0, sizeof(uref));
	uref.report_type = HID_REPORT_TYPE_INPUT;
	uref.report_id = 0;
	uref.field_index = 0;
	uref.usage_index = 0;
	retval = ioctl(fd, HIDIOCGUSAGE, &uref);
	if(retval == -1){
		return retval;
	}
	*byte1 = uref.value;

/* Get Byte1 */
	memset(&uref, 0, sizeof(uref));
	uref.report_type = HID_REPORT_TYPE_INPUT;
	uref.report_id = 0;
	uref.field_index = 0;
	uref.usage_index = 1;
	retval = ioctl(fd, HIDIOCGUSAGE, &uref);
	if(retval == -1){
		return retval;
	}
	*byte2 = uref.value;

	return THUM_ERROR_SUCCESS;
}

/* Calculate temperatures in F and C as well as dewpoint based on bytes passed in.
 * PRE: byte1 and byte2 should hold valid values read from the device
 * RETURNS: THUM_ERROR_SUCCESS upon success, THUM_ERROR_RESULTOUTOFRANGE upon
 *          failure
 */
long CalculateTemps(unsigned char byte1, unsigned char byte2){
	long retval = THUM_ERROR_SUCCESS;
	tempC = ((((double)byte1 * 256.0) + (double)byte2) * 0.01) - 40.0 - 1.1111;

    if(tempC < -40.0 || tempC > 124.0){
    	return THUM_ERROR_RESULTOUTOFRANGE;
    }

    tempF = ((((double)byte1 * 256.0) + (double)byte2) * 0.018) - 40.0 - 2;

	return retval;
}

/*
 * Calculate relative humidity based on bytes passed in.
 * PRE: The value of global variable tempC should be valid.  CalculateRH()
 *      should always be called *after* CalculateTemps().
 * RETURNS: THUM_ERROR_SUCCESS upon success, THUM_ERROR_RESULTOUTOFRANGE upon
 *          failure
 */
long CalculateRH(unsigned char byte1, unsigned char byte2){
	long retval = THUM_ERROR_SUCCESS;

    double tempVal = byte1 * 256.0 + (double)byte2; /* temporary value for readability */
    rh = -4.0 + (0.0405 * tempVal) + (-0.0000028 * tempVal*tempVal);
    rh = (tempC - 25.0) * (0.01 + (0.00008 * tempVal)) + rh;

    if(rh < 0.0 || rh > 100.0){
    	return THUM_ERROR_RESULTOUTOFRANGE;
    }

	return retval;
}

/*
 * Calculate the dewpoint
 * PRE: CalculateDewPt() *must* be called after CalculateRH(), which *must*
 *      be called after CalculateTemps();
 * RETURNS: THUM_ERROR_SUCCESS upon success, THUM_ERROR_RESULTOUTOFRANGE upon
 *          failure
 */
void CalculateDewPt(void){
	/* first calculate Es */
    double Es = 6.11 * pow(10.0, (7.5 * tempC) / (237.7 + tempC));
    /* next calculate E */
    double E = rh * Es / 100.0;
    /* calculate dew point
     * log is ln according to math.h
	 */
    dewPtC = (-430.22+237.7 * log(E)) / (-log(E) + 19.08);
    /* and in degrees F: */
    dewPtF = (9.0/5.0) * dewPtC + 32.0;
}


/* Tells the unit to take a IR proximity reading
 * NOTE: the pauses (sleep(1)) are needed, but the 1000 is just a guess
 * RETURNS: THUM_ERROR_SUCCESS upon success, the error code from first error
 *          encountered otherwise upon failure
 */
long ReadIRprox(void){
        long retval = THUM_ERROR_SUCCESS;
		unsigned char byte1;
		unsigned char byte2;

        if (DeviceDetected==THUM_FALSE){
                DeviceDetected=FindHID();
		}

        if(DeviceDetected==THUM_TRUE){
                /* read ir prox */
/*                sleep(.1); */
                sleep(1);
                retval = WriteToDevice(4, 1);
                if(retval != THUM_ERROR_SUCCESS){
                        return retval;
                }
/*                sleep(.2); */
                sleep(1);
                retval = ReadFromDevice(&byte1, &byte2);
                if (retval != THUM_ERROR_SUCCESS){
                        return retval;
                }
                retval = CalculateIRprox(byte1, byte2);
                if (retval != THUM_ERROR_SUCCESS){
                        return retval;
                }

        } else {
                retval = THUM_ERROR_THUMNOTFOUND;
        }
        return retval;
}


/* Gets the IR proximity value from the unit
 * RETURNS: the value of irProx
 */
double GetIRprox(void){
        return irProx;
}

/* Calculate IR proximity
 * PRE: cByte1 should hold the second byte received from the device (Buffers[1])
 *      and cByte2 should hold the third byte (Buffers[2]).
 * RETURNS: THUM_ERROR_SUCCESS upon success, THUM_ERROR_RESULTOUTOFRANGE upon
 *          failure
 */
long CalculateIRprox(unsigned char byte1, unsigned char byte2){
        long retval = THUM_ERROR_SUCCESS;
        irProx = ((double)byte2 * (5000 / 256));
        return retval;
}

/* Tells the unit to take a Temp only reading from LM35
 * NOTE: the pauses (sleep(1)) are needed, but the 1 is just a guess
 * RETURNS: THUM_ERROR_SUCCESS upon success, the error code from first error
 *          encountered otherwise upon failure
 */
long ReadTempOnly(void){
        long retval = THUM_ERROR_SUCCESS;
		unsigned char byte1;
		unsigned char byte2;

        if (DeviceDetected==THUM_FALSE){
                DeviceDetected=FindHID();
		}

        if(DeviceDetected==THUM_TRUE){
                /* read temp only sensor - Dallas DS18s20 */
/*               sleep(.100); */
                sleep(1);
                retval = WriteToDevice(8, 1);   /* DS18s20 */
                if(retval != THUM_ERROR_SUCCESS){
                        return retval;
                }
/*              sleep(.750); */
                sleep(1);
                retval = ReadFromDevice(&byte1, &byte2);
                if (retval != THUM_ERROR_SUCCESS){
                        return retval;
                }

                retval = CalculateTempOnly(byte1, byte2);
                if (retval != THUM_ERROR_SUCCESS){
                        return retval;
                }

                if((byte1 == 0x08) && (byte2 == 0x01)){
                        /* dallas sensor not present */
                        /* read temp only sensor - LM35 */
                        sleep(.100);
                        retval = WriteToDevice(5, 1);  /* LM35 */
                        if(retval != THUM_ERROR_SUCCESS){
                                return retval;
                        }
                        sleep(.200);
                        retval = ReadFromDevice(&byte1, &byte2);
                        if (retval != THUM_ERROR_SUCCESS){
                                return retval;
                        }
                        if ((byte1 == 0x00) && (byte2 != 0xFF)){
                                /* LM35 present */
                                retval = CalculateTempOnlyLM(byte1, byte2);
                                if (retval != THUM_ERROR_SUCCESS){
                                        return retval;
                                }
                        }

                }

        } else {
                retval = THUM_ERROR_THUMNOTFOUND;
        }
        return retval;
}

/* Gets the TempOnly value from LM35 of the unit
 * RETURNS: the value of tempOnlyC or tempOnlyF
 */
double GetTempOnly(void){
        if(tempUnit == 'F'){
                return tempOnlyF;
        } else {
                return tempOnlyC;
        }
}

/* Calculate TempOnly
 * PRE: cByte1 should hold the second byte received from the device (Buffers[1])
 *      and cByte2 should hold the third byte (Buffers[2]).
 * RETURNS: THUM_ERROR_SUCCESS upon success, THUM_ERROR_RESULTOUTOFRANGE upon
 *          failure
 */
long CalculateTempOnly(unsigned char byte1, unsigned char byte2){
        long retval = THUM_ERROR_SUCCESS;
        tempOnlyC = ((((double)byte1 * 256.0) + (double)byte2) /100);  /* DS18s20 */
        tempOnlyF = (tempOnlyC * 1.8) + 32;
        return retval;
}

/* Calculate TempOnlyLM
 * PRE: cByte1 should hold the second byte received from the device (Buffers[1])
 *      and cByte2 should hold the third byte (Buffers[2]).
 * RETURNS: THUM_ERROR_SUCCESS upon success, THUM_ERROR_RESULTOUTOFRANGE upon
 * failure
 */
long CalculateTempOnlyLM(unsigned char byte1, unsigned char byte2){
        /* calculate temp for LM35 temp sensor */
        long retval = THUM_ERROR_SUCCESS;
        tempOnlyC = (((double)byte2 * 5 / 256)) * 100;  /* LM35DZ */
        tempOnlyF = (tempOnlyC * 1.8) + 32;

        return retval;
}

/* Tells the unit to take a Switch reading
 * NOTE: the pauses (::Sleep(1000)) are needed, but the 1000 is just a guess
 * RETURNS: THUM_ERROR_SUCCESS upon success, the error code from first error
 *          encountered otherwise upon failure
 */
extern long ReadSwitch(void){
        long retval = THUM_ERROR_SUCCESS;
		unsigned char byte1;
		unsigned char byte2;

        if (DeviceDetected==THUM_FALSE){
                DeviceDetected=FindHID();
		}

        if(DeviceDetected==THUM_TRUE){
                /* read switch */
                sleep(1);
                retval = WriteToDevice(6, 1);
                if(retval != THUM_ERROR_SUCCESS){
                        return retval;
                }
                sleep(1);
                retval = ReadFromDevice(&byte1, &byte2);
                if (retval != THUM_ERROR_SUCCESS){
                        return retval;
                }

                retval = CalculateSwitch(byte1, byte2);
                if (retval != THUM_ERROR_SUCCESS){
                        return retval;
                }

        } else {
                retval = THUM_ERROR_THUMNOTFOUND;
        }
        return retval;
}


/* Tells the unit to take a Switch2 reading
 * NOTE: the pauses (::Sleep(1000)) are needed, but the 1000 is just a guess
 * RETURNS: THUM_ERROR_SUCCESS upon success, the error code from first error
 *          encountered otherwise upon failure
 */
extern long ReadSwitch2(void){
        long retval = THUM_ERROR_SUCCESS;
		unsigned char byte1;
		unsigned char byte2;

        if (DeviceDetected==THUM_FALSE){
                DeviceDetected=FindHID();
		}

        if(DeviceDetected==THUM_TRUE){
                /* read switch2*/
                sleep(1);
                retval = WriteToDevice(10, 1);
                if(retval != THUM_ERROR_SUCCESS){
                        return retval;
                }
                sleep(1);
                retval = ReadFromDevice(&byte1, &byte2);
                if (retval != THUM_ERROR_SUCCESS){
                        return retval;
                }

                retval = CalculateSwitch(byte1, byte2);
                if (retval != THUM_ERROR_SUCCESS){
                        return retval;
                }

        } else {
                retval = THUM_ERROR_THUMNOTFOUND;
        }
        return retval;
}


/* Tells the unit to take a Switch3 reading
 * NOTE: the pauses (::Sleep(1000)) are needed, but the 1000 is just a guess
 * RETURNS: THUM_ERROR_SUCCESS upon success, the error code from first error
 *          encountered otherwise upon failure
 */
extern long ReadSwitch3(void){
        long retval = THUM_ERROR_SUCCESS;
		unsigned char byte1;
		unsigned char byte2;

        if (DeviceDetected==THUM_FALSE){
                DeviceDetected=FindHID();
		}

        if(DeviceDetected==THUM_TRUE){
                /* read switch3*/
                sleep(1);
                retval = WriteToDevice(11, 1);
                if(retval != THUM_ERROR_SUCCESS){
                        return retval;
                }
                sleep(1);
                retval = ReadFromDevice(&byte1, &byte2);
                if (retval != THUM_ERROR_SUCCESS){
                        return retval;
                }

                retval = CalculateSwitch(byte1, byte2);
                if (retval != THUM_ERROR_SUCCESS){
                        return retval;
                }

        } else {
                retval = THUM_ERROR_THUMNOTFOUND;
        }
        return retval;
}

/* Gets the Switch value from the unit
 * RETURNS: the value of switchVal
 */
extern double GetSwitch(void){
        return switchVal;
}

/* Gets the Switch2 value from the unit
 * RETURNS: the value of switchVal
 */
extern double GetSwitch2(void){
        return switch2Val;
}

/* Gets the Switch3 value from the unit
 * RETURNS: the value of switchVal
 */
extern double GetSwitch3(void){
        return switch3Val;
}

/* Calculate Switch value
 * PRE: byte1 should hold the second byte received from the device 
 *      and byte2 should hold the third byte.
 * RETURNS: THUM_ERROR_SUCCESS upon success, THUM_ERROR_RESULTOUTOFRANGE upon
 *          failure
 */
long CalculateSwitch(unsigned char byte1, unsigned char byte2){
        long retval = THUM_ERROR_SUCCESS;
        switchVal = ((double)byte2);
        return retval;
}

/* Tells the unit to take a Ultrasonic proximity reading
 * NOTE: the pauses (sleep(1)) are needed, but the 1 is just a guess
 * RETURNS: THUM_ERROR_SUCCESS upon success, the error code from first error
 *          encountered otherwise upon failure
 */
extern long ReadUltrasonic(void){
        long retval = THUM_ERROR_SUCCESS;
		unsigned char byte1;
		unsigned char byte2;
		
        if (DeviceDetected==THUM_FALSE){
                DeviceDetected=FindHID();
		}

        if(DeviceDetected==THUM_TRUE){
                /* read us prox */
                sleep(1);
                retval = WriteToDevice(7, 1);
                if(retval != THUM_ERROR_SUCCESS){
                        return retval;
                }
                sleep(1);
                retval = ReadFromDevice(&byte1, &byte2);
                if (retval != THUM_ERROR_SUCCESS){
                        return retval;
                }
                retval = CalculateUltrasonic(byte1, byte2);
                if (retval != THUM_ERROR_SUCCESS){
                        return retval;
                }

        } else {
                retval = THUM_ERROR_THUMNOTFOUND;
        }
        return retval;
}

/* Gets the Ultrasonic proximity value from the unit
 * RETURNS: the value of ultrasonicProx
 */
extern double GetUltrasonic(void){
        return ultrasonicProx;
}

/* Calculate Ultrasonic proximity
 * PRE: cByte1 should hold the second byte received from the device (Buffers[1])
 *      and cByte2 should hold the third byte (Buffers[2]).
 * RETURNS: THUM_ERROR_SUCCESS upon success, THUM_ERROR_RESULTOUTOFRANGE upon
 *          failure
 */
long CalculateUltrasonic(unsigned char byte1, unsigned char byte2){
        long retval = THUM_ERROR_SUCCESS;
        ultrasonicProx = (((double)byte1 * 256.0) + (double)byte2);
        return retval;
}


/* Tells the unit to take a Temp only reading from LM35
 * NOTE: the pauses (sleep(1)) are needed, but the 1 is just a guess
 * RETURNS: THUM_ERROR_SUCCESS upon success, the error code from first error
 *          encountered otherwise upon failure
 */
extern long ReadExternalTempOnly(void){
        long retval = THUM_ERROR_SUCCESS;
		unsigned char byte1;
		unsigned char byte2;
		
        if (DeviceDetected==THUM_FALSE){
                DeviceDetected=FindHID();
		}

        if(DeviceDetected==THUM_TRUE){
                /* read temp only sensor - Dallas DS18s20 */
                sleep(.100);
                retval = WriteToDevice(9, 1);  /* DS18s20 */
                if(retval != THUM_ERROR_SUCCESS){
                        return retval;
                }
                sleep(.750);
                retval = ReadFromDevice(&byte1, &byte2);
                if (retval != THUM_ERROR_SUCCESS){
                        return retval;
                }

                retval = CalculateExternalTempOnly(byte1, byte2);
                if (retval != THUM_ERROR_SUCCESS){
                        return retval;
                }
        } else {
                retval = THUM_ERROR_THUMNOTFOUND;
        }
        return retval;
}


/* Gets the ExternalTempOnly value from DS18s20 of the unit
 * RETURNS: the value of tempOnlyC or tempOnlyF
 */
extern double GetExternalTempOnly(void){
        if(tempUnit == 'F'){
                return externalTempOnlyF;
        } else {
                return externalTempOnlyC;
        }
}

/* Calculate ExternalTempOnly
 * PRE: cByte1 should hold the second byte received from the device (Buffers[1])
 *      and cByte2 should hold the third byte (Buffers[2]).
 * RETURNS: THUM_ERROR_SUCCESS upon success, THUM_ERROR_RESULTOUTOFRANGE upon
 *          failure
 */
long CalculateExternalTempOnly(unsigned char byte1, unsigned char byte2){
        long retval = THUM_ERROR_SUCCESS;
        externalTempOnlyC = ((((double)byte1 * 256.0) + (double)byte2) /100);  /* DS18s20 */
        externalTempOnlyF = (externalTempOnlyC * 1.8) + 32;
        return retval;
}

/* Return library version */
void  GetVersion(char* v){
        char version[THUM_MAXSTRLEN] = THUM_LIB_VERSION;
        const int emptyVerLen = 9; /* Name keyword expands to 9 chars when no tag assigned */
        if(strlen(version) > emptyVerLen){
                int size = strlen(version) - emptyVerLen;
                char* i = version;
                strncpy(version, version + (emptyVerLen - 2), size);
                version[size] = '\0';
                while(*i) *i++ = *i == '_' ? '.' : *i;
                strcpy(v, version);
        }else{
                strcpy(v, "unknown");
        }


}

/* Toggles verbosity */
void SetVerbose(int v){
	if(v == 0){
		verbose = 0;
	} else {
		
		verbose = 1;
	}
}
