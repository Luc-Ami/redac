/*
 * DO NOT EDIT THIS FILE - it is generated by Glade.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


/* translations */
#include <libintl.h>
#include <locale.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gtk/gtk.h>

#include "support.h"

GtkWidget*
lookup_widget                          (GtkWidget       *widget,
                                        const gchar     *widget_name)
{
  GtkWidget *parent, *found_widget;

  for (;;)
    {
      if (GTK_IS_MENU (widget))
        parent = gtk_menu_get_attach_widget (GTK_MENU (widget));
      else
        parent = gtk_widget_get_parent (widget);
      if (!parent)
        parent = (GtkWidget*) g_object_get_data (G_OBJECT (widget), "GladeParentKey");
      if (parent == NULL)
        break;
      widget = parent;
    }

  found_widget = (GtkWidget*) g_object_get_data (G_OBJECT (widget),
                                                 widget_name);
  if (!found_widget)
    g_warning ("Widget not found: %s", widget_name);
  return found_widget;
}

static GList *pixmaps_directories = NULL;

/* Use this function to set the directory containing installed pixmaps. */
void
add_pixmap_directory                   (const gchar     *directory)
{
  pixmaps_directories = g_list_prepend (pixmaps_directories,
                                        g_strdup (directory));
}

/* This is an internally used function to find pixmap files. */
static gchar*
find_pixmap_file                       (const gchar     *filename)
{
  GList *elem;

  /* We step through each of the pixmaps directory to find it. */
  elem = pixmaps_directories;
  while (elem)
    {
      gchar *pathname = g_strdup_printf ("%s%s%s", (gchar*)elem->data,
                                         G_DIR_SEPARATOR_S, filename);
      if (g_file_test (pathname, G_FILE_TEST_EXISTS))
        return pathname;
      g_free (pathname);
      elem = elem->next;
    }
  return NULL;
}

/* This is an internally used function to create pixmaps. */
GtkWidget*
create_pixmap                          (GtkWidget       *widget,
                                        const gchar     *filename)
{
  gchar *pathname = NULL;
  GtkWidget *pixmap;

  if (!filename || !filename[0])
      return gtk_image_new ();

  pathname = find_pixmap_file (filename);

  if (!pathname)
    {
      g_warning (_("Couldn't find pixmap file: %s"), filename);
      return gtk_image_new ();
    }

  pixmap = gtk_image_new_from_file (pathname);
  g_free (pathname);
  return pixmap;
}

/* This is an internally used function to create pixmaps. */
GdkPixbuf*
create_pixbuf                          (const gchar     *filename)
{
  gchar *pathname = NULL;
  GdkPixbuf *pixbuf;
  GError *error = NULL;

  if (!filename || !filename[0])
      return NULL;

  pathname = find_pixmap_file (filename);

  if (!pathname)
    {
      g_warning (_("Couldn't find pixmap file: %s"), filename);
      return NULL;
    }

  pixbuf = gdk_pixbuf_new_from_file (pathname, &error);
  if (!pixbuf)
    {
      fprintf (stderr, "Failed to load pixbuf file: %s: %s\n",
               pathname, error->message);
      g_error_free (error);
    }
  g_free (pathname);
  return pixbuf;
}

/* This is used to set ATK action descriptions. */
void
glade_set_atk_action_description       (AtkAction       *action,
                                        const gchar     *action_name,
                                        const gchar     *description)
{
  gint n_actions, i;

  n_actions = atk_action_get_n_actions (action);
  for (i = 0; i < n_actions; i++)
    {
      if (!strcmp (atk_action_get_name (action, i), action_name))
        atk_action_set_description (action, i, description);
    }
}

/******************************
  get a path to the  file
  containing all the Gtk rich 
  text datas
*******************************/
gchar *get_path_to_datas_file(gchar *str)
{
 gint i;
 gchar *s1=g_strdup_printf("%s", str);
 for(i=0;i<=strlen(s1);i++) {
  if(s1[i]==' ' || s1[i]=='.')
    s1[i]='_';
 }
 gchar *s2 = g_strconcat(g_get_home_dir(),"/","Notes-", s1,".kw", NULL);
 g_free(s1);
 return s2;
}

