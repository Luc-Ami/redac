#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


#ifndef PDF_H
#define PDF_H

#include <poppler.h>
#include "support.h"
#include "misc.h"

void PDF_display_page (GtkWidget *window1, gint page, PopplerDocument *pdoc, APP_data    *data);
void PDF_moveUp (GtkWidget *parentWindow, APP_data    *data);
void PDF_moveDown (GtkWidget *parentWindow, APP_data    *data);
void PDF_moveBackward (GtkWidget *parentWindow, APP_data    *data);
void PDF_moveForward (GtkWidget *parentWindow, APP_data    *data);
void PDF_moveHome (GtkWidget *parentWindow, APP_data    *data);
void PDF_moveEnd (GtkWidget *parentWindow, APP_data    *data);
void PDF_goto(GtkWidget *parentWindow, APP_data    *data, gint pg);
void PDF_set_text_annot_selection (gint x, gint y, gint w, gint h, gint pdf_page, PopplerDocument *doc, GtkWidget *win, GtkWidget *sw, APP_data    *data);
void PDF_set_highlight_selection (gint x, gint y, gint w, gint h, gint pdf_page, PopplerDocument *doc, GtkWidget *win, GtkWidget *sw, APP_data    *data);
void PDF_get_text_selection (gint x, gint y, gint w, gint h, gint pdf_page, GtkWidget *sw, APP_data *data);
GList *PDF_get_annot_mapping(APP_data    *data);
PopplerAnnot * PDF_find_annot_at_position(gint x, gint y, APP_data    *data);
void PDF_set_free_text_annot_selection (gint x, gint y, gint w, gint h, gint pdf_page, PopplerDocument *doc, GtkWidget *win, GtkWidget *sw, APP_data *data);
void undo_free_all_PDF_ops(APP_data *data);
#endif /* PDF_H */
