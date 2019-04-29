/***************************
  functions to manage files
****************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* translations */
#include <libintl.h>
#include <locale.h>

#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gstdio.h> /* g_fopen, etc */
#include <gst/gst.h>
#include "interface.h" /* glade requirement */
#include "support.h" /* glade requirement */
#include "mttfiles.h"

/*
 * Internal helper: equivalent to mkdir -p on linux.
 * Will recursively create missing folders to make the required foldername, and mode.
 * Always returns true!
 */
gboolean mkFullDir(gchar *folderName, gint mode)
{
  gchar *partialFolderName, *pPartialFolderName;
  gchar **folderParts;
  gint i = 0;
  
  /* Completely split folderName into parts */
  folderParts = g_strsplit_set(folderName, G_DIR_SEPARATOR_S, -1);
  partialFolderName = g_strdup(folderName);
  pPartialFolderName = partialFolderName;

  while (folderParts[i] != NULL) {
    pPartialFolderName = g_stpcpy(pPartialFolderName, folderParts[i]);
    pPartialFolderName = g_stpcpy(pPartialFolderName, G_DIR_SEPARATOR_S);
    
    if (!g_file_test (partialFolderName, G_FILE_TEST_IS_DIR)) {
      g_mkdir(partialFolderName, mode);
    }
    i++;
  }
  
  return TRUE;
}


/**************************************************************
 *    Keyfile interface commands
 **************************************************************/

/*******************************************
 from Intel's conman, so many thanks !
  here : https://kernel.googlesource.com/pub/scm/network/connman/connman/+/2ea605a3fe31f03384fa3ad0f01a901da3a7c095/src/storage.c
*******************************************/
static GKeyFile *storage_load(const char *pathname)
{
	GKeyFile *keyfile = NULL;
	GError *error = NULL;
	keyfile = g_key_file_new();
	if (!g_key_file_load_from_file(keyfile, pathname, 0, &error)) {
		printf("Unable to load %s: %s", pathname, error->message);
		g_clear_error(&error);
		g_key_file_free(keyfile);
		keyfile = NULL;
	}
	return keyfile;
}
/*

 store all program's settings

*/
gint storage_save( gchar *pathname, APP_data *data_app)
{
  gchar *data = NULL, buffer[81];
  gsize length = 0;
  GError *error = NULL;
  gint ret = 0;
  GKeyFile *keyString;
  gint width_height[2], pos_x, pos_y;
  time_t rawtime; 
  GdkRGBA color;   
  GtkWidget *pBtnColor; 

  keyString = g_object_get_data(G_OBJECT(mainWindow), "config"); 
  /* we get the current RGBA color */
  pBtnColor=lookup_widget(GTK_WIDGET(mainWindow), "color_button");
  gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER(pBtnColor), &color); 

  gchar* tmpStr = g_strdup_printf(_(" %s settings auto-generated - Do not edit!"), "Redac 0.1");
  g_key_file_set_comment (keyString, NULL, NULL, tmpStr, NULL);
  g_key_file_set_string(keyString, "version-info", "version", CURRENT_VERSION);
  /* we store window geometry */
  gtk_window_get_position (GTK_WINDOW(mainWindow), &pos_x, &pos_y); 
  gtk_window_get_size (GTK_WINDOW(mainWindow), &width_height[0], &width_height[1]);
  g_key_file_set_integer (keyString, "application", "geometry.x", pos_x);
  g_key_file_set_integer (keyString, "application", "geometry.y", pos_y);
  g_key_file_set_integer (keyString, "application", "geometry.width", width_height[0]);
  g_key_file_set_integer (keyString, "application", "geometry.height",width_height[1]);

  g_key_file_set_double(keyString, "application", "fg.color.red", color.red);
  g_key_file_set_double(keyString, "application", "fg.color.green", color.green);
  g_key_file_set_double(keyString, "application", "fg.color.blue", color.blue);



  /* we get the current date */
  time ( &rawtime );
  strftime(buffer, 80, "%c", localtime(&rawtime));

  if(!g_key_file_get_string(keyString, "application", "current-file", NULL)) {
       printf("* invalid file name *\n");
       g_key_file_set_string(keyString, "application", "current-file", g_strdup_printf("Redac-%s.kw", buffer));
  }
  if(!g_key_file_get_string(keyString, "application", "current-PDF-file", NULL)) {
       printf("* invalid file name *\n");
       g_key_file_set_string(keyString, "application", "current-PDF-file", "");
  }

  g_key_file_set_integer(keyString, "reference-document", "page", data_app->curPDFpage);

  data = g_key_file_to_data(keyString, &length, &error);

  if (!g_file_set_contents(gConfigFile, data, length, &error)) {
		printf("Failed to store information: %s", error->message);
		g_error_free(error);
  }
  g_free(data);
  g_free(tmpStr);

  return ret;
}

/*******************************
 Creates a new config file
 ******************************/
