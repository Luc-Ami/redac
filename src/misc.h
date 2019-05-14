/*
 * DO NOT EDIT THIS FILE - it is generated by Glade.
 */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


#ifndef MISC_H
#define MISC_H

#include <gst/gst.h>
#include <gtkspell/gtkspell.h>
#include <poppler.h>
#include "support.h"

#define PDF_VIEW_MAX_WIDTH 2000
#define PDF_VIEW_MAX_HEIGHT 2000

#define CROBAR_VIEW_MAX_WIDTH 1000
#define CROBAR_VIEW_MAX_HEIGHT 1000

#define CURRENT_STACK_EDITOR 0
#define CURRENT_STACK_PDF 1
#define CURRENT_STACK_SKETCH 2

#define PDF_SEL_MODE_TEXT 0
#define PDF_SEL_MODE_PICT 1
#define PDF_SEL_MODE_HIGH 2
#define PDF_SEL_MODE_NOTE 3

#define PDF_SCROLL_STEP 10

#define LEFT_QUADDING 0
#define CENTER_QUADDING 1
#define RIGHT_QUADDING 2
#define FILL_QUADDING 3

#define PDF_LOADED 0
#define PDF_NON_MODIF 1
#define PDF_MODIF 2


/* structures */

typedef struct  {
  gint curStack;
  gint PDFpage;
  gint opCode;
  gint prevQuadding;
  gint offset;
  GtkTextIter start_sel, end_sel;
  guint8 *serialized_buffer;
  gsize buffer_length;
  gsize str_len;
  GtkTextMark *undoMark;
  GtkTextMark *beforeMark;
  gboolean fIsStart;
  gint x1, y1, x2, y2;
  gdouble pen_width;
  GdkRGBA color;
  GdkPixbuf *pix;
  PopplerAnnot *annot;
  gint annotType;
  gchar *annotStr;
  PopplerRectangle rectangle;
} undo_datas;


typedef struct {
  gint x1, y1, x2, y2, w, h, x1_event_root, y1_event_root;

  gboolean button_pressed;
  gboolean fPencilTool;
  gboolean fDarkTheme;
  gboolean fPdfLoaded;
  gboolean fAudioLoaded;
  gboolean fAudioPlaying;

  GtkWidget    *window;
  GtkWidget    *appWindow;
  GtkWidget    *PDFdrawable;
  GtkWidget    *SketchDrawable;
  GtkWidget    *PDFScrollable;
  GtkWidget    *SketchScrollable;
  GtkWidget    *pBtnUndo;
  gint         clipboardMode;  /* 0= text 1=picture */
  gdouble      iAudioSmartRew; /* Audio : rewing jump in secs when user clicks Play/pause button */
  gdouble      iAudioSmartJump; /* Audio : rewing jump in secs when user clicks forward/backward buttons */
  gint kw_paragraph_alignment;
  GtkWidget *statusbar1;
  GtkWidget *statusbar2;
  GKeyFile *keystring;
  GtkTextView *view;
  GtkTextBuffer *buffer;
  GtkStack *stack;
  cairo_surface_t *Sketchsurface;
  cairo_surface_t *surface;
  GtkTextIter *curpos;
  gint curPDFpage;
  gint totalPDFpages;
  gdouble PDFWidth;
  gdouble PDFHeight;
  gdouble PDFratio;
  gint currentStack;
  PopplerDocument *doc;
  GList *pdfSearch;
  GList *pdfAnnotMapping;
  GList *undoList;
  PopplerAnnot *current_annot;
  PopplerAnnotMapping map;
  undo_datas undo;
  GtkSpellChecker* spell;
  GstElement *pipeline;
  gint64 audio_total_duration;
  gint64 audio_current_position;
} APP_data;

typedef struct {
   gint page;
   gint nb_hits_at_page;
   GList *hits;
} PDF_search_results;



GArray *
pgd_annots_create_quads_array_for_rectangle (PopplerRectangle *rect);
gint misc_get_current_alignment(GtkTextBuffer *buffer);
void misc_colorinvert_picture(GdkPixbuf *pb);
void misc_set_gui_in_PDF_mode(GtkWidget *window1, gint prevStack);
void misc_set_gui_in_editor_mode(GtkWidget *window1, gint prevStack);
void misc_set_gui_in_sketch_mode(GtkWidget *window1, gint prevStack);
void update_statusbarSketch(APP_data *data);
void update_statusbarPDF(APP_data *data);
void update_PDF_state(APP_data *data, gint state);
void misc_setup_text_buffer_tags(GtkTextBuffer *buffer);
gdouble misc_get_PDF_ratio(gdouble pdf_width, gdouble draw_width);
void misc_clear_text(GtkTextBuffer *buffer, const gchar  *tag);
void misc_append_empty_paragraph(GtkTextBuffer *buffer, gint row, gint total);
void misc_remove_alignment_tags(GtkTextBuffer *buffer, GtkTextIter start, GtkTextIter end);
gchar *misc_get_pango_string(const gchar *key, const gint modifier);
void undo_popup_menu(GtkWidget *attach_widget, GtkMenu *menu);
gint misc_get_paragraph_quadding(GtkTextBuffer *buffer, GtkTextIter iter);
GtkTextTag *misc_get_tag_from_code(GtkTextBuffer *buffer, gint code);
void misc_init_vars(APP_data *data );
void misc_init_spell_checker(APP_data *data );
gchar *misc_get_extract_from_document(APP_data *data );
void misc_jump_to_end_view(GtkTextBuffer *buffer, GtkTextView *view);
void misc_set_sensitive_format_buttons(gboolean state, APP_data *data);
#endif /* MISC_H */
