#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


/* translations */
#include <libintl.h>
#include <locale.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gstdio.h> /* g_fopen, etc */
#include <poppler.h>
#include "interface.h" /* glade requirement */
#include "support.h" /* glade requirement */
#include "misc.h"
#include "search.h"


/* inspiration source for those routines :
http://www.bravegnu.org/gtktext/x276.html 
thanks !
*/

/* 

 compute next and previous PDF page with hits, 
 jnown the current page


*/

gint search_goto_prev_PDF_page(gint page, APP_data    *data)
{
  GList *l=NULL;
  PDF_search_results *results;
  gint ret=page;

  l=g_list_last(data->pdfSearch);
  for(l;l!=NULL;l=l->prev) {
     results=(PDF_search_results *)l->data;
     if(results->page<page) {
        ret=results->page;
        break;
     }
  }
 return ret;
}





gint search_goto_next_PDF_page(gint page, APP_data    *data)
{
  GList *l=NULL;
  PDF_search_results *results;
  gint ret=page;

  l=data->pdfSearch;
  for(l;l!=NULL;l=l->next) {
     results=(PDF_search_results *)l->data;
     if(results->page>page) {
        ret=results->page;
        break;
     }
  }
 return ret;
}

/*************************************************

  draw a translucent selection rectangle on PDF

***************************************************/
void search_draw_selection_rectangle(cairo_t *cr, APP_data *data, PopplerRectangle *rectangle, gdouble scale, GdkRGBA *color)
{
  /* test */
  gdouble y, w,h;

  w=ABS(rectangle->x1-rectangle->x2);
  h=ABS(rectangle->y1-rectangle->y2);
  y=MIN(rectangle->y1,rectangle->y2);
  y=data->PDFHeight-y;
  cairo_set_source_rgba(cr, color->red, color->green, color->blue, 0.5);
// printf("x1=%.2f y1=%.2f x2=%.2f y2=%.2f \n", rectangle->x1, rectangle->y1, rectangle->x2, rectangle->y2);
  cairo_rectangle(cr, rectangle->x1*scale, y*scale-h*scale, w*scale, h*scale);
  cairo_fill(cr);
  gtk_widget_queue_draw (data->PDFdrawable);
}

/*

 check if current PDF page has hits
 if YES, draw selectino rectangles

*/
void search_draw_selection_current_page(gint page, APP_data *data, cairo_surface_t *surface)
{
  GList *l=NULL, *lr=NULL;
  PDF_search_results *results;
  PopplerRectangle *rectangle;
  cairo_t *cr;
  GtkStyleContext *context;
  GdkRGBA *color;
  /* get current theme' selection color.
     see:  https://stackoverflow.com/questions/47371967/getting-objects-out-of-a-gvalue */
  context=gtk_widget_get_style_context (GTK_WIDGET(data->appWindow));
  gtk_style_context_get (context, GTK_STATE_FLAG_SELECTED, GTK_STYLE_PROPERTY_BACKGROUND_COLOR, &color, NULL);

  cr = cairo_create (surface);
  l=data->pdfSearch;
  for(l;l!=NULL;l=l->next) {
     results=(PDF_search_results *)l->data;
     if(results->page==page) {
       lr=results->hits;
       for(lr;lr!=NULL;lr=lr->next) {
            rectangle=(PopplerRectangle *)lr->data;
            search_draw_selection_rectangle(cr, data, rectangle, data->PDFratio, color);
            
       }
     }
  }
  cairo_destroy (cr);
  gdk_rgba_free (color);
}

/*****************************
 search inside PDF documents
 loaded in memory

****************************/
/*

 search text inside PDF page

*/

GList *search_hits_inside_PDF_page(PopplerPage *Page, const gchar *text)
{
  GList *hits=NULL;

  hits=poppler_page_find_text (Page, text);

  return hits;
}

/*

  search all occurrences for a string
  inside a PDF file
  returns total number of hits
  APP_data are modifyed, all deep search
  datas are in APP_data->pdfSearch structure

*/

gint search_hits_inside_PDF_document(APP_data *data, const gchar *tmpStr)
{
  gint j, ret=0, count=0;
  GList *hits=NULL;

  for(j=0;j<data->totalPDFpages;j++) {    
    hits=search_hits_inside_PDF_page( poppler_document_get_page (data->doc, j), tmpStr);
    if (hits!=NULL) {
        ret = g_list_length (hits); 
        count=count+ret;
        PDF_search_results *results;
        results=g_malloc(sizeof(PDF_search_results));
        results->page=j;
        results->nb_hits_at_page=ret;
        results->hits=hits;
        data->pdfSearch=g_list_append(data->pdfSearch, results);
        // printf("total string found %d times in PDF page %d \n", ret, j);
        /* we must change current page to jump to first occurrence */
        data->curPDFpage=j;
    }
                   
  }/* next j*/
;
  return count;
}
/*

  free memory from previous PDF search results

*/

