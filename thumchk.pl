#!/usr/bin/perl -w
# thukchk.pl
# Use this script to monitor output of a THUM device and send email if results
# are out of range
#
# $Revision: 1.2 $
# $Name: release-1_0_7 $
# $Date: 2006/09/17 21:40:53 $
#
###############################################################################

use strict;
use Net::SMTP_auth;
use Sys::Syslog;
use Getopt::Std;
use vars qw($opt_l $opt_e $opt_a $opt_A $opt_p $opt_v $opt_f $opt_c $opt_t $opt_h $opt_d $opt_s $opt_2 $opt_3 $opt_x $opt_i $opt_u $opt_m);

getopts('leaAp:vfcthds23xium');

### GLOBALS ###

# input file name
my $config_file = "/etc/thumchk.conf";

# flag to determine if we need to send an email
my $send_email = 0;

# flag to determine if we should check if returned values
# exceed specified threshold vlaues
my $check_thresh = 0;

# send email if thresh vale is exceeded
my $email_on_alarm = 0;

# flag to determine if we should print output to syslog instead of stdout
my $print_to_syslog = 0;

# human-friendly name for this THUM device
my $monitor_name = "Default THUM";

# holds output text
my $email_msg = "";

# needed by send_email()
my $email_to = "";
my $email_from = "";
my $email_subj = "";
my $smtp_server = "";
my $smtp_user = "";
my $smtp_pass = "";
my $smtp_auth = "";
my $smtp_auth_type = "";

# threshold values
my $thresh_templo = "";
my $thresh_temphi = "";
my $thresh_humidlo = "";
my $thresh_humidhi = "";
my $thresh_dewptlo = "";
my $thresh_dewpthi = "";
my $thresh_switch = "";
my $thresh_switch2 = "";
my $thresh_switch3 = "";
my $thresh_exttemplo = "";
my $thresh_exttemphi = "";
my $thresh_irlo = "";
my $thresh_irhi = "";
my $thresh_ultrasoniclo = "";
my $thresh_ultrasonichi = "";

# polling interval (seconds)
my $poll_interval = 0;

# path to thumctl program (here we're assuming it's in the user's current working durectory):
my $thumctl = "thumctl";

# Values returned from thumctl
my $tValue = 0.0;
my $hValue = 0.0;
my $dValue = 0.0;
my $sValue = 0.0;
my $s2Value = 0.0;
my $s3Value = 0.0;
my $xValue = 0.0;
my $iValue = 0.0;
my $uValue = 0.0;


### BEGIN MAIN PROGRAM ###

# Check args
$poll_interval   = $opt_p if defined $opt_p;
$send_email      = 1      if defined $opt_e;
$check_thresh    = 1      if defined $opt_a or defined $opt_A;
$email_on_alarm  = 1      if defined $opt_A;
$print_to_syslog = 1      if defined $opt_l;

# Ensure we have something to pass on to thumctl
if (!defined $opt_t && !defined $opt_h && !defined $opt_d && !defined $opt_s &&
	!defined $opt_2 && !defined $opt_3 && !defined $opt_x && !defined $opt_i && 
    !defined $opt_u){
	print "Must specify at least one of the following flags:\n";
	print "-t, -h, -d, -s, -2, -3, -x, -i, -u\n";
	print "Usage:\n";
	print "thumchk.pl <options> where <options> is one or more of the following:\n";
	print "\t-v  use verbose output\n";
	print "\t-f  display temperatures in degrees F\n";
	print "\t-c  display temperatures in degrees C\n";
	print "\t-t  take a temperature reading\n";
	print "\t-h  take a relative humudity reading\n";
	print "\t-d  take a dew point reading\n";
	print "\t-s  take a switch reading\n";
    print "\t-2  take a switch reading from switch 2\n";
	print "\t-3  take a switch reading from switch 3\n";
	print "\t-x  take an external temperature reading\n";
	print "\t-i  take an IR proximity reading\n";
	print "\t-u  take an ultrasonic reading\n";
	print "\n";
	print "\t-p n specify the polling interval.  Take a reading every n seconds\n";
	print "\t-e email results using email settings from thumchk.conf\n";
	print "\t-a check results against alarm threshold values specified in thumchk.conf\n";
	print "\t-A check results against alarm threshold values specified in thumchk.conf and send email if threshold is met or exceeded\n";
    print "\t-l print output to syslog instead of standard output\n";
	print "\n";
	exit 1;
}

# Get settings from config file
&read_config_file();

