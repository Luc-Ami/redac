#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

/* translations */
#include <libintl.h>
#include <locale.h>

#include <stdlib.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <poppler.h>
#include "support.h"
#include "interface.h"
#include "callbacks.h"
#include "mttfiles.h"
#include "mttexport.h"
#include "misc.h"
#include "search.h"
#include "pdf.h"
#include "undo.h"
#include "paving.h"

/********************
  global vars 
*******************/

extern gchar *gConfigFile;

static gboolean fdont_care=FALSE;
static gboolean show_toolbar=TRUE;
static gboolean fBold = FALSE;
static gboolean fItalic = FALSE;
static gboolean fUnderline = FALSE;
static gboolean fSuperscript = FALSE;
static gboolean fSubscript = FALSE;
static gboolean fHighlight = FALSE;
static gboolean fStrikethrough = FALSE;
static gboolean fQuotation = FALSE;
static gboolean fUserClickedButton = FALSE;
static gint iPendingFormat = 0; /* a flag to report a formatting to the next call */
static gint iPendingCol =-1, iPendingRow = -1;
static gint iColMem =-1, iRowMem= -1; /* in order to check if the user gone backward */
static gint kw_paragraph_alignment = KW_ALIGNMENT_LEFT;

/****************************************
  callback : updade infos on statusbar 
*****************************************/ 
void update_statusbar(GtkTextBuffer *buffer, APP_data *data) 
{
  gchar *msg;
  gint row, col, total_chars, total_words;
  GtkTextIter iter, start, end;
  GtkWidget *window1=NULL, *button;
  GtkTextTag *tag;
  GtkTextTagTable *tagTable1;
  gboolean ok;
  GKeyFile *keyString;
  GtkTextView *view;
  GtkStatusbar  *statusbar=data->statusbar1;

  window1 = data->appWindow;
  view=data->view;

  fUserClickedButton = TRUE;
  gtk_statusbar_pop(statusbar, 0); 
  gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
  
  row = gtk_text_iter_get_line(&iter);
  col = gtk_text_iter_get_line_offset(&iter);
  total_chars = gtk_text_buffer_get_char_count(buffer);
  total_words = countWords(buffer);
  msg = g_strdup_printf(_("Paragraph: %d/%d  %d Chars  %d Words"),row+1,  
                           gtk_text_buffer_get_line_count(buffer), total_chars, total_words);/* page=(row+1)/66+1 */
  /* section to apply marking on last char */

  start = iter;
  end = iter;

  iPendingFormat++;
  tagTable1 = gtk_text_buffer_get_tag_table(buffer);
  if(fBold) {
     tag = gtk_text_tag_table_lookup(tagTable1, "bold");
     if((col<iColMem)||(row<iRowMem)) {
        fBold=FALSE;
        iPendingFormat = -1;
     }
     else {
         if(iPendingFormat>1) {
           start = end;      
           gtk_text_iter_set_line_offset (&start,iPendingCol);
           gtk_text_buffer_apply_tag(buffer, tag, &start, &end);
         } 
     }
  }
  if(fItalic) {
     tag = gtk_text_tag_table_lookup(tagTable1, "italic");
     if((col<iColMem)||(row<iRowMem)) {
        fItalic=FALSE;
        iPendingFormat = -1;
     }
     else {
       if(iPendingFormat>1) {
         start = end;      
         gtk_text_iter_set_line_offset (&start,iPendingCol);
         gtk_text_buffer_apply_tag(buffer, tag, &start, &end);
       }
     }
  }
  if(fUnderline) {
     //iPendingFormat++;
     tag = gtk_text_tag_table_lookup(tagTable1, "underline");
     if((col<iColMem)||(row<iRowMem)) {
        fUnderline=FALSE;
        iPendingFormat = -1;
     }
     else {
       if(iPendingFormat>1) {
         start = end;     
         gtk_text_iter_set_line_offset (&start,iPendingCol);
         gtk_text_buffer_apply_tag(buffer, tag, &start, &end);
       }
     }
  }

  if(fSuperscript) {
     tag = gtk_text_tag_table_lookup(tagTable1, "superscript");
     if((col<iColMem)||(row<iRowMem)) {
        fSuperscript=FALSE;
        iPendingFormat = -1;
     }
     else {
       if((iPendingFormat>1) && (iPendingCol>=0)) {
         start = end;     
         gtk_text_iter_set_line_offset (&start,iPendingCol);
         gtk_text_buffer_apply_tag(buffer, tag, &start, &end);
       }
     }
  }

  if(fSubscript) {
     tag = gtk_text_tag_table_lookup(tagTable1, "subscript");
     if((col<iColMem)||(row<iRowMem)) {
        fSubscript=FALSE;
        iPendingFormat = -1;
     }
     else {
       if((iPendingFormat>1) && (iPendingCol>=0)) {
         start = end;     
         gtk_text_iter_set_line_offset (&start,iPendingCol);
         gtk_text_buffer_apply_tag(buffer, tag, &start, &end);
       }
     }
  }
  if(fHighlight) {
     tag = gtk_text_tag_table_lookup(tagTable1, "highlight");
     if((col<iColMem)||(row<iRowMem)) {
        fHighlight=FALSE;
        iPendingFormat = -1;
     }
     else {
       if((iPendingFormat>1) && (iPendingCol>=0)) {
         start = end;     
         gtk_text_iter_set_line_offset (&start,iPendingCol);
         gtk_text_buffer_apply_tag(buffer, tag, &start, &end);
       }
     }
  }  
  if(fStrikethrough) {
     tag = gtk_text_tag_table_lookup(tagTable1, "strikethrough");
     if((col<iColMem)||(row<iRowMem)) {
        fStrikethrough=FALSE;
        iPendingFormat = -1;
     }
     else {
       if((iPendingFormat>1) && (iPendingCol>=0)) {
         start = end;     
         gtk_text_iter_set_line_offset (&start,iPendingCol);
         gtk_text_buffer_apply_tag(buffer, tag, &start, &end);
       }
     }
  }  
  if(fQuotation) {
     tag = gtk_text_tag_table_lookup(tagTable1, "quotation");
     if((col<iColMem)||(row<iRowMem)) {
        fQuotation=FALSE;
        iPendingFormat = -1;
     }
     else {
       if((iPendingFormat>1) && (iPendingCol>=0)) {
         start = end;     
         gtk_text_iter_set_line_offset (&start,iPendingCol);
         gtk_text_buffer_apply_tag(buffer, tag, &start, &end);
       }
     }
  }  

  if(iPendingFormat>1)
     iPendingFormat=0;
  gtk_statusbar_push(statusbar, 0, msg);
  g_free(msg);

  keyString = g_object_get_data(G_OBJECT(window1), "config");
  msg = g_strdup_printf(_("%s"), g_key_file_get_string(keyString, "application", "current-file", NULL));
  gtk_label_set_markup (lookup_widget(GTK_WIDGET(window1), "labelMainTitle"),
                             g_strdup_printf(_("<small><b>%s-<span foreground=\"red\">modified</span></b></small>"), msg));
//  gtk_header_bar_set_subtitle (lookup_widget(GTK_WIDGET(window1), "headBar"),
  //                           msg);
  //gtk_window_set_title (GTK_WINDOW(window1),msg);
  g_free(msg);
  /* now we check which tags are used */
  fUserClickedButton = FALSE;/* in order to avoid double calls to format callbacks */
  button = GTK_TOOL_BUTTON(lookup_widget(GTK_WIDGET(window1), "button_bold"));
  if((get_tag_in_selection("bold", start)) || ((iPendingFormat>0)&&(fBold) ) ) {
     toggle_css_value(button, TRUE);
  }
  else {
    toggle_css_value(button, FALSE);
  }
  button = GTK_TOOL_BUTTON(lookup_widget(GTK_WIDGET(window1), "button_italic"));
  if((get_tag_in_selection("italic", start)) || ((iPendingFormat>0)&&(fItalic) )  ) {
          toggle_css_value(button, TRUE);
  }
  else {
          toggle_css_value(button, FALSE);
  }
  button = GTK_TOOL_BUTTON(lookup_widget(GTK_WIDGET(window1), "button_underline"));
  if((get_tag_in_selection("underline", start)) || ((iPendingFormat>0)&&(fUnderline) ) ) {
          toggle_css_value(button, TRUE);
  }
  else {
          toggle_css_value(button, FALSE);
  }

  button = GTK_TOOL_BUTTON(lookup_widget(GTK_WIDGET(window1), "button_superscript"));
  if((get_tag_in_selection("superscript", start)) || ((iPendingFormat>0)&&(fSuperscript) ) ) {
          toggle_css_value(button, TRUE);
  }
  else {
          toggle_css_value(button, FALSE);
  }
  button = GTK_TOOL_BUTTON(lookup_widget(GTK_WIDGET(window1), "button_subscript"));
  if((get_tag_in_selection("subscript", start)) || ((iPendingFormat>0)&&(fSubscript) ) ) {
          toggle_css_value(button, TRUE);
  }
  else {
          toggle_css_value(button, FALSE);
  }
  button = GTK_TOOL_BUTTON(lookup_widget(GTK_WIDGET(window1), "button_highlight"));
  if((get_tag_in_selection("highlight", start)) || ((iPendingFormat>0)&&(fHighlight) ) ) {
          toggle_css_value(button, TRUE);
  }
  else {
          toggle_css_value(button, FALSE);
  }
  button = GTK_TOOL_BUTTON(lookup_widget(GTK_WIDGET(window1), "button_quotation"));
  if((get_tag_in_selection("quotation", start)) || ((iPendingFormat>0)&&(fQuotation) ) ) {
          toggle_css_value(button, TRUE);
  }
  else {
          toggle_css_value(button, FALSE);
  } 
  button = GTK_TOOL_BUTTON(lookup_widget(GTK_WIDGET(window1), "button_strikethrough"));
  if((get_tag_in_selection("strikethrough", start)) || ((iPendingFormat>0)&&(fStrikethrough) ) ) {
          toggle_css_value(button, TRUE);
  }
  else {
          toggle_css_value(button, FALSE);
  }
 /* we get current alignment - if different, we change the radiobuttons */
  gint al=misc_get_current_alignment(buffer);
  if(al!=kw_paragraph_alignment) {
      set_alignment_button(window1, al);
  }
  fUserClickedButton = TRUE;

}

void mark_set_callback(GtkTextBuffer *buffer, 
    const GtkTextIter *new_location, GtkTextMark *mark, APP_data *data) 
{                      
  update_statusbar(buffer,data);
}


/***********************************
  draw PDF surface
  thanks to : // https://yassernour.wordpress.com/2010/04/04/how-hard-to-build-a-pdf-viewer/
************************************/
gboolean draw_callback(GtkWidget *widget, cairo_t *cr, APP_data *data) 
{
  if(data->doc!=NULL) {
     cairo_set_source_surface (cr, data->surface, 0, 0);
     cairo_paint (cr);
  }
  return FALSE;
}

/***********************************
 
  draw sketch Stack
************************************/
gboolean sketch_draw_callback(GtkWidget *widget, cairo_t *cr, APP_data *data) 
{
  cairo_set_source_surface (cr, data->Sketchsurface, 0, 0);
  cairo_paint (cr);
  return FALSE;
}

/****************************
 pencil button toggled
***************************/
void on_button_button_pencil_toggled(GtkButton  *button, APP_data *user_data)
{
  GtkToolItem *tmpButton=NULL;

  tmpButton = GTK_TOOL_BUTTON(lookup_widget(GTK_WIDGET(button), "button_pencil"));
  if (gtk_toggle_tool_button_get_active (GTK_TOOL_BUTTON(tmpButton)))  {
      user_data->fPencilTool=TRUE;
  }
  else
    user_data->fPencilTool=FALSE;
}

/***************************************************
  Draw a text on the screen
  the origin of the line was stored during the lAST
  mouse left-click
****************************************************/
static void draw_text (gdouble x, gdouble y, APP_data *data, gchar *str )
{
  GKeyFile *keyString;
  GdkRGBA color;   
  GtkWidget *pBtnColor; 
  cairo_t *cr = NULL;
  cairo_text_extents_t extents;
  gint root_xs, root_ys, i=0, j=0, w=0, h=0, xorg, yorg;
  gchar *newFont;
 
  gchar **sdata = g_strsplit (str, "\n", -1);

  /* get absolute screen coordinates */
  gdk_window_get_origin (gtk_widget_get_window (data->SketchDrawable), &root_xs, &root_ys);  
  /* we get the current RGBA color */
  pBtnColor=lookup_widget(GTK_WIDGET(data->appWindow), "color_button");
  gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER(pBtnColor), &color);
  /* Paint to the surface, where we store our state */
  cr = cairo_create (data->Sketchsurface);
  cairo_set_source_rgb(cr, color.red, color.green, color.blue);

  keyString = g_object_get_data(G_OBJECT(data->appWindow), "config");

  PangoContext* context = gtk_widget_get_pango_context  (data->SketchScrollable);
  PangoFontDescription *desc = pango_context_get_font_description(context);    
  newFont = g_key_file_get_string(keyString, "sketch", "font", NULL);
  
  if (newFont != NULL) { 
        desc = pango_font_description_from_string (newFont);
        if (desc != NULL) {
          cairo_select_font_face(cr, pango_font_description_get_family (desc), 
                                     pango_font_description_get_style(desc), 
                                     CAIRO_FONT_WEIGHT_BOLD);
          cairo_set_font_size(cr, pango_font_description_get_size(desc)/1000 );
          pango_font_description_free(desc);
        }
        else {
            cairo_select_font_face(cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
            cairo_set_font_size(cr, 15 );
        }
        g_free(newFont);
  }
 
  /* we compute total size for undo engine */
  for( i = 0; sdata[i]; i++ ) {
    cairo_text_extents(cr, sdata[i], &extents);
    if(extents.width>w)
      w=(gint)extents.width;
    if(extents.height>h)
      h=(gint)extents.height;
    j++;
  }

  /* undo engine */
  xorg=data->x1-root_xs;
  if(xorg<0)
    xorg=0;
  yorg=data->y1-root_ys-h;
  if(yorg<0)
    yorg=0;

  data->undo.pix=gdk_pixbuf_get_from_surface (data->Sketchsurface,
                             xorg, yorg, w+10, (gint)(1.5*j*h)+2);

  data->undo.x1=xorg;
  data->undo.y1=yorg;
  if(w>0 && h>0) {
     undo_push(data->currentStack,OP_SKETCH_ANNOT, data);
     /* loop until all splitted multiline text is displayed */
     cairo_move_to (cr, (gdouble)xorg, (gdouble)data->y1-root_ys);
     for( i = 0; sdata[i]; i++ ) {
      cairo_show_text(cr, sdata[i]);
      cairo_move_to (cr, (gdouble)xorg, (gdouble)data->y1-root_ys+(i+1)*extents.height*1.5);
      cairo_stroke(cr);
    }
  }/* endif */

  cairo_destroy(cr);
  gtk_widget_queue_draw (data->SketchDrawable);
  g_strfreev(sdata);
}
/***************************************************
  Draw a line on the screen
  the origin of the line was stored during the LAST
  mouse left-click
 x, y : relative coordinates
****************************************************/
static void draw_brush (gdouble x, gdouble y, APP_data *data, gdouble pen_width )
{
  gint x2,y2, w, h;
  gdouble pen_width_margin;
  GdkRGBA color;   
  GtkWidget *pBtnColor; 
  cairo_t *cr = NULL;

  pen_width_margin=(pen_width/2)+1;
  /* we get the current RGBA color */
  pBtnColor=lookup_widget(GTK_WIDGET(data->appWindow), "color_button");
  gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER(pBtnColor), &color);
  /* Paint to the surface, where we store our state */
  cr = cairo_create (data->Sketchsurface);
  cairo_set_source_rgb(cr, color.red, color.green, color.blue);
  cairo_set_line_width (cr, pen_width);
  /* we get a copy of underlaying rectangle for undo engine */
  w=ABS((gint)x-data->x1);
  h=ABS((gint)y-data->y1);
  /* x2, y2 : coordinates, in relative mode, of upperleft quadrant */
  if((gint)x<data->x1) 
     x2=(gint)x;
  else
   x2=data->x1;

  if((gint)y<data->y1) 
     y2=(gint)y;
  else
   y2=data->y1;

  if(x2<=0)
   x2+pen_width_margin;
  if(y2<=0)
   y2+pen_width_margin;


  if(w>0 && h>0 ) {
    data->undo.pix=gdk_pixbuf_get_from_surface (data->Sketchsurface,
                             x2-pen_width_margin, y2-pen_width_margin,
                             w+(gint)2*pen_width_margin, h+(gint)2*pen_width_margin);

    data->undo.x1=x2-pen_width_margin;
    data->undo.y1=y2-pen_width_margin;
    cairo_move_to (cr, (gdouble)data->x1, (gdouble)data->y1);
    cairo_line_to (cr, x, y);
    cairo_stroke(cr);
    cairo_destroy(cr);
    gtk_widget_queue_draw (data->SketchDrawable);
    data->x1=(gint)x;
    data->y1=(gint)y;
    /* undo engine */
    undo_push(data->currentStack,OP_SET_POINT, data);
  }
}

