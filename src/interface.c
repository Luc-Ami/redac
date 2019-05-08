#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>

/* translations */
#include <libintl.h>
#include <locale.h>

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <poppler.h>
#include "support.h"
#include "misc.h"
#include "interface.h"
#include "callbacks.h"
#include "mttfiles.h"
#include "mttimport.h"
#include "audio.h"
/* pixmaps */
#include "./icons/superscript.xpm"
#include "./icons/superscript_light.xpm"
#include "./icons/subscript.xpm"
#include "./icons/subscript_light.xpm"
#include "./icons/highlight.xpm"
#include "./icons/quotation.xpm"
#include "./icons/quotation_light.xpm"
#include "./icons/star_highlight.xpm"
#include "./icons/star_highlight_light.xpm"
#include "./icons/pencil.xpm"
#include "./icons/pencil_light.xpm"
#include "./icons/text_select.xpm"
#include "./icons/text_select_light.xpm"
#include "./icons/text_note.xpm"
#include "./icons/text_note_light.xpm"
#include "./icons/pdf.xpm"

/*********************************
  setup default values for display
for paddings and removing inner border from entries, many thanks to them : https://github.com/jy2wong/luakit/commit/6dfffb296f562b26c1eb6020c94b1d3e0bde336b
anf for a clean demo for gtktextview tuning : https://gist.github.com/mariocesar/f051ce1dda4ef0041aed
************************************/
void set_up_view( GtkWidget *window1, APP_data *data_app )
{
  guint major, minor;
  GtkCssProvider* css_provider = gtk_css_provider_new();

  /* check Gtk 3.x version */
  major=gtk_get_major_version ();
  minor=gtk_get_minor_version ();

  const char css[] = 
"  #labelHitsFrame { border-top-left-radius: 6px; border-bottom-left-radius: 6px;border-top-right-radius: 6px; border-bottom-right-radius: 6px; border-style: solid; border-width: 2px; padding: 0px; }\n"
"  #search_entry { border: none; padding: 4px; }\n"
"  #page_frame { border-top-left-radius: 6px; border-bottom-left-radius: 6px;border-top-right-radius: 6px; border-bottom-right-radius: 6px; border-style: solid; border-width: 2px; padding: 2px; }\n"
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
"  #view  { color: pink; background-color: #FFFFE0;}\n"
"  #view:selected, #view:selected:focus { background-color: blue; color:red; }\n"

"  Label#PDF_modified_label { border-top-left-radius: 6px; border-bottom-left-radius: 6px;border-top-right-radius: 6px; border-bottom-right-radius: 6px; border-style: solid; border-width: 6px; }\n";
  gtk_css_provider_load_from_data(css_provider,css,-1,NULL);
 /*----- css *****/
  GdkScreen* screen = gdk_screen_get_default();
  gtk_style_context_add_provider_for_screen (screen,GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

/********************************
  check if we have a dark or 
  light theme
*********************************/
void check_up_theme( GtkWidget *window1, APP_data *data_app )
{
  GdkRGBA fg, bg;
  gdouble textAvg, bgAvg;
 
  GdkScreen* screen = gdk_screen_get_default();
  GtkStyleContext* style_context = gtk_widget_get_style_context(window1);

 /* check if we have a drak or light theme idea from : 
https://lzone.de/blog/Detecting%20a%20Dark%20Theme%20in%20GTK  
*/
  gtk_style_context_get_color (style_context, GTK_STATE_FLAG_NORMAL, &fg);
  gtk_style_context_get_background_color (style_context, GTK_STATE_FLAG_NORMAL, &bg);

  textAvg = fg.red+fg.green+fg.blue;
  bgAvg = bg.red+bg.green+bg.blue;

  if (textAvg > bgAvg)  {
     data_app->fDarkTheme=TRUE;
  }
  else {
     data_app->fDarkTheme=FALSE;
  }
}

/*****************************
 retrieve theme's selection
 color
****************************/
void get_theme_selection_color(GtkWidget *widget)
{
  GdkRGBA  color1, color2;

  GdkScreen* screen = gdk_screen_get_default();
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
static gboolean widget_is_dark(GtkWidget *widget) 
{
  GdkRGBA fg, bg;
  gdouble textAvg;
 
  GdkScreen* screen = gdk_screen_get_default();
  GtkStyleContext* style_context = gtk_widget_get_style_context(widget);

  /* check the TEXT color is the best way, since a background can be an image, not a text !*/
  gtk_style_context_get_color (style_context, GTK_STATE_FLAG_NORMAL, &fg);
  
  textAvg = fg.red+fg.green+fg.blue;

  if (textAvg >1.5)  
     return TRUE;
  else 
     return FALSE;
}

/*******************************

  right click on PDF surface
  Pop-Up menu
********************************/

GtkWidget *create_menu_PDF(GtkWidget *win, APP_data *data_app)
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

  menu1PDFColorAnnot= gtk_menu_item_new_with_mnemonic (_("Annotation _color ... "));
  gtk_widget_show (menu1PDFColorAnnot);
  gtk_container_add (GTK_CONTAINER (menu1PDF), menu1PDFColorAnnot);

  menu1PDFRemoveAnnot= gtk_menu_item_new_with_mnemonic (_("_Remove annotation "));
  gtk_widget_show (menu1PDFRemoveAnnot);
  gtk_container_add (GTK_CONTAINER (menu1PDF), menu1PDFRemoveAnnot);

  separator1 = gtk_separator_menu_item_new ();
  gtk_widget_show (separator1);
  gtk_container_add (GTK_CONTAINER (menu1PDF), separator1);
  gtk_widget_set_sensitive (separator1, FALSE);

  menuCancelPDF = gtk_menu_item_new_with_mnemonic (_("C_ancel"));
  gtk_widget_show (menuCancelPDF);
  gtk_container_add (GTK_CONTAINER (menu1PDF), menuCancelPDF);

  if(data_app->current_annot==NULL) {
    /* we deactivate menus since there isn't any annotation on current page */
    gtk_widget_set_sensitive(menu1PDFEditAnnot, FALSE);
    gtk_widget_set_sensitive(menu1PDFColorAnnot, FALSE);
    gtk_widget_set_sensitive(menu1PDFRemoveAnnot, FALSE);
  }
  else {/* we check if it's a text annot */
    if( poppler_annot_get_annot_type (data_app->current_annot)  !=POPPLER_ANNOT_TEXT && 
            poppler_annot_get_annot_type (data_app->current_annot)  !=POPPLER_ANNOT_FREE_TEXT)
           gtk_widget_set_sensitive(menu1PDFEditAnnot, FALSE);
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

GtkWidget *create_menu_sketch(GtkWidget *win, APP_data *data_app)
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

  menuCenteredPasteSketch= gtk_menu_item_new_with_mnemonic (_("Paste and _center image "));
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
/*******************
  PopUp Menu
********************/
GtkWidget*
create_menu1 (GtkWidget *win, APP_data *data_app)
{
  GtkWidget *menu1;
  GtkWidget *submenu1;
  GtkWidget *submenu2;
  GtkWidget *submenu3;
  GtkWidget *submenu4;
  GtkWidget *submenu5;
  GtkWidget *submenu6;
  GtkWidget *new1;
  GtkWidget *boxnew1;
  GtkWidget *lblnew1;
  GtkWidget *image58;
  GtkWidget *save3;
  GtkWidget *boxsave3;
  GtkWidget *lblsave3;
  GtkWidget *image59;
  GtkWidget *open2;
  GtkWidget *lblopen2;
  GtkWidget *boxopen2;
  GtkWidget *image60;
  GtkWidget *menuPDF;
  GtkWidget *lblPDF;
  GtkWidget *boxPDF;
  GtkWidget *imagePDF;
  GtkWidget *loadPDF;
  GtkWidget *savePDF;
  GtkWidget *imageSketch;
  GtkWidget *lblSketch;
  GtkWidget *boxSketch;
  GtkWidget *saveSketch;
  GtkWidget *clearSketch;
  GtkWidget *manageImage;
  GtkWidget *menuAudio;
  GtkWidget *lblAudio;
  GtkWidget *boxAudio;
  GtkWidget *imageMenuAudio;
  GtkWidget *AudioLoad;
  GtkWidget *AudioCloseFile;
  GtkWidget *recent1;
  GtkWidget *lblrecent1;
  GtkWidget *boxrecent1;
  GtkWidget *import1;
  GtkWidget *lblimport1;
  GtkWidget *boximport1;
  GtkWidget *misc1;
  GtkWidget *about1;
  GtkWidget *keyHelp1;
  GtkWidget *wiki1;
  GtkWidget *imageHelp;
  GtkWidget *boxHelp;
  GtkWidget *lblHelp;
  GtkWidget *insertWPFile;
  GtkWidget *insertRedacFile;
  GtkWidget *imageImport1;
  GtkWidget *image61;
  GtkWidget *separator14;
  GtkWidget *separator1;
  GtkWidget *separator2;
  GtkWidget *separator3;
  GtkWidget *recent[MAX_RECENT_FILES];
  gint i;

  GKeyFile *keyString;
  gchar *tmpStr;
  gboolean fActivMenu;


  keyString=data_app->keystring;
  if(keyString==NULL) {
     printf("* INTERNAL ERROR in module interface.c *\n");
     return NULL;
  }
  menu1 = gtk_menu_new ();  
  gtk_widget_set_name(GTK_WIDGET(menu1),"menu1");/* for css */
  /* new complex Gtk 3.10+ way to set-up an image menu ;( */
  lblnew1 = gtk_label_new  (_("New note "));
  boxnew1=gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  image58 = gtk_image_new_from_icon_name  ("gtk-new", GTK_ICON_SIZE_MENU);
  new1=gtk_menu_item_new ();
  gtk_container_add (GTK_CONTAINER (boxnew1), image58);
  gtk_container_add (GTK_CONTAINER (boxnew1), lblnew1);
  gtk_container_add (GTK_CONTAINER (new1), boxnew1);
  gtk_widget_show_all (new1);
  gtk_container_add (GTK_CONTAINER (menu1), new1);
  /* save as */
  lblsave3 = gtk_label_new (_("Save the note as ...  "));
  image59 = gtk_image_new_from_icon_name  ("gtk-save-as", GTK_ICON_SIZE_MENU);
  boxsave3=gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  save3=gtk_menu_item_new ();
  gtk_container_add (GTK_CONTAINER (boxsave3), image59);
  gtk_container_add (GTK_CONTAINER (boxsave3), lblsave3);
  gtk_container_add (GTK_CONTAINER (save3), boxsave3);
  gtk_widget_show_all (save3);
  gtk_container_add (GTK_CONTAINER (menu1), save3);
  /* open */
  lblopen2 = gtk_label_new (_("Open a previous note ..."));
  image60 = gtk_image_new_from_icon_name ("gtk-open", GTK_ICON_SIZE_MENU);
  boxopen2=gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  open2=gtk_menu_item_new ();
  gtk_container_add (GTK_CONTAINER (boxopen2), image60);
  gtk_container_add (GTK_CONTAINER (boxopen2), lblopen2);
  gtk_container_add (GTK_CONTAINER (open2), boxopen2);
  gtk_widget_show_all (open2);
  gtk_container_add (GTK_CONTAINER (menu1), open2);
  /* separator */
  separator1 = gtk_separator_menu_item_new ();
  gtk_widget_show (separator1);
  gtk_container_add (GTK_CONTAINER (menu1), separator1);
  gtk_widget_set_sensitive (separator1, FALSE);
  /* recent */
  lblrecent1 = gtk_label_new (_("Recent notes ..."));
  image61 = gtk_image_new_from_icon_name ("document-open-recent", GTK_ICON_SIZE_MENU);
  boxrecent1=gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  recent1=gtk_menu_item_new ();
  gtk_container_add (GTK_CONTAINER (boxrecent1), image61);
  gtk_container_add (GTK_CONTAINER (boxrecent1), lblrecent1);
  gtk_container_add (GTK_CONTAINER (recent1), boxrecent1);
  gtk_widget_show_all (recent1);
  gtk_container_add (GTK_CONTAINER (menu1), recent1);
 
  /* we update summary of current file */
  store_current_file_in_keyfile(keyString, g_key_file_get_string(keyString, "history","recent-file-0", NULL) , 
                                 misc_get_extract_from_document(data_app));

  /* separator */
  separator2 = gtk_separator_menu_item_new ();
  gtk_widget_show (separator2);
  gtk_container_add (GTK_CONTAINER (menu1), separator2);
  gtk_widget_set_sensitive (separator2, FALSE);
  /* import */
  imageImport1=gtk_image_new_from_icon_name ("insert-text", GTK_ICON_SIZE_MENU);
  lblimport1= gtk_label_new (_("Insert text from a file "));
  boximport1=gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  import1=gtk_menu_item_new ();
  gtk_container_add (GTK_CONTAINER (boximport1), imageImport1);
  gtk_container_add (GTK_CONTAINER (boximport1), lblimport1);
  gtk_container_add (GTK_CONTAINER (import1), boximport1);
  gtk_widget_show_all (import1);
  gtk_container_add (GTK_CONTAINER (menu1), import1);
  /* insert/import submenu */
  submenu5 = gtk_menu_new ();
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(import1), submenu5);
  insertWPFile=gtk_menu_item_new_with_mnemonic (_("from a RTF file ..."));
  gtk_widget_show (insertWPFile);
  gtk_container_add (GTK_CONTAINER (submenu5), insertWPFile);

  insertRedacFile=gtk_menu_item_new_with_mnemonic (_("Merge with a _note ..."));
  gtk_widget_show (insertRedacFile);
  gtk_container_add (GTK_CONTAINER (submenu5), insertRedacFile);

  separator14 = gtk_separator_menu_item_new ();
  gtk_container_add (GTK_CONTAINER (menu1), separator14);
  gtk_widget_set_sensitive (separator14, FALSE);
  /* PDF */
  imagePDF = gtk_image_new_from_icon_name ("application-pdf", GTK_ICON_SIZE_MENU);
  lblPDF=gtk_label_new ("PDF ");
  boxPDF=gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  menuPDF=gtk_menu_item_new ();
  gtk_container_add (GTK_CONTAINER (boxPDF), imagePDF);
  gtk_container_add (GTK_CONTAINER (boxPDF), lblPDF);
  gtk_container_add (GTK_CONTAINER (menuPDF), boxPDF);
  gtk_widget_show_all (menuPDF);
  gtk_container_add (GTK_CONTAINER (menu1), menuPDF);

 /*  submenu PDF >> submenu2 */
  submenu2 = gtk_menu_new ();
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuPDF), submenu2);
  loadPDF=gtk_menu_item_new_with_mnemonic (_("Load _PDF file ..."));
  gtk_widget_show (loadPDF);
  gtk_container_add (GTK_CONTAINER (submenu2), loadPDF);

  savePDF=gtk_menu_item_new_with_mnemonic (_("Save _modifyed PDF file ..."));
  gtk_widget_show (savePDF);
  gtk_container_add (GTK_CONTAINER (submenu2), savePDF);
  gtk_widget_set_sensitive (savePDF, data_app->fPdfLoaded);

  /* audio menu */
  imageMenuAudio = gtk_image_new_from_icon_name ("audio-input-microphone", GTK_ICON_SIZE_MENU);
  lblAudio=gtk_label_new ("Audio");
  boxAudio=gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  menuAudio=gtk_menu_item_new ();
  gtk_container_add (GTK_CONTAINER (boxAudio), imageMenuAudio);
  gtk_container_add (GTK_CONTAINER (boxAudio), lblAudio);
  gtk_container_add (GTK_CONTAINER (menuAudio), boxAudio);
  gtk_widget_show_all (menuAudio);
  gtk_container_add (GTK_CONTAINER (menu1), menuAudio);  

  /* submenu4 = submenu for Audio */
  submenu4 = gtk_menu_new ();
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuAudio), submenu4);
  AudioLoad=gtk_menu_item_new_with_mnemonic (_("Load A_udio file ..."));
  gtk_widget_show (AudioLoad);
  gtk_container_add (GTK_CONTAINER (submenu4), AudioLoad);
  AudioCloseFile=gtk_menu_item_new_with_mnemonic (_("_Close current Audio file"));
  gtk_widget_show (AudioCloseFile);
  gtk_container_add (GTK_CONTAINER (submenu4), AudioCloseFile);
  gtk_widget_set_sensitive (AudioCloseFile, data_app->fAudioLoaded);

  /* sketches - images */
  imageSketch=gtk_image_new_from_icon_name ("image-x-generic", GTK_ICON_SIZE_MENU);
  boxSketch=gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  lblSketch=gtk_label_new (_("Sketches"));
  manageImage=gtk_menu_item_new ();
  gtk_container_add (GTK_CONTAINER (boxSketch), imageSketch);
  gtk_container_add (GTK_CONTAINER (boxSketch), lblSketch);
  gtk_container_add (GTK_CONTAINER (manageImage), boxSketch);
  gtk_widget_show_all (manageImage);
  gtk_container_add (GTK_CONTAINER (menu1), manageImage);
  /* submenu3 = sketches */
  submenu3 = gtk_menu_new ();
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(manageImage), submenu3);
  saveSketch=gtk_menu_item_new_with_mnemonic (_("Save current s_ketch ..."));
  gtk_widget_show (saveSketch);
  gtk_container_add (GTK_CONTAINER (submenu3), saveSketch);
  clearSketch=gtk_menu_item_new_with_mnemonic (_("Clear current sketch"));
  gtk_widget_show (clearSketch);
  gtk_container_add (GTK_CONTAINER (submenu3), clearSketch);
  if(data_app->currentStack==CURRENT_STACK_SKETCH) 
     gtk_widget_set_sensitive(clearSketch, TRUE);
  else
     gtk_widget_set_sensitive(clearSketch, FALSE);

  /* separator */
  separator3 = gtk_separator_menu_item_new ();
  gtk_widget_show (separator3);
  gtk_container_add (GTK_CONTAINER (menu1), separator3);
  gtk_widget_set_sensitive (separator3, FALSE);
  /* help */
  imageHelp=gtk_image_new_from_icon_name ("help-contents", GTK_ICON_SIZE_MENU);
  lblHelp=gtk_label_new (_("Help"));
  boxHelp=gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  misc1 = gtk_menu_item_new ();
  gtk_container_add (GTK_CONTAINER (boxHelp), imageHelp);
  gtk_container_add (GTK_CONTAINER (boxHelp), lblHelp);
  gtk_container_add (GTK_CONTAINER (misc1), boxHelp);
  gtk_widget_show_all (misc1);
  gtk_container_add (GTK_CONTAINER (menu1), misc1);
  /* submenu6 = misc-help */
  submenu6 = gtk_menu_new ();
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(misc1), submenu6);
  keyHelp1=gtk_menu_item_new_with_mnemonic (_("_Keyboard shortcuts ..."));
  gtk_widget_show (keyHelp1);
  gtk_container_add (GTK_CONTAINER (submenu6), keyHelp1);
  wiki1=gtk_menu_item_new_with_mnemonic (_("Online _Wiki ..."));
  gtk_widget_show (wiki1);
  gtk_container_add (GTK_CONTAINER (submenu6), wiki1);
  about1=gtk_menu_item_new_with_mnemonic (_("_About Redac ..."));
  gtk_widget_show (about1);
  gtk_container_add (GTK_CONTAINER (submenu6), about1);

 
  g_signal_connect ((gpointer) new1, "activate",
                    G_CALLBACK (new_project),
                    data_app);
  g_signal_connect ((gpointer) save3, "activate",
                    G_CALLBACK (save_standard_file),
                    data_app);
  g_signal_connect ((gpointer) open2, "activate",
                    G_CALLBACK (on_open_clicked),
                    data_app);
  g_signal_connect ((gpointer) recent1, "activate",
                    G_CALLBACK (menuitem_response),
                    data_app);

  g_signal_connect ((gpointer) insertWPFile, "activate",
                    G_CALLBACK (on_import_clicked),
                    data_app);
  g_signal_connect ((gpointer) insertRedacFile, "activate",
                    G_CALLBACK (on_merge_clicked),
                    data_app);

  g_signal_connect ((gpointer) loadPDF, "activate",
                    G_CALLBACK (on_loadPDF_clicked),
                    data_app);
  g_signal_connect ((gpointer) savePDF, "activate",
                    G_CALLBACK (on_savePDF_clicked),
                    data_app);

  g_signal_connect ((gpointer) clearSketch, "activate",
                    G_CALLBACK (on_clearSketch_clicked),
                    data_app);
  g_signal_connect ((gpointer) saveSketch, "activate",
                    G_CALLBACK (on_saveSketch_clicked),
                    data_app);


  g_signal_connect ((gpointer) AudioLoad, "activate",
                    G_CALLBACK (on_loadAudio_clicked),
                    data_app);
  g_signal_connect ((gpointer) AudioCloseFile, "activate",
                    G_CALLBACK (on_AudioCloseFile_clicked),
                    data_app);
  g_signal_connect ((gpointer) about1, "activate",
                    G_CALLBACK (on_about1_activate),
                    data_app);
  g_signal_connect ((gpointer) keyHelp1, "activate",
                    G_CALLBACK (on_keyHelp1_activate),
                    data_app);
  g_signal_connect ((gpointer) wiki1, "activate",
                    G_CALLBACK (on_wiki1_activate),
                    data_app);


  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (menu1, menu1, "menu1");
  GLADE_HOOKUP_OBJECT (menu1, new1, "new1");
  GLADE_HOOKUP_OBJECT (menu1, import1, "import1");
  GLADE_HOOKUP_OBJECT (menu1, save3, "save3");
  GLADE_HOOKUP_OBJECT (menu1, open2, "open2");
  GLADE_HOOKUP_OBJECT (menu1, recent1, "recent1");
  return menu1;
}


