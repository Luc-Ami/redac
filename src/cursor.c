/**********************************
* 
* this module is part of Redac
 * liucence : GPL v3
********************************/


#include <gtk/gtk.h>
#include "misc.h"

static GdkCursor *CursorNew = NULL;

/*************************************/
/* busy cursor           */
/***********************************/
void cursor_busy (GtkWidget *w)
{
  GdkDisplay *display;

  display = gtk_widget_get_display (w);

  if (CursorNew!=NULL) {
	  g_object_unref (CursorNew);
  }
  CursorNew = gdk_cursor_new_for_display (display, GDK_WATCH);
  gdk_window_set_cursor (gtk_widget_get_window (w), CursorNew);
  
}

/*************************************/
/* pointer (hand) cursor           */
/***********************************/
void cursor_pointer_hand (GtkWidget *w)
{
  GdkDisplay *display;

  display = gtk_widget_get_display (w);

  if (CursorNew!=NULL) {
	  g_object_unref (CursorNew);
  }
  CursorNew = gdk_cursor_new_for_display (display, GDK_HAND2);
  gdk_window_set_cursor (gtk_widget_get_window (w), CursorNew);

}

/*************************************/
/* move object     cursor           */
/***********************************/
void cursor_move_task (GtkWidget *w)
{
  GdkDisplay *display;

  display = gtk_widget_get_display (w);

  if (CursorNew!=NULL) {
	  g_object_unref (CursorNew);
  }
  CursorNew = gdk_cursor_new_for_display (display, GDK_FLEUR);
  gdk_window_set_cursor (gtk_widget_get_window (w), CursorNew);

}

/*************************************/
/* resize task cursor           */
/***********************************/
void cursor_resize_task (GtkWidget *w)
{
  GdkDisplay *display;

  display = gtk_widget_get_display (w);

  if (CursorNew!=NULL) {
	  g_object_unref (CursorNew);
  }
  CursorNew = gdk_cursor_new_for_display (display, GDK_SB_H_DOUBLE_ARROW);
  gdk_window_set_cursor (gtk_widget_get_window (w), CursorNew);

}


/*************************************/
/*  normal cursor         */
/***********************************/
void cursor_normal (GtkWidget *w)
{
  GdkDisplay *display;

  display = gtk_widget_get_display (w);

  if (CursorNew!=NULL) {
	  g_object_unref (CursorNew);
  }
  CursorNew = gdk_cursor_new_from_name (display, "default");
  gdk_window_set_cursor (gtk_widget_get_window(w), CursorNew);

}

/********************
  free memory
********************/
void cursor_free (void)
{
  if (CursorNew!=NULL) {
	  g_object_unref (CursorNew);
  }
  CursorNew = NULL;
}


/*************************************/
/* draw  cursor           */
/***********************************/
void cursor_dot_pencil (GtkWidget *w)
{
  GdkDisplay *display;

  display = gtk_widget_get_display (w);

  if (CursorNew!=NULL) {
	  g_object_unref (CursorNew);
  }
  CursorNew = gdk_cursor_new_for_display (display, GDK_DOT);
  gdk_window_set_cursor (gtk_widget_get_window (w), CursorNew);

}

/*************************************/
/* draw  cursor           */
/***********************************/
void cursor_cross (GtkWidget *w)
{
  GdkDisplay *display;

  display = gtk_widget_get_display (w);

  if (CursorNew!=NULL) {
	  g_object_unref (CursorNew);
  }
  CursorNew = gdk_cursor_new_for_display (display, GDK_CROSSHAIR);
  gdk_window_set_cursor (gtk_widget_get_window (w), CursorNew);

}

/*************************************/
/* hand  cursor           */
/***********************************/
void cursor_hand (GtkWidget *w)
{
  GdkDisplay *display;

  display = gtk_widget_get_display (w);

  if (CursorNew!=NULL) {
	  g_object_unref (CursorNew);
  }
  CursorNew = gdk_cursor_new_for_display (display, GDK_HAND2);
  gdk_window_set_cursor (gtk_widget_get_window (w), CursorNew);

}

/*************************************/
/* hand  cursor           */
/***********************************/
void cursor_icon (GtkWidget *w)
{
  GdkDisplay *display;

  display = gtk_widget_get_display (w);

  if (CursorNew!=NULL) {
	  g_object_unref (CursorNew);
  }
  CursorNew = gdk_cursor_new_for_display (display, GDK_ICON);
  gdk_window_set_cursor (gtk_widget_get_window (w), CursorNew);

}


/*************************************/
/* hand  cursor           */
/***********************************/
void cursor_text_select (GtkWidget *w)
{
  GdkDisplay *display;

  display = gtk_widget_get_display (w);

  if (CursorNew!=NULL) {
	  g_object_unref (CursorNew);
  }
  CursorNew = gdk_cursor_new_for_display (display, GDK_XTERM);
  gdk_window_set_cursor (gtk_widget_get_window (w), CursorNew);

}