/********************************
  button-release on PDF renderer
  we check if we are in selection/clip/high/note
  mode and if, we close the selection window
 Then we switch according to clipmode
********************************************/
gboolean on_PDF_draw_button_release_callback(GtkWidget *widget, GdkEvent *event, APP_data *data)
{
  GdkPixbuf *pPixDatas;
  gint root_xs, root_ys;

  if(!data->button_pressed || !data->doc)
     return TRUE;

  gtk_widget_destroy (GTK_WIDGET(data->window));
 
  data->button_pressed=FALSE;
  /* get absolute screen coordinates */
  gdk_window_get_origin (gtk_widget_get_window (data->PDFScrollable), &root_xs, &root_ys);

  switch(data->clipboardMode) {
     case PDF_SEL_MODE_TEXT: { /*clip mode text */
       /* voir : poppler_page_get_crop_box ()*/
       if( data->w>0 && data->h>0) {
           PDF_get_text_selection (data->x1-root_xs, data->y1-root_ys, data->w, data->h, 
                        data->curPDFpage, data->PDFScrollable, data);
       }
       break;
     }
     case PDF_SEL_MODE_PICT: { /* clip mode picture */
       if( data->w>0 && data->h>0) {
         pPixDatas=gdk_pixbuf_get_from_window (gtk_widget_get_window (data->PDFScrollable),
                             data->x1-root_xs, data->y1-root_ys,
                             data->w, data->h);
   
         /* save pixbuf to ClipBoard */
         GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
         gtk_clipboard_set_image   (clipboard, pPixDatas);
         g_object_unref(pPixDatas);
       }
       break;
     }
     case PDF_SEL_MODE_HIGH: {/* highlighting selection mode */
       if( data->w>0 && data->h>0) {
         PDF_set_highlight_selection(data->x1-root_xs, data->y1-root_ys, data->w, data->h, 
                        data->curPDFpage, data->doc, data->appWindow, data->PDFScrollable, data);
         update_PDF_state(data, PDF_MODIF);
       }
       break;
     }
     case PDF_SEL_MODE_NOTE: { /* simple text note */
       /* bug hunt : force to a widh and height of at least 24 pixels ! */
       if(data->w<24)
         data->w=24;
       if(data->h<24)
         data->h=24;

       PDF_set_text_annot_selection(data->x1-root_xs, data->y1-root_ys, data->w, data->h, 
                        data->curPDFpage, data->doc, data->appWindow, data->PDFScrollable, data);
       update_PDF_state(data, PDF_MODIF);
//TODO in 2037 ? PDF_set_free_text_annot_selection(data->x1-root_xs, data->y1-root_ys, data->w, data->h, 
  //                     data->curPDFpage, data->doc, data->appWindow, data->PDFScrollable, data);
       break;
     }
     default:;
  }/* end switch */ 

  return TRUE;
}

/*************************************
  code borrowed and adapted from stuff
  of Gnome team (Gtk2) : gnome-screenshot
  this event is associated to the 
   translucent selection window
source = https://github.com/GNOME/gnome-screenshot/blob/master/src/screenshot-area-selection.c
******************************************/

static gboolean
select_window_draw (GtkWidget *window, cairo_t *cr, gpointer unused)
{
  GtkStyleContext *style;

  style = gtk_widget_get_style_context (window);

  if(gtk_widget_get_app_paintable (window)) {
      cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
      cairo_set_source_rgba (cr, 0, 0, 0, 0);
      cairo_paint (cr);

      gtk_style_context_save (style);
      gtk_style_context_add_class (style, GTK_STYLE_CLASS_RUBBERBAND);

      gtk_render_background (style, cr, 0, 0,
                             gtk_widget_get_allocated_width (window),
                             gtk_widget_get_allocated_height (window));
      gtk_render_frame (style, cr, 0, 0,
                        gtk_widget_get_allocated_width (window),
                        gtk_widget_get_allocated_height (window));

      gtk_style_context_restore (style);
  }

  return TRUE;
}

/*************************************
  code borrowed and adapted from stuff
  of Gnome team (Gtk2) : gnome-screenshot
  this event is associated to the 
   translucent selection window
******************************************/

static GtkWidget *create_select_window (void)
{
  GtkWidget *window;
  GdkScreen *screen;
  GdkVisual *visual;

  screen = gdk_screen_get_default ();
  visual = gdk_screen_get_rgba_visual (screen);

  window = gtk_window_new (GTK_WINDOW_POPUP);
  if(gdk_screen_is_composited (screen) && visual) {
      gtk_widget_set_visual (window, visual);
      gtk_widget_set_app_paintable (window, TRUE);
  }

  g_signal_connect (window, "draw", G_CALLBACK (select_window_draw), NULL);

  gtk_window_move (GTK_WINDOW (window), -100, -100);
  gtk_window_resize (GTK_WINDOW (window), 10, 10);
  gtk_widget_show (window);
  
  return window;
}

/*****************************************
  button-press on PDF renderer
  inspired, but different, from
  Gnome-screenshot : it's called
  by a click event on PDF drawing area !

******************************************/
gboolean on_PDF_draw_button_press_callback(GtkWidget *widget, GdkEvent *event, APP_data *data)
{  
  if(!data->doc)
    return TRUE;

  if (gdk_event_get_event_type(event) == GDK_BUTTON_PRESS)  {
   if(event->button.button==3) {
       data->button_pressed=FALSE;/* yes, to avoid mistakes on drawings */
       GtkMenu *menu;
       GtkWidget *window1 = data->appWindow;
       /* we must free any existing maping ! */
       if(data->pdfAnnotMapping) {
          poppler_page_free_annot_mapping (data->pdfAnnotMapping);// supprimer pour permettre annulation couleurs
          data->pdfAnnotMapping=NULL;
       }
       /* we must get the annot mapping for current page */
       data->pdfAnnotMapping=PDF_get_annot_mapping(data);
       /* now the PopUp becomes smart since it's now if there is a mapping */
       gint root_xs, root_ys;
       /* get absolute screen coordinates */
       gdk_window_get_origin (gtk_widget_get_window (data->PDFScrollable), &root_xs,&root_ys);
       PopplerAnnot *current_annot=NULL;
       current_annot = PDF_find_annot_at_position((gint)event->button.x_root-root_xs, (gint)event->button.y_root-root_ys, data);
       data->current_annot=current_annot;
       menu = create_menu_PDF(window1, data);
       gtk_menu_popup(GTK_MENU (menu), NULL, NULL, NULL, NULL, 1, gtk_get_current_event_time());

   }
   else {
        data->x1=(gint)event->button.x_root;
        data->y1=(gint)event->button.y_root;
        data->w=0;
        data->h=0;
        data->button_pressed=TRUE;
        data->window = create_select_window();
   }
  }

  return TRUE;
}

/*******************************************
  motion notif to catch mouse's movements
  in order to set-tup a selection box
  code bprrowed and adapted from stuff
  of Gnome team (Gtk2) : gnome-screenshot
********************************************/
gboolean on_PDF_draw_motion_event_callback(GtkWidget *widget, GdkEvent  *event, APP_data *data)
{
  GtkWidget *window=data->window;
  if(!data->button_pressed || !data->doc)
    return TRUE;

  data->w = ABS (data->x1 - event->button.x_root);
  data->h = ABS (data->y1 - event->button.y_root);
  data->x1 = MIN (data->x1, event->button.x_root);
  data->y1 = MIN (data->y1, event->button.y_root);

  if(data->w <= 0 || data->h <= 0) {
      gtk_window_move (GTK_WINDOW (window), -100, -100);
      gtk_window_resize (GTK_WINDOW (window), 10, 10);
      return TRUE;
  }

  gtk_window_move (GTK_WINDOW (window), data->x1, data->y1);
  gtk_window_resize (GTK_WINDOW (window), data->w, data->h);

  /* We (ab)use app-paintable to indicate if we have an RGBA window */
  if(!gtk_widget_get_app_paintable (window)) {
      GdkWindow *gdkwindow = gtk_widget_get_window (window);
      /* Shape the window to make only the outline visible */
      if (data->w> 2 && data->h > 2) {
          cairo_region_t *region;
          cairo_rectangle_int_t region_rect = {
            0, 0,
            data->w, data->h
          };

          region = cairo_region_create_rectangle (&region_rect);
          region_rect.x++;
          region_rect.y++;
          region_rect.width -= 2;
          region_rect.height -= 2;
          cairo_region_subtract_rectangle (region, &region_rect);

          gdk_window_shape_combine_region (gdkwindow, region, 0, 0);

          cairo_region_destroy (region);
      }
      else
        gdk_window_shape_combine_region (gdkwindow, NULL, 0, 0);
  }

  return TRUE;
}
/************************************
 Button press on sketch stack
*************************************/
gboolean on_sketch_draw_button_press_callback(GtkWidget *widget, GdkEvent *event, APP_data *data)

{
  /* we must check if it's a RIGHT click code==3, left code ==1 middle code=2 */
  if (gdk_event_get_event_type(event) == GDK_BUTTON_PRESS)  {
    data->x1=(gint)event->button.x;/* common to Right click and left click in pencil mode - relative coordinates to current window, drawing area without decorations*/
    data->y1=(gint)event->button.y;
    data->x1_event_root =  event->button.x_root;/* for undo : absolute screen coordinates */
    data->y1_event_root =  event->button.y_root;
    if(event->button.button==3) {
       data->button_pressed=FALSE;/* yes, to avoid mistakes on drawings */
       GtkMenu *menu;
       GtkWidget *window1 = data->appWindow;
       menu = create_menu_sketch(window1, data);
       gtk_menu_popup(GTK_MENU (menu), NULL, NULL, NULL, NULL, 1, gtk_get_current_event_time());
       return TRUE;
    }
    if(data->fPencilTool) {
        data->button_pressed=TRUE;
    }
    else /* are we  in screenshot mode or annotation mode ? */
      {
           data->x1=(gint)event->button.x_root;
           data->y1=(gint)event->button.y_root;
           data->button_pressed=TRUE;
           data->window = create_select_window();
    }
    return TRUE;
  }/* button pressed */
  return FALSE;
}

/********************************************
 button release on sketch area

*******************************************/
gboolean on_sketch_draw_button_release_callback(GtkWidget *widget, GdkEvent *event, APP_data *data)

{
  GdkPixbuf *pPixDatas;
  gint root_xs, root_ys;
  gchar *tmpStr;
  GdkRGBA color; 
  GtkWidget *pBtnColor;
  GtkClipboard* clipboard;

  data->x1_event_root = MIN (data->x1_event_root, event->button.x_root);
  data->y1_event_root = MIN (data->y1_event_root, event->button.y_root);

  if(data->button_pressed) {    
    if(data->fPencilTool) {
      data->button_pressed=FALSE;
      return TRUE;
    }
    gtk_window_close (GTK_WINDOW(data->window));
    data->button_pressed=FALSE;
    /* annotation mode _ yes I reuse PDF flag to simplify code */
    if(data->clipboardMode==PDF_SEL_MODE_NOTE) {  
      pBtnColor=lookup_widget(GTK_WIDGET(data->appWindow), "color_button");
      /* we get the current RGBA color */
      gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER(pBtnColor), &color);
      tmpStr=dialog_add_text_annotation(data->appWindow, "", data);
      if(tmpStr!=NULL) {
        draw_text (event->button.x, event->button.y, data, tmpStr );
      }
      g_free(tmpStr);
      return TRUE;
    }/* endif sel note */

    /* we quit if it's a mistake click without any selection */
    if( data->w>0 && data->h>0) {
      /* get absolute screen coordinates , it's a request to copy selected area to clipboard */
      gdk_window_get_origin (gtk_widget_get_window (data->SketchScrollable), &root_xs, &root_ys);
      pPixDatas=gdk_pixbuf_get_from_window (gtk_widget_get_window (data->SketchScrollable),
                             data->x1-root_xs, data->y1-root_ys,
                             data->w, data->h);   
      /* save pixbuf to ClipBoard */
      clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
      gtk_clipboard_set_image   (clipboard, pPixDatas);
      g_object_unref(pPixDatas);
    }/* endif w h >0 */
  }/* endif button pressed */
  return TRUE;
}
/******************************************
  mouse move on Sketch area

******************************************/
gboolean on_sketch_draw_motion_event_callback(GtkWidget *widget, GdkEvent *event, APP_data *data)
{
 GtkWidget *window=data->window;
 GKeyFile *keyString;

 keyString = g_object_get_data(G_OBJECT(data->appWindow), "config");

 if(data->button_pressed) {
   data->x1_event_root = MIN (data->x1_event_root, event->button.x_root);
   data->y1_event_root = MIN (data->y1_event_root, event->button.y_root);

   if(data->fPencilTool) {
        if (data->button_pressed)  {
          draw_brush (event->button.x, event->button.y, data, g_key_file_get_double(keyString, "sketch", "pen-width", NULL));
          return TRUE;
        } 
   }
   else /* we are in screenshot or annotation mode */
    {
      data->w = ABS (data->x1 - event->button.x_root);
      data->h = ABS (data->y1 - event->button.y_root);
      data->x1 = MIN (data->x1, event->button.x_root);
      data->y1 = MIN (data->y1, event->button.y_root);
      data->x1_event_root = MIN (data->x1_event_root, event->button.x_root);
      data->y1_event_root = MIN (data->y1_event_root, event->button.y_root);

      if (data->w <= 0 || data->h <= 0) {
         gtk_window_move (GTK_WINDOW (window), -100, -100);
         gtk_window_resize (GTK_WINDOW (window), 10, 10);
        return TRUE;
      }
      gtk_window_move (GTK_WINDOW (window), data->x1, data->y1);
      gtk_window_resize (GTK_WINDOW (window), data->w, data->h);
      if (!gtk_widget_get_app_paintable (window)) {
         GdkWindow *gdkwindow = gtk_widget_get_window (window);
         /* Shape the window to make only the outline visible */
         if (data->w> 2 && data->h > 2) {
             cairo_region_t *region;
             cairo_rectangle_int_t region_rect = {
               0, 0,
               data->w, data->h
             };

             region = cairo_region_create_rectangle (&region_rect);
             region_rect.x++;
             region_rect.y++;
             region_rect.width -= 2;
             region_rect.height -= 2;
             cairo_region_subtract_rectangle (region, &region_rect);

             gdk_window_shape_combine_region (gdkwindow, region, 0, 0);
             cairo_region_destroy (region);
         }
         else
           gdk_window_shape_combine_region (gdkwindow, NULL, 0, 0);
      }/* if paintable */
    return TRUE;
    }/* screenshot mode */
  }/* endif button pressed */
  return FALSE;
}

