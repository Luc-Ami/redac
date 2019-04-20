/***************************
  functions to export files
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
#include <poppler.h>
#include "interface.h" /* glade requirement */
#include "support.h" /* glade requirement */
#include "misc.h"
#include "pdf.h"
#include "undo.h"

/********************************
  test if there is an annotation 
  for current mouse click
  position

********************************/
PopplerAnnot *PDF_find_annot_at_position(gint x, gint y, APP_data *data)
{
  PopplerRectangle selection; /* in original PDF points 4 gdouble bounding rectangle coord*/
  gdouble ratio, v_v_adj, v_h_adj;
  gint i=0, x1, y1, x2, y2;
  GList *l=NULL;
  PopplerAnnotMapping *current_annot_map;
  PopplerAnnot *current_annot=NULL;

  if(data->pdfAnnotMapping==NULL)
    return current_annot;
  /* we read current adjustments on scrollable window */
  GtkAdjustment *v_adj= gtk_scrolled_window_get_vadjustment (data->PDFScrollable);
  GtkAdjustment *h_adj= gtk_scrolled_window_get_hadjustment (data->PDFScrollable);
  v_v_adj = gtk_adjustment_get_value (v_adj);
  v_h_adj = gtk_adjustment_get_value (h_adj);   
  ratio = data->PDFratio;
  /* we translate to PDF coordinates */
  selection.x1= (gdouble) (x+v_h_adj)/ratio;
  selection.y1= (gdouble)data->PDFHeight- (y+v_v_adj)/ratio;/* yes PDF inverted vertical coordinates ! */
  /* now we search the first PopplerAnnot with coincident coordinates */
  l=data->pdfAnnotMapping;
  for(l;l!=NULL;l=l->next) {
    current_annot_map=(PopplerAnnotMapping *)l->data;
 //  printf("annot=%d coordonneés x1=%.2f y1=%.2f x2=%.2f y2=%.2f souris x1=%.2f y1=%.2f \n", i+1, current_annot_map->area.x1, 
   //           current_annot_map->area.y1, current_annot_map->area.x2, current_annot_map->area.y2, selection.x1,selection.y1);

    if(selection.x1>=current_annot_map->area.x1 && selection.y1>=current_annot_map->area.y1 
       && selection.x1<=current_annot_map->area.x2 && selection.y1<=current_annot_map->area.y2) {
        // printf("bingo ! coinc for annot =%d annotype =%d \n", i+1, poppler_annot_get_annot_type (current_annot_map->annot));
        current_annot=current_annot_map->annot;
        data->x1=(gint)current_annot_map->area.x1;
        data->y1=(gint)current_annot_map->area.y1;
        data->x2=(gint)current_annot_map->area.x2;
        data->y2=(gint)current_annot_map->area.y2;
    }
    i++;
  }
   
  return current_annot;
}

/**********************************
  get the annot mapping for the
  current page
  when this function is called,
  the 'data" structure contains 
  all necessary datas : current page,
  a pointer on PDF, doc, and so son
************************************/
GList *PDF_get_annot_mapping(APP_data *data)
{
  PopplerPage *pPage;

  if(data->doc==NULL)
    return NULL;
  pPage = poppler_document_get_page (data->doc, data->curPDFpage);
  if(pPage==NULL)
    return NULL;  
  return poppler_page_get_annot_mapping (pPage);
}