void createNewKeyFile(GKeyFile *keyString)
{
    gint width_height[2], i;
    gchar buffer[81];
    time_t rawtime; 
    gchar *tmpStr2;
    GError *error;

    gchar* tmpStr = g_strdup_printf(_(" %s settings auto-generated - Do not edit!"), "Redac 0.1");
    g_key_file_set_comment (keyString, NULL, NULL, tmpStr, NULL);
    g_key_file_set_string(keyString, "version-info", "version", CURRENT_VERSION);
    g_key_file_set_integer (keyString, "application", "geometry.x", 0);
    g_key_file_set_integer (keyString, "application", "geometry.y", 0);
    /* we get the current  window geometry */
    gtk_window_get_size (GTK_WINDOW(mainWindow),
                       &width_height[0], &width_height[1]);
    g_key_file_set_integer (keyString, "application", "geometry.width",  width_height[0]);
    g_key_file_set_integer (keyString, "application", "geometry.height", width_height[1]);
    /* we get the current date */
    time ( &rawtime );
    strftime(buffer, 80, "%c", localtime(&rawtime));/* don't change parameter %x */
  
    g_key_file_set_string(keyString, "application", "current-file", get_path_to_datas_file(buffer));

    /* setup and store the basename */
    tmpStr2= g_key_file_get_string(keyString, "application", "current-file", NULL);
    g_key_file_set_string(keyString, "application", "current-file-basename", 
                      g_filename_display_basename (tmpStr2));

    g_free(tmpStr2);
    /* general settings */
    g_key_file_set_boolean(keyString, "application", "interval-save", FALSE);
    g_key_file_set_boolean(keyString, "application", "autoreload-PDF", FALSE);
    g_key_file_set_boolean(keyString, "application", "audio-file-loaded", FALSE);
    g_key_file_set_double(keyString, "application", "audio-file-rewind-step", 2.0);
    g_key_file_set_double(keyString, "application", "audio-file-marks-step", 15.0);
    g_key_file_set_boolean(keyString, "application", "audio-auto-rewind", TRUE);
    g_key_file_set_integer (keyString, "application", "interval-save-frequency", 5);
    g_key_file_set_boolean(keyString, "application", "prompt-before-quit", TRUE);
    g_key_file_set_boolean(keyString, "application", "prompt-before-overwrite", TRUE);
    /* PDF file */
    g_key_file_set_string(keyString, "application", "current-PDF-file", "");
    g_key_file_set_string(keyString, "application", "current-PDF-file-basename", "");
    /* default foreground color */
    g_key_file_set_double(keyString, "application", "fg.color.red", 0.15);
    g_key_file_set_double(keyString, "application", "fg.color.green", 0.94);
    g_key_file_set_double(keyString, "application", "fg.color.blue", 0.09);

    /* audio */
    /* recent files */
    for(i=0;i<MAX_RECENT_FILES;i++) {
       g_key_file_set_string(keyString, "history", g_strdup_printf("recent-file-%d", i), 
                      "");
       g_key_file_set_string(keyString, "history", g_strdup_printf("recent-content-%d", i), 
                      "");
    }
    /* editor */
    /* text color */
    g_key_file_set_double(keyString, "editor", "text.color.red", 0);
    g_key_file_set_double(keyString, "editor", "text.color.green", 0);
    g_key_file_set_double(keyString, "editor", "text.color.blue", 0);
    /* background color : cream */
    g_key_file_set_double(keyString, "editor", "paper.color.red", 0.984);
    g_key_file_set_double(keyString, "editor", "paper.color.green", 0.9764);
    g_key_file_set_double(keyString, "editor", "paper.color.blue", 0.9451);
    /* default text font */
    g_key_file_set_string(keyString, "editor", "font", "Sans 12");
    /* PDF */
    g_key_file_set_double(keyString, "reference-document", "zoom", 1);
    g_key_file_set_integer(keyString, "reference-document", "page", 0);
    /* background color : white */
    g_key_file_set_double(keyString, "reference-document", "paper.color.red", 1);
    g_key_file_set_double(keyString, "reference-document", "paper.color.green", 1);
    g_key_file_set_double(keyString, "reference-document", "paper.color.blue", 1);
    /* sketch */
    /* background color : white */
    g_key_file_set_double(keyString, "sketch", "paper.color.red", 1);
    g_key_file_set_double(keyString, "sketch", "paper.color.green", 1);
    g_key_file_set_double(keyString, "sketch", "paper.color.blue", 1);
    /* default text font */
    g_key_file_set_string(keyString, "sketch", "font", "Sans 14");
    g_key_file_set_double(keyString, "sketch", "pen-width", 2);

    gchar *context = g_key_file_to_data (keyString, NULL, &error);
    g_file_set_contents (gConfigFile, context, -1, NULL);
    g_free(tmpStr);
    g_free(context);
    return;
}

/* see : http://distro.ibiblio.org/vectorlinux/stretchedthin/yad/0.12.2/src/yad-0.12.2/src/util.c
 * Attempts to load the searchmonkey config.ini file.
 * If invalid, or non-existant, creates new config file.
 */
void createGKeyFile(GtkWidget *win)
{
  GKeyFile *keyString;
  gint i, width, height, pos_x, pos_y;
  gsize length;
  gchar *tmpStr, buffer[81];
  time_t rawtime; 
  GdkRGBA color;   
  GtkWidget *pBtnColor; 

  keyString = g_key_file_new ();

  if(!g_key_file_load_from_file (keyString, gConfigFile,
                                  G_KEY_FILE_KEEP_COMMENTS,
                                  NULL)) {printf("pb =%s\n",gConfigFile );
    createNewKeyFile(keyString);
  } 
  /* we read datas for config.ini file */

  if (g_key_file_has_key(keyString, "application", "geometry.width", NULL)) {
    width = g_key_file_get_integer (keyString, "application", "geometry.width",  NULL);  
  }
  else width =900;
  if (g_key_file_has_key(keyString, "application", "geometry.height", NULL)) {
    height = g_key_file_get_integer (keyString, "application", "geometry.height",  NULL);  
  }
  else height =512;
  pos_x = 64;
  pos_y = 64;
  if (g_key_file_has_key(keyString, "application", "geometry.x", NULL)) {
    pos_x = g_key_file_get_integer (keyString, "application", "geometry.x",  NULL);  
  }
  if (g_key_file_has_key(keyString, "application", "geometry.y", NULL)) {
    pos_y = g_key_file_get_integer (keyString, "application", "geometry.y",  NULL);  
  }

  gtk_window_move(GTK_WINDOW(win), pos_x, pos_y);
  gtk_window_resize (GTK_WINDOW(win), width, height);
  /* some stuff about default file */
  if(!g_key_file_has_key(keyString, "application", "current-file", NULL)) { 
    /* we get the current date */
    time ( &rawtime );
    strftime(buffer, 80, "%c", localtime(&rawtime));
    g_key_file_set_string(keyString, "application", "current-file", 
                      get_path_to_datas_file(buffer));
  }
  /* general settings */
  if(!g_key_file_has_key(keyString, "application", "interval-save", NULL)) {
    g_key_file_set_boolean(keyString, "application", "interval-save", FALSE);
  }
  if(!g_key_file_has_key(keyString, "application", "autoreload-PDF", NULL)) {
    g_key_file_set_boolean(keyString, "application", "autoreload-PDF", FALSE);
  }
  if(!g_key_file_has_key(keyString, "application", "audio-auto-rewind", NULL)) {
    g_key_file_set_boolean(keyString, "application", "audio-auto-rewind", FALSE);
  }

  if(!g_key_file_has_key(keyString, "application", "interval-save-frequency", NULL)) {
    g_key_file_set_integer (keyString, "application", "interval-save-frequency", 5);
  }
  if(!g_key_file_has_key(keyString, "application", "prompt-before-quit", NULL)) {
    g_key_file_set_boolean(keyString, "application", "prompt-before-quit", TRUE);
  }
  if(!g_key_file_has_key(keyString, "application", "prompt-before-overwrite", NULL)) {
    g_key_file_set_boolean(keyString, "application", "prompt-before-overwrite", TRUE);
  }
  if(!g_key_file_has_key(keyString, "application", "audio-file-loaded", NULL)) {
    g_key_file_set_boolean(keyString, "application", "audio-file-loaded", FALSE);
  }
  if(!g_key_file_has_key(keyString, "application", "audio-file-rewind-step", NULL)) {
    g_key_file_set_double(keyString, "application", "audio-file-rewind-step", 2.0);
  }
  if(!g_key_file_has_key(keyString, "application", "audio-file-marks-step", NULL)) {
    g_key_file_set_double(keyString, "application", "audio-file-marks-step", 15.0);
  }

  /* get and store the basename */
  tmpStr= g_key_file_get_string(keyString, "application", "current-file", NULL);
  g_key_file_set_string(keyString, "application", "current-file-basename", 
                      g_filename_display_basename (tmpStr));  
  /* default forreground color */
   if(!g_key_file_has_key(keyString, "application", "fg.color.red", NULL)) { 
      g_key_file_set_double(keyString, "application", "fg.color.red", 0.15);
   }
   if(!g_key_file_has_key(keyString, "application", "fg.color.green", NULL)) { 
      g_key_file_set_double(keyString, "application", "fg.color.green", 0.94);
   }
   if(!g_key_file_has_key(keyString, "application", "fg.color.blue", NULL)) { 
      g_key_file_set_double(keyString, "application", "fg.color.blue", 0.09);
   }
  /* set default value for color chooser button */
  pBtnColor=lookup_widget(GTK_WIDGET(win), "color_button");
  color.red=g_key_file_get_double(keyString, "application", "fg.color.red", NULL);
  color.green=g_key_file_get_double(keyString, "application", "fg.color.green", NULL);
  color.blue=g_key_file_get_double(keyString, "application", "fg.color.blue", NULL);
  color.alpha=1;
  gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER(pBtnColor), &color); 
  /* check for recent files and recent files summary */
  for(i=0;i<MAX_RECENT_FILES;i++) {
     if(!g_key_file_has_key(keyString, "history", g_strdup_printf("recent-file-%d",i ), NULL)) {
         g_key_file_set_string(keyString, "history", g_strdup_printf("recent-file-%d", i),"");
     }
     if(!g_key_file_has_key(keyString, "history", g_strdup_printf("recent-content-%d",i ), NULL)) {
         g_key_file_set_string(keyString, "history", g_strdup_printf("recent-content-%d", i),"");
     }
  }


  /* if the current file-0 is empty ther is something wrong, so we store the current file name as first entry */
  if(!g_key_file_has_key(keyString, "history", "recent-file-0", NULL)) {
         g_key_file_set_string(keyString, "history", "recent-file-0", tmpStr );
  }
  g_free(tmpStr);