/*****************************

  CB : clear current sketch
*****************************/
void
on_clearSketch_clicked  (GtkButton  *button, APP_data *data_app)
{
  GKeyFile *keyString;
  GdkRGBA color;
  GtkWidget *alertDlg;
  GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
  gint ret;

  alertDlg=gtk_message_dialog_new (data_app->appWindow,  flags,
                     GTK_MESSAGE_WARNING,
                     GTK_BUTTONS_OK_CANCEL,"%s",
                    _("Do you really want to erase your sketch ?\nThis operation can't be cancelled once done ! ")
                     );
              
  ret =  gtk_dialog_run(GTK_DIALOG(alertDlg));
  gtk_widget_destroy (GTK_WIDGET(alertDlg));
  if(ret==GTK_RESPONSE_OK) {
      keyString = g_object_get_data(G_OBJECT(data_app->appWindow), "config");
      color.red=g_key_file_get_double(keyString, "sketch", "paper.color.red", NULL);
      color.green=g_key_file_get_double(keyString, "sketch", "paper.color.green", NULL);
      color.blue=g_key_file_get_double(keyString, "sketch", "paper.color.blue", NULL);
      color.alpha=1;
      cairo_t *cr;
      cr = cairo_create (data_app->Sketchsurface);
      cairo_set_source_rgb(cr, color.red, color.green, color.blue);
      cairo_rectangle(cr, 0, 0, CROBAR_VIEW_MAX_WIDTH, CROBAR_VIEW_MAX_HEIGHT);
      cairo_fill(cr);
      cairo_destroy (cr);
      gtk_widget_queue_draw ( data_app->SketchDrawable);
      /* we must remove ALL sketch related operations from Undo stack ! */
      undo_free_all_sketch_ops(data_app);
  }
}

/*****************************
 prefs spin buttons callbacks

*****************************/
gdouble on_rewGapSpin_value_changed_event (GtkSpinButton *a_spinner, gpointer user_data) {
gdouble val;
   return gtk_spin_button_get_value (a_spinner);
}


/*************************************

callback : button "prefs" clicked

**************************************/

void
on_prefs_clicked  (GtkButton  *button, APP_data *data_app)
{
  GtkWidget *dialog;
  GdkRGBA text_color_bg, text_color_fg, sketch_color_bg;   
  GtkWidget *pBtnColor; 
  GKeyFile *keyString;
  gchar *newFont;
  gdouble rewValue, jumpValue, pen_width;

  dialog= create_prefs_dialog(data_app->appWindow, data_app);
  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK) {
    /* we get the current RGBA color */
    pBtnColor=lookup_widget(GTK_WIDGET(dialog), "color_button_editor_fg");
    gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER(pBtnColor), &text_color_fg);
    pBtnColor=lookup_widget(GTK_WIDGET(dialog), "color_button_editor_bg");
    gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER(pBtnColor), &text_color_bg);
    pBtnColor=lookup_widget(GTK_WIDGET(dialog), "color_button_sketch_bg");
    gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER(pBtnColor), &sketch_color_bg);
    /* we setup config file */
    keyString = g_object_get_data(G_OBJECT(data_app->appWindow), "config"); 

    /* global prefs */
    g_key_file_set_boolean(keyString, "application", "interval-save",  
                 gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (lookup_widget(GTK_WIDGET(dialog), "configAutoSave"))));
    g_key_file_set_boolean(keyString, "application", "autoreload-PDF",  
                 gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (lookup_widget(GTK_WIDGET(dialog), "configAutoReloadPDF"))));
    g_key_file_set_boolean(keyString, "application", "prompt-before-quit",  
                 gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (lookup_widget(GTK_WIDGET(dialog), "configPromptQuit"))));
    g_key_file_set_boolean(keyString, "application", "prompt-before-overwrite",  
                 gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (lookup_widget(GTK_WIDGET(dialog), "configPromptOverwrite"))));

    if(g_key_file_get_boolean(keyString, "application", "interval-save",  NULL) ) {
      gtk_widget_show( lookup_widget(GTK_WIDGET(data_app->appWindow),"image_task_due"));
    }
    else
      gtk_widget_hide( lookup_widget(GTK_WIDGET(data_app->appWindow),"image_task_due"));

    g_key_file_set_double(keyString, "editor", "text.color.red", text_color_fg.red);
    g_key_file_set_double(keyString, "editor", "text.color.green", text_color_fg.green);
    g_key_file_set_double(keyString, "editor", "text.color.blue", text_color_fg.blue);

    g_key_file_set_double(keyString, "editor", "paper.color.red", text_color_bg.red);
    g_key_file_set_double(keyString, "editor", "paper.color.green", text_color_bg.green);
    g_key_file_set_double(keyString, "editor", "paper.color.blue", text_color_bg.blue);

    g_key_file_set_double(keyString, "reference-document", "paper.color.red", text_color_bg.red);
    g_key_file_set_double(keyString, "reference-document", "paper.color.green", text_color_bg.green);
    g_key_file_set_double(keyString, "reference-document", "paper.color.blue", text_color_bg.blue);

    g_key_file_set_double(keyString, "sketch", "paper.color.red", sketch_color_bg.red);
    g_key_file_set_double(keyString, "sketch", "paper.color.green", sketch_color_bg.green);
    g_key_file_set_double(keyString, "sketch", "paper.color.blue", sketch_color_bg.blue);
    pen_width=gtk_spin_button_get_value(GTK_SPIN_BUTTON(lookup_widget(GTK_WIDGET(dialog), "pen_width_Spin")));
    g_key_file_set_double(keyString, "sketch", "pen-width", pen_width);
    
    rewValue=gtk_spin_button_get_value(GTK_SPIN_BUTTON(lookup_widget(GTK_WIDGET(dialog), "rewGapSpin")));
    g_key_file_set_double(keyString, "application", "audio-file-rewind-step", rewValue);
    jumpValue=gtk_spin_button_get_value(GTK_SPIN_BUTTON(lookup_widget(GTK_WIDGET(dialog), "jumpGapSpin")));
    g_key_file_set_double(keyString, "application", "audio-file-marks-step", jumpValue);
    g_key_file_set_boolean(keyString, "application","audio-auto-rewind",  
                 gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (lookup_widget(GTK_WIDGET(dialog), "configAutoRewindPlayer"))));
    if(g_key_file_get_boolean(keyString, "application", "audio-auto-rewind",  NULL) ) {
      gtk_widget_show( lookup_widget(GTK_WIDGET(data_app->appWindow),"image_audio_jump_to_start"));
    }
    else
      gtk_widget_hide( lookup_widget(GTK_WIDGET(data_app->appWindow),"image_audio_jump_to_start"));
    /* get the fonts */
    newFont = gtk_font_chooser_get_font(GTK_FONT_CHOOSER(lookup_widget(GTK_WIDGET(dialog), "font_button_sketch") ));
    if(newFont!=NULL) {
       g_key_file_set_string(keyString, "sketch", "font",newFont);
       g_free(newFont);
    }
    /* modify editor's textview default font */
    gchar *fntFamily=NULL;
    gint fntSize=12;
   // PangoContext* context = gtk_widget_get_pango_context  (data_app->view);
    PangoFontDescription *desc;// = pango_context_get_font_description(context);    
    newFont = gtk_font_chooser_get_font(GTK_FONT_CHOOSER(lookup_widget(GTK_WIDGET(dialog), "font_button_editor") ));
    g_key_file_set_string(keyString, "editor", "font",newFont);
    if (newFont != NULL) { 
        desc = pango_font_description_from_string (newFont);
        if (desc != NULL) {
          fntFamily= pango_font_description_get_family (desc);
          fntSize=pango_font_description_get_size(desc)/1000;
        }
        g_free(newFont);
    }
    /* same for colors, with CSS */
    
    text_color_fg.alpha=1;/* for future with alpha channel */
    text_color_bg.alpha=1;
  
    GtkCssProvider* css_provider = gtk_css_provider_new();
    gchar *css;
    css = g_strdup_printf("  #view  { font-family:%s; font-size:%dpx; color: #%.2x%.2x%.2x; background-color: #%.2x%.2x%.2x; }\n  #view:selected, #view:selected:focus { background-color: blue; color:white; }\n",
                 fntFamily,
                 fntSize,
                 (gint)( text_color_fg.red*255),(gint)( text_color_fg.green*255), (gint)(text_color_fg.blue*255),
                (gint)( text_color_bg.red*255),(gint)( text_color_bg.green*255), (gint)(text_color_bg.blue*255));
    if(desc)
      pango_font_description_free(desc);

    gtk_css_provider_load_from_data(css_provider,css,-1,NULL);
    GdkScreen* screen = gdk_screen_get_default();
    gtk_style_context_add_provider_for_screen (screen,GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_free(css);
  }
  gtk_widget_destroy(GTK_WIDGET(dialog));

}


/**************************
  set alignment button to
  a specific alignment
***************************/
void set_alignment_button(GtkWidget *win, gint alignment)
{
  switch(alignment) {
    case KW_ALIGNMENT_LEFT: {
      gtk_toggle_tool_button_set_active(GTK_TOOL_BUTTON(lookup_widget(GTK_WIDGET(win), "pRadioButtonLeft")) , TRUE);
      break;
    }
    case KW_ALIGNMENT_CENTER: {
      gtk_toggle_tool_button_set_active(GTK_TOOL_BUTTON(lookup_widget(GTK_WIDGET(win), "pRadioButtonCenter")) , TRUE);
      break;
    }
    case KW_ALIGNMENT_RIGHT: {
      gtk_toggle_tool_button_set_active(GTK_TOOL_BUTTON(lookup_widget(GTK_WIDGET(win), "pRadioButtonRight")) , TRUE);
      break;
    }
    case KW_ALIGNMENT_FILL: {
      gtk_toggle_tool_button_set_active(GTK_TOOL_BUTTON(lookup_widget(GTK_WIDGET(win), "pRadioButtonFill")) , TRUE);
      break;
    }
   default:{
     printf("* error on setting alignment button - set to Left *\n");
     gtk_toggle_tool_button_set_active(GTK_TOOL_BUTTON(lookup_widget(GTK_WIDGET(win), "pRadioButtonLeft")) , TRUE);
   }  
  }/* end switch */
}

/*************************************
  callback : get the copy to clipboard
  mode for PDF documents
***************************************/
void on_button_clip_mode_toggled (GtkButton *button, APP_data *user_data)
{
  GtkToolItem *tmpButton=NULL;

  tmpButton = GTK_TOOL_BUTTON(lookup_widget(GTK_WIDGET(button), "pRadioButtonTextSelect"));

  if (gtk_toggle_tool_button_get_active (GTK_TOOL_BUTTON(tmpButton)))  
  {
       user_data->clipboardMode=PDF_SEL_MODE_TEXT;
  }
  else 
  {
     tmpButton = GTK_TOOL_BUTTON(lookup_widget(GTK_WIDGET(button), "pRadioButtonPictureSelect"));

     if (gtk_toggle_tool_button_get_active (GTK_TOOL_BUTTON(tmpButton)))  
       {
          user_data->clipboardMode=PDF_SEL_MODE_PICT;
       }
     else 
       {
         tmpButton = GTK_TOOL_BUTTON(lookup_widget(GTK_WIDGET(button), "pRadioButtonHiglightSelect"));
         if (gtk_toggle_tool_button_get_active (GTK_TOOL_BUTTON(tmpButton)))  
           {
            user_data->clipboardMode=PDF_SEL_MODE_HIGH;
           }
         else 
           {
             user_data->clipboardMode=PDF_SEL_MODE_NOTE;
           }
        }
  }/* 1er choix */
}

/*********************************************
  callback : generic for radio toggle buttons
  for alignment : left, center and so on
 READY FOR LOCAL just remove globals
***********************************************/
void
on_button_alignment_toggled (GtkButton *button, APP_data *data)
{
  GtkToolItem *tmpButton=NULL;

  tmpButton = GTK_TOOL_BUTTON(lookup_widget(GTK_WIDGET(button), "pRadioButtonLeft"));
  if (gtk_toggle_tool_button_get_active (GTK_TOOL_BUTTON(tmpButton)))  {
    //   printf("bingo left\n");
       on_left_justify_clicked(data);
       kw_paragraph_alignment = KW_ALIGNMENT_LEFT;
       data->kw_paragraph_alignment = KW_ALIGNMENT_LEFT;
       return;
   }
  tmpButton = GTK_TOOL_BUTTON(lookup_widget(GTK_WIDGET(button), "pRadioButtonCenter"));
  if (gtk_toggle_tool_button_get_active (GTK_TOOL_BUTTON(tmpButton)))  {
     //  printf("bingo Center\n");
       on_center_justify_clicked(data);
       kw_paragraph_alignment = KW_ALIGNMENT_CENTER;
       data->kw_paragraph_alignment = KW_ALIGNMENT_CENTER;
       return;
   }
  tmpButton = GTK_TOOL_BUTTON(lookup_widget(GTK_WIDGET(button), "pRadioButtonRight"));
  if (gtk_toggle_tool_button_get_active (GTK_TOOL_BUTTON(tmpButton)))  {
      // printf("bingo Right\n");
       on_right_justify_clicked(data);
       kw_paragraph_alignment = KW_ALIGNMENT_RIGHT;
       data->kw_paragraph_alignment = KW_ALIGNMENT_RIGHT;
       return;
   }
  tmpButton = GTK_TOOL_BUTTON(lookup_widget(GTK_WIDGET(button), "pRadioButtonFill"));
  if (gtk_toggle_tool_button_get_active (GTK_TOOL_BUTTON(tmpButton)))  {
      // printf("bingo Fill\n");
       on_fill_justify_clicked(data);
       kw_paragraph_alignment = KW_ALIGNMENT_FILL;
       data->kw_paragraph_alignment = KW_ALIGNMENT_FILL;
   }
}

/**********************************
  callback : button bold clicked 
***********************************/
void
on_bold_clicked  (GtkButton  *button, APP_data *data)
{
  GtkTextBuffer *buffer;
  GtkTextIter start, end, iter;
  gboolean fExistSelection, fIsBold = FALSE;
  GtkTextTag *tag;
  GtkTextTagTable *tagTable1;  


  if(!fUserClickedButton)
     return;
  fIsBold = gtk_toggle_tool_button_get_active (GTK_TOOL_BUTTON(button));
  buffer = data->buffer;
  tagTable1 = gtk_text_buffer_get_tag_table(buffer);
  tag = gtk_text_tag_table_lookup(tagTable1, "bold");

  if(!gtk_text_buffer_get_has_selection (buffer)) {
     fBold=!fBold;
     fUserClickedButton = FALSE;
     gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
     iRowMem = gtk_text_iter_get_line(&iter);
     iPendingCol  = gtk_text_iter_get_line_offset(&iter);
     iColMem = iPendingCol;
     return;
  }
  fExistSelection = gtk_text_buffer_get_selection_bounds (buffer,
                                      &start,
                                      &end);
  if(fExistSelection) {
    /* now we apply the tag */
     data->undo.serialized_buffer=NULL;
     if(fIsBold) {
        gtk_text_buffer_apply_tag(buffer, tag, &start, &end);
        data->undo.start_sel=start;
        data->undo.end_sel=end;
        undo_push(data->currentStack, OP_SET_BOLD, data);
     }
     else {
       gtk_text_buffer_remove_tag(buffer, tag, &start, &end);
       data->undo.start_sel=start;
       data->undo.end_sel=end;
       undo_push(data->currentStack, OP_UNSET_BOLD, data);
     } 
  }  
}

