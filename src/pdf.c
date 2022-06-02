/***************************
  functions to export files
****************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* translations */
#include <libintl.h>
#include <locale.h>

#include <string.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gstdio.h> /* g_fopen, etc */
#include <poppler.h>
#include "interface.h" /* glade requirement */
#include "support.h" /* glade requirement */
#include "misc.h"
#include "pdf.h"
#include "undo.h"
#include "search.h"

/********************************
  test if there is an annotation 
  for current mouse click
  position

********************************/
PopplerAnnot *PDF_find_annot_at_position (gint x, gint y, APP_data *data)
{
  gdouble ratio, v_v_adj, v_h_adj;
  gint i=0, x1, y1, x2, y2;
  GList *l = NULL;	
  PopplerRectangle selection; /* in original PDF points 4 gdouble bounding rectangle coord*/
  PopplerAnnotMapping *current_annot_map;
  PopplerAnnot *current_annot = NULL;

  if(data->pdfAnnotMapping == NULL) {
    return current_annot;
  }
  /* we read current adjustments on scrollable window */
  GtkAdjustment *v_adj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW(data->PDFScrollable));
  GtkAdjustment *h_adj = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW(data->PDFScrollable));
  v_v_adj = gtk_adjustment_get_value (v_adj);
  v_h_adj = gtk_adjustment_get_value (h_adj);   
  ratio = data->PDFratio;
  /* we translate to PDF coordinates */
  selection.x1 = (gdouble) (x+v_h_adj)/ratio;
  selection.y1 = (gdouble)data->PDFHeight - (y+v_v_adj)/ratio;/* yes PDF inverted vertical coordinates ! */
  /* now we search the first PopplerAnnot with coincident coordinates */
  l = data->pdfAnnotMapping;
  for(l;l!=NULL;l=l->next) {
    current_annot_map = (PopplerAnnotMapping *)l->data;
 //  printf("annot=%d coordonneés x1=%.2f y1=%.2f x2=%.2f y2=%.2f souris x1=%.2f y1=%.2f \n", i+1, current_annot_map->area.x1, 
   //           current_annot_map->area.y1, current_annot_map->area.x2, current_annot_map->area.y2, selection.x1,selection.y1);

    if(selection.x1 >= current_annot_map->area.x1 && selection.y1 >= current_annot_map->area.y1 
       && selection.x1 <= current_annot_map->area.x2 && selection.y1 <= current_annot_map->area.y2) {
        // printf("bingo ! coinc for annot =%d annotype =%d \n", i+1, poppler_annot_get_annot_type (current_annot_map->annot));
        current_annot = current_annot_map->annot;
        data->x1 = (gint)current_annot_map->area.x1;
        data->y1 = (gint)current_annot_map->area.y1;
        data->x2 = (gint)current_annot_map->area.x2;
        data->y2 = (gint)current_annot_map->area.y2;
    }
    i++;
  }
   
  return current_annot;
}

/*************************************************
  get the annot mapping for the current page
  when this function is called,
  the 'data" structure contains 
  all necessary datas : current page,
  a pointer on PDF, doc, and so son
*************************************************/
GList *PDF_get_annot_mapping (APP_data *data)
{
  PopplerPage *pPage;

  if(data->doc == NULL) {
    return NULL;
  }
  pPage = poppler_document_get_page (data->doc, data->curPDFpage);
  if(pPage == NULL) {
    return NULL;
  }  
  return poppler_page_get_annot_mapping (pPage);
}