/**********************************
   set up the toolbar
***********************************/
GtkWidget *main_wp_toolbar(GtkWidget *window, APP_data *data_app)
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
  GtkWidget *icon_superscript;
  GtkWidget *icon_subscript;
  GtkWidget *icon_strike;
  GtkWidget *icon_highlight;
  GtkWidget *icon_quotation;
  /* alignment tools */
  GtkWidget *icon_left_format;
  GtkWidget *icon_center_format;
  GtkWidget *icon_right_format;
  GtkWidget *icon_fill_format;
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
  GtkToolItem *pRadioButtonHiAnnotSelect;
  GtkWidget *icon_text_select;
  GtkWidget *icon_picture_select;
  GtkWidget *icon_highlight_select;
  GtkWidget *icon_text_annot;
  GtkWidget *icon_pencil;
  GtkToolItem *button_pencil;
  GtkToolItem *color_button_item;
  GtkWidget *color_button;
  GtkAccelGroup *accel_group;
  gboolean fIsDark=FALSE;
  /* audio section */
  GtkToolItem *audioSeperator;
  GtkToolItem *pRadioButtonPlayPauseAudio;
  GtkToolItem *pRadioButtonRewindAudio;
  GtkToolItem *pRadioButtonGotoAudio;
  GtkToolItem *pRadioButtonGoJumpAudio;
  GtkWidget *AudioDisplayCounter;
  GtkWidget *iconButtonPlayAudio;
  GtkWidget *iconButtonPauseAudio;
  GtkWidget *iconButtonHomeAudio;
  GtkWidget *iconButtonGotoAudio;
  GtkWidget *iconButtonGoJumpAudio;

  toolbar = gtk_toolbar_new();
  gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
  gtk_widget_set_margin_top(toolbar, 8);
  gtk_widget_set_margin_bottom(toolbar, 8);
  gtk_widget_set_margin_left(toolbar, 8);
  gtk_widget_set_margin_right(toolbar, 8);
  fIsDark= widget_is_dark(toolbar);
  /* toolbar toggle buttons bold */
  icon_bold = gtk_image_new_from_icon_name("format-text-bold-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
  icon_italic = gtk_image_new_from_icon_name("format-text-italic-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
  icon_underline = gtk_image_new_from_icon_name("format-text-underline-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
/*
  if(data_app->fDarkTheme)
     ico = gdk_pixbuf_new_from_xpm_data((const char **)clear_format_light_xpm);
  else
     ico = gdk_pixbuf_new_from_xpm_data((const char **)clear_format_xpm);
  icon_clear_format = gtk_image_new_from_pixbuf(ico);
  g_object_unref(ico); 
*/
  icon_clear_format=gtk_image_new_from_icon_name ("edit-clear-all-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);

/*
  if(data_app->fDarkTheme)
     ico = gdk_pixbuf_new_from_xpm_data((const char **)undo_light_xpm);
  else
     ico = gdk_pixbuf_new_from_xpm_data((const char **)undo_xpm);
  icon_undo =gtk_image_new_from_pixbuf(ico);
  g_object_unref(ico);
*/

  icon_undo =gtk_image_new_from_icon_name ("edit-undo-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);

  if( fIsDark)
      ico = gdk_pixbuf_new_from_xpm_data((const char **)superscript_light_xpm);
  else
      ico = gdk_pixbuf_new_from_xpm_data((const char **)superscript_xpm);
  icon_superscript  =gtk_image_new_from_pixbuf(ico);
  g_object_unref(ico); 

  if(fIsDark)
      ico = gdk_pixbuf_new_from_xpm_data((const char **)subscript_light_xpm);
  else
      ico = gdk_pixbuf_new_from_xpm_data((const char **)subscript_xpm);
  icon_subscript=gtk_image_new_from_pixbuf(ico);
  g_object_unref(ico); 

/*
  if(data_app->fDarkTheme)
     ico = gdk_pixbuf_new_from_xpm_data((const char **)strike_light_xpm);
  else
     ico = gdk_pixbuf_new_from_xpm_data((const char **)strike_xpm);
  icon_strike = gtk_image_new_from_pixbuf(ico);
  g_object_unref(ico); 
*/
  icon_strike =gtk_image_new_from_icon_name ("format-text-strikethrough-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);

  ico = gdk_pixbuf_new_from_xpm_data((const char **)highlight_xpm);
  icon_highlight=gtk_image_new_from_pixbuf(ico);
  g_object_unref(ico); 

  if(fIsDark)
     ico = gdk_pixbuf_new_from_xpm_data((const char **)quotation_light_xpm);
  else
     ico = gdk_pixbuf_new_from_xpm_data((const char **)quotation_xpm);
  icon_quotation=gtk_image_new_from_pixbuf(ico);
  g_object_unref(ico); 

  /* edition buttons */
  /* formatting buttons */

  button_bold = gtk_toggle_tool_button_new ();
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button_bold, -1);  
  gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(button_bold),icon_bold);
  gtk_widget_set_tooltip_text(GTK_WIDGET(button_bold), _("Toggle to/from bold either \nfor current selection\nor next typing."));

  button_italic = gtk_toggle_tool_button_new ();
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button_italic, -1);  

  gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(button_italic),icon_italic);
  gtk_widget_set_tooltip_text(GTK_WIDGET(button_italic), _("Toggle to/from italic either \nfor current selection\nor next typing."));
  button_underline = gtk_toggle_tool_button_new ();
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button_underline, -1); 

  gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(button_underline),icon_underline);
  gtk_widget_set_tooltip_text(GTK_WIDGET(button_underline), _("Toggle to/from underline either \nfor current selection\nor next typing."));
  /* other formattings */
   button_superscript = gtk_toggle_tool_button_new ();
   gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button_superscript, -1); 
   gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(button_superscript),icon_superscript);
 gtk_widget_set_tooltip_text(GTK_WIDGET(button_superscript), _("Toggle to/from superscript either \nfor current selection\nor next typing."));
   button_subscript=gtk_toggle_tool_button_new ();
   gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button_subscript, -1);    
   gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(button_subscript),icon_subscript);
 gtk_widget_set_tooltip_text(GTK_WIDGET(button_subscript), _("Toggle to/from subscript either \nfor current selection\nor next typing."));
   button_strikethrough=gtk_toggle_tool_button_new ();
   gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button_strikethrough, -1); 
   gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(button_strikethrough),icon_strike);
 gtk_widget_set_tooltip_text(GTK_WIDGET(button_strikethrough), _("Toggle to/from strikethrough \nfor current selection\nor next typing."));
   button_highlight=gtk_toggle_tool_button_new ();
   gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button_highlight, -1); 
   gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(button_highlight),icon_highlight);
 gtk_widget_set_tooltip_text(GTK_WIDGET(button_highlight), _("Apply highlighting \nfor current selection\nor next typing."));
   button_quotation=gtk_toggle_tool_button_new ();
   gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button_quotation, -1); 
   gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(button_quotation),icon_quotation);
gtk_widget_set_tooltip_text(GTK_WIDGET(button_quotation), _("Toggle to/from quotation style either \nfor current selection\nor next typing."));
  gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON(button_bold), FALSE);
  gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON(button_italic), FALSE);
  gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON(button_underline), FALSE);
 
  /* radiobuttons : thanks to Pascal developers 
   here: https://fr.wikibooks.org/wiki/Programmation_GTK2_en_Pascal/GtkRadioToolButton*/
  pRadioButtonLeft = gtk_radio_tool_button_new(NULL);

  icon_left_format = gtk_image_new_from_icon_name("format-justify-left-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
  icon_center_format = gtk_image_new_from_icon_name("format-justify-center-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
  icon_right_format = gtk_image_new_from_icon_name("format-justify-right-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
  icon_fill_format = gtk_image_new_from_icon_name("format-justify-fill-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);

  GSList *group = NULL; 

  gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(pRadioButtonLeft), icon_left_format);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), pRadioButtonLeft, -1);
  gtk_widget_set_tooltip_text(GTK_WIDGET(pRadioButtonLeft), _("Align to left \nthe whole current paragraph.")); 
  gtk_radio_tool_button_set_group(GTK_RADIO_TOOL_BUTTON(pRadioButtonLeft), group);
  group = gtk_radio_tool_button_get_group(GTK_RADIO_TOOL_BUTTON(pRadioButtonLeft));

  pRadioButtonCenter = gtk_radio_tool_button_new(group);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), pRadioButtonCenter, -1);
  gtk_widget_set_tooltip_text(GTK_WIDGET(pRadioButtonCenter), _("Align to center \nthe whole current paragraph.")); 
  gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(pRadioButtonCenter), icon_center_format);
  gtk_radio_tool_button_set_group(GTK_RADIO_TOOL_BUTTON(pRadioButtonCenter), group);
  group = gtk_radio_tool_button_get_group(GTK_RADIO_TOOL_BUTTON(pRadioButtonCenter));

  pRadioButtonRight = gtk_radio_tool_button_new(group);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), pRadioButtonRight, -1);
  gtk_widget_set_tooltip_text(GTK_WIDGET(pRadioButtonRight), _("Align to right \nthe whole current paragraph.")); 
  gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(pRadioButtonRight), icon_right_format);
  gtk_radio_tool_button_set_group(GTK_RADIO_TOOL_BUTTON(pRadioButtonRight), group);
  group = gtk_radio_tool_button_get_group(GTK_RADIO_TOOL_BUTTON(pRadioButtonRight));

  pRadioButtonFill = gtk_radio_tool_button_new(group);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), pRadioButtonFill, -1);
  gtk_widget_set_tooltip_text(GTK_WIDGET(pRadioButtonFill), _("Justify left AND right \nthe whole current paragraph.")); 
  gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(pRadioButtonFill), icon_fill_format);
  gtk_radio_tool_button_set_group(GTK_RADIO_TOOL_BUTTON(pRadioButtonFill), group);
  group = gtk_radio_tool_button_get_group(GTK_RADIO_TOOL_BUTTON(pRadioButtonFill));

  gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON(pRadioButtonLeft), TRUE);

 
  /* standard button */

  button_clear_format = gtk_tool_button_new (icon_clear_format,NULL);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button_clear_format, -1);  
  gtk_widget_set_tooltip_text(GTK_WIDGET(button_clear_format), _("Remove all character \nformattings from current selection.")); 
  /* undo button */

  button_undo = gtk_tool_button_new (icon_undo,NULL);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button_undo, -1);   
  gtk_widget_set_tooltip_text(GTK_WIDGET(button_undo), _("Undo last operation."));
  gtk_widget_set_sensitive(GTK_WIDGET(button_undo), FALSE);

  standardSeperator=gtk_separator_tool_item_new ();
  gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(standardSeperator), TRUE);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), standardSeperator, -1);

  /* clipboard mode radiobuttons */
  pRadioButtonTextSelect=gtk_radio_tool_button_new(NULL);


  if(fIsDark)
      ico = gdk_pixbuf_new_from_xpm_data((const char **)text_select_light_xpm);
  else
      ico = gdk_pixbuf_new_from_xpm_data((const char **)text_select_xpm);
  icon_text_select  =gtk_image_new_from_pixbuf(ico);
  g_object_unref(ico); 

