/* 
 * THUM.h
 * Header file for libTHUM.so
 * $Revision: 1.4 $
 * $Name:  $
 * $Date: 2006/09/17 23:12:48 $
 */

#ifndef THUM_H
#define THUM_H
/* THUM.h */

/* ERROR CODES */
#define THUM_ERROR_SUCCESS          0L      /* no error */
#define THUM_ERROR_BADTEMPUNIT      1L      /* bad temperature unit (not F, f, C, or c) */
#define THUM_ERROR_THUMNOTFOUND     2L      /* THUM device not found */
#define THUM_ERROR_READTIMEOUT      3L      /* Read operation timed out */
#define THUM_ERROR_WRITEFAILED      4L      /* Write operation failed */
#define THUM_ERROR_READFAILED       5L      /* Read operation failed */
#define THUM_ERROR_RESULTOUTOFRANGE 6L      /* Temp or RH was out of range */
#define THUM_ERROR_PARAMOUTOFRANGE  7L      /* Parameter was out of range */

/* CONSTANTS */
#define THUM_MAXSTRLEN    1024
/* vendor and product IDs for the THUM device */
#define THUM_LIB_VERSION  "$Name:  $"
#define THUM_VENDORID     0x0C70
#define THUM_PRODID       0x0750
#define THUM_TRUE         1
#define THUM_FALSE        0
#define THUM_MAXHIDDEVLEN 1024 /* max length of hiddev string (eg /dev/usb/hiddev0) */

/* usage codes: */
#define THUM_USAGE_INPUT_F1  0xffa10003
#define THUM_USAGE_INPUT_F2  0xffa10004
#define THUM_USAGE_OUTPUT_F1 0xffa10005
#define THUM_USAGE_OUTPUT_F2 0xffa10006

/* exported functions: */
long ReadTemp(void);                 /* Tells the unit to take an internal temp reading reading */
long ReadRH(void);                   /* Tells the unit to take a RH reading */
long ReadDewPt(void);                /* Tells the unit to take a dew point reading */
char GetTempUnit(void);              /* Gets the temperatire unit (F or C) */
long SetTempUnit(char unit);         /* Sets the temperature unit (to F or C) */
double GetTemp(void);                /* Gets the termperature from the unit */
double GetRH(void);                  /* Gets the relative humidity from the unit */
double GetDewPt(void);               /* Gets the dew point from the unit */
long Reset(void);                    /* Reset the device */
void SetDevID(unsigned short id);    /* sets the ID of the device to communicate with */
unsigned short GetDevID(void);       /* returns the ID of the device currently being communicated with */
long ReadIRprox(void);               /* Tells the unit to take an IR proximity reading */
double GetIRprox(void);              /* Gets the IR proximity reading */
long ReadTempOnly(void);             /* Take a temp only reading from LM35 */
double GetTempOnly(void);            /* Gets the temp as obtained from ReadTempOnly() */
long ReadSwitch(void);               /* Takes a switch reading */
double GetSwitch(void);              /* Gets the switch value */
long ReadSwitch2(void);               /* Takes a switch2 reading */
double GetSwitch2(void);              /* Gets the switch2 value */
long ReadSwitch3(void);               /* Takes a switch3 reading */
double GetSwitch3(void);              /* Gets the switch3 value */
long ReadUltrasonic(void);           /* Tells the unit to take an ultrasonic proximity reading */
double GetUltrasonic(void);          /* Get the ultrasonic proximity reading */
long ReadExternalTempOnly(void);     /* Get temp from external Dallas sensor only */
double GetExternalTempOnly(void);    /* Get external temp only */
void  GetVersion(char* v);             /* Get library version number, store in v */
void SetVerbose(int v);              /* Toggle verbosity */

/* internal helpers: */
int find_hiddev(const char *name, const struct stat *status, int type);
int FindHID(void);
int isTHUM(const char *devName);
long ReadFromDevice(unsigned char* byte1, unsigned char* byte2);
long WriteToDevice(unsigned char byte1, unsigned char byte2);
long CalculateTemps(unsigned char byte1, unsigned char byte2);
long CalculateRH(unsigned char byte1, unsigned char byte2);
void CalculateDewPt(void);
long CalculateIRprox(unsigned char byte1, unsigned char byte2);
long CalculateTempOnly(unsigned char byte1, unsigned char byte2);
long CalculateTempOnlyLM(unsigned char byte1, unsigned char byte2);
long CalculateSwitch(unsigned char byte1, unsigned char byte2);
long CalculateUltrasonic(unsigned char byte1, unsigned char byte2);
long CalculateExternalTempOnly(unsigned char byte1, unsigned char byte2);

#endif