/* TODO : same thing for file content ? */

  /* PDF stuff - thus we gain a g_free(tmpStr) ;-) */
  if(!g_key_file_has_key(keyString, "application", "current-PDF-file", NULL)) { 
     g_key_file_set_string(keyString, "application","current-PDF-file", 
                      "");
  }
  tmpStr= g_key_file_get_string(keyString, "application", "current-PDF-file", NULL);
  g_key_file_set_string(keyString, "application", "current-PDF-file-basename", 
                      g_filename_display_basename (tmpStr));
  g_free(tmpStr);
  /* editor */
  /* text color */
  if(!g_key_file_has_key(keyString, "editor", "text.color.red", NULL)) { 
    g_key_file_set_double(keyString, "editor", "text.color.red", 0);
  }
  if(!g_key_file_has_key(keyString, "editor", "text.color.green", NULL)) { 
    g_key_file_set_double(keyString, "editor", "text.color.green", 0);
  }
  if(!g_key_file_has_key(keyString, "editor", "text.color.blue", NULL)) { 
    g_key_file_set_double(keyString, "editor", "text.color.blue", 0);
  }
  /* background color : cream */
  if(!g_key_file_has_key(keyString, "editor", "paper.color.red", NULL)) { 
     g_key_file_set_double(keyString, "editor", "paper.color.red", 0.984);
  }
  if(!g_key_file_has_key(keyString, "editor", "paper.color.green", NULL)) { 
      g_key_file_set_double(keyString, "editor", "paper.color.green", 0.9764);
  }
  if(!g_key_file_has_key(keyString, "editor", "paper.color.blue", NULL)) { 
    g_key_file_set_double(keyString, "editor", "paper.color.blue", 0.9451);
  }
  /* default text font */
  if(!g_key_file_has_key(keyString, "editor", "font", NULL)) { 
     g_key_file_set_string(keyString, "editor", "font", "Sans 12");
  }

  /* PDF */
  if(!g_key_file_has_key(keyString, "reference-document", "zoom", NULL)) { 
     g_key_file_set_double(keyString, "reference-document", "zoom", 1);
  }
  if(!g_key_file_has_key(keyString, "reference-document", "page", NULL)) { 
     g_key_file_set_integer(keyString, "reference-document", "page", 0);
  }
  /* background color : white */
  if(!g_key_file_has_key(keyString, "reference-document", "paper.color.red", NULL)) { 
     g_key_file_set_double(keyString, "reference-document", "paper.color.red", 1);
  }
  if(!g_key_file_has_key(keyString, "reference-document", "paper.color.green", NULL)) { 
      g_key_file_set_double(keyString, "reference-document", "paper.color.green", 1);
  }
  if(!g_key_file_has_key(keyString, "reference-document", "paper.color.blue", NULL)) { 
    g_key_file_set_double(keyString, "reference-document", "paper.color.blue", 1);
  }

  /* sketch */
  /* background color : white */
  if(!g_key_file_has_key(keyString, "sketch", "paper.color.red", NULL)) { 
     g_key_file_set_double(keyString, "sketch", "paper.color.red", 1);
  }
  if(!g_key_file_has_key(keyString, "sketch", "paper.color.green", NULL)) { 
      g_key_file_set_double(keyString, "sketch", "paper.color.green", 1);
  }
  if(!g_key_file_has_key(keyString, "sketch", "paper.color.blue", NULL)) { 
    g_key_file_set_double(keyString, "sketch", "paper.color.blue", 1);
  }
  /* default text font */
  if(!g_key_file_has_key(keyString, "sketch", "font", NULL)) { 
        g_key_file_set_string(keyString, "sketch", "font", "Sans 14");
  }

  if(!g_key_file_has_key(keyString, "sketch", "pen-width", NULL)) { 
     g_key_file_set_double(keyString, "sketch", "pen-width", 2);
  }

  /* store in main window */
  g_object_set_data(G_OBJECT(win), "config", keyString);
//  g_key_file_free (keyString);
}

/*
 * Saves config file (config.ini) to disk, and frees associated memory.
 * Automatically called when save data is destroyed (i.e. user closed searchmonkey).
 */
void destroyGKeyFile(GtkWidget *win)
{
  GKeyFile *keyString = g_object_get_data(G_OBJECT(win),"config");
  printf("* GKeyfile destroyed successfully *\n");

  //  storeGKeyFile(keyString);

  g_key_file_free(keyString);
  if (gConfigFile != NULL) {
    g_free(gConfigFile);
  }
}

void storeGKeyFile(GKeyFile *keyString)
{
  gsize length;
  gchar **outText;
  GError *error = NULL;
  gchar *folderName;
  GtkWidget *okCancelDialog;
  
     
  /* Write the configuration file to disk */
  folderName = g_path_get_dirname(gConfigFile);
  outText = g_key_file_to_data (keyString, &length, &error);

  if (!g_file_get_contents (gConfigFile, outText, &length, &error)) {
    /* Unable to immediately write to file, so attempt to recreate folders */
    mkFullDir(folderName, S_IRWXU);

    if (!g_file_get_contents (gConfigFile, outText, &length, &error)) { 
      g_print(_("Error saving %s: %s\n"), gConfigFile, error->message);
      g_error_free(error);
      error = NULL;
    }
  }
  
  g_free(outText);
  g_free(folderName);
}

/***********************************
  set defaults values for 
  current filename and store 
  in config file in  memory  
**********************************/
void store_current_file_in_keyfile(GKeyFile *keyString, gchar *filename, gchar *summary)
{
   g_key_file_set_string(keyString, "application", "current-file", filename);
   /* setup and store the basename */
   g_key_file_set_string(keyString, "application", "current-file-basename", 
                      g_filename_display_basename (filename));
   g_key_file_set_string(keyString, "history", "recent-file-0", filename );
   g_key_file_set_string(keyString, "history", "recent-content-0", summary );/* start of file's content */
}

/***********************************
 rearrange files' order in recent
  file list ; do the same for
  extracts
***********************************/
void rearrange_recent_file_list(GKeyFile *keyString)
{
  gint i;
  gchar *recent_prev, *content_prev;

  for(i=MAX_RECENT_FILES-1; i>0; i--) {     
     if(g_key_file_has_key(keyString, "history", g_strdup_printf("recent-file-%d",i-1 ), NULL)) {
                 recent_prev = g_strdup_printf("%s", g_key_file_get_string(keyString, "history", 
                                          g_strdup_printf("recent-file-%d",i-1), NULL));
                 g_key_file_set_string(keyString, "history", g_strdup_printf("recent-file-%d",i), recent_prev);
                 g_free(recent_prev);
                 /* summaries */
                 content_prev = g_strdup_printf("%s", g_key_file_get_string(keyString, "history", 
                                          g_strdup_printf("recent-content-%d",i-1), NULL));
                 g_key_file_set_string(keyString, "history", g_strdup_printf("recent-content-%d",i), content_prev);
                 g_free(content_prev);

     }
  }
}
/* *********************************
   function to get a valid file name
   in the $TEMPDIR system repertory

***********************************/
gchar *GetTempFileName(gchar *fileSchema)
{
 gchar *tmpFile = NULL;

 tmpFile = tempnam(NULL, fileSchema );
 return tmpFile;
}

/****************************
  load a file in internal rich
  text format
*******************************/
gint load_gtk_rich_text(gchar *filename, GtkTextBuffer *buffer, GtkWidget *window1)
{
  GtkTextIter start, end; 
  gsize length;
  guint8 *txtbuffer;
  gint i;
  FILE *inputFile;
  GError **error;
  glong fileSize;
  gchar rich_text_sign[]={0x47, 0x54, 0x4B, 0x54, 0x45, 0x58, 0x54,0x00};
  GtkWidget *alertDlg;
  GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

  GdkAtom format = gtk_text_buffer_register_deserialize_tagset(buffer, "application/x-gtk-text-buffer-rich-text");

  /* we open the file */
  inputFile = fopen(filename,"rb");
  if(inputFile==NULL) {
          printf("* ERROR : can't open Redac file:%s *\n", filename);
          return -1;
  }
  /* we compute the size before dynamically allocate buffer */
  glong prev = ftell(inputFile);   
  fseek(inputFile, 0L, SEEK_END);
  glong sz = ftell(inputFile);
  fseek(inputFile, prev, SEEK_SET);
  /* we allocate the buffer */
  if(sz<=0)
    return -1;
  txtbuffer = g_malloc0(sz*sizeof(guint8)+sizeof(guint8));
  fileSize = fread(txtbuffer, sizeof(guint8), sz, inputFile);
  fclose(inputFile);
  /* now we check if it's a TRUE Gtk Rich text file */
  if (strncmp(txtbuffer,&rich_text_sign,7)!=0) {/* sign GTKTEXT string */
      g_free(txtbuffer);
      alertDlg =  gtk_message_dialog_new (GTK_WINDOW(window1),
                                      flags,
                                      GTK_MESSAGE_ERROR,
                                      GTK_BUTTONS_OK,
                                      _("The file :%s isn't a correct file\nor it's currupted. Operation cancelled !"),
                                      filename);
     gint retrun= gtk_dialog_run(GTK_DIALOG(alertDlg));
     gtk_widget_destroy (GTK_WIDGET(alertDlg));
     return -1;
  }
  misc_clear_text(buffer, "left");
  gtk_text_buffer_get_start_iter(buffer, &start);
  //gtk_text_buffer_get_iter_at_offset(buffer,&start,0);
  gtk_text_buffer_get_end_iter (buffer, &end);
  gtk_text_buffer_remove_all_tags (buffer, &start, &end);
  //gtk_text_buffer_deserialize_set_can_create_tags(buffer,format,TRUE); //SURTOU pas car c rée tags incrémentaux !!!
  gboolean deserialized = gtk_text_buffer_deserialize(buffer, buffer, format, &start, txtbuffer, fileSize, NULL);/* NULL mandatory ? ! */
  g_free(txtbuffer);
  gtk_label_set_markup (GTK_LABEL(lookup_widget(GTK_WIDGET(window1), "labelMainTitle")),
                             g_strdup_printf(_("<small><b><span foreground=\"green\">Loaded</span>-%s</b></small>"), filename));
  return 0;
}

/****************************
  save a file in internal rich
  text format TODO modifiy for summarries 
*******************************/
gint save_gtk_rich_text(gchar *filename, GtkTextBuffer *buffer)
{
  GtkTextIter start, end; 
  gsize length;
  gint i, ret=0;
  FILE *outputFile;

  gtk_text_buffer_get_start_iter(buffer, &start);
  gtk_text_buffer_get_end_iter(buffer, &end);
  /* we remove all GtkSpell tags ! */
  gtk_text_buffer_remove_tag_by_name (buffer, "gtkspell-misspelled", &start, &end);

  GdkAtom format = gtk_text_buffer_register_serialize_tagset(buffer, "application/x-gtk-text-buffer-rich-text");
  guint8 *serialized = gtk_text_buffer_serialize(buffer, buffer, format, &start, &end, &length);

  /* we save as internal Gtk riche text format not the standard RTF format */
  outputFile = fopen(filename, "wb");
  fwrite(serialized, sizeof(guint8), length, outputFile);
  fclose(outputFile);
  return ret;
}

/**********************************
  callback : button open clicked 
Please Note : we only reload NATIVE
 rich text (as Gtk meaning) datas,
not standard Word Processor file

************************************/
void
on_open_clicked (GtkButton *button, APP_data *data_app)
{
  GKeyFile *keyString;
  GtkTextBuffer *buffer;
  gchar *path_to_file, *filename;
  gchar buffer_date[81];
  gint ret;
  GtkFileFilter *filter = gtk_file_filter_new ();
  gtk_file_filter_add_pattern (filter, "*.kw");
  gtk_file_filter_set_name (filter, _("Redac files"));

  buffer = data_app->buffer;
  GtkWidget *window1 = data_app->appWindow;
  keyString = g_object_get_data(G_OBJECT(window1), "config");

  GtkWidget *dialog = create_loadFileDialog(data_app);
  /* Set defaults, or get saved values*/
  /* we should replace home dir by the current path if it exists ! */
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), g_get_home_dir());
  /* we must extract a short filename */
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
  /* run dialog */
  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
    gtk_widget_destroy (GTK_WIDGET(dialog)); 
    /* first, we save once more time the current file */
    path_to_file = g_key_file_get_string(keyString, "application", "current-file", NULL);
    ret = save_gtk_rich_text(path_to_file, buffer);
    g_free(path_to_file);
    if(load_gtk_rich_text(filename, buffer, window1)==0) {   
       /* rearrange list of recent files */
       rearrange_recent_file_list(keyString);
       /* we change the default values for gkeyfile + summary */
       store_current_file_in_keyfile(keyString, filename, misc_get_extract_from_document(data_app ));
       /* now we set-up a new default filename */
       gtk_label_set_markup (GTK_LABEL(lookup_widget(GTK_WIDGET(window1), "labelMainTitle")),
                             g_strdup_printf(_("<small><b>%s</b></small>"), filename)); 
       g_free(filename);
    }
  }
  else
     gtk_widget_destroy (GTK_WIDGET(dialog)); 
}