/*****************************************
  display a PDF, autosized to allow 
  scrolling of gtk_drawing_area
******************************************/
void PDF_display_page (GtkWidget *window1, gint page, PopplerDocument *pdoc, APP_data *data)
{
  GKeyFile *keyString;
  GdkRGBA color; 
  gint w, h;
  gdouble width, height, ratio;
  cairo_t *cr;
  PopplerPage *pPage;
  GtkWidget *canvas;

  keyString = g_object_get_data (G_OBJECT(data->appWindow), "config");
  color.red = g_key_file_get_double (keyString, "reference-document", "paper.color.red", NULL);
  color.green = g_key_file_get_double (keyString, "reference-document", "paper.color.green", NULL);
  color.blue = g_key_file_get_double (keyString, "reference-document", "paper.color.blue", NULL);
  color.alpha = 1;

  canvas = lookup_widget(GTK_WIDGET(window1), "crPDF");
  pPage = poppler_document_get_page (pdoc, page);
  poppler_page_get_size (pPage, &width, &height);
 
  //g_key_file_set_double(keyString, "reference-document ", "zoom", data->PDFratio);
  ratio = data->PDFratio;
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
  cairo_set_source_rgb (cr, color.red, color.green, color.blue);
  cairo_rectangle (cr, 0, 0, w, h);
  cairo_fill (cr);
  // cairo_set_source_surface(cr, data->surface, 30, 30);
  cairo_scale (cr, ratio, ratio);  
  poppler_page_render (pPage, cr);  
  cairo_destroy (cr);
  gtk_widget_set_size_request (canvas, w, h);
  gtk_widget_queue_draw (canvas);
  g_object_unref (pPage);
  if (data->pdfSearch) {/* special for Manjaro ;-) */
     search_draw_selection_current_page (data->curPDFpage, data, data->surface); 
  }
}

/*******************************
  various functions to navigate
  the PDF document
*********************************/
void PDF_moveUp (GtkWidget *parentWindow, APP_data *data)
{
  GtkAdjustment *verticalAdjust;
  gdouble adj_value;
  gdouble upper = 0;
  gdouble page_size = 0;

  if(!data->doc) {
     return;
  }
  verticalAdjust = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW(data->PDFScrollable));
  adj_value = gtk_adjustment_get_value (verticalAdjust);
  upper     = gtk_adjustment_get_upper (verticalAdjust);
  page_size = gtk_adjustment_get_page_size (verticalAdjust);
  if(adj_value>0) {
     gtk_adjustment_set_value (verticalAdjust,adj_value - PDF_SCROLL_STEP);
  }
  else {
    if(data->curPDFpage > 0) {
      gtk_adjustment_set_value (verticalAdjust, upper - PDF_SCROLL_STEP);
      PDF_moveBackward (parentWindow, data);
    }
  }
}

/* ********************************

see : https://stackoverflow.com/questions/11353184/gtk-programmatically-scroll-back-a-single-line-in-scrolled-window-containing-a 

***********************************/

void PDF_moveDown (GtkWidget *parentWindow, APP_data *data)
{
  GtkAdjustment *verticalAdjust;
  gdouble upper = 0;
  gdouble page_size = 0;
  gdouble adj_value;

  if(!data->doc) {
     return;
  }
  verticalAdjust = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW(data->PDFScrollable));
  upper     = gtk_adjustment_get_upper (verticalAdjust);
  adj_value = gtk_adjustment_get_value (verticalAdjust);
  page_size = gtk_adjustment_get_page_size (verticalAdjust);

  if(adj_value < upper - page_size) {
          gtk_adjustment_set_value (verticalAdjust, gtk_adjustment_get_value (verticalAdjust) + PDF_SCROLL_STEP);
  } 
  else {
    if(data->curPDFpage != (data->totalPDFpages) - 1) {
       gtk_adjustment_set_value (verticalAdjust, 0);
       PDF_moveForward (parentWindow, data);
    }
  }
}


void PDF_moveBackward (GtkWidget *parentWindow, APP_data *data)
{
  if(!data->doc) {
     return;
  }
  if(data->curPDFpage != 0) {
    data->curPDFpage--;
  }
  update_statusbarPDF (data);
  PDF_display_page (parentWindow, data->curPDFpage, data->doc, data);
//  if(data->pdfSearch)
  //   search_draw_selection_current_page(data->curPDFpage, data, data->surface); 

}


void PDF_moveForward (GtkWidget *parentWindow, APP_data    *data)
{
  if(!data->doc) {
     return;
  }
  if(data->curPDFpage != (data->totalPDFpages) - 1){
    data->curPDFpage++;
  }
  update_statusbarPDF (data);
  PDF_display_page (parentWindow, data->curPDFpage, data->doc, data);
//  if(data->pdfSearch)
  //   search_draw_selection_current_page(data->curPDFpage, data, data->surface); 
}


