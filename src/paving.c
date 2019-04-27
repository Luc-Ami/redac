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
#include "paving.h"
/********************************
 callbacks

********************************/
/*
  callback : button cancel clicked 

*/
void
on_cancel_clicked (GtkButton *button, APP_data *data)
{


 gtk_dialog_response (GTK_DIALOG(lookup_widget(GTK_WIDGET(button), "pavingDialog") ),
                     GTK_RESPONSE_CANCEL);

}


/*
  callback : button [1] file #1 clicked 

*/
void
on_recent1_clicked (GtkButton *button, APP_data *data)
{


 gtk_dialog_response (GTK_DIALOG(lookup_widget(GTK_WIDGET(button), "pavingDialog") ),
                       PAVING_BUTTON1);

}
/*
  callback : button [2] file #1 clicked 

*/
void
on_recent2_clicked (GtkButton *button, APP_data *data)
{


 gtk_dialog_response (GTK_DIALOG(lookup_widget(GTK_WIDGET(button), "pavingDialog") ),
                       PAVING_BUTTON2);

}

/*
  callback : button [3] file #1 clicked 

*/
void
on_recent3_clicked (GtkButton *button, APP_data *data)
{


 gtk_dialog_response (GTK_DIALOG(lookup_widget(GTK_WIDGET(button), "pavingDialog") ),
                       PAVING_BUTTON3);

}

/*
  callback : button [4] file #1 clicked 

*/
void
on_recent4_clicked (GtkButton *button, APP_data *data)
{


 gtk_dialog_response (GTK_DIALOG(lookup_widget(GTK_WIDGET(button), "pavingDialog") ),
                       PAVING_BUTTON4);

}

/*
  callback : button [5] file #1 clicked 

*/
void
on_recent5_clicked (GtkButton *button, APP_data *data)
{


 gtk_dialog_response (GTK_DIALOG(lookup_widget(GTK_WIDGET(button), "pavingDialog") ),
                       PAVING_BUTTON5);

}
/*
  callback : button [6] file #1 clicked 

*/
void
on_recent6_clicked (GtkButton *button, APP_data *data)
{


 gtk_dialog_response (GTK_DIALOG(lookup_widget(GTK_WIDGET(button), "pavingDialog") ),
                       PAVING_BUTTON6);

}

/*
  callback : button [7] file #1 clicked 

*/
void
on_recent7_clicked (GtkButton *button, APP_data *data)
{


 gtk_dialog_response (GTK_DIALOG(lookup_widget(GTK_WIDGET(button), "pavingDialog") ),
                       PAVING_BUTTON7);

}

/*
  callback : button [8] file #1 clicked 

*/
void
on_recent8_clicked (GtkButton *button, APP_data *data)
{


 gtk_dialog_response (GTK_DIALOG(lookup_widget(GTK_WIDGET(button), "pavingDialog") ),
                       PAVING_BUTTON8);

}

/*
  callback : button [6] file #1 clicked 

*/
void
on_recent9_clicked (GtkButton *button, APP_data *data)
{


 gtk_dialog_response (GTK_DIALOG(lookup_widget(GTK_WIDGET(button), "pavingDialog") ),
                       PAVING_BUTTON9);

}

/*
  callback : button [10] file #1 clicked 

*/
void
on_recent10_clicked (GtkButton *button, APP_data *data)
{


 gtk_dialog_response (GTK_DIALOG(lookup_widget(GTK_WIDGET(button), "pavingDialog") ),
                       PAVING_BUTTON10);

}
/********************************
  build a block button 
  parameters : - rank in order
  to set up accordingly to CSS
  - name of file
  - summary of file
  Output : a pointer on a new 
  button widget

*******************************/

