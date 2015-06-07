# qrigol - A control program for the Rigol DS1xxxE series oscilloscopes

Copyright 2015 by
Al Williams WD5GNR http://www.awce.com al.williams@awce.com
All Rights Reserved

Platform: Linux

License: GPL2

Note: This program is not affiliated in any way with Rigol. Rigol is doubtless
trademarked and it, along with other trademarks, are the property of their
respective owners.

# Features:


* Uses USB communications with Scope
* Allows keyboard to be unlocked so you can use both the panel and the software
* Allows easy reading of all measurements as well as logging of all measurements
* Control of common scope functions
* Saves waveforms in CSV format
* Integrates with external plot software like gnuplot to qtiplot
* Diagnostic mode to send raw commands to scope

# Known Limitations

* Does not manage the display, digital features, or some basic setup items (not planned)
* Does not handle Video or ALT trigger (Video would be easy to add; ALT is a lot of screens)
* Does not always set limits correctly for some controls (will fix)
* Some cosmetic defects on layouts (will fix)
* Tab order and other keyboard usability features not right (will fix)
* No Icon (will fix)
* Not all tool tip help is in place (will fix)
* No help system (will fix)
* Does not do XY, Delayed, or Roll mode
* Does not correctly handle floating point if your locale uses "," as a separator (will fix)

# Binary packages
In the packages directory is a 64-bit DEB and RPM. This is currently at version 0.1 which is older than
what you would get by cloning off of github and building.

# Notes
If you have older firmware, read below. You need permissions on your scope's device (read below).
To avoid confusion: the PC connects to the BACK of the Rigol scope, not the port in the front.

# Screen Shots

The interface consists of tabs. This one lets you connect to the device and exercise basic control:

![Screenshot](https://raw.githubusercontent.com/wd5gnr/qrigol/master/screenshots/screenshot_200.png)

Here's the vertical tab:

![Screenshot](https://raw.githubusercontent.com/wd5gnr/qrigol/master/screenshots/screenshot_201.png)

The Horizontal tab:

![Screenshot](https://raw.githubusercontent.com/wd5gnr/qrigol/master/screenshots/screenshot_202.png)

The trigger tab mutates depending on the type of trigger (EDGE, PULSE, and SLOPE currently supported)

![Screenshot](https://raw.githubusercontent.com/wd5gnr/qrigol/master/screenshots/screenshot_203.png)


You can read all measurements and also log them to CSV format in a variety of way on demand or on a timer

![Screenshot](https://raw.githubusercontent.com/wd5gnr/qrigol/master/screenshots/screenshot_204.png)


This tab lets you export the data buffers or plot them with an external tool, including Open Logic
Sniffer or Sigrok.

![Screenshot](https://raw.githubusercontent.com/wd5gnr/qrigol/master/screenshots/screenshot_205.png)


# Building

Use QtCreator and load the .pro file as a project. Select a kit for your configuration and build.

Might work on Windows, although you'd almost surely have to change the port selection and there might
be a few other things that don't work. No idea. Use Linux!

# Permissions

You need to have read/write permissions on the usbtmc file you are using for your scope. One easy way
to do this is to install sigrok which has definitions for many common scopes including your Rigol.
The setup there puts read write permission in the plugdev group, so you need to be a member of that
group. If your distribution's package of Sigrok doesn't include the udev setup, you might want
to read this: http://sigrok.org/wiki/Fx2lafw#Install_the_udev_rules_file.

If you prefer, you can try the following file (save as /etc/udev/rules.d/40-rigol.rules)

    ACTION!="add|change", GOTO="rigol_rules_end"
    SUBSYSTEM!="usb|usbmisc|usb_device", GOTO="rigol_rules_end"

    # Rigol DS1000 series
    ATTRS{idVendor}=="1ab1", ATTRS{idProduct}=="0588", MODE="664", GROUP="plugdev"

    # Rigol DS2000 series
    ATTRS{idVendor}=="1ab1", ATTRS{idProduct}=="04b0", MODE="664", GROUP="plugdev"

    # Rigol DG4000 series
    ATTRS{idVendor}=="1ab1", ATTRS{idProduct}=="0641", MODE="664", GROUP="plugdev"

    LABEL="rigol_rules_end"

After you fix udev, you need to restart udev (or reboot the PC if you are lazy). You probably need a command
like "sudo /etc/init.d/udev restart"

Another alternative (not suggested) is to run the program as root. Or, issue "sudo chmod 666 /dev/usbtmc0"
(using whatever number your device is in place of 0, of course). You'd have to do this on every reboot
or put it (without the sudo) in local.rc or some other autostart location.

# A Note About Firmware Versions

I tried to make the program compatible with the old and new format of data. However, there is at least
one report of it not working on an old (pre 4.0) version of the firmware (in particular, the export
functions will fail). Please report this behavior and I will send you a test version. I will remove
this part of the README if and when this is resolved.