/*****************************************
  display a PDF, autosized to allow 
  scrolling
 of gtk_drawing_area
******************************************/
void PDF_display_page (GtkWidget *window1, gint page, PopplerDocument *pdoc, APP_data *data)
{
  GKeyFile *keyString;
  GdkRGBA color; 
  gint w, h;
  gdouble width, height, ratio;
  cairo_t *cr, *PDF_cr;
  PopplerPage *pPage;
  GtkWidget *canvas;

  keyString = g_object_get_data(G_OBJECT(data->appWindow), "config");
  color.red=g_key_file_get_double(keyString, "reference-document", "paper.color.red", NULL);
  color.green=g_key_file_get_double(keyString, "reference-document", "paper.color.green", NULL);
  color.blue=g_key_file_get_double(keyString, "reference-document", "paper.color.blue", NULL);
  color.alpha=1;

  canvas = lookup_widget(GTK_WIDGET(window1), "crPDF");
  pPage = poppler_document_get_page (pdoc, page);
  poppler_page_get_size (pPage, &width, &height);
  //ratio = misc_get_PDF_ratio(width,  gtk_widget_get_allocated_width (window1));
  //data->PDFratio=ratio;

  //g_key_file_set_double(keyString, "reference-document ", "zoom", data->PDFratio);
  ratio=data->PDFratio;
  w = (gint) (width*ratio);
  h = (gint) (height*ratio);

  cairo_surface_destroy (data->surface);
  data->surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, w, h);
  cr = cairo_create (data->surface);
  /* blue-grey background */
  //cairo_set_source_rgb(cr, 0.88, 0.88, 0.95);
  //cairo_rectangle(cr, 0, 0, w, h);
  //cairo_fill(cr);
  /*  background centered */
  cairo_set_source_rgb(cr, color.red, color.green, color.blue);
  cairo_rectangle(cr, 0, 0, w, h);
  cairo_fill(cr);
  // cairo_set_source_surface(cr, data->surface, 30, 30);
  cairo_scale(cr, ratio,ratio);  
  poppler_page_render (pPage, cr);  
  cairo_destroy (cr);
  gtk_widget_set_size_request (canvas, w, h);
  gtk_widget_queue_draw (canvas);
  g_object_unref(pPage);
  if(data->pdfSearch)/* special for Manjaro ;-) */
     search_draw_selection_current_page(data->curPDFpage, data, data->surface); 
}

/*******************************
  various functions to navigate
  the PDF doc
*********************************/
void PDF_moveUp (GtkWidget *parentWindow, APP_data *data)
{
  GtkAdjustment *verticalAdjust;
  gdouble adj_value;
  gdouble upper = 0;
  gdouble page_size = 0;

  if(!data->doc)
     return;

  verticalAdjust = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(data->PDFScrollable));
  adj_value=gtk_adjustment_get_value(verticalAdjust);
  upper = gtk_adjustment_get_upper(verticalAdjust);
  page_size = gtk_adjustment_get_page_size(verticalAdjust);
  if(adj_value>0)
     gtk_adjustment_set_value (verticalAdjust,adj_value-PDF_SCROLL_STEP);
  else {
    if(data->curPDFpage>0) {
      gtk_adjustment_set_value (verticalAdjust,upper-PDF_SCROLL_STEP);
      PDF_moveBackward (parentWindow, data);
    }
  }
}

/* 

see : https://stackoverflow.com/questions/11353184/gtk-programmatically-scroll-back-a-single-line-in-scrolled-window-containing-a 

*/

void PDF_moveDown (GtkWidget *parentWindow, APP_data *data)
{
  GtkAdjustment *verticalAdjust;
  gdouble upper = 0;
  gdouble page_size = 0;
  gdouble adj_value;

  if(!data->doc)
     return;

  verticalAdjust = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(data->PDFScrollable));
  upper = gtk_adjustment_get_upper(verticalAdjust);
  adj_value = gtk_adjustment_get_value( verticalAdjust);
  page_size = gtk_adjustment_get_page_size(verticalAdjust);

  if (adj_value < upper - page_size) {
          gtk_adjustment_set_value (verticalAdjust,gtk_adjustment_get_value(verticalAdjust)+PDF_SCROLL_STEP);
  } 
  else {
    if(data->curPDFpage != (data->totalPDFpages) - 1) {
       gtk_adjustment_set_value (verticalAdjust,0);
       PDF_moveForward (parentWindow, data);
    }
  }
}


void PDF_moveBackward (GtkWidget *parentWindow, APP_data *data)
{
  if(!data->doc)
     return;
  if( data->curPDFpage != 0) {
    data->curPDFpage--;
  }
  update_statusbarPDF(data);
  PDF_display_page(parentWindow, data->curPDFpage, data->doc, data);
//  if(data->pdfSearch)
  //   search_draw_selection_current_page(data->curPDFpage, data, data->surface); 

}


void PDF_moveForward (GtkWidget *parentWindow, APP_data    *data)
{
  if(!data->doc)
     return;

  if( data->curPDFpage != (data->totalPDFpages) - 1){
    data->curPDFpage++;
  }
  update_statusbarPDF(data);
  PDF_display_page(parentWindow, data->curPDFpage, data->doc, data);
//  if(data->pdfSearch)
  //   search_draw_selection_current_page(data->curPDFpage, data, data->surface); 
}