/*
  callback : button italic clicked 

*/
void
on_italic_clicked     (GtkButton  *button, APP_data *data)
{
  GtkTextBuffer *buffer;
  GtkTextIter start, end, iter;
  gboolean fExistSelection, fIsItalic = FALSE;
  GtkTextTag *tag;
  GtkTextTagTable *tagTable1;

  if(!fUserClickedButton)
     return;

  fIsItalic = gtk_toggle_tool_button_get_active (GTK_TOOL_BUTTON(button));
  buffer = data->buffer;
  tagTable1 = gtk_text_buffer_get_tag_table(buffer);
  tag = gtk_text_tag_table_lookup(tagTable1, "italic");

  if(!gtk_text_buffer_get_has_selection (buffer)) {
     fItalic=!fItalic;
     fUserClickedButton = FALSE;
     gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
     iRowMem = gtk_text_iter_get_line(&iter);
     iPendingCol  = gtk_text_iter_get_line_offset(&iter);
     iColMem = iPendingCol;
     return;
  }
  fExistSelection = gtk_text_buffer_get_selection_bounds (buffer,
                                      &start,
                                      &end);
  if(fExistSelection) {
    /* now we apply the tag */
     data->undo.serialized_buffer=NULL;
     if(fIsItalic){
        gtk_text_buffer_apply_tag(buffer, tag, &start, &end);
        data->undo.start_sel=start;
        data->undo.end_sel=end;
        undo_push(data->currentStack, OP_SET_ITALIC, data);
     }
     else {
       gtk_text_buffer_remove_tag(buffer, tag, &start, &end);
       data->undo.start_sel=start;
       data->undo.end_sel=end;
       undo_push(data->currentStack, OP_UNSET_ITALIC, data);
     } 
  }  
}

/*
  callback : button underline clicked 

*/
void
on_underline_clicked  (GtkButton *button, APP_data *data)
{
  GtkTextBuffer *buffer;
  GtkTextIter start, end, iter;
  gboolean fExistSelection, fIsUnderline = FALSE;
  GtkTextTag *tag;
  GtkTextTagTable *tagTable1;

  if(!fUserClickedButton)
     return;

  fIsUnderline = gtk_toggle_tool_button_get_active (GTK_TOOL_BUTTON(button));
  buffer = data->buffer;
  tagTable1 = gtk_text_buffer_get_tag_table(buffer);
  tag = gtk_text_tag_table_lookup(tagTable1, "underline");

  if(!gtk_text_buffer_get_has_selection (buffer)) {
     /* it's a demand to switch to insert/append mode with a specific format */
     fUnderline=!fUnderline;
     gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
     iRowMem = gtk_text_iter_get_line(&iter);
     iPendingCol  = gtk_text_iter_get_line_offset(&iter);
     iColMem = iPendingCol;
     fUserClickedButton = FALSE;
     return;
  }
  fExistSelection = gtk_text_buffer_get_selection_bounds (buffer, &start, &end);
  if(fExistSelection) {
    /* now we apply the tag */
     data->undo.serialized_buffer=NULL;
     if(fIsUnderline){
        gtk_text_buffer_apply_tag(buffer, tag, &start, &end);
        data->undo.start_sel=start;
        data->undo.end_sel=end;
        undo_push(data->currentStack, OP_SET_UNDERLINE, data);
     }
     else {
       gtk_text_buffer_remove_tag(buffer, tag, &start, &end);
        data->undo.start_sel=start;
        data->undo.end_sel=end;
        undo_push(data->currentStack, OP_UNSET_UNDERLINE, data);
     } 
  }  
}


/*
  callback : superscript shortcut called
  <CTRL>+<^>

*/
void
on_superscript_clicked     (GtkButton  *button, APP_data  *data)
{
  GtkTextBuffer *buffer;
  GtkTextIter start, end, iter;
  gboolean fExistSelection,fIsSuperscript = FALSE;
  GtkTextTag *tag;
  GtkTextTagTable *tagTable1;

  if(!fUserClickedButton) 
     return;
  
  fIsSuperscript = gtk_toggle_tool_button_get_active (GTK_TOOL_BUTTON(button));
  buffer = data->buffer;
  tagTable1 = gtk_text_buffer_get_tag_table(buffer);
  tag = gtk_text_tag_table_lookup(tagTable1, "superscript");

  if(!gtk_text_buffer_get_has_selection (buffer)) {
     /* it's a demand to switch to insert/append mode with a specific format */
     fSuperscript=!fSuperscript;
     gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
     iRowMem = gtk_text_iter_get_line(&iter);
     iPendingCol  = gtk_text_iter_get_line_offset(&iter);
     iColMem = iPendingCol;
     fUserClickedButton = FALSE;
     return;
  }
  fExistSelection = gtk_text_buffer_get_selection_bounds (buffer, &start, &end);

  if(fExistSelection) {
     /* we must get a tag for the current selection */
   //  fSuperscript=!fSuperscript;
  //fIsSuperscript = fSuperscript;
    /* now we apply the tag */
     data->undo.serialized_buffer=NULL;
     if(fIsSuperscript){
        //gtk_text_buffer_remove_tag_by_name (buffer, "superscript", &start, &end);
        gtk_text_buffer_apply_tag(buffer, tag, &start, &end);
        data->undo.start_sel=start;
        data->undo.end_sel=end;
        undo_push(data->currentStack, OP_SET_SUPER, data);
     }
     else {
       gtk_text_buffer_remove_tag(buffer, tag, &start, &end);
       data->undo.start_sel=start;
       data->undo.end_sel=end;
       undo_push(data->currentStack, OP_UNSET_SUPER, data);
     } 
  }  
}

/*
  callback : subscript shortcut called
  <CTRL>+<*>

*/
void
on_subscript_clicked (GtkButton *button, APP_data *data)
{
  GtkTextBuffer *buffer;
  GtkTextIter start, end, iter;
  gboolean fExistSelection;
  GtkTextTag *tag;
  GtkTextTagTable *tagTable1;
  gboolean fIsSubcript = FALSE;

  if(!fUserClickedButton)
     return;

  fIsSubcript = gtk_toggle_tool_button_get_active (GTK_TOOL_BUTTON(button));
  buffer = data->buffer;
  tagTable1 = gtk_text_buffer_get_tag_table(buffer);
  tag = gtk_text_tag_table_lookup(tagTable1, "subscript");

  if(!gtk_text_buffer_get_has_selection (buffer)) {
     fSubscript=!fSubscript;

     /* it's a demand to switch to insert/append mode with a specific format */

     gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
     iRowMem = gtk_text_iter_get_line(&iter);
     iPendingCol  = gtk_text_iter_get_line_offset(&iter);
     iColMem = iPendingCol;
     fUserClickedButton = FALSE;
     return;
  }
  fExistSelection = gtk_text_buffer_get_selection_bounds (buffer, &start, &end);
  if(fExistSelection) {
    /* we must get a tag for the current selection */
     //fSubscript=!fSubscript;
     //fIsSubcript = fSubscript;
    /* now we apply the tag */
     data->undo.serialized_buffer=NULL;
     if(fIsSubcript){
        //gtk_text_buffer_remove_tag_by_name (buffer, "subscript", &start, &end);
        gtk_text_buffer_apply_tag(buffer, tag, &start, &end);
        data->undo.start_sel=start;
        data->undo.end_sel=end;
        undo_push(data->currentStack, OP_SET_SUB, data);
     }
     else {
        gtk_text_buffer_remove_tag(buffer, tag, &start, &end);
        data->undo.start_sel=start;
        data->undo.end_sel=end;
        undo_push(data->currentStack, OP_UNSET_SUB, data);
     } 
  }  
}

/*
  callback : highlight shortcut called
  <CTRL>+<h>

*/
void
on_highlight_clicked (GtkButton *button, APP_data *data)
{
  GtkTextBuffer *buffer;
  GtkTextIter start, end, iter;
  gboolean fExistSelection;
  GtkTextTag *tag;
  GtkTextTagTable *tagTable1;
  gboolean fIsHighlight = FALSE;

  if(!fUserClickedButton)
     return;

  fIsHighlight = gtk_toggle_tool_button_get_active (GTK_TOOL_BUTTON(button));
  buffer = data->buffer;
  tagTable1 = gtk_text_buffer_get_tag_table(buffer);
  tag = gtk_text_tag_table_lookup(tagTable1, "highlight");

  if(!gtk_text_buffer_get_has_selection (buffer)) {
     fHighlight=!fHighlight;
     /* it's a demand to switch to insert/append mode with a specific format */

     gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
     iRowMem = gtk_text_iter_get_line(&iter);
     iPendingCol  = gtk_text_iter_get_line_offset(&iter);
     iColMem = iPendingCol;
     fUserClickedButton = FALSE;
     return;
  }
  fExistSelection = gtk_text_buffer_get_selection_bounds (buffer, &start, &end);
  if(fExistSelection) {
    /* we must get a tag for the current selection */
    // fHighlight=!fHighlight;
    // fIsHighlight = fHighlight;
    /* now we apply the tag */
     data->undo.serialized_buffer=NULL;
     if(fIsHighlight){
        //gtk_text_buffer_remove_tag_by_name (buffer, "highlight", &start, &end);
        gtk_text_buffer_apply_tag(buffer, tag, &start, &end);
        data->undo.start_sel=start;
        data->undo.end_sel=end;
        undo_push(data->currentStack, OP_SET_HIGH, data);
     }
     else {
        gtk_text_buffer_remove_tag(buffer, tag, &start, &end);
        data->undo.start_sel=start;
        data->undo.end_sel=end;
        undo_push(data->currentStack, OP_UNSET_HIGH, data);
     } 
   /* we remove any selected area */
//gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
  // gtk_text_buffer_select_range (buffer,&iter, &iter);

  }  
}

/*
  callback : Strikethrough shortcut called
  <CTRL>+<k>

*/
void 
on_strikethrough_clicked (GtkButton *button, APP_data *data)
{
  GtkTextBuffer *buffer;
  GtkTextIter start, end, iter;
  gboolean fExistSelection;
  GtkTextTag *tag;
  GtkTextTagTable *tagTable1;
  gboolean fIsStrikethrough = FALSE;

  if(!fUserClickedButton)
     return;

  fIsStrikethrough = gtk_toggle_tool_button_get_active (GTK_TOOL_BUTTON(button));
  buffer = data->buffer;
  tagTable1 = gtk_text_buffer_get_tag_table(buffer);
  tag = gtk_text_tag_table_lookup(tagTable1, "strikethrough");

  if(!gtk_text_buffer_get_has_selection (buffer)) {
     fStrikethrough=!fStrikethrough;

     /* it's a demand to switch to insert/append mode with a specific format */

     gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
     iRowMem = gtk_text_iter_get_line(&iter);
     iPendingCol  = gtk_text_iter_get_line_offset(&iter);
     iColMem = iPendingCol;
     fUserClickedButton = FALSE;
     return;
  }
  fExistSelection = gtk_text_buffer_get_selection_bounds (buffer, &start, &end);
  if(fExistSelection) {
    /* we must get a tag for the current selection */
     //fStrikethrough=!fStrikethrough;
     //fIsStrikethrough = fStrikethrough;
     data->undo.serialized_buffer=NULL;
    /* now we apply the tag */
     if(fIsStrikethrough){
        //gtk_text_buffer_remove_tag_by_name (buffer, "strikethrough", &start, &end);
       gtk_text_buffer_apply_tag(buffer, tag, &start, &end);
       data->undo.start_sel=start;
       data->undo.end_sel=end;
       undo_push(data->currentStack, OP_SET_STRIKE, data);
     }
     else {
       gtk_text_buffer_remove_tag(buffer, tag, &start, &end);
       data->undo.start_sel=start;
       data->undo.end_sel=end;
       undo_push(data->currentStack, OP_UNSET_STRIKE, data);
     } 
  }  
}



/*
  callback : quotation shortcut called
  <CTRL>+<">

*/
void on_quotation_clicked   (GtkButton  *button,  APP_data *data)
{
  GtkTextBuffer *buffer;
  GtkTextIter start, end, iter;
  gboolean fExistSelection;
  GtkTextTag *tag;
  GtkTextTagTable *tagTable1;
  gboolean fIsQuotation = FALSE;

  if(!fUserClickedButton)
     return;

  fIsQuotation = gtk_toggle_tool_button_get_active (GTK_TOOL_BUTTON(button));
  buffer = data->buffer;
  tagTable1 = gtk_text_buffer_get_tag_table(buffer);
  tag = gtk_text_tag_table_lookup(tagTable1, "quotation");

  if(!gtk_text_buffer_get_has_selection (buffer)) {
     fQuotation=!fQuotation;

     /* it's a request to switch to insert/append mode with a specific format */

     gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
     iRowMem = gtk_text_iter_get_line(&iter);
     iPendingCol  = gtk_text_iter_get_line_offset(&iter);
     iColMem = iPendingCol;
     fUserClickedButton = FALSE;
     return;
  }
  fExistSelection = gtk_text_buffer_get_selection_bounds (buffer,
                                      &start,
                                      &end);
  if(fExistSelection) {
    /* we must get a tag for the current selection */
     //fQuotation=!fQuotation;
     //fIsQuotation = fQuotation;
    /* now we apply the tag */
     data->undo.serialized_buffer=NULL;
     if(fIsQuotation){
        //gtk_text_buffer_remove_tag_by_name (buffer, "quotation", &start, &end);
       gtk_text_buffer_apply_tag(buffer, tag, &start, &end);
       data->undo.start_sel=start;
       data->undo.end_sel=end;
       undo_push(data->currentStack, OP_SET_QUOTE, data);
     }
     else {
       gtk_text_buffer_remove_tag(buffer, tag, &start, &end);
       data->undo.start_sel=start;
       data->undo.end_sel=end;
       undo_push(data->currentStack, OP_UNSET_QUOTE, data);
     } 
  }  
}

/*
  callback : button left justify clicked 

*/
void
on_left_justify_clicked (APP_data *data)
{
  GtkTextBuffer *buffer;
  GtkTextIter start, end, iter;
  GtkTextTag *tag;
  GtkTextTagTable *tagTable1;
  gint row, col, total;
  gboolean fExistSelection;
  
  if(!fUserClickedButton)
     return;
  buffer = data->buffer;

  tagTable1 = gtk_text_buffer_get_tag_table(buffer);
  tag = gtk_text_tag_table_lookup(tagTable1, "left");
  /* there is a selection ? */
  if(gtk_text_buffer_get_has_selection (buffer)) {
     fExistSelection = gtk_text_buffer_get_selection_bounds (buffer, &iter, &end);
     if(!fExistSelection) {
        printf("* Error : NO bounds I exit *\n");     
        return;
     }
     /* we rewind to the start of the first selected line */
     row = gtk_text_iter_get_line(&iter);
     gtk_text_buffer_get_iter_at_line (buffer,&start,row);
  }
  else {printf("* Warning : I compute a default selection *\n");
     gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
     row = gtk_text_iter_get_line(&iter);
     col = gtk_text_iter_get_line_offset(&iter);
     total = gtk_text_buffer_get_line_count(buffer);
     misc_append_empty_paragraph(buffer, row, total);
     gtk_text_buffer_get_iter_at_line (buffer,&start,row);
     if(total>row) {
       gtk_text_buffer_get_iter_at_line (buffer,&end,row+1);
     }
     else {
        gtk_text_buffer_get_end_iter (buffer, &end);
     }
  }/* elseif NO selection */
  data->undo.serialized_buffer=NULL;
  data->undo.prevQuadding=misc_get_paragraph_quadding(buffer, start);
  data->undo.start_sel=start;
  data->undo.end_sel=end;
  undo_push(data->currentStack, OP_ALIGN_LEFT, data);
  /* clear all unwanted tags */
  misc_remove_alignment_tags(buffer,start,end);
  /* now we apply the tag */
  gtk_text_buffer_apply_tag(buffer, tag, &start, &end);  
}

