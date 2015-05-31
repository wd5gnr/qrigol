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

# Screen Shots

The interface consists of tabs. This one lets you connect to the device and exercise basic control:

![Screenshot](https://raw.githubusercontent.com/wd5gnr/qrigol/master/screenshots/screenshot_126.jpg)

Here's the vertical tab:

![Screenshot](https://raw.githubusercontent.com/wd5gnr/qrigol/master/screenshots/screenshot_127.jpg)

The Horizontal tab:

![Screenshot](https://raw.githubusercontent.com/wd5gnr/qrigol/master/screenshots/screenshot_128.jpg)

The trigger tab mutates depending on the type of trigger (EDGE, PULSE, and SLOPE currently supported)

![Screenshot](https://raw.githubusercontent.com/wd5gnr/qrigol/master/screenshots/screenshot_129.jpg)

This tab lets you export the data buffers or plot them with an external tool

![Screenshot](https://raw.githubusercontent.com/wd5gnr/qrigol/master/screenshots/screenshot_130.jpg)

