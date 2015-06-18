whistle.city
============

Whistle.city is a social urban game that improves the perception of neighbourhoods. It works with interactive objects called Sparrows, which are adopted by participating citizens and organisations and attached to their building’s windows.
When someone whistles in the vicinity of a Sparrow, it lights up playfully in various colours and animations. The Sparrows are network-connected into the [whistle.city](http://whistle.city) platform on which activity data is visualised. You can follow how your street’s Sparrow is doing. Collectively you can grow your neighbourhood Sparrow, and get it in fantastic shape!

Whistle.city has been played in various forms and names, such as in Ghent (ZWERM, 2013) and is still active in Hasselt (De Mussen van Hasselt).

The Sparrows are Open Source hardware and software, so anyone can build and adapt on the concept (for non-commercial purposes). Citizens, local organisations, coders, school teachers, students, hackers, artists, social workers, researchers, FabLab masters,… can be gathered in workshops to identify locations, explore collaborations and discuss possible integration and adaption of the whistle.city project in your city. 

Hardware
--------
You can find construction details and the descriptions on the required materials in the "Sparrow Assembly Instructions”. If you are interested in buying a kit containing all required hardware for assembly, contact us via http://whistle.city

Software
--------
The Sparrow software is [OpenFrameworks](http://openframeworks.cc) code. We use an [Olimex A20](https://www.olimex.com/Products/OLinuXino/A20/A20-OLinuXino-MICRO/open-source-hardware) running Debian Linux, but should be easily adaptable to [Raspberry Pie II](https://www.raspberrypi.org). If configuring Debian for OpenFrameworks and installing the ofxHwdPlugin addon dependencies confuses you, contact us for a fully functional disk image and/or further instructions at [whistle.city](http://whistle.city)

1. Follow [instructions](https://www.olimex.com/wiki/A20-OLinuXino-MICRO) of installing and configuring the Olimex A20
2. Download [OpenFrameworks for ARMv7](http://openframeworks.cc/download/) and follow the instructions.
3. Copy the "ofxHwdPlugin" addon to your OpenFrameworks addon folder. Currently works on ARM Debian, but we are working on a mac-port. Copy the SparrowsII folder to your OpenFrameworks apps > myApps folder. 
4. You will probably want to enable some scripts to auto-start the Sparrows application on startup. 