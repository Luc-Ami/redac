#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>
/* translations */
#include <libintl.h>
#include <locale.h>
#include <unistd.h>
#include <stdio.h>
#include <glib.h>
#include <glib/gstdio.h> /* g_fopen, etc */
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <poppler.h>

/* from redac */

#include "support.h"
#include "misc.h"
#include "interface.h"
#include "callbacks.h"
#include "mttfiles.h"
#include "mttimport.h"
#include "audio.h"
#include "pdf.h"

/* pixmaps */


#include "./icons/highlight.xpm"

#include "./icons/pdf.xpm"

/*********************************
  setup default values for display
for paddings and removing inner border from entries, many thanks to them : https://github.com/jy2wong/luakit/commit/6dfffb296f562b26c1eb6020c94b1d3e0bde336b
anf for a clean demo for gtktextview tuning : https://gist.github.com/mariocesar/f051ce1dda4ef0041aed
************************************/
void set_up_view (GtkWidget *window1, APP_data *data_app)
{
  guint major, minor;
  GtkCssProvider* css_provider = gtk_css_provider_new();

  /* check Gtk 3.x version */
  major = gtk_get_major_version ();
  minor = gtk_get_minor_version ();

  const char css[] = 
"  #labelOther { color:black; background-image: none; background-color: #FFFFFF; }\n"
"  #labelPdfSketch  { color:black; background-image: none; background-color: #CBCBD0; }\n"
"  #labelFormat  { color:black; background-image: none; background-color: #CBCBD0; }\n"
"  #labelAlignment { color:black; background-image: none; background-color: #FFFFFF; }\n"
"  #labelHitsFrame { border-top-left-radius: 6px; border-bottom-left-radius: 6px;border-top-right-radius: 6px; border-bottom-right-radius: 6px; border-style: solid; border-width: 1px; padding: 0px; }\n"
"  #search_entry { border: none; padding: 4px; }\n"
"  #page_frame { border-top-left-radius: 6px; border-bottom-left-radius: 6px;border-top-right-radius: 6px; border-bottom-right-radius: 6px; border-style: solid; border-width: 1px; padding: 2px; }\n"
"  #page_title { font-weight:600; border: none; padding: 2px; }\n"
"  #page_entry { border: none; padding: 2px; }\n"
"  #page_label { font-weight:600; border: none; padding: 2px; }\n"
"  #gridDisplay { background-image: none; background-color: #FFFFFF; }\n"
"  #gridTitle {  background-image: none; background-color:#E9E9E6; color: black; }\n"
"  #recentGrid {  border-style: none; border-width: 5px;  border-radius: 5px;}\n"
"  #myButtonCancel{  font-family: DejaVu Sans;font-style: normal; border-radius: 3px; background-image: none;}\n"
"  #myButtonCancel:hover { background-color: red; }\n"

"  #myButton_label_title_1, #myButton_label_title_2, #myButton_label_title_3, #myButton_label_title_4, #myButton_label_title_5, #myButton_label_title_6, #myButton_label_title_7, #myButton_label_title_8, #myButton_label_title_9, #myButton_label_title_10 {  border-top-left-radius: 8px; border-top-right-radius: 8px; background-color: #6F7DC8; color: white; box-shadow: 2px 2px 4px #888888; }\n"
"  #myButton_label_content_1, #myButton_label_content_2, #myButton_label_content_3, #myButton_label_content_4, #myButton_label_content_5, #myButton_label_content_6, #myButton_label_content_7, #myButton_label_content_8, #myButton_label_content_9, #myButton_label_content_10 { border-radius: 8px; background-color: white; color: black; box-shadow: 2px 2px 4px #888888;}\n"
"  #myButton_1, #myButton_2, #myButton_3, #myButton_4, #myButton_5, #myButton_6, #myButton_7, #myButton_8, #myButton_9, #myButton_10{ color: black; font-family: DejaVu Sans; font-style: normal; border-radius: 8px;  border-image: none;  padding: 0px 0px;}\n"
"  #myButton_1:hover, #myButton_2:hover, #myButton_3:hover, #myButton_4:hover, #myButton_5:hover, #myButton_6:hover, #myButton_7:hover, #myButton_8:hover, #myButton_9:hover, #myButton_10:hover{  border-color: green; border-style: solid; border-image: none; border-top: none; border-left: none; border-right: none; border-width: 4px;}\n"
"  #myButton_1:focus, #myButton_2:focus, #myButton_3:focus, #myButton_4:focus, #myButton_5:focus, #myButton_6:focus, #myButton_7:focus, #myButton_8:focus, #myButton_9:focus, #myButton_10:focus{  border-style: solid; border-color:orange; border-image: none; border-top: none; border-left: none; border-right: none; border-width: 4px;}\n"
"  GtkMenu .menuitem {   padding: 4px 0px; }\n"
"  #view  { color: black; background-color: #FFFFE0;}\n"
"  #view:selected, #view:selected:focus { background-color: @selected_bg_color; color:@selected_fg_color; }\n"

"  Label#PDF_modified_label { border-top-left-radius: 6px; border-bottom-left-radius: 6px;border-top-right-radius: 6px; border-bottom-right-radius: 6px; border-style: solid; border-width: 6px; }\n";
  gtk_css_provider_load_from_data(css_provider,css,-1,NULL);
 /*----- css *****/
  GdkScreen* screen = gdk_screen_get_default ();
  gtk_style_context_add_provider_for_screen (screen,GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

/********************************
  check if we have a dark or 
  light theme
*********************************/
void check_up_theme (GtkWidget *window1, APP_data *data_app)
{
  GdkRGBA fg, bg;
  gdouble textAvg, bgAvg;
 
  GdkScreen* screen = gdk_screen_get_default ();
  GtkStyleContext* style_context = gtk_widget_get_style_context (window1);

 /* check if we have a drak or light theme idea from : 
https://lzone.de/blog/Detecting%20a%20Dark%20Theme%20in%20GTK  
*/
  gtk_style_context_get_color (style_context, GTK_STATE_FLAG_NORMAL, &fg);
 // gtk_style_context_get_background_color (style_context, GTK_STATE_FLAG_NORMAL, &bg); // deprecated
  gtk_style_context_lookup_color (style_context, "focus_color", &bg);
  
  textAvg = fg.red + fg.green + fg.blue;
  bgAvg = bg.red + bg.green + bg.blue;

// printf ("test couleurs avant =%.2f arrière =%.2f \n", textAvg, bgAvg);

  if (textAvg > bgAvg)  {
     data_app->fDarkTheme = TRUE;
     printf ("* Redac : Dark theme\n");
  }
  else {
     data_app->fDarkTheme = FALSE;
     printf ("* Redac : Light theme\n");
  }
}

/*****************************
 retrieve theme's selection
 color
****************************/
void get_theme_selection_color (GtkWidget *widget)
{
  GdkRGBA  color1, color2;

  GdkScreen* screen = gdk_screen_get_default ();
  GtkStyleContext* style_context = gtk_widget_get_style_context(widget);

  gtk_style_context_get (style_context, GTK_STATE_FLAG_SELECTED,
                         "background-color", &color1,
                         NULL);

  gtk_style_context_lookup_color (style_context, "focus_color", &color2);
  
}
/*****************************
 test Css configuration for
 a widget
******************************/
static gboolean widget_is_dark (GtkWidget *widget) 
{
  GdkRGBA fg, bg;
  gdouble textAvg;
 
  GdkScreen* screen = gdk_screen_get_default ();
  GtkStyleContext* style_context = gtk_widget_get_style_context (widget);

  /* check the TEXT color is the best way, since a background can be an image, not a text !*/
  gtk_style_context_get_color (style_context, GTK_STATE_FLAG_NORMAL, &fg);
  
  textAvg = fg.red + fg.green + fg.blue;

  if (textAvg > 1.5) { 
     return TRUE;
  }
  else {
     return FALSE;
  }
}


/*********************************

  prepare textview

*********************************/
GtkWidget *prepare_view ()
{
  GtkWidget *view;

  view = gtk_text_view_new ();
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW(view), GTK_WRAP_WORD);
  gtk_widget_set_name (view, "view");
  gtk_text_view_set_left_margin (GTK_TEXT_VIEW(view), 8);
  gtk_text_view_set_right_margin (GTK_TEXT_VIEW(view), 8);
  return view;
}

/**********************************
  prepare sketch background

**********************************/
void sketch_prepare (APP_data *data )
{
  cairo_t *cr;
  GdkRGBA color;

  cr = cairo_create (data->Sketchsurface);
  color.red   = g_key_file_get_double (data->keystring, "sketch", "paper.color.red", NULL);
  color.green = g_key_file_get_double (data->keystring, "sketch", "paper.color.green", NULL);
  color.blue  = g_key_file_get_double (data->keystring, "sketch", "paper.color.blue", NULL);
  color.alpha = 1;
  cairo_set_source_rgb (cr, color.red, color.green, color.blue);
  cairo_rectangle (cr, 0, 0, CROBAR_VIEW_MAX_WIDTH, CROBAR_VIEW_MAX_HEIGHT);
  cairo_fill (cr);
  cairo_destroy (cr);
  gtk_widget_queue_draw (data->SketchDrawable);
}

/*********************************
  prepare sketch drawable

*********************************/
GtkWidget *sketch_prepare_drawable ()
{
  GtkWidget *crCrobar;

  crCrobar=gtk_drawing_area_new ();
  gtk_widget_set_app_paintable (crCrobar, TRUE);
  gtk_widget_show (crCrobar);
  gtk_widget_set_size_request (crCrobar, CROBAR_VIEW_MAX_WIDTH, CROBAR_VIEW_MAX_HEIGHT);
  gtk_widget_set_hexpand (crCrobar, TRUE);
  gtk_widget_set_vexpand (crCrobar, TRUE);
  /* mandatoty : add new events management to gtk_drawing_area ! */
      gtk_widget_set_events (crCrobar, gtk_widget_get_events (crCrobar)
      | GDK_BUTTON_PRESS_MASK
      | GDK_BUTTON_RELEASE_MASK
      | GDK_POINTER_MOTION_MASK
      | GDK_POINTER_MOTION_HINT_MASK);
  return crCrobar;
}


/*******************************

  right click on PDF surface
  Pop-Up menu
********************************/

GtkWidget *create_menu_PDF (GtkWidget *win, APP_data *data_app)
{
  GtkWidget *menu1PDF; 
  GtkWidget *menu1PDFEditAnnot;
  GtkWidget *menu1PDFColorAnnot;
  GtkWidget *menu1PDFRemoveAnnot;
  GtkWidget *menuCancelPDF;
  GtkWidget *separator1;

  menu1PDF = gtk_menu_new (); 

  menu1PDFEditAnnot = gtk_menu_item_new_with_mnemonic (_("_Edit annotation ... "));
  gtk_widget_show (menu1PDFEditAnnot);
  gtk_container_add (GTK_CONTAINER (menu1PDF), menu1PDFEditAnnot);

  menu1PDFColorAnnot = gtk_menu_item_new_with_mnemonic (_("Annotation _color ... "));
  gtk_widget_show (menu1PDFColorAnnot);
  gtk_container_add (GTK_CONTAINER (menu1PDF), menu1PDFColorAnnot);

  menu1PDFRemoveAnnot = gtk_menu_item_new_with_mnemonic (_("_Remove annotation "));
  gtk_widget_show (menu1PDFRemoveAnnot);
  gtk_container_add (GTK_CONTAINER (menu1PDF), menu1PDFRemoveAnnot);

  separator1 = gtk_separator_menu_item_new ();
  gtk_widget_show (separator1);
  gtk_container_add (GTK_CONTAINER (menu1PDF), separator1);
  gtk_widget_set_sensitive (separator1, FALSE);

  menuCancelPDF = gtk_menu_item_new_with_mnemonic (_("C_ancel"));
  gtk_widget_show (menuCancelPDF);
  gtk_container_add (GTK_CONTAINER (menu1PDF), menuCancelPDF);

  if (data_app->current_annot == NULL) {
    /* we deactivate menus since there isn't any annotation on current page */
    gtk_widget_set_sensitive (menu1PDFEditAnnot, FALSE);
    gtk_widget_set_sensitive (menu1PDFColorAnnot, FALSE);
    gtk_widget_set_sensitive (menu1PDFRemoveAnnot, FALSE);
  }
  else {/* we check if it's a text annot */
    if( poppler_annot_get_annot_type (data_app->current_annot) != POPPLER_ANNOT_TEXT && 
            poppler_annot_get_annot_type (data_app->current_annot)  !=POPPLER_ANNOT_FREE_TEXT) {
                           gtk_widget_set_sensitive (menu1PDFEditAnnot, FALSE);
    }
  }
  g_signal_connect ((gpointer) menu1PDFEditAnnot, "activate",
                    G_CALLBACK (on_menuPDFEditAnnot),
                    data_app);
  g_signal_connect ((gpointer) menu1PDFColorAnnot, "activate",
                    G_CALLBACK (on_menuPDFColorAnnot),
                    data_app);
  g_signal_connect ((gpointer) menu1PDFRemoveAnnot, "activate",
                    G_CALLBACK (on_menuPDFRemoveAnnot),
                    data_app);


  GLADE_HOOKUP_OBJECT_NO_REF (menu1PDF, menu1PDF, "menu1PDF");
  GLADE_HOOKUP_OBJECT (menu1PDF, menu1PDFEditAnnot, "menu1PDFEditAnnot");
  GLADE_HOOKUP_OBJECT (menu1PDF, menu1PDFColorAnnot, "menu1PDFColorAnnot");
  GLADE_HOOKUP_OBJECT (menu1PDF, menu1PDFRemoveAnnot, "menu1PDFRemoveAnnot");

  return menu1PDF;
}

/*******************************

  right click on Sketch surface
  Pop-Up menu
********************************/

GtkWidget *create_menu_sketch (GtkWidget *win, APP_data *data_app)
{
  GtkWidget *menu1Sketch; 
  GtkWidget *menuPasteSketch;
  GtkWidget *menuCenteredPasteSketch;
  GtkWidget *menuCancelSketch;
  GtkWidget *separator1;

  menu1Sketch = gtk_menu_new (); 

  menuPasteSketch = gtk_menu_item_new_with_mnemonic (_("_Paste image in place "));
  gtk_widget_show (menuPasteSketch);
  gtk_container_add (GTK_CONTAINER (menu1Sketch), menuPasteSketch);

  menuCenteredPasteSketch = gtk_menu_item_new_with_mnemonic (_("Paste and _center image "));
  gtk_widget_show (menuCenteredPasteSketch);
  gtk_container_add (GTK_CONTAINER (menu1Sketch), menuCenteredPasteSketch);

  separator1 = gtk_separator_menu_item_new ();
  gtk_widget_show (separator1);
  gtk_container_add (GTK_CONTAINER (menu1Sketch), separator1);
  gtk_widget_set_sensitive (separator1, FALSE);

  menuCancelSketch = gtk_menu_item_new_with_mnemonic (_("C_ancel"));
  gtk_widget_show (menuCancelSketch);
  gtk_container_add (GTK_CONTAINER (menu1Sketch), menuCancelSketch);

  g_signal_connect ((gpointer) menuPasteSketch, "activate",
                    G_CALLBACK (on_menuPasteSketch),
                    data_app);
  g_signal_connect ((gpointer) menuCenteredPasteSketch, "activate",
                    G_CALLBACK (on_menuCenteredPasteSketch),
                    data_app);


  GLADE_HOOKUP_OBJECT_NO_REF (menu1Sketch, menu1Sketch, "menu1Sketch");
  GLADE_HOOKUP_OBJECT (menu1Sketch, menuPasteSketch, "menuPasteSketch");
  GLADE_HOOKUP_OBJECT (menu1Sketch, menuCenteredPasteSketch, "menuCenteredPasteSketch");


  return menu1Sketch;
}



/**********************************
   set up the toolbar
***********************************/
GtkWidget *main_wp_toolbar (GtkWidget *window, APP_data *data_app)
{
  GdkPixbuf *ico;
  GtkWidget *toolbar;
  /* format tools */
  GtkToolItem *button_bold;
  GtkWidget *icon_bold;
  GtkToolItem *button_italic;
  GtkWidget *icon_italic;
  GtkToolItem *button_underline;
  GtkWidget *icon_underline;
  GtkToolItem *button_superscript;
  GtkToolItem *button_subscript;
  GtkToolItem *button_strikethrough;
  GtkToolItem *button_highlight;
  GtkToolItem *button_quotation;
  GtkWidget *icon_superscript, *icon_subscript;
  GtkWidget *icon_strike, *icon_highlight;
  GtkWidget *icon_quotation;
  /* alignment tools */
  GtkWidget *icon_left_format, *icon_center_format;
  GtkWidget *icon_right_format, *icon_fill_format;
  GtkToolItem *pRadioButtonLeft;
  GtkToolItem *pRadioButtonCenter;
  GtkToolItem *pRadioButtonRight;
  GtkToolItem *pRadioButtonFill;

  GtkToolItem *button_clear_format;
  GtkWidget *icon_clear_format;
  GtkToolItem *button_undo;
  GtkWidget *icon_undo;
  GtkToolItem *standardSeperator;
  /* PDF and image sections */
  GtkToolItem *pRadioButtonTextSelect;
  GtkToolItem *pRadioButtonPictureSelect;
  GtkToolItem *pRadioButtonHiglightSelect;
  GtkToolItem *pRadioButtonHighlightRect;   
  GtkToolItem *pRadioButtonHiAnnotSelect;
  GtkWidget *icon_text_select, *icon_picture_select;
  GtkWidget *icon_highlight_select, *icon_text_annot, *icon_highlight_rectangle;
  GtkWidget *icon_pencil;
  GtkToolItem *button_pencil;
  GtkToolItem *color_button_item;
  GtkWidget *color_button;
  GtkAccelGroup *accel_group;
  gboolean fIsDark = FALSE;
  /* audio section */
  GtkToolItem *audioSeperator;
  GtkToolItem *pRadioButtonPlayPauseAudio;
  GtkToolItem *pRadioButtonRewindAudio;
  GtkToolItem *pRadioButtonGotoAudio;
  GtkToolItem *pRadioButtonGoJumpAudio;
  GtkToolItem *pRadioButtonCopyPosAudio;
  GtkWidget *AudioDisplayCounter, *iconButtonPlayAudio, *iconButtonCopyPos;
  GtkWidget *iconButtonPauseAudio, *iconButtonHomeAudio;
  GtkWidget *iconButtonGotoAudio, *iconButtonGoJumpAudio;

  fIsDark = data_app->fDarkTheme;
  toolbar = GTK_WIDGET(gtk_builder_get_object (data_app->builder, "toolbar"));

  /* toolbar toggle buttons bold */
  icon_bold = gtk_image_new_from_icon_name ("format-text-bold-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
  icon_italic = gtk_image_new_from_icon_name ("format-text-italic-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
  icon_underline = gtk_image_new_from_icon_name ("format-text-underline-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
  icon_clear_format = gtk_image_new_from_icon_name ("edit-clear-all-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);

  icon_undo = gtk_image_new_from_icon_name ("edit-undo-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);

  icon_superscript  = gtk_image_new_from_icon_name ("view-sort-descending-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR); 
    
  icon_subscript  = gtk_image_new_from_icon_name ("view-sort-ascending-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);   

  icon_strike = gtk_image_new_from_icon_name ("format-text-strikethrough-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);

  ico = gdk_pixbuf_new_from_xpm_data ((const char **)highlight_xpm);
  icon_highlight = gtk_image_new_from_pixbuf (ico);
  g_object_unref(ico); 

  icon_quotation = gtk_image_new_from_icon_name ("format-indent-more-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);
  
  /* note editor buttons : formatting buttons */

  button_bold = gtk_toggle_tool_button_new ();
  gtk_toolbar_insert  (GTK_TOOLBAR(toolbar), button_bold, -1);  
  gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON(button_bold),icon_bold);
  gtk_widget_set_tooltip_text (GTK_WIDGET(button_bold), _("Toggle to/from bold either \nfor current selection\nor next typing."));

  button_italic = gtk_toggle_tool_button_new ();
  gtk_toolbar_insert (GTK_TOOLBAR(toolbar), button_italic, -1);  

  gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON(button_italic),icon_italic);
  gtk_widget_set_tooltip_text (GTK_WIDGET(button_italic), _("Toggle to/from italic either \nfor current selection\nor next typing."));
  button_underline = gtk_toggle_tool_button_new ();
  gtk_toolbar_insert (GTK_TOOLBAR(toolbar), button_underline, -1); 

  gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON(button_underline),icon_underline);
  gtk_widget_set_tooltip_text (GTK_WIDGET(button_underline), _("Toggle to/from underline either \nfor current selection\nor next typing."));
  /* other formattings */
   button_superscript = gtk_toggle_tool_button_new ();
   gtk_toolbar_insert (GTK_TOOLBAR(toolbar), button_superscript, -1); 
   gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON(button_superscript),icon_superscript);
 gtk_widget_set_tooltip_text (GTK_WIDGET(button_superscript), _("Toggle to/from superscript either \nfor current selection\nor next typing."));
   button_subscript = gtk_toggle_tool_button_new ();
   gtk_toolbar_insert (GTK_TOOLBAR(toolbar), button_subscript, -1);    
   gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON(button_subscript),icon_subscript);
 gtk_widget_set_tooltip_text (GTK_WIDGET(button_subscript), _("Toggle to/from subscript either \nfor current selection\nor next typing."));
   button_strikethrough = gtk_toggle_tool_button_new ();
   gtk_toolbar_insert (GTK_TOOLBAR(toolbar), button_strikethrough, -1); 
   gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON(button_strikethrough),icon_strike);
 gtk_widget_set_tooltip_text (GTK_WIDGET(button_strikethrough), _("Toggle to/from strikethrough \nfor current selection\nor next typing."));
   button_highlight = gtk_toggle_tool_button_new ();
   gtk_toolbar_insert (GTK_TOOLBAR(toolbar), button_highlight, -1); 
   gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON(button_highlight),icon_highlight);
 gtk_widget_set_tooltip_text (GTK_WIDGET(button_highlight), _("Apply highlighting \nfor current selection\nor next typing."));
   button_quotation = gtk_toggle_tool_button_new ();
   gtk_toolbar_insert (GTK_TOOLBAR(toolbar), button_quotation, -1); 
   gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON(button_quotation),icon_quotation);
gtk_widget_set_tooltip_text (GTK_WIDGET(button_quotation), _("Toggle to/from quotation style either \nfor current selection\nor next typing."));
  gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON(button_bold), FALSE);
  gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON(button_italic), FALSE);
  gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON(button_underline), FALSE);
 
  /* radiobuttons : thanks to Pascal developers 
   here: https://fr.wikibooks.org/wiki/Programmation_GTK2_en_Pascal/GtkRadioToolButton*/
  pRadioButtonLeft = gtk_radio_tool_button_new (NULL);

  icon_left_format = gtk_image_new_from_icon_name ("format-justify-left-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
  icon_center_format = gtk_image_new_from_icon_name ("format-justify-center-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
  icon_right_format = gtk_image_new_from_icon_name ("format-justify-right-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
  icon_fill_format = gtk_image_new_from_icon_name ("format-justify-fill-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);

  GSList *group = NULL; 

  gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON(pRadioButtonLeft), icon_left_format);
  gtk_toolbar_insert (GTK_TOOLBAR(toolbar), pRadioButtonLeft, -1);
  gtk_widget_set_tooltip_text (GTK_WIDGET(pRadioButtonLeft), _("Align to left \nthe whole current paragraph.")); 
  gtk_radio_tool_button_set_group (GTK_RADIO_TOOL_BUTTON(pRadioButtonLeft), group);
  group = gtk_radio_tool_button_get_group (GTK_RADIO_TOOL_BUTTON(pRadioButtonLeft));

  pRadioButtonCenter = gtk_radio_tool_button_new(group);
  gtk_toolbar_insert (GTK_TOOLBAR(toolbar), pRadioButtonCenter, -1);
  gtk_widget_set_tooltip_text (GTK_WIDGET(pRadioButtonCenter), _("Align to center \nthe whole current paragraph.")); 
  gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON(pRadioButtonCenter), icon_center_format);
  gtk_radio_tool_button_set_group (GTK_RADIO_TOOL_BUTTON(pRadioButtonCenter), group);
  group = gtk_radio_tool_button_get_group (GTK_RADIO_TOOL_BUTTON(pRadioButtonCenter));

  pRadioButtonRight = gtk_radio_tool_button_new(group);
  gtk_toolbar_insert (GTK_TOOLBAR(toolbar), pRadioButtonRight, -1);
  gtk_widget_set_tooltip_text (GTK_WIDGET(pRadioButtonRight), _("Align to right \nthe whole current paragraph.")); 
  gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON(pRadioButtonRight), icon_right_format);
  gtk_radio_tool_button_set_group (GTK_RADIO_TOOL_BUTTON(pRadioButtonRight), group);
  group = gtk_radio_tool_button_get_group (GTK_RADIO_TOOL_BUTTON(pRadioButtonRight));

  pRadioButtonFill = gtk_radio_tool_button_new(group);
  gtk_toolbar_insert (GTK_TOOLBAR(toolbar), pRadioButtonFill, -1);
  gtk_widget_set_tooltip_text (GTK_WIDGET(pRadioButtonFill), _("Justify left AND right \nthe whole current paragraph.")); 
  gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON(pRadioButtonFill), icon_fill_format);
  gtk_radio_tool_button_set_group (GTK_RADIO_TOOL_BUTTON(pRadioButtonFill), group);
  group = gtk_radio_tool_button_get_group (GTK_RADIO_TOOL_BUTTON(pRadioButtonFill));

  gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON(pRadioButtonLeft), TRUE);

 
  /* standard button */

  button_clear_format = gtk_tool_button_new (icon_clear_format, NULL);
  gtk_toolbar_insert (GTK_TOOLBAR(toolbar), button_clear_format, -1);  
  gtk_widget_set_tooltip_text (GTK_WIDGET(button_clear_format), _("Remove all character \nformattings from current selection.")); 
  /* undo button */

  button_undo = gtk_tool_button_new (icon_undo, NULL);
  gtk_toolbar_insert (GTK_TOOLBAR(toolbar), button_undo, -1);   
  gtk_widget_set_tooltip_text (GTK_WIDGET(button_undo), _("Undo last operation."));
  gtk_widget_set_sensitive (GTK_WIDGET(button_undo), FALSE);

  standardSeperator = gtk_separator_tool_item_new ();
  gtk_separator_tool_item_set_draw (GTK_SEPARATOR_TOOL_ITEM(standardSeperator), TRUE);
  gtk_toolbar_insert (GTK_TOOLBAR(toolbar), standardSeperator, -1);

  /* clipboard mode radiobuttons */
  pRadioButtonTextSelect = gtk_radio_tool_button_new (NULL);

  icon_text_select = gtk_image_new_from_icon_name ("edit-select-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);

/*
  if(data_app->fDarkTheme)
      ico = gdk_pixbuf_new_from_xpm_data((const char **)camera_light_xpm);
  else
      ico = gdk_pixbuf_new_from_xpm_data((const char **)camera_xpm);
  icon_picture_select  =gtk_image_new_from_pixbuf(ico);
  g_object_unref(ico); 
*/
  icon_picture_select = gtk_image_new_from_icon_name ("camera-photo-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);

  icon_highlight_select = gtk_image_new_from_icon_name ("starred-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);
  // edit-select-all-symbolic
  icon_highlight_rectangle = gtk_image_new_from_icon_name ("insert-object-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);

  icon_text_annot = gtk_image_new_from_icon_name ("media-view-subtitles-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);


  GSList *group_clip = NULL; 
  gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON(pRadioButtonTextSelect), icon_text_select);
  gtk_toolbar_insert  (GTK_TOOLBAR(toolbar), pRadioButtonTextSelect, -1);
  gtk_widget_set_tooltip_text  (GTK_WIDGET(pRadioButtonTextSelect),
                   _("Copy selected area inside PDF files\nto clipboard as pure text.")); 
  gtk_radio_tool_button_set_group (GTK_RADIO_TOOL_BUTTON(pRadioButtonTextSelect), group_clip);
  group_clip = gtk_radio_tool_button_get_group (GTK_RADIO_TOOL_BUTTON(pRadioButtonTextSelect));

  pRadioButtonPictureSelect = gtk_radio_tool_button_new(group_clip);
  gtk_toolbar_insert (GTK_TOOLBAR(toolbar), pRadioButtonPictureSelect, -1);
  gtk_widget_set_tooltip_text (GTK_WIDGET(pRadioButtonPictureSelect),
                 _("Copy selected area inside PDF files or sketches \nto clipboard as pure image.")); 
  gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON(pRadioButtonPictureSelect), icon_picture_select);
  gtk_radio_tool_button_set_group (GTK_RADIO_TOOL_BUTTON(pRadioButtonPictureSelect), group_clip);
  group_clip = gtk_radio_tool_button_get_group (GTK_RADIO_TOOL_BUTTON(pRadioButtonPictureSelect));

  gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON(pRadioButtonTextSelect), TRUE);

  pRadioButtonHiglightSelect = gtk_radio_tool_button_new(group_clip);
  gtk_toolbar_insert (GTK_TOOLBAR(toolbar), pRadioButtonHiglightSelect, -1);
  gtk_widget_set_tooltip_text (GTK_WIDGET(pRadioButtonHiglightSelect),
                   _("Highlight selected text inside PDF document. \nPlease don't forget to save your PDF document !"));
  gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON(pRadioButtonHiglightSelect), icon_highlight_select);
  gtk_radio_tool_button_set_group (GTK_RADIO_TOOL_BUTTON(pRadioButtonHiglightSelect), group_clip);
  group_clip = gtk_radio_tool_button_get_group (GTK_RADIO_TOOL_BUTTON(pRadioButtonHiglightSelect));


  pRadioButtonHighlightRect = gtk_radio_tool_button_new(group_clip);
  gtk_toolbar_insert (GTK_TOOLBAR(toolbar), pRadioButtonHighlightRect, -1);
  gtk_widget_set_tooltip_text (GTK_WIDGET(pRadioButtonHighlightRect),
                      _("Highlight selected area inside PDF document. \nPlease don't forget to save your PDF document !"));
  gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON(pRadioButtonHighlightRect), icon_highlight_rectangle);
  gtk_radio_tool_button_set_group (GTK_RADIO_TOOL_BUTTON(pRadioButtonHighlightRect), group_clip);
  group_clip = gtk_radio_tool_button_get_group (GTK_RADIO_TOOL_BUTTON(pRadioButtonHighlightRect));    

  pRadioButtonHiAnnotSelect = gtk_radio_tool_button_new(group_clip);
  gtk_toolbar_insert (GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(pRadioButtonHiAnnotSelect), -1);
  gtk_widget_set_tooltip_text (GTK_WIDGET(pRadioButtonHiAnnotSelect), 
                _("Set-up text annotation inside \nselected area on PDF or sketch document. \nPlease don't forget to save your PDF/sketch document to keep \nyour changes !"));
  gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON(pRadioButtonHiAnnotSelect), icon_text_annot);
  gtk_radio_tool_button_set_group (GTK_RADIO_TOOL_BUTTON(pRadioButtonHiAnnotSelect), group_clip);
  group_clip = gtk_radio_tool_button_get_group (GTK_RADIO_TOOL_BUTTON(pRadioButtonHiAnnotSelect));

  /* sketch tools */

  icon_pencil = gtk_image_new_from_icon_name ("input-tablet-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);  
  
  button_pencil = gtk_radio_tool_button_new(group_clip);
  gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON(button_pencil),icon_pencil); 
  gtk_widget_set_tooltip_text (GTK_WIDGET(button_pencil), _("Freehand drawing tool"));
  gtk_toolbar_insert (GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(button_pencil), -1);
  gtk_radio_tool_button_set_group (GTK_RADIO_TOOL_BUTTON(button_pencil), group_clip);
  group_clip = gtk_radio_tool_button_get_group (GTK_RADIO_TOOL_BUTTON(button_pencil));
  
  /* color button */
  color_button_item = gtk_tool_item_new();
  gtk_toolbar_insert (GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(color_button_item), -1);
  gtk_widget_set_tooltip_text (GTK_WIDGET(color_button_item),
                              _("Click here to choose color for :\n- highlighting of PDF areas.\n-annotation color for PDF and sketches documents.\n-pencil color for sketches."));
  color_button = gtk_color_button_new (); 
  gtk_container_add (GTK_CONTAINER(color_button_item), color_button);
  
  /* audio buttons */
  audioSeperator=gtk_separator_tool_item_new ();
  gtk_separator_tool_item_set_draw (GTK_SEPARATOR_TOOL_ITEM(audioSeperator), TRUE);
  gtk_toolbar_insert (GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(audioSeperator), -1); 
 /* playing speed combobox */
  GtkToolItem *audioPlaySpeedContainer = gtk_tool_item_new ();
  GtkWidget *audioPlaySpeed = gtk_combo_box_text_new ();
  gtk_widget_set_tooltip_text (GTK_WIDGET(audioPlaySpeed),
                             _("Choose here the playing speed. 1 for a normal speed,\nover 1 for a faster speed,\nunder 1 for a slower speed."));
 // gtk_widget_show (audioPlaySpeed);
  gtk_container_add (GTK_CONTAINER(audioPlaySpeedContainer), audioPlaySpeed);
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (audioPlaySpeed), _("×1.5"));
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (audioPlaySpeed), _("×1.2"));
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (audioPlaySpeed), _("×1.0"));
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (audioPlaySpeed), _("×0.8"));
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (audioPlaySpeed), _("×0.5"));
  gtk_combo_box_set_active (GTK_COMBO_BOX (audioPlaySpeed), 2);
  gtk_widget_set_sensitive (GTK_WIDGET (audioPlaySpeed), FALSE);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), audioPlaySpeedContainer, -1);  

  iconButtonCopyPos = gtk_image_new_from_icon_name ("document-open-recent-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);
  iconButtonPlayAudio = gtk_image_new_from_icon_name ("media-playback-start-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);
  iconButtonPauseAudio = gtk_image_new_from_icon_name ("media-playback-pause-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_widget_show (iconButtonPauseAudio);
  iconButtonHomeAudio   = gtk_image_new_from_icon_name ("media-skip-backward-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);
  iconButtonGotoAudio   = gtk_image_new_from_icon_name ("media-skip-forward-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);
  iconButtonGoJumpAudio = gtk_image_new_from_icon_name ("find-location-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);
  pRadioButtonPlayPauseAudio = gtk_tool_button_new (iconButtonPlayAudio,NULL);
  gtk_toolbar_insert (GTK_TOOLBAR(toolbar), pRadioButtonPlayPauseAudio, -1);  
  //gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON(pRadioButtonPlayPauseAudio),iconButtonPlayAudio);
  gtk_widget_set_tooltip_text (GTK_WIDGET(pRadioButtonPlayPauseAudio), _("Play or pause-rewind current audio file.\nRewind's length is defined in 'audio' settings."));
  gtk_widget_set_sensitive (GTK_WIDGET(pRadioButtonPlayPauseAudio), FALSE);
  /* counter */
  GtkToolItem *audio_position = gtk_tool_item_new();
  gtk_widget_set_tooltip_text (GTK_WIDGET(audio_position), _("Position within current audio file"));
  GtkWidget *audio_position_label = gtk_label_new("<tt><big>--:--:--</big></tt>");/* tt for Monospace font family */
  gtk_label_set_use_markup (GTK_LABEL (audio_position_label), TRUE);
  gtk_container_add (GTK_CONTAINER(audio_position), audio_position_label);
  gtk_toolbar_insert (GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(audio_position), -1);
  gtk_widget_set_sensitive (GTK_WIDGET(audio_position), FALSE);
  /* total audio file duration */
  GtkToolItem *audio_total = gtk_tool_item_new();
  gtk_widget_set_tooltip_text (GTK_WIDGET(audio_total), _("Total duration of current audio file"));
  GtkWidget *audio_total_label = gtk_label_new ("<tt><small>/--:--:--</small></tt>");
  gtk_label_set_use_markup (GTK_LABEL (audio_total_label), TRUE);
  gtk_container_add (GTK_CONTAINER(audio_total), audio_total_label);
  gtk_toolbar_insert (GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(audio_total), -1);
  gtk_widget_set_sensitive (GTK_WIDGET(audio_total), FALSE);
  /* goto previous mark */
  pRadioButtonRewindAudio = gtk_tool_button_new (iconButtonHomeAudio,NULL);
  gtk_toolbar_insert (GTK_TOOLBAR(toolbar), pRadioButtonRewindAudio, -1);  
  gtk_widget_set_tooltip_text (GTK_WIDGET(pRadioButtonRewindAudio), _("Jump backward in current audio file.\nJump's length is defined in 'audio' settings."));
  gtk_widget_set_sensitive (GTK_WIDGET(pRadioButtonRewindAudio), FALSE);
  /* goto next mark */
  pRadioButtonGotoAudio = gtk_tool_button_new (iconButtonGotoAudio,NULL);
  gtk_toolbar_insert (GTK_TOOLBAR(toolbar), pRadioButtonGotoAudio, -1);  
  gtk_widget_set_tooltip_text (GTK_WIDGET(pRadioButtonGotoAudio), _("Jump forward in current audio file.\nJump's length is defined in 'audio' settings."));
  gtk_widget_set_sensitive (GTK_WIDGET(pRadioButtonGotoAudio), FALSE);
  /* jump to ... */
  pRadioButtonGoJumpAudio = gtk_tool_button_new (iconButtonGoJumpAudio, NULL);
  gtk_toolbar_insert (GTK_TOOLBAR(toolbar), pRadioButtonGoJumpAudio, -1);  
  gtk_widget_set_tooltip_text (GTK_WIDGET(pRadioButtonGoJumpAudio), _("Jump to a specifed position within current audio file."));
  gtk_widget_set_sensitive (GTK_WIDGET(pRadioButtonGoJumpAudio), FALSE); 
  /* copy current position */
  pRadioButtonCopyPosAudio = gtk_tool_button_new (iconButtonCopyPos, NULL);
  gtk_toolbar_insert (GTK_TOOLBAR(toolbar), pRadioButtonCopyPosAudio, -1);  
  gtk_widget_set_tooltip_text (GTK_WIDGET(pRadioButtonCopyPosAudio), _("Copy current position in audio file to editor"));
  gtk_widget_set_sensitive (GTK_WIDGET(pRadioButtonCopyPosAudio), FALSE); 

  /* since we start with ediror, we hide some widgets */
  
  gtk_widget_show_all (toolbar);
  /* set cSS states */
  toggle_css_value (GTK_WIDGET(button_bold), FALSE);
  toggle_css_value (GTK_WIDGET(button_italic), FALSE);
  toggle_css_value (GTK_WIDGET(button_underline), FALSE);
  /* accelerators - it's complex for GtkToggleTool buttons - we must get a pointer on the child inside the toggle 

   see : https://stackoverflow.com/questions/19657017/handling-clicked-accelerator-on-a-gtktoggletoolbutton-gtkmm 
  */
  accel_group = gtk_accel_group_new ();
  gtk_widget_add_accelerator (gtk_bin_get_child(GTK_BIN(button_bold)), "clicked", accel_group,
                              GDK_KEY_b, (GdkModifierType) GDK_CONTROL_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (gtk_bin_get_child(GTK_BIN(button_italic)), "clicked", accel_group,
                              GDK_KEY_i, (GdkModifierType) GDK_CONTROL_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (gtk_bin_get_child(GTK_BIN(button_underline)), "clicked", accel_group,
                              GDK_KEY_u, (GdkModifierType) GDK_CONTROL_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (gtk_bin_get_child(GTK_BIN(button_superscript)), "clicked", accel_group,
                              GDK_KEY_dead_circumflex, (GdkModifierType) GDK_CONTROL_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (gtk_bin_get_child(GTK_BIN(button_subscript)), "clicked", accel_group,
                              GDK_KEY_underscore, (GdkModifierType) GDK_CONTROL_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (gtk_bin_get_child(GTK_BIN(button_strikethrough)), "clicked", accel_group,
                              GDK_KEY_k, (GdkModifierType) GDK_CONTROL_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (gtk_bin_get_child(GTK_BIN(button_highlight)), "clicked", accel_group,
                              GDK_KEY_h, (GdkModifierType) GDK_CONTROL_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (gtk_bin_get_child(GTK_BIN(button_quotation)), "clicked", accel_group,
                              GDK_KEY_quotedbl, (GdkModifierType) GDK_CONTROL_MASK,
                              GTK_ACCEL_VISIBLE);

  gtk_widget_add_accelerator (gtk_bin_get_child(GTK_BIN(pRadioButtonLeft)), "clicked", accel_group,
                              GDK_KEY_parenleft, (GdkModifierType) GDK_CONTROL_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (gtk_bin_get_child(GTK_BIN(pRadioButtonRight)), "clicked", accel_group,
                              GDK_KEY_parenright, (GdkModifierType) GDK_CONTROL_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (gtk_bin_get_child(GTK_BIN(pRadioButtonFill)), "clicked", accel_group,
                              GDK_KEY_equal, (GdkModifierType) GDK_CONTROL_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (gtk_bin_get_child(GTK_BIN(pRadioButtonCenter)), "clicked", accel_group,
                              GDK_KEY_colon, (GdkModifierType) GDK_CONTROL_MASK,
                              GTK_ACCEL_VISIBLE);
  /* accelerator for Main menu : NO, it should be a keybind in callbacks.c */
  gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);

  /* format callbacks */
  g_signal_connect (G_OBJECT(button_bold), "toggled", 
        G_CALLBACK(on_bold_clicked), data_app);
  g_signal_connect (G_OBJECT(button_italic), "toggled", 
        G_CALLBACK(on_italic_clicked), data_app);
  g_signal_connect (G_OBJECT(button_underline), "toggled", 
        G_CALLBACK(on_underline_clicked), data_app);
  /* other formatings */
  g_signal_connect (G_OBJECT(button_superscript), "toggled", 
        G_CALLBACK(on_superscript_clicked), data_app);
  g_signal_connect (G_OBJECT(button_subscript), "toggled", 
        G_CALLBACK(on_subscript_clicked), data_app);
  g_signal_connect (G_OBJECT(button_strikethrough), "toggled", 
        G_CALLBACK(on_strikethrough_clicked), data_app);
  g_signal_connect (G_OBJECT(button_highlight), "toggled", 
        G_CALLBACK(on_highlight_clicked), data_app);
  g_signal_connect (G_OBJECT(button_quotation), "toggled", 
        G_CALLBACK(on_quotation_clicked), data_app);

  g_signal_connect (G_OBJECT(button_clear_format), "clicked", 
        G_CALLBACK(on_clear_format_clicked), data_app);
  g_signal_connect (G_OBJECT(button_undo), "clicked", 
        G_CALLBACK(on_undo_clicked), data_app);
  g_signal_connect (G_OBJECT(pRadioButtonPlayPauseAudio), "clicked", 
        G_CALLBACK(on_play_pause_clicked), data_app);
  g_signal_connect (G_OBJECT(pRadioButtonRewindAudio), "clicked", 
        G_CALLBACK(on_jump_prev_clicked), data_app);
  g_signal_connect (G_OBJECT(pRadioButtonGotoAudio), "clicked", 
        G_CALLBACK(on_jump_next_clicked), data_app);
  g_signal_connect (G_OBJECT(pRadioButtonGoJumpAudio), "clicked", 
        G_CALLBACK(on_go_jump_clicked), data_app);

  g_signal_connect (G_OBJECT(pRadioButtonCopyPosAudio), "clicked", 
        G_CALLBACK(on_copy_pos_audio_clicked), data_app);

  g_signal_connect ((gpointer) audioPlaySpeed, "changed",
                    G_CALLBACK (on_audioPlaySpeed_changed),
                    data_app);

  /* for toogle radio buttons we must pass GSlist as parameter in order to get the Active button in group */
  g_signal_connect (G_OBJECT(pRadioButtonLeft), "toggled", 
        G_CALLBACK(on_button_alignment_toggled), data_app);
  g_signal_connect (G_OBJECT(pRadioButtonCenter), "toggled", 
        G_CALLBACK(on_button_alignment_toggled), data_app);
  g_signal_connect (G_OBJECT(pRadioButtonRight), "toggled", 
        G_CALLBACK(on_button_alignment_toggled), data_app);
  g_signal_connect (G_OBJECT(pRadioButtonFill), "toggled", 
        G_CALLBACK(on_button_alignment_toggled), data_app);

  g_signal_connect (G_OBJECT(pRadioButtonTextSelect), "toggled", 
        G_CALLBACK(on_button_clip_mode_toggled), data_app);
  g_signal_connect (G_OBJECT(pRadioButtonPictureSelect), "toggled", 
        G_CALLBACK(on_button_clip_mode_toggled), data_app);
  g_signal_connect (G_OBJECT(pRadioButtonHiglightSelect), "toggled", 
        G_CALLBACK(on_button_clip_mode_toggled), data_app);
        
  g_signal_connect (G_OBJECT(pRadioButtonHighlightRect), "toggled", 
        G_CALLBACK(on_button_clip_mode_toggled), data_app);        
        
  g_signal_connect (G_OBJECT(pRadioButtonHiAnnotSelect), "toggled", 
        G_CALLBACK(on_button_clip_mode_toggled), data_app);
  g_signal_connect (G_OBJECT(button_pencil), "toggled", 
        G_CALLBACK(on_button_button_pencil_toggled), data_app);

  /* register objects */
  GLADE_HOOKUP_OBJECT (window, button_bold, "button_bold");
  GLADE_HOOKUP_OBJECT (window, button_italic, "button_italic");
  GLADE_HOOKUP_OBJECT (window, button_underline, "button_underline");
  GLADE_HOOKUP_OBJECT (window, button_superscript, "button_superscript");
  GLADE_HOOKUP_OBJECT (window, button_subscript, "button_subscript");
  GLADE_HOOKUP_OBJECT (window, button_strikethrough, "button_strikethrough");
  GLADE_HOOKUP_OBJECT (window, button_highlight, "button_highlight");
  GLADE_HOOKUP_OBJECT (window, button_quotation, "button_quotation");
  GLADE_HOOKUP_OBJECT (window, button_clear_format, "button_clear_format");
  GLADE_HOOKUP_OBJECT (window, pRadioButtonLeft, "pRadioButtonLeft");
  GLADE_HOOKUP_OBJECT (window, pRadioButtonCenter, "pRadioButtonCenter");
  GLADE_HOOKUP_OBJECT (window, pRadioButtonRight, "pRadioButtonRight");
  GLADE_HOOKUP_OBJECT (window, pRadioButtonFill, "pRadioButtonFill");
  
  
  GLADE_HOOKUP_OBJECT (window, pRadioButtonTextSelect, "pRadioButtonTextSelect");
  GLADE_HOOKUP_OBJECT (window, pRadioButtonPictureSelect, "pRadioButtonPictureSelect");
  GLADE_HOOKUP_OBJECT (window, pRadioButtonHiglightSelect, "pRadioButtonHiglightSelect");
  GLADE_HOOKUP_OBJECT (window, pRadioButtonHighlightRect, "pRadioButtonHighlightRect");  
  
  
  GLADE_HOOKUP_OBJECT (window, pRadioButtonHiAnnotSelect, "pRadioButtonHiAnnotSelect");
  GLADE_HOOKUP_OBJECT (window, button_pencil, "button_pencil");
  GLADE_HOOKUP_OBJECT (window, color_button, "color_button");
  GLADE_HOOKUP_OBJECT (window, pRadioButtonPlayPauseAudio, "pRadioButtonPlayPauseAudio");
  GLADE_HOOKUP_OBJECT (window, pRadioButtonCopyPosAudio, "pRadioButtonCopyPosAudio");


  GLADE_HOOKUP_OBJECT (window, pRadioButtonRewindAudio, "pRadioButtonRewindAudio");
  GLADE_HOOKUP_OBJECT (window, pRadioButtonGotoAudio, "pRadioButtonGotoAudio");
  GLADE_HOOKUP_OBJECT (window, pRadioButtonGoJumpAudio, "pRadioButtonGoJumpAudio");
  GLADE_HOOKUP_OBJECT (window, iconButtonPlayAudio, "iconButtonPlayAudio");
  GLADE_HOOKUP_OBJECT (window, iconButtonPauseAudio, "iconButtonPauseAudio");

  GLADE_HOOKUP_OBJECT (window, audio_position, "audio_position");
  GLADE_HOOKUP_OBJECT (window, audio_position_label, "audio_position_label");

  GLADE_HOOKUP_OBJECT (window, audio_total, "audio_total");
  GLADE_HOOKUP_OBJECT (window, audio_total_label, "audio_total_label");
  GLADE_HOOKUP_OBJECT (window, audioPlaySpeed, "audioPlaySpeed");

  GLADE_HOOKUP_OBJECT (window, button_undo, "button_undo");
  data_app->pBtnUndo = GTK_WIDGET(button_undo);
  return toolbar;
}

/****************************
  main window
****************************/
GtkWidget *UI_main_window (GApplication *app, APP_data *data)
{
  GtkWidget *win;

 // win=gtk_window_new(GTK_WINDOW_TOPLEVEL);

/* version Glade */
  win = GTK_WIDGET (gtk_builder_get_object (data->builder, "window_main"));
  data->appWindow = win;
  gtk_window_set_application (GTK_WINDOW(win), GTK_APPLICATION(app));
  gtk_builder_connect_signals (data->builder, data);/* now all calls will get data structure as first parameter */
//  win = gtk_application_window_new (GTK_APPLICATION (app));
  gtk_widget_show (win);
  gtk_window_set_position (GTK_WINDOW(win), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size (GTK_WINDOW(win), 980, 700);
  gtk_window_set_resizable (GTK_WINDOW(win), TRUE);
  gtk_window_set_decorated (GTK_WINDOW(win), TRUE);
  gtk_window_set_title (GTK_WINDOW(win), "Redac !");
  gtk_window_set_icon_name (GTK_WINDOW (win), "redac");
  return win;
}


/****************************
  set up main statusbar
****************************/
void UI_statusbar (GtkWidget *window, GtkWidget *grid, APP_data *data)
{
  GtkWidget *statusbar, *PDF_modified_label;
  GtkWidget *search_entry, *buttonPrevOccurrence;
  GtkWidget *buttonNextOccurrence, *labelHitsFrame;
  GtkWidget *labelHitsGrid, *labelHits;
  GtkWidget *buttonReplace, *buttonZoomIn;
  GtkWidget *buttonZoomOut, *buttonZoomFitBest;
  GtkWidget *replace_entry, *image_task_due;
  GtkWidget *image_pdf_modif, *image_audio_jump_to_start;

  buttonPrevOccurrence = GTK_WIDGET (gtk_builder_get_object (data->builder, "buttonPrevOccurrence")); 
  
  buttonNextOccurrence = GTK_WIDGET (gtk_builder_get_object (data->builder, "buttonNextOccurrence")); 
  
  buttonReplace = GTK_WIDGET (gtk_builder_get_object (data->builder, "buttonReplace")); 
   
 
  /* PDF zoom buttons */
  buttonZoomIn = GTK_WIDGET (gtk_builder_get_object (data->builder, "buttonZoomIn")); 
  buttonZoomOut = GTK_WIDGET (gtk_builder_get_object (data->builder, "buttonZoomOut")); 
  buttonZoomFitBest =  GTK_WIDGET (gtk_builder_get_object (data->builder, "buttonZoomFitBest")); 
  
  statusbar = GTK_WIDGET (gtk_builder_get_object (data->builder, "statusbar")); 
 
  PDF_modified_label = GTK_WIDGET (gtk_builder_get_object (data->builder, "PDF_modified_label")); 
 
  /* editor */
  
  labelHitsFrame = GTK_WIDGET (gtk_builder_get_object (data->builder, "labelHitsFrame")); 

  labelHitsGrid = GTK_WIDGET (gtk_builder_get_object (data->builder, "labelHitsGrid")); 

  search_entry = GTK_WIDGET (gtk_builder_get_object (data->builder, "search_entry")); 

  labelHits = GTK_WIDGET (gtk_builder_get_object (data->builder, "labelHits")); 

  replace_entry = GTK_WIDGET (gtk_builder_get_object (data->builder, "replace_entry")); 
  
  /* indicator auto-save */
  image_task_due = GTK_WIDGET (gtk_builder_get_object (data->builder, "image_task_due")); 
  
  /* audio auto repeat - jump to start icon */
  
  image_audio_jump_to_start = GTK_WIDGET (gtk_builder_get_object (data->builder, "image_audio_jump_to_start")); 
  
  image_pdf_modif = GTK_WIDGET (gtk_builder_get_object (data->builder, "image_pdf_modif"));  

  /* callbacks are inside main.ui file */

  data->statusbar1 = statusbar;
  
  GLADE_HOOKUP_OBJECT (window, buttonNextOccurrence, "buttonNextOccurrence");
  GLADE_HOOKUP_OBJECT (window, buttonPrevOccurrence, "buttonPrevOccurrence");
  GLADE_HOOKUP_OBJECT (window, labelHitsFrame, "labelHitsFrame");
  GLADE_HOOKUP_OBJECT (window, labelHits, "labelHits");
  GLADE_HOOKUP_OBJECT (window, PDF_modified_label, "PDF_modified_label");
  GLADE_HOOKUP_OBJECT (window, search_entry, "search_entry");
  GLADE_HOOKUP_OBJECT (window, replace_entry, "replace_entry");
  GLADE_HOOKUP_OBJECT (window, buttonReplace, "buttonReplace");
  GLADE_HOOKUP_OBJECT (window, buttonZoomIn, "buttonZoomIn");
  GLADE_HOOKUP_OBJECT (window, buttonZoomOut, "buttonZoomOut");
  GLADE_HOOKUP_OBJECT (window, buttonZoomFitBest, "buttonZoomFitBest");
  GLADE_HOOKUP_OBJECT (window, image_task_due, "image_task_due");
  GLADE_HOOKUP_OBJECT (window, image_pdf_modif, "image_pdf_modif");
  GLADE_HOOKUP_OBJECT (window, image_audio_jump_to_start, "image_audio_jump_to_start");
} 
/**********************************
  add PDF page indicator
**********************************/
void UI_pdf_page_widget (GtkWidget *window, GtkWidget *grid, APP_data *data)
{
  GtkWidget *page_frame, *page_grid;
  GtkWidget *page_title, *page_entry;
  GtkWidget *page_label;

  page_frame = GTK_WIDGET (gtk_builder_get_object (data->builder, "page_frame")); 
  
  page_grid = GTK_WIDGET (gtk_builder_get_object (data->builder, "page_grid")); 

  page_title = GTK_WIDGET (gtk_builder_get_object (data->builder, "page_title")); 
  
  page_entry = GTK_WIDGET (gtk_builder_get_object (data->builder, "page_entry")); 
  
  page_label = GTK_WIDGET (gtk_builder_get_object (data->builder, "page_label")); 

  /* callbacks are in main.ui file */

  GLADE_HOOKUP_OBJECT (window, page_title, "page_title");
  GLADE_HOOKUP_OBJECT (window, page_entry, "page_entry");
  GLADE_HOOKUP_OBJECT (window, page_label, "page_label");
  GLADE_HOOKUP_OBJECT (window, page_frame, "page_frame");
}

/*********************************
  load from file dialog 
  * sFileType : message about
  *  files
*********************************/

GtkWidget *create_loadFileDialog (APP_data *data, gchar *sFileType) {
  GtkWidget *loadFileDialog;
 

  loadFileDialog = gtk_file_chooser_dialog_new (sFileType, 
                              GTK_WINDOW(data->appWindow), GTK_FILE_CHOOSER_ACTION_OPEN, 
                              _("_Cancel"),  GTK_RESPONSE_CANCEL,
                              _("_Open"),  GTK_RESPONSE_OK,
                              NULL);
                              
  g_object_set (loadFileDialog, "local-only", FALSE,  NULL);
  gtk_window_set_modal (GTK_WINDOW (loadFileDialog), TRUE);
  gtk_window_set_destroy_with_parent (GTK_WINDOW (loadFileDialog), TRUE);
  gtk_window_set_icon_name (GTK_WINDOW (loadFileDialog), "gtk-file");
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (loadFileDialog), TRUE);
  gtk_window_set_type_hint (GTK_WINDOW (loadFileDialog), GDK_WINDOW_TYPE_HINT_DIALOG);

  GLADE_HOOKUP_OBJECT_NO_REF (loadFileDialog, loadFileDialog, "loadFileDialog");
 
  return loadFileDialog;
}

/*********************************
  save to file dialog 
*********************************/
GtkWidget *create_saveFileDialog (APP_data *data) {
  GtkWidget *saveFileDialog;

  saveFileDialog = gtk_file_chooser_dialog_new (_("Save your work and export to standard Word processor file..."), 
                                    GTK_WINDOW(data->appWindow),
                                    GTK_FILE_CHOOSER_ACTION_SAVE, 
                                    ("_Cancel"),  GTK_RESPONSE_CANCEL,
                                    ("_Save"),  GTK_RESPONSE_OK,
                                    NULL);
                                    
  g_object_set (saveFileDialog, "local-only", FALSE,  NULL);
  gtk_window_set_modal (GTK_WINDOW (saveFileDialog), TRUE);
  gtk_window_set_destroy_with_parent (GTK_WINDOW (saveFileDialog), TRUE);
  gtk_window_set_icon_name (GTK_WINDOW (saveFileDialog), "gtk-file");
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (saveFileDialog), TRUE);
  gtk_window_set_type_hint (GTK_WINDOW (saveFileDialog), GDK_WINDOW_TYPE_HINT_DIALOG);


  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (saveFileDialog, saveFileDialog, "saveFileDialog");

  return saveFileDialog;
}

/*********************************
  help dialog for keys
*********************************/
GtkWidget *misc_create_help_dialog (GtkWidget *win)
{
  GtkWidget *helpDialog;
  GtkWidget *dialog_vbox11;
  GtkWidget *gridDialogHeader;
  GtkWidget *iconTitle;
  GtkWidget *labelTitle;
  GtkWidget *subTitle;
  GtkWidget *gridDialog;
  GtkWidget *labelAlignment;
  GtkWidget *labelFormat;
  GtkWidget *labelSpace;
  GtkWidget *labelFormatBoldTitle;
  GtkWidget *labelAlignmentShortcuts;
  GtkWidget *headerBar;

  helpDialog = gtk_dialog_new ();
  
  // https://stackoverflow.com/questions/53587997/how-to-fix-gtk-warning-content-added-to-the-action-area-of-a-dialog-using-he
  helpDialog = gtk_dialog_new_with_buttons ( _("Help..."),
                                       GTK_WINDOW(win),
                                       GTK_DIALOG_USE_HEADER_BAR, /// Use this FLAG here
                                       _("Ok"),
                                       1,
                                       NULL );
  
  
  headerBar = gtk_dialog_get_header_bar (GTK_DIALOG(helpDialog));
  gtk_header_bar_set_subtitle (GTK_HEADER_BAR (headerBar), _("Keyboard shortcuts"));
  gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (headerBar), FALSE);
  // gtk_window_set_title (GTK_WINDOW (helpDialog), _("Help..."));
  gtk_window_set_position (GTK_WINDOW (helpDialog), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (helpDialog), TRUE);
  gtk_window_set_resizable (GTK_WINDOW (helpDialog), TRUE);
  gtk_window_set_type_hint (GTK_WINDOW (helpDialog), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_transient_for (GTK_WINDOW (helpDialog),  GTK_WINDOW(win)); 
  dialog_vbox11 = gtk_dialog_get_content_area (GTK_DIALOG (helpDialog));
  gtk_widget_show (dialog_vbox11);
  gridDialogHeader = gtk_grid_new ();
  gtk_widget_set_name (gridDialogHeader, "gridDialogHeader");
  g_object_set (gridDialogHeader, "margin-start", 10, NULL);
  g_object_set (gridDialogHeader, "margin-end", 10, NULL);
  gtk_widget_show (gridDialogHeader);
  gtk_box_pack_start (GTK_BOX (dialog_vbox11), gridDialogHeader, TRUE, TRUE, 4);

  /* icon */
  iconTitle = gtk_image_new_from_icon_name ("input-keyboard-symbolic",GTK_ICON_SIZE_DIALOG);
 
  gtk_widget_set_halign (GTK_WIDGET (iconTitle), GTK_ALIGN_CENTER);
  gtk_widget_set_valign (GTK_WIDGET (iconTitle), GTK_ALIGN_CENTER);
 
  gtk_widget_set_hexpand (GTK_WIDGET(iconTitle), FALSE);
  
  gtk_widget_set_margin_start (GTK_WIDGET(iconTitle), 5);
  gtk_widget_set_margin_top (GTK_WIDGET(iconTitle), 5);  
    
  gtk_header_bar_pack_start (GTK_HEADER_BAR (headerBar), GTK_WIDGET(iconTitle));
    
  subTitle = gtk_label_new ("");
  gtk_label_set_markup(GTK_LABEL(subTitle), _("<i>Here is the various keys to set-up formatings and so on.\nWhen 2 keys are displayed, please keep in mind that\nyou must press the 2 keys in the same time.</i>"));
//  gtk_misc_set_padding (GTK_MISC (subTitle), 5, 5);
  
  gtk_widget_set_margin_start (GTK_WIDGET(subTitle), 5);
  gtk_widget_set_margin_top (GTK_WIDGET(subTitle), 5);   
  
  gtk_grid_attach (GTK_GRID(gridDialogHeader), subTitle, 1,1,2,1);

  gridDialog = gtk_grid_new();
  gtk_widget_set_name(gridDialog, "gridDialog");
 // gtk_grid_set_column_homogeneous (GTK_GRID(gridDialog), TRUE);
  g_object_set (gridDialog, "margin-start", 10, NULL);
  g_object_set (gridDialog, "margin-end", 10, NULL);
  gtk_widget_show (gridDialog);
  gtk_box_pack_start (GTK_BOX (dialog_vbox11), gridDialog, TRUE, TRUE, 4);
  /* labels */
  labelFormat = gtk_label_new(_("<b><big>Format</big></b>"));
  gtk_widget_set_name(GTK_WIDGET(labelFormat), "labelFormat");
  g_object_set (labelFormat, "margin-start", 4, NULL);
  g_object_set (labelFormat, "margin-end", 4, NULL);
  gtk_widget_set_hexpand(GTK_WIDGET(labelFormat), TRUE);
  gtk_widget_set_halign (GTK_WIDGET(labelFormat), GTK_ALIGN_FILL);
  gtk_label_set_use_markup (GTK_LABEL (labelFormat), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelFormat, 0, 0, 3, 1);

  labelAlignment = gtk_label_new(_("<b><big>Alignment</big></b>"));
  gtk_widget_set_name(GTK_WIDGET(labelAlignment), "labelAlignment");
  g_object_set (labelAlignment, "margin-start", 4, NULL);
  g_object_set (labelAlignment, "margin-end", 4, NULL);
  gtk_widget_set_hexpand(GTK_WIDGET(labelAlignment), TRUE);
  gtk_widget_set_halign (GTK_WIDGET(labelAlignment), GTK_ALIGN_FILL);
  gtk_label_set_use_markup (GTK_LABEL (labelAlignment), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelAlignment, 3, 0, 2, 1);

  GtkWidget *labelPdfSketch = gtk_label_new(_("<b><big>PDF-Sketch</big></b>"));
  gtk_widget_set_name(GTK_WIDGET(labelPdfSketch), "labelPdfSketch");
  g_object_set (labelPdfSketch, "margin-start", 4, NULL);
  g_object_set (labelPdfSketch, "margin-end", 4, NULL);
  gtk_widget_set_hexpand(GTK_WIDGET(labelPdfSketch), TRUE);
  gtk_widget_set_halign (GTK_WIDGET(labelPdfSketch), GTK_ALIGN_FILL);
  gtk_label_set_use_markup (GTK_LABEL (labelPdfSketch), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelPdfSketch, 5, 0, 2, 1);

  GtkWidget *labelOther = gtk_label_new(_("<b><big>Application</big></b>"));
  gtk_widget_set_name(GTK_WIDGET(labelOther), "labelOther");
  g_object_set (labelOther, "margin-start", 4, NULL);
  g_object_set (labelOther, "margin-end", 4, NULL);
  gtk_widget_set_hexpand(GTK_WIDGET(labelOther), TRUE);
  gtk_widget_set_halign (GTK_WIDGET(labelOther), GTK_ALIGN_FILL);
  gtk_label_set_use_markup (GTK_LABEL (labelOther), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelOther, 7, 0, 2, 1);

  labelFormatBoldTitle = gtk_label_new(_("Bold"));
  g_object_set (labelFormatBoldTitle, "margin", 4, NULL);
  gtk_widget_set_halign (GTK_WIDGET(labelFormatBoldTitle), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelFormatBoldTitle, 0, 1, 1, 1);

  GtkWidget *labelFormatItalicTitle = gtk_label_new(_("Italic"));
  g_object_set (labelFormatItalicTitle, "margin", 4, NULL);
  gtk_widget_set_halign (GTK_WIDGET(labelFormatItalicTitle), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelFormatItalicTitle, 0, 2, 1, 1);

  GtkWidget *labelFormatUnderlineTitle = gtk_label_new(_("Underline"));
  g_object_set (labelFormatUnderlineTitle, "margin", 4, NULL);
  gtk_widget_set_halign (GTK_WIDGET(labelFormatUnderlineTitle), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelFormatUnderlineTitle, 0, 3, 1, 1);

  GtkWidget *labelFormatSuperscriptTitle = gtk_label_new(_("Superscript"));
  g_object_set (labelFormatSuperscriptTitle, "margin", 4, NULL);
  gtk_widget_set_halign (GTK_WIDGET(labelFormatSuperscriptTitle), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelFormatSuperscriptTitle, 0, 4, 1, 1);

  GtkWidget *labelFormatSubscriptTitle = gtk_label_new(_("Subscript"));
  g_object_set (labelFormatSubscriptTitle, "margin", 4, NULL);
  gtk_widget_set_halign (GTK_WIDGET(labelFormatSubscriptTitle), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelFormatSubscriptTitle, 0, 5, 1, 1);

  GtkWidget *labelFormatstrikethroughTitle = gtk_label_new(_("Strikethrough"));
  g_object_set (labelFormatstrikethroughTitle, "margin", 4, NULL);
  gtk_widget_set_halign (GTK_WIDGET(labelFormatstrikethroughTitle), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelFormatstrikethroughTitle, 0, 6, 1, 1);

  GtkWidget *labelFormatHighlightTitle = gtk_label_new(_("Highlight"));
  g_object_set (labelFormatHighlightTitle, "margin", 4, NULL);
  gtk_widget_set_halign (GTK_WIDGET(labelFormatHighlightTitle), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelFormatHighlightTitle, 0, 7, 1, 1);

  GtkWidget *labelFormatQuotationTitle = gtk_label_new(_("Quotation"));
  g_object_set (labelFormatQuotationTitle, "margin", 4, NULL);
  gtk_widget_set_halign (GTK_WIDGET(labelFormatQuotationTitle), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelFormatQuotationTitle, 0, 8, 1, 1);

  GtkWidget *labelUpLowCaseTitle = gtk_label_new(_("Toggle up/lower case"));
  g_object_set (labelUpLowCaseTitle, "margin", 4, NULL);
  gtk_widget_set_halign (GTK_WIDGET(labelUpLowCaseTitle), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelUpLowCaseTitle, 0, 9, 1, 1);

  GtkWidget *labelBoldShortcut = gtk_label_new(misc_get_pango_string("b", 1));
  gtk_widget_set_halign (GTK_WIDGET(labelBoldShortcut), GTK_ALIGN_START);
  gtk_widget_show(labelBoldShortcut);
  gtk_label_set_use_markup (GTK_LABEL (labelBoldShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelBoldShortcut, 1, 1, 1, 1);

  GtkWidget *labelItalicShortcut = gtk_label_new(misc_get_pango_string("i", 1));
  gtk_widget_set_halign (GTK_WIDGET(labelItalicShortcut), GTK_ALIGN_START);
  gtk_widget_show(labelItalicShortcut);
  gtk_label_set_use_markup (GTK_LABEL (labelItalicShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelItalicShortcut, 1, 2, 1, 1);

  GtkWidget *labelUnderlineShortcut = gtk_label_new(misc_get_pango_string("u", 1));
  gtk_widget_set_halign (GTK_WIDGET(labelUnderlineShortcut), GTK_ALIGN_START);
  gtk_label_set_use_markup (GTK_LABEL (labelUnderlineShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelUnderlineShortcut, 1, 3, 1, 1);

  GtkWidget *labelSuperscriptShortcut = gtk_label_new(misc_get_pango_string("^", 1));
  gtk_widget_set_halign (GTK_WIDGET(labelSuperscriptShortcut), GTK_ALIGN_START);
  gtk_label_set_use_markup (GTK_LABEL (labelSuperscriptShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelSuperscriptShortcut, 1, 4, 1, 1);

  GtkWidget *labelSubscriptShortcut = gtk_label_new(misc_get_pango_string("_", 1));
  gtk_widget_set_halign (GTK_WIDGET(labelSubscriptShortcut), GTK_ALIGN_START);
  gtk_label_set_use_markup (GTK_LABEL (labelSubscriptShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelSubscriptShortcut, 1, 5, 1, 1);

  GtkWidget *labelStrikethroughShortcut = gtk_label_new(misc_get_pango_string("k", 1));
  gtk_widget_set_halign (GTK_WIDGET(labelStrikethroughShortcut), GTK_ALIGN_START);
  gtk_label_set_use_markup (GTK_LABEL (labelStrikethroughShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelStrikethroughShortcut, 1, 6, 1, 1);

  GtkWidget *labelHighlightShortcut = gtk_label_new(misc_get_pango_string("h", 1));
  gtk_widget_set_halign (GTK_WIDGET(labelHighlightShortcut), GTK_ALIGN_START);
  gtk_label_set_use_markup (GTK_LABEL (labelHighlightShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelHighlightShortcut, 1, 7, 1, 1);

  GtkWidget *labelQuotationShortcut = gtk_label_new(misc_get_pango_string("\"", 1));
  gtk_widget_set_halign (GTK_WIDGET(labelQuotationShortcut), GTK_ALIGN_START);
  gtk_label_set_use_markup (GTK_LABEL (labelQuotationShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelQuotationShortcut, 1, 8, 1, 1);

  GtkWidget *labelUpLowCaseShortcut = gtk_label_new(misc_get_pango_string("F3", 1));
  gtk_widget_set_halign (GTK_WIDGET(labelUpLowCaseShortcut), GTK_ALIGN_START);
  gtk_label_set_use_markup (GTK_LABEL (labelUpLowCaseShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelUpLowCaseShortcut, 1, 9, 1, 1);


/* aligments */
  GtkWidget *labelAlignmentLeftShortcuts = gtk_label_new(_("Left"));
  gtk_widget_set_halign (GTK_WIDGET(labelAlignmentLeftShortcuts), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelAlignmentLeftShortcuts, 3, 1, 1, 1);

  GtkWidget *labelAlignmentCenterShortcuts = gtk_label_new(_("Center"));
  gtk_widget_set_halign (GTK_WIDGET(labelAlignmentCenterShortcuts), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelAlignmentCenterShortcuts, 3, 2, 1, 1);

  GtkWidget *labelAlignmentRightShortcuts = gtk_label_new(_("Right"));
  gtk_widget_set_halign (GTK_WIDGET(labelAlignmentRightShortcuts), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelAlignmentRightShortcuts, 3, 3, 1, 1);

  GtkWidget *labelAlignmentFillShortcuts = gtk_label_new(_("Fill"));
  gtk_widget_set_halign (GTK_WIDGET(labelAlignmentFillShortcuts), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelAlignmentFillShortcuts, 3, 4, 1, 1);

  GtkWidget *labelAlignmentLeftShortcut = gtk_label_new(misc_get_pango_string("(", 1));
  gtk_widget_set_halign (GTK_WIDGET(labelAlignmentLeftShortcut), GTK_ALIGN_START);
  gtk_widget_show(labelAlignmentLeftShortcut);
  gtk_label_set_use_markup (GTK_LABEL (labelAlignmentLeftShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelAlignmentLeftShortcut, 4, 1, 1, 1);

  GtkWidget *labelAlignmentCenterShortcut = gtk_label_new(misc_get_pango_string(":", 1));
  gtk_widget_set_halign (GTK_WIDGET(labelAlignmentCenterShortcut), GTK_ALIGN_START);
  gtk_label_set_use_markup (GTK_LABEL (labelAlignmentCenterShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelAlignmentCenterShortcut, 4, 2, 1, 1);

  GtkWidget *labelAlignmentRightShortcut = gtk_label_new(misc_get_pango_string(")", 1));
  gtk_widget_set_halign (GTK_WIDGET(labelAlignmentRightShortcut), GTK_ALIGN_START);
  gtk_label_set_use_markup (GTK_LABEL (labelAlignmentRightShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelAlignmentRightShortcut, 4, 3, 1, 1);

  GtkWidget *labelAlignmentFillShortcut = gtk_label_new(misc_get_pango_string("=", 1));
  gtk_widget_set_halign (GTK_WIDGET(labelAlignmentFillShortcut), GTK_ALIGN_START);
  gtk_label_set_use_markup (GTK_LABEL (labelAlignmentFillShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelAlignmentFillShortcut, 4, 4, 1, 1);
/* PDF & sketch */
  GtkWidget *labelPdfPgDownShortcuts = gtk_label_new(_("PDF : go next page"));
  gtk_widget_set_halign (GTK_WIDGET(labelPdfPgDownShortcuts), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelPdfPgDownShortcuts, 5, 1, 1, 1);

  GtkWidget *labelPdfPgDownShortcut = gtk_label_new(misc_get_pango_string(_("PgDown"), 0));
  gtk_widget_set_halign (GTK_WIDGET(labelPdfPgDownShortcut), GTK_ALIGN_START);
  gtk_label_set_use_markup (GTK_LABEL (labelPdfPgDownShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelPdfPgDownShortcut, 6, 1, 1, 1);

  GtkWidget *labelPdfPgUpShortcuts = gtk_label_new(_("PDF : go previous page"));
  gtk_widget_set_halign (GTK_WIDGET(labelPdfPgUpShortcuts), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelPdfPgUpShortcuts, 5, 2, 1, 1);

  GtkWidget *labelPdfPgUpShortcut = gtk_label_new(misc_get_pango_string(_("PgUp"), 0));
  gtk_widget_set_halign (GTK_WIDGET(labelPdfPgUpShortcut), GTK_ALIGN_START);
  gtk_label_set_use_markup (GTK_LABEL (labelPdfPgUpShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelPdfPgUpShortcut, 6, 2, 1, 1);

  GtkWidget *labelPdfFirstShortcuts = gtk_label_new(_("PDF : go first page"));
  gtk_widget_set_halign (GTK_WIDGET(labelPdfFirstShortcuts), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelPdfFirstShortcuts, 5, 3, 1, 1);

  GtkWidget *labelPdfFirstShortcut = gtk_label_new(misc_get_pango_string("Home", 0));
  gtk_widget_set_halign (GTK_WIDGET(labelPdfFirstShortcut), GTK_ALIGN_START);
  gtk_label_set_use_markup (GTK_LABEL (labelPdfFirstShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelPdfFirstShortcut, 6, 3, 1, 1);

  GtkWidget *labelPdfLastShortcuts = gtk_label_new(_("PDF : go last page"));
  gtk_widget_set_halign (GTK_WIDGET(labelPdfLastShortcuts), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelPdfLastShortcuts, 5, 4, 1, 1);

  GtkWidget *labelPdfLastShortcut = gtk_label_new(misc_get_pango_string("End", 0));
  gtk_widget_set_halign (GTK_WIDGET(labelPdfLastShortcut), GTK_ALIGN_START);
  gtk_label_set_use_markup (GTK_LABEL (labelPdfLastShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelPdfLastShortcut, 6, 4, 1, 1);

  GtkWidget *labelPdfSearchShortcuts = gtk_label_new(_("PDF : search"));
  gtk_widget_set_halign (GTK_WIDGET(labelPdfSearchShortcuts), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelPdfSearchShortcuts, 5, 5, 1, 1);

  GtkWidget *labelPdfSearchShortcut = gtk_label_new(misc_get_pango_string("f", 1));
  gtk_widget_set_halign (GTK_WIDGET(labelPdfSearchShortcut), GTK_ALIGN_START);
  gtk_label_set_use_markup (GTK_LABEL (labelPdfSearchShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelPdfSearchShortcut, 6, 5, 1, 1);

  GtkWidget *labelPdfZoomInShortcuts = gtk_label_new(_("PDF : zoom in"));
  gtk_widget_set_halign (GTK_WIDGET(labelPdfZoomInShortcuts), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelPdfZoomInShortcuts, 5, 6, 1, 1);

  GtkWidget *labelPdfZoomInShortcut = gtk_label_new(misc_get_pango_string("+", 1));
  gtk_widget_set_halign (GTK_WIDGET(labelPdfZoomInShortcut), GTK_ALIGN_START);
  gtk_widget_show(labelPdfZoomInShortcut);
  gtk_label_set_use_markup (GTK_LABEL (labelPdfZoomInShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelPdfZoomInShortcut, 6, 6, 1, 1);

  GtkWidget *labelPdfZoomOutShortcuts = gtk_label_new(_("PDF : zoom out"));
  gtk_widget_set_halign (GTK_WIDGET(labelPdfZoomOutShortcuts), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelPdfZoomOutShortcuts, 5, 7, 1, 1);

  GtkWidget *labelPdfZoomOutShortcut = gtk_label_new(misc_get_pango_string("-", 1));
  gtk_widget_set_halign (GTK_WIDGET(labelPdfZoomOutShortcut), GTK_ALIGN_START);
  gtk_label_set_use_markup (GTK_LABEL (labelPdfZoomOutShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelPdfZoomOutShortcut, 6, 7, 1, 1);
/* sketch */
  GtkWidget *labelSketchPenUpShortcuts = gtk_label_new(_("Sketch : larger pen"));
  gtk_widget_set_halign (GTK_WIDGET(labelSketchPenUpShortcuts), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelSketchPenUpShortcuts, 5, 8, 1, 1);

  GtkWidget *labelSketchPenUpShortcut = gtk_label_new(misc_get_pango_string("+", 1));
  gtk_widget_set_halign (GTK_WIDGET(labelSketchPenUpShortcut), GTK_ALIGN_START);
  gtk_label_set_use_markup (GTK_LABEL (labelSketchPenUpShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelSketchPenUpShortcut, 6, 8, 1, 1);

  GtkWidget *labelSketchPenDwnShortcuts = gtk_label_new(_("Sketch : leaner pen"));
  gtk_widget_set_halign (GTK_WIDGET(labelSketchPenDwnShortcuts), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelSketchPenDwnShortcuts, 5, 9, 1, 1);

  GtkWidget *labelSketchPenDwnShortcut = gtk_label_new(misc_get_pango_string("-", 1));
  gtk_widget_set_halign (GTK_WIDGET(labelSketchPenDwnShortcut), GTK_ALIGN_START);
  gtk_label_set_use_markup (GTK_LABEL (labelSketchPenDwnShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelSketchPenDwnShortcut, 6, 9, 1, 1);
/* application */
  GtkWidget *labelOthersHelpShortcuts = gtk_label_new(_("Help"));
  gtk_widget_set_halign (GTK_WIDGET(labelOthersHelpShortcuts), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelOthersHelpShortcuts, 7, 1, 1, 1);

  GtkWidget *labelOtherHelpShortcut = gtk_label_new(misc_get_pango_string("F1", 1));
  gtk_widget_set_halign (GTK_WIDGET(labelOtherHelpShortcut), GTK_ALIGN_START);
  gtk_label_set_use_markup (GTK_LABEL (labelOtherHelpShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelOtherHelpShortcut, 8, 1, 1, 1);

  GtkWidget *labelOthersPasteImageShortcuts = gtk_label_new(_("Paste image"));
  gtk_widget_set_halign (GTK_WIDGET(labelOthersPasteImageShortcuts), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelOthersPasteImageShortcuts, 7, 2, 1, 1);

  GtkWidget *labelOtherPasteImageShortcut = gtk_label_new(misc_get_pango_string("v", 1));
  gtk_widget_set_halign (GTK_WIDGET(labelOtherPasteImageShortcut), GTK_ALIGN_START);
  gtk_label_set_use_markup (GTK_LABEL (labelOtherPasteImageShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelOtherPasteImageShortcut, 8, 2, 1, 1);

  GtkWidget *labelOthersStorageShortcuts = gtk_label_new(_("Call main menu"));
  gtk_widget_set_halign (GTK_WIDGET(labelOthersStorageShortcuts), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelOthersStorageShortcuts, 7, 3, 1, 1);

  GtkWidget *labelOthersStorageShortcut = gtk_label_new(misc_get_pango_string("m", 1));
  gtk_widget_set_halign (GTK_WIDGET(labelOthersStorageShortcut), GTK_ALIGN_START);
  gtk_label_set_use_markup (GTK_LABEL (labelOthersStorageShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelOthersStorageShortcut, 8, 3, 1, 1);

  GtkWidget *labelOthersQuickSaveShortcuts = gtk_label_new(_("Quick save"));
  gtk_widget_set_halign (GTK_WIDGET(labelOthersQuickSaveShortcuts), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelOthersQuickSaveShortcuts, 7, 4, 1, 1);

  GtkWidget *labelOthersQuickSaveShortcut = gtk_label_new(misc_get_pango_string("s", 1));
  gtk_widget_set_halign (GTK_WIDGET(labelOthersQuickSaveShortcut), GTK_ALIGN_START);
  gtk_label_set_use_markup (GTK_LABEL (labelOthersQuickSaveShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelOthersQuickSaveShortcut, 8, 4, 1, 1);

  GtkWidget *labelOthersLoadPDFShortcuts = gtk_label_new(_("Load PDF file "));
  gtk_widget_set_halign (GTK_WIDGET(labelOthersLoadPDFShortcuts), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelOthersLoadPDFShortcuts, 7, 5, 1, 1);

  GtkWidget *labelOthersLoadPDFShortcut = gtk_label_new(misc_get_pango_string("d", 1));
  gtk_widget_set_halign (GTK_WIDGET(labelOthersLoadPDFShortcut), GTK_ALIGN_START);
  gtk_widget_show(labelOthersLoadPDFShortcut);
  gtk_label_set_use_markup (GTK_LABEL (labelOthersLoadPDFShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelOthersLoadPDFShortcut, 8, 5, 1, 1);

  GtkWidget *labelOthersFindShortcuts = gtk_label_new(_("Find for selected text"));
  gtk_widget_set_halign (GTK_WIDGET(labelOthersFindShortcuts), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelOthersFindShortcuts, 7, 6, 1, 1);

  GtkWidget *labelOthersFindShortcut = gtk_label_new(misc_get_pango_string("f", 1));
  gtk_widget_set_halign (GTK_WIDGET(labelOthersFindShortcut), GTK_ALIGN_START);
  gtk_label_set_use_markup (GTK_LABEL (labelOthersFindShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelOthersFindShortcut, 8, 6, 1, 1);

  GtkWidget *labelOthersHideShortcuts = gtk_label_new(_("Hide/show main toolbar"));
  gtk_widget_set_halign (GTK_WIDGET(labelOthersHideShortcuts), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelOthersHideShortcuts, 7, 7, 1, 1);

  GtkWidget *labelOthersHideShortcut = gtk_label_new(misc_get_pango_string("F10", 1));
  gtk_widget_set_halign (GTK_WIDGET(labelOthersHideShortcut), GTK_ALIGN_START);
  gtk_label_set_use_markup (GTK_LABEL (labelOthersHideShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelOthersHideShortcut, 8, 7, 1, 1);

  GtkWidget *labelOthersEditorPaneShortcuts = gtk_label_new(_("Switch to editor pane"));
  gtk_widget_set_halign (GTK_WIDGET(labelOthersEditorPaneShortcuts), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelOthersEditorPaneShortcuts, 7, 8, 1, 1);

  GtkWidget *labelOthersEditorPaneShortcut = gtk_label_new(misc_get_pango_string("1", 1));
  gtk_widget_set_halign (GTK_WIDGET(labelOthersEditorPaneShortcut), GTK_ALIGN_START);
  gtk_label_set_use_markup (GTK_LABEL (labelOthersEditorPaneShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelOthersEditorPaneShortcut, 8, 8, 1, 1);

  GtkWidget *labelOthersPDFPaneShortcuts = gtk_label_new(_("Switch to PDF pane"));
  gtk_widget_set_halign (GTK_WIDGET(labelOthersPDFPaneShortcuts), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelOthersPDFPaneShortcuts, 7, 9, 1, 1);

  GtkWidget *labelOthersPDFPaneShortcut = gtk_label_new(misc_get_pango_string("2", 1));
  gtk_widget_set_halign (GTK_WIDGET(labelOthersPDFPaneShortcut), GTK_ALIGN_START);
  gtk_label_set_use_markup (GTK_LABEL (labelOthersPDFPaneShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelOthersPDFPaneShortcut, 8, 9, 1, 1);

  GtkWidget *labelOthersSketchPaneShortcuts = gtk_label_new(_("Switch to sketch pane"));
  gtk_widget_set_halign (GTK_WIDGET(labelOthersSketchPaneShortcuts), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelOthersSketchPaneShortcuts, 7, 10, 1, 1);

  GtkWidget *labelOthersSketchPaneShortcut = gtk_label_new(misc_get_pango_string("3", 1));
  gtk_widget_set_halign (GTK_WIDGET(labelOthersSketchPaneShortcut), GTK_ALIGN_START);
  gtk_label_set_use_markup (GTK_LABEL (labelOthersSketchPaneShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelOthersSketchPaneShortcut, 8, 10, 1, 1);

  GtkWidget *labelOthersQuitShortcuts = gtk_label_new(_("Quit application"));
  g_object_set (labelOthersQuitShortcuts, "margin", 4, NULL);
  gtk_widget_set_halign (GTK_WIDGET(labelOthersQuitShortcuts), GTK_ALIGN_END);
  gtk_grid_attach (GTK_GRID (gridDialog), labelOthersQuitShortcuts, 7, 11, 1, 1);

  GtkWidget *labelOthersQuitShortcut = gtk_label_new(misc_get_pango_string("q", 1));
  gtk_widget_set_halign (GTK_WIDGET(labelOthersQuitShortcut), GTK_ALIGN_START);
  gtk_label_set_use_markup (GTK_LABEL (labelOthersQuitShortcut), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelOthersQuitShortcut, 8, 11, 1, 1);

  GtkWidget *separator=gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_grid_attach (GTK_GRID (gridDialog), separator, 0, 12, 9, 1);


  gtk_widget_show_all(helpDialog);
  return helpDialog;
}

/**************************************************************
  dialog to set up an annotation inside the PDF or Sketch 
  with option to choose font
**************************************************************/
gchar *dialog_add_text_annotation (GtkWidget *win, gchar *current_str, APP_data *data)
{
  GtkWidget *annotDialog, *gridDialog;
  GtkWidget *dialog_vbox11, *entry;
  GtkTextBuffer *buffer;
  GtkWidget  *labelFont;
  GtkWidget *font_button, *separator;
  GtkWidget *headerBar;
  gchar *newFont, *tmpStr=NULL;
  gint ret;
  GKeyFile *keyString;

  keyString = g_object_get_data (G_OBJECT(win), "config"); 
  
  
 // https://stackoverflow.com/questions/53587997/how-to-fix-gtk-warning-content-added-to-the-action-area-of-a-dialog-using-he
  annotDialog = gtk_dialog_new_with_buttons ( _("New Annotation ..."),
                                       GTK_WINDOW(win),
                                       GTK_DIALOG_USE_HEADER_BAR, /// Use this FLAG here
                                       _("Cancel"),
                                       GTK_RESPONSE_CANCEL,
                                       _("Ok"),
                                       GTK_RESPONSE_OK,
                                       NULL );  
  
  
  headerBar = gtk_dialog_get_header_bar (GTK_DIALOG(annotDialog));
  g_object_set (headerBar, "spacing", 48, NULL);
  gtk_header_bar_set_subtitle (GTK_HEADER_BAR (headerBar), _("Type here annotation text"));
  gtk_window_set_default_size(GTK_WINDOW(annotDialog),400, 180);
 // gtk_window_set_title (GTK_WINDOW (annotDialog), _("New Annotation ..."));
  gtk_window_set_position (GTK_WINDOW (annotDialog), GTK_WIN_POS_MOUSE);
  gtk_window_set_modal (GTK_WINDOW (annotDialog), TRUE);
  gtk_window_set_resizable (GTK_WINDOW (annotDialog), TRUE);
  gtk_window_set_decorated (GTK_WINDOW (annotDialog), TRUE);


  gtk_window_set_type_hint (GTK_WINDOW (annotDialog), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_transient_for (GTK_WINDOW (annotDialog),  GTK_WINDOW(win)); 
  dialog_vbox11 = gtk_dialog_get_content_area (GTK_DIALOG (annotDialog));
  gtk_widget_show (dialog_vbox11);
  gridDialog = gtk_grid_new();
  gtk_widget_show (gridDialog);
  gtk_box_pack_start (GTK_BOX (dialog_vbox11), gridDialog, TRUE, TRUE, 4);
  /* entry */
  buffer = gtk_text_buffer_new(NULL);
  entry = gtk_text_view_new_with_buffer (buffer);
  gtk_widget_set_hexpand (entry,TRUE);
  gtk_widget_set_vexpand (entry,TRUE);
  gtk_text_buffer_set_text (buffer, current_str, -1);
  gtk_widget_show (entry);
  gtk_grid_attach (GTK_GRID (gridDialog), entry, 0, 0, 2, 1);

  separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_widget_show (separator);
  gtk_grid_attach (GTK_GRID (gridDialog), separator, 0, 1, 2, 1);
  /* font - only for Sketch */
  labelFont = gtk_label_new(_("Current font : "));
  gtk_widget_show (labelFont);
  gtk_grid_attach (GTK_GRID (gridDialog), labelFont, 0, 2, 1, 1);
  font_button = gtk_font_button_new_with_font ("sans 14");
  gtk_widget_show (font_button);
  gtk_grid_attach (GTK_GRID(gridDialog),font_button, 1, 2 , 1, 1);
  gtk_font_chooser_set_font (GTK_FONT_CHOOSER(font_button), 
                 g_key_file_get_string (keyString, "sketch", "font", NULL));
  if (data->currentStack==CURRENT_STACK_SKETCH) {
    gtk_widget_set_sensitive (GTK_WIDGET(labelFont), TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET(font_button), TRUE);
  } else {
    gtk_widget_set_sensitive (GTK_WIDGET(labelFont), FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET(font_button), FALSE);
  }
 
  /* run */
  ret = gtk_dialog_run (GTK_DIALOG(annotDialog));
  if (ret == GTK_RESPONSE_OK) {
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter (buffer, &start);
    gtk_text_buffer_get_end_iter (buffer, &end);
    tmpStr = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
    /* get the fonts */
    newFont = gtk_font_chooser_get_font(GTK_FONT_CHOOSER(font_button));
    if (newFont != NULL) {
       g_key_file_set_string (keyString, "sketch", "font",newFont);
       g_free (newFont);
    }
  }
  gtk_widget_destroy (GTK_WIDGET(annotDialog));
  return tmpStr;
}



/********************************
  choose color for Annotations
********************************/
GtkWidget *create_annotationColourDialog (APP_data *data_app, gchar *msg)
{
  GtkWidget *highlightColourDialog;


  highlightColourDialog = gtk_color_chooser_dialog_new (msg,GTK_WINDOW(data_app->appWindow));
  gtk_container_set_border_width (GTK_CONTAINER (highlightColourDialog), 5);
  gtk_window_set_resizable (GTK_WINDOW (highlightColourDialog), FALSE);
  gtk_window_set_destroy_with_parent (GTK_WINDOW (highlightColourDialog), TRUE);
  gtk_window_set_icon_name (GTK_WINDOW (highlightColourDialog), "gtk-select-color");
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (highlightColourDialog), TRUE);
  gtk_window_set_type_hint (GTK_WINDOW (highlightColourDialog), GDK_WINDOW_TYPE_HINT_DIALOG);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (highlightColourDialog, highlightColourDialog, "highlightColourDialog");

  return highlightColourDialog;
}

/**********************************
  jump to audio file position
  dialog
**********************************/
GtkWidget *misc_create_go_jump_dialog (APP_data *data_app)
{
  GtkWidget *goJumpDialog, *dialog_vbox11;
  GtkWidget *gridDialog, *gridHeader;
  GtkWidget *labelgoJumpDialog, *iconGoJumpDialog;
  GtkWidget *dialog_action_area11, *okbutton6;
  GtkWidget *imageOkButton6, *cancelbutton;
  GtkWidget *imagecancelbutton, *labelHour;
  GtkWidget *labelMinute, *labelSecond;
  GtkWidget *labelCurpos, *labelTitleCurpos, *labelNewPosition;
  gdouble hour, min, sec;
  GtkWidget *headerBar;
  
  // https://stackoverflow.com/questions/53587997/how-to-fix-gtk-warning-content-added-to-the-action-area-of-a-dialog-using-he
  goJumpDialog = gtk_dialog_new_with_buttons (_("Jump to..."),
                                       GTK_WINDOW(data_app->appWindow),
                                       GTK_DIALOG_USE_HEADER_BAR, /// Use this FLAG here
                                       _("Cancel"),
                                       GTK_RESPONSE_CANCEL,
                                      _("Ok"),
                                       GTK_RESPONSE_OK,                                       
                                       NULL );
  
  
  headerBar = gtk_dialog_get_header_bar (GTK_DIALOG(goJumpDialog));
  gtk_header_bar_set_subtitle (GTK_HEADER_BAR (headerBar), _("Select a new time position"));
  g_object_set (headerBar, "spacing", 48, NULL);
  /* we read and convert to human form the media duration */

 // goJumpDialog = gtk_dialog_new ();
//  gtk_window_set_title (GTK_WINDOW (goJumpDialog), _("Jump to..."));
  gtk_window_set_position (GTK_WINDOW (goJumpDialog), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (goJumpDialog), TRUE);
  gtk_window_set_resizable (GTK_WINDOW (goJumpDialog), TRUE);
  gtk_window_set_type_hint (GTK_WINDOW (goJumpDialog), GDK_WINDOW_TYPE_HINT_DIALOG);

  gtk_window_set_transient_for (GTK_WINDOW (goJumpDialog),  GTK_WINDOW(data_app->appWindow)); 

  dialog_vbox11 = gtk_dialog_get_content_area (GTK_DIALOG (goJumpDialog));
  gtk_widget_show (dialog_vbox11);
  gridDialog = gtk_grid_new();
  gridHeader = gtk_grid_new();
  g_object_set (gridDialog, "margin-start", 8, NULL);
  g_object_set (gridDialog, "margin-end", 8, NULL);
 // gtk_grid_set_row_homogeneous (GTK_GRID (gridDialog), TRUE); 
  gtk_grid_set_column_homogeneous (GTK_GRID (gridDialog), TRUE); 
  gtk_grid_set_column_homogeneous (GTK_GRID (gridHeader), FALSE); 
  gtk_widget_show (gridDialog);
  gtk_widget_show (gridHeader);
  gtk_grid_attach (GTK_GRID (gridDialog), gridHeader, 0, 0, 3, 1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox11), gridDialog, TRUE, TRUE, 4);

  /* main icon */
 // iconGoJumpDialog = gtk_image_new_from_icon_name ("find-location-symbolic",GTK_ICON_SIZE_DIALOG);
 // gtk_widget_show (iconGoJumpDialog);  
//  g_object_set (iconGoJumpDialog, "margin-start", 4, NULL);
//  g_object_set (iconGoJumpDialog, "margin-end", 4, NULL);
//  gtk_header_bar_pack_start (GTK_HEADER_BAR (headerBar), GTK_WIDGET(iconGoJumpDialog));
  /* labels */
  labelgoJumpDialog = gtk_label_new(_("<i>Please choose a time location within the audio file.\n</i>"));
  gtk_widget_show (labelgoJumpDialog);
  g_object_set (labelgoJumpDialog, "margin-start", 4, NULL);
  g_object_set (labelgoJumpDialog, "margin-end", 4, NULL);
  gtk_label_set_use_markup (GTK_LABEL (labelgoJumpDialog), TRUE);
  gtk_grid_attach (GTK_GRID (gridHeader), labelgoJumpDialog, 1, 0, 1, 1);

  /* various infos */
  labelTitleCurpos = gtk_label_new (_("<b>Current position :</b>"));
  gtk_widget_show (labelTitleCurpos);
  g_object_set (labelTitleCurpos, "margin-start", 4, NULL);
  g_object_set (labelTitleCurpos, "margin-end", 4, NULL);
  gtk_label_set_use_markup (GTK_LABEL (labelTitleCurpos), TRUE);
  gtk_grid_attach (GTK_GRID (gridHeader), labelTitleCurpos, 0, 1, 3, 1);

  labelCurpos = gtk_label_new (g_strdup_printf (_("<i> %s of %s</i>"), 
               audio_gst_time_to_str(data_app->audio_current_position),
               audio_gst_time_to_str(data_app->audio_total_duration)));
  gtk_widget_show (labelCurpos);
  g_object_set (labelCurpos, "margin-start", 4, NULL);
  g_object_set (labelCurpos, "margin-end", 4, NULL);
  gtk_label_set_use_markup (GTK_LABEL (labelCurpos), TRUE);
  gtk_grid_attach (GTK_GRID (gridHeader), labelCurpos, 0, 2, 3, 1);

  labelNewPosition = gtk_label_new (_("<i><b>\nNew position : </b></i>"));
  gtk_widget_show (labelNewPosition);
  g_object_set (labelCurpos, "margin-start", 4, NULL);
  g_object_set (labelCurpos, "margin-end", 4, NULL);
  gtk_label_set_use_markup (GTK_LABEL (labelNewPosition), TRUE);
  gtk_grid_attach (GTK_GRID (gridHeader), labelNewPosition, 0, 3, 3, 1);


  labelHour = gtk_label_new (_("Hour :"));
  gtk_widget_show (labelHour);
  g_object_set (labelHour, "margin-start", 10, NULL);
  g_object_set (labelHour, "margin-end", 4, NULL);
  gtk_grid_attach (GTK_GRID (gridDialog), labelHour, 0, 1, 1, 1); 

  labelMinute = gtk_label_new (_("Minute :"));
  gtk_widget_show (labelMinute);
  g_object_set (labelMinute, "margin-start", 4, NULL);
  g_object_set (labelMinute, "margin-end", 4, NULL);
  gtk_grid_attach (GTK_GRID (gridDialog), labelMinute, 1, 1, 1, 1); 

  labelSecond = gtk_label_new (_("Second :"));
  gtk_widget_show (labelSecond);
  g_object_set (labelSecond, "margin-start", 4, NULL);
  g_object_set (labelSecond, "margin-end", 10, NULL);
  gtk_grid_attach (GTK_GRID (gridDialog), labelSecond, 2, 1, 1, 1); 

  /* adjustments */
  GtkAdjustment *hour_adj = gtk_adjustment_new (0, 0, 23, 1, 10, 0);
  GtkWidget *hourSpin = gtk_spin_button_new (GTK_ADJUSTMENT (hour_adj), 1, 0);
  gtk_widget_show (hourSpin);
  gtk_grid_attach (GTK_GRID(gridDialog), hourSpin, 0, 2 , 1, 1);

  GtkAdjustment *minute_adj = gtk_adjustment_new (0, 0, 59, 1, 10, 0);
  GtkWidget *minuteSpin = gtk_spin_button_new (GTK_ADJUSTMENT (minute_adj), 1, 0);
  gtk_widget_show (minuteSpin);
  gtk_grid_attach (GTK_GRID(gridDialog), minuteSpin, 1, 2 , 1, 1);

  GtkAdjustment *second_adj = gtk_adjustment_new (0, 0, 59, 1, 10, 0);
  GtkWidget *secondSpin = gtk_spin_button_new (GTK_ADJUSTMENT (second_adj), 1, 0);
  gtk_widget_show (secondSpin);
  gtk_grid_attach (GTK_GRID(gridDialog), secondSpin, 2, 2 , 1, 1);

  
  /* buttons */
 

  GLADE_HOOKUP_OBJECT_NO_REF (goJumpDialog, goJumpDialog, "goJumpDialog");
  GLADE_HOOKUP_OBJECT (goJumpDialog, hourSpin, "hourSpin");
  GLADE_HOOKUP_OBJECT (goJumpDialog, minuteSpin, "minuteSpin");
  GLADE_HOOKUP_OBJECT (goJumpDialog, secondSpin, "secondSpin");

  return goJumpDialog;
}

/*************************************
  About dialog

*************************************/
GtkWidget *create_aboutRedac (APP_data *data_app)
{
  GtkWidget *dialog;
  GdkPixbuf *aboutRedac_icon_pixbuf;
  const gchar *authors[] = {_("Project Manager:"),"Luc A. <amiluc_bis@yahoo.fr>", NULL };
  const gchar *artists[] = {"Luc A. <amiluc_bis@yahoo.fr>", _("Redac logos + main icon :"), "Luc A. <amiluc_bis@yahoo.fr>", NULL};

  dialog = gtk_about_dialog_new ();
  gtk_about_dialog_set_program_name (GTK_ABOUT_DIALOG(dialog),"");
  aboutRedac_icon_pixbuf = create_pixbuf ("splash.png");
  if (aboutRedac_icon_pixbuf) {
      gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), aboutRedac_icon_pixbuf);
      g_object_unref(G_OBJECT(aboutRedac_icon_pixbuf ));
  }
  //gtk_about_dialog_set_version (GTK_ABOUT_DIALOG(dialog),"");
  gtk_about_dialog_set_copyright (GTK_ABOUT_DIALOG(dialog), "");

  gtk_about_dialog_set_version (GTK_ABOUT_DIALOG(dialog), g_strdup_printf("%s\n%s", PACKAGE_VERSION, "2022-05"));/* PACKAGE version from config.h */  
  
  gtk_about_dialog_set_comments (GTK_ABOUT_DIALOG(dialog), 
     _("Note  utility written in GTK+ and licensed under GPL v.3"));
  gtk_about_dialog_set_website (GTK_ABOUT_DIALOG(dialog), 
      "https://github.com/Luc-Ami/redac/wiki");
  gtk_about_dialog_set_license_type (GTK_ABOUT_DIALOG(dialog), GTK_LICENSE_GPL_3_0);
  gtk_about_dialog_set_license (GTK_ABOUT_DIALOG(dialog),"GPL V3.0");  

  gtk_about_dialog_set_authors (GTK_ABOUT_DIALOG(dialog), authors);
  gtk_about_dialog_set_artists (GTK_ABOUT_DIALOG(dialog), artists);
  gtk_about_dialog_set_translator_credits (GTK_ABOUT_DIALOG(dialog), _("Translators credits"));

  gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW(data_app->appWindow));                               
  gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_type_hint (GTK_WINDOW (dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_widget_show_all (dialog);
  return dialog;
}

/*********************************
  Redac preparation for a standard
  g_application

**********************************/
void redac_prepare_GUI (GApplication *app, APP_data *data)
{
  GdkRGBA color;
  GtkStack *stack;
  GtkStackSwitcher *switcher;
  GtkWidget *mainWindow, *vGrid;
  GtkWidget *scrolledwindow1, *scrolledwindowPDF;
  GtkWidget *viewportPDF, *crPDF;
  GtkWidget *scrolledwindowCrobar, *toolbar;
  GtkWidget *view, *viewPDF;
  GtkWidget *viewCrobar, *crCrobar;
  GtkWidget *gridStatusBar;
  GKeyFile *keyString;
  GtkTextBuffer *buffer;
  time_t rawtime;
  gchar *path_to_file, buffer_date[81];

  mainWindow =  UI_main_window (app, data);
  gtk_widget_show (GTK_WIDGET(mainWindow));
  vGrid  = GTK_WIDGET(gtk_builder_get_object (data->builder, "vGrid"));
  gtk_widget_show (GTK_WIDGET(vGrid));

  /* guess the style for current theme */
  check_up_theme (mainWindow, data);
  /* true headerBar is in Glade main.ui file */

  /* gtkstack definitions and building */

  stack = GTK_STACK(GTK_WIDGET (gtk_builder_get_object (data->builder, "stack")));
  switcher = GTK_STACK_SWITCHER (GTK_WIDGET(gtk_builder_get_object (data->builder, "switcher")));  
  gtk_stack_switcher_set_stack (switcher, stack);
  
  /* toolbar */
  toolbar = main_wp_toolbar (mainWindow, data);

  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow1);

  /* and a PDF scrolling window ! */
  scrolledwindowPDF = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindowPDF);
  viewportPDF = gtk_viewport_new (NULL,NULL);
  crPDF = PDF_prepare_drawable();
  gtk_container_add (GTK_CONTAINER(viewportPDF), crPDF);  
  gtk_container_add (GTK_CONTAINER(scrolledwindowPDF), viewportPDF);

  /* and a paint/draw/hand notes scrolling window ! */
  scrolledwindowCrobar = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindowCrobar);
  viewCrobar = gtk_viewport_new (NULL, NULL);
  crCrobar = sketch_prepare_drawable ();
  gtk_container_add (GTK_CONTAINER(viewCrobar), crCrobar);  
  gtk_container_add (GTK_CONTAINER(scrolledwindowCrobar), viewCrobar);

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow1),GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindowPDF), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindowPDF),GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindowCrobar), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindowCrobar),GTK_SHADOW_ETCHED_IN);

  /* main Gtk text view */
  view = prepare_view ();
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), view);

  /* place stack */ 
  gtk_stack_add_titled (stack, GTK_WIDGET(scrolledwindow1), "Note", _("Notes"));
  gtk_stack_add_titled (stack, GTK_WIDGET(scrolledwindowPDF), "Refe", _("Reference"));
  gtk_stack_add_titled (stack, GTK_WIDGET(scrolledwindowCrobar), "Sket", _("Sketch"));
  /* set-up default font for text view */
  set_up_view (mainWindow, data);
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(view));

  misc_setup_text_buffer_tags (buffer);

  /* two statusbars arranged in a GtkGrid */
//  gridStatusBar = gtk_grid_new();

  gridStatusBar = GTK_WIDGET(gtk_builder_get_object (data->builder, "gridStatusBar"));
  /* statusbar */
  UI_statusbar (mainWindow, gridStatusBar, data );  
  /* mimic nice page jumper of Evince */
  UI_pdf_page_widget (mainWindow, gridStatusBar, data);

  gtk_widget_show_all(mainWindow);

  /* quasi global vars  */
  misc_init_vars (data);
  data->stack = stack;
  data->appWindow = mainWindow;
  data->PDFdrawable = crPDF;
  data->SketchDrawable = crCrobar;
  data->PDFScrollable = scrolledwindowPDF;
  data->SketchScrollable = scrolledwindowCrobar;
  data->buffer = buffer;
  data->view = GTK_TEXT_VIEW(view);
  data->pipeline = gst_element_factory_make ("playbin", "redac");
  /* spell checker !! after vars init for view value */
  misc_init_spell_checker (data);

  cairo_surface_destroy (data->Sketchsurface);
  data->Sketchsurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, CROBAR_VIEW_MAX_WIDTH, CROBAR_VIEW_MAX_HEIGHT);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (mainWindow, mainWindow, "mainWindow");
  GLADE_HOOKUP_OBJECT (mainWindow, vGrid, "vGrid");
  GLADE_HOOKUP_OBJECT (mainWindow, scrolledwindow1, "scrolledwindow1");
  GLADE_HOOKUP_OBJECT (mainWindow, scrolledwindowPDF, "scrolledwindowPDF");
  GLADE_HOOKUP_OBJECT (mainWindow, scrolledwindowCrobar, "scrolledwindowCrobar");
  GLADE_HOOKUP_OBJECT (mainWindow, crPDF, "crPDF");
  GLADE_HOOKUP_OBJECT (mainWindow, crCrobar, "crCrobar");
  GLADE_HOOKUP_OBJECT (mainWindow, toolbar, "toolbar");
  GLADE_HOOKUP_OBJECT (mainWindow, gridStatusBar, "gridStatusBar");
  GLADE_HOOKUP_OBJECT (mainWindow, view, "view");
  GLADE_HOOKUP_OBJECT (mainWindow, buffer, "buffer");

  /* we parse datas from config file */

  /* we get the configuration file */
  data->gConfigFile = g_build_filename(g_get_user_config_dir (), 
                       "/redac/", KILOWRITER_CONFIG, NULL); /* Create hidden directory to store Kilowriter data */
  /* we check if the directory already exists */
  if(!g_file_test (data->gConfigFile, G_FILE_TEST_EXISTS)) {
     printf ("* config.ini file absent or corrupted ! *\n");
     /* we create the directory */
     gint i = g_mkdir (g_strdup_printf ("%s/redac/", g_get_user_config_dir ()), S_IRWXU);/* i's a macro from gstdio.h */
  }

  createGKeyFile (data, mainWindow);
  /* we get the saved window geometry see : https://gist.github.com/zdxerr/709169  */

  keyString = g_object_get_data (G_OBJECT(mainWindow), "config");
  /* same for application */
  data->keystring       = keyString;
  data->iAudioSmartRew  = g_key_file_get_double (keyString, "application", "audio-file-rewind-step", NULL);
  data->iAudioSmartJump = g_key_file_get_double (keyString, "application", "audio-file-marks-step", NULL);

  misc_set_font_color_settings (data);

  /* we paint sketch background now */
  sketch_prepare (data);

  /* reload last document */
  gchar *s1 = g_key_file_get_string (keyString, "application", "current-file", NULL);
  if(load_gtk_rich_text (s1, buffer, mainWindow, data)!=0) {
     misc_clear_text (buffer, "left");
     printf ("* Redac :  can't reload last work or it's the first start of this software ! *\n");
     /* the default filename is built inside gKeyfile if it isn"t already exists ! */
     /* then we must add a default file name for default path ; in other case, we'll get a segfault */
     /* we get the current date */
     time (&rawtime);
     strftime( buffer_date, 80, "%c", localtime (&rawtime));/* don't change parameter %x */
     /* now we set-up a new default filename */
     path_to_file =  get_path_to_datas_file (buffer_date);
     gtk_label_set_markup (GTK_LABEL(GTK_WIDGET(gtk_builder_get_object (data->builder, "labelMainTitle"))),
                             g_strdup_printf (_("<small><b>%s</b></small>"), path_to_file));
     
     /* rearrange list of recent files */
     rearrange_recent_file_list (keyString);
     /* we change the default values for gkeyfile */
     store_current_file_in_keyfile (keyString, path_to_file, "[...]");
  }
  else {
     store_current_file_in_keyfile (keyString, s1, misc_get_extract_from_document (data));  
  }
  g_free (s1);

  /*  reload last PDF file ? */
  gtk_widget_hide (lookup_widget(GTK_WIDGET(data->appWindow),"image_pdf_modif"));
  s1 = g_key_file_get_string (keyString, "application", "current-PDF-file", NULL);
  if(g_key_file_get_boolean (keyString, "application", "autoreload-PDF",NULL )) {
     if(g_file_test (s1, G_FILE_TEST_EXISTS) ){
        /* reset default PDF zoom ratio */
        data->PDFratio   = g_key_file_get_double (keyString, "reference-document", "zoom", NULL);
        data->curPDFpage = g_key_file_get_integer (keyString, "reference-document", "page", NULL);
        quick_load_PDF (s1, data);
     }
  }
  g_free (s1);

  update_statusbar (buffer, data);

 /* we preset the cursor */
  gtk_widget_grab_focus (GTK_WIDGET(view));

 /* callbacks */
 
  g_signal_connect (buffer, "changed",
        G_CALLBACK(update_statusbar), data);

  g_signal_connect (buffer, "mark_set", 
        G_CALLBACK(mark_set_callback), data);

  g_signal_connect (view, "cut-clipboard", 
        G_CALLBACK(cut_to_clipboard), data);

  g_signal_connect (view, "copy-clipboard", 
        G_CALLBACK(copy_to_clipboard), data);

  g_signal_connect (view, "paste-clipboard", 
        G_CALLBACK(paste_clipboard), data);

  g_signal_connect (view, "backspace", 
        G_CALLBACK(backspace), data);

  g_signal_connect (view, "delete-from-cursor", 
        G_CALLBACK(delete), data);

  g_signal_connect (G_OBJECT(mainWindow), "delete_event",
        G_CALLBACK(on_close_window_clicked), data);
  /* keypress event in order to catch shortcuts WITHOUT widgets */
  g_signal_connect (G_OBJECT(mainWindow), "key-press-event", 
                   G_CALLBACK(key_event), data);
  
  g_signal_connect (scrolledwindow1, "size-allocate", G_CALLBACK (ScrollToEnd), data);
  
  g_signal_connect (G_OBJECT(scrolledwindowPDF), "size-allocate",
                  G_CALLBACK (on_PDF_size_changed),
                  data);

  g_signal_connect (G_OBJECT(scrolledwindowPDF), "scroll-event",
                  G_CALLBACK (on_PDF_scroll_event),
                  data);
  
  g_signal_connect (G_OBJECT(stack), "notify::visible-child",
                  G_CALLBACK (on_stack_changed),
                  data);
  g_signal_connect (G_OBJECT(crPDF), "draw",
					 G_CALLBACK(draw_callback), data); 
 /* events to build selection rectangles on the PDF renderer */
  g_signal_connect (G_OBJECT(crPDF), "button-press-event",
					 G_CALLBACK(on_PDF_draw_button_press_callback), data);
  g_signal_connect (G_OBJECT(crPDF), "button-release-event",
					 G_CALLBACK(on_PDF_draw_button_release_callback), data);
  g_signal_connect (G_OBJECT(crPDF), "motion-notify-event",
					 G_CALLBACK(on_PDF_draw_motion_event_callback), data);
  /* events for sketch area */
  g_signal_connect (G_OBJECT(crCrobar), "draw",
					 G_CALLBACK(sketch_draw_callback), data); 
  g_signal_connect (G_OBJECT(crCrobar), "button-press-event",
					 G_CALLBACK(on_sketch_draw_button_press_callback), data);
  g_signal_connect (G_OBJECT(crCrobar), "button-release-event",
					 G_CALLBACK(on_sketch_draw_button_release_callback), data);
  g_signal_connect (G_OBJECT(crCrobar), "motion-notify-event",
					 G_CALLBACK(on_sketch_draw_motion_event_callback), data);

  /* enter/leave mouse events for drawings */

  g_signal_connect (G_OBJECT(scrolledwindowCrobar), "enter-notify-event", G_CALLBACK (on_sketch_draw_enter_event_callback), data);
  g_signal_connect (G_OBJECT(scrolledwindowCrobar), "leave-notify-event", G_CALLBACK (on_sketch_draw_leave_event_callback), data);

  /* enter/leave mouse events for PDF */

  g_signal_connect (G_OBJECT(scrolledwindowPDF), "enter-notify-event", G_CALLBACK (on_PDF_enter_event_callback), data);
  g_signal_connect (G_OBJECT(scrolledwindowPDF), "leave-notify-event", G_CALLBACK (on_PDF_leave_event_callback), data);


  misc_set_gui_in_editor_mode (data->appWindow, CURRENT_STACK_EDITOR); 
  /* add timeout for 5 minutes, 300 secs - should be improved in a next release */
  misc_prepare_timeouts (data);

}
