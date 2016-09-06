THUM with Raspberry Pi
=======================
Retrieved from: http://practicaldesign.com/index.php/THUM/thumraspberrypi.html
See also: http://practicaldesign.com/images/stories/pdf/Linuxbrochure.pdf

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
root@raspberrypi:~# tar -zxvf thum.pi.src.tar.gz
thum-src/
thum-src/THUM.c
thum-src/thumchk.conf
thum-src/libTHUM.so.1
thum-src/thumctl.c
thum-src/thumctl
thum-src/Makefile
thum-src/thumchk.pl
thum-src/THUM.h
thum-src/Makefile_thumctl
thum-src/THUM.o
```

```bash
root@raspberrypi:~# cd thum-src
```

```bash
root@raspberrypi:~/thum-src# make all
gcc -fPIC -g -c -Wall THUM.c
THUM.c: In function GetVersion:
THUM.c:1000:29: warning: operation on i may be undefined
[-Wsequence-point]
gcc -shared -Wl,-soname,libTHUM.so.1 -o libTHUM.so.1 THUM.o -lc -lm
```

```bash
root@raspberrypi:~/thum-src# make install
cp -f libTHUM.so.1 /usr/lib
ln -sf /usr/lib/libTHUM.so.1 /usr/lib/libTHUM.so
/sbin/ldconfig
root@raspberrypi:~/thum-src# thumc
thumchk.pl  thumctl
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

I take (@djflix) take no credit for the original code or instructions. Only commits and changes from now on can be accredited to me.