void PDF_moveHome (GtkWidget *parentWindow, APP_data *data)
{
  if(!data->doc) {
     return;
  }
  data->curPDFpage = 0;
  update_statusbarPDF (data);
  PDF_display_page (parentWindow, 0, data->doc, data);
//  if(data->pdfSearch)
  //   search_draw_selection_current_page(data->curPDFpage, data, data->surface); 

}


void PDF_moveEnd (GtkWidget *parentWindow, APP_data *data)
{
  if(!data->doc) {
     return;
  }
  data->curPDFpage = data->totalPDFpages - 1;
  update_statusbarPDF (data);
  PDF_display_page (parentWindow, data->curPDFpage, data->doc, data);
//  if(data->pdfSearch)
  //   search_draw_selection_current_page(data->curPDFpage, data, data->surface); 

}

void PDF_goto (GtkWidget *parentWindow, APP_data *data, gint pg)
{
  data->curPDFpage = pg;
  update_statusbarPDF (data);
  PDF_display_page (parentWindow, data->curPDFpage, data->doc, data);
//  if(data->pdfSearch)
  //   search_draw_selection_current_page(data->curPDFpage, data, data->surface); 

}

/*********************************

  Poppler add text annotation
  requires 0.24+ of Poppler lib

********************************/

void PDF_set_text_annot_selection (gint x, gint y, gint w, gint h, gint pdf_page, 
                                   PopplerDocument *doc, GtkWidget *win, GtkWidget *sw, 
                                   APP_data *data)
{
  PopplerPage *page;
  PopplerRectangle selection, rect_annot; /* in original PDF points 4 gdouble bounding rectnagle coord*/
  gdouble width, height, ratio, v_v_adj, v_h_adj;
  gchar *tmpStr = NULL;
  GdkRGBA color;   
  GtkWidget *pBtnColor;   
    
  /* we get the current RGBA color */
  pBtnColor = lookup_widget (GTK_WIDGET(data->appWindow), "color_button");
  gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER(pBtnColor), &color); 
  /* we read current adjustments on scrollable window */
  GtkAdjustment *v_adj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW(sw));
  GtkAdjustment *h_adj = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW(sw));
  v_v_adj = gtk_adjustment_get_value (v_adj);
  v_h_adj = gtk_adjustment_get_value (h_adj);
  /* we get a pointer on the current page */
  page = poppler_document_get_page (doc, pdf_page);
  /* we convert surface/Gdk scaled coordinates to PDF coordinates */
  poppler_page_get_size (page, &width, &height);
  ratio = data->PDFratio;/* don't use other computed value !!! 2 aug 2018 */
  /* we translate to PDF coordinates */
  selection.x1 = (gdouble) (x+v_h_adj) / ratio;
  selection.y1 = (gdouble) (y+v_v_adj) / ratio;
  selection.x2 = (gdouble) w+(x+v_h_adj) / ratio;
  selection.y2 = (gdouble) h+(y+v_v_adj) / ratio;
  
  rect_annot.x1 = selection.x1;
  rect_annot.y1 = height - selection.y1;
  rect_annot.x2 = selection.x2;
  rect_annot.y2 = height - selection.y2;
  /* a nice dialog to set-tup the note */
  tmpStr = dialog_add_text_annotation (data->appWindow, "", data);
  if(tmpStr!=NULL) {
    if(data->undo.annotStr != NULL) {
           g_free (data->undo.annotStr);
    }
    PopplerAnnot *my_annot= poppler_annot_text_new (doc, &rect_annot);
    /* change background color of the annotation icon */
    PopplerColor *my_color = poppler_color_new ();
    my_color->red   = 65535*color.red;
    my_color->green = 65535*color.green;
    my_color->blue  = 65535*color.blue;
    poppler_annot_set_color (my_annot, my_color);
    poppler_annot_set_contents (my_annot, tmpStr);

    /* POPPLER_ANNOT_TEXT_ICON_COMMENT = bubble  */
    poppler_annot_text_set_icon (POPPLER_ANNOT_TEXT(my_annot), POPPLER_ANNOT_TEXT_ICON_COMMENT);
    poppler_annot_text_set_is_open (POPPLER_ANNOT_TEXT(my_annot), FALSE);
   
    poppler_page_add_annot (page,my_annot);
    poppler_color_free (my_color);

    /* undo datas */
    data->undo.PDFpage  = data->curPDFpage;
    data->undo.curStack = CURRENT_STACK_PDF;
    undo_push (data->currentStack, OP_SET_TEXT_ANNOT, my_annot, data);
   }
  g_free (tmpStr);
  g_object_unref (page);
  /* we redraw the page  */
  PDF_display_page (win, pdf_page, doc, data);
 
}