void PDF_moveHome (GtkWidget *parentWindow, APP_data *data)
{
  if(!data->doc)
     return;

  data->curPDFpage = 0;
  update_statusbarPDF(data);
  PDF_display_page(parentWindow, 0, data->doc, data);
//  if(data->pdfSearch)
  //   search_draw_selection_current_page(data->curPDFpage, data, data->surface); 

}


void PDF_moveEnd (GtkWidget *parentWindow, APP_data *data)
{
  if(!data->doc)
     return;

  data->curPDFpage = data->totalPDFpages-1;
  update_statusbarPDF(data);
  PDF_display_page(parentWindow, data->curPDFpage, data->doc, data);
//  if(data->pdfSearch)
  //   search_draw_selection_current_page(data->curPDFpage, data, data->surface); 

}

void PDF_goto(GtkWidget *parentWindow, APP_data *data, gint pg)
{
  data->curPDFpage = pg;
  update_statusbarPDF(data);
  PDF_display_page(parentWindow, data->curPDFpage, data->doc, data);
//  if(data->pdfSearch)
  //   search_draw_selection_current_page(data->curPDFpage, data, data->surface); 

}
/*

  Poppler add text annotation
  requires 0.24+ Poppler

*/

void PDF_set_text_annot_selection (gint x, gint y, gint w, gint h, gint pdf_page, PopplerDocument *doc, GtkWidget *win, GtkWidget *sw, APP_data *data)
{
  PopplerPage *page;
  PopplerRectangle selection, rect_annot; /* in original PDF points 4 gdouble bounding rectnagle coord*/
  gdouble width, height, ratio, v_v_adj, v_h_adj;
  gchar *tmpStr;
  GdkRGBA color;   
  GtkWidget *pBtnColor;   
    
  /* we get the current RGBA color */
  pBtnColor=lookup_widget(GTK_WIDGET(data->appWindow), "color_button");
  gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER(pBtnColor), &color); 
  /* we read current adjustments on scrollable window */
  GtkAdjustment *v_adj= gtk_scrolled_window_get_vadjustment (sw);
  GtkAdjustment *h_adj= gtk_scrolled_window_get_hadjustment (sw);
  v_v_adj = gtk_adjustment_get_value (v_adj);
  v_h_adj = gtk_adjustment_get_value (h_adj);
  /* we get a pointer on the current page */
  page = poppler_document_get_page(doc, pdf_page);
  /* we convert surface/Gdk scaled coordinates to PDF coordinates */
  poppler_page_get_size (page, &width, &height);
  ratio = data->PDFratio;/* don't use other computed value !!! 2 aug 2018 */
  /* we translate to PDF coordinates */
  selection.x1= (gdouble) (x+v_h_adj)/ratio;
  selection.y1= (gdouble) (y+v_v_adj)/ratio;
  selection.x2= (gdouble) w+(x+v_h_adj)/ratio;
  selection.y2= (gdouble) h+(y+v_v_adj)/ratio;
  
  rect_annot.x1=selection.x1;
  rect_annot.y1=height-selection.y1;
  rect_annot.x2=selection.x2;
  rect_annot.y2=height-selection.y2;
  /* a nice dialog to set-tup the note */
  tmpStr=dialog_add_text_annotation(data->appWindow, "", data);
  if(tmpStr!=NULL) {
    if(data->undo.annotStr!=NULL)
           g_free(data->undo.annotStr);
    PopplerAnnot *my_annot= poppler_annot_text_new (doc, &rect_annot);
    /* change background color of the annotation icon */
    PopplerColor *my_color=poppler_color_new();
    my_color->red=65535*color.red;
    my_color->green=65535*color.green;
    my_color->blue=65535*color.blue;
    poppler_annot_set_color (my_annot,my_color);
    poppler_annot_set_contents (my_annot,tmpStr);

    /* POPPLER_ANNOT_TEXT_ICON_COMMENT = bubble  */
    poppler_annot_text_set_icon (POPPLER_ANNOT_TEXT(my_annot), POPPLER_ANNOT_TEXT_ICON_COMMENT);
    poppler_annot_text_set_is_open (POPPLER_ANNOT_TEXT(my_annot),FALSE);
   
    poppler_page_add_annot (page,my_annot);
    poppler_color_free (my_color);
    /* undo datas */
    data->undo.annot=my_annot;
    data->undo.PDFpage=data->curPDFpage;
    data->undo.curStack=CURRENT_STACK_PDF;
    undo_push(data->currentStack, OP_SET_TEXT_ANNOT, data);
   }
  g_free(tmpStr);
  g_object_unref(page);
  /* we redraw the page  */
  PDF_display_page (win, pdf_page, doc, data);
 
}

