# Redac

![Redac Icon](https://nsa40.casimages.com/img/2019/04/21/190421110735999272.png)
Redac is a note taker, PDF editor, sketch manager and audio transcriber tool for students and teachers, written with Gtk3 toolkit. It's intended to be simple, light, usefull for academic purposes.
![iHM preview](https://nsa40.casimages.com/img/2019/04/21/19042111032873051.png)

Required libraries
---
In order to successfuly compile with the providen Makefile, you must install in your GNU/linux machine :

- libgtk3 and its development files
- libinitl "      "      "      "
- libpoppler, libpoppler-glib >=0.26.5 "   " "" 
- libgtkspell3-3-dev
- libgtkspell3-3-0
- libenchant-dev
- libgstreamer1.0-dev
- ibgstreamer1.0-dev base plugins

Those names are for Ubuntu, please note that with your distribution they can change.
There is a **wiki** frequently updated.
You can also find a tutorial (in french) for this software in **datas/** directory.

Compilation :
-------------


Redac if built upon standard GNU compilation chain and autootols.
Before compiling, prepare building chain  :

**chmod +x autogen.sh

./autogen.sh**

Then, after installation of dependencies :

**./configure**

Build Redac locally :

**make**

Test Redac locally :

**./src/redac**

Install Redac for your system

**sudo make install**

Clean directories :

**make clean**