/**********************************************
 * never officially implemented despite job
 * done by a student at GSOC :(
 * 
 * ******************************************/
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
  pBtnColor = lookup_widget(GTK_WIDGET(data->appWindow), "color_button");
  gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER(pBtnColor), &color); 

  /* we read current adjustments on scrollable window */
  GtkAdjustment *v_adj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW(sw));
  GtkAdjustment *h_adj = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW(sw));
  v_v_adj = gtk_adjustment_get_value (v_adj);
  v_h_adj = gtk_adjustment_get_value (h_adj);
  printf("h-adj=%.2f  v-adj=%.2f\n", v_h_adj, v_v_adj);
  /* we get a pointer on the current page */
  page = poppler_document_get_page (doc, pdf_page);
  /* we convert surface/Gdk scaled coordinates to PDF coordinates */
  poppler_page_get_size (page, &width, &height);
  ratio = misc_get_PDF_ratio(width,  gtk_widget_get_allocated_width (sw));
  /* we translate to PDF coordinates */
  selection.x1 = (gdouble) (x+v_h_adj)/ratio;
  selection.y1 = (gdouble) (y+v_v_adj)/ratio;
  selection.x2 = (gdouble) (x+v_h_adj+w)/ratio;
  selection.y2 = (gdouble) (y+v_v_adj+h)/ratio;
printf("coord width=%.2f height pdf=%.2f x1=%2f y1=%.2f x2=%.2f y2=%.2f ratio=%.2f \n", width, height,selection.x1, selection.y1, selection.x2, selection.y2, ratio);

  rect_annot.x1 = selection.x1;
  rect_annot.y1 = height-selection.y1;
  rect_annot.x2 = selection.x2;
  rect_annot.y2 = height-selection.y2;

  PopplerAnnot *my_annot = poppler_annot_square_new (doc, &rect_annot);
 /* we read object values */
 
  PopplerColor *my_color   = poppler_color_new();
  PopplerColor *my_colorFg = poppler_color_new();
  /* color */
  my_color->red   = 65535*color.red;
  my_color->green = 65535*color.green;
  my_color->blue  = 65535*color.blue;
  /* external rectangle */
  my_colorFg->red   = 0;
  my_colorFg->green = 65535*color.green;
  my_colorFg->blue  = 65535*color.blue;
  poppler_annot_set_color (my_annot,my_color);  
//  poppler_annot_square_set_interior_color(my_annot, my_color);
  poppler_annot_set_contents (my_annot,"Essais");
  poppler_annot_markup_set_label (POPPLER_ANNOT_MARKUP(my_annot),"Author");
  
  poppler_page_add_annot (page,my_annot);
  poppler_color_free (my_color);
  poppler_color_free (my_colorFg);
  g_object_unref (page);
  /* we redraw the page  */
  PDF_display_page (win, pdf_page, doc, data);
}

/*********************************************
  highlight linear selection   on PDF page
  requires poppler>=0.26 to highlight !
********************************************/

void PDF_set_highlight_linear_selection 
                    (gint x, gint y, gint w, gint h, gint pdf_page, 
                    PopplerDocument *doc, GtkWidget *win, GtkWidget *sw, APP_data *data)
{
  PopplerPage *page;
  GArray  *quads_array;
  PopplerRectangle selection, rect_annot; /* in original PDF points 4 gdouble bounding rectnagle coord*/
  gdouble width, height, ratio, v_v_adj, v_h_adj;
  gint i, undo_index;
  guint n_rects, rectCount;  
  GdkRGBA color;   
  GtkWidget *pBtnColor, *canvas;  

  cairo_t *cr;
  
  /* linear highlight */
  PopplerRectangle *pRects, sel, rectMem;
  PopplerRectangle *tRects = NULL;
  PopplerAnnot *my_annot;

    
  /* we get the current RGBA color */
  pBtnColor = lookup_widget (GTK_WIDGET(data->appWindow), "color_button");
  gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER(pBtnColor), &color); 
  
  /* we get a pointer on the current page */
  page = poppler_document_get_page (doc, pdf_page);
  /* we convert surface/Gdk scaled coordinates to PDF coordinates */
  poppler_page_get_size (page, &width, &height);
  /* get cairo context in order to draw selection */
  canvas = lookup_widget (GTK_WIDGET(data->appWindow), "crPDF");   
  ratio = data->PDFratio;/* don't use other computed value !!! 2 aug 2018 */
  cr = cairo_create (data->surface);
  cairo_scale (cr, ratio, ratio); /* mandatory don't remove !!!! 25 may 2022 */  
  
  /* we read current adjustments on scrollable window */
  GtkAdjustment *v_adj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (sw));
  GtkAdjustment *h_adj = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (sw));
  v_v_adj = gtk_adjustment_get_value (v_adj);
  v_h_adj = gtk_adjustment_get_value (h_adj);
  
  printf ("linear highlight ratio =%.2f \n", ratio);
  /* we translate to PDF coordinates */
  selection.x1 = (gdouble) (x+v_h_adj) / ratio;
  selection.y1 = (gdouble) (y+v_v_adj) / ratio;
  selection.x2 = (gdouble) (x+v_h_adj+w) / ratio;
  selection.y2 = (gdouble) (y+v_v_adj+h) / ratio;
  
  /* we remove any selection from page */
  cairo_region_t *region =  poppler_page_get_selected_region (page,
                                  ratio,
                                  POPPLER_SELECTION_GLYPH,
                                  &selection);
                                  
  cairo_region_destroy (region); 
 // PDF_display_page (data->appWindow, pdf_page, data->doc, data); /* mandatory in order to update display of removed selection */
 // gtk_widget_queue_draw (canvas);
  
  /* we are looking for the rectangles in the selected area */
  poppler_page_get_text_layout_for_area (page, &selection, &tRects, &n_rects);
  printf("nbre rect selection  = %d \n", n_rects ); 
  
  if(n_rects>0) {
	  /* we store useful values for first popplerrectangle */
	  rectMem = tRects[0];
	  printf ("mémorisé position 0 x1 = %.2f y1 =%.2f x2= %.2f y2=%.2f \n", rectMem.x1, rectMem.y1, rectMem.x2, rectMem.y2);
	                   
	  /* we draw all sub -rectangles of selected area  */
	  i = 0;
	  undo_index = 0;
	  while (i<n_rects) {
		 sel = tRects[i];/* contient TOUs les rectangles de la page */
		 sel.x1 = tRects[i].x1;/* datas are ALREADY scaled and shift, don't modify values ! 31/5/2022 */
		 sel.y1 = tRects[i].y1;
		 sel.x2 = tRects[i].x2;
		 sel.y2 = tRects[i].y2;	 
		/*bug : si o fait scroller la page, le surlignement n'est plus à sa place alors que la sélection simple marche */ 
         if(sel.y1>rectMem.y1) {
				  
			  rect_annot.x1 = rectMem.x1;
			  rect_annot.y1 = height - rectMem.y1;
			  rect_annot.x2 = sel.x2;
			  rect_annot.y2 = height - rectMem.y2;     

			  quads_array = pgd_annots_create_quads_array_for_rectangle (&rect_annot);
			  my_annot = poppler_annot_text_markup_new_highlight (doc, &rect_annot, quads_array);
			  g_array_free (quads_array, TRUE);
			  PopplerColor *my_color = poppler_color_new ();
			  /* color */
			  my_color->red   = 65535*color.red;
			  my_color->green = 65535*color.green;
			  my_color->blue  = 65535*color.blue;
			  poppler_annot_set_color (my_annot, my_color);  
			  poppler_page_add_annot (page, my_annot);
			  poppler_color_free (my_color);
			  /* we store useful values */
			  rectMem = tRects[i];
			  printf ("tintin \n");
			  /* undo datas */
			  undo_index++;			   
			  data->undo.undo_index = undo_index;
			  data->undo.PDFpage  = data->curPDFpage;
			  data->undo.curStack = CURRENT_STACK_PDF;
			  if(data->undo.annotStr != NULL) {
					   g_free (data->undo.annotStr);
			  }
			  undo_push (data->currentStack, OP_SET_HIGHLIGHT_ANNOT, my_annot, data);
			  
         }/* endif EOL change*/
		 i++;
		 if(i>=n_rects) {/* there is probably a line pending, so we flush the line */
			 printf ("coucou \n");
			  rect_annot.x1 = rectMem.x1;
			  rect_annot.y1 = height - rectMem.y1;
			  rect_annot.x2 = sel.x2;
			  rect_annot.y2 = height - rectMem.y2;     

			  quads_array = pgd_annots_create_quads_array_for_rectangle (&rect_annot);
			  my_annot = poppler_annot_text_markup_new_highlight (doc, &rect_annot, quads_array);
			  g_array_free (quads_array, TRUE);
			  PopplerColor *my_color = poppler_color_new ();
			  /* color */
			  my_color->red   = 65535*color.red;
			  my_color->green = 65535*color.green;
			  my_color->blue  = 65535*color.blue;
			  poppler_annot_set_color (my_annot, my_color);  
			  poppler_page_add_annot (page, my_annot);
			  poppler_color_free (my_color);	
			  
			  /* undo datas */
			  if(undo_index<1) {
				  undo_index = 1;
			  }
			  data->undo.undo_index = undo_index;			  
			  data->undo.PDFpage  = data->curPDFpage;
			  data->undo.curStack = CURRENT_STACK_PDF;
			  if(data->undo.annotStr != NULL) {
					   g_free (data->undo.annotStr);
			  }
			  undo_push (data->currentStack, OP_SET_HIGHLIGHT_ANNOT, my_annot, data);
			  	printf ("j'ai empîlé avec un index de :%d\n", undo_index);		  		 
		 }     
	  }/* wend */
	printf ("undo index vaut : %d \n", undo_index);  
	  
	  PDF_display_page (data->appWindow, pdf_page, data->doc, data); /* mandatory in order to update display of removed selection */
	  /* we redraw the page  */
	  gtk_widget_queue_draw (canvas);
	  // gtk_widget_queue_draw (data->PDFdrawable);
	  cairo_destroy (cr);
	  g_object_unref (page);
	  g_free (pRects);
	  g_free (tRects);  

	//  undo_push (data->currentStack, OP_SET_HIGHLIGHT_ANNOT, data);
  }/* endif n_rects */
}

