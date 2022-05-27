
#include <libintl.h>
#include <locale.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <glib.h>
#include <glib/gstdio.h> /* g_fopen, etc */
#include <gtk/gtk.h>
#include "support.h"
#include "callbacks.h"
#include "interface.h"
#include "mttfiles.h"
#include "misc.h"
#include "undo.h"
#include "cursor.h"


/*********************************
  startup for a standard
  g_application

**********************************/

static void redac_startup (APP_data *data)
{

  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);
  add_pixmap_directory (PACKAGE_DATA_DIR "/pixmaps/" PACKAGE); /* New location for all pixmaps */
  add_pixmap_directory (PACKAGE_DATA_DIR "/pixmaps"); /* Gnome users /usr/share/pixmaps folder */  
  add_ui_directory (PACKAGE_DATA_DIR "/redac/ui" ); /* New location for all glade xml files */

}


/*********************************
  shutdown for a standard
  g_application called always by
  * g_application_quit()
**********************************/
static void
redac_shutdown (GApplication *app, APP_data *data)
{
	printf ("* Redac : clearing clipboard *\n");
	gtk_clipboard_clear (
	                      gtk_clipboard_get (GDK_SELECTION_CLIPBOARD)/* in order to avoid issues hash_table and sometimes segfaults */
	                    );
}	


/*********************************
  activation for a standard
  g_application
**********************************/
static void
redac_activate (GApplication *app, APP_data *data)
{
  GList *list;
  GtkWidget *window, *scrolledwindow1;
  GError *err = NULL; 

  /* get datas from xml Glade file - CRITICAL : only in activate phase !!!! */
  data->builder = NULL;
  data->builder = gtk_builder_new ();

  /* load UI file */

  if(!gtk_builder_add_from_file (data->builder, find_ui_file ("main.ui"), &err)) {
     g_error_free (err);
     misc_halt_after_glade_failure (data);
  }

  list = gtk_application_get_windows (GTK_APPLICATION(app));
  /* test for uniqueness */
  if (list)
    {
      gtk_window_present (GTK_WINDOW (list->data));
    }
  else
    {
      redac_prepare_GUI (app, data);
      gtk_window_set_application (GTK_WINDOW (data->appWindow), GTK_APPLICATION(app));
      gtk_widget_show (GTK_WIDGET(data->appWindow));
      gtk_widget_show (GTK_WIDGET(data->view));
      /* move to something like end of text */
      scrolledwindow1 = lookup_widget (GTK_WIDGET(data->appWindow), "scrolledwindow1");
      // misc_jump_to_end_view (scrolledwindow1, data->buffer, data->view);
      cursor_normal (data->appWindow);
    }
}

/**********************

  MAIN

***********************/

int main (int argc, char *argv[]) {
  GtkApplication *app;
  GApplicationFlags flags = G_APPLICATION_FLAGS_NONE;
  APP_data app_data;
  gint status;

  /* get infos from config.h */
  printf ("version %s \n", PACKAGE_VERSION);/* first autoge,sh then .c:confure and finally make modify config.h */  

  /* Initialize GStreamer */
  gst_init (&argc, &argv);
  app = gtk_application_new ("org.gtk.redac", flags);
  app_data.app = G_APPLICATION (app);
  g_signal_connect (app_data.app, "startup", G_CALLBACK (redac_startup), &app_data);
  g_signal_connect (app_data.app, "activate", G_CALLBACK (redac_activate), &app_data);
  g_signal_connect (app_data.app, "shutdown", G_CALLBACK (redac_shutdown), &app_data);
  /* main loop */
  status = g_application_run (G_APPLICATION (app_data.app), argc, argv);

  g_object_unref (app_data.builder);

  g_object_unref (app);

  return status;
}
