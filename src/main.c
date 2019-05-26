
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


/************************
 global vars
*************************/
gchar *gConfigFile = NULL; /* created by main(), destroyed by destroyGKeyFile() */
/**********************

  MAIN

***********************/

int main(int argc, char *argv[]) {
  APP_data app_data;
  GdkRGBA color, b_color;
  GtkStack *stack;
  GtkStackSwitcher *switcher;
  GtkWidget *mainWindow;
  GtkWidget *vGrid;
  GtkWidget *scrolledwindow1;
  GtkWidget *scrolledwindowPDF;
  GtkWidget *viewportPDF;
  GtkWidget *crPDF;
  GtkWidget *scrolledwindowCrobar;
  GtkWidget *toolbar;
  GtkWidget *view;
  GtkWidget *viewPDF;
  GtkWidget *viewCrobar;
  GtkWidget *crCrobar;
  GdkPixbuf *ico;
  GtkWidget *gridStatusBar;
  GKeyFile *keyString;
  GtkTextBuffer *buffer;
  GSList *accelerator_list;
  time_t rawtime;
  gchar *path_to_file, buffer_date[81];

  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  /* test system directories - NEVER free the array os gchars ! */
  // const gchar * const *dirs = g_get_system_data_dirs();
 // unsigned int i;
 // for ( i= 0; dirs[i] != NULL; ++i) {
   //  printf("XDG system datas dirs =%s\n", dirs[i]);
 // }


  gtk_init(&argc, &argv);
  /* Initialize GStreamer */
  gst_init (&argc, &argv);
  /* image directories */
  add_pixmap_directory (PACKAGE_DATA_DIR "/pixmaps/" PACKAGE); /* New location for all pixmaps */
  add_pixmap_directory (PACKAGE_DATA_DIR "/pixmaps"); /* Gnome users /usr/share/pixmaps folder */

  mainWindow = UI_main_window();

  GtkWidget *headBar=gtk_grid_new ();
  /* main vbox packing widget */
  vGrid=gtk_grid_new();
  gtk_grid_set_row_homogeneous (GTK_GRID(vGrid),FALSE);
  gtk_container_add(GTK_CONTAINER(mainWindow), vGrid);
  /* guess the style for current theme */
  check_up_theme( mainWindow, &app_data );
  /* pseudo headerBar */
  UI_headerBar(mainWindow, headBar, &app_data);
  g_object_set(headBar, "margin-top", 4, NULL);
  g_object_set(headBar, "margin-bottom", 4, NULL);
  g_object_set(headBar, "margin-left", 8, NULL);
  gtk_grid_attach(GTK_GRID(vGrid), headBar, 0, 0, 1, 1);
  /* gtkstack definitions and building */
  stack  = GTK_STACK(gtk_stack_new ());
  switcher = GTK_STACK_SWITCHER(gtk_stack_switcher_new ());
  gtk_stack_set_transition_type (stack, GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT);// or GTK_STACK_TRANSITION_TYPE_CROSSFADE
  gtk_stack_switcher_set_stack (switcher, stack);
  gtk_grid_attach(GTK_GRID(headBar),GTK_WIDGET(switcher), 2, 0, 1, 1);
  /* in order to breath some extra space from window's edges */
  g_object_set (stack, "margin-left", 12, NULL);
  g_object_set (stack, "margin-right", 12, NULL);
  gtk_grid_attach(GTK_GRID(vGrid), GTK_WIDGET(stack), 0,2,1,1);
  gtk_widget_set_halign (GTK_WIDGET(switcher), GTK_ALIGN_CENTER);

  /* toolbar */
  toolbar = main_wp_toolbar(mainWindow,  &app_data);
  gtk_grid_attach(GTK_GRID(vGrid), toolbar, 0,1,1,1);
  /* and a scrolling window ! */
  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow1);
  /* and a PDF scrolling window ! */
  scrolledwindowPDF = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindowPDF);
  viewportPDF = gtk_viewport_new (NULL,NULL);
  crPDF = gtk_drawing_area_new();
  gtk_widget_show ( crPDF);

  gtk_widget_set_size_request (crPDF, PDF_VIEW_MAX_WIDTH, PDF_VIEW_MAX_HEIGHT);
  gtk_widget_set_hexpand(crPDF, TRUE);
  gtk_widget_set_vexpand(crPDF, TRUE);
  /* mandatoty : add new events management to gtk_drawing_area ! */
      gtk_widget_set_events (crPDF, gtk_widget_get_events (crPDF)
      | GDK_BUTTON_PRESS_MASK
      | GDK_BUTTON_RELEASE_MASK
      | GDK_POINTER_MOTION_MASK
      | GDK_POINTER_MOTION_HINT_MASK);

  gtk_container_add(GTK_CONTAINER(viewportPDF), crPDF);  
  gtk_container_add(GTK_CONTAINER(scrolledwindowPDF), viewportPDF);
  /* and a paint/draw/hand notes scrolling window ! */
  scrolledwindowCrobar = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindowCrobar);
  viewCrobar = gtk_viewport_new (NULL,NULL);
  crCrobar = gtk_drawing_area_new();
  gtk_widget_set_app_paintable(crCrobar, TRUE);
  gtk_widget_show ( crCrobar);
  gtk_widget_set_size_request (crCrobar, CROBAR_VIEW_MAX_WIDTH, CROBAR_VIEW_MAX_HEIGHT);
  gtk_widget_set_hexpand(crCrobar, TRUE);
  gtk_widget_set_vexpand(crCrobar, TRUE);
  /* mandatoty : add new events management to gtk_drawing_area ! */
      gtk_widget_set_events (crCrobar, gtk_widget_get_events (crCrobar)
      | GDK_BUTTON_PRESS_MASK
      | GDK_BUTTON_RELEASE_MASK
      | GDK_POINTER_MOTION_MASK
      | GDK_POINTER_MOTION_HINT_MASK);

  gtk_container_add(GTK_CONTAINER(viewCrobar), crCrobar);  
  gtk_container_add(GTK_CONTAINER(scrolledwindowCrobar), viewCrobar);

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW (scrolledwindow1),GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindowPDF), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW (scrolledwindowPDF),GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindowCrobar), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW (scrolledwindowCrobar),GTK_SHADOW_ETCHED_IN);


  /* main Gtk text view */
  view = gtk_text_view_new();
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_WORD);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), view);
  gtk_widget_set_name(view, "view");
  gtk_text_view_set_left_margin (GTK_TEXT_VIEW(view), 8);
  gtk_text_view_set_right_margin (GTK_TEXT_VIEW(view), 8);

  /* place stack */ 
  gtk_stack_add_titled(stack, GTK_WIDGET(scrolledwindow1), "Note", _("Notes"));
  gtk_stack_add_titled(stack, GTK_WIDGET(scrolledwindowPDF), "Refe", _("Reference"));
  gtk_stack_add_titled(stack, GTK_WIDGET(scrolledwindowCrobar), "Sket", _("Sketch"));
  /* set-up default font for text view */
  set_up_view(mainWindow, &app_data);
  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));

  //get_theme_selection_color(view);

  misc_setup_text_buffer_tags(buffer);

  /* two statusbars arranged in a GtkGrid */
  gridStatusBar = gtk_grid_new();
  g_object_set (gridStatusBar, "margin-top", 4, NULL);
  g_object_set (gridStatusBar, "margin-bottom", 6, NULL);
  gtk_grid_attach(GTK_GRID(vGrid), GTK_WIDGET(gridStatusBar), 0,3,1,1);
  /* statusbar */
  UI_statusbar(mainWindow, gridStatusBar, &app_data );  
  /* mimic nice page jumper of Evince */
  UI_pdf_page_widget (mainWindow, gridStatusBar, &app_data);

  gtk_widget_show_all(mainWindow);

  /* quasi global vars  */
  misc_init_vars(&app_data );
  app_data.stack=stack;
  app_data.appWindow=mainWindow;
  app_data.PDFdrawable = crPDF;
  app_data.SketchDrawable= crCrobar;
  app_data.PDFScrollable = scrolledwindowPDF;
  app_data.SketchScrollable= scrolledwindowCrobar;
  app_data.buffer = buffer;
  app_data.view=view;
  app_data.pipeline = gst_element_factory_make ("playbin", "redac");
  /* spell checker !! after vars init for view value */
  misc_init_spell_checker(&app_data );

  cairo_surface_destroy (app_data.Sketchsurface);
  app_data.Sketchsurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, CROBAR_VIEW_MAX_WIDTH, CROBAR_VIEW_MAX_HEIGHT);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (mainWindow, mainWindow, "mainWindow");
  GLADE_HOOKUP_OBJECT (mainWindow, headBar, "headBar");
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

  /* we get the configuration file */

  gConfigFile = g_build_filename(g_get_user_config_dir (), 
                       "/redac/", KILOWRITER_CONFIG, NULL); /* Create hidden directory to store Kilowriter data */

  /* we check if the directory already exists */
  if(!g_file_test (gConfigFile, G_FILE_TEST_EXISTS)) {
     printf("* config.ini file absent or corrupted ! *\n");
     /* we create the directory */
     gint i=g_mkdir (g_strdup_printf("%s/redac/", g_get_user_config_dir ()), S_IRWXU);/* i's a macro from gstdio.h */
  }
  /* we parse datas from config file */
  createGKeyFile (mainWindow);
  /* we get the saved window geometry see : https://gist.github.com/zdxerr/709169  */

  keyString = g_object_get_data(G_OBJECT(mainWindow), "config");
  /* same for application */
  app_data.keystring = keyString;
  app_data.iAudioSmartRew=g_key_file_get_double(keyString, "application", "audio-file-rewind-step", NULL);
  app_data.iAudioSmartJump=g_key_file_get_double(keyString, "application", "audio-file-marks-step", NULL);

  /* set-up default value only with CSS */
  gchar *fntFamily=NULL;
  gint fntSize=12;
 // PangoContext* context = gtk_widget_get_pango_context  (GTK_WIDGET(app_data.view));
  PangoFontDescription *desc;// = pango_context_get_font_description(context);   
  desc = pango_font_description_from_string (g_key_file_get_string(keyString, "editor", "font", NULL));
  if (desc != NULL) {
          fntFamily= pango_font_description_get_family (desc);
          fntSize=pango_font_description_get_size(desc)/1000;
  }
  
  /* change gtktextview colors compliant with Gtk 3.16+ pLease note : re-change seleted state is mandatory, even if defned in interface*/
  color.red=g_key_file_get_double(keyString, "editor", "text.color.red", NULL);
  color.green=g_key_file_get_double(keyString, "editor", "text.color.green", NULL);
  color.blue=g_key_file_get_double(keyString, "editor", "text.color.blue", NULL);
  color.alpha=1;
  /* paper color */
  b_color.red=g_key_file_get_double(keyString, "editor", "paper.color.red", NULL);
  b_color.green=g_key_file_get_double(keyString, "editor", "paper.color.green", NULL);
  b_color.blue=g_key_file_get_double(keyString, "editor", "paper.color.blue", NULL);
  b_color.alpha=1;

  GtkCssProvider* css_provider = gtk_css_provider_new();
  gchar *css;
  css = g_strdup_printf("  #view  { font-family:%s; font-size:%dpx; color: #%.2x%.2x%.2x; background-color: #%.2x%.2x%.2x; }\n  #view:selected, #view:selected:focus { background-color: @selected_bg_color; color:@selected_fg_color; }\n",
                 fntFamily,
                 fntSize,
                 (gint)( color.red*255),(gint)( color.green*255), (gint)(color.blue*255),
                (gint)( b_color.red*255),(gint)( b_color.green*255), (gint)(b_color.blue*255));

  if(desc)
      pango_font_description_free(desc);

  gtk_css_provider_load_from_data(css_provider,css,-1,NULL);
  GdkScreen* screen = gdk_screen_get_default();
  gtk_style_context_add_provider_for_screen (screen,GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_free(css);


  /* we paint sketch background now */
  cairo_t *cr;
  cr = cairo_create (app_data.Sketchsurface);
  color.red=g_key_file_get_double(keyString, "sketch", "paper.color.red", NULL);
  color.green=g_key_file_get_double(keyString, "sketch", "paper.color.green", NULL);
  color.blue=g_key_file_get_double(keyString, "sketch", "paper.color.blue", NULL);
  color.alpha=1;
  cairo_set_source_rgb(cr, color.red, color.green, color.blue);
  cairo_rectangle(cr, 0, 0, CROBAR_VIEW_MAX_WIDTH, CROBAR_VIEW_MAX_HEIGHT);
  cairo_fill(cr);
  cairo_destroy (cr);
  gtk_widget_queue_draw ( app_data.SketchDrawable);

  /* reload last document */
  gchar *s1 = g_key_file_get_string(keyString, "application", "current-file", NULL);
  if(load_gtk_rich_text(s1, buffer, mainWindow)!=0) {
     misc_clear_text(buffer, "left");
     printf("* can't reload last work or it's the first start of this software ! *\n");
     /* the default filename is built inside gKeyfile if it isn"t already exists ! */
     /* then we must add a default file name for default path ; in other case, we'll get a segfault */
     /* we get the current date */
     time ( &rawtime );
     strftime(buffer_date, 80, "%c", localtime(&rawtime));/* don't change parameter %x */
     /* now we set-up a new default filename */
     path_to_file =  get_path_to_datas_file(buffer_date);
     gtk_label_set_markup (GTK_LABEL(lookup_widget(GTK_WIDGET(mainWindow), "labelMainTitle")),
                             g_strdup_printf(_("<small><b>%s</b></small>"), path_to_file));
     
     /* rearrange list of recent files */
     rearrange_recent_file_list(keyString);
     /* we change the default values for gkeyfile */
     store_current_file_in_keyfile(keyString, path_to_file, "[...]");
  }
  else
     store_current_file_in_keyfile(keyString, s1, misc_get_extract_from_document(&app_data ));  
  g_free(s1);

  /*  reload last PDF file ? */
  gtk_widget_hide( lookup_widget(GTK_WIDGET(app_data.appWindow),"image_pdf_modif"));
  s1 = g_key_file_get_string(keyString, "application", "current-PDF-file", NULL);
  if(g_key_file_get_boolean(keyString, "application", "autoreload-PDF",NULL )) {
     if(g_file_test (s1, G_FILE_TEST_EXISTS) ){
        /* reset default PDF zoom ratio */
        app_data.PDFratio=g_key_file_get_double(keyString, "reference-document", "zoom", NULL);
        app_data.curPDFpage=g_key_file_get_integer(keyString, "reference-document", "page", NULL);
        quick_load_PDF(s1, &app_data);
     }
  }
  g_free(s1);

  update_statusbar(buffer, &app_data);

 /* we preset the cursor */
  gtk_widget_grab_focus(GTK_WIDGET(view));

 /* callbacks */
 
  g_signal_connect(buffer, "changed",
        G_CALLBACK(update_statusbar), &app_data);

  g_signal_connect(buffer, "mark_set", 
        G_CALLBACK(mark_set_callback), &app_data);

  g_signal_connect(view, "cut-clipboard", 
        G_CALLBACK(cut_to_clipboard), &app_data);

  g_signal_connect(view, "copy-clipboard", 
        G_CALLBACK(copy_to_clipboard), &app_data);

  g_signal_connect(view, "paste-clipboard", 
        G_CALLBACK(paste_clipboard), &app_data);

  g_signal_connect(view, "backspace", 
        G_CALLBACK(backspace), &app_data);

  g_signal_connect(view, "delete-from-cursor", 
        G_CALLBACK(delete), &app_data);

  g_signal_connect(G_OBJECT(mainWindow), "delete_event",
        G_CALLBACK(on_quit_clicked), &app_data);
  /* keypress event in order to catch shortcuts WITHOUT widgets */
  g_signal_connect(G_OBJECT(mainWindow), "key-press-event", 
                   G_CALLBACK(key_event), &app_data);

  g_signal_connect (G_OBJECT(scrolledwindowPDF), "size-allocate",
                  G_CALLBACK (on_PDF_size_changed),
                  &app_data);

  g_signal_connect (G_OBJECT(scrolledwindowPDF), "scroll-event",
                  G_CALLBACK (on_PDF_scroll_event),
                  &app_data);
  
  g_signal_connect (G_OBJECT(stack), "notify::visible-child",
                  G_CALLBACK (on_stack_changed),
                  &app_data);
  g_signal_connect(G_OBJECT(crPDF), "draw",
					 G_CALLBACK(draw_callback), &app_data); 
 /* events to build selection rectangles on the PDF renderer */
  g_signal_connect(G_OBJECT(crPDF), "button-press-event",
					 G_CALLBACK(on_PDF_draw_button_press_callback), &app_data);
  g_signal_connect(G_OBJECT(crPDF), "button-release-event",
					 G_CALLBACK(on_PDF_draw_button_release_callback), &app_data);
  g_signal_connect(G_OBJECT(crPDF), "motion-notify-event",
					 G_CALLBACK(on_PDF_draw_motion_event_callback), &app_data);
  /* events for sketch area */
  g_signal_connect(G_OBJECT(crCrobar), "draw",
					 G_CALLBACK(sketch_draw_callback), &app_data); 
  g_signal_connect(G_OBJECT(crCrobar), "button-press-event",
					 G_CALLBACK(on_sketch_draw_button_press_callback), &app_data);
  g_signal_connect(G_OBJECT(crCrobar), "button-release-event",
					 G_CALLBACK(on_sketch_draw_button_release_callback), &app_data);
  g_signal_connect(G_OBJECT(crCrobar), "motion-notify-event",
					 G_CALLBACK(on_sketch_draw_motion_event_callback), &app_data);

  misc_set_gui_in_editor_mode(app_data.appWindow, CURRENT_STACK_EDITOR); 
  /* add timeout for 5 minutes, 300 secs - should be improved in a next release */
  if(g_key_file_get_boolean(keyString, "application", "interval-save",  NULL) ) {
      gtk_widget_show( lookup_widget(GTK_WIDGET(app_data.appWindow),"image_task_due"));
    }
    else
      gtk_widget_hide( lookup_widget(GTK_WIDGET(app_data.appWindow),"image_task_due"));
  g_timeout_add_seconds(300, timeout_quick_save, &app_data);/* yes, it's an intelligent call, keep as it */
  /* timer for audio display */
  g_timeout_add_seconds(1, timeout_audio_display_position, &app_data);/* yes, it's an intelligent call, keep as it */
  /* display auto-repeat indicator ? */
  if(g_key_file_get_boolean(keyString, "application", "audio-auto-rewind",  NULL) ) {
      gtk_widget_show( lookup_widget(GTK_WIDGET(app_data.appWindow),"image_audio_jump_to_start"));
    }
    else
      gtk_widget_hide( lookup_widget(GTK_WIDGET(app_data.appWindow),"image_audio_jump_to_start"));

  /* move to something like end of text */
  misc_jump_to_end_view(buffer, view);
  /* main loop */
  gtk_main();

  return 0;
}