/*********************************************
  highlight selection rectangle on PDF page
  requires poppler>=0.26 to highlight !
********************************************/

void PDF_set_highlight_selection (gint x, gint y, gint w, gint h, gint pdf_page, 
                                  PopplerDocument *doc, GtkWidget *win, GtkWidget *sw, 
                                  APP_data *data)
{
  PopplerPage *page;
  GArray  *quads_array;
  PopplerRectangle selection, rect_annot; /* in original PDF points 4 gdouble bounding rectnagle coord*/
  gdouble width, height, ratio, v_v_adj, v_h_adj;
  GdkRGBA color;   
  GtkWidget *pBtnColor;  
    
  /* we get the current RGBA color */
  pBtnColor = lookup_widget (GTK_WIDGET(data->appWindow), "color_button");
  gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER(pBtnColor), &color); 

  /* we read current adjustments on scrollable window */
  GtkAdjustment *v_adj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (sw));
  GtkAdjustment *h_adj = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (sw));
  v_v_adj = gtk_adjustment_get_value (v_adj);
  v_h_adj = gtk_adjustment_get_value (h_adj);

  /* we get a pointer on the current page */
  page = poppler_document_get_page (doc, pdf_page);
  /* we convert surface/Gdk scaled coordinates to PDF coordinates */
  poppler_page_get_size (page, &width, &height);
  ratio = data->PDFratio;/* don't use other computed value !!! 2 aug 2018 */

  /* we translate to PDF coordinates */
  selection.x1 = (gdouble) (x+v_h_adj) / ratio;
  selection.y1 = (gdouble) (y+v_v_adj) / ratio;
  selection.x2 = (gdouble) (x+v_h_adj+w) / ratio;
  selection.y2 = (gdouble) (y+v_v_adj+h) / ratio;  
    
