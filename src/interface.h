/*
 * DO NOT EDIT THIS FILE - it is generated by Glade.
 */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef INTERFACE_H
#define INTERFACE_H

#include "support.h"
#include "misc.h"
void check_up_theme( GtkWidget *window1, APP_data *data_app );
void get_theme_selection_color(GtkWidget *widget);
void set_up_view( GtkWidget *window1, APP_data *data_app);
GtkWidget *main_wp_toolbar(GtkWidget *window, APP_data *data_app);
GtkWidget *UI_main_window(GApplication *app, APP_data *data);
void UI_headerBar(GtkWidget *window, GtkWidget *grid, APP_data *data);
void UI_statusbar(GtkWidget *window, GtkWidget *grid, APP_data *data);
void UI_pdf_page_widget (GtkWidget *window, GtkWidget *grid, APP_data *data);

GtkWidget *create_loadFileDialog (APP_data *data, gchar *sFileType);
GtkWidget *create_saveFileDialog (APP_data *data);
GtkWidget *misc_create_help_dialog(GtkWidget *win);
// GtkWidget *create_prefs_dialog(GtkWidget *win, APP_data *data_app);
gchar *dialog_add_text_annotation(GtkWidget *win, gchar *current_str, APP_data *data);
GtkWidget *create_menu_sketch(GtkWidget *win, APP_data *data_app);
GtkWidget *create_menu_PDF(GtkWidget *win, APP_data *data_app);
GtkWidget*
create_annotationColourDialog (APP_data *data_app, gchar *msg);
GtkWidget *misc_create_go_jump_dialog(APP_data *data_app);
GtkWidget *create_aboutRedac (APP_data *data_app);
void redac_prepare_GUI (GApplication *app, APP_data *data);
#endif /* INTERFACE_H */
