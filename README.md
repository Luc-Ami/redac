# Redac

![Redac Icon](https://nsa40.casimages.com/img/2019/04/21/190421110735999272.png)
Redac is a note taker, PDF editor, sketch manager and audio transcriber tool for students and teachers, written with Gtk3 toolkit
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

Those names if for Ubuntu, with your distribution they can change.
There is a **wiki** frequently updated.
You can also find a tutorial (in french) for this software in **datas/** directory.

Compilation :
-------------
once all dependancies are installed, just type (within **redac/** source directory) :

**make**

if something is wrong, retry :

**make**


In order to install software :

**sudo make install**

Once installed, 'Redac' is present in your **desktop menu.**

In order to clean directory for objects files :

**make clean**

And of course :
**sudo make uninstall** 
if you want to remove Redac.
