THUM with Raspberry Pi
=======================
This application is provided by Practical Design Group for use with their THUM sensor. I've modified the Makefile to work on my recent version of Raspbian, as the provided Makefile did not work for me.

Retrieved from: http://practicaldesign.com/index.php/THUM/thumraspberrypi.html
See also: http://practicaldesign.com/images/stories/pdf/Linuxbrochure.pdf

To build:
---
`make all`

To install:
---
`sudo make install`

To uninstall:
---
`sudo make uninstall`

Usage
---
```
thumctl <options> where <options> is one or more of the following:
-H print this message
-V print THUM library version
-v use verbose output
-f display temperatures in degrees F
-c display temperatures in degrees C
-t take a temperature reading
-h take a relative humidity reading
-d take a dew point reading
-s take a switch reading
-2 take a switch reading from switch 2 -3 take a switch reading from switch 3 -x take an external temperature reading -i take an IR proximity reading
-u take an ultrasonic reading
```

Original instructions
-----------------
Notes:  You must have a USB Hub with sufficient power available to power on the device for a reading.  "Reading failed" is a good general indicator of not sufficient power to the device.

* You should copy thumctl and thumchk.pl to /usr/bin or /usr/local/bin
* When using the thumctl command, you must use at least one switch.

example:  `thumctl -t`
If you run `thumctl` without a switch, you will need to unplug/replug in the THUM to get it to start responding again.
Required dependencies:  libhid, hibusb-dev, libusb-0.1-4, libnet-smtpauth-perl

Sample Output:
```bash
root@raspberrypi:~# uname -a
Linux raspberrypi 3.2.27+ #250 PREEMPT Thu Oct 18 19:03:02 BST 2012 armv6l
GNU/Linux
```

```bash
root@raspberrypi:~/thum-src# thumctl
1/26/2013 15:17:15 23.258900 C 27.625240 3.556923 C 1.000000 0.010000 C
19.000000 1.000000
```

```bash
root@raspberrypi:~# thumchk.pl -v -f -t -h
Enabling verbose output
Setting temperature unit to F
Taking internal temperature reading
Checking for THUM device at /dev/usb/hiddev0
Found Practical Design Gr USB Temp/Humidity Sensor on /dev/usb/hiddev0
Found THUM device at /dev/usb/hiddev0
Taking relative humidity reading
1/26/2013 15:23:06 74.712000 F 25.570978
```

Disclaimer
---------------
I (@djflix) take take no credit for the original code or instructions. Original code (first commit) provided by Practical Design Group.
