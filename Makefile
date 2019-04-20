CC=gcc
CFLAGS=-w -W -pedantic -O2
RM=/bin/rm -f
BINARY=redac
LDFLAGS=`pkg-config --cflags  poppler-glib glib-2.0 gtk+-3.0 gtkspell3-3.0 gstreamer-1.0 `
PREFIX = /usr
SRC=src/
PACKAGE=redac
redac: main.o undo.o misc.o  pdf.o search.o mttfiles.o mttexport.o mttimport.o interface.o callbacks.o support.o audio.o paving.o
	${CC} ${CFLAGS} main.o undo.o misc.o  pdf.o search.o mttfiles.o mttexport.o mttimport.o interface.o callbacks.o support.o audio.o paving.o `pkg-config  gstreamer-1.0  gtkspell3-3.0 poppler-glib glib-2.0 gtk+-3.0  --libs`  -o ${BINARY} 


paving.o: ${SRC}paving.c
	${CC} ${CFLAGS} ${LDFLAGS} -c ${SRC}paving.c

audio.o: ${SRC}audio.c
	${CC} ${CFLAGS} ${LDFLAGS} -c ${SRC}audio.c

support.o: ${SRC}support.c
	${CC} ${CFLAGS} ${LDFLAGS} -c ${SRC}support.c 

callbacks.o: ${SRC}callbacks.c
	${CC} ${CFLAGS} ${LDFLAGS} -c  ${SRC}callbacks.c 

interface.o: ${SRC}interface.c
	${CC} ${CFLAGS} ${LDFLAGS} -c  ${SRC}interface.c 

mttfiles.o: ${SRC}mttfiles.c
	${CC} ${CFLAGS} ${LDFLAGS} -c  ${SRC}mttfiles.c 

mttexport.o: ${SRC}mttexport.c
	${CC} ${CFLAGS} ${LDFLAGS} -c  ${SRC}mttexport.c 

mttimport.o: ${SRC}mttimport.c
	${CC} ${CFLAGS} ${LDFLAGS} -c  ${SRC}mttimport.c 

search.o: ${SRC}search.c
	${CC} ${CFLAGS} ${LDFLAGS} -c  ${SRC}search.c 

pdf.o: ${SRC}pdf.c
	${CC} ${CFLAGS} ${LDFLAGS} -c  ${SRC}pdf.c 

misc.o: ${SRC}misc.c
	${CC} ${CFLAGS} ${LDFLAGS} -c  ${SRC}misc.c 

undo.o: ${SRC}undo.c
	${CC} ${CFLAGS} ${LDFLAGS} -c  ${SRC}undo.c 

main.o: ${SRC}main.c
	${CC} ${CFLAGS} ${LDFLAGS} -c  ${SRC}main.c

.PHONY: clean
clean: 
	${RM} *.o redac

.PHONY: install
install:
	echo "-------------------------------"
	echo "* I install binaries *"
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp ${BINARY} $(DESTDIR)$(PREFIX)/bin/${BINARY} 
	echo "* I install desktop integration files *"
	mkdir -p $(DESTDIR)$(PREFIX)/share/applications
	cp datas/${BINARY}.desktop $(DESTDIR)$(PREFIX)/share/applications/${BINARY}.desktop
	mkdir -p $(DESTDIR)$(PREFIX)/share/pixmaps
	cp icons/${BINARY}.png $(DESTDIR)$(PREFIX)/share/pixmaps/${BINARY}.png
	echo "* TODO : I will install help files ;-) "
	echo "* I install localization files *"
	mkdir -p $(DESTDIR)$(PREFIX)/share/locale/fr/LC_MESSAGES
	cp po/fr.mo $(DESTDIR)$(PREFIX)/share/locale/fr/LC_MESSAGES/${BINARY}.mo
	echo "* Installation done ! "

.PHONY: uninstall
uninstall:
	echo "* I remove binaries *"
	rm -f $(DESTDIR)$(PREFIX)/bin/${BINARY}
	rm -f $(DESTDIR)$(PREFIX)/share/pixmaps/${BINARY}.png
	rm -f $(DESTDIR)$(PREFIX)/share/applications/${BINARY}.desktop
	echo "* I remove localization files *"
	rm -f $(DESTDIR)$(PREFIX)/share/locale/fr/LC_MESSAGES/${BINARY}.mo