/*********************************
  callback : button quit clicked 

***********************************/
gboolean
on_quit_clicked (GtkWidget *window1, GdkEvent *event, APP_data *data_app)
{
  GtkTextBuffer *buffer;
  GtkWidget *dialog;
  GKeyFile *keyString;
  gchar *path_to_file;
  gint ret;
  gboolean flag=TRUE;

  keyString = g_object_get_data(G_OBJECT(window1), "config");
  GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

  if(g_key_file_get_boolean(keyString, "application", "prompt-before-quit",NULL )) {
      dialog = gtk_message_dialog_new (GTK_WINDOW(window1), flags,
                    GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL,
                _("Do you really want to quit this program ?\nTake it easy, your work\nwill be automatically saved."));
      if(gtk_dialog_run(GTK_DIALOG(dialog))!=GTK_RESPONSE_OK)
         flag=FALSE;
      gtk_widget_destroy(GTK_WIDGET(dialog));
  }
  if(flag) {
     buffer = data_app->buffer;
     /* default dump in current folder ! */
     path_to_file = g_key_file_get_string(keyString, "application", "current-file", NULL);
     /* native Gtk dump */
     ret = save_gtk_rich_text(path_to_file, buffer);
     /* RTF compatible version */
     ret = save_RTF_rich_text(path_to_file, buffer);
     /* we change the default values for gkeyfile */
     store_current_file_in_keyfile(keyString, path_to_file, misc_get_extract_from_document(data_app));
     g_free(path_to_file);
     storage_save( gConfigFile, data_app);
     destroyGKeyFile(window1);
     undo_free_all(data_app);
     printf("* Freed successfully All undo datas *\n");
     //g_object_unref(data_app->spell);
     gst_element_set_state (data_app->pipeline, GST_STATE_NULL);
     gst_object_unref (data_app->pipeline);
     gtk_main_quit();
  }
 return TRUE;
}

/***********************************

  rapid save files

***********************************/
void quick_save (APP_data *data)
{
  GtkTextBuffer *buffer;
  GKeyFile *keyString;
  gchar *filename, *tmpFileName;
  gint ret;
  GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

  buffer = data->buffer;
  GtkWidget *window1 = data->appWindow;
  keyString = g_object_get_data(G_OBJECT(window1), "config");
  /* first, we get as default the current filename  */
  filename = g_key_file_get_string(keyString, "application", "current-file", NULL);
  tmpFileName = g_strdup_printf("%s.rtf", filename);

  ret = save_gtk_rich_text(filename, buffer);
  /* we save also in standard Word processor format !!! */
  ret = save_RTF_rich_text(tmpFileName, buffer);
  if(ret!=0) {
      GtkWidget *alertDlg;
      alertDlg =  gtk_message_dialog_new (GTK_WINDOW(window1),
                                      flags,
                                      GTK_MESSAGE_ERROR,
                                      GTK_BUTTONS_OK,
                                      _("Problem during quick save of :\n%s"),
                                      filename);
      ret=gtk_dialog_run(GTK_DIALOG(alertDlg));
      gtk_widget_destroy (GTK_WIDGET(alertDlg));
  }
  else {
    gtk_label_set_markup (GTK_LABEL(lookup_widget(GTK_WIDGET(window1), "labelMainTitle")),
                             g_strdup_printf(_("<small><b>%s-<span foreground=\"green\">saved</span></b></small>"), filename));
    //gtk_header_bar_set_subtitle (lookup_widget(GTK_WIDGET(window1), "headBar"),
      //                       g_strdup_printf(_("%s-saved"), filename));
    //gtk_window_set_title (GTK_WINDOW(window1),g_strdup_printf(_("Redac:%s-saved"),filename) );
    /* we change the default values for gkeyfile */
    store_current_file_in_keyfile(keyString, filename, misc_get_extract_from_document(data));
  }
  g_free(filename);
  g_free(tmpFileName);
  /* we free previous search datas */
  search_free_PDF_search_datas(data);
}