/*
  if(data_app->fDarkTheme)
      ico = gdk_pixbuf_new_from_xpm_data((const char **)camera_light_xpm);
  else
      ico = gdk_pixbuf_new_from_xpm_data((const char **)camera_xpm);
  icon_picture_select  =gtk_image_new_from_pixbuf(ico);
  g_object_unref(ico); 
*/
icon_picture_select =gtk_image_new_from_icon_name ("camera-photo-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);


  if(fIsDark)
      ico = gdk_pixbuf_new_from_xpm_data((const char **)star_highlight_light_xpm);
  else
      ico = gdk_pixbuf_new_from_xpm_data((const char **)star_highlight_xpm);
  icon_highlight_select=gtk_image_new_from_pixbuf(ico);
  g_object_unref(ico);

  if(fIsDark)
      ico = gdk_pixbuf_new_from_xpm_data((const char **)text_note_light_xpm);
  else
      ico = gdk_pixbuf_new_from_xpm_data((const char **)text_note_xpm);
  icon_text_annot  =gtk_image_new_from_pixbuf(ico);
  g_object_unref(ico); 


  GSList *group_clip = NULL; 
  gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(pRadioButtonTextSelect), icon_text_select);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), pRadioButtonTextSelect, -1);
  gtk_widget_set_tooltip_text(GTK_WIDGET(pRadioButtonTextSelect), _("Copy selected area inside PDF files\nto clipboard as pure text.")); 
  gtk_radio_tool_button_set_group(GTK_RADIO_TOOL_BUTTON(pRadioButtonTextSelect), group_clip);
  group_clip = gtk_radio_tool_button_get_group(GTK_RADIO_TOOL_BUTTON(pRadioButtonTextSelect));

  pRadioButtonPictureSelect = gtk_radio_tool_button_new(group_clip);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), pRadioButtonPictureSelect, -1);
  gtk_widget_set_tooltip_text(GTK_WIDGET(pRadioButtonPictureSelect), _("Copy selected area inside PDF files or sketches \nto clipboard as pure image.")); 
  gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(pRadioButtonPictureSelect), icon_picture_select);
  gtk_radio_tool_button_set_group(GTK_RADIO_TOOL_BUTTON(pRadioButtonPictureSelect), group_clip);
  group_clip = gtk_radio_tool_button_get_group(GTK_RADIO_TOOL_BUTTON(pRadioButtonPictureSelect));

  gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON(pRadioButtonTextSelect), TRUE);

  pRadioButtonHiglightSelect=gtk_radio_tool_button_new(group_clip);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), pRadioButtonHiglightSelect, -1);
  gtk_widget_set_tooltip_text(GTK_WIDGET(pRadioButtonHiglightSelect), _("Highlight selected area inside PDF document. \nPlease don't forget to save your PDF document !"));
  gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(pRadioButtonHiglightSelect), icon_highlight_select);
  gtk_radio_tool_button_set_group(GTK_RADIO_TOOL_BUTTON(pRadioButtonHiglightSelect), group_clip);
  group_clip = gtk_radio_tool_button_get_group(GTK_RADIO_TOOL_BUTTON(pRadioButtonHiglightSelect));

  pRadioButtonHiAnnotSelect=gtk_radio_tool_button_new(group_clip);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(pRadioButtonHiAnnotSelect), -1);
  gtk_widget_set_tooltip_text(GTK_WIDGET(pRadioButtonHiAnnotSelect), _("Set-up text annotation inside \nselected area on PDF or sketch document. \nPlease don't forget to save your PDF/sketch document to keep \nyour changes !"));
  gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(pRadioButtonHiAnnotSelect), icon_text_annot);
  gtk_radio_tool_button_set_group(GTK_RADIO_TOOL_BUTTON(pRadioButtonHiAnnotSelect), group_clip);
  group_clip = gtk_radio_tool_button_get_group(GTK_RADIO_TOOL_BUTTON(pRadioButtonHiAnnotSelect));
  /* sketch tools */
  if(fIsDark)
     ico = gdk_pixbuf_new_from_xpm_data((const char **)pencil_light_xpm);
  else
     ico = gdk_pixbuf_new_from_xpm_data((const char **)pencil_xpm);
  icon_pencil=gtk_image_new_from_pixbuf(ico);
  g_object_unref(ico); 
  button_pencil= gtk_radio_tool_button_new(group_clip);
  gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(button_pencil),icon_pencil); 
  gtk_widget_set_tooltip_text(GTK_WIDGET(button_pencil), _("Freehand drawing tool"));
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(button_pencil), -1);
  gtk_radio_tool_button_set_group(GTK_RADIO_TOOL_BUTTON(button_pencil), group_clip);
  group_clip = gtk_radio_tool_button_get_group(GTK_RADIO_TOOL_BUTTON(button_pencil));
  
  /* color button */
  color_button_item = gtk_tool_item_new();
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(color_button_item), -1);
  gtk_widget_set_tooltip_text(GTK_WIDGET(color_button_item), _("Click here to choose color for :\n- highlighting of PDF areas.\n-annotation color for PDF and sketches documents.\n-pencil color for sketches."));
  color_button=gtk_color_button_new (); 
  gtk_container_add(GTK_CONTAINER(color_button_item), color_button);
  
  /* audio buttons */
  audioSeperator=gtk_separator_tool_item_new ();
  gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(audioSeperator), TRUE);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(audioSeperator), -1); 
 /* playing speed combobox */
  GtkToolItem *audioPlaySpeedContainer=gtk_tool_item_new();
  GtkWidget *audioPlaySpeed=gtk_combo_box_text_new();
  gtk_widget_set_tooltip_text(GTK_WIDGET(audioPlaySpeed), _("Choose here the playing speed. 1 for a normal speed,\nover 1 for a faster speed,\nunder 1 for a slower speed."));
 // gtk_widget_show (audioPlaySpeed);
  gtk_container_add(GTK_CONTAINER(audioPlaySpeedContainer), audioPlaySpeed);
  gtk_combo_box_text_append_text  (GTK_COMBO_BOX_TEXT (audioPlaySpeed), _("×1.5"));
  gtk_combo_box_text_append_text  (GTK_COMBO_BOX_TEXT (audioPlaySpeed), _("×1.2"));
  gtk_combo_box_text_append_text  (GTK_COMBO_BOX_TEXT (audioPlaySpeed), _("×1.0"));
  gtk_combo_box_text_append_text  (GTK_COMBO_BOX_TEXT (audioPlaySpeed), _("×0.8"));
  gtk_combo_box_text_append_text  (GTK_COMBO_BOX_TEXT (audioPlaySpeed), _("×0.5"));
  gtk_combo_box_set_active (GTK_COMBO_BOX(audioPlaySpeed), 2);
  gtk_widget_set_sensitive(GTK_WIDGET(audioPlaySpeed), FALSE);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), audioPlaySpeedContainer, -1);  

  iconButtonPlayAudio =gtk_image_new_from_icon_name ("media-playback-start", GTK_ICON_SIZE_LARGE_TOOLBAR);
  iconButtonPauseAudio =gtk_image_new_from_icon_name ("media-playback-pause", GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_widget_show(iconButtonPauseAudio);
  iconButtonHomeAudio=gtk_image_new_from_icon_name ("media-skip-backward", GTK_ICON_SIZE_LARGE_TOOLBAR);
  iconButtonGotoAudio=gtk_image_new_from_icon_name ("media-skip-forward", GTK_ICON_SIZE_LARGE_TOOLBAR);
  iconButtonGoJumpAudio=gtk_image_new_from_icon_name ("go-jump", GTK_ICON_SIZE_LARGE_TOOLBAR);
  pRadioButtonPlayPauseAudio = gtk_tool_button_new (iconButtonPlayAudio,NULL);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), pRadioButtonPlayPauseAudio, -1);  
  //gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(pRadioButtonPlayPauseAudio),iconButtonPlayAudio);
  gtk_widget_set_tooltip_text(GTK_WIDGET(pRadioButtonPlayPauseAudio), _("Play or pause-rewind current audio file.\nRewind's length is defined in 'audio' settings."));
  gtk_widget_set_sensitive(GTK_WIDGET(pRadioButtonPlayPauseAudio), FALSE);
  /* counter */
  GtkToolItem *audio_position=gtk_tool_item_new();
  gtk_widget_set_tooltip_text(GTK_WIDGET(audio_position), _("Position within current audio file"));
  GtkWidget *audio_position_label = gtk_label_new("<tt><big>--:--:--</big></tt>");/* tt for Monospace font family */
  gtk_label_set_use_markup (GTK_LABEL (audio_position_label), TRUE);
  gtk_container_add(GTK_CONTAINER(audio_position), audio_position_label);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(audio_position), -1);
  gtk_widget_set_sensitive(GTK_WIDGET(audio_position), FALSE);
  /* total audio file duration */
  GtkToolItem *audio_total=gtk_tool_item_new();
  gtk_widget_set_tooltip_text(GTK_WIDGET(audio_total), _("Total duration of current audio file"));
  GtkWidget *audio_total_label = gtk_label_new("<tt><small>/--:--:--</small></tt>");
  gtk_label_set_use_markup (GTK_LABEL (audio_total_label), TRUE);
  gtk_container_add(GTK_CONTAINER(audio_total), audio_total_label);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(audio_total), -1);
  gtk_widget_set_sensitive(GTK_WIDGET(audio_total), FALSE);
  /* goto previous mark */
  pRadioButtonRewindAudio = gtk_tool_button_new (iconButtonHomeAudio,NULL);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), pRadioButtonRewindAudio, -1);  
  gtk_widget_set_tooltip_text(GTK_WIDGET(pRadioButtonRewindAudio), _("Jump backward in current audio file.\nJump's length is defined in 'audio' settings."));
  gtk_widget_set_sensitive(GTK_WIDGET(pRadioButtonRewindAudio), FALSE);
  /* goto next mark */
  pRadioButtonGotoAudio = gtk_tool_button_new (iconButtonGotoAudio,NULL);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), pRadioButtonGotoAudio, -1);  
  gtk_widget_set_tooltip_text(GTK_WIDGET(pRadioButtonGotoAudio), _("Jump forward in current audio file.\nJump's length is defined in 'audio' settings."));
  gtk_widget_set_sensitive(GTK_WIDGET(pRadioButtonGotoAudio), FALSE);
  /* jump to ... */
  pRadioButtonGoJumpAudio = gtk_tool_button_new (iconButtonGoJumpAudio,NULL);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), pRadioButtonGoJumpAudio, -1);  
  gtk_widget_set_tooltip_text(GTK_WIDGET(pRadioButtonGoJumpAudio), _("Jump to a specifed position within current audio file."));
  gtk_widget_set_sensitive(GTK_WIDGET(pRadioButtonGoJumpAudio), FALSE); 
  

  gtk_widget_show_all(toolbar);
  /* set cSS states */
  toggle_css_value(GTK_WIDGET(button_bold), FALSE);
  toggle_css_value(GTK_WIDGET(button_italic), FALSE);
  toggle_css_value(GTK_WIDGET(button_underline), FALSE);
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
  g_signal_connect(G_OBJECT(button_bold), "toggled", 
        G_CALLBACK(on_bold_clicked), data_app);
  g_signal_connect(G_OBJECT(button_italic), "toggled", 
        G_CALLBACK(on_italic_clicked), data_app);
  g_signal_connect(G_OBJECT(button_underline), "toggled", 
        G_CALLBACK(on_underline_clicked), data_app);
  /* other formatings */
  g_signal_connect(G_OBJECT(button_superscript), "toggled", 
        G_CALLBACK(on_superscript_clicked), data_app);
  g_signal_connect(G_OBJECT(button_subscript), "toggled", 
        G_CALLBACK(on_subscript_clicked), data_app);
  g_signal_connect(G_OBJECT(button_strikethrough), "toggled", 
        G_CALLBACK(on_strikethrough_clicked), data_app);
  g_signal_connect(G_OBJECT(button_highlight), "toggled", 
        G_CALLBACK(on_highlight_clicked), data_app);
  g_signal_connect(G_OBJECT(button_quotation), "toggled", 
        G_CALLBACK(on_quotation_clicked), data_app);

  g_signal_connect(G_OBJECT(button_clear_format), "clicked", 
        G_CALLBACK(on_clear_format_clicked), data_app);
  g_signal_connect(G_OBJECT(button_undo), "clicked", 
        G_CALLBACK(on_undo_clicked), data_app);
  g_signal_connect(G_OBJECT(pRadioButtonPlayPauseAudio), "clicked", 
        G_CALLBACK(on_play_pause_clicked), data_app);
  g_signal_connect(G_OBJECT(pRadioButtonRewindAudio), "clicked", 
        G_CALLBACK(on_jump_prev_clicked), data_app);
  g_signal_connect(G_OBJECT(pRadioButtonGotoAudio), "clicked", 
        G_CALLBACK(on_jump_next_clicked), data_app);
  g_signal_connect(G_OBJECT(pRadioButtonGoJumpAudio), "clicked", 
        G_CALLBACK(on_go_jump_clicked), data_app);

  g_signal_connect ((gpointer) audioPlaySpeed, "changed",
                    G_CALLBACK (on_audioPlaySpeed_changed),
                    data_app);

  /* for toogle radio buttons we must pass GSlist as parameter in order to get the Active button in group */
  g_signal_connect(G_OBJECT(pRadioButtonLeft), "toggled", 
        G_CALLBACK(on_button_alignment_toggled), data_app);
  g_signal_connect(G_OBJECT(pRadioButtonCenter), "toggled", 
        G_CALLBACK(on_button_alignment_toggled), data_app);
  g_signal_connect(G_OBJECT(pRadioButtonRight), "toggled", 
        G_CALLBACK(on_button_alignment_toggled), data_app);
  g_signal_connect(G_OBJECT(pRadioButtonFill), "toggled", 
        G_CALLBACK(on_button_alignment_toggled), data_app);

  g_signal_connect(G_OBJECT(pRadioButtonTextSelect), "toggled", 
        G_CALLBACK(on_button_clip_mode_toggled), data_app);
  g_signal_connect(G_OBJECT(pRadioButtonPictureSelect), "toggled", 
        G_CALLBACK(on_button_clip_mode_toggled), data_app);
  g_signal_connect(G_OBJECT(pRadioButtonHiglightSelect), "toggled", 
        G_CALLBACK(on_button_clip_mode_toggled), data_app);
  g_signal_connect(G_OBJECT(pRadioButtonHiAnnotSelect), "toggled", 
        G_CALLBACK(on_button_clip_mode_toggled), data_app);
  g_signal_connect(G_OBJECT(button_pencil), "toggled", 
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
  GLADE_HOOKUP_OBJECT (window, pRadioButtonHiAnnotSelect, "pRadioButtonHiAnnotSelect");
  GLADE_HOOKUP_OBJECT (window, button_pencil, "button_pencil");
  GLADE_HOOKUP_OBJECT (window, color_button, "color_button");
  GLADE_HOOKUP_OBJECT (window, pRadioButtonPlayPauseAudio, "pRadioButtonPlayPauseAudio");
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
  data_app->pBtnUndo=GTK_WIDGET(button_undo);
  return toolbar;
}