/*
  callback : button center justify clicked 

*/
void
on_center_justify_clicked (APP_data *data)
{
  GtkTextBuffer *buffer;
  GtkTextIter start, end, iter;
  GtkTextTag *tag;
  GtkTextTagTable *tagTable1;
  gint row, col, total;
  gboolean fExistSelection;
  
  if(!fUserClickedButton)
     return;
  buffer = data->buffer;
  tagTable1 = gtk_text_buffer_get_tag_table(buffer);
  tag = gtk_text_tag_table_lookup(tagTable1, "center");
  /* there is a selection ? */
  if(gtk_text_buffer_get_has_selection (buffer)) {
     fExistSelection = gtk_text_buffer_get_selection_bounds (buffer,
                                      &iter,
                                      &end);
     if(!fExistSelection) {
        printf("NOT bounds I exit \n");     
        return;
     }
     /* we rewind to the start of the first selected line */
     row = gtk_text_iter_get_line(&iter);
     gtk_text_buffer_get_iter_at_line (buffer,&start,row);
  }
  else {printf("I compute a default selection \n");
     gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
     row = gtk_text_iter_get_line(&iter);
     col = gtk_text_iter_get_line_offset(&iter);
     /* we check and perhaps append an empty paragraph */
     total = gtk_text_buffer_get_line_count(buffer);
     misc_append_empty_paragraph(buffer, row, total);
     gtk_text_buffer_get_iter_at_line (buffer,&start,row);
     if(total>row) {
       gtk_text_buffer_get_iter_at_line (buffer,&end,row+1);
     }
     else {
        gtk_text_buffer_get_end_iter (buffer, &end);
     }
  }/* elseif NO selection */
  data->undo.serialized_buffer=NULL;
  data->undo.prevQuadding=misc_get_paragraph_quadding(buffer, start);
  data->undo.start_sel=start;
  data->undo.end_sel=end;
  undo_push(data->currentStack, OP_ALIGN_CENTER, data);
  /* clear all unwanted tags */
  misc_remove_alignment_tags(buffer,start,end);
  /* now we apply the tag */
  gtk_text_buffer_apply_tag(buffer, tag, &start, &end);  
}

/*
  callback : button right justify clicked 

*/
void
on_right_justify_clicked (APP_data *data)
{
  GtkTextBuffer *buffer;
  GtkTextIter start, end, iter;
  GtkTextTag *tag;
  GtkTextTagTable *tagTable1;
  gint row, col, total;
  gboolean fExistSelection;
  
  if(!fUserClickedButton)
     return;
  buffer = data->buffer;
  tagTable1 = gtk_text_buffer_get_tag_table(buffer);
  tag = gtk_text_tag_table_lookup(tagTable1, "right");
  /* there is a selection ? */
  if(gtk_text_buffer_get_has_selection (buffer)) {
     fExistSelection = gtk_text_buffer_get_selection_bounds (buffer,
                                      &iter,
                                      &end);
     if(!fExistSelection) {
        printf("NOT bounds I exit \n");     
        return;
     }
     /* we rewind to the start of the first selected line */
     row = gtk_text_iter_get_line(&iter);
     gtk_text_buffer_get_iter_at_line (buffer,&start,row);
  }
  else {printf("I compute a default selection \n");
     gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
     row = gtk_text_iter_get_line(&iter);
     col = gtk_text_iter_get_line_offset(&iter);
     total = gtk_text_buffer_get_line_count(buffer);
     misc_append_empty_paragraph(buffer, row, total);
     gtk_text_buffer_get_iter_at_line (buffer,&start,row);
     if(total>row) {
       gtk_text_buffer_get_iter_at_line (buffer,&end,row+1);
     }
     else {
        gtk_text_buffer_get_end_iter (buffer, &end);
     }
  }/* elseif NO selection */
  data->undo.serialized_buffer=NULL;
  data->undo.prevQuadding=misc_get_paragraph_quadding(buffer, start);
  data->undo.start_sel=start;
  data->undo.end_sel=end;
  undo_push(data->currentStack, OP_ALIGN_RIGHT, data);
  /* clear all unwanted tags */
  misc_remove_alignment_tags(buffer,start,end);
  /* now we apply the tag */
  gtk_text_buffer_apply_tag(buffer, tag, &start, &end); 
}

/*
  callback : button fill justify clicked 

*/
void
on_fill_justify_clicked (APP_data *data)
{
  GtkTextBuffer *buffer;
  GtkTextIter start, end, iter;
  GtkTextTag *tag;
  GtkTextTagTable *tagTable1;
  gint row, col, total;
  gboolean fExistSelection;
  
  if(!fUserClickedButton)
     return;
  buffer = data->buffer;
  tagTable1 = gtk_text_buffer_get_tag_table(buffer);
  tag = gtk_text_tag_table_lookup(tagTable1, "fill");
  /* there is a selection ? */
  if(gtk_text_buffer_get_has_selection (buffer)) {
     fExistSelection = gtk_text_buffer_get_selection_bounds (buffer, &iter, &end);
     if(!fExistSelection) {
        printf("NOT bounds I exit \n");     
        return;
     }
     /* we rewind to the start of the first selected line */
     row = gtk_text_iter_get_line(&iter);
     gtk_text_buffer_get_iter_at_line (buffer,&start,row);
  }
  else {printf("I compute a default selection \n");
     gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
     row = gtk_text_iter_get_line(&iter);
     col = gtk_text_iter_get_line_offset(&iter);
     total = gtk_text_buffer_get_line_count(buffer);
     misc_append_empty_paragraph(buffer, row, total);
     gtk_text_buffer_get_iter_at_line (buffer,&start,row);
     if(total>row) {
       gtk_text_buffer_get_iter_at_line (buffer,&end,row+1);
     }
     else {
        gtk_text_buffer_get_end_iter (buffer, &end);
     }
  }/* elseif NO selection */
  data->undo.serialized_buffer=NULL;
  data->undo.prevQuadding=misc_get_paragraph_quadding(buffer, start);
  data->undo.start_sel=start;
  data->undo.end_sel=end;
  undo_push(data->currentStack, OP_ALIGN_FILL, data);
  /* clear all unwanted tags */
  misc_remove_alignment_tags(buffer,start,end);
  /* now we apply the tag */
  gtk_text_buffer_apply_tag(buffer, tag, &start, &end); 
}

/*
  callback : button clear format clicked 

*/
void
on_clear_format_clicked (GtkButton *button, APP_data *data)
{
  GtkTextBuffer *buffer;
  GtkTextIter start, end;
  gboolean fExistSelection;
  GtkTextTagTable *tagTable1;

  buffer = data->buffer;
  tagTable1 = gtk_text_buffer_get_tag_table(buffer);
  
  if(!gtk_text_buffer_get_has_selection (buffer)) {
     printf("no selection \n");
     return;
  }
  fExistSelection = gtk_text_buffer_get_selection_bounds (buffer, &start, &end);
  if(fExistSelection) {
    /* now we apply the tag */
     gtk_text_buffer_remove_all_tags (buffer,  &start, &end);

  }  
}

/**********************************
  callback : button undo clicked 
  the number of operations which
  can cancelled is the 
 MAX_UNDO_OPERATION
 define in undo.h
***********************************/
void on_undo_clicked (GtkButton *button, APP_data *data)
{ 
  if(data->undoList!=NULL) {
    undo_pop(data->currentStack, data);
  }
}

/**************************************************
  callback find PREVIOUS 
  works only if thereis already a found location
  PLEASE NOTE : for PDF document, previous go backward
  to the first page with hits.
*****************************************************/
void
on_find_prev_clicked (GtkButton *button, APP_data *data)
{
  GtkTextBuffer *buffer;
  GtkWidget *search_entry, *tView;
  gchar *tmpStr;
  GtkTextIter iter, start;
  gint ret;

  GtkWidget *window1 = data->appWindow;
  buffer = data->buffer;
  search_entry = lookup_widget(GTK_WIDGET(window1), "search_entry");
  tView=lookup_widget(GTK_WIDGET(window1), "view");
  if(search_entry) {
    tmpStr=gtk_entry_get_text (GTK_ENTRY(search_entry));
    if(tmpStr) {
      if(data->currentStack==CURRENT_STACK_EDITOR) {
        /* we get a pointer on current position */
        gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
        gtk_text_buffer_get_start_iter (buffer, &start);
        find_previous(tView, buffer, tmpStr);
      }
      else {
       update_statusbarPDF(data);
       ret=search_goto_prev_PDF_page(data->curPDFpage, data);
       if(ret<data->curPDFpage) {
          data->curPDFpage=ret;
          PDF_display_page (window1,data->curPDFpage, data->doc, data);
          search_draw_selection_current_page(data->curPDFpage, data, data->surface);
       }
      }
    }
  }
}


/**************************************************
  callback find NEXT 
  works only if thereis already a found location
 PLEASE NOTE : for PDF document, next go forward
  to the first page with hits.
**************************************************/
void
on_find_next_clicked (GtkButton *button, APP_data *data)
{
  GtkTextBuffer *buffer;
  GtkWidget *search_entry, *tView;
  gchar *tmpStr;
  GtkTextIter iter, start;
  gint ret;

  GtkWidget *window1 = data->appWindow;
  buffer = data->buffer;

  search_entry = lookup_widget(GTK_WIDGET(window1), "search_entry");
  tView=lookup_widget(GTK_WIDGET(window1), "view");
  if(search_entry) {
    tmpStr=gtk_entry_get_text (GTK_ENTRY(search_entry));
    if(tmpStr) {
      if(data->currentStack==CURRENT_STACK_EDITOR) {
        /* we get a pointer on current position */
        gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
        gtk_text_buffer_get_start_iter (buffer, &start);
        find_next(tView, buffer, tmpStr);
      }
      else {
        update_statusbarPDF(data);
        ret=search_goto_next_PDF_page(data->curPDFpage, data);
        if(ret>data->curPDFpage) {
           data->curPDFpage=ret;
           PDF_display_page (window1,data->curPDFpage, data->doc, data);
           search_draw_selection_current_page(data->curPDFpage, data, data->surface);
           
        }
      }
    }
  }
}

/******************************
 change PDF page directly
******************************/
void
on_page_entry_changed  (GtkEntry *entry, APP_data *data)
{
  /* we must check if it's a compatible numeric value */
  if(!data->doc) {
    return;
  }
  if(strlen(gtk_entry_get_text(entry))==0) {
     //gtk_entry_set_text(entry, g_strdup_printf("%d", data->curPDFpage+1));
     return;
  }
  gint i=atoi(gtk_entry_get_text(entry));
//  printf("pdf page %d\n", i);
  if(i<1 || i>data->totalPDFpages) {
     gtk_entry_set_text(entry, g_strdup_printf("%d", data->curPDFpage+1));
     return;
  }
  PDF_goto(data->appWindow, data, i-1);
}
/************************************************
 callback find : move selection to the FIRST hit
 PLEASE NOTE : for PDF document, jumps
  to the first page with hits.
*************************************************/
void
on_find_changed (GtkSearchEntry *entry, APP_data *data)
{
  GtkTextBuffer *buffer;
  GtkWidget *tView, *bPrev, *bNext, *bReplace, *pReplaceEntry, *hits;
  gchar *tmpStr;
  GtkTextIter iter, start;
  gint i=0, j, count=0;
  gboolean sensitive=TRUE;
  
 
  GtkWidget *window1 = data->appWindow;
  buffer = data->buffer;

  tView=lookup_widget(GTK_WIDGET(window1), "view");
  hits = lookup_widget(GTK_WIDGET(window1), "labelHits");
  bPrev= lookup_widget(GTK_WIDGET(window1), "buttonNextOccurrence");
  bNext= lookup_widget(GTK_WIDGET(window1), "buttonPrevOccurrence");
  bReplace= lookup_widget(GTK_WIDGET(window1), "buttonReplace");
  pReplaceEntry= lookup_widget(GTK_WIDGET(window1), "replace_entry");

  if(entry) {
    tmpStr=gtk_entry_get_text (GTK_ENTRY(entry));
    if(tmpStr) {
        if(strlen(tmpStr)>0) {
           if(data->currentStack==CURRENT_STACK_EDITOR) { 
               i= search_count_matches(buffer, tmpStr);
               fdont_care=TRUE;
           }
           else {
             if(data->currentStack==CURRENT_STACK_PDF) {
              /* we must reset the GList */
                search_free_PDF_search_datas(data);
                if(data->doc) {
                  count=0;
                  data->pdfSearch=NULL;
                  i=search_hits_inside_PDF_document(data, tmpStr);
                  /* we draw rectangles */
                  PDF_display_page (window1,data->curPDFpage, data->doc, data);                        
                  search_draw_selection_current_page(data->curPDFpage, data, data->surface);                   
                  update_statusbarPDF(data);
                  gtk_widget_grab_focus(GTK_WIDGET(data->PDFScrollable));
                  /* we check results */
                 // GList *l=g_list_first(data->pdfSearch);
                 /* printf("check Glist !\n");
                  for(l;l!=NULL;l=l->next) {
                     PDF_search_results *results;
                     results=(PDF_search_results *)l->data;
                     printf("page=%d nb hits par page=%d \n", 
                            results->page, results->nb_hits_at_page);
                  }*/                            
              }/* if doc */
             }/* if currentstack==PDF */
           }
        }/* strlen */
        /* we activate or not the [next] and [previous] buttons */
        if(i<2) {
          sensitive=FALSE;
        }
        gtk_widget_set_sensitive(GTK_WIDGET(bNext),sensitive);
        gtk_widget_set_sensitive(GTK_WIDGET(bPrev),sensitive);
        if(i>0) {
          sensitive=TRUE;
        }
        if(data->currentStack==CURRENT_STACK_EDITOR) {
            gtk_widget_set_sensitive(GTK_WIDGET(bReplace),sensitive);
            gtk_widget_set_sensitive(GTK_WIDGET(pReplaceEntry),sensitive);
        }
        gtk_label_set_text(GTK_LABEL(hits), g_strdup_printf(_("%d hits"), i));
        /* we get a pointer on current position */
        gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
        gtk_text_buffer_get_start_iter (buffer, &start);
        if(data->currentStack==CURRENT_STACK_EDITOR) {
            find(tView, buffer, tmpStr, &start,0);
        }
        else {
             printf("request to find inside PDF \n");
        }
    }
  }
}

/***********************

 callback for replace
************************/
void
on_replace_clicked  (GtkButton *button, APP_data *data)
{
  GtkTextBuffer *buffer;
  GtkWidget *bPrev, *bNext, *bReplace, *pReplaceEntry, *hits;
  GtkWidget *window1 = data->appWindow;
  GtkWidget *replace_entry, *find_entry;
  GtkTextIter iter, start, end;
  GtkTextMark *mark1, *mark2;
  gchar *tmpStr, *findStr;
  gint remaining_hits=0;
  gboolean sensitive=TRUE;
  gsize length;

  buffer = data->buffer;
  find_entry=lookup_widget(GTK_WIDGET(window1), "search_entry");
  findStr=gtk_entry_get_text(find_entry);
  replace_entry=lookup_widget(GTK_WIDGET(window1), "replace_entry");
  bPrev= lookup_widget(GTK_WIDGET(window1), "buttonNextOccurrence");
  bNext= lookup_widget(GTK_WIDGET(window1), "buttonPrevOccurrence");
  bReplace= lookup_widget(GTK_WIDGET(window1), "buttonReplace");
  hits = lookup_widget(GTK_WIDGET(window1), "labelHits");

  if( gtk_text_buffer_get_selection_bounds (buffer, &start,&end)) {
       tmpStr=gtk_entry_get_text(replace_entry);
       if(tmpStr) {
         /* undo engine */
         data->undo.annotStr=NULL;
         data->undo.pix=NULL;

         GdkAtom format = gtk_text_buffer_register_serialize_tagset(data->buffer, "application/x-gtk-text-buffer-rich-text");
         data->undo.serialized_buffer=gtk_text_buffer_serialize(data->buffer, data->buffer, format, &start, &end, &length);
         data->undo.str_len=strlen(tmpStr);
         mark1=gtk_text_buffer_create_mark (data->buffer, NULL,&start,FALSE);
         gtk_text_buffer_delete (buffer, &start,&end);
         gtk_text_buffer_insert (buffer,&start,tmpStr,-1);
         gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
         mark2=gtk_text_buffer_create_mark (data->buffer, NULL,&iter,FALSE);
         data->undo.undoMark=mark2;
         data->undo.beforeMark=mark1;
         /* update for remaining hits */
         remaining_hits= search_count_matches(buffer, findStr);
         if(remaining_hits<1) {
           sensitive=FALSE;
         }   
         gtk_widget_set_sensitive(GTK_WIDGET(bNext),sensitive);
         gtk_widget_set_sensitive(GTK_WIDGET(bPrev),sensitive);
         gtk_widget_set_sensitive(GTK_WIDGET(bReplace),sensitive);
         gtk_widget_set_sensitive(GTK_WIDGET(replace_entry),sensitive);
         gtk_label_set_text(GTK_LABEL(hits), g_strdup_printf(_("%d hits"), remaining_hits)); 
       }
       undo_push(data->currentStack, OP_REPLACE_TEXT, data);
  } 
}