/*****************************
 remove all unwanted chars from
 a filename
********************************/

/*******************************
 check if a selected part of the
text has a specicic tag 
*******************************/
gboolean get_tag_in_selection(gchar *tag_name, GtkTextIter start)
{
  GSList *tags, *l;/* from pidgin code, thanks, here =https://sourceforge.net/p/pidgin/mailman/pidgin-commits/?viewmonth=200512 */
  gboolean retvalue = FALSE;

  tags = gtk_text_iter_get_tags(&start);
  GtkTextTag *tag3;
  for(l = tags; l != NULL; l = l->next) {
     tag3 = GTK_TEXT_TAG(l->data);
     if(tag3) {
        gchar *sTag;
        g_object_get(tag3, "name", &sTag, NULL);/* mandatory with Gtk3, we can't use tag3->name */
        if(g_strcasecmp(sTag, tag_name)==0) {
                 // printf("count=%d Bingo=%s\n", count, sTag);
                  retvalue =TRUE;
        }
        g_free(sTag);
     }
  }
  g_slist_free(tags);
  return retvalue;
}

/*************************************
  play with css for a widget 
 see : https://www.developpez.net/forums/d1546525/c-cpp/bibliotheques/gtkp/gtkp-c-cpp/gtk3p-gestion-styles/ 
 and better : https://developer.gnome.org/gtk3/stable/gtk-migrating-GtkStyleContext-css.html
*************************************/
void toggle_css_value(GtkWidget *button, gboolean toggle_state)
{
 GtkStyleContext *css;
 GdkRGBA *color1;
 GdkRGBA  color2;

 //css = gtk_widget_get_style_context (GTK_WIDGET(button));
 //gtk_style_context_get (css, GTK_STATE_FLAG_NORMAL,
   //                    "background-color", &color1,
     //                  NULL);
 if(toggle_state) {
  // gtk_style_context_remove_class (css,"red_bg");
  // gtk_style_context_add_class (css,"green_bg");
  gtk_toggle_tool_button_set_active (GTK_TOOL_BUTTON(button), TRUE); 
  //gtk_tool_button_set_label(GTK_TOOL_BUTTON(button) , "*");
  color2.red = 0.1;
  color2.green=1.0;
  color2.blue = 0.1;
  color2.alpha = 0.5;
  //gtk_style_context_lookup_color (css, "bg_color", &color2);
 }
 else {
   //gtk_style_context_remove_class (css,"green_bg");
   //gtk_style_context_add_class (css,"red_bg");
  gtk_toggle_tool_button_set_active (GTK_TOOL_BUTTON(button), FALSE); 
  //gtk_tool_button_set_label(GTK_TOOL_BUTTON(button) , "");
  color2.red = 1.0;
  color2.green=0.1;
  color2.blue = 0.1;
  color2.alpha = 0.5;
  //gtk_style_context_lookup_color (css, "bg_color", &color2);
 }
 //gdk_rgba_free (color1);
}

/*********************************************

  count word in a GtkTextBuffer ;
 adapted from a generic C source here :
https://www.geeksforgeeks.org/count-words-in-a-given-string/
  parameter : a pointer on a GtkTextBuffer
  output = number of words
********************************************/
unsigned  countWords(GtkTextBuffer *buffer)
{
  gboolean state = FALSE;
  unsigned  wc = 0;  /* word count */
  gchar *str, *str_at_start;
  GtkTextIter start, end;
 
  if(buffer==NULL)
     return -1;
  /* we get the str inside the gtktextbuffer */
  gtk_text_buffer_get_start_iter (buffer, &start);
  gtk_text_buffer_get_end_iter (buffer, &end);
  str = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
  str_at_start = str;/* in other cases g_free() leads to a segfault because *str is manipulated */
  /* Scan all characters one by one */
  while (*str) {
      /* If next character is a separator, set the state as OUT */
        if (*str == ' ' || *str == '\n' || *str == '\t')
            state = FALSE; 
        /*  If next character is not a word separator and 
         state is OUT, then set the state as IN and 
         increment word count */
        else if (state == FALSE) {
            state = TRUE;
            ++wc;
        }
        /* Move to next character */
        ++str;
  }/* wend */
  g_free(str_at_start);
  return wc;
}
