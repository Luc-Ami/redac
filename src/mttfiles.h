/*
 * File: mttfiles.c header
 * Description: Contains config save/restore specific functions so that widgets can keep their
 *              settings betweeen saves.
 *
 *
 *
 */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef MTTFILES_H
#define MTTFILES_H

#include <gtk/gtk.h>
#include "misc.h"

/* Local macros */
#define CONFIG_DISABLE_SAVE_CONFIG_STRING "CONFIG_DISABLE_SAVE_CONFIG" /* Used by config->restart to check for reset */
#define CONFIG_DISABLE_SAVE_CONFIG 1 /* Used by config->restart to check for reset */
#define CURRENT_VERSION "0.0.1"  /*Current version of Kilowriter */
#define MASTER_OPTIONS_DATA "options1" /* Storage for the Phase One data inside the g_object */
#define MAX_RECENT_FILES 10

/*
 structure to store configuration
 cf. http://www.gtkbook.com/tutorial.php?page=keyfile
*/
typedef struct
{
  gchar *name;
  gint x_mainwindow, y_mainwindow, w_mainwindow, h_mainwindow;
} mttSettings;

extern gchar *gConfigFile;
extern GtkWidget *mainWindow;
gint storage_save( gchar *pathname, APP_data *data_app);
void createGKeyFile(APP_data *data_app, GtkWidget *win);
void destroyGKeyFile(APP_data *data_app, GtkWidget *win);
void storeGKeyFile(APP_data *data_app, GKeyFile *keyString);
void store_current_file_in_keyfile(GKeyFile *keyString, gchar *filename, gchar *summary);
void rearrange_recent_file_list(GKeyFile *keyString);
gchar *GetTempFileName(gchar *fileSchema);
gint load_gtk_rich_text(gchar *filename, GtkTextBuffer *buffer, GtkWidget *window1, APP_data *data);
gint save_gtk_rich_text(gchar *filename, GtkTextBuffer *buffer);
void
on_open_clicked                 (GtkButton       *button,
                                        APP_data *data_app);
gboolean
on_quit_clicked                 (GtkWidget *window1, GdkEvent  *event, APP_data *data_app);
void quick_save(  APP_data    *data);
void save_standard_file(GtkMenuItem *menuitem,  APP_data    *data);
void
    on_loadAudio_clicked  (GtkButton *button, APP_data *data);
void
on_AudioCloseFile_clicked  (GtkButton *button, APP_data *data);
void
    on_loadPDF_clicked       (GtkButton  *button, APP_data *data);
void quick_load_PDF(gchar *filename, APP_data *data);
void
    on_savePDF_clicked       (GtkButton  *button, APP_data *data);
void
on_saveSketch_clicked        (GtkButton  *button, APP_data *data_app);
void
on_help_clicked                 (GtkWidget        *win);
void new_project(GtkMenuItem *menuitem,
               APP_data    *user_data);
void file_alert_dialog (gchar *filename, GtkWidget *window1 );
#endif /* MTTFILES_H */
