AM_CFLAGS = $(REDAC_CFLAGS)
AM_CPPFLAGS = -DLOCALEDIR=\""$(localedir)"\"
bin_PROGRAMS = redac
redac_SOURCES = \
	main.c \
	undo.c undo.h\
	misc.c misc.h \
	pdf.c pdf.h \
	search.c search.h \
	mttfiles.c mttfiles.h \
	mttexport.c mttexport.h \
	mttimport.c mttimport.h \
	interface.c interface.h \
	callbacks.c callbacks.h \
	support.c support.h \
	audio.c audio.h \
	paving.c paving.h

redac_LDADD = $(REDAC_LIBS)

