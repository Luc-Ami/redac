/*
 * DO NOT EDIT THIS FILE - it is generated by Glade.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef SUPPORT_H
#define SUPPORT_H

#include <gtk/gtk.h>

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    g_object_ref (widget), (GDestroyNotify) g_object_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)

/*
 * Standard gettext macros.
 */
#define _(String) gettext (String)

/* directories : managed by config.h */


#define KILOWRITER_CONFIG "config.ini" /* Configuration name for Kilowriter options - main()*/
#define KW_ALIGNMENT_LEFT 0
#define KW_ALIGNMENT_CENTER 1
#define KW_ALIGNMENT_RIGHT 2
#define KW_ALIGNMENT_FILL 3
#define KW_ALIGNMENT_DEFAULT -1


/*
 * Public Functions.
 */

/*
 * This function returns a widget in a component created by Glade.
 * Call it with the toplevel widget in the component (i.e. a window/dialog),
 * or alternatively any widget in the component, and the name of the widget
 * you want returned.
 */
GtkWidget*  lookup_widget              (GtkWidget       *widget,
                                        const gchar     *widget_name);


/* Use this function to set the directory containing installed pixmaps. */
void        add_pixmap_directory       (const gchar     *directory);


/*
 * Private Functions.
 */

/* This is used to create the pixmaps used in the interface. */
GtkWidget*  create_pixmap              (GtkWidget       *widget,
                                        const gchar     *filename);

/* This is used to create the pixbufs used in the interface. */
GdkPixbuf*  create_pixbuf              (const gchar     *filename);

/* This is used to set ATK action descriptions. */
void        glade_set_atk_action_description (AtkAction       *action,
                                              const gchar     *action_name,
                                              const gchar     *description);
gchar *get_path_to_datas_file(gchar *str);
gboolean get_tag_in_selection(gchar *tag_name, GtkTextIter start);
void toggle_css_value(GtkWidget *button, gboolean toggle_state);
unsigned  countWords(GtkTextBuffer *buffer);

void add_ui_directory  (const gchar  *directory);
gchar *find_ui_file  (const gchar  *filename);
gchar *find_pixmap_file (const gchar *filename);

#endif /* SUPPORT_H */