# Build command line
my $command = "$thumctl ";
$command .= "-v " if (defined $opt_v);
$command .= "-f " if (defined $opt_f);
$command .= "-c " if (defined $opt_c);
$command .= "-t " if (defined $opt_t);
$command .= "-h " if (defined $opt_h);
$command .= "-d " if (defined $opt_d);
$command .= "-s " if (defined $opt_s);
$command .= "-2 " if (defined $opt_2);
$command .= "-3 " if (defined $opt_3);
$command .= "-x " if (defined $opt_x);
$command .= "-i " if (defined $opt_i);
$command .= "-u " if (defined $opt_u);

# Take a reading, and if we need to poll, continue to take readings
# at specified interval until user hits ^C
&take_reading();
while($poll_interval > 0){
    sleep($poll_interval);
	&take_reading();
}
exit 0;
### END MAIN PROGRAM


### SUBROUTINES ###

# read_config_file()
# Reads config file and sets variables appropriately
sub read_config_file(){
	open(CONFIG, $config_file) or die "Can't open config file $config_file: $!";
	while( my $line = <CONFIG>){
		next if $line =~ /^\s*#/; # skip comments
   	 	next if $line =~ /^\s*$/; # skip blank lines
	
		chomp($line);             # remove newline
	
		my $name;                 # LHS of config entry
		my $val;                  # RHS of config entry
		($name, $val) = split(/=/, $line);
	
		# remove leading and trailing whitespace, ensure $name is lower case:
		$name =~ s/^\s+//;
		$name =~ s/\s+$//;
		$name = lc $name;
		$val  =~ s/^\s+//;
		$val  =~ s/\s+$//;
		
		if($name eq "monitor_name"){
			$monitor_name = $val;
		} elsif ($name eq "email_to"){
			$email_to = $val;
		} elsif ($name eq "email_from"){
			$email_from = $val;
		} elsif ($name eq "email_subj"){
			$email_subj = $val;
		} elsif ($name eq "smtp_server"){
			$smtp_server = $val;
		} elsif ($name eq "smtp_user"){
			$smtp_user = $val;
		} elsif ($name eq "smtp_pass"){
			$smtp_pass = $val;
        } elsif ($name eq "smtp_auth"){
            $smtp_auth = $val;
        } elsif ($name eq "smtp_auth_type"){
            $smtp_auth_type = $val;
		} elsif ($name eq "thresh_templo"){
			$thresh_templo = $val;
		} elsif ($name eq "thresh_temphi"){
			$thresh_temphi = $val;
		} elsif ($name eq "thresh_humidlo"){
			$thresh_humidlo = $val;
		} elsif ($name eq "thresh_humidhi"){
			$thresh_humidhi = $val;
		} elsif ($name eq "thresh_dewptlo"){
			$thresh_dewptlo = $val;
		} elsif ($name eq "thresh_dewpthi"){
			$thresh_dewpthi = $val;
		} elsif ($name eq "thresh_switch"){
			$thresh_switch = $val;
		} elsif ($name eq "thresh_switch2"){
			$thresh_switch2 = $val;
		} elsif ($name eq "thresh_switch3"){
			$thresh_switch3 = $val;
		} elsif ($name eq "thresh_exttemplo"){
			$thresh_exttemplo = $val;
		} elsif ($name eq "thresh_exttemphi"){
			$thresh_exttemphi = $val;
		} elsif ($name eq "thresh_irlo"){
			$thresh_irlo = $val;
		} elsif ($name eq "thresh_irhi"){
			$thresh_irhi = $val;
		} elsif ($name eq "thresh_ultrasoniclo"){
			$thresh_ultrasoniclo = $val;
		} elsif ($name eq "thresh_ultrasonichi"){
			$thresh_ultrasonichi = $val;
		} else {
    		&output("Unrecognized config file entry:\n$line\nIgnoring\n");
		}
	}
	close(CONFIG);
}