/*********************************
 save in a standard
  text format ;
  we save TWO files :
  - one as native Gtk Rich text
  - other as standard RTF (Ms) 
********************************/
void save_standard_file(GtkMenuItem *menuitem, APP_data  *data)
{
  GError *error = NULL;
  GKeyFile *keyString;
  gchar *filename, *newFilename, *tmpFileName;
  GtkTextBuffer *buffer;
  GtkWidget *alertDlg;
  GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
  gint ret;

  buffer = data->buffer;
  GtkWidget *window1 = data->appWindow;
  keyString = g_object_get_data(G_OBJECT(window1), "config");
  /* first, we get as default the current filename  */
  filename = g_key_file_get_string(keyString, "application", "current-file-basename", NULL);

  GtkWidget *dialog = create_saveFileDialog(data);
  /* Set defaults and get path for current filename */
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), 
                         g_path_get_dirname(g_key_file_get_string(keyString, "application", "current-file", NULL) ));
  /* we must extract a short filename */
  gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), filename);
  /* run dialog */
  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
         newFilename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
         /* we check if filename has .kw extension */
         if( !g_str_has_suffix (newFilename, ".kw" )) {
              /* we add suffix to the filename */
              tmpFileName = g_strdup_printf("%s.kw", newFilename);
              g_free(newFilename);
              newFilename = tmpFileName;
         }
         /* we must check if a file with same name exists */
         if(g_file_test (newFilename, G_FILE_TEST_EXISTS) 
              && g_key_file_get_boolean(keyString, "application", "prompt-before-overwrite",NULL )) {
           /* dialog to avoid overwriting */
           alertDlg =  gtk_message_dialog_new (GTK_WINDOW(dialog),
                                      flags,
                                      GTK_MESSAGE_ERROR,
                                      GTK_BUTTONS_OK_CANCEL,
                                      _("The file :%s exists !\nOverwrite existing file ?"),
                                      newFilename);
           if(gtk_dialog_run(GTK_DIALOG(alertDlg))==GTK_RESPONSE_CANCEL) {
                gtk_widget_destroy (GTK_WIDGET(alertDlg));
                gtk_widget_destroy (GTK_WIDGET(dialog));
                g_free(newFilename);
                return;
           }
         }
         ret = save_gtk_rich_text(newFilename, buffer);
         /* we save also in standard Word processor format !!! */
         ret = save_RTF_rich_text(newFilename, buffer);
         /* we setup the window's title */
         gtk_label_set_markup (GTK_LABEL(lookup_widget(GTK_WIDGET(window1), "labelMainTitle")),
                             g_strdup_printf(_("<small><b>%s-<span foreground=\"green\">saved</span></b></small>"), newFilename));
         //gtk_header_bar_set_subtitle (lookup_widget(GTK_WIDGET(window1), "headBar"),
           //                  g_strdup_printf(_("%s"), newFilename));
         // gtk_window_set_title (GTK_WINDOW(window1),g_strdup_printf("Redac:%s",newFilename) );
         /* rearrange list of recent files */
         rearrange_recent_file_list(keyString);
         /* we change the default values for gkeyfile */
         store_current_file_in_keyfile(keyString, newFilename, misc_get_extract_from_document(data));
         g_free(newFilename);
  }
  gtk_widget_destroy (GTK_WIDGET(dialog));
  g_free(filename);
}
/**************************************
  quick load a PDF at program's restart
***************************************/
void quick_load_PDF(gchar *filename, APP_data *data)
{
  GKeyFile *keyString;
  GError* err = NULL;
  gchar *uri_path;
  cairo_t *cr;
  gint w, h; 
  gdouble width, height, ratio;
  GtkWidget *canvas;
  PopplerPage *page;
  PopplerDocument *doc =NULL;

  GtkWidget *window1 = data->appWindow;
  keyString = g_object_get_data(G_OBJECT(window1),"config");
  canvas = lookup_widget(GTK_WIDGET(window1), "crPDF");

  /* we free previous search datas */
  search_free_PDF_search_datas(data);
  /* we convert finename's path to URI path */
  uri_path = g_filename_to_uri(filename, NULL,NULL);
  /* we load PDF Poppler's doc */
  doc = poppler_document_new_from_file(uri_path, NULL, &err);
  g_free(uri_path);
  if (!doc) {
        printf("%s\n", err->message);
        g_error_free(err);
        return;
  }
  /* we store PDF filename and path in config file */
  g_key_file_set_string(keyString, "application", "current-PDF-file", filename);
  g_key_file_set_string(keyString, "application", "current-PDF-file-basename", 
                      g_filename_display_basename (filename));
  if(data->doc) 
    g_object_unref(data->doc);
  undo_free_all_PDF_ops(data);
  data->doc=doc;
  //data->curPDFpage=0;
  data->button_pressed=FALSE;
  data->totalPDFpages=poppler_document_get_n_pages(data->doc);
  if(data->currentStack == CURRENT_STACK_PDF ) {
      update_statusbarPDF(data);
  }
  page = poppler_document_get_page(data->doc, data->curPDFpage);
  poppler_page_get_size (page, &width, &height);
  data->PDFWidth=width;
  data->PDFHeight=height;
  ratio = data->PDFratio;
//  data->PDFratio=ratio;

  w = (gint) (width*ratio);
  h = (gint) (height*ratio);

  cairo_surface_destroy (data->surface);
  /* some stuff to zoom */
  data->surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, w, h);
  cr = cairo_create (data->surface);
  /* fill in white the page */
  cairo_set_source_rgb(cr, 1.0, 1, 1);
  cairo_rectangle(cr, 0, 0, w, h);
  cairo_fill(cr);
  cairo_scale(cr, ratio,ratio);
  poppler_page_render (page, cr);
  g_object_unref(page);
  cairo_destroy (cr);
  gtk_widget_set_size_request (canvas, w, h);
  gtk_widget_queue_draw (canvas);

  data->fPdfLoaded=TRUE;
  update_PDF_state(data, PDF_NON_MODIF);
}

/**********************************

  CB : load a reference AUDIO file
***********************************/
void
on_loadAudio_clicked  (GtkButton *button, APP_data *data)
{
  GKeyFile *keyString;
  GError* err = NULL;
  gchar *uri_path, *filename;

  GtkWidget *window1 = data->appWindow;
  keyString = g_object_get_data(G_OBJECT(window1),"config");

 
  GtkFileFilter *filter = gtk_file_filter_new ();
  gtk_file_filter_add_pattern (filter, "*.mp3");
  gtk_file_filter_add_pattern (filter, "*.MP3");
  gtk_file_filter_add_pattern (filter, "*.wav");
  gtk_file_filter_add_pattern (filter, "*.WAV");
  gtk_file_filter_add_pattern (filter, "*.m4a");
  gtk_file_filter_add_pattern (filter, "*.M4A");
  gtk_file_filter_add_pattern (filter, "*.OGG");
  gtk_file_filter_add_pattern (filter, "*.ogg");
  gtk_file_filter_add_pattern (filter, "*.WMA");
  gtk_file_filter_add_pattern (filter, "*.wma");
  gtk_file_filter_set_name (filter, _("Audio files"));
  GtkWidget *dialog = create_loadFileDialog(data);
  /* we should replace home dir by the current path if it exists ! */
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), g_get_home_dir());

  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
    gtk_widget_destroy (GTK_WIDGET(dialog)); 
    /* we convert finename's path to URI path */
    uri_path = g_filename_to_uri(filename, NULL,NULL);
    if(data->pipeline) {
       gst_element_set_state (data->pipeline, GST_STATE_NULL);
       //gst_object_unref (data->pipeline);!!! surtout pas 
    }
    g_object_set (G_OBJECT (data->pipeline), "uri", gst_filename_to_uri(filename, &err), NULL);
    /* TODO we free previous Audio datas */
    /* TODO we load AUDIO datas */
    g_free(uri_path);
    // TODO error management */

    /*  we unlock widgets and set dispplays */
    gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget(GTK_WIDGET(window1), "pRadioButtonPlayPauseAudio")) , TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget(GTK_WIDGET(window1), "pRadioButtonRewindAudio")) , TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget(GTK_WIDGET(window1), "pRadioButtonGotoAudio")) , TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget(GTK_WIDGET(window1), "pRadioButtonGoJumpAudio")) , TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget(GTK_WIDGET(window1), "audio_position")) , TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget(GTK_WIDGET(window1), "audio_total")) , TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget(GTK_WIDGET(window1), "audio_position_label")) , TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget(GTK_WIDGET(window1), "audio_total_label")) , TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget(GTK_WIDGET(window1), "audioPlaySpeed")) , TRUE);

    /* vars and flags */
    data->button_pressed=FALSE;
    data->fAudioLoaded=TRUE;
    data->fAudioPlaying=FALSE;
     data->audio_current_position=0;
    audio_get_duration(data->pipeline, &data->audio_total_duration );
    gtk_label_set_markup ( GTK_LABEL(lookup_widget(GTK_WIDGET(window1), "audio_position_label")), "<tt><big>00:00:00</big></tt>");
    gtk_label_set_markup ( GTK_LABEL(lookup_widget(GTK_WIDGET(window1), "audio_total_label")), 
                  g_strdup_printf("<tt><small>/%s</small></tt>", (gchar*)
                     audio_gst_time_to_str(data->audio_total_duration)));
    g_free(filename);

  }
  else
        gtk_widget_destroy (GTK_WIDGET(dialog)); 
}

