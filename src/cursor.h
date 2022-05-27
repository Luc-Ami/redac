/*
 * various utilities and constants
 * this module is part of Redac
 * liucence : GPL v3
 */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


#ifndef CURSOR_H
#define CURSOR_H

#include <gtk/gtk.h>
#include "support.h"


/* functions */
void cursor_busy (GtkWidget *w);
void cursor_resize_task (GtkWidget *w);
void cursor_move_task (GtkWidget *w);
void cursor_normal (GtkWidget *w);
void cursor_free (void);
void cursor_pointer_hand (GtkWidget *w);
void cursor_dot_pencil (GtkWidget *w);
void cursor_cross (GtkWidget *w);
void cursor_hand (GtkWidget *w);
void cursor_icon (GtkWidget *w);
void cursor_text_select (GtkWidget *w);

#endif /* CURSOR_H */
