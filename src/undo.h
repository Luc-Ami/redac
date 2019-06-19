/*
 * File: undo.c header
 * Description: Contains config undo operations
 *
 *
 *
 */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef UNDO_H
#define UNDO_H

#include <gtk/gtk.h>
#include <glib.h>
#include <poppler.h>
#include "support.h"
#include "misc.h"

/* Local macros */

#define MAX_UNDO_OPERATIONS 20
#define OP_NONE 0
#define OP_INS_CHAR 1
#define OP_DEL_CHAR 2
#define OP_INS_BLOCK 3
#define OP_DEL_BLOCK 4
#define OP_INS_IMG 5
#define OP_DEL_IMG 6
#define OP_SET_BOLD 7
#define OP_SET_ITALIC 8
#define OP_SET_UNDERLINE 9
#define OP_SET_STRIKE 10
#define OP_SET_SUPER 11
#define OP_SET_SUB 12
#define OP_SET_HIGH 13
#define OP_SET_QUOTE 14
#define OP_REPLACE_TEXT 15
#define OP_ALIGN_LEFT 16
#define OP_ALIGN_CENTER 17
#define OP_ALIGN_RIGHT 18
#define OP_ALIGN_FILL 19

#define OP_UNSET_BOLD 20
#define OP_UNSET_ITALIC 21
#define OP_UNSET_UNDERLINE 22
#define OP_UNSET_STRIKE 23
#define OP_UNSET_SUPER 24
#define OP_UNSET_SUB 25
#define OP_UNSET_HIGH 26
#define OP_UNSET_QUOTE 27

#define OP_TOGGLE_CASE 30

#define OP_SET_TEXT_ANNOT 50
#define OP_SET_HIGHLIGHT_ANNOT 51
#define OP_SET_ANNOT_COLOR 52
#define OP_SET_ANNOT_STR 53
#define OP_SET_ANNOT_FREE_TEXT 54
#define OP_SET_ANNOT_BOX 55
#define OP_REMOVE_ANNOT 56

#define OP_SET_POINT 100
#define OP_PASTE_PIXBUF 101
#define OP_SKETCH_ANNOT 102



void undo_push(gint current_stack,  gint op, APP_data *data);
void undo_pop(gint current_stack, APP_data *data);
void undo_free_all(APP_data *data);
void undo_free_all_PDF_ops(APP_data *data);
void undo_free_all_sketch_ops(APP_data *data);
void undo_reset_serialized_buffer(APP_data *data);
#endif /* UNDO_H */