/****************************
  main window
****************************/
GtkWidget *UI_main_window(void)
{
  GtkWidget *win;

  win=gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(win), 980, 700);
  gtk_window_set_resizable (GTK_WINDOW(win), TRUE);
  gtk_window_set_decorated (GTK_WINDOW(win), TRUE);
  gtk_window_set_title(GTK_WINDOW(win), "Redac !");
  gtk_window_set_icon_name (GTK_WINDOW (win), "redac");
  return win;
}
/********************************
 build pseudo Gtk3 headerbar
*******************************/
void UI_headerBar(GtkWidget *window, GtkWidget *grid, APP_data *data)
{
  GtkWidget *main_menu;
  GtkWidget *main_menu_icon;
  GtkWidget *button_prefs;
  GtkWidget *icon_prefs;
  GtkWidget *labelMainTitle;


  main_menu = gtk_button_new();
  gtk_button_set_relief(GTK_BUTTON(main_menu), GTK_RELIEF_NONE);
  main_menu_icon=gtk_image_new_from_icon_name ("view-list-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_button_set_image(GTK_BUTTON(main_menu),main_menu_icon);
  gtk_widget_set_tooltip_text(main_menu, _("Menu to access various file's operations"));
  gtk_grid_attach(GTK_GRID(grid), main_menu, 0, 0, 1, 1);

  button_prefs = gtk_button_new();
  gtk_button_set_relief(GTK_BUTTON(button_prefs), GTK_RELIEF_NONE);
  icon_prefs=gtk_image_new_from_icon_name ("view-more-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_button_set_image(GTK_BUTTON(button_prefs),icon_prefs);
  gtk_widget_set_tooltip_text(button_prefs, _("Application settings."));
  gtk_grid_attach(GTK_GRID(grid), button_prefs, 1, 0, 1, 1);

  labelMainTitle=gtk_label_new(_("noname"));
  gtk_label_set_markup (GTK_LABEL (labelMainTitle), _("<small><b>noname</b></small>"));
  g_object_set (labelMainTitle, "margin-left", 24, NULL);
  g_object_set (labelMainTitle, "margin-right", 24, NULL);
  gtk_grid_attach(GTK_GRID(grid), labelMainTitle, 3, 0, 1, 1);

  /* signals */
  g_signal_connect(G_OBJECT(main_menu), "clicked", 
        G_CALLBACK(on_main_menu_clicked), data);

  g_signal_connect(G_OBJECT(button_prefs), "clicked", 
        G_CALLBACK(on_prefs_clicked), data);

  GLADE_HOOKUP_OBJECT (window, main_menu, "main_menu");
  GLADE_HOOKUP_OBJECT (window, labelMainTitle, "labelMainTitle");
  GLADE_HOOKUP_OBJECT (window, button_prefs, "button_prefs");

}
/****************************
  set up main statusbar
****************************/
void UI_statusbar(GtkWidget *window, GtkWidget *grid, APP_data *data)
{
  GtkWidget *statusbar;
  GtkWidget *PDF_modified_label;
  GtkWidget *search_entry;
  GtkWidget *buttonPrevOccurrence;
  GtkWidget *buttonNextOccurrence;
  GtkWidget *labelHitsFrame;
  GtkWidget *labelHitsGrid;
  GtkWidget *labelHits;
  GtkWidget *buttonReplace;
  GtkWidget *buttonZoomIn;
  GtkWidget *buttonZoomOut;
  GtkWidget *buttonZoomFitBest;
  GtkWidget *replace_entry;
  GtkWidget *image_task_due;
  GtkWidget *image_pdf_modif;
  GtkWidget *image_audio_jump_to_start;
  GdkPixbuf *ico;

  buttonPrevOccurrence = gtk_button_new_from_icon_name("go-up-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
  gtk_widget_set_tooltip_text(buttonPrevOccurrence, _("Click here to to go to the previous hit.\nWith PDF documents we jump to previous PAGE with hits"));

  buttonNextOccurrence = gtk_button_new_from_icon_name("go-down-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
  gtk_widget_set_tooltip_text(buttonNextOccurrence, _("Click here to go to the next hit.\nWith PDF documents we jump to next PAGE with hits"));

  buttonReplace = gtk_button_new_from_icon_name("edit-find-replace", GTK_ICON_SIZE_SMALL_TOOLBAR);
  gtk_widget_set_tooltip_text(buttonReplace, _("Click here to replace text.\nYou can't change text inside PDF documents."));
  gtk_widget_set_sensitive(GTK_WIDGET(buttonReplace), FALSE);
  /* PDF zoom buttons */
  buttonZoomIn = gtk_button_new_from_icon_name("zoom-in-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
  gtk_widget_set_tooltip_text(buttonZoomIn, _("Click to Zoom in the PDF document."));
  gtk_widget_set_sensitive(GTK_WIDGET(buttonZoomIn), FALSE);
  buttonZoomOut = gtk_button_new_from_icon_name("zoom-out-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
  gtk_widget_set_tooltip_text(buttonZoomOut, _("Click to Zoom out the PDF document."));
  gtk_widget_set_sensitive(GTK_WIDGET(buttonZoomOut), FALSE);
  buttonZoomFitBest = gtk_button_new_from_icon_name("zoom-fit-best-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
  gtk_widget_set_tooltip_text(buttonZoomFitBest, _("Click to display the PDF document at its best size."));
  gtk_widget_set_sensitive(GTK_WIDGET(buttonZoomFitBest), FALSE);

  statusbar = gtk_statusbar_new();
  gtk_grid_attach(GTK_GRID(grid), statusbar, 0,0,1,1);

  PDF_modified_label = gtk_label_new("   ");
  gtk_label_set_use_markup (GTK_LABEL (PDF_modified_label), TRUE);
  gtk_grid_attach(GTK_GRID(grid), PDF_modified_label,1,0,1,1);

 
  labelHitsFrame = gtk_frame_new(NULL);
  gtk_widget_set_name(labelHitsFrame, "labelHitsFrame" );
  g_object_set (labelHitsFrame, "margin-right", 24, NULL);
  gtk_grid_attach(GTK_GRID(grid), labelHitsFrame,5,0,1,1);

  labelHitsGrid = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_set_homogeneous(GTK_BOX(labelHitsGrid), FALSE);
  gtk_container_add( GTK_CONTAINER( labelHitsFrame), labelHitsGrid);

  search_entry = gtk_search_entry_new ();
  gtk_widget_set_name(search_entry, "search_entry" );
  gtk_box_pack_start(GTK_BOX(labelHitsGrid), search_entry, TRUE, FALSE, 0);
  gtk_widget_set_tooltip_text(search_entry, _("Type here the expression to find."));

  labelHits = gtk_label_new(_("--hits"));
  gtk_box_pack_start(GTK_BOX(labelHitsGrid), labelHits, TRUE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(labelHitsGrid), buttonPrevOccurrence, TRUE, FALSE, 0);
  g_object_set (buttonPrevOccurrence, "margin-left", 6, NULL);
  g_object_set (buttonPrevOccurrence, "margin-right", 2, NULL);

  gtk_box_pack_start(GTK_BOX(labelHitsGrid), buttonNextOccurrence, TRUE, FALSE, 0);
  g_object_set (buttonNextOccurrence, "margin-left", 6, NULL);
  g_object_set (buttonNextOccurrence, "margin-right", 2, NULL);

  gtk_widget_set_sensitive(GTK_WIDGET(buttonNextOccurrence),FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(buttonPrevOccurrence),FALSE);

  gtk_grid_attach(GTK_GRID(grid), buttonReplace, 8,0,1,1);
  g_object_set (buttonReplace, "margin-left", 6, NULL);

  replace_entry = gtk_entry_new ();
  gtk_widget_set_tooltip_text(GTK_WIDGET(replace_entry), _("Type here the replacing expression."));
  gtk_grid_attach(GTK_GRID(grid), replace_entry, 9,0,1,1);
  gtk_widget_set_sensitive(GTK_WIDGET(replace_entry),FALSE);

  gtk_grid_attach(GTK_GRID(grid), buttonZoomOut, 10,0,1,1);
  gtk_grid_attach(GTK_GRID(grid), buttonZoomFitBest, 11,0,1,1);
  g_object_set (buttonZoomFitBest, "margin-left",6, NULL);
  gtk_grid_attach(GTK_GRID(grid), buttonZoomIn, 12,0,1,1);
  g_object_set (buttonZoomIn, "margin-left",6, NULL);
  /* indicator auto-save */
  image_task_due =gtk_image_new_from_icon_name ("alarm-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
  gtk_widget_set_tooltip_text(image_task_due, _("Automatic background saving of your work (editor part)\nis activated."));
  gtk_grid_attach(GTK_GRID(grid), image_task_due, 13,0,1,1);
  g_object_set (image_task_due, "margin-left", 36, NULL);
  /* audio auto repeat - jump to start icon */
  image_audio_jump_to_start=gtk_image_new_from_icon_name ("media-playlist-repeat", GTK_ICON_SIZE_SMALL_TOOLBAR);
  gtk_widget_set_tooltip_text(image_audio_jump_to_start, _("Automatic rewind to start of media after playing\nis activated."));
  gtk_grid_attach(GTK_GRID(grid), image_audio_jump_to_start, 15,0,1,1);
  g_object_set (image_audio_jump_to_start, "margin-left", 12, NULL);

  ico = gdk_pixbuf_new_from_xpm_data((const char **)pdf_xpm);
  image_pdf_modif =gtk_image_new_from_pixbuf(ico);
  g_object_unref(ico);
  gtk_widget_set_tooltip_text(image_pdf_modif, _("Please, notice that that the current PDF is modifyed\nand should be saved."));
  gtk_grid_attach(GTK_GRID(grid), image_pdf_modif, 14,0,1,1);
  g_object_set (image_pdf_modif, "margin-left", 36, NULL);

  /* callbacks */
  g_signal_connect(G_OBJECT(buttonReplace), "clicked", 
       G_CALLBACK(on_replace_clicked), data);
  g_signal_connect(search_entry, "search-changed", 
                   G_CALLBACK(on_find_changed), data);

  g_signal_connect(G_OBJECT(buttonNextOccurrence), "clicked", 
        G_CALLBACK(on_find_next_clicked), data);
  g_signal_connect(G_OBJECT(buttonPrevOccurrence), "clicked", 
        G_CALLBACK(on_find_prev_clicked), data);

  g_signal_connect(G_OBJECT(buttonZoomIn), "clicked", 
        G_CALLBACK(on_PDF_zoom_in_clicked), data);
  g_signal_connect(G_OBJECT(buttonZoomOut), "clicked", 
        G_CALLBACK(on_PDF_zoom_out_clicked), data);
  g_signal_connect(G_OBJECT(buttonZoomFitBest), "clicked", 
        G_CALLBACK(on_PDF_zoom_fit_best_clicked), data);

  data->statusbar1 =statusbar;
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
  GtkWidget *page_frame;
  GtkWidget *page_grid;
  GtkWidget *page_title;
  GtkWidget *page_entry;
  GtkWidget *page_label;

  page_frame=gtk_frame_new(NULL);
  gtk_frame_set_shadow_type (GTK_FRAME(page_frame), GTK_SHADOW_OUT);
  gtk_widget_set_name(page_frame, "page_frame" );
  g_object_set (page_frame, "margin-left", 6, NULL);
  g_object_set (page_frame, "margin-right", 6, NULL);
  gtk_widget_set_hexpand (page_frame, FALSE);
  gtk_grid_attach(GTK_GRID(grid), page_frame,2,0,1,1);
  page_grid=gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_set_homogeneous(GTK_BOX(page_grid), FALSE);
  gtk_container_add( GTK_CONTAINER(page_frame), page_grid);

  page_title=gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(page_title), _("Pg:"));
  gtk_widget_set_name(page_title, "page_title" );
  gtk_entry_set_max_length (GTK_ENTRY (page_title), 3);
  gtk_entry_set_width_chars (GTK_ENTRY (page_title), 3);
  gtk_widget_set_sensitive(GTK_WIDGET(page_title), FALSE);
  gtk_widget_set_hexpand (page_title, TRUE);
  gtk_entry_set_has_frame(GTK_ENTRY(page_title), FALSE );
  gtk_box_pack_start(GTK_BOX(page_grid), page_title, TRUE, FALSE, 0);

  page_entry = gtk_entry_new ();
  gtk_entry_set_has_frame(GTK_ENTRY(page_entry), TRUE );
  gtk_widget_set_name(page_entry, "page_entry" );
  gtk_entry_set_max_length (GTK_ENTRY (page_entry), 4);
  gtk_entry_set_width_chars (GTK_ENTRY (page_entry), 3);
  gtk_widget_set_hexpand (page_entry, TRUE);
  gtk_box_pack_start(GTK_BOX(page_grid), page_entry, TRUE, FALSE, 0);
  gtk_widget_set_tooltip_text(page_entry, _("Type here the #of PDF page where you want to jump."));

  page_label=gtk_entry_new();
  gtk_entry_set_width_chars (GTK_ENTRY (page_label), 5);
  gtk_entry_set_text(GTK_ENTRY(page_label), _("of--"));
  gtk_widget_set_sensitive(GTK_WIDGET(page_label), FALSE);
  gtk_widget_set_hexpand (page_label, TRUE);
  gtk_box_pack_start(GTK_BOX(page_grid), page_label, TRUE, FALSE, 0);
  gtk_widget_set_name(page_label, "page_label");
  // gtk_editable_set_editable(GTK_EDITABLE(page_label), FALSE);

  g_signal_connect(page_entry, "changed", 
                   G_CALLBACK(on_page_entry_changed), data);

  GLADE_HOOKUP_OBJECT (window, page_title, "page_title");
  GLADE_HOOKUP_OBJECT (window, page_entry, "page_entry");
  GLADE_HOOKUP_OBJECT (window, page_label, "page_label");
  GLADE_HOOKUP_OBJECT (window, page_frame, "page_frame");
}


/*********************************
  load from file dialog 
*********************************/
GtkWidget*
create_loadFileDialog (APP_data *data)
{
  GtkWidget *loadFileDialog;
  GtkWidget *dialog_vbox6;
  GtkWidget *dialog_action_area6;
  GtkWidget *button43;
  GtkWidget *image43;
  GtkWidget *button44;
  GtkWidget *image44;

  loadFileDialog = gtk_file_chooser_dialog_new (_("Open file..."), 
                              GTK_WINDOW(data->appWindow), GTK_FILE_CHOOSER_ACTION_OPEN, NULL);
  g_object_set (loadFileDialog, "local-only", FALSE,  NULL);
  gtk_window_set_modal (GTK_WINDOW (loadFileDialog), TRUE);
  gtk_window_set_destroy_with_parent (GTK_WINDOW (loadFileDialog), TRUE);
  gtk_window_set_icon_name (GTK_WINDOW (loadFileDialog), "gtk-file");
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (loadFileDialog), TRUE);
  gtk_window_set_type_hint (GTK_WINDOW (loadFileDialog), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialog_vbox6 = gtk_dialog_get_content_area (GTK_DIALOG (loadFileDialog));
  gtk_widget_show (dialog_vbox6);

  dialog_action_area6 = gtk_dialog_get_action_area (GTK_DIALOG (loadFileDialog));
  gtk_widget_show (dialog_action_area6);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area6), GTK_BUTTONBOX_END);

  button43=  gtk_button_new_with_label (_("Cancel"));
  image43 = gtk_image_new_from_icon_name ("gtk-cancel",  GTK_ICON_SIZE_BUTTON);
  gtk_button_set_always_show_image (GTK_BUTTON (button43), TRUE);
  gtk_button_set_image (GTK_BUTTON (button43), image43);
  gtk_widget_show (button43);
  gtk_dialog_add_action_widget (GTK_DIALOG (loadFileDialog), button43, GTK_RESPONSE_CANCEL);
  gtk_widget_set_can_default (button43, TRUE);

  button44=  gtk_button_new_with_label (_("Open"));
  image44 = gtk_image_new_from_icon_name ("gtk-open",  GTK_ICON_SIZE_BUTTON);
  gtk_button_set_always_show_image (GTK_BUTTON (button44), TRUE);
  gtk_button_set_image (GTK_BUTTON (button44), image44);
  gtk_widget_show (button44);
  gtk_dialog_add_action_widget (GTK_DIALOG (loadFileDialog), button44, GTK_RESPONSE_OK);
  gtk_widget_set_can_default (button44, TRUE);
  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (loadFileDialog, loadFileDialog, "loadFileDialog");
  GLADE_HOOKUP_OBJECT_NO_REF (loadFileDialog, dialog_vbox6, "dialog_vbox6");
  GLADE_HOOKUP_OBJECT_NO_REF (loadFileDialog, dialog_action_area6, "dialog_action_area6");
  GLADE_HOOKUP_OBJECT (loadFileDialog, button43, "button43");
  GLADE_HOOKUP_OBJECT (loadFileDialog, button44, "button44");

  gtk_widget_grab_default (button44);
  return loadFileDialog;
}


/*********************************
  save to file dialog 
*********************************/
GtkWidget*
create_saveFileDialog (APP_data *data)
{
  GtkWidget *saveFileDialog;
  GtkWidget *dialog_vbox6;
  GtkWidget *dialog_action_area6;
  GtkWidget *button43;
  GtkWidget *image43;
  GtkWidget *button44;
  GtkWidget *image44;

    saveFileDialog = gtk_file_chooser_dialog_new (_("Save your work and export to standard Word processor file..."), 
                                    GTK_WINDOW(data->appWindow),
                                    GTK_FILE_CHOOSER_ACTION_SAVE, NULL);
  g_object_set (saveFileDialog, "local-only", FALSE,  NULL);
  gtk_window_set_modal (GTK_WINDOW (saveFileDialog), TRUE);
  gtk_window_set_destroy_with_parent (GTK_WINDOW (saveFileDialog), TRUE);
  gtk_window_set_icon_name (GTK_WINDOW (saveFileDialog), "gtk-file");
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (saveFileDialog), TRUE);
  gtk_window_set_type_hint (GTK_WINDOW (saveFileDialog), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialog_vbox6 = gtk_dialog_get_content_area (GTK_DIALOG (saveFileDialog));
  gtk_widget_show (dialog_vbox6);

  dialog_action_area6 = gtk_dialog_get_action_area (GTK_DIALOG (saveFileDialog));
  gtk_widget_show (dialog_action_area6);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area6), GTK_BUTTONBOX_END);

  button43=  gtk_button_new_with_label (_("Cancel"));
  image43 = gtk_image_new_from_icon_name ("gtk-cancel",  GTK_ICON_SIZE_BUTTON);
  gtk_button_set_always_show_image (GTK_BUTTON (button43), TRUE);
  gtk_button_set_image (GTK_BUTTON (button43), image43);
  gtk_widget_show (button43);
  gtk_dialog_add_action_widget (GTK_DIALOG (saveFileDialog), button43, GTK_RESPONSE_CANCEL);
  gtk_widget_set_can_default (button43, TRUE);

  button44=  gtk_button_new_with_label (_("Save"));
  image44 = gtk_image_new_from_icon_name ("gtk-save",  GTK_ICON_SIZE_BUTTON);
  gtk_button_set_always_show_image (GTK_BUTTON (button44), TRUE);
  gtk_button_set_image (GTK_BUTTON (button44), image44);
  gtk_widget_show (button44);
  gtk_dialog_add_action_widget (GTK_DIALOG (saveFileDialog), button44, GTK_RESPONSE_OK);
  gtk_widget_set_can_default (button44, TRUE);
  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (saveFileDialog, saveFileDialog, "saveFileDialog");
  GLADE_HOOKUP_OBJECT_NO_REF (saveFileDialog, dialog_vbox6, "dialog_vbox6");
  GLADE_HOOKUP_OBJECT_NO_REF (saveFileDialog, dialog_action_area6, "dialog_action_area6");
  GLADE_HOOKUP_OBJECT (saveFileDialog, button43, "button43");
  GLADE_HOOKUP_OBJECT (saveFileDialog, button44, "button44");

  gtk_widget_grab_default (button44);
  return saveFileDialog;
}

/*********************************
  help dialog 
*********************************/
GtkWidget *misc_create_help_dialog(GtkWidget *win)
{
  GtkWidget *helpDialog;
  GtkWidget *dialog_vbox11;
  GtkWidget *gridDialog;
  GtkWidget *labelAlignment;
  GtkWidget *labelFormat;
  GtkWidget *labelSpace;
  GtkWidget *labelFormatBoldTitle;
  GtkWidget *labelAlignmentShortcuts;
  GtkWidget *dialog_action_area11;
  GtkWidget *okbutton6;
  GtkWidget *imageOkButton6;

  helpDialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (helpDialog), _("Help..."));
  gtk_window_set_position (GTK_WINDOW (helpDialog), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (helpDialog), TRUE);
  gtk_window_set_resizable (GTK_WINDOW (helpDialog), TRUE);
  gtk_window_set_type_hint (GTK_WINDOW (helpDialog), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_transient_for (GTK_WINDOW (helpDialog),  GTK_WINDOW(win)); 
  dialog_vbox11 = gtk_dialog_get_content_area (GTK_DIALOG (helpDialog));
  gtk_widget_show (dialog_vbox11);
  gridDialog = gtk_grid_new();
  gtk_widget_show (gridDialog);
  gtk_box_pack_start (GTK_BOX (dialog_vbox11), gridDialog, TRUE, TRUE, 4);
  /* labels */
  labelFormat = gtk_label_new(_("<b><big>Format shortcuts:\n</big></b>"));
  g_object_set(labelFormat, "margin-left", 24, NULL);
  g_object_set(labelFormat, "margin-right", 24, NULL);
  gtk_label_set_use_markup (GTK_LABEL (labelFormat), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelFormat, 0, 0, 2, 1);

  labelAlignment = gtk_label_new(_("<b><big>Alignment shortcuts:\n</big></b>"));
  g_object_set(labelAlignment, "margin-left", 24, NULL);
  g_object_set(labelAlignment, "margin-right", 24, NULL);
  gtk_label_set_use_markup (GTK_LABEL (labelAlignment), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelAlignment, 3, 0, 2, 1);

  GtkWidget *labelPdfSketch = gtk_label_new(_("<b><big>PDF, Sketch shortcuts:\n</big></b>"));
  g_object_set(labelPdfSketch, "margin-left", 24, NULL);
  g_object_set(labelPdfSketch, "margin-right", 24, NULL);
  gtk_label_set_use_markup (GTK_LABEL (labelPdfSketch), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelPdfSketch, 5, 0, 2, 1);

  GtkWidget *labelOther = gtk_label_new(_("<b><big>Application shortcuts:\n</big></b>"));
  g_object_set(labelOther, "margin-left", 24, NULL);
  g_object_set(labelOther, "margin-right", 24, NULL);
  gtk_label_set_use_markup (GTK_LABEL (labelOther), TRUE);
  gtk_grid_attach (GTK_GRID (gridDialog), labelOther, 7, 0, 2, 1);

  labelFormatBoldTitle = gtk_label_new(_("<b>Bold</b>"));
  g_object_set (labelFormatBoldTitle, "margin", 4, NULL);
  gtk_label_set_use_markup (GTK_LABEL (labelFormatBoldTitle), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelFormatBoldTitle, 0, 1, 1, 1);

  GtkWidget *labelFormatItalicTitle = gtk_label_new(_("<b>Italic</b>"));
  g_object_set (labelFormatItalicTitle, "margin", 4, NULL);
  gtk_label_set_use_markup (GTK_LABEL (labelFormatItalicTitle), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelFormatItalicTitle, 0, 2, 1, 1);

  GtkWidget *labelFormatUnderlineTitle = gtk_label_new(_("<b>Underline</b>"));
  g_object_set (labelFormatUnderlineTitle, "margin", 4, NULL);
  gtk_label_set_use_markup (GTK_LABEL (labelFormatUnderlineTitle), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelFormatUnderlineTitle, 0, 3, 1, 1);

  GtkWidget *labelFormatSuperscriptTitle = gtk_label_new(_("<b>Superscript</b>"));
  g_object_set (labelFormatSuperscriptTitle, "margin", 4, NULL);
  gtk_label_set_use_markup (GTK_LABEL (labelFormatSuperscriptTitle), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelFormatSuperscriptTitle, 0, 4, 1, 1);

  GtkWidget *labelFormatSubscriptTitle = gtk_label_new(_("<b>Subscript</b>"));
  g_object_set (labelFormatSubscriptTitle, "margin", 4, NULL);
  gtk_widget_show(labelFormatSubscriptTitle);
  gtk_label_set_use_markup (GTK_LABEL (labelFormatSubscriptTitle), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelFormatSubscriptTitle, 0, 5, 1, 1);

  GtkWidget *labelFormatstrikethroughTitle = gtk_label_new(_("<b>Strikethrough</b>"));
  g_object_set (labelFormatstrikethroughTitle, "margin", 4, NULL);
  gtk_widget_show(labelFormatstrikethroughTitle);
  gtk_label_set_use_markup (GTK_LABEL (labelFormatstrikethroughTitle), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelFormatstrikethroughTitle, 0, 6, 1, 1);

  GtkWidget *labelFormatHighlightTitle = gtk_label_new(_("<b>Highlight</b>"));
  g_object_set (labelFormatHighlightTitle, "margin", 4, NULL);
  gtk_label_set_use_markup (GTK_LABEL (labelFormatHighlightTitle), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelFormatHighlightTitle, 0, 7, 1, 1);

  GtkWidget *labelFormatQuotationTitle = gtk_label_new(_("<b>Quotation</b>"));
  g_object_set (labelFormatQuotationTitle, "margin", 4, NULL);
  gtk_label_set_use_markup (GTK_LABEL (labelFormatQuotationTitle), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelFormatQuotationTitle, 0, 8, 1, 1);

  GtkWidget *labelBoldShortcut = gtk_label_new(misc_get_pango_string("b", 1));
  gtk_widget_show(labelBoldShortcut);
  gtk_label_set_use_markup (GTK_LABEL (labelBoldShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelBoldShortcut, 1, 1, 1, 1);

  GtkWidget *labelItalicShortcut = gtk_label_new(misc_get_pango_string("i", 1));
  gtk_widget_show(labelItalicShortcut);
  gtk_label_set_use_markup (GTK_LABEL (labelItalicShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelItalicShortcut, 1, 2, 1, 1);

  GtkWidget *labelUnderlineShortcut = gtk_label_new(misc_get_pango_string("u", 1));
  gtk_label_set_use_markup (GTK_LABEL (labelUnderlineShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelUnderlineShortcut, 1, 3, 1, 1);

  GtkWidget *labelSuperscriptShortcut = gtk_label_new(misc_get_pango_string("^", 1));
  gtk_label_set_use_markup (GTK_LABEL (labelSuperscriptShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelSuperscriptShortcut, 1, 4, 1, 1);

  GtkWidget *labelSubscriptShortcut = gtk_label_new(misc_get_pango_string("_", 1));
  gtk_label_set_use_markup (GTK_LABEL (labelSubscriptShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelSubscriptShortcut, 1, 5, 1, 1);

  GtkWidget *labelStrikethroughShortcut = gtk_label_new(misc_get_pango_string("k", 1));
  gtk_label_set_use_markup (GTK_LABEL (labelStrikethroughShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelStrikethroughShortcut, 1, 6, 1, 1);

  GtkWidget *labelHighlightShortcut = gtk_label_new(misc_get_pango_string("h", 1));
  gtk_label_set_use_markup (GTK_LABEL (labelHighlightShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelHighlightShortcut, 1, 7, 1, 1);

  GtkWidget *labelQuotationShortcut = gtk_label_new(misc_get_pango_string("\"", 1));
  gtk_label_set_use_markup (GTK_LABEL (labelQuotationShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelQuotationShortcut, 1, 8, 1, 1);
/* aligments */
  GtkWidget *labelAlignmentLeftShortcuts = gtk_label_new(_("<b>Left</b>"));
  gtk_label_set_use_markup (GTK_LABEL (labelAlignmentLeftShortcuts), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelAlignmentLeftShortcuts, 3, 1, 1, 1);

  GtkWidget *labelAlignmentCenterShortcuts = gtk_label_new(_("<b>Center</b>"));
  gtk_label_set_use_markup (GTK_LABEL (labelAlignmentCenterShortcuts), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelAlignmentCenterShortcuts, 3, 2, 1, 1);

  GtkWidget *labelAlignmentRightShortcuts = gtk_label_new(_("<b>Right</b>"));
  gtk_label_set_use_markup (GTK_LABEL (labelAlignmentRightShortcuts), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelAlignmentRightShortcuts, 3, 3, 1, 1);

  GtkWidget *labelAlignmentFillShortcuts = gtk_label_new(_("<b>Fill</b>"));
  gtk_label_set_use_markup (GTK_LABEL (labelAlignmentFillShortcuts), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelAlignmentFillShortcuts, 3, 4, 1, 1);

  GtkWidget *labelAlignmentLeftShortcut = gtk_label_new(misc_get_pango_string("(", 1));
  gtk_widget_show(labelAlignmentLeftShortcut);
  gtk_label_set_use_markup (GTK_LABEL (labelAlignmentLeftShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelAlignmentLeftShortcut, 4, 1, 1, 1);

  GtkWidget *labelAlignmentCenterShortcut = gtk_label_new(misc_get_pango_string(":", 1));
  gtk_label_set_use_markup (GTK_LABEL (labelAlignmentCenterShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelAlignmentCenterShortcut, 4, 2, 1, 1);

  GtkWidget *labelAlignmentRightShortcut = gtk_label_new(misc_get_pango_string(")", 1));
  gtk_label_set_use_markup (GTK_LABEL (labelAlignmentRightShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelAlignmentRightShortcut, 4, 3, 1, 1);

  GtkWidget *labelAlignmentFillShortcut = gtk_label_new(misc_get_pango_string("=", 1));
  gtk_label_set_use_markup (GTK_LABEL (labelAlignmentFillShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelAlignmentFillShortcut, 4, 4, 1, 1);
/* PDF & sketch */
  GtkWidget *labelPdfPgDownShortcuts = gtk_label_new(_("<b>PDF : go next page</b>"));
  gtk_label_set_use_markup (GTK_LABEL (labelPdfPgDownShortcuts), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelPdfPgDownShortcuts, 5, 1, 1, 1);

  GtkWidget *labelPdfPgDownShortcut = gtk_label_new(misc_get_pango_string(_("PgDown"), 0));
  gtk_label_set_use_markup (GTK_LABEL (labelPdfPgDownShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelPdfPgDownShortcut, 6, 1, 1, 1);

  GtkWidget *labelPdfPgUpShortcuts = gtk_label_new(_("<b>PDF : go previous page</b>"));
  gtk_widget_show(labelPdfPgUpShortcuts);
  gtk_label_set_use_markup (GTK_LABEL (labelPdfPgUpShortcuts), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelPdfPgUpShortcuts, 5, 2, 1, 1);

  GtkWidget *labelPdfPgUpShortcut = gtk_label_new(misc_get_pango_string(_("PgUp"), 0));
  gtk_label_set_use_markup (GTK_LABEL (labelPdfPgUpShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelPdfPgUpShortcut, 6, 2, 1, 1);

  GtkWidget *labelPdfFirstShortcuts = gtk_label_new(_("<b>PDF : go first page</b>"));
  gtk_label_set_use_markup (GTK_LABEL (labelPdfFirstShortcuts), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelPdfFirstShortcuts, 5, 3, 1, 1);

  GtkWidget *labelPdfFirstShortcut = gtk_label_new(misc_get_pango_string("Home", 0));
  gtk_label_set_use_markup (GTK_LABEL (labelPdfFirstShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelPdfFirstShortcut, 6, 3, 1, 1);

  GtkWidget *labelPdfLastShortcuts = gtk_label_new(_("<b>PDF : go last page</b>"));
  gtk_label_set_use_markup (GTK_LABEL (labelPdfLastShortcuts), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelPdfLastShortcuts, 5, 4, 1, 1);

  GtkWidget *labelPdfLastShortcut = gtk_label_new(misc_get_pango_string("End", 0));
  gtk_label_set_use_markup (GTK_LABEL (labelPdfLastShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelPdfLastShortcut, 6, 4, 1, 1);

  GtkWidget *labelPdfSearchShortcuts = gtk_label_new(_("<b>PDF : search</b>"));
  gtk_widget_show(labelPdfSearchShortcuts);
  gtk_label_set_use_markup (GTK_LABEL (labelPdfSearchShortcuts), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelPdfSearchShortcuts, 5, 5, 1, 1);

  GtkWidget *labelPdfSearchShortcut = gtk_label_new(misc_get_pango_string("f", 1));
  gtk_label_set_use_markup (GTK_LABEL (labelPdfSearchShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelPdfSearchShortcut, 6, 5, 1, 1);

  GtkWidget *labelPdfZoomInShortcuts = gtk_label_new(_("<b>PDF : zoom in</b>"));
  gtk_label_set_use_markup (GTK_LABEL (labelPdfZoomInShortcuts), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelPdfZoomInShortcuts, 5, 6, 1, 1);

  GtkWidget *labelPdfZoomInShortcut = gtk_label_new(misc_get_pango_string("+", 1));
  gtk_widget_show(labelPdfZoomInShortcut);
  gtk_label_set_use_markup (GTK_LABEL (labelPdfZoomInShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelPdfZoomInShortcut, 6, 6, 1, 1);

  GtkWidget *labelPdfZoomOutShortcuts = gtk_label_new(_("<b>PDF : zoom out</b>"));
  gtk_label_set_use_markup (GTK_LABEL (labelPdfZoomOutShortcuts), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelPdfZoomOutShortcuts, 5, 7, 1, 1);

  GtkWidget *labelPdfZoomOutShortcut = gtk_label_new(misc_get_pango_string("-", 1));
  gtk_label_set_use_markup (GTK_LABEL (labelPdfZoomOutShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelPdfZoomOutShortcut, 6, 7, 1, 1);
/* sketch */
  GtkWidget *labelSketchPenUpShortcuts = gtk_label_new(_("<b>Sketch : larger pen</b>"));
  gtk_label_set_use_markup (GTK_LABEL (labelSketchPenUpShortcuts), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelSketchPenUpShortcuts, 5, 8, 1, 1);

  GtkWidget *labelSketchPenUpShortcut = gtk_label_new(misc_get_pango_string("+", 1));
  gtk_label_set_use_markup (GTK_LABEL (labelSketchPenUpShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelSketchPenUpShortcut, 6, 8, 1, 1);

  GtkWidget *labelSketchPenDwnShortcuts = gtk_label_new(_("<b>Sketch : leaner pen</b>"));
  gtk_label_set_use_markup (GTK_LABEL (labelSketchPenDwnShortcuts), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelSketchPenDwnShortcuts, 5, 9, 1, 1);

  GtkWidget *labelSketchPenDwnShortcut = gtk_label_new(misc_get_pango_string("-", 1));
  gtk_label_set_use_markup (GTK_LABEL (labelSketchPenDwnShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelSketchPenDwnShortcut, 6, 9, 1, 1);
/* application */
  GtkWidget *labelOthersHelpShortcuts = gtk_label_new(_("<b>Help</b>"));
  gtk_label_set_use_markup (GTK_LABEL (labelOthersHelpShortcuts), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelOthersHelpShortcuts, 7, 1, 1, 1);

  GtkWidget *labelOtherHelpShortcut = gtk_label_new(misc_get_pango_string("F1", 1));
  gtk_label_set_use_markup (GTK_LABEL (labelOtherHelpShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelOtherHelpShortcut, 8, 1, 1, 1);

  GtkWidget *labelOthersPasteImageShortcuts = gtk_label_new(_("<b>Paste image</b>"));
  gtk_label_set_use_markup (GTK_LABEL (labelOthersPasteImageShortcuts), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelOthersPasteImageShortcuts, 7, 2, 1, 1);

  GtkWidget *labelOtherPasteImageShortcut = gtk_label_new(misc_get_pango_string("v", 1));
  gtk_widget_show(labelOtherPasteImageShortcut);
  gtk_label_set_use_markup (GTK_LABEL (labelOtherPasteImageShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelOtherPasteImageShortcut, 8, 2, 1, 1);

  GtkWidget *labelOthersStorageShortcuts = gtk_label_new(_("<b>Call main menu</b>"));
  gtk_widget_show(labelOthersStorageShortcuts);
  gtk_label_set_use_markup (GTK_LABEL (labelOthersStorageShortcuts), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelOthersStorageShortcuts, 7, 3, 1, 1);

  GtkWidget *labelOthersStorageShortcut = gtk_label_new(misc_get_pango_string("m", 1));
  gtk_label_set_use_markup (GTK_LABEL (labelOthersStorageShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelOthersStorageShortcut, 8, 3, 1, 1);

  GtkWidget *labelOthersQuickSaveShortcuts = gtk_label_new(_("<b>Quick save</b>"));
  gtk_label_set_use_markup (GTK_LABEL (labelOthersQuickSaveShortcuts), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelOthersQuickSaveShortcuts, 7, 4, 1, 1);

  GtkWidget *labelOthersQuickSaveShortcut = gtk_label_new(misc_get_pango_string("s", 1));
  gtk_label_set_use_markup (GTK_LABEL (labelOthersQuickSaveShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelOthersQuickSaveShortcut, 8, 4, 1, 1);

  GtkWidget *labelOthersLoadPDFShortcuts = gtk_label_new(_("<b>Load PDF file </b>"));
  gtk_label_set_use_markup (GTK_LABEL (labelOthersLoadPDFShortcuts), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelOthersLoadPDFShortcuts, 7, 5, 1, 1);

  GtkWidget *labelOthersLoadPDFShortcut = gtk_label_new(misc_get_pango_string("d", 1));
  gtk_widget_show(labelOthersLoadPDFShortcut);
  gtk_label_set_use_markup (GTK_LABEL (labelOthersLoadPDFShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelOthersLoadPDFShortcut, 8, 5, 1, 1);

  GtkWidget *labelOthersFindShortcuts = gtk_label_new(_("<b>Find for selected text</b>"));
  gtk_widget_show(labelOthersFindShortcuts);
  gtk_label_set_use_markup (GTK_LABEL (labelOthersFindShortcuts), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelOthersFindShortcuts, 7, 6, 1, 1);

  GtkWidget *labelOthersFindShortcut = gtk_label_new(misc_get_pango_string("f", 1));
  gtk_label_set_use_markup (GTK_LABEL (labelOthersFindShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelOthersFindShortcut, 8, 6, 1, 1);

  GtkWidget *labelOthersHideShortcuts = gtk_label_new(_("<b>Hide/show main toolbar</b>"));
  gtk_widget_show(labelOthersHideShortcuts);
  gtk_label_set_use_markup (GTK_LABEL (labelOthersHideShortcuts), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelOthersHideShortcuts, 7, 7, 1, 1);

  GtkWidget *labelOthersHideShortcut = gtk_label_new(misc_get_pango_string("F10", 1));
  gtk_label_set_use_markup (GTK_LABEL (labelOthersHideShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelOthersHideShortcut, 8, 7, 1, 1);

  GtkWidget *labelOthersEditorPaneShortcuts = gtk_label_new(_("<b>Switch to editor pane</b>"));
  gtk_label_set_use_markup (GTK_LABEL (labelOthersEditorPaneShortcuts), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelOthersEditorPaneShortcuts, 7, 8, 1, 1);

  GtkWidget *labelOthersEditorPaneShortcut = gtk_label_new(misc_get_pango_string("1", 1));
  gtk_label_set_use_markup (GTK_LABEL (labelOthersEditorPaneShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelOthersEditorPaneShortcut, 8, 8, 1, 1);

  GtkWidget *labelOthersPDFPaneShortcuts = gtk_label_new(_("<b>Switch to PDF pane</b>"));
  gtk_label_set_use_markup (GTK_LABEL (labelOthersPDFPaneShortcuts), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelOthersPDFPaneShortcuts, 7, 9, 1, 1);

  GtkWidget *labelOthersPDFPaneShortcut = gtk_label_new(misc_get_pango_string("2", 1));
  gtk_label_set_use_markup (GTK_LABEL (labelOthersPDFPaneShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelOthersPDFPaneShortcut, 8, 9, 1, 1);


  GtkWidget *labelOthersSketchPaneShortcuts = gtk_label_new(_("<b>Switch to sketch pane</b>"));
  gtk_label_set_use_markup (GTK_LABEL (labelOthersSketchPaneShortcuts), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelOthersSketchPaneShortcuts, 7, 10, 1, 1);

  GtkWidget *labelOthersSketchPaneShortcut = gtk_label_new(misc_get_pango_string("3", 1));
  gtk_label_set_use_markup (GTK_LABEL (labelOthersSketchPaneShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelOthersSketchPaneShortcut, 8, 10, 1, 1);



  GtkWidget *labelOthersQuitShortcuts = gtk_label_new(_("<b>Quit application</b>"));
  gtk_label_set_use_markup (GTK_LABEL (labelOthersQuitShortcuts), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelOthersQuitShortcuts, 7, 11, 1, 1);

  GtkWidget *labelOthersQuitShortcut = gtk_label_new(misc_get_pango_string("q", 1));
  gtk_label_set_use_markup (GTK_LABEL (labelOthersQuitShortcut), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelOthersQuitShortcut, 8, 11, 1, 1);

  GtkWidget *labelRedacVersion=gtk_label_new("<small><i>Redac version 0.1 - (c) L.A. 2018-2019</i></small>");
  gtk_label_set_use_markup (GTK_LABEL (labelRedacVersion), TRUE);
  gtk_grid_attach(GTK_GRID (gridDialog), labelRedacVersion, 0, 12, 6, 1);


  /* buttons */
  dialog_action_area11 = gtk_dialog_get_action_area (GTK_DIALOG (helpDialog));
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area11), GTK_BUTTONBOX_CENTER);

  okbutton6=  gtk_button_new_with_label (_("Ok"));
  imageOkButton6 = gtk_image_new_from_icon_name ("gtk-ok",  GTK_ICON_SIZE_BUTTON);
  gtk_button_set_always_show_image (GTK_BUTTON (okbutton6), TRUE);
  gtk_button_set_image (GTK_BUTTON (okbutton6), imageOkButton6);

  gtk_dialog_add_action_widget (GTK_DIALOG (helpDialog), okbutton6, GTK_RESPONSE_OK);
  gtk_widget_set_can_default (okbutton6, TRUE);
  gtk_widget_show_all(helpDialog);
  return helpDialog;
}

/**************************************************************
  dialog to set up an annotation inside the PDF or Sketch 
  with option to choose font
**************************************************************/
gchar *dialog_add_text_annotation(GtkWidget *win, gchar *current_str, APP_data *data)
{
  GtkWidget *annotDialog;
  GtkWidget *gridDialog;
  GtkWidget *dialog_vbox11;
  GtkWidget *entry;
  GtkTextBuffer *buffer;
  GtkWidget *dialog_action_area11;
  GtkWidget *okbutton6;
  GtkWidget *imageOkButton6;
  GtkWidget *cancelbutton;
  GtkWidget *imagecancelbutton;
  GtkWidget *labelFont;
  GtkWidget *font_button;
  GtkWidget *separator;
  gchar *newFont;
  gchar *tmpStr=NULL;
  gint ret;
  GKeyFile *keyString;

  keyString = g_object_get_data(G_OBJECT(win), "config"); 
  annotDialog = gtk_dialog_new ();
  gtk_window_set_default_size(GTK_WINDOW(annotDialog), 240, 180);
  gtk_window_set_title (GTK_WINDOW (annotDialog), _("New Annotation ..."));
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
  buffer=gtk_text_buffer_new(NULL);
  entry = gtk_text_view_new_with_buffer (buffer);
  gtk_widget_set_hexpand (entry,TRUE);
  gtk_widget_set_vexpand (entry,TRUE);
  gtk_text_buffer_set_text(buffer, current_str, -1);
  gtk_widget_show(entry);
  gtk_grid_attach(GTK_GRID (gridDialog), entry, 0, 0, 2, 1);

  separator=gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_widget_show(separator);
  gtk_grid_attach(GTK_GRID (gridDialog), separator, 0, 1, 2, 1);
  /* font - only for Sketch */
  labelFont=gtk_label_new(_("Current font : "));
  gtk_widget_show(labelFont);
  gtk_grid_attach(GTK_GRID (gridDialog), labelFont, 0, 2, 1, 1);
  font_button=gtk_font_button_new_with_font ("sans 14");
  gtk_widget_show(font_button);
  gtk_grid_attach(GTK_GRID(gridDialog),font_button, 1, 2 , 1, 1);
  gtk_font_chooser_set_font (GTK_FONT_CHOOSER(font_button), 
                 g_key_file_get_string(keyString, "sketch", "font", NULL));
  if(data->currentStack==CURRENT_STACK_SKETCH) {
    gtk_widget_set_sensitive(GTK_WIDGET(labelFont), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(font_button), TRUE);
  } else {
    gtk_widget_set_sensitive(GTK_WIDGET(labelFont), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(font_button), FALSE);
  }
  /* action area */
  /* buttons */
  dialog_action_area11 = gtk_dialog_get_action_area (GTK_DIALOG (annotDialog));
  gtk_widget_show (dialog_action_area11);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area11), GTK_BUTTONBOX_CENTER);

  okbutton6=  gtk_button_new_with_label (_("Ok"));
  imageOkButton6 = gtk_image_new_from_icon_name ("gtk-ok",  GTK_ICON_SIZE_BUTTON);
  gtk_button_set_always_show_image (GTK_BUTTON (okbutton6), TRUE);
  gtk_button_set_image (GTK_BUTTON (okbutton6), imageOkButton6);
  gtk_widget_show (okbutton6);

  cancelbutton= gtk_button_new_with_label (_("Cancel"));
  imagecancelbutton = gtk_image_new_from_icon_name ("gtk-cancel",  GTK_ICON_SIZE_BUTTON);
  gtk_button_set_always_show_image (GTK_BUTTON (cancelbutton), TRUE);
  gtk_button_set_image (GTK_BUTTON (cancelbutton), imagecancelbutton);
  gtk_widget_show (cancelbutton);
  gtk_dialog_add_action_widget (GTK_DIALOG (annotDialog), cancelbutton, GTK_RESPONSE_CANCEL);
  gtk_dialog_add_action_widget (GTK_DIALOG (annotDialog), okbutton6, GTK_RESPONSE_OK);
  gtk_widget_set_can_default (okbutton6, TRUE);

  /* run */
  ret=gtk_dialog_run(GTK_DIALOG(annotDialog));
  if(ret==GTK_RESPONSE_OK) {
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter (buffer,&start);
    gtk_text_buffer_get_end_iter (buffer,&end);
    tmpStr=gtk_text_buffer_get_text (buffer,&start, &end,FALSE);
    /* get the fonts */
    newFont = gtk_font_chooser_get_font(GTK_FONT_CHOOSER(font_button));
    if(newFont!=NULL) {
       g_key_file_set_string(keyString, "sketch", "font",newFont);
       g_free(newFont);
    }
  }
  gtk_widget_destroy (GTK_WIDGET(annotDialog));
  return tmpStr;
}


/******************
preferences dialog

*******************/
GtkWidget *create_prefs_dialog(GtkWidget *win, APP_data *data_app)
{
  GKeyFile *keyString;
  GdkRGBA color;   
  gchar *newFont;
  gdouble rewGap, jumpGap, pen_width;

  GtkWidget *configDialog;
  GtkWidget *dialog_vbox5;
  GtkWidget *configNotebook;
  GtkWidget *vbox26;
  GtkWidget *configAutoSave;
  GtkWidget *configAutoReloadPDF;
  GtkWidget *configPromptQuit;
  GtkWidget *configPromptOverwrite;
  GtkWidget *configAutoRewindPlayer;

  GtkWidget *font_button_editor;
  GtkWidget *font_button_sketch;
  GtkWidget *color_button_editor_fg;
  GtkWidget *color_button_editor_bg;
  GtkWidget *color_button_PDF_bg;
  GtkWidget *color_button_sketch_bg;

  GtkWidget *label246;
  GtkWidget *vbox29;
  GtkWidget *table6;
  GtkWidget *label249;
  GtkWidget *label250;
  GtkWidget *label271; 
  GtkWidget *label993;
  GtkWidget *label264;
  GtkWidget *label991;
  GtkWidget *vbox28;
  GtkWidget *label274;
  GtkWidget *label992;
  GtkWidget *label273;
  GtkWidget *labelAudio;
  GtkWidget *label_pen_width;
  GtkWidget *vboxAudio;
  GtkWidget *dialog_action_area5;
  GtkWidget *cancelbutton2;
  GtkWidget *imagecancelbutton2;
  GtkWidget *okbutton3;
  GtkWidget *imageokbutton3;

  configDialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (configDialog), _("Settings ..."));
  gtk_window_set_modal (GTK_WINDOW (configDialog), TRUE);

  gtk_window_set_transient_for (GTK_WINDOW (configDialog), GTK_WINDOW(data_app->appWindow)); 
  gtk_window_set_destroy_with_parent (GTK_WINDOW (configDialog), TRUE);
  gtk_window_set_icon_name (GTK_WINDOW (configDialog), "gtk-preferences");
  gtk_window_set_type_hint (GTK_WINDOW (configDialog), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialog_vbox5 = gtk_dialog_get_content_area (GTK_DIALOG (configDialog));
  gtk_box_set_spacing (GTK_BOX(dialog_vbox5), 2);
  gtk_widget_show (dialog_vbox5);

  configNotebook = gtk_notebook_new ();
  gtk_container_set_border_width (GTK_CONTAINER (configNotebook), 4);
  gtk_widget_show (configNotebook);
  gtk_box_pack_start (GTK_BOX (dialog_vbox5), configNotebook, TRUE, TRUE, 4);

  vbox26 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_widget_show (vbox26);
  gtk_container_add (GTK_CONTAINER (configNotebook), vbox26);

  configAutoSave = gtk_check_button_new_with_mnemonic (_("Save current editor document every 5 minutes"));
  gtk_widget_show (configAutoSave);
  gtk_box_pack_start (GTK_BOX (vbox26), configAutoSave, FALSE, FALSE, 5);
  gtk_widget_set_sensitive (GTK_WIDGET(configAutoSave), TRUE);

  configAutoReloadPDF = gtk_check_button_new_with_mnemonic (_("Reload last used PDF at program's start "));
  gtk_widget_show (configAutoReloadPDF);
  gtk_box_pack_start (GTK_BOX (vbox26), configAutoReloadPDF, FALSE, FALSE, 0);
  gtk_widget_set_tooltip_text ( configAutoReloadPDF, _("If available in your system, the last PDF document used will be automatically\nreload when the programm starts."));

  configPromptQuit = gtk_check_button_new_with_mnemonic (_("Ask before quit program"));
  gtk_widget_show (configPromptQuit);
  gtk_box_pack_start (GTK_BOX (vbox26), configPromptQuit, FALSE, FALSE, 0);
  gtk_widget_set_tooltip_text ( configPromptQuit, _("Force a prompt before quit program. Take it easy, your work will be automatically saved\nwhatever you check or not this option."));

  configPromptOverwrite = gtk_check_button_new_with_mnemonic (_("Ask before overwriting files"));
  gtk_widget_show (configPromptOverwrite);
  gtk_box_pack_start (GTK_BOX (vbox26), configPromptOverwrite, FALSE, FALSE, 0);
  gtk_widget_set_tooltip_text ( configPromptOverwrite, _("Force a prompt before any files are overwrote"));

  label246 = gtk_label_new (_("File management"));
  gtk_widget_show (label246);

/* Display Icon in the tab - thanks to old developpers  here ;  http://vim.1045645.n5.nabble.com/Patch-Nicer-notebook-tabs-with-GTK2-td1206288.html */
  GtkWidget   *tab_boxGlobalSettings = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
  gtk_widget_show(tab_boxGlobalSettings);
  GtkWidget    *iconGlobalSettings = gtk_image_new_from_icon_name ("document-save", GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_widget_show(iconGlobalSettings);
  gtk_misc_set_padding(GTK_MISC(iconGlobalSettings), 0, 2);
  gtk_box_pack_start(GTK_BOX(tab_boxGlobalSettings), iconGlobalSettings, FALSE, FALSE, 2);
  gtk_misc_set_padding(GTK_MISC(label246), 2, 2);
  gtk_container_add(GTK_CONTAINER(tab_boxGlobalSettings), label246); 
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (configNotebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (configNotebook), 0), tab_boxGlobalSettings);

  vbox29 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_widget_show (vbox29);
  gtk_container_add (GTK_CONTAINER (configNotebook), vbox29);

  table6 = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID(table6), 4);
  gtk_widget_show (table6);
  gtk_box_pack_start (GTK_BOX (vbox29), table6, TRUE, TRUE, 4);

  label249 = gtk_label_new (_("Text font:"));
  gtk_misc_set_alignment (GTK_MISC (label249), 1, 0.5);
  gtk_widget_show (label249);
  gtk_grid_attach(GTK_GRID(table6),label249, 0, 1 , 1, 1);
  gtk_misc_set_padding (GTK_MISC (label249), 5, 0);

  font_button_editor=gtk_font_button_new_with_font ("sans 12");
  gtk_widget_show(font_button_editor);
  gtk_grid_attach(GTK_GRID(table6),font_button_editor, 1, 1 , 1, 1);


  label250 = gtk_label_new (_("Text background color:"));
  gtk_misc_set_alignment (GTK_MISC (label250), 1, 0.5);
  gtk_widget_show (label250);
  gtk_grid_attach(GTK_GRID(table6),label250, 0, 2 , 1, 1);
  gtk_misc_set_padding (GTK_MISC (label250), 5, 0);

  color_button_editor_bg=gtk_color_button_new (); 
  gtk_widget_show (color_button_editor_bg);
  gtk_grid_attach(GTK_GRID(table6),color_button_editor_bg, 1, 2 , 1, 1);

  label271 = gtk_label_new (_("Text foreground color:"));
  gtk_misc_set_alignment (GTK_MISC (label271), 1, 0.5);
  gtk_widget_show (label271);
  gtk_grid_attach(GTK_GRID(table6),label271, 0, 3 , 1, 1);
  gtk_misc_set_padding (GTK_MISC (label271), 5, 0);

  color_button_editor_fg=gtk_color_button_new (); 
  gtk_widget_show (color_button_editor_fg);
  gtk_grid_attach(GTK_GRID(table6),color_button_editor_fg, 1, 3 , 1, 1); 

  label993 = gtk_label_new (_("Editor"));
  gtk_widget_show (label993);
  GtkWidget   *tab_boxApplicationsSettings = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
  gtk_widget_show(tab_boxApplicationsSettings);
  GtkWidget    *iconApplicationsSettings = gtk_image_new_from_icon_name ("accessories-text-editor", GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_widget_show(iconApplicationsSettings);
  gtk_misc_set_padding(GTK_MISC(iconApplicationsSettings), 0, 2);
  gtk_box_pack_start(GTK_BOX(tab_boxApplicationsSettings), iconApplicationsSettings, FALSE, FALSE, 2);
  gtk_misc_set_padding(GTK_MISC(label993), 2, 2);
  gtk_container_add(GTK_CONTAINER(tab_boxApplicationsSettings), label993); 
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (configNotebook), 
                              gtk_notebook_get_nth_page (GTK_NOTEBOOK (configNotebook), 1), tab_boxApplicationsSettings);

  GtkWidget *hbox27 = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID(hbox27), 4);
  gtk_widget_show (hbox27);
  gtk_container_add (GTK_CONTAINER (configNotebook), hbox27);

  label264 = gtk_label_new (_("Document paper color : "));
  gtk_misc_set_alignment (GTK_MISC (label264), 1, 0.5);
  gtk_widget_show (label264);
  gtk_grid_attach(GTK_GRID(hbox27),label264, 0, 0 , 1, 1);

  color_button_PDF_bg=gtk_color_button_new (); 
  gtk_widget_show (color_button_PDF_bg);
  gtk_grid_attach(GTK_GRID(hbox27),color_button_PDF_bg, 1, 0 , 1, 1);

  GtkWidget *label2641 = gtk_label_new (_("<i>(Please reload PDF document to complete operation)</i>"));
  gtk_label_set_use_markup (GTK_LABEL (label2641), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label2641), 1, 0.5);
  gtk_widget_show (label2641);
  gtk_grid_attach(GTK_GRID(hbox27),label2641, 0, 1 , 2, 1);


  label991 = gtk_label_new (_("PDF document"));
  gtk_widget_show (label991);

  GtkWidget *tab_boxCSVSettings = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
  gtk_widget_show(tab_boxCSVSettings);
  GtkWidget *iconCSVSettings = gtk_image_new_from_icon_name ("application-pdf", GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_widget_show(iconCSVSettings);
  gtk_misc_set_padding(GTK_MISC(iconCSVSettings), 0, 2);
  gtk_box_pack_start(GTK_BOX(tab_boxCSVSettings), iconCSVSettings, FALSE, FALSE, 2);
  gtk_misc_set_padding(GTK_MISC(label991), 2, 2);
  gtk_container_add(GTK_CONTAINER(tab_boxCSVSettings), label991); 
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (configNotebook), 
                              gtk_notebook_get_nth_page (GTK_NOTEBOOK (configNotebook), 2),       tab_boxCSVSettings);
  /* sketch section */
  vbox28 = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID(vbox28), 4);
  gtk_widget_show (vbox28);
  gtk_container_add (GTK_CONTAINER (configNotebook), vbox28);

  label274 = gtk_label_new (_("Paper color : "));
  gtk_label_set_use_markup (GTK_LABEL (label274), TRUE);
  gtk_widget_show (label274);
  gtk_grid_attach(GTK_GRID(vbox28),label274, 0, 0 , 1, 1);
  gtk_misc_set_alignment (GTK_MISC (label274), 1, 0.5);
 
  GtkWidget *label2742 = gtk_label_new (_("<i>(Remain unchanged until you clear sketch)</i>"));
  gtk_label_set_use_markup (GTK_LABEL (label2742), TRUE);
  gtk_widget_show (label2742);
  gtk_grid_attach(GTK_GRID(vbox28),label2742, 0, 1 , 2, 1);
  gtk_misc_set_alignment (GTK_MISC (label2742), 1, 0.5);

  color_button_sketch_bg=gtk_color_button_new (); 
  gtk_widget_show (color_button_sketch_bg);
  gtk_grid_attach(GTK_GRID(vbox28),color_button_sketch_bg, 1, 0 , 1, 1);
 
  label992 = gtk_label_new (_("Annotation text font:"));
  gtk_widget_show (label992);
  gtk_grid_attach(GTK_GRID(vbox28),label992, 0, 2 , 1, 1);
  gtk_misc_set_alignment (GTK_MISC (label992), 1, 0.5);

  font_button_sketch=gtk_font_button_new_with_font ("sans 14");
  gtk_widget_show(font_button_sketch);
  gtk_grid_attach(GTK_GRID(vbox28),font_button_sketch, 1, 2 , 1, 1);

  label_pen_width = gtk_label_new (_("Pen width in pixels :"));
  gtk_widget_show (label_pen_width);
  gtk_grid_attach(GTK_GRID(vbox28),label_pen_width, 0, 3 , 1, 1);
  gtk_misc_set_alignment (GTK_MISC (label_pen_width), 1, 0.5);

  GtkAdjustment *pen_width_adj = gtk_adjustment_new (1, 1, 20, 1, 10, 0);
  GtkWidget *pen_width_Spin = gtk_spin_button_new (GTK_ADJUSTMENT (pen_width_adj), 2, 1);
  gtk_widget_show (pen_width_Spin);
  gtk_grid_attach(GTK_GRID(vbox28),pen_width_Spin, 1, 3 , 1, 1);

  label273 = gtk_label_new (_("Sketch"));
  gtk_widget_show (label273);
 
  GtkWidget *tab_boxSystemSettings = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
  gtk_widget_show(tab_boxSystemSettings);
  GtkWidget *iconSystemSettings = gtk_image_new_from_icon_name ("applications-graphics", GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_widget_show(iconSystemSettings);
  gtk_misc_set_padding(GTK_MISC(iconSystemSettings), 0, 2);
  gtk_box_pack_start(GTK_BOX(tab_boxSystemSettings), iconSystemSettings, FALSE, FALSE, 2);
  gtk_misc_set_padding(GTK_MISC(label273), 2, 2);
  gtk_container_add(GTK_CONTAINER(tab_boxSystemSettings), label273); 
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (configNotebook), 
                             gtk_notebook_get_nth_page (GTK_NOTEBOOK (configNotebook), 3),       
                             tab_boxSystemSettings);

  /* audio settings section */
  vboxAudio = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID(vboxAudio), 4);
  gtk_widget_show (vboxAudio);
  gtk_container_add (GTK_CONTAINER (configNotebook), vboxAudio);
  GtkWidget *labelAudioRew = gtk_label_new (_("<b>Play/pause rewind jump in seconds.</b>\n<i>Minimum : 1sec, maximum 10 secs.</i>"));
  gtk_misc_set_alignment (GTK_MISC (labelAudioRew), 1, 0.5);
  gtk_label_set_use_markup (GTK_LABEL (labelAudioRew), TRUE);
  gtk_widget_show (labelAudioRew);
  gtk_grid_attach(GTK_GRID(vboxAudio),labelAudioRew, 0, 0 , 1, 1);
  gtk_misc_set_alignment (GTK_MISC (labelAudioRew), 0, 0.5);
  GtkAdjustment *rewGap_adj = gtk_adjustment_new (2, 1, 10, 1, 1, 0);
  GtkWidget *rewGapSpin = gtk_spin_button_new (GTK_ADJUSTMENT (rewGap_adj), 1, 0);
  gtk_widget_show (rewGapSpin);
  gtk_grid_attach(GTK_GRID(vboxAudio),rewGapSpin, 1, 0 , 1, 1);
  GtkWidget *labelAudioStep = gtk_label_new (_("<b>Back or forward jump in seconds.</b>\n<i>Minimum : 10sec, maximum 600 secs (10 minutes).</i>"));
  gtk_misc_set_alignment (GTK_MISC (labelAudioStep), 1, 0.5);
  gtk_label_set_use_markup (GTK_LABEL (labelAudioStep), TRUE);
  gtk_widget_show (labelAudioStep);
  gtk_grid_attach(GTK_GRID(vboxAudio),labelAudioStep, 0, 2 , 1, 1);
  gtk_misc_set_alignment (GTK_MISC (labelAudioStep), 0, 0.5);
  GtkAdjustment *jumpGap_adj = gtk_adjustment_new (10, 1, 600, 1, 10, 0);
  GtkWidget *jumpGapSpin = gtk_spin_button_new (GTK_ADJUSTMENT (jumpGap_adj), 1, 0);
  gtk_widget_show (jumpGapSpin);
  gtk_grid_attach(GTK_GRID(vboxAudio),jumpGapSpin, 1, 2 , 1, 1);

  GtkWidget *separator=gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_widget_show(separator);
  g_object_set(separator, "margin-top", 24, NULL);
  gtk_grid_attach(GTK_GRID(vboxAudio),separator, 0, 3 , 2, 1);
  configAutoRewindPlayer= gtk_check_button_new_with_mnemonic (_("Auto rewind to start after playing completed."));
  gtk_widget_show (configAutoRewindPlayer);
  gtk_grid_attach(GTK_GRID(vboxAudio),configAutoRewindPlayer, 0, 4 , 1, 1);
  gtk_widget_set_sensitive (GTK_WIDGET(configAutoRewindPlayer), TRUE);

  labelAudio=gtk_label_new (_("Audio"));
  gtk_widget_show (labelAudio);
  GtkWidget   *tab_boxAudioSettings = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
  gtk_widget_show(tab_boxAudioSettings);
  GtkWidget    *iconAudioSettings = gtk_image_new_from_icon_name ("audio-x-generic", GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_widget_show(iconAudioSettings);
  gtk_misc_set_padding(GTK_MISC(iconAudioSettings), 0, 2);
  gtk_box_pack_start(GTK_BOX(tab_boxAudioSettings), iconAudioSettings, FALSE, FALSE, 2);
  gtk_misc_set_padding(GTK_MISC(labelAudio), 2, 2);
  gtk_container_add(GTK_CONTAINER(tab_boxAudioSettings), labelAudio); 
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (configNotebook), 
                             gtk_notebook_get_nth_page (GTK_NOTEBOOK (configNotebook), 4),       
                             tab_boxAudioSettings);

  dialog_action_area5 = gtk_dialog_get_action_area (GTK_DIALOG (configDialog));
  gtk_widget_show (dialog_action_area5);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area5), GTK_BUTTONBOX_END);

  cancelbutton2=  gtk_button_new_with_label (_("Cancel"));
  imagecancelbutton2 = gtk_image_new_from_icon_name ("gtk-cancel",  GTK_ICON_SIZE_BUTTON);
  gtk_button_set_always_show_image (GTK_BUTTON (cancelbutton2), TRUE);
  gtk_button_set_image (GTK_BUTTON (cancelbutton2), imagecancelbutton2);
  gtk_widget_show (cancelbutton2);
  gtk_dialog_add_action_widget (GTK_DIALOG (configDialog), cancelbutton2, GTK_RESPONSE_CANCEL);
  gtk_widget_set_can_default (cancelbutton2, TRUE);

  okbutton3 = gtk_button_new_with_label (_("Ok"));
  imageokbutton3=gtk_image_new_from_icon_name ("gtk-ok",  GTK_ICON_SIZE_BUTTON);
  gtk_button_set_always_show_image (GTK_BUTTON (okbutton3), TRUE);
  gtk_button_set_image (GTK_BUTTON (okbutton3), imageokbutton3);
  gtk_widget_show (okbutton3);
  gtk_dialog_add_action_widget (GTK_DIALOG (configDialog), okbutton3, GTK_RESPONSE_OK);
  gtk_widget_set_can_default (okbutton3, TRUE);

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

  color.red=g_key_file_get_double(keyString, "editor", "text.color.red", NULL);
  color.green=g_key_file_get_double(keyString, "editor", "text.color.green", NULL);
  color.blue=g_key_file_get_double(keyString, "editor", "text.color.blue", NULL);
  color.alpha=1;
  gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER(color_button_editor_fg), &color);

  color.red=g_key_file_get_double(keyString, "editor", "paper.color.red", NULL);
  color.green=g_key_file_get_double(keyString, "editor", "paper.color.green", NULL);
  color.blue=g_key_file_get_double(keyString, "editor", "paper.color.blue", NULL);
  color.alpha=1;
  gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER(color_button_editor_bg), &color);

  color.red=g_key_file_get_double(keyString, "reference-document", "paper.color.red", NULL);
  color.green=g_key_file_get_double(keyString, "reference-document", "paper.color.green", NULL);
  color.blue=g_key_file_get_double(keyString, "reference-document", "paper.color.blue", NULL);
  color.alpha=1;
  gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER(color_button_PDF_bg), &color);    

  color.red=g_key_file_get_double(keyString, "sketch", "paper.color.red", NULL);
  color.green=g_key_file_get_double(keyString, "sketch", "paper.color.green", NULL);
  color.blue=g_key_file_get_double(keyString, "sketch", "paper.color.blue", NULL);
  color.alpha=1;
  gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER(color_button_sketch_bg), &color);
 
  gtk_font_chooser_set_font (GTK_FONT_CHOOSER(font_button_editor ), 
                 g_key_file_get_string(keyString, "editor", "font", NULL));
  gtk_font_chooser_set_font (GTK_FONT_CHOOSER(font_button_sketch ), 
                 g_key_file_get_string(keyString, "sketch", "font", NULL));
  pen_width=g_key_file_get_double(keyString, "sketch", "pen-width", NULL);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(pen_width_Spin), pen_width);

  rewGap=g_key_file_get_double(keyString, "application", "audio-file-rewind-step", NULL);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(rewGapSpin), rewGap);
  jumpGap=g_key_file_get_double(keyString, "application", "audio-file-marks-step", NULL);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(jumpGapSpin), jumpGap);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (configAutoRewindPlayer),
            g_key_file_get_boolean(keyString, "application", "audio-auto-rewind",NULL ));
  /* callbacks for spin buttons - not useful*/
  g_signal_connect ((gpointer) rewGapSpin, "value-changed",
                    G_CALLBACK (on_rewGapSpin_value_changed_event), data_app);
 
/*                   NULL);
  g_signal_connect ((gpointer) jumpGapSpin, "value-changed",
                    G_CALLBACK (on_jumpGapSpin_value_changed_event),
                    NULL);
*/
  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (configDialog, configDialog, "configDialog");
  GLADE_HOOKUP_OBJECT_NO_REF (configDialog, dialog_vbox5, "dialog_vbox5");
  GLADE_HOOKUP_OBJECT (configDialog, configNotebook, "configNotebook");
  GLADE_HOOKUP_OBJECT (configDialog, vbox26, "vbox26");
  GLADE_HOOKUP_OBJECT (configDialog, configAutoSave, "configAutoSave");
  GLADE_HOOKUP_OBJECT (configDialog, configAutoReloadPDF, "configAutoReloadPDF");
  GLADE_HOOKUP_OBJECT (configDialog, configPromptQuit, "configPromptQuit");
  GLADE_HOOKUP_OBJECT (configDialog, configPromptOverwrite, "configPromptOverwrite");
  GLADE_HOOKUP_OBJECT (configDialog, color_button_PDF_bg, "color_button_PDF_bg");
  GLADE_HOOKUP_OBJECT (configDialog, color_button_editor_bg, "color_button_editor_bg");
  GLADE_HOOKUP_OBJECT (configDialog, color_button_editor_fg, "color_button_editor_fg");
  GLADE_HOOKUP_OBJECT (configDialog, font_button_editor, "font_button_editor");
  GLADE_HOOKUP_OBJECT (configDialog, color_button_sketch_bg, "color_button_sketch_bg");
  GLADE_HOOKUP_OBJECT (configDialog, pen_width_Spin, "pen_width_Spin");
  GLADE_HOOKUP_OBJECT (configDialog, font_button_sketch, "font_button_sketch");
  GLADE_HOOKUP_OBJECT (configDialog, rewGapSpin, "rewGapSpin");
  GLADE_HOOKUP_OBJECT (configDialog, jumpGapSpin, "jumpGapSpin");
  GLADE_HOOKUP_OBJECT (configDialog, configAutoRewindPlayer, "configAutoRewindPlayer");

  GLADE_HOOKUP_OBJECT (configDialog, label246, "label246");
  GLADE_HOOKUP_OBJECT (configDialog, vbox29, "vbox29");
  GLADE_HOOKUP_OBJECT (configDialog, table6, "table6");
  GLADE_HOOKUP_OBJECT (configDialog, label249, "label249");
  GLADE_HOOKUP_OBJECT (configDialog, label250, "label250");
  GLADE_HOOKUP_OBJECT (configDialog, label271, "label271");
  GLADE_HOOKUP_OBJECT (configDialog, label993, "label993");
  GLADE_HOOKUP_OBJECT (configDialog, label264, "label264");
  GLADE_HOOKUP_OBJECT (configDialog, label991, "label991");
  GLADE_HOOKUP_OBJECT (configDialog, vbox28, "vbox28");
  GLADE_HOOKUP_OBJECT (configDialog, label274, "label274");
  GLADE_HOOKUP_OBJECT (configDialog, label992, "label992");
  GLADE_HOOKUP_OBJECT (configDialog, label273, "label273");
  GLADE_HOOKUP_OBJECT_NO_REF (configDialog, dialog_action_area5, "dialog_action_area5");
  GLADE_HOOKUP_OBJECT (configDialog, cancelbutton2, "cancelbutton2");
  GLADE_HOOKUP_OBJECT (configDialog, okbutton3, "okbutton3");

  gtk_widget_grab_default (okbutton3);
  return configDialog;

}

/********************************

  choose color for Annotations
********************************/

GtkWidget*
create_annotationColourDialog (APP_data *data_app, gchar *msg)
{
  GtkWidget *highlightColourDialog;
  GtkWidget *ok_button1;
  GtkWidget *cancel_button1;
  GtkWidget *help_button1;
  GtkWidget *color_selection1;

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
GtkWidget *misc_create_go_jump_dialog(APP_data *data_app)
{
  GtkWidget *goJumpDialog;
  GtkWidget *dialog_vbox11;
  GtkWidget *gridDialog, *gridHeader;
  GtkWidget *labelgoJumpDialog;
  GtkWidget *iconGoJumpDialog;
  GtkWidget *dialog_action_area11;
  GtkWidget *okbutton6;
  GtkWidget *imageOkButton6;
  GtkWidget *cancelbutton;
  GtkWidget *imagecancelbutton;
  GtkWidget *labelHour;
  GtkWidget *labelMinute;
  GtkWidget *labelSecond;
  GtkWidget *labelCurpos;
  GtkWidget *labelTitleCurpos;
  GtkWidget *labelNewPosition;
  gdouble hour, min, sec;

  /* we read and convert to human form the media duration */

  goJumpDialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (goJumpDialog), _("Jump to..."));
  gtk_window_set_position (GTK_WINDOW (goJumpDialog), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (goJumpDialog), TRUE);
  gtk_window_set_resizable (GTK_WINDOW (goJumpDialog), TRUE);
  gtk_window_set_type_hint (GTK_WINDOW (goJumpDialog), GDK_WINDOW_TYPE_HINT_DIALOG);

  gtk_window_set_transient_for (GTK_WINDOW (goJumpDialog),  GTK_WINDOW(data_app->appWindow)); 

  dialog_vbox11 = gtk_dialog_get_content_area (GTK_DIALOG (goJumpDialog));
  gtk_widget_show (dialog_vbox11);
  gridDialog = gtk_grid_new();
  gridHeader = gtk_grid_new();
  g_object_set(gridDialog, "margin-left", 8, NULL);
  g_object_set(gridDialog, "margin-right", 8, NULL);
 // gtk_grid_set_row_homogeneous (GTK_GRID (gridDialog), TRUE); 
  gtk_grid_set_column_homogeneous (GTK_GRID (gridDialog), TRUE); 
  gtk_grid_set_column_homogeneous (GTK_GRID (gridHeader), FALSE); 
  gtk_widget_show (gridDialog);
  gtk_widget_show (gridHeader);
  gtk_grid_attach(GTK_GRID (gridDialog), gridHeader, 0, 0, 3, 1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox11), gridDialog, TRUE, TRUE, 4);

  /* main icon */
  iconGoJumpDialog=gtk_image_new_from_icon_name ("go-jump",GTK_ICON_SIZE_DIALOG);
  gtk_widget_show(iconGoJumpDialog);  
  g_object_set(iconGoJumpDialog, "margin-left", 4, NULL);
  g_object_set(iconGoJumpDialog, "margin-right", 4, NULL);
  gtk_grid_attach(GTK_GRID (gridHeader), iconGoJumpDialog, 0, 0, 1, 1);
  /* labels */
  labelgoJumpDialog = gtk_label_new(_("<b><big>Select a new time position : \n</big></b><i>Please choose a time location within the audio file.\n</i>"));
  gtk_widget_show(labelgoJumpDialog);
  g_object_set(labelgoJumpDialog, "margin-left", 4, NULL);
  g_object_set(labelgoJumpDialog, "margin-right", 4, NULL);
  gtk_label_set_use_markup (GTK_LABEL (labelgoJumpDialog), TRUE);
  gtk_grid_attach(GTK_GRID (gridHeader), labelgoJumpDialog, 1, 0, 1, 1);

  /* various infos */
  labelTitleCurpos= gtk_label_new(_("<b>Current position :</b>"));
  gtk_widget_show(labelTitleCurpos);
  g_object_set(labelTitleCurpos, "margin-left", 4, NULL);
  g_object_set(labelTitleCurpos, "margin-right", 4, NULL);
  gtk_label_set_use_markup (GTK_LABEL (labelTitleCurpos), TRUE);
  gtk_grid_attach(GTK_GRID (gridHeader), labelTitleCurpos, 0, 1, 3, 1);

  labelCurpos= gtk_label_new(g_strdup_printf(_("<i> %s of %s</i>"), 
               audio_gst_time_to_str(data_app->audio_current_position),
               audio_gst_time_to_str(data_app->audio_total_duration)));
  gtk_widget_show(labelCurpos);
  g_object_set(labelCurpos, "margin-left", 4, NULL);
  g_object_set(labelCurpos, "margin-right", 4, NULL);
  gtk_label_set_use_markup (GTK_LABEL (labelCurpos), TRUE);
  gtk_grid_attach(GTK_GRID (gridHeader), labelCurpos, 0, 2, 3, 1);

  labelNewPosition=gtk_label_new(_("<i><b>\nNew position : </b></i>"));
  gtk_widget_show(labelNewPosition);
  g_object_set(labelCurpos, "margin-left", 4, NULL);
  g_object_set(labelCurpos, "margin-right", 4, NULL);
  gtk_label_set_use_markup (GTK_LABEL (labelNewPosition), TRUE);
  gtk_grid_attach(GTK_GRID (gridHeader), labelNewPosition, 0, 3, 3, 1);


  labelHour = gtk_label_new(_("Hour :"));
  gtk_widget_show(labelHour);
  g_object_set(labelHour, "margin-left", 10, NULL);
  g_object_set(labelHour, "margin-right", 4, NULL);
  gtk_grid_attach(GTK_GRID (gridDialog), labelHour, 0, 1, 1, 1); 

  labelMinute = gtk_label_new(_("Minute :"));
  gtk_widget_show(labelMinute);
  g_object_set(labelMinute, "margin-left", 4, NULL);
  g_object_set(labelMinute, "margin-right", 4, NULL);
  gtk_grid_attach(GTK_GRID (gridDialog), labelMinute, 1, 1, 1, 1); 

  labelSecond = gtk_label_new(_("Second :"));
  gtk_widget_show(labelSecond);
  g_object_set(labelSecond, "margin-left", 4, NULL);
  g_object_set(labelSecond, "margin-right", 10, NULL);
  gtk_grid_attach(GTK_GRID (gridDialog), labelSecond, 2, 1, 1, 1); 

  /* adjustments */
  GtkAdjustment *hour_adj = gtk_adjustment_new (0, 0, 23, 1, 10, 0);
  GtkWidget *hourSpin = gtk_spin_button_new (GTK_ADJUSTMENT (hour_adj), 1, 0);
  gtk_widget_show (hourSpin);
  gtk_grid_attach(GTK_GRID(gridDialog),hourSpin, 0, 2 , 1, 1);

  GtkAdjustment *minute_adj = gtk_adjustment_new (0, 0, 59, 1, 10, 0);
  GtkWidget *minuteSpin = gtk_spin_button_new (GTK_ADJUSTMENT (minute_adj), 1, 0);
  gtk_widget_show (minuteSpin);
  gtk_grid_attach(GTK_GRID(gridDialog),minuteSpin, 1, 2 , 1, 1);

  GtkAdjustment *second_adj = gtk_adjustment_new (0, 0, 59, 1, 10, 0);
  GtkWidget *secondSpin = gtk_spin_button_new (GTK_ADJUSTMENT (second_adj), 1, 0);
  gtk_widget_show (secondSpin);
  gtk_grid_attach(GTK_GRID(gridDialog),secondSpin, 2, 2 , 1, 1);

  
  /* buttons */
  dialog_action_area11 = gtk_dialog_get_action_area (GTK_DIALOG (goJumpDialog));
  gtk_widget_show (dialog_action_area11);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area11), GTK_BUTTONBOX_CENTER);

  okbutton6=  gtk_button_new_with_label (_("Ok"));
  imageOkButton6 = gtk_image_new_from_icon_name ("gtk-ok",  GTK_ICON_SIZE_BUTTON);
  gtk_button_set_always_show_image (GTK_BUTTON (okbutton6), TRUE);
  gtk_button_set_image (GTK_BUTTON (okbutton6), imageOkButton6);
  gtk_widget_show (okbutton6);

  cancelbutton=  gtk_button_new_with_label (_("Cancel"));
  imagecancelbutton = gtk_image_new_from_icon_name ("gtk-cancel",  GTK_ICON_SIZE_BUTTON);
  gtk_button_set_always_show_image (GTK_BUTTON (cancelbutton), TRUE);
  gtk_button_set_image (GTK_BUTTON (cancelbutton), imagecancelbutton);
  gtk_widget_show (cancelbutton);
  gtk_dialog_add_action_widget (GTK_DIALOG (goJumpDialog), cancelbutton, GTK_RESPONSE_CANCEL);
  gtk_dialog_add_action_widget (GTK_DIALOG (goJumpDialog), okbutton6, GTK_RESPONSE_OK);
  gtk_widget_set_can_default (okbutton6, TRUE);

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
  gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog),"");
  gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), 
     _("Note  utility written in GTK+ and licensed under GPL v.3"));
  gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), 
      "https://github.com/Luc-Ami/redac/wiki");
  gtk_about_dialog_set_license_type (GTK_ABOUT_DIALOG(dialog), GTK_LICENSE_GPL_3_0);

  gtk_about_dialog_set_authors (GTK_ABOUT_DIALOG(dialog), authors);
  gtk_about_dialog_set_artists (GTK_ABOUT_DIALOG(dialog), artists);
  gtk_about_dialog_set_translator_credits(GTK_ABOUT_DIALOG(dialog), _("Translators credits"));

  gtk_window_set_transient_for (GTK_WINDOW (dialog),
                               GTK_WINDOW(data_app->appWindow));                               
  gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_type_hint (GTK_WINDOW (dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_widget_show_all( dialog );
  return dialog;
}