//printf("coord width=%.2f height pdf=%.2f x1=%2f y1=%.2f x2=%.2f y2=%.2f ratio=%.2f \n", width, height,selection.x1, selection.y1, selection.x2, selection.y2, ratio);

  rect_annot.x1 = selection.x1;
  rect_annot.y1 = height - selection.y1;
  rect_annot.x2 = selection.x2;
  rect_annot.y2 = height - selection.y2;

  quads_array = pgd_annots_create_quads_array_for_rectangle (&rect_annot);
  PopplerAnnot *my_annot = poppler_annot_text_markup_new_highlight (doc, &rect_annot, quads_array);
  g_array_free (quads_array, TRUE);
  PopplerColor *my_color = poppler_color_new ();
  /* color */
  my_color->red   = 65535*color.red;
  my_color->green = 65535*color.green;
  my_color->blue  = 65535*color.blue;
  poppler_annot_set_color (my_annot, my_color);  
  poppler_page_add_annot (page, my_annot);
  poppler_color_free (my_color);
  g_object_unref (page);
  /* we redraw the page  */
  PDF_display_page (win, pdf_page, doc, data);
  
  /* undo datas */
  data->undo.annot    = my_annot;
  data->undo.PDFpage  = data->curPDFpage;
  data->undo.curStack = CURRENT_STACK_PDF;
  if(data->undo.annotStr != NULL) {
           g_free (data->undo.annotStr);
  }
  undo_push (data->currentStack, OP_SET_HIGHLIGHT_RECTANGLE_ANNOT, my_annot, data);
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
  gchar *tmpStr = NULL;
  PopplerPage *page;
  PopplerRectangle selection, sel; /* in original PDF points 4 gdouble bounding rectnagle coord*/
  /* for page text layouts */ 
  PopplerRectangle *pRects;

  guint rectCount;
  gdouble width, height, ratio, v_v_adj, v_h_adj;  

  cairo_t *cr;

  GtkWidget *canvas;
  GtkStyleContext *context;
  GdkRGBA *color, *fColor;
  
  /* get current theme' selection color. see:  https://stackoverflow.com/questions/47371967/getting-objects-out-of-a-gvalue */
  context = gtk_widget_get_style_context (GTK_WIDGET(data->appWindow));
  gtk_style_context_get (context, GTK_STATE_FLAG_SELECTED, GTK_STYLE_PROPERTY_BACKGROUND_COLOR, &color, GTK_STYLE_PROPERTY_COLOR, &fColor, NULL);  

  /* we get a pointer on the current page */
  page = poppler_document_get_page (data->doc, pdf_page);
  poppler_page_get_size (page, &width, &height); 
  
  /* get cairo context in order to draw selection */
  canvas = lookup_widget (GTK_WIDGET(data->appWindow), "crPDF");
  ratio = data->PDFratio;

  cr = cairo_create (data->surface);
  cairo_scale (cr, ratio, ratio); /* mandatory don't remove !!!! 25 may 2022 */
  
  /* we read current adjustments on scrollable window */
  GtkAdjustment *v_adj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW(sw));
  GtkAdjustment *h_adj = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW(sw));
  v_v_adj = gtk_adjustment_get_value (v_adj);
  v_h_adj = gtk_adjustment_get_value (h_adj);

  /* we convert surface/Gdk scaled coordinates to PDF coordinates */
  /* we translate to PDF coordinates */
  selection.x1 = (gdouble) (x + v_h_adj) / ratio;
  selection.y1 = (gdouble) (y + v_v_adj) / ratio;
  selection.x2 = (gdouble) (x + v_h_adj + w) / ratio;
  selection.y2 = (gdouble) (y + v_v_adj + h) / ratio;
   
   /* reste un bug : si tu sélectionnes hors-zone, il choisit le texte le plus proche !!! 31/5/2022 */
  