/************************************
 copy an image from clipboard
 For now we can paste a pixbuf
 from anywhere to Note or Sketch
**************************************/
gint clipboard_paste_image(APP_data *data, gboolean center)
{
 GtkTextIter start, end, iter;
 GtkTextMark *mark1, *mark2;
 GdkPixbuf *pixbuf, *pixbuf2;
 gint x2,y2, root_xs, root_ys;
 cairo_t *cr;
 
 GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
 pixbuf = gtk_clipboard_wait_for_image (clipboard);
 if(pixbuf) {
    if(data->currentStack==CURRENT_STACK_EDITOR) {
     /* TODO we resize PIXbuf since the PDF can be rescaled ! */
     gtk_text_buffer_get_iter_at_mark(data->buffer, &iter, gtk_text_buffer_get_insert(data->buffer));
     pixbuf2= gdk_pixbuf_scale_simple (pixbuf,
                         (gint) gdk_pixbuf_get_width (pixbuf),
                         (gint) gdk_pixbuf_get_height (pixbuf),
                          GDK_INTERP_BILINEAR );
     /* undo engine */
     
     mark2=gtk_text_buffer_create_mark (data->buffer, NULL,&iter,FALSE);
     mark1=gtk_text_buffer_create_mark (data->buffer, NULL,&iter,FALSE);
     data->undo.serialized_buffer=NULL;
     data->undo.annotStr=NULL;
     data->undo.pix=NULL;
     data->undo.undoMark=mark2;
     data->undo.beforeMark=mark2;
     undo_push(data->currentStack, OP_INS_IMG, data);

     gtk_text_buffer_insert_pixbuf(data->buffer, &iter, pixbuf2);
     g_object_unref(pixbuf2);
    }/* endif stack editor */
    if(data->currentStack==CURRENT_STACK_SKETCH) {
       cr = cairo_create (data->Sketchsurface);
       gtk_widget_grab_focus(GTK_WIDGET(data->appWindow));
       g_usleep(400000);/* required : we must wait until the rc-intersect has done is job after popup closing ! */
       /* if center flag we compute the correct x and y positions */
       if(!center) {
         /* all datas are relatve to upper left quadrant in absolute mode = entire screen */
         /* in absolute coordinates FOR the drawable, not the entire window negative coordinates are possible 
            if the window is in outer position from screen - coordinates are relatives to the reference Window 
             if it isn't the entire screen ! Please note the pause in order to allow the screen to refresh before screenshot*/
         data->undo.pix=gdk_pixbuf_get_from_window (gdk_get_default_root_window (),
                             data->x1_event_root, data->y1_event_root,
                             (gint) gdk_pixbuf_get_width (pixbuf)+2, (gint) gdk_pixbuf_get_height (pixbuf)+2);
         data->undo.x1=data->x1;
         data->undo.y1=data->y1;
         gdk_cairo_set_source_pixbuf(cr, pixbuf, data->x1, data->y1);
       }
       else {/* TODO : switch to drawables, not entire screen for undo */
          x2=(CROBAR_VIEW_MAX_WIDTH-(gint) gdk_pixbuf_get_width (pixbuf))/2;
          if(x2<0)
             x2=0;
          y2=(CROBAR_VIEW_MAX_HEIGHT-(gint) gdk_pixbuf_get_height (pixbuf))/2;
          if(y2<0)
             y2=0;
          gdk_window_get_origin (gtk_widget_get_window (data->SketchDrawable), &root_xs, &root_ys);
          data->undo.pix=gdk_pixbuf_get_from_window (gdk_get_default_root_window (),
                             root_xs+x2, root_ys+y2,
                             (gint) gdk_pixbuf_get_width (pixbuf), (gint) gdk_pixbuf_get_height (pixbuf));
          data->undo.x1=x2;
          data->undo.y1=y2;
          gdk_cairo_set_source_pixbuf(cr, pixbuf, x2, y2);
       }
       /* undo engine */
       undo_push(data->currentStack, OP_PASTE_PIXBUF, data);

       cairo_paint(cr);
       cairo_destroy(cr);
       gtk_widget_queue_draw (data->SketchDrawable);
    }
   return 0;
 }/* endif pixbuf */
 else
   return -1;
}

/****************************
  up and down keys on sketch
******************************/

void sketch_moveUp (GtkWidget *parentWindow, APP_data *data)
{
  GtkAdjustment *verticalAdjust;
  gdouble adj_value;

  verticalAdjust = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(data->SketchScrollable));
  gtk_adjustment_set_value (verticalAdjust,gtk_adjustment_get_value(verticalAdjust)-PDF_SCROLL_STEP);
}


void sketch_moveDown (GtkWidget *parentWindow, APP_data *data)
{
  GtkAdjustment *verticalAdjust;
  gdouble upper = 0;
  gdouble page_size = 0;
  gdouble adj_value;

  verticalAdjust = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(data->SketchScrollable));
  upper = gtk_adjustment_get_upper(verticalAdjust);
  adj_value = gtk_adjustment_get_value( verticalAdjust);
  page_size = gtk_adjustment_get_page_size(verticalAdjust);

  if (adj_value < upper - page_size) {
          gtk_adjustment_set_value (verticalAdjust,gtk_adjustment_get_value(verticalAdjust)+PDF_SCROLL_STEP);
  } 
}

/******************************
 CTRL+V  or SHIFT+INS
 - we use the PopUp menu inside the textview
 The rich text formatting is stored.
 Note : if the clipboard contains a pure Image,
 the paste operation if already catched
*********************************************/
void paste_clipboard(GtkTextView *view, APP_data *data)
{
  GtkTextMark *mark1, *mark2;
  GtkTextIter iter, start;

  gtk_text_buffer_get_iter_at_mark(data->buffer, &iter, gtk_text_buffer_get_insert(data->buffer));
  mark2=gtk_text_buffer_create_mark (data->buffer, NULL,&iter,FALSE);

  /* position just before insertion point */
  gboolean fbackwrd=gtk_text_iter_backward_char (&iter);
  if(gtk_text_iter_get_offset(&iter)==0) 
     data->undo.fIsStart=TRUE;
  else
     data->undo.fIsStart=FALSE;
  mark1=gtk_text_buffer_create_mark (data->buffer, NULL,&iter,FALSE);
  //TODO undo_reset_serialized_buffer(data);
  data->undo.serialized_buffer=NULL;
  data->undo.annotStr=NULL;
  data->undo.pix=NULL;
  data->undo.undoMark=mark2;
  data->undo.beforeMark=mark1;
  undo_push(data->currentStack, OP_INS_BLOCK, data);
}
/******************************
 CTRL+X ; works even if
- there is no selection
- we use the PopUp menu inside the textview
The rich text formatting is stored
*********************************************/
void cut_to_clipboard(GtkTextView *view, APP_data *data)
{
  GtkTextIter start, end;
  GtkTextMark *mark;
  gsize length;
  gboolean fSel;

  /* we check if there is a selection */
  if(gtk_text_buffer_get_has_selection (data->buffer)) {
    fSel=gtk_text_buffer_get_selection_bounds (data->buffer, &start, &end);
    mark=gtk_text_buffer_create_mark (data->buffer, NULL,&start,FALSE);
    /* now we must preserve the richtext content and push it on undo engine */
    //TODO undo_reset_serialized_buffer(data);
    data->undo.start_sel=start;
    data->undo.end_sel=end;
    data->undo.undoMark=mark;
    GdkAtom format = gtk_text_buffer_register_serialize_tagset(data->buffer, "application/x-gtk-text-buffer-rich-text");
    data->undo.serialized_buffer=gtk_text_buffer_serialize(data->buffer, data->buffer, format, &start, &end, &length);
    data->undo.buffer_length=length;
    data->undo.annotStr=NULL;
    data->undo.pix=NULL;
    undo_push(data->currentStack, OP_DEL_BLOCK, data);
  }
}
/*****************************
  [BACKSPACE] or SHIFT+BCKSPC
*****************************/
void backspace(GtkTextView *view, APP_data *data)
{
  GtkTextIter start, end, iter;
  GtkTextMark *mark, *mark2;
  gsize length;
  gboolean fSel;

  /* we check if there is a selection */
  if(gtk_text_buffer_get_has_selection (data->buffer)) {
    fSel=gtk_text_buffer_get_selection_bounds (data->buffer, &start, &end);
    mark=gtk_text_buffer_create_mark (data->buffer, NULL,&start,FALSE);
    /* now we must preserve the richtext content and push it on undo engine */
    data->undo.start_sel=start;
    data->undo.end_sel=end;
  }
  else {
    /* we must check if we are at offset 0, then exit */
    mark2=gtk_text_buffer_get_insert(data->buffer);
    gtk_text_buffer_get_iter_at_mark(data->buffer, &iter, mark2);
    if(gtk_text_iter_get_offset(&iter)>0) {     
      end=iter;
      data->undo.end_sel=iter;
      gboolean fbackwrd=gtk_text_iter_backward_char (&iter);
      data->undo.start_sel=iter;
      start=iter;
      mark=gtk_text_buffer_create_mark (data->buffer, NULL,&iter,FALSE);
    }
    else
      return;
  }
  /* we store datas */
  data->undo.undoMark=mark;
  data->undo.annotStr=NULL;
  data->undo.pix=NULL;
  //TODO undo_reset_serialized_buffer(data);
  GdkAtom format = gtk_text_buffer_register_serialize_tagset(data->buffer, "application/x-gtk-text-buffer-rich-text");
  data->undo.serialized_buffer=gtk_text_buffer_serialize(data->buffer, data->buffer, format, &start, &end, &length);
  data->undo.buffer_length=length;
  undo_push(data->currentStack, OP_DEL_BLOCK, data);
}