GtkWidget *paving_new_button_block(gint rank, gchar *strFile, gchar *str )
{
  GtkWidget *btn;
  GtkWidget *label_title;
  GtkWidget *label_content;
  GtkWidget *grid;

  grid=gtk_grid_new();
  label_title=gtk_label_new("");
  label_content=gtk_label_new("");
  gtk_misc_set_alignment (GTK_MISC(label_content), 0,0);
  gtk_label_set_markup (GTK_LABEL (label_title), 
                        g_markup_printf_escaped("<u><b>%s</b></u>",  strFile));
  gtk_label_set_markup (GTK_LABEL (label_content), 
                        g_markup_printf_escaped("<small>%.200s …</small>",  str));
  gtk_label_set_line_wrap (GTK_LABEL (label_content), TRUE);
  gtk_label_set_line_wrap (GTK_LABEL (label_title), TRUE);

  gtk_widget_set_hexpand(GTK_WIDGET(label_content), TRUE);
  gtk_widget_set_hexpand(GTK_WIDGET(label_title), TRUE);

  gtk_widget_set_vexpand(GTK_WIDGET(label_content), TRUE);

  gtk_widget_set_margin_top(label_title, 4);
  gtk_widget_set_margin_left(label_title, 4);
  gtk_widget_set_margin_right(label_title, 4);
  gtk_widget_set_margin_bottom(label_content, 4);
  gtk_widget_set_margin_left(label_content, 4);
  gtk_widget_set_margin_right(label_content, 4);
  gtk_misc_set_padding (GTK_MISC (label_content), 5, 5);
  g_object_set (grid, "margin", 0, NULL);

  gtk_grid_attach(GTK_GRID(grid),label_title, 0, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid),label_content, 0, 1, 1, 1);

  btn=gtk_button_new();
  gtk_widget_set_name(btn, g_strdup_printf("myButton_%d", rank));
  gtk_widget_set_name(label_title, g_strdup_printf("myButton_label_title_%d", rank));
  gtk_widget_set_name(label_content, g_strdup_printf("myButton_label_content_%d", rank));

  gtk_widget_set_size_request(btn, 160, 130);
  g_object_set (btn, "margin", 0, NULL);
  gtk_widget_set_can_focus (btn, TRUE);
 
  gtk_container_add (GTK_CONTAINER (btn), grid);
  return btn;
}