printf("coord width=%d height =%.d x1=%2f y1=%.2f x2=%.2f y2=%.2f ratio=%.2f \n", w, h,selection.x1, selection.y1, selection.x2, selection.y2, ratio);

  GtkWidget *labelClipBoard = GTK_WIDGET (gtk_builder_get_object (data->builder, "labelClipBoard"));
  tmpStr = poppler_page_get_selected_text (page, POPPLER_SELECTION_GLYPH, &selection);

  /* we remove any selection from page */
  cairo_region_t *region =  poppler_page_get_selected_region (page,
                                  ratio,
                                  POPPLER_SELECTION_GLYPH,
                                  &selection);
                                  
  cairo_region_destroy (region); 
  PDF_display_page (data->appWindow, pdf_page, data->doc, data); /* mandatory in order to update display of removed selection */                  
                               
  /* remove cairo context */
  cairo_destroy (cr);
  g_object_unref (page);

  /* copy text to ClipBoard */

  if(tmpStr) {
	if(strlen (tmpStr) > 0) {
	     /*  Sets the contents of the clipboard to the given UTF-8 string. 
	      * GTK+ will make a copy of the text and take responsibility for 
	      * responding for requests for the text, and for converting the text 
	      * into the requested format.*/
	     
	     gtk_clipboard_set_text (gtk_clipboard_get (GDK_SELECTION_CLIPBOARD), tmpStr, -1);// GDK_SELECTION_PRIMARY in order to avoid gtk_hash_table issues when Redac quits
	     misc_display_clipboard_text_info ((const gchar *) tmpStr, data);
	     g_free (tmpStr);/* OK, extracted text can be freed */
	}
	else {
	  /* length of string == 0 */	
	  gtk_clipboard_set_text (gtk_clipboard_get (GDK_SELECTION_CLIPBOARD), "", -1);
	  gtk_label_set_text (GTK_LABEL(labelClipBoard), _("---"));
    }

  }
  else {
	/* selection empty */
	gtk_clipboard_set_text (gtk_clipboard_get (GDK_SELECTION_CLIPBOARD), "", -1);
	gtk_label_set_text (GTK_LABEL(labelClipBoard), _("---"));
  }
  
}

/***************************************
  
  zoom IN PDF
  
***************************************/
void on_PDF_zoom_in_clicked (GtkButton *button, APP_data *data)
{
  GKeyFile *keyString;

  keyString = g_object_get_data (G_OBJECT(data->appWindow), "config");

  if(data->doc) {
     data->PDFratio = data->PDFratio * 1.1;
     if(data->PDFratio * data->PDFWidth > PDF_VIEW_MAX_WIDTH) {
         data->PDFratio = PDF_VIEW_MAX_WIDTH / data->PDFWidth;
     }
     PDF_display_page (data->appWindow, data->curPDFpage, data->doc, data);
     g_key_file_set_double (keyString, "reference-document", "zoom", data->PDFratio);
  }
}

/*************************
 
  zoom OUT PDF
 
*************************/
void on_PDF_zoom_out_clicked  (GtkButton *button, APP_data *data)
{
  GKeyFile *keyString;

  keyString = g_object_get_data (G_OBJECT(data->appWindow), "config");

  if(data->doc) {
     data->PDFratio = data->PDFratio * 0.9;
     if(data->PDFratio < 0.5) {
         data->PDFratio = 0.5;
     }
     PDF_display_page (data->appWindow, data->curPDFpage, data->doc, data);
     g_key_file_set_double (keyString, "reference-document", "zoom", data->PDFratio);
  }
}

/*********************************
 
  prepare PDF drawable

*********************************/
GtkWidget *PDF_prepare_drawable ()
{
  GtkWidget *crPDF;

  crPDF = gtk_drawing_area_new ();
  gtk_widget_show (crPDF);

  gtk_widget_set_size_request (crPDF, PDF_VIEW_MAX_WIDTH, PDF_VIEW_MAX_HEIGHT);
  gtk_widget_set_hexpand (crPDF, TRUE);
  gtk_widget_set_vexpand (crPDF, TRUE);
  /* mandatoty : add new events management to gtk_drawing_area ! */
      gtk_widget_set_events (crPDF, gtk_widget_get_events (crPDF)
      | GDK_BUTTON_PRESS_MASK
      | GDK_BUTTON_RELEASE_MASK
      | GDK_POINTER_MOTION_MASK
      | GDK_POINTER_MOTION_HINT_MASK);
  return crPDF;
}

