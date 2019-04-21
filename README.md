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
Since there isn't, for yet, any **configure** script, you're supposed to install yourself required libraries. There is a **makefile** in root directory, when you've unzipped or 'branched' Redac sources. You can examine and edit/change default values (the makefile is **very simple**) if your Gnu/linux system isn't standard.

Once all dependancies are installed, just type (within **redac/** source directory) :

**make**

if something is wrong, retry :

**make**

If you want to test Redac before installation, just type, within source directory :

**./redac**

In order to install software :

**sudo make install**

Once installed, 'Redac' is present in your **desktop menu.**

In order to clean directory of object files :

**make clean**

And of course, if you want to remove Redac :

**sudo make uninstall** 