/***************************************
  modal window for paving
  please note : CSS are declared in
  interface.c module
***************************************/
GtkWidget *paving_window(APP_data *user_data )
{
  GtkWidget *pavingDialog;
  GtkWidget *gridDisplay;
  GtkWidget *gridTitle;
  GtkWidget *labelTitle;
  GtkWidget *subTitle;
  GtkWidget *iconTitle;

  GtkWidget *gridFiles;
  GtkWidget *window1 = user_data->appWindow;
  GtkWidget *dialog_vbox;
  GtkWidget *button1, *button2, *button3, *button4, *button5, *button6, *button7, *button8, *button9, *button10, *buttonCancel;
  GKeyFile *keyString;
  gchar *recent_prev, *content_prev;
  gint i, count=0;


  keyString=g_object_get_data(G_OBJECT(user_data->appWindow), "config");

  pavingDialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (pavingDialog), _("Recent notes ..."));
  gtk_widget_set_name(pavingDialog, "pavingDialog");
  gtk_window_set_position (GTK_WINDOW (pavingDialog), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (pavingDialog), TRUE);
  gtk_window_set_resizable (GTK_WINDOW (pavingDialog), TRUE);
  gtk_window_set_default_size(GTK_WINDOW(pavingDialog), 850, 600);
  gtk_window_set_resizable (GTK_WINDOW(pavingDialog), TRUE);
  gtk_window_set_type_hint (GTK_WINDOW (pavingDialog), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_transient_for (GTK_WINDOW (pavingDialog),  GTK_WINDOW(window1)); 
  dialog_vbox = gtk_dialog_get_content_area (GTK_DIALOG (pavingDialog));

  /* display grid */
  gridDisplay = gtk_grid_new ();
  gtk_widget_set_name(gridDisplay, "gridDisplay");
  gtk_grid_set_row_homogeneous(GTK_GRID(gridDisplay), FALSE);
  gtk_grid_set_column_homogeneous(GTK_GRID(gridDisplay), FALSE);
  gtk_container_set_border_width(GTK_CONTAINER (gridDisplay), 0);
  gtk_grid_set_column_spacing(GTK_GRID(gridDisplay), 0);
  gtk_grid_set_row_spacing(GTK_GRID(gridDisplay), 0);
  g_object_set (gridDisplay, "margin", 10, NULL);
  gtk_container_add (GTK_CONTAINER (dialog_vbox), gridDisplay);
  /* title section */
  gridTitle=gtk_grid_new ();
  gtk_widget_set_name(gridTitle, "gridTitle");
  gtk_grid_set_row_homogeneous(GTK_GRID(gridTitle), FALSE);
  gtk_grid_set_column_homogeneous(GTK_GRID(gridTitle), FALSE);
  gtk_container_set_border_width(GTK_CONTAINER (gridTitle), 0);
  gtk_grid_set_column_spacing(GTK_GRID(gridTitle), 0);
  gtk_grid_set_row_spacing(GTK_GRID(gridTitle), 0);
  g_object_set (gridTitle, "margin", 10, NULL);  
  gtk_grid_attach(GTK_GRID(gridDisplay), gridTitle, 0,0,1,1);
  /* icon */
  iconTitle=gtk_image_new_from_icon_name ("document-open-recent",GTK_ICON_SIZE_DIALOG);
  gtk_misc_set_alignment (GTK_MISC (iconTitle), 0, 0.5);
  gtk_widget_set_hexpand(GTK_WIDGET(iconTitle), FALSE);
  gtk_grid_attach(GTK_GRID(gridTitle), iconTitle, 0,0,1,1);
  /* title */
  labelTitle= gtk_label_new("");
  gtk_label_set_markup (GTK_LABEL (labelTitle),_("<big><b>Recent notes :</b></big>"));
  gtk_misc_set_alignment (GTK_MISC (labelTitle), 0, 0.5);
  gtk_widget_set_hexpand(GTK_WIDGET(labelTitle), FALSE);
  gtk_grid_attach(GTK_GRID(gridTitle), labelTitle, 1,0,1,1);
  subTitle= gtk_label_new("");
  gtk_grid_attach(GTK_GRID(gridTitle), subTitle, 1,1,2,1);

  /* grid for files  */
  gridFiles = gtk_grid_new ();
  gtk_grid_set_row_homogeneous(GTK_GRID(gridFiles), TRUE);
  gtk_grid_set_column_homogeneous(GTK_GRID(gridFiles), TRUE);
  gtk_container_set_border_width(GTK_CONTAINER (gridFiles), 0);
  gtk_grid_set_column_spacing(GTK_GRID(gridFiles), 0);
  gtk_grid_set_row_spacing(GTK_GRID(gridFiles), 0);
  gtk_widget_set_name(gridFiles, "recentGrid");
  g_object_set (gridFiles, "margin", 15, NULL);
  gtk_grid_attach(GTK_GRID(gridDisplay), gridFiles, 0,1,1,1);

  /* cancel button */
  /* gtk3 stuff for a button with text+image ! */
  buttonCancel = gtk_button_new_with_label (_("Cancel"));
  gtk_widget_set_name(buttonCancel, "myButtonCancel");
  GtkWidget *buttonCancelImage = gtk_image_new_from_icon_name ("gtk-cancel",  GTK_ICON_SIZE_DIALOG);
  gtk_button_set_always_show_image (GTK_BUTTON (buttonCancel), TRUE);
  gtk_button_set_image (GTK_BUTTON (buttonCancel), buttonCancelImage);
  gtk_widget_set_margin_left(buttonCancel, 32);
  gtk_widget_set_margin_right(buttonCancel, 32);
  gtk_grid_attach(GTK_GRID(gridTitle), buttonCancel, 3, 1, 1, 1);


  /* creates all buttons */
  if(g_key_file_has_key(keyString, "history", "recent-file-0", NULL)) {
                 recent_prev = g_strdup_printf("%s", g_key_file_get_string(keyString, "history", 
                                          "recent-file-0", NULL));
                 if(strlen(recent_prev)>0) {
                   /* summaries */
                   count++;
                   content_prev = g_strdup_printf("%s", g_key_file_get_string(keyString, "history", 
                                          "recent-content-0", NULL));
                   button1=paving_new_button_block(1, recent_prev, content_prev );
                   g_free(content_prev);
                   gtk_grid_attach(GTK_GRID(gridFiles), button1, 0, 0, 1, 1);
                   g_signal_connect(G_OBJECT(button1), "clicked", 
                              G_CALLBACK(on_recent1_clicked), user_data);
                 }
                 g_free(recent_prev);
  }

  if(g_key_file_has_key(keyString, "history", "recent-file-1", NULL)) {
                 recent_prev = g_strdup_printf("%s", g_key_file_get_string(keyString, "history", 
                                          "recent-file-1", NULL));
                 if(strlen(recent_prev)>0) {
                   /* summaries */
                   count++;
                   content_prev = g_strdup_printf("%s", g_key_file_get_string(keyString, "history", 
                                          "recent-content-1", NULL));
                   button2=paving_new_button_block(2, recent_prev, content_prev );
                   g_free(content_prev);
                   gtk_grid_attach(GTK_GRID(gridFiles), button2, 1, 0, 1, 1);
                   g_signal_connect(G_OBJECT(button2), "clicked", 
                              G_CALLBACK(on_recent2_clicked), user_data);
                 }
                 g_free(recent_prev);
 }

 if(g_key_file_has_key(keyString, "history", "recent-file-2", NULL)) {
                 recent_prev = g_strdup_printf("%s", g_key_file_get_string(keyString, "history", 
                                          "recent-file-2", NULL));
                 if(strlen(recent_prev)>0) {
                   /* summaries */
                   count++;
                   content_prev = g_strdup_printf("%s", g_key_file_get_string(keyString, "history", 
                                          "recent-content-2", NULL));
                   button3=paving_new_button_block(3, recent_prev, content_prev );
                   g_free(content_prev);
                   gtk_grid_attach(GTK_GRID(gridFiles), button3, 2, 0, 1, 1);
                   g_signal_connect(G_OBJECT(button3), "clicked", 
                              G_CALLBACK(on_recent3_clicked), user_data);
                 }
                 g_free(recent_prev);
 }
 if(g_key_file_has_key(keyString, "history", "recent-file-3", NULL)) {
                 recent_prev = g_strdup_printf("%s", g_key_file_get_string(keyString, "history", 
                                          "recent-file-3", NULL));
                 if(strlen(recent_prev)>0) {
                   /* summaries */
                   count++;
                   content_prev = g_strdup_printf("%s", g_key_file_get_string(keyString, "history", 
                                          "recent-content-3", NULL));
                   button4=paving_new_button_block(4, recent_prev, content_prev );
                   g_free(content_prev);
                   gtk_grid_attach(GTK_GRID(gridFiles), button4, 0, 1, 1, 1);
                   g_signal_connect(G_OBJECT(button4), "clicked", 
                              G_CALLBACK(on_recent4_clicked), user_data);
                 }
                 g_free(recent_prev);
 }
 if(g_key_file_has_key(keyString, "history", "recent-file-4", NULL)) {
                 recent_prev = g_strdup_printf("%s", g_key_file_get_string(keyString, "history", 
                                          "recent-file-4", NULL));
                 if(strlen(recent_prev)>0) {
                   /* summaries */
                   count++;
                   content_prev = g_strdup_printf("%s", g_key_file_get_string(keyString, "history", 
                                          "recent-content-4", NULL));
                   button5=paving_new_button_block(5, recent_prev, content_prev );
                   g_free(content_prev);
                   gtk_grid_attach(GTK_GRID(gridFiles), button5, 1, 1, 1, 1);
                   g_signal_connect(G_OBJECT(button5), "clicked", 
                              G_CALLBACK(on_recent5_clicked), user_data);
                 }
                 g_free(recent_prev);
 }

 if(g_key_file_has_key(keyString, "history", "recent-file-5", NULL)) {
                 recent_prev = g_strdup_printf("%s", g_key_file_get_string(keyString, "history", 
                                          "recent-file-5", NULL));
                 if(strlen(recent_prev)>0) {
                   /* summaries */
                   count++;
                   content_prev = g_strdup_printf("%s", g_key_file_get_string(keyString, "history", 
                                          "recent-content-5", NULL));
                   button6=paving_new_button_block(6, recent_prev, content_prev );
                   g_free(content_prev);
                   gtk_grid_attach(GTK_GRID(gridFiles), button6, 2, 1, 1, 1);
                   g_signal_connect(G_OBJECT(button6), "clicked", 
                              G_CALLBACK(on_recent6_clicked), user_data);
                 }
                 g_free(recent_prev);
 }

 if(g_key_file_has_key(keyString, "history", "recent-file-6", NULL)) {
                 recent_prev = g_strdup_printf("%s", g_key_file_get_string(keyString, "history", 
                                          "recent-file-6", NULL));
                 if(strlen(recent_prev)>0) {
                   /* summaries */
                   count++;
                   content_prev = g_strdup_printf("%s", g_key_file_get_string(keyString, "history", 
                                          "recent-content-6", NULL));
                   button7=paving_new_button_block(7, recent_prev, content_prev );
                   g_free(content_prev);
                   gtk_grid_attach(GTK_GRID(gridFiles), button7, 0, 2, 1, 1);
                   g_signal_connect(G_OBJECT(button7), "clicked", 
                              G_CALLBACK(on_recent7_clicked), user_data);
                 }
                 g_free(recent_prev);
 }

 if(g_key_file_has_key(keyString, "history", "recent-file-7", NULL)) {
                 recent_prev = g_strdup_printf("%s", g_key_file_get_string(keyString, "history", 
                                          "recent-file-7", NULL));
                 if(strlen(recent_prev)>0) {
                   count++;
                   content_prev = g_strdup_printf("%s", g_key_file_get_string(keyString, "history", 
                                          "recent-content-7", NULL));
                   button8=paving_new_button_block(8, recent_prev, content_prev );
                   g_free(content_prev);
                   gtk_grid_attach(GTK_GRID(gridFiles), button8, 1, 2, 1, 1);
                   g_signal_connect(G_OBJECT(button8), "clicked", 
                              G_CALLBACK(on_recent8_clicked), user_data);
                 }
                 g_free(recent_prev);
 }

 if(g_key_file_has_key(keyString, "history", "recent-file-8", NULL)) {
                 recent_prev = g_strdup_printf("%s", g_key_file_get_string(keyString, "history", 
                                          "recent-file-8", NULL));
                 if(strlen(recent_prev)>0) {
                   count++;
                   content_prev = g_strdup_printf("%s", g_key_file_get_string(keyString, "history", 
                                          "recent-content-8", NULL));
                   button9=paving_new_button_block(9, recent_prev, content_prev );
                   g_free(content_prev);
                   gtk_grid_attach(GTK_GRID(gridFiles), button9, 2, 2, 1, 1);
                   g_signal_connect(G_OBJECT(button9), "clicked", 
                              G_CALLBACK(on_recent9_clicked), user_data);
                 }
                 g_free(recent_prev);
 }

 if(g_key_file_has_key(keyString, "history", "recent-file-9", NULL)) {
                 recent_prev = g_strdup_printf("%s", g_key_file_get_string(keyString, "history", 
                                          "recent-file-9", NULL));
                 if(strlen(recent_prev)>0) {
                   count++;
                   content_prev = g_strdup_printf("%s", g_key_file_get_string(keyString, "history", 
                                          "recent-content-9", NULL));
                   button10=paving_new_button_block(10, recent_prev, content_prev );
                   g_free(content_prev);
                   gtk_grid_attach(GTK_GRID(gridFiles), button10, 0, 3, 1, 1);
                   g_signal_connect(G_OBJECT(button10), "clicked", 
                              G_CALLBACK(on_recent10_clicked), user_data);
                 }
                 g_free(recent_prev);
  }
  if(count==1) {
      gtk_label_set_markup (GTK_LABEL (subTitle),
                  _("<i>Here is the last note you've used. The file's complete name and path and first words are displayed.\nClick on a note to reload it ...</i>"));
  }
  else {
      gtk_label_set_markup (GTK_LABEL (subTitle),
                  g_strdup_printf(_("<i>Here is the last %d notes you've used. The file's complete name and path and first words are displayed.\nClick on a note to reload it ...</i>"), count));

  }

  /* branch signals */
  g_signal_connect(G_OBJECT(buttonCancel), "clicked", 
        G_CALLBACK(on_cancel_clicked), user_data);


  GLADE_HOOKUP_OBJECT_NO_REF (pavingDialog, pavingDialog, "pavingDialog");
  GLADE_HOOKUP_OBJECT (pavingDialog, buttonCancel, "buttonCancel");
/* si tu laisses sur un objet non initialisé = plantage 
  GLADE_HOOKUP_OBJECT (pavingDialog, button1, "button1");
  GLADE_HOOKUP_OBJECT (pavingDialog, button2, "button2");
  GLADE_HOOKUP_OBJECT (pavingDialog, button3, "button3");
  GLADE_HOOKUP_OBJECT (pavingDialog, button4, "button4");
  GLADE_HOOKUP_OBJECT (pavingDialog, button5, "button5");
  GLADE_HOOKUP_OBJECT (pavingDialog, button6, "button6");
  GLADE_HOOKUP_OBJECT (pavingDialog, button7, "button7");
  GLADE_HOOKUP_OBJECT (pavingDialog, button8, "button8");
  GLADE_HOOKUP_OBJECT (pavingDialog, button9, "button9");
  GLADE_HOOKUP_OBJECT (pavingDialog, button10, "button10");*/
  gtk_widget_show_all(pavingDialog);
  return pavingDialog;
}
