/*
 * File: mttimport.c header
 * Description: Contains importation specific functions so that widgets can keep their
 *              settings betweeen saves.
 *
 *
 *
 */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef MTTIMPORT_H
#define MTTIMPORT_H

#include <gtk/gtk.h>
#include "misc.h"

/* from Searchmonkey  */
/***************************
from catdoc and me ;-)
***************************/
#define fDot 0x0001   
#define fGlsy 0x0002
#define fComplex 0x0004
#define fPictures 0x0008 
#define fEncrypted 0x100
#define fReadOnly 0x400
#define fReserved 0x800
#define fExtChar 0x1000
#define fWord6 0xA5CC
#define fWord95 0xA5DC
#define fWord97 0xA5EC
#define iUnknownFile 0
#define iOldMSWordFile 1
#define iOleMsWordFile 2
#define iXmlMsWordFile 3
#define iMsWriteFile 4
#define iXmlOdtFile 5
#define iRtfFile 6
#define iPdfFile 7
#define iAbiwordFile 8
#define iXmlOdpFile 9
#define iXmlOdsFile 10
#define iXmlMsXlsFile 11
#define iXmlMsPptFile 12
/* for code pages conversions */
#define iCpUtf16 1
#define iCpW1252 2
#define iCpIbm437 3
#define iCpIso8859_1 4


void
on_import_clicked (GtkButton *button, APP_data *data_app);

#endif /* MTTIMPORT_H */