/************************************

  CB : close current audio file
  and dree datas
************************************/
void
on_AudioCloseFile_clicked  (GtkButton *button, APP_data *data)
{
  GKeyFile *keyString;
  GError* err = NULL;
  gchar *uri_path, *filename;

  GtkWidget *window1 = data->appWindow;
  keyString = g_object_get_data(G_OBJECT(window1),"config");

  // printf("demande libérer mémoire fichier audio ! \n");
    gtk_label_set_markup ( GTK_LABEL(lookup_widget(GTK_WIDGET(window1), "audio_position_label")), "<tt><big>--:--:--</big></tt>");
    gtk_label_set_markup ( GTK_LABEL(lookup_widget(GTK_WIDGET(window1), "audio_total_label")), "<tt><small>/--:--:--</small></tt>");
    gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget(GTK_WIDGET(window1), "pRadioButtonPlayPauseAudio")) , FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget(GTK_WIDGET(window1), "pRadioButtonRewindAudio")) , FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget(GTK_WIDGET(window1), "pRadioButtonGotoAudio")) , FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget(GTK_WIDGET(window1), "pRadioButtonGoJumpAudio")) , FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget(GTK_WIDGET(window1), "audio_position")) , FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget(GTK_WIDGET(window1), "audio_total")) , FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget(GTK_WIDGET(window1), "audio_position_label")) , FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget(GTK_WIDGET(window1), "audio_total_label")) , FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(lookup_widget(GTK_WIDGET(window1), "audioPlaySpeed")) , FALSE);
   /* vars and flags */
    data->button_pressed=FALSE;
    data->fAudioLoaded=FALSE;
    data->fAudioPlaying=FALSE;
    data->audio_total_duration=0;
    data->audio_current_position=0;
    if(data->pipeline) {
       gst_element_set_state (data->pipeline, GST_STATE_NULL);
       //gst_object_unref (data->pipeline);!!! surtout pas 
    }
}
/**********************************

  CB : load a reference PDF file
***********************************/
void
on_loadPDF_clicked  (GtkButton *button, APP_data *data)
{
  GKeyFile *keyString;
  GError* err = NULL;
  gchar *uri_path, *filename;
  cairo_t *cr;
  gint w, h; 
  gdouble width, height, ratio;
  GtkWidget *canvas;
  PopplerPage *page;
  PopplerDocument *doc =NULL;

  GtkWidget *window1 = data->appWindow;
  keyString = g_object_get_data(G_OBJECT(window1),"config");
  canvas = lookup_widget(GTK_WIDGET(window1), "crPDF");

  /* we free previous search datas */
  search_free_PDF_search_datas(data);
 
  GtkFileFilter *filter = gtk_file_filter_new ();
  gtk_file_filter_add_pattern (filter, "*.pdf");
  gtk_file_filter_add_pattern (filter, "*.PDF");
  gtk_file_filter_set_name (filter, _("PDF files"));
  GtkWidget *dialog = create_loadFileDialog(data);
  /* we should replace home dir by the current path if it exists ! */
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), g_get_home_dir());

  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
    gtk_widget_destroy (GTK_WIDGET(dialog)); 
    /* we convert finename's path to URI path */
    uri_path = g_filename_to_uri(filename, NULL,NULL);
    /* we load PDF Poppler's doc */
    doc = poppler_document_new_from_file(uri_path, NULL, &err);
    g_free(uri_path);
    if (!doc) {
        printf("%s\n", err->message);
        g_error_free(err);
        return;
    }
    /* we store PDF filename and path in config file */
    g_key_file_set_string(keyString, "application", "current-PDF-file", filename);
    g_key_file_set_string(keyString, "application", "current-PDF-file-basename", 
                      g_filename_display_basename (filename));
 if(data->doc)
    g_object_unref(data->doc);
    undo_free_all_PDF_ops(data);
    data->doc=doc;
    data->curPDFpage=0;
    data->button_pressed=FALSE;
    data->totalPDFpages=poppler_document_get_n_pages(data->doc);
    if(data->currentStack == CURRENT_STACK_PDF ) {
      update_statusbarPDF(data);
    }
    page = poppler_document_get_page(data->doc, 0);
    poppler_page_get_size (page, &width, &height);
    data->PDFWidth=width;
    data->PDFHeight=height;
    ratio = misc_get_PDF_ratio(width,  gtk_widget_get_allocated_width (window1));
    data->PDFratio=ratio;

    w = (gint) (width*ratio);
    h = (gint) (height*ratio);

    cairo_surface_destroy (data->surface);
    /* some stuff to zoom */
    data->surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, w, h);
    cr = cairo_create (data->surface);
    /* fill in white the page */
    cairo_set_source_rgb(cr, 1.0, 1, 1);
    cairo_rectangle(cr, 0, 0, w, h);
    cairo_fill(cr);
    cairo_scale(cr, ratio,ratio);
    poppler_page_render (page, cr);
    g_object_unref(page);
    cairo_destroy (cr);
    gtk_widget_set_size_request (canvas, w, h);
    gtk_widget_queue_draw (canvas);
    g_free(filename);
    data->fPdfLoaded=TRUE;
    update_PDF_state(data, PDF_NON_MODIF);
  }
  else
        gtk_widget_destroy (dialog); 
}