# take_reading()
# executes $command 
# checks returned values against alarm threshold values
# send email if values exceed threshold values
sub take_reading(){
	my $cmdresult = `$command`;
	chomp $cmdresult;
	$cmdresult =~ s/^\s+//;
	$cmdresult =~ s/\s+$//;
    &output("$cmdresult\n");

	# check alarm levels if we need to:
	if($check_thresh){
		my @results = split(/ /, $cmdresult);
		@results = @results[2..$#results]; # discard date and time
		if(defined $opt_t){
			my $temp = $results[0];
			@results = @results[2..$#results] if $#results > 2; # discard temp and temp units
			if($temp <= $thresh_templo or $temp >= $thresh_temphi){
                $send_email = 1 if($email_on_alarm);
				&output("ALERT! -- Temperature was $temp\n" . 
                        "Low Temperature threshold was $thresh_templo\n" .
                        "Hi Temperature threshold was $thresh_temphi\n");
			}
		}
		if(defined $opt_h){
			my $humid = $results[0];
			@results = @results[1..$#results] if $#results > 1; # disacrd humidity...
			if($humid <= $thresh_humidlo or $humid >= $thresh_humidhi){
                $send_email = 1 if($email_on_alarm);
				&output("ALERT! -- Humidity was $humid\n" . 
                        "Low Humidity threshold was $thresh_humidlo\n" .
                        "Hi Humidity threshold was $thresh_humidhi\n");
			}
		}
		if(defined $opt_d){
			my $dewpt = $results[0];
			@results = @results[2..$#results] if $#results > 2;
			if($dewpt <= $thresh_dewptlo or $dewpt >= $thresh_dewpthi){
                $send_email = 1 if($email_on_alarm);
				&output("ALERT! -- Dew Point was $dewpt\n" .
                        "Low Dew Point threshold was $thresh_dewptlo\n" .
                        "Hi Dew Point threshold was $thresh_dewpthi\n"); 
			}
		}
		if(defined $opt_s){
			my $switch = $results[0];
			@results = @results[1..$#results] if $#results > 1;
			if($switch == $thresh_switch){
                $send_email = 1 if($email_on_alarm);
				&outout("ALERT! -- Switch value was $switch\nSwitch threshold was $thresh_switch\n");
			}
		}
		if(defined $opt_2){
			my $switch = $results[0];
			@results = @results[1..$#results] if $#results > 1;
			if($switch == $thresh_switch2){
                $send_email = 1 if($email_on_alarm);
				&output("ALERT! -- Switch2 value was $switch\nSwitch2 threshold was $thresh_switch\n");
			}
        }
		if(defined $opt_3){
			my $switch = $results[0];
			@results = @results[1..$#results] if $#results > 1;
			if($switch == $thresh_switch3){
                $send_email = 1 if($email_on_alarm);
				&output("ALERT! -- Switch3 value was $switch\nSwitch3 threshold was $thresh_switch\n");
			}
        }
		if(defined $opt_x){
			my $exttemp = $results[0];
			@results = @results[2..$#results] if $#results > 2;
			if($exttemp <= $thresh_exttemplo or $exttemp >= $thresh_exttemphi){
                $send_email = 1 if($email_on_alarm);
				&output("ALERT! -- External temperature was $exttemp\n" .
                          "Low External temperature threshold was $thresh_exttemplo\n" .
                          "Hi External temperature threshold was $thresh_exttemphi\n");
			}
		}
		if(defined $opt_i){
			my $ir = $results[0];
			@results = @results[1..$#results] if $#results > 1;
			if($ir <= $thresh_irlo or $ir >= $thresh_irhi){
                $send_email = 1 if($email_on_alarm);
				&output("ALERT! -- IR reading was $ir\n".
                          "Low IR threshold was $thresh_irlo\n" .
                          "Hi IR threshold was $thresh_irhi\n");
			}
		}
		if(defined $opt_u){
			my $ultrasonic = $results[0];
			@results = @results[1..$#results] if $#results > 1;
			if($ultrasonic <= $thresh_ultrasoniclo or $ultrasonic >= $thresh_ultrasonichi){
                $send_email = 1 if($email_on_alarm);
				&output("ALERT! -- Ultrasonic reading was $ultrasonic\n".
                          "Low Ultrasonic threshold was $thresh_ultrasoniclo\n" .
                          "Hi Ultrasonic threshold was $thresh_ultrasonichi\n");
			}
		}
	} # end if($check_thresh)

    &send_mail() if $send_email;
}

# send_mail()
# Does the busywork of sending an email
# All usued values ($email_to, $email_from, etc...) are declaed as globals above
sub send_mail(){
	if($send_email){
		&output("Sending email\n");
		my @message = ("From: $email_from",
    		           "To: $email_to",
					   "Subject: $email_subj",
                       "",
				   	   $email_msg);

		my $smtp = Net::SMTP_auth->new($smtp_server) 
    	 	       or die "Cannot connect to $smtp_server\n";
        if($smtp_auth ne 0){		   
    		$smtp->auth($smtp_auth_type, $smtp_user, $smtp_pass) or die "Auth error";
        }
		$smtp->mail($email_from) or die "From error";
		$smtp->to($email_to)     or die "To error";
		$smtp->data()            or die "Data error";
		foreach my $line (@message){
			$smtp->datasend("$line\n") or die "Data Send error";
		}
		$smtp->dataend()               or die "Data end error";
		$smtp->quit()                  or die "Quit error";
	}
}

# output()
# This is just a print() replacement that prints to all various destinations
sub output(){
    my $msg = shift;
    syslog('info', "[thumchk.pl] $msg") if $print_to_syslog;
    $email_msg .= $msg if $send_email; 
    print "$msg";
}

