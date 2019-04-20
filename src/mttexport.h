/*
 * File: mttexport.c header
 * Description: Contains config save/restore specific functions so that widgets can keep their
 *              settings betweeen saves.
 *
 *
 *
 */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef MTTEXPORT_H
#define MTTEXPORT_H

#include <gtk/gtk.h>
#include "misc.h"

/* Local macros */

#define MAX_TAGS_STACK_SIZE 256 /* maximum size of the stack containg Gtk Tags during conversion */
#define STACK_TAG_NONE 0
#define STACK_TAG_BOLD 1
#define STACK_TAG_ITALIC 2
#define STACK_TAG_UNDERLINE 4
#define STACK_TAG_STRIKE 8
#define STACK_TAG_HIGHLIGHT 16
#define STACK_TAG_LEFT 32
#define STACK_TAG_CENTER 64
#define STACK_TAG_RIGHT 128
#define STACK_TAG_FILL 256
#define STACK_TAG_SUPERSCRIPT 512
#define STACK_TAG_SUBSCRIPT 1024
#define STACK_TAG_QUOTATION 2048


gint save_RTF_rich_text(gchar *filename, GtkTextBuffer *buffer);

#endif /* MTTEXPORT_H */