/******************************************
  save a modifyed PDF file, for example
  when the user adds highlightings
  it's a safe increlental writing,
  you can overwrite as you want the current
  file
*******************************************/
void
on_savePDF_clicked  (GtkButton *button, APP_data *data)
{
  GtkWidget *canvas;
  PopplerPage *page;
  GKeyFile *keyString;
  gchar *filename, *newFilename, *tmpFileName, *tmpAuxFile;
  GError* err = NULL;
  GtkWidget *alertDlg;
  GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

  GtkWidget *window1 = data->appWindow;
  keyString = g_object_get_data(G_OBJECT(window1), "config");
  GtkFileFilter *filter = gtk_file_filter_new ();
  gtk_file_filter_add_pattern (filter, "*.pdf");
  gtk_file_filter_add_pattern (filter, "*.PDF");
  gtk_file_filter_set_name (filter, _("PDF files"));
  filename = g_key_file_get_string(keyString, "application", "current-file-basename", NULL);

  GtkWidget *dialog = create_saveFileDialog(data);

  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), 
                         g_path_get_dirname(g_key_file_get_string(keyString, "application", "current-file", NULL) ));
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
  /* we must extract a short filename */
  gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), g_strdup_printf("%s.PDF",filename));
  /* run dialog */
  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
         newFilename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
         /* we check if filename has .pdf extension */
         if( !g_str_has_suffix (newFilename, ".PDF" ) && !g_str_has_suffix (newFilename, ".pdf" )) {
              /* we correct the filename */
              tmpFileName = g_strdup_printf("%s.PDF", newFilename);
              g_free(newFilename);
              newFilename = tmpFileName;
         }
         /* we must check if a file with same name exists */
         if(g_file_test (newFilename, G_FILE_TEST_EXISTS) ) {
           /* dialog to avoid overwriting */

           alertDlg = gtk_message_dialog_new (GTK_WINDOW(dialog),
                          flags,
                          GTK_MESSAGE_ERROR,
                          GTK_BUTTONS_OK_CANCEL,
                          _("The file :\n%s already exists !\nDo you really want to Overwrite an existing PDF file ?"),
                          newFilename);
           if(gtk_dialog_run(GTK_DIALOG(alertDlg))==GTK_RESPONSE_CANCEL) {
              gtk_widget_destroy (GTK_WIDGET(alertDlg));
              gtk_widget_destroy (GTK_WIDGET(dialog));
              g_free(newFilename);
              return;
           }
         }
         /* first we save to temporary folder and clean memory */
         tmpAuxFile = GetTempFileName("Redac");
         poppler_document_save (data->doc, g_filename_to_uri(tmpAuxFile, NULL,NULL),
                                &err);    
         g_object_unref(data->doc);
         /* then we load temporary file to memory */
         data->doc = poppler_document_new_from_file(g_filename_to_uri(tmpAuxFile, NULL,NULL), NULL, &err);
         g_free(tmpAuxFile);
         /* then we save to the TRUE path */
         poppler_document_save (data->doc, g_filename_to_uri(newFilename, NULL,NULL),
                                &err);    
         /* we store PDF filename and path in config file */
         g_key_file_set_string(keyString, "application", "current-PDF-file", newFilename);
         g_key_file_set_string(keyString, "application", "current-PDF-file-basename", 
                      g_filename_display_basename (newFilename)); 
         g_free(newFilename);
         /* and we redraw the PDF window */
         PDF_display_page(data->appWindow, data->curPDFpage, data->doc, data);
         update_PDF_state(data, PDF_NON_MODIF);
  }
  gtk_widget_destroy (GTK_WIDGET(dialog));
  g_free(filename);
}

/*

  CB : save current sketch

*/
void
on_saveSketch_clicked  (GtkButton  *button, APP_data *data_app)
{
  GKeyFile *keyString;
  GError *error = NULL;
  gchar *newFilename, *tmpFileName;
  GtkWidget *alertDlg;
  GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

  GtkWidget *window1 = data_app->appWindow;
  keyString = g_object_get_data(G_OBJECT(window1), "config");
  GtkFileFilter *filter = gtk_file_filter_new ();
  gtk_file_filter_add_pattern (filter, "*.png");
  gtk_file_filter_add_pattern (filter, "*.PNG");
  gtk_file_filter_set_name (filter, _("PNG Image files"));

  GtkWidget *dialog = create_saveFileDialog(data_app);

  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), 
                         g_path_get_dirname(g_key_file_get_string(keyString, "application", "current-file", NULL) ));
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
  /* we must extract a short filename */
  gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), g_strdup_printf("%s.PNG",_("noname")));
  /* run dialog */
  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
         newFilename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
         /* we check if filename has .PNG extension */
         if( !g_str_has_suffix (newFilename, ".PNG" ) && !g_str_has_suffix (newFilename, ".png" )) {
              /* we correct the filename */
              tmpFileName = g_strdup_printf("%s.PNG", newFilename);
              g_free(newFilename);
              newFilename = tmpFileName;
         }
         /* we must check if a file with same name exists */
         if(g_file_test (newFilename, G_FILE_TEST_EXISTS) ) {
           /* dialog to avoid overwriting */
           alertDlg =  gtk_message_dialog_new (GTK_WINDOW(dialog),
                                      flags,
                                      GTK_MESSAGE_ERROR,
                                      GTK_BUTTONS_OK_CANCEL,
                                      _("The file :%s exists !\nOverwrite existing file ?"),
                                      newFilename);
           if(gtk_dialog_run(GTK_DIALOG(alertDlg))==GTK_RESPONSE_CANCEL) {
                gtk_widget_destroy (GTK_WIDGET(alertDlg));
                gtk_widget_destroy (GTK_WIDGET(dialog));
                g_free(newFilename);
                return;
           }
         }
         GdkPixbuf *pPixDatas;
         pPixDatas=gdk_pixbuf_get_from_surface (data_app->Sketchsurface,
                             0,0,CROBAR_VIEW_MAX_WIDTH,CROBAR_VIEW_MAX_HEIGHT);
         gdk_pixbuf_save (pPixDatas, newFilename, "png", &error,  NULL);
         g_object_unref(pPixDatas);
         g_free(newFilename);
  }
  gtk_widget_destroy (GTK_WIDGET(dialog));
}

/**********************
 
callback for help

**********************/
void
on_help_clicked   (GtkWidget  *win)
{
  GtkWidget *dlg;

  dlg = misc_create_help_dialog(win);
  gtk_dialog_run(GTK_DIALOG(dlg));
  gtk_widget_destroy (GTK_WIDGET(dlg));
}

/**************************************

 callback : setup a new environment

***************************************/
void new_project(GtkMenuItem *menuitem, APP_data  *user_data)
{
  GKeyFile *keyString;
  GtkTextBuffer *buffer;
  gchar *path_to_file, buffer_date[81];
  GtkTextTag *tag;
  time_t rawtime;
  gint ret;


  buffer = user_data->buffer;
  GtkWidget *window1 = user_data->appWindow;

  keyString = g_object_get_data(G_OBJECT(window1), "config");
  /* first, we save once more time the current file */
  path_to_file = g_key_file_get_string(keyString, "application", "current-file", NULL);
  ret = save_gtk_rich_text(path_to_file, buffer);
  g_free(path_to_file);
  /* reset undo engine */
  undo_free_all(user_data);
  /* now we clear the datas */
  misc_clear_text(buffer, "left");
  
  /* we get the current date */
  time ( &rawtime );
  strftime(buffer_date, 80, "%c", localtime(&rawtime));/* don't change parameter %x */
  /* now we set-up a new default filename */
  path_to_file =  get_path_to_datas_file(buffer_date);
  gtk_label_set_markup (GTK_LABEL(lookup_widget(GTK_WIDGET(window1), "labelMainTitle")),
                             g_strdup_printf(_("<small><b>%s</b></small>"), path_to_file));

 // gtk_window_set_title (GTK_WINDOW(window1),g_strdup_printf(_("Redac:%s"), path_to_file));
  /* and we save the new "empty file" */
  ret = save_gtk_rich_text(path_to_file, buffer);  
  /* rearrange list of recent files */
  rearrange_recent_file_list(keyString);
  /* we change the default values for gkeyfile */
  store_current_file_in_keyfile(keyString, path_to_file, " ");
  user_data->fPdfLoaded=FALSE;
}

