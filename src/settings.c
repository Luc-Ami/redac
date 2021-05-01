/**************************
  preferences/settings
  module

**************************/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

/* translations */
#include <libintl.h>
#include <locale.h>

#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "support.h"
#include "misc.h"
#include "settings.h"

/* ------functions -------- */


/*******************************************
 settings dialog  preferences
 * we use an UI file WITHOUT
 * dialog window in order
 * to proper usage of the
 * HeaderBar

*******************************************/
GtkWidget *create_prefs_dialog (GtkWidget *win, APP_data *data_app)
{
  GKeyFile *keyString;
  GdkRGBA color;   
  gchar *newFont;
  gdouble rewGap, jumpGap, pen_width;

  GtkWidget *notebook, *content_area, *hbar;
  GtkWidget *configDialog;
  GtkWidget *configAutoSave, *configAutoReloadPDF;
  GtkWidget *configPromptQuit, *configPromptOverwrite, *configAutoRewindPlayer;
  GtkWidget *icon;
  GtkWidget *font_button_editor, *font_button_sketch;
  GtkWidget *color_button_editor_fg, *color_button_editor_bg;
  GtkWidget *color_button_PDF_bg, *color_button_sketch_bg;

  /* get datas from xml Glade file - CRITICAL : only in activate phase !!!! */
  GtkBuilder *builder = NULL;
  builder = gtk_builder_new ();
  GError *err = NULL; 
  
  /* loading GUI from a glade file  */
  if(!gtk_builder_add_from_file (builder, find_ui_file ("settings.ui"), &err)) {
     g_error_free (err);
     printf ("* REDAC CRITICAL : can't load glade UI file, quit application ! *\n");
     g_application_quit (G_APPLICATION(data_app->app));
  }  
  /* Get the dialog from the glade file. */
  gtk_builder_connect_signals (builder, data_app);
  notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook1"));
  data_app->tmpBuilder = builder;

 // https://stackoverflow.com/questions/53587997/how-to-fix-gtk-warning-content-added-to-the-action-area-of-a-dialog-using-he
  configDialog = gtk_dialog_new_with_buttons ( _("Settings"),
                                       GTK_WINDOW(data_app->appWindow),
                                       GTK_DIALOG_USE_HEADER_BAR, /// Use this FLAG here
                                       _("Cancel"),
                                       GTK_RESPONSE_CANCEL,
                                      _("Ok"),
                                       GTK_RESPONSE_OK,                                       
                                       NULL );

  gtk_window_set_destroy_with_parent (GTK_WINDOW (configDialog), TRUE);
  content_area = gtk_dialog_get_content_area (GTK_DIALOG(configDialog));
  gtk_container_add (GTK_CONTAINER(content_area), notebook);
  hbar  = gtk_dialog_get_header_bar (GTK_DIALOG(configDialog));
  gtk_header_bar_set_subtitle (GTK_HEADER_BAR (hbar), _("Define global settings for Redac!.")); 
  icon = gtk_image_new_from_icon_name ("preferences-other-symbolic", GTK_ICON_SIZE_DIALOG);
  gtk_widget_set_halign (GTK_WIDGET (icon), GTK_ALIGN_CENTER);
  gtk_widget_set_valign (GTK_WIDGET (icon), GTK_ALIGN_CENTER);
 
  gtk_widget_set_hexpand (GTK_WIDGET(icon), FALSE);
  
  gtk_widget_set_margin_start (GTK_WIDGET(icon), 5);
  gtk_widget_set_margin_top (GTK_WIDGET(icon), 5);  
  gtk_widget_show (icon);  
  gtk_header_bar_pack_start (GTK_HEADER_BAR (hbar), GTK_WIDGET(icon));
  /* get pointers on various GtkWidgets */
  configAutoSave = GTK_WIDGET(gtk_builder_get_object (data_app->tmpBuilder, "configAutoSave"));

  configAutoReloadPDF = GTK_WIDGET(gtk_builder_get_object (data_app->tmpBuilder, "configAutoReloadPDF"));

  configPromptQuit = GTK_WIDGET(gtk_builder_get_object (data_app->tmpBuilder, "configPromptQuit"));

  configPromptOverwrite = GTK_WIDGET(gtk_builder_get_object (data_app->tmpBuilder, "configPromptOverwrite"));

  font_button_editor = GTK_WIDGET(gtk_builder_get_object (data_app->tmpBuilder, "font_button_editor"));

  color_button_editor_bg = GTK_WIDGET(gtk_builder_get_object (data_app->tmpBuilder, "color_button_editor_bg"));

  color_button_editor_fg = GTK_WIDGET(gtk_builder_get_object (data_app->tmpBuilder, "color_button_editor_fg"));

  color_button_PDF_bg = GTK_WIDGET(gtk_builder_get_object (data_app->tmpBuilder, "color_button_PDF_bg"));

  /* sketch section */

  color_button_sketch_bg = GTK_WIDGET(gtk_builder_get_object (data_app->tmpBuilder, "color_button_sketch_bg"));

  font_button_sketch = GTK_WIDGET(gtk_builder_get_object (data_app->tmpBuilder, "font_button_sketch"));

  GtkAdjustment *pen_width_adj = gtk_adjustment_new (1, 1, 20, 1, 10, 0);
  GtkWidget *pen_width_Spin = GTK_WIDGET(gtk_builder_get_object (data_app->tmpBuilder, "pen_width_Spin"));
 
  /* audio settings section */
  
  GtkAdjustment *rewGap_adj = gtk_adjustment_new (2, 1, 10, 1, 1, 0);
  GtkWidget *rewGapSpin = GTK_WIDGET(gtk_builder_get_object (data_app->tmpBuilder, "rewGapSpin"));

  GtkAdjustment *jumpGap_adj = gtk_adjustment_new (10, 1, 600, 1, 10, 0);
  GtkWidget *jumpGapSpin = GTK_WIDGET(gtk_builder_get_object (data_app->tmpBuilder, "jumpGapSpin"));

  configAutoRewindPlayer = GTK_WIDGET(gtk_builder_get_object (data_app->tmpBuilder, "configAutoRewindPlayer"));

  /* current values */
  keyString = g_object_get_data(G_OBJECT(data_app->appWindow), "config");

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (configAutoSave), 
           g_key_file_get_boolean(keyString, "application", "interval-save",NULL ) );

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (configAutoReloadPDF),
           g_key_file_get_boolean(keyString, "application", "autoreload-PDF",NULL ) );
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (configPromptQuit),
            g_key_file_get_boolean(keyString, "application", "prompt-before-quit",NULL ));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (configPromptOverwrite),
            g_key_file_get_boolean(keyString, "application", "prompt-before-overwrite",NULL ));

  color.red = g_key_file_get_double(keyString, "editor", "text.color.red", NULL);
  color.green = g_key_file_get_double(keyString, "editor", "text.color.green", NULL);
  color.blue = g_key_file_get_double(keyString, "editor", "text.color.blue", NULL);
  color.alpha = 1;
  gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER(color_button_editor_fg), &color);
  color.red = g_key_file_get_double(keyString, "editor", "paper.color.red", NULL);
  color.green = g_key_file_get_double(keyString, "editor", "paper.color.green", NULL);
  color.blue = g_key_file_get_double(keyString, "editor", "paper.color.blue", NULL);
  color.alpha = 1;
  gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER(color_button_editor_bg), &color);
  color.red = g_key_file_get_double(keyString, "reference-document", "paper.color.red", NULL);
  color.green = g_key_file_get_double(keyString, "reference-document", "paper.color.green", NULL);
  color.blue = g_key_file_get_double(keyString, "reference-document", "paper.color.blue", NULL);
  color.alpha = 1;
  gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER(color_button_PDF_bg), &color);    
  color.red = g_key_file_get_double(keyString, "sketch", "paper.color.red", NULL);
  color.green = g_key_file_get_double(keyString, "sketch", "paper.color.green", NULL);
  color.blue = g_key_file_get_double(keyString, "sketch", "paper.color.blue", NULL);
  color.alpha = 1;
  gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER(color_button_sketch_bg), &color);
  gtk_font_chooser_set_font (GTK_FONT_CHOOSER(font_button_editor ), 
                 g_key_file_get_string(keyString, "editor", "font", NULL));
  gtk_font_chooser_set_font (GTK_FONT_CHOOSER(font_button_sketch ), 
                 g_key_file_get_string(keyString, "sketch", "font", NULL));
  pen_width = g_key_file_get_double(keyString, "sketch", "pen-width", NULL);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(pen_width_Spin), pen_width);

  rewGap = g_key_file_get_double(keyString, "application", "audio-file-rewind-step", NULL);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(rewGapSpin), rewGap);
  jumpGap = g_key_file_get_double(keyString, "application", "audio-file-marks-step", NULL);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(jumpGapSpin), jumpGap);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (configAutoRewindPlayer),
            g_key_file_get_boolean(keyString, "application", "audio-auto-rewind",NULL ));

  return configDialog;

}