void search_free_PDF_search_datas(APP_data *data)
{

  GList *l=g_list_first(data->pdfSearch);
  //printf("free Glist !\n");
  for(l;l!=NULL;l=l->next) {
        PDF_search_results *results;
        results=(PDF_search_results *)l->data;
        if(results->hits!=NULL) {
           GList *lr=g_list_first(results->hits);
           //printf("je décanille +%d valeurs\n", g_list_length (results->hits));
           /* we remove all PopplerRectangles */
           for(lr;lr!=NULL;lr=lr->next) {
             PopplerRectangle *rectangle;
             rectangle=(PopplerRectangle *)lr->data;
             //printf("x1=%.2f y1=%.2f x2=%.2f y2=%.2f \n", rectangle->x1, rectangle->y1, rectangle->x2, rectangle->y2);
            if(rectangle!=NULL)
                poppler_rectangle_free (rectangle);
           }
           //printf("fin decanille \n");
           g_list_free (lr);
           
        }
        g_free(results);
  }

  g_list_free (data->pdfSearch);
  data->pdfSearch=NULL;
 // printf("removed datas \n");
}

/******************************

  count matches of text
  in buffer
******************************/
gint search_count_matches(GtkTextBuffer *buffer, const gchar *text)
{
  gint ret=0;
  GtkTextIter mstart, mend;
  GtkTextIter start_match, end_match;
  GtkTextIter start_find, end_find;
  gboolean found;

  gtk_text_buffer_get_start_iter(buffer, &start_find);
  gtk_text_buffer_get_end_iter(buffer, &end_find);

  while (gtk_text_iter_forward_search(&start_find, text, 
        GTK_TEXT_SEARCH_TEXT_ONLY | 
        GTK_TEXT_SEARCH_VISIBLE_ONLY | GTK_TEXT_SEARCH_CASE_INSENSITIVE, 
        &start_match, &end_match, NULL)) {

    ret++;
    gint offset = gtk_text_iter_get_offset(&end_match);
    gtk_text_buffer_get_iter_at_offset(buffer, &start_find, offset);
  }

  return ret;
}
/************************************

  search for a string text from 
  Iter position iter
************************************/

void
find (GtkWidget *view, GtkTextBuffer *buffer, const gchar *text, GtkTextIter *iter, gint direction)
{
  GtkTextIter mstart, mend;
  gboolean found, ok;
  GtkTextMark *last_pos, *prev_last_pos;


  /* remove mark - I don't know if it's really mandatory ... */
  //prev_last_pos = gtk_text_buffer_get_mark (buffer,"last_pos");
  //if(prev_last_pos) {
     //gtk_text_buffer_delete_mark (buffer, prev_last_pos);
  //}

  if(direction==0)
     found = gtk_text_iter_forward_search (iter, text, GTK_TEXT_SEARCH_CASE_INSENSITIVE, &mstart, &mend, NULL);
  else {
     ok=gtk_text_iter_backward_chars(iter, 1);
     found = gtk_text_iter_backward_search (iter, text, GTK_TEXT_SEARCH_CASE_INSENSITIVE, &mstart, &mend, NULL);
     mend = mstart;
     if(found)
         ok = gtk_text_iter_forward_chars(&mend, strlen(text));
  }

  if (found)
    {      
      gtk_text_buffer_select_range (buffer, &mstart, &mend);
      last_pos = gtk_text_buffer_create_mark (buffer, "last_pos",  &mend, FALSE);
      gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(view), last_pos);
    }
}

void
find_next (GtkWidget *view, GtkTextBuffer *buffer, const gchar *text)
{
  GtkTextMark *last_pos;
  GtkTextIter iter;


  last_pos = gtk_text_buffer_get_mark (buffer, "last_pos");
  if (last_pos == NULL)
    return;

  gtk_text_buffer_get_iter_at_mark (buffer, &iter, last_pos);
  find (GTK_TEXT_VIEW (view), buffer, text, &iter, 0);/* 0 == direction forward */
}

void
find_previous (GtkWidget *view, GtkTextBuffer *buffer, const gchar *text)
{
  GtkTextMark *last_pos;
  GtkTextIter iter;


  last_pos = gtk_text_buffer_get_mark (buffer, "last_pos");
  if (last_pos == NULL)
    return;

  gtk_text_buffer_get_iter_at_mark (buffer, &iter, last_pos);
  find (GTK_TEXT_VIEW (view), buffer, text, &iter, 1);/* 0 == direction forward */
}