/*********************************************
  delete from cursor signal
  key bindings : [DEL], CTRL+DEL, CTRL+BKSPC
TODO check this function, add undo for word deletion with backspc

*/
void delete(GtkTextView *view, GtkDeleteType type, gint count, APP_data *data)
{
 printf("delete signal type=%d units =%d \n", type, count);
  GtkTextIter start, end, iter;
  GtkTextMark *mark;
  gsize length;
  gboolean fSel=FALSE;
  gboolean flag, flag_is_start=FALSE;

  /* we check if there is a selection */
  if(gtk_text_buffer_get_has_selection (data->buffer)) {
    fSel=gtk_text_buffer_get_selection_bounds (data->buffer, &start, &end);
    mark=gtk_text_buffer_create_mark (data->buffer, NULL,&start,FALSE);
  }
  else {
    gtk_text_buffer_get_iter_at_mark(data->buffer, &iter, gtk_text_buffer_get_insert(data->buffer));
    flag=gtk_text_iter_backward_char (&iter);
    /* test if we are at start of buffer ! */
    if(!flag){
      flag_is_start=TRUE;
      gtk_text_buffer_get_start_iter(data->buffer, &iter);
    }
    mark=gtk_text_buffer_create_mark (data->buffer, NULL,&iter,FALSE);
    if(!flag_is_start)
        flag=gtk_text_iter_forward_char (&iter);
    start=iter;
    /* we must exit if we are at the end of buffer */
    flag=gtk_text_iter_forward_char (&iter);
    if(!flag) {/* We are at end of buffer nothing to DELete */
      return;
    }
    end=iter;
  }
  /* now we must preserve the richtext content and push it on undo engine */
  data->undo.start_sel=start;
  data->undo.fIsStart=flag_is_start;
  data->undo.end_sel=end;
  data->undo.undoMark=mark;
  data->undo.annotStr=NULL;
  data->undo.pix=NULL;
  GdkAtom format = gtk_text_buffer_register_serialize_tagset(data->buffer, "application/x-gtk-text-buffer-rich-text");
  data->undo.serialized_buffer=gtk_text_buffer_serialize(data->buffer, data->buffer, format, &start, &end, &length);
  data->undo.buffer_length=length;

  if(!fSel)
     undo_push(data->currentStack, OP_DEL_CHAR, data);
  else
     undo_push(data->currentStack, OP_DEL_BLOCK, data);

}
/******************************
  intervalometer function
  called every 5 minutes
  (e.g. 300 secs) to quick save
 must return TRUE to continue
 schedulling - of course, the
 quick saving is controlled by a flag, but timout runs aniway
***************************************************************/
gboolean timeout_quick_save( APP_data *data)
{
  GKeyFile *keyString;
  static count=0;
  GtkWidget *alertDlg;
  GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

  keyString = g_object_get_data(G_OBJECT(data->appWindow), "config"); 
  count++;

  /* busy mouse cursor */
  if(g_key_file_get_boolean(keyString, "application", "interval-save", NULL)) {
       alertDlg =  gtk_message_dialog_new (data->appWindow,
                                      flags,
                                      GTK_MESSAGE_INFO,
                                      GTK_BUTTONS_NONE,
                                      _("I save you work ... wait a moment, please. "),
                                      NULL);
       quick_save(data);
       gtk_widget_destroy (GTK_WIDGET(alertDlg));
  }
  return TRUE;
}
/******************************
  keypress events 
*******************************/
gboolean
key_event(GtkWidget *widget, GdkEventKey *event, APP_data *data)
{
  GtkWidget *search_entry, *replace_entry, *page_entry;
  gchar *tmpStr;
  GtkTextIter start, end, iter;
  GtkTextMark *mark1, *mark2;
  gdouble pen_width;
  GKeyFile *keyString;

  keyString = g_object_get_data(G_OBJECT(data->appWindow), "config"); 
  pen_width=g_key_file_get_double(keyString, "sketch", "pen-width", NULL);

// printf("key=%s\n", gdk_keyval_name (event->keyval));
  search_entry = lookup_widget(GTK_WIDGET(data->appWindow), "search_entry");
  replace_entry = lookup_widget(GTK_WIDGET(data->appWindow), "replace_entry");
  page_entry = lookup_widget(GTK_WIDGET(data->appWindow), "page_entry");
  if(gtk_widget_is_focus (search_entry) || gtk_widget_is_focus (replace_entry) || gtk_widget_is_focus (page_entry)) {
    fdont_care=FALSE;
    printf("DEBUG : I shortcut for -fdont_care- variable \n");
    return FALSE;
  }
  
    if((event->state & GDK_CONTROL_MASK )== GDK_CONTROL_MASK) {

       switch(event->keyval) {
         case GDK_KEY_KP_1:
         case GDK_KEY_1:{
           if(data->currentStack!=CURRENT_STACK_EDITOR) {
             gtk_stack_set_visible_child_name (data->stack,"Note");
             return TRUE;
           }
           break;
         }
         case GDK_KEY_KP_2:
         case GDK_KEY_2:{
           if(data->currentStack!=CURRENT_STACK_PDF) {
             gtk_stack_set_visible_child_name (data->stack,"Refe");
             return TRUE;
           }
           break;
         }
         case GDK_KEY_KP_3:
         case GDK_KEY_3:{
           if(data->currentStack!=CURRENT_STACK_SKETCH) {
             gtk_stack_set_visible_child_name (data->stack,"Sket");
             return TRUE;
           }
           break;
         }
         case GDK_KEY_s:{ 
           quick_save(data);
           break; 
         }
         case GDK_KEY_d:{ 
           on_loadPDF_clicked (NULL, data);
           break; 
         }
         case GDK_KEY_v:{ 
           if(clipboard_paste_image(data, FALSE)!=0) {printf("* no image to paste ! *\n");
               return FALSE;/* user asked to paste somothing else that an image so we continue to dispatch signal */
           }
           return TRUE;
           break; 
         }
         case GDK_KEY_z:{ 
           on_undo_clicked (NULL, data);
           break; 
         }
         case GDK_KEY_m:{ 
           on_main_menu_clicked(lookup_widget(data->appWindow, "main_menu"), data);
           break; 
         }
         case GDK_KEY_F10:{
           show_toolbar=!show_toolbar;
           if(!show_toolbar) 
              gtk_widget_hide(lookup_widget(data->appWindow, "toolbar"));                      
           else
              gtk_widget_show(lookup_widget(data->appWindow, "toolbar"));
           return TRUE;
           break;
         }
         case GDK_KEY_F1:
         case GDK_KEY_question:{
          /* help mode same as in Gnome 2.19+  */
          on_help_clicked(widget);
          break;
         }
         case GDK_KEY_q: {
           /* request to quit application */
           on_quit_clicked(widget , NULL, data);
           break;
         }
         case GDK_KEY_Delete:
         case GDK_KEY_BackSpace:{        
           printf("* Special keys CTRL+DEL and CTRL+BCKSPC are not managed for yet, sorry */\n");
           return TRUE;
           break;
         }
         case GDK_KEY_f: { /* request to find a string */
           /* we check is there is already a selected area */
           search_entry = lookup_widget(GTK_WIDGET(data->appWindow), "search_entry");
           if( gtk_text_buffer_get_selection_bounds (data->buffer, &start,&end)) {
              tmpStr=gtk_text_buffer_get_text(data->buffer,&start,&end,FALSE);
              gtk_entry_set_text(search_entry,tmpStr);
              gtk_widget_grab_focus(search_entry);
              g_free(tmpStr);
           }                      
           break;
         }
         case GDK_KEY_plus:case GDK_KEY_KP_Add:{
           if(data->currentStack==CURRENT_STACK_PDF) {
                 on_PDF_zoom_in_clicked(widget, data);
           return TRUE;
           }
          if(data->currentStack==CURRENT_STACK_SKETCH) {
                 pen_width=pen_width+1;
                 if(pen_width>20)
                    pen_width=20;
                 g_key_file_set_double(keyString, "sketch", "pen-width", pen_width);
                 update_statusbarSketch(data);
           return TRUE;
           }
           break;
         }
         case GDK_KEY_minus:case GDK_KEY_KP_Subtract:{
           if(data->currentStack==CURRENT_STACK_PDF) {
                 on_PDF_zoom_out_clicked(widget, data);
           return TRUE;
           }
          if(data->currentStack==CURRENT_STACK_SKETCH) {
                 pen_width=pen_width-1;
                 if(pen_width<1)
                    pen_width=1;
                 g_key_file_set_double(keyString, "sketch", "pen-width", pen_width);
                 update_statusbarSketch(data);
           return TRUE;
           }
           break;
         }

         default:;
       }/* end switch */
     }
    else {/* other special keys */
       switch(event->keyval) {
         case GDK_KEY_Insert:{
           if(gtk_text_view_get_overwrite (data->view)) {
              printf("* I cancel overwrite mode *\n");
           }
           else
              printf("* OK, we are already in insertion mode *\n");
           gtk_text_view_set_overwrite (data->view, FALSE);
           return TRUE;
           break;
         }
         case GDK_KEY_Down:{
           if(data->currentStack==CURRENT_STACK_PDF) {
              PDF_moveDown(widget, data);
           }
           else {
             if(data->currentStack==CURRENT_STACK_SKETCH) {
              sketch_moveDown(widget, data);
             }
           }
           break;
         }                  
         case GDK_KEY_Up:{
           if(data->currentStack==CURRENT_STACK_PDF) {
              PDF_moveUp(widget, data);
           }
           else {
             if(data->currentStack==CURRENT_STACK_SKETCH) {
               sketch_moveUp(widget, data);
             }
           }
           break;
         }         
         case GDK_KEY_Page_Up:{
           if(data->currentStack==CURRENT_STACK_PDF)
              PDF_moveBackward (widget, data);
           break;
         }
         case GDK_KEY_Page_Down:{
           if(data->currentStack==CURRENT_STACK_PDF)
              PDF_moveForward (widget, data);
           break;
         }
         case GDK_KEY_Home:{
           if(data->currentStack==CURRENT_STACK_PDF)
              PDF_moveHome (widget, data);
           break;
         }
         case GDK_KEY_End:{
           if(data->currentStack==CURRENT_STACK_PDF)
              PDF_moveEnd (widget, data);
           break;
         }
         case GDK_KEY_Delete:{ /* should be managed by signal so we return FALSE*/              
           break;
         }
         case GDK_KEY_BackSpace:{ /* idem */       
           break;
         }
         case GDK_KEY_F1:{/* standard F1 help mode same old pc's */
          on_help_clicked(widget);
          break;
         }
         default:{/* supposed standard printable chars , be careful backspace is treated as printable by X11 !!! */
            if(gdk_keyval_to_unicode (event->keyval)!=0) {
// printf("key=%s\n", gdk_keyval_name (event->keyval));
               /* we push a single char to undo engine */
               gtk_text_buffer_get_iter_at_mark(data->buffer, &iter, gtk_text_buffer_get_insert(data->buffer));
               mark2=gtk_text_buffer_create_mark (data->buffer, NULL,&iter,FALSE);
               mark1=gtk_text_buffer_create_mark (data->buffer, NULL,&iter,FALSE);
               data->undo.serialized_buffer=NULL;
               data->undo.annotStr=NULL;
               data->undo.pix=NULL;
               data->undo.undoMark=mark1;
               data->undo.beforeMark=mark2;
               undo_push(data->currentStack, OP_INS_CHAR, data);
            }
         }
       }/* end switch special keys */
    }
    return FALSE;
}


/***********************************************
 CB : response to recent-files list of menitems
 with 'paving' display
*************************************************/
void menuitem_response(GtkMenuItem *menuitem, APP_data *user_data)
{
  GtkWidget *currentFileDialog;
  gchar *path_to_file, *recent_prev, *content_prev;
  GKeyFile *keyString;
  GtkTextBuffer *buffer;
  gint i, ret, file_to_load;

  buffer = user_data->buffer;
  GtkWidget *window1 = user_data->appWindow;
  keyString = g_object_get_data(G_OBJECT(window1), "config");

  /* first, we save once more time the current file */
  path_to_file = g_key_file_get_string(keyString, "application", "current-file", NULL);
  ret = save_gtk_rich_text(path_to_file, buffer);
  g_free(path_to_file);

  /* now we display the blocks with files and summaries */
 
  currentFileDialog=paving_window(user_data );
  ret=gtk_dialog_run(GTK_DIALOG(currentFileDialog));
  gtk_widget_destroy (GTK_WIDGET(currentFileDialog));

  if(ret>=PAVING_BUTTON1 && ret<=PAVING_BUTTON10) {
     file_to_load=ret-PAVING_BUTTON1;
     /* we read the file path within le config.ini file */
     path_to_file=g_strdup_printf("%s", g_key_file_get_string(keyString, "history", 
                                          g_strdup_printf("recent-file-%d",file_to_load), NULL));
     /* we check if the file exists */
     if(g_file_test (path_to_file, G_FILE_TEST_EXISTS)) {
       /* now we clear the datas */
       misc_clear_text(buffer, "left");
       /* now we set-up a new default filename */
       gtk_window_set_title (GTK_WINDOW(window1),g_strdup_printf(_("%s"), path_to_file));
       /* and we finally reload datas */
       if(load_gtk_rich_text(path_to_file, buffer, window1)!=0) {
          misc_clear_text(buffer, "left");
       }
       /* rearrange list of recent files only if we haven't choosen the first !*/
       if(ret!=PAVING_BUTTON1) {
         /* we remove the choosen file */
         if(ret<PAVING_BUTTON10){
            g_key_file_set_string(keyString, "history", g_strdup_printf("recent-file-%d",ret-PAVING_BUTTON1), "");
            g_key_file_set_string(keyString, "history", g_strdup_printf("recent-content-%d",ret-PAVING_BUTTON1), "");
         }
         /* we rearrange recent files & contents */
         for(i=ret-PAVING_BUTTON1;i>0;i--) {
            // printf("boucle paving i=%d \n", i);
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
             }/* endif */
         }/* next i*/
       }/* endif */
       /* we change the default values for gkeyfile */
       store_current_file_in_keyfile(keyString, path_to_file, misc_get_extract_from_document(user_data));/* on the top, new doc loaded */
     }
     g_free(path_to_file);       
  }/* endif Buttons */
}
/**************************
  function to fix the PopUp menu near the button
  From : https://stackoverflow.com/questions/11756767/place-popup-menu-below-widget-gtk
  adapted by me for Gtk3
***************************/
static void
set_position(GtkMenu *menu, gint *x, gint *y, gboolean *push_in, gpointer user_data)
{
  GtkRequisition minimum_size, natural_size;
  GtkWidget *button = GTK_WIDGET(user_data);
  GdkWindow *gtk_window = gtk_widget_get_parent_window(GTK_WIDGET(button));
  gdk_window_get_origin(gtk_window, x, y);
  gtk_widget_get_preferred_size (button, &minimum_size, &natural_size);/* yes, the TRUE widget ! */
  *x += natural_size.width/2;
  *y += natural_size.height;
}

/*************************
  detect stack change
**************************/
void
on_stack_changed (GObject *gobject, GParamSpec *pspec, APP_data *user_data)
{
  gchar *tmpStr=NULL;
  gint prevStack;
  GtkTextBuffer *buffer=user_data->buffer;
  GtkTextIter iter;

  gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));
  
  GtkWidget *statusbar = user_data->statusbar1; /* main statusbar */
  tmpStr = gtk_stack_get_visible_child_name (GTK_STACK(gobject));

  prevStack=user_data->currentStack;
  if(prevStack== CURRENT_STACK_EDITOR) {
     gtk_text_buffer_select_range (buffer, &iter, &iter); // unselect text
  }
  if(prevStack== CURRENT_STACK_PDF) {
     /* unselect in PDF page */
     if(user_data->pdfSearch) {
       printf("I shall remove PDF marks of search \n");
       search_free_PDF_search_datas(user_data);
     }
  }
  /* we change widgets state according to current stack */
  if(g_ascii_strncasecmp ((gchar*)  tmpStr,"Note",4* sizeof(gchar))  ==  0 ) {
     update_statusbar(user_data->buffer, user_data);
     user_data->currentStack = CURRENT_STACK_EDITOR;
     misc_set_gui_in_editor_mode(user_data->appWindow, prevStack);   
     gtk_widget_grab_focus(GTK_WIDGET(user_data->view));
  }
  else if(g_ascii_strncasecmp ((gchar*)  tmpStr,"Refe",4* sizeof(gchar))  ==  0 ) {
         update_statusbarPDF(user_data); 
         user_data->currentStack = CURRENT_STACK_PDF;
         misc_set_gui_in_PDF_mode(user_data->appWindow, prevStack);
         gtk_widget_grab_focus(GTK_WIDGET(user_data->PDFScrollable));
       }
       else if(g_ascii_strncasecmp ((gchar*)  tmpStr,"Sket",4* sizeof(gchar))  ==  0 ) {
                  update_statusbarSketch(user_data);
                  user_data->currentStack = CURRENT_STACK_SKETCH;
                  misc_set_gui_in_sketch_mode(user_data->appWindow, prevStack);// tempo until sketch built !!!!
             }
  /* we store current cursor position */
  user_data->curpos=&iter;
  /* never free tmpStr ! */
}
/************************************
  main menu
************************************/

void
on_main_menu_clicked  (GtkButton  *button,  APP_data *data_app)
{
  GtkWidget *menu, *window1;
  window1 = data_app->appWindow;
  menu = create_menu1(window1, data_app);
  gtk_menu_popup(GTK_MENU (menu), NULL, NULL,  (GtkMenuPositionFunc) set_position, GTK_WIDGET(button),
                 1, gtk_get_current_event_time());

}
void on_doc_show_menu(GtkMenuToolButton *button, GtkMenu *menu, APP_data *data_app)
{
  GtkWidget *window1;

  gtk_menu_popup(GTK_MENU (menu), NULL, NULL,  (GtkMenuPositionFunc) set_position, GTK_WIDGET(button),
                 1, gtk_get_current_event_time()); 
}

/******************************
  size of PDF area changed
******************************/
void on_PDF_size_changed (GtkWidget *widget, GdkRectangle *allocation, APP_data *data_app)
{
  if(data_app->doc) {
     PDF_display_page(data_app->appWindow, data_app->curPDFpage, data_app->doc, data_app);
  }
}


/******************
  zoom fit-best PDF
*******************/
void on_PDF_zoom_fit_best_clicked  (GtkButton *button, APP_data *data)
{
  GKeyFile *keyString;

  keyString = g_object_get_data(G_OBJECT(data->appWindow), "config");

  if(data->doc) {
     data->PDFratio=misc_get_PDF_ratio(data->PDFWidth,  gtk_widget_get_allocated_width (data->appWindow));
     PDF_display_page(data->appWindow, data->curPDFpage, data->doc, data);
     g_key_file_set_double(keyString, "reference-document", "zoom", data->PDFratio);
  }
}

/*****************************
  mouse scroll on PDF page
 thanks to a bug report here :
 https://bugzilla.gnome.org/show_bug.cgi?id=675959
***************************************/

gboolean on_PDF_scroll_event (GtkWidget *widget, GdkEvent *event, APP_data *data)
{
  GdkEventScroll *scroll_event;

  scroll_event = (GdkEventScroll *) event;

  if(scroll_event->delta_y<0) {
    /* smooth scroll Up */
    PDF_moveUp(widget, data);
  }

  if(scroll_event->delta_y>0) {
    /* smooth scroll down */
    PDF_moveDown(widget, data);
  }
   return TRUE;  
}

/*************************************
  CB : called by paste in sketch menu
**************************************/
void on_menuPasteSketch(GtkMenuItem *menuitem, APP_data *user_data)
{

  gtk_widget_destroy(GTK_WIDGET(lookup_widget(GTK_WIDGET(menuitem), "menu1Sketch")));
  if(clipboard_paste_image(user_data, FALSE)!=0)
               printf("* can't paste inside sketch *\n"); 
}

/*************************************
  CB : called by paste in sketch menu
  here it's the 'centered' pasting
**************************************/
void on_menuCenteredPasteSketch(GtkMenuItem *menuitem, APP_data *user_data)
{
  if(clipboard_paste_image(user_data, TRUE)!=0)
               printf("* can't paste inside sketch *\n"); 
}