void PDF_set_free_text_annot_selection (gint x, gint y, gint w, gint h, gint pdf_page, PopplerDocument *doc, GtkWidget *win, GtkWidget *sw, APP_data *data)
{
  PopplerPage *page;
  PopplerRectangle selection, rect_annot; /* in original PDF points 4 gdouble bounding rectnagle coord*/
  gdouble width, height, ratio, v_v_adj, v_h_adj;
  GdkRGBA color;   
  GtkWidget *pBtnColor;  
/* attempt to recode */
gchar *appearanceString; 
// AnnotFreeTextQuadding quadding; 
    
  /* we get the current RGBA color */
  pBtnColor=lookup_widget(GTK_WIDGET(data->appWindow), "color_button");
  gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER(pBtnColor), &color); 

  /* we read current adjustments on scrollable window */
  GtkAdjustment *v_adj= gtk_scrolled_window_get_vadjustment (sw);
  GtkAdjustment *h_adj= gtk_scrolled_window_get_hadjustment (sw);
  v_v_adj = gtk_adjustment_get_value (v_adj);
  v_h_adj = gtk_adjustment_get_value (h_adj);
  printf("h-adj=%.2f  v-adj=%.2f\n", v_h_adj, v_v_adj);
  /* we get a pointer on the current page */
  page = poppler_document_get_page(doc, pdf_page);
  /* we convert surface/Gdk scaled coordinates to PDF coordinates */
  poppler_page_get_size (page, &width, &height);
  ratio = misc_get_PDF_ratio(width,  gtk_widget_get_allocated_width (sw));
  /* we translate to PDF coordinates */
  selection.x1= (gdouble) (x+v_h_adj)/ratio;
  selection.y1= (gdouble) (y+v_v_adj)/ratio;
  selection.x2= (gdouble) (x+v_h_adj+w)/ratio;
  selection.y2= (gdouble) (y+v_v_adj+h)/ratio;
printf("coord width=%.2f height pdf=%.2f x1=%2f y1=%.2f x2=%.2f y2=%.2f ratio=%.2f \n", width, height,selection.x1, selection.y1, selection.x2, selection.y2, ratio);

  rect_annot.x1=selection.x1;
  rect_annot.y1=height-selection.y1;
  rect_annot.x2=selection.x2;
  rect_annot.y2=height-selection.y2;

  PopplerAnnot *my_annot = poppler_annot_square_new (doc, &rect_annot);
 /* we read object values */
 
  PopplerColor *my_color=poppler_color_new();
  PopplerColor *my_colorFg=poppler_color_new();
  /* color */
    my_color->red=65535*color.red;
    my_color->green=65535*color.green;
    my_color->blue=65535*color.blue;
    /* external rectangle */
    my_colorFg->red=0;
    my_colorFg->green=65535*color.green;
    my_colorFg->blue=65535*color.blue;
  poppler_annot_set_color (my_annot,my_color);  
//  poppler_annot_square_set_interior_color(my_annot, my_color);
poppler_annot_set_contents (my_annot,"Essais");
  poppler_annot_markup_set_label (my_annot,"Author");
  
  poppler_page_add_annot (page,my_annot);
  poppler_color_free (my_color);
  poppler_color_free (my_colorFg);
  g_object_unref(page);
  /* we redraw the page  */
  PDF_display_page (win, pdf_page, doc, data);
}
/*********************************************
  highlight selection rectangle 
  on PDF page
  requires poppler>=0.26 to highlight !
********************************************/