/*************************************
  CB : called by edit annotation in 
  PDF popup menu
**************************************/
void on_menuPDFEditAnnot(GtkMenuItem *menuitem, APP_data *user_data)
{
  gchar *tmpStr=NULL;

  if(user_data->undo.annotStr!=NULL)
           g_free(user_data->undo.annotStr);
  user_data->undo.annotStr=g_strdup_printf("%s", poppler_annot_get_contents ( user_data->current_annot));

  tmpStr=dialog_add_text_annotation(user_data->appWindow, poppler_annot_get_contents ( user_data->current_annot), user_data);
  if(tmpStr!=NULL) {
     /* undo engine */
     user_data->undo.curStack=CURRENT_STACK_PDF;
     user_data->undo.opCode=OP_SET_ANNOT_STR;
     user_data->undo.pix=NULL;
     user_data->undo.x1=user_data->x1;
     user_data->undo.y1=user_data->y1;
     user_data->undo.x2=user_data->x2;
     user_data->undo.y2=user_data->y2;
     user_data->undo.PDFpage=user_data->curPDFpage;
     undo_push(user_data->currentStack, OP_SET_ANNOT_STR, user_data);
     poppler_annot_set_contents (user_data->current_annot,tmpStr);
  }
  else { 
    g_free(user_data->undo.annotStr);
    user_data->undo.annotStr=NULL;
  }
  g_free(tmpStr);
  PDF_display_page(user_data->PDFScrollable, user_data->curPDFpage, user_data->doc, user_data);
}

/*************************************
  CB : called by change color annotation 
  in  PDF popup menu
***************************************/
void on_menuPDFColorAnnot(GtkMenuItem *menuitem, APP_data *user_data)
{
  GdkRGBA color, cp;
  PopplerColor *current_color;

  current_color=  poppler_annot_get_color ( user_data->current_annot);
  cp.red = (gdouble)current_color->red/65535;
  cp.green = (gdouble)current_color->green/65535;
  cp.blue = (gdouble)current_color->blue/65535;
  /* undo engine */
  user_data->undo.curStack=CURRENT_STACK_PDF;
  user_data->undo.opCode=OP_SET_ANNOT_COLOR;
  user_data->undo.color=cp;
  user_data->undo.annot=user_data->current_annot;
  user_data->undo.x1=user_data->x1;
  user_data->undo.y1=user_data->y1;
  user_data->undo.x2=user_data->x2;
  user_data->undo.y2=user_data->y2;
  user_data->undo.PDFpage=user_data->curPDFpage;
  user_data->undo.pix=NULL;
  if(user_data->undo.annotStr!=NULL)
           g_free(user_data->undo.annotStr);
  GtkColorChooserDialog *dialog = create_annotationColourDialog(user_data, _("Choose annotation color ..."));
  gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dialog), &cp);
  g_free(current_color);
  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK) {
        gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog), &color);
        current_color=poppler_color_new();
        current_color->red=65535*color.red;
        current_color->green=65535*color.green;
        current_color->blue=65535*color.blue;
        poppler_annot_set_color (user_data->current_annot, current_color);
        poppler_color_free(current_color);
        undo_push(user_data->currentStack, OP_SET_ANNOT_COLOR, user_data);
  }
  gtk_widget_destroy(GTK_WIDGET(dialog));
  PDF_display_page(user_data->PDFScrollable, user_data->curPDFpage, user_data->doc, user_data);
}

/************************************
  CB : called by remove annotation 
  in  PDF popup menu
*************************************/
void on_menuPDFRemoveAnnot(GtkMenuItem *menuitem, APP_data  *user_data)
{
  gint ret=0;
  GdkRGBA cp;
  PopplerColor *current_color;

  current_color=  poppler_annot_get_color ( user_data->current_annot);
  cp.red = (gdouble)current_color->red/65535;
  cp.green = (gdouble)current_color->green/65535;
  cp.blue = (gdouble)current_color->blue/65535;
  g_free(current_color);
  user_data->undo.curStack=CURRENT_STACK_PDF;
  user_data->undo.opCode=OP_REMOVE_ANNOT;
  user_data->undo.color=cp;
     user_data->undo.x1=user_data->x1;
     user_data->undo.y1=user_data->y1;
     user_data->undo.x2=user_data->x2;
     user_data->undo.y2=user_data->y2;
     user_data->undo.pix=NULL;
     user_data->undo.PDFpage=user_data->curPDFpage;
     if(user_data->undo.annotStr!=NULL)
           g_free(user_data->undo.annotStr);
     user_data->undo.annotStr=g_strdup_printf("%s", poppler_annot_get_contents ( user_data->current_annot));
     user_data->undo.annotType=poppler_annot_get_annot_type (user_data->current_annot);
     undo_push(user_data->currentStack, OP_REMOVE_ANNOT, user_data);
  /* Popplerrectangle for simple text annotation, PopplerQuads for others ! */
  if(user_data->undo.annotType!=POPPLER_ANNOT_TEXT && user_data->undo.annotType!=POPPLER_ANNOT_HIGHLIGHT) {
     GtkWidget *dialog;
     GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
     dialog = gtk_message_dialog_new (user_data->appWindow,
                                      flags,
                                      GTK_MESSAGE_QUESTION,
                                      GTK_BUTTONS_OK_CANCEL,
                                      _("Warning !\nI can't, for now, *undo* removing for\nthis kind of annotation !\nProceed aniway ?"));
     if(gtk_dialog_run(GTK_DIALOG(dialog))==GTK_RESPONSE_CANCEL) 
        ret=-1;
     gtk_widget_destroy(GTK_WIDGET(dialog));
  }
  if(ret==0)
      poppler_page_remove_annot (poppler_document_get_page (user_data->doc, user_data->curPDFpage),
                           user_data->current_annot);
  PDF_display_page(user_data->PDFScrollable, user_data->curPDFpage, user_data->doc, user_data);
}

/*****************************

  audio callbacks

****************************/

void
on_play_pause_clicked (GtkButton *button, APP_data *data)
{
  gboolean ret=FALSE;
  gint64 pos;
  GKeyFile *keyString;
  gint64 step;

  keyString = g_object_get_data(G_OBJECT(data->appWindow), "config");
  step=(gint64)g_key_file_get_double(keyString, "application", "audio-file-rewind-step", NULL);

  if(data->fAudioPlaying)  {
    /* thus we must pause playing */
    data->fAudioPlaying=FALSE;
    gst_element_set_state (data->pipeline, GST_STATE_PAUSED);
    /* we change the icon to pause || */
    gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON( lookup_widget(GTK_WIDGET(data->appWindow), "pRadioButtonPlayPauseAudio")  ), 
                       GTK_IMAGE(lookup_widget(GTK_WIDGET(data->appWindow), "iconButtonPlayAudio")));
    /* and we do a backward jump */
    audio_get_position(data->pipeline, &pos );
    data->audio_current_position=pos;
    audio_seek_backward(data->pipeline, step, pos, data->audio_total_duration);
  }
  else {
   /* we change the icon to pause || */
    gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON( lookup_widget(GTK_WIDGET(data->appWindow), "pRadioButtonPlayPauseAudio")  ), 
                       GTK_IMAGE(lookup_widget(GTK_WIDGET(data->appWindow), "iconButtonPauseAudio")));
    /* thus we must (re)start playing */
    data->fAudioPlaying=TRUE;
    gst_element_set_state (data->pipeline, GST_STATE_PLAYING);
  }
  /*  display current position */
  ret=FALSE;
  audio_get_position(data->pipeline, &pos );
  data->audio_current_position=pos;
  /* we change the position display */
  gtk_label_set_markup ( GTK_LABEL(lookup_widget(GTK_WIDGET(data->appWindow), "audio_position_label")), 
                           g_strdup_printf("<tt><big>%s</big></tt>", 
                           audio_gst_time_to_str(data->audio_current_position))); 
}


void
on_jump_prev_clicked (GtkButton *button, APP_data *data)
{
  gboolean ret=FALSE;
  gint64 pos;
  GKeyFile *keyString;
  gint64 step;
 
  keyString = g_object_get_data(G_OBJECT(data->appWindow), "config");
  step=(gint64)g_key_file_get_double(keyString, "application", "audio-file-marks-step", NULL);

  audio_get_position(data->pipeline, &pos );
  audio_seek_backward(data->pipeline, step, pos, data->audio_total_duration);
  /* refresh current position */
  audio_get_position(data->pipeline, &pos );
  data->audio_current_position=pos;
  /* we change the position display */
  gtk_label_set_markup ( GTK_LABEL(lookup_widget(GTK_WIDGET(data->appWindow), "audio_position_label")), 
                           g_strdup_printf("<tt><big>%s</big></tt>", 
                           audio_gst_time_to_str(data->audio_current_position)));
} 

void
on_jump_next_clicked (GtkButton *button, APP_data *data)
{
  gboolean ret=FALSE;
  gint64 pos;
  GKeyFile *keyString;
  gint64 step;

  keyString = g_object_get_data(G_OBJECT(data->appWindow), "config");
  step=(gint64)g_key_file_get_double(keyString, "application", "audio-file-marks-step", NULL);

  audio_get_position(data->pipeline, &pos );
  audio_seek_forward(data->pipeline, step, pos, data->audio_total_duration);
  /* refresh current position */
  audio_get_position(data->pipeline, &pos );
  data->audio_current_position=pos;
  /* we change the position display */
  gtk_label_set_markup ( GTK_LABEL(lookup_widget(GTK_WIDGET(data->appWindow), "audio_position_label")), 
                           g_strdup_printf("<tt><big>%s</big></tt>", 
                           audio_gst_time_to_str(data->audio_current_position))); 

}

void on_go_jump_clicked(GtkButton *button, APP_data *data)
{
  GtkWidget *dialog;
  gdouble val, hour, min, sec;
  gint64 pos=0;

  /* refresh current position */
  audio_get_position(data->pipeline, &pos );
  data->audio_current_position = pos;

  dialog= misc_create_go_jump_dialog(data);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK) {
    /* we get the current values */
    hour=gtk_spin_button_get_value(GTK_SPIN_BUTTON(lookup_widget(GTK_WIDGET(dialog), "hourSpin")));
    min=gtk_spin_button_get_value(GTK_SPIN_BUTTON(lookup_widget(GTK_WIDGET(dialog), "minuteSpin")));
    sec=gtk_spin_button_get_value(GTK_SPIN_BUTTON(lookup_widget(GTK_WIDGET(dialog), "secondSpin")));
    /* we seek to the new position */
    audio_seek_to_time (data->pipeline, 
                        audio_time_to_gst_time((gint64)hour,(gint64) min ,(gint64)sec, data->audio_total_duration));
    /* refresh current position */
    audio_get_position(data->pipeline, &pos );
    data->audio_current_position=pos;
    /* we change the position display */
    gtk_label_set_markup ( GTK_LABEL(lookup_widget(GTK_WIDGET(data->appWindow), "audio_position_label")), 
                           g_strdup_printf("<tt><big>%s</big></tt>", 
                           audio_gst_time_to_str(data->audio_current_position)));    
  }
  gtk_widget_destroy(GTK_WIDGET(dialog));
}

/******************************
  intervalometer function
  called every ????
  (e.g. 300 secs) to quick save
 must return TRUE to continue
 schedulling - of course, the
 quick saving is controlled by a flag, but timout runs aniway
***************************************************************/
gboolean timeout_audio_display_position( APP_data *data)
{
  GKeyFile *keyString;
  gint64 pos, len;
  GstBus *bus;
  GstMessage *msg;
  gboolean error;

  keyString = g_object_get_data(G_OBJECT(data->appWindow), "config"); 

  error=FALSE;
  if(data->fAudioPlaying && data->fAudioLoaded) {
    /* refresh current position */
    audio_get_position(data->pipeline, &pos );
    data->audio_current_position=pos;
    bus = gst_element_get_bus (data->pipeline);
    msg=gst_bus_timed_pop_filtered (bus, 0, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);
    if(msg!=NULL) {
       error=TRUE; 
         printf("* Gstreamer : Bus error message ! \n");
       gst_message_unref (msg);
    } 
    if(pos>=data->audio_total_duration || error) {
        data->fAudioPlaying=FALSE;
        gst_element_set_state (data->pipeline, GST_STATE_PAUSED);
        /* we change button icon */
        gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON( lookup_widget(GTK_WIDGET(data->appWindow), "pRadioButtonPlayPauseAudio")  ), 
                       GTK_IMAGE(lookup_widget(GTK_WIDGET(data->appWindow), "iconButtonPlayAudio")));
        data->audio_current_position=data->audio_total_duration;
        /* if option checked, we rewind to audio's start */
        if( g_key_file_get_boolean(keyString, "application", "audio-auto-rewind",NULL )){
           audio_seek_to_time (data->pipeline, audio_time_to_gst_time(0,0 ,0, data->audio_total_duration));
           /* refresh current position */
           audio_get_position(data->pipeline, &pos );
           data->audio_current_position=pos;
           /* we change the position display */
           gtk_label_set_markup ( GTK_LABEL(lookup_widget(GTK_WIDGET(data->appWindow), "audio_position_label")), 
                           g_strdup_printf("<tt><big>%s</big></tt>", 
                           audio_gst_time_to_str(data->audio_current_position)));  
        }
    }
    gst_object_unref (bus);
    /* we change the position display */
    gtk_label_set_markup ( GTK_LABEL(lookup_widget(GTK_WIDGET(data->appWindow), "audio_position_label")), 
                           g_strdup_printf("<tt><big>%s</big></tt>", 
                           audio_gst_time_to_str(data->audio_current_position))); 
  }
  return TRUE;
}
/**************************/
/* audio speed call back */
/**************************/
void on_audioPlaySpeed_changed(GtkComboBox *combobox, APP_data *data)
{
  gdouble speed;
  gint64 pos;
  GstEvent *seek_event;/* see : https://stackoverflow.com/questions/29222275/how-to-modify-playback-speed-of-audio-stream-in-gstreamer */
  gboolean result;

  speed=gtk_combo_box_get_active (GTK_COMBO_BOX(lookup_widget(GTK_WIDGET(data->appWindow), "audioPlaySpeed")));
  switch((gint)speed) {
    case 0:{
       speed=1.5;
       break;
    }
    case 1:{
       speed=1.2;
       break;
    }
    case 2:{
       speed=1;
       break;
    }
    case 3:{
       speed=0.8;
       break;
    }
    case 4:{
       speed=0.5;
       break;
    }
    default:{
      printf("* unknown error on playing speed selection ! *\n");
      speed=1;
    }
  }/* end switch */
  /* we set the new speed */
  audio_get_position(data->pipeline, &pos );
  data->audio_current_position=pos;
  seek_event = gst_event_new_seek (speed, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE,
        GST_SEEK_TYPE_SET, pos, GST_SEEK_TYPE_NONE, 0);
  result=gst_element_send_event(data->pipeline, seek_event);
  if(!result) {
     printf("* Warning ! Can't set up the requested audio speed ! *\n");
  }
}

void
on_about1_activate (GtkMenuItem  *menuitem, APP_data *data)
{
  GtkWidget *aboutDialog = create_aboutRedac(data);
  
  gtk_dialog_run(GTK_DIALOG (aboutDialog));  
  gtk_widget_destroy(GTK_WIDGET (aboutDialog));
  return;
}

void on_wiki1_activate (GtkMenuItem  *menuitem, APP_data *data)
{
 gboolean ret;
 GError *error = NULL;
 GtkWidget *alertDlg;
 GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
 gint rep;

 // g_type_init();

  ret = g_app_info_launch_default_for_uri("https://github.com/Luc-Ami/redac/wiki", NULL, &error);
  if (!ret) {
      alertDlg = gtk_message_dialog_new_with_markup (GTK_WINDOW(data->appWindow),
                          flags,
                          GTK_MESSAGE_ERROR,
                          GTK_BUTTONS_OK,NULL,
                          _("<big><b>Can't access to online wiki !</b></big>\nPlease, check your Internet connection\n and/or you desfault desktop file browser parameters."));
      rep=gtk_dialog_run(GTK_DIALOG(alertDlg));
      gtk_widget_destroy (GTK_WIDGET(alertDlg));
  }
}

void
on_keyHelp1_activate (GtkMenuItem  *menuitem, APP_data *data)
{
  on_help_clicked(data->appWindow);
  return;
}