void PDF_set_highlight_selection (gint x, gint y, gint w, gint h, gint pdf_page, PopplerDocument *doc, GtkWidget *win, GtkWidget *sw, APP_data *data)
{
  PopplerPage *page;
  GArray  *quads_array;
  PopplerRectangle selection, rect_annot; /* in original PDF points 4 gdouble bounding rectnagle coord*/
  gdouble width, height, ratio, v_v_adj, v_h_adj;
  GdkRGBA color;   
  GtkWidget *pBtnColor;  
    
  /* we get the current RGBA color */
  pBtnColor=lookup_widget(GTK_WIDGET(data->appWindow), "color_button");
  gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER(pBtnColor), &color); 

  /* we read current adjustments on scrollable window */
  GtkAdjustment *v_adj= gtk_scrolled_window_get_vadjustment (sw);
  GtkAdjustment *h_adj= gtk_scrolled_window_get_hadjustment (sw);
  v_v_adj = gtk_adjustment_get_value (v_adj);
  v_h_adj = gtk_adjustment_get_value (h_adj);

  /* we get a pointer on the current page */
  page = poppler_document_get_page(doc, pdf_page);
  /* we convert surface/Gdk scaled coordinates to PDF coordinates */
  poppler_page_get_size (page, &width, &height);
  ratio = data->PDFratio;/* don't use other computed value !!! 2 aug 2018 */
  /* we translate to PDF coordinates */
  selection.x1= (gdouble) (x+v_h_adj)/ratio;
  selection.y1= (gdouble) (y+v_v_adj)/ratio;
  selection.x2= (gdouble) (x+v_h_adj+w)/ratio;
  selection.y2= (gdouble) (y+v_v_adj+h)/ratio;
//printf("coord width=%.2f height pdf=%.2f x1=%2f y1=%.2f x2=%.2f y2=%.2f ratio=%.2f \n", width, height,selection.x1, selection.y1, selection.x2, selection.y2, ratio);

  rect_annot.x1=selection.x1;
  rect_annot.y1=height-selection.y1;
  rect_annot.x2=selection.x2;
  rect_annot.y2=height-selection.y2;

  quads_array = pgd_annots_create_quads_array_for_rectangle (&rect_annot);
  PopplerAnnot *my_annot = poppler_annot_text_markup_new_highlight (doc, &rect_annot, quads_array);
  g_array_free (quads_array, TRUE);
  PopplerColor *my_color=poppler_color_new();
  /* color */
    my_color->red=65535*color.red;
    my_color->green=65535*color.green;
    my_color->blue=65535*color.blue;
  poppler_annot_set_color (my_annot,my_color);  
  poppler_page_add_annot (page,my_annot);
  poppler_color_free (my_color);
  g_object_unref(page);
  /* we redraw the page  */
  PDF_display_page (win, pdf_page, doc, data);
  /* undo datas */
  data->undo.annot=my_annot;
  data->undo.PDFpage=data->curPDFpage;
  data->undo.curStack=CURRENT_STACK_PDF;
  if(data->undo.annotStr!=NULL)
           g_free(data->undo.annotStr);
  undo_push(data->currentStack, OP_SET_HIGHLIGHT_ANNOT, data);
}

/*******************************************************

  get text for selection - full local
  entry parameters : x,y,w,h = bounding selection box
  win = application's window
  sw = PDF scrolled window
*******************************************************/

void PDF_get_text_selection (gint x, gint y, gint w, gint h, gint pdf_page, 
                             GtkWidget *sw, APP_data *data)
{
  gchar *tmpStr;
  PopplerPage *page;
  PopplerRectangle selection; /* in original PDF points 4 gdouble bounding rectnagle coord*/
  gdouble ratio, v_v_adj, v_h_adj;
 
  /* we read current adjustments on scrollable window */
  GtkAdjustment *v_adj= gtk_scrolled_window_get_vadjustment (sw);
  GtkAdjustment *h_adj= gtk_scrolled_window_get_hadjustment (sw);
  v_v_adj = gtk_adjustment_get_value (v_adj);
  v_h_adj = gtk_adjustment_get_value (h_adj);
  /* we get a pointer on the current page */
  page = poppler_document_get_page(data->doc, pdf_page);
  /* we convert surface/Gdk scaled coordinates to PDF coordinates */
  ratio = data->PDFratio;/* don't use other computed value !!! 2 aug 2018 */
  /* we translate to PDF coordinates */
  selection.x1= (gdouble) (x+v_h_adj)/ratio;
  selection.y1= (gdouble) (y+v_v_adj)/ratio;
  selection.x2= (gdouble) (x+v_h_adj+w)/ratio;
  selection.y2= (gdouble) (y+v_v_adj+h)/ratio;
//printf("coord width=%.2f height pdf=%.2f x1=%2f y1=%.2f x2=%.2f y2=%.2f ratio=%.2f \n", width, height,selection.x1, selection.y1, selection.x2, selection.y2, ratio);
  tmpStr = poppler_page_get_selected_text(page, POPPLER_SELECTION_GLYPH, &selection);

  /* copy text to ClipBoard */
  GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  gtk_clipboard_set_text (clipboard, tmpStr, -1);
  g_object_unref(page);
  g_free(tmpStr);
}

