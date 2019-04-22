#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <stdlib.h>
/* translations */
#include <libintl.h>
#include <locale.h>

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <poppler.h>
#include <gtkspell/gtkspell.h>
#include "support.h"
#include "misc.h"
#include "undo.h"
/*

  functions from Poppler glib demo

*/
static inline void
pgd_annots_set_poppler_quad_from_rectangle (PopplerQuadrilateral *quad,
                                            PopplerRectangle     *rect)
{
    quad->p1.x = rect->x1;
    quad->p1.y = rect->y1;
    quad->p2.x = rect->x2;
    quad->p2.y = rect->y1;
    quad->p3.x = rect->x1;
    quad->p3.y = rect->y2;
    quad->p4.x = rect->x2;
    quad->p4.y = rect->y2;
}


GArray *
pgd_annots_create_quads_array_for_rectangle (PopplerRectangle *rect)
{
    GArray *quads_array;
    PopplerQuadrilateral *quad;

    quads_array = g_array_sized_new (FALSE, FALSE, sizeof (PopplerQuadrilateral), 1);
    g_array_set_size (quads_array, 1);

    quad = &g_array_index (quads_array, PopplerQuadrilateral, 0);
    pgd_annots_set_poppler_quad_from_rectangle (quad, rect);

    return quads_array;
}



/****************************************
  function to get the current alignment
  and setup the radiobuttons
 full local
*****************************************/
gint misc_get_current_alignment(GtkTextBuffer *buffer)
{
  gint retval = KW_ALIGNMENT_LEFT;
  GtkTextIter start, end, iter;
  GtkTextTag *tag;
  GtkTextTagTable *tagTable1;
  gint row, col, total;
  
  gtk_text_buffer_get_iter_at_mark(buffer, &iter, gtk_text_buffer_get_insert(buffer));

  row = gtk_text_iter_get_line(&iter);
  col = gtk_text_iter_get_line_offset(&iter);
  total = gtk_text_buffer_get_line_count(buffer);

  tagTable1 = gtk_text_buffer_get_tag_table(buffer);
 
  /* now we apply the tag */
  gtk_text_buffer_get_iter_at_line (buffer,&start,row);
  if(total>row) {
    gtk_text_buffer_get_iter_at_line (buffer,&end,row+1);
  }
  else {
     gtk_text_buffer_get_end_iter (buffer, &end);
  }

  if( get_tag_in_selection("left", start))
     return KW_ALIGNMENT_LEFT;
  if( get_tag_in_selection("center", start))
     return KW_ALIGNMENT_CENTER;
  if( get_tag_in_selection("right", start))
     return KW_ALIGNMENT_RIGHT;
  if( get_tag_in_selection("fill", start))
     return KW_ALIGNMENT_FILL;
  return retval;
}

/************************************
  invert colors for a RGB pixbuf
  the gchar *pixel works as a POKE()
 to the pointer *pb to the true pixbuf 
source : http://gtkbook.sourceforge.net/improc/node16.html
***********************************/
void misc_colorinvert_picture(GdkPixbuf *pb)
{ 
  gint ht,wt;
  gint i=0,j=0, rowstride=0;  
  gint bpp=0;
  gchar *pixel;


  if(gdk_pixbuf_get_bits_per_sample(pb)!=8)   //we handle only 24 bit images.
  	return;                               //look at 3 bytes per pixel.

  bpp=3;	         	  //getting attributes of height,
  ht=gdk_pixbuf_get_height(pb);   //width, and bpp.Also get pointer
  wt=gdk_pixbuf_get_width(pb);	  //to pixels.
  pixel=gdk_pixbuf_get_pixels(pb);
  rowstride=wt*bpp;

  for(i=0;i<ht;i++)		//iterate over the height of image.
    for(j=0;j<rowstride;j+=bpp)   //read every pixel in the row.skip
				//bpp bytes 
      {	
      
      //access pixel[i][j] as
      // pixel[i*rowstride + j]

      //access red,green and blue as
	pixel[i*rowstride + j+0]=255-pixel[i*rowstride + j+0];
	pixel[i*rowstride + j+1]=255-pixel[i*rowstride + j+1];
	pixel[i*rowstride + j+2]=255-pixel[i*rowstride + j+2];	      
      }  
  return;
}

/*
  activate/deactivate buttons in PDF stack
entry : window1 == mainWindow

*/
void misc_set_gui_in_PDF_mode(GtkWidget *window1, gint prevStack)
{
  GtkTextIter iter;
  GtkWidget *pSearchEntry=lookup_widget(window1, "search_entry"); 
  GtkWidget *pNext=lookup_widget(window1, "buttonNextOccurrence");   
  GtkWidget *pPrev=lookup_widget(window1, "buttonPrevOccurrence");   

  /* redio tool buttons upper toolbar */
  GtkWidget *pButtonText= lookup_widget(window1, "pRadioButtonTextSelect");
  GtkWidget *pButtonPict= lookup_widget(window1, "pRadioButtonPictureSelect");
  GtkWidget *pButtonHigh= lookup_widget(window1,"pRadioButtonHiglightSelect");
  GtkWidget *pButtonAnnot= lookup_widget(window1,"pRadioButtonHiAnnotSelect");
  GtkWidget *pButtonZoomIn=lookup_widget(window1,"buttonZoomIn");
  GtkWidget *pButtonZoomOut=lookup_widget(window1,"buttonZoomOut");
  GtkWidget *pbuttonZoomFitBest=lookup_widget(window1,"buttonZoomFitBest");
  GtkWidget *pButtonPencil=lookup_widget(window1,"button_pencil");
  GtkWidget *pLabelPDFMod=lookup_widget(window1,"PDF_modified_label");
  GtkWidget *pg_frame= lookup_widget(GTK_WIDGET(window1), "page_frame"); 
  GtkWidget *pg_title= lookup_widget(GTK_WIDGET(window1), "page_title"); 
  GtkWidget *pg_entry= lookup_widget(GTK_WIDGET(window1), "page_entry"); 
  GtkWidget *pg_label= lookup_widget(GTK_WIDGET(window1), "page_label"); 
  GtkWidget *pg_labelHitsFrame= lookup_widget(GTK_WIDGET(window1), "labelHitsFrame"); 
  GtkWidget *pLabelHits=lookup_widget(GTK_WIDGET(window1), "labelHits"); 
  /* find and change buttons, statusbar */
  GtkWidget *pButtonReplace= lookup_widget(window1, "buttonReplace");
  GtkWidget *pReplaceEntry = lookup_widget(window1, "replace_entry");
  /* remove searchentry and remove all found and highlighted text */
  gtk_entry_set_text(pSearchEntry,"");
  gtk_label_set_text(GTK_LABEL(pLabelHits), _("0 hits"));
  /* de-active widgets */
  gtk_widget_set_sensitive(pSearchEntry, TRUE);
  gtk_widget_set_sensitive(pButtonText, TRUE);
  gtk_widget_set_sensitive(pButtonPict, TRUE);
  gtk_widget_set_sensitive(pButtonHigh, TRUE);
  gtk_widget_set_sensitive(pButtonAnnot, TRUE);
  gtk_widget_set_sensitive(pButtonReplace, FALSE);
  gtk_widget_set_sensitive(pReplaceEntry, FALSE);
  gtk_widget_set_sensitive(pButtonZoomIn, TRUE);
  gtk_widget_set_sensitive(pbuttonZoomFitBest, TRUE);
  gtk_widget_set_sensitive(pButtonZoomOut, TRUE);
  gtk_widget_set_sensitive(pButtonPencil, FALSE);

  gtk_widget_hide(lookup_widget(GTK_WIDGET(window1), "button_bold") );
  gtk_widget_hide(lookup_widget(GTK_WIDGET(window1), "button_italic") );
  gtk_widget_hide(lookup_widget(GTK_WIDGET(window1), "button_underline") );
  gtk_widget_hide(lookup_widget(GTK_WIDGET(window1), "button_superscript") );
  gtk_widget_hide(lookup_widget(GTK_WIDGET(window1), "button_subscript") );
  gtk_widget_hide(lookup_widget(GTK_WIDGET(window1), "button_strikethrough") );
  gtk_widget_hide(lookup_widget(GTK_WIDGET(window1), "button_highlight") );
  gtk_widget_hide(lookup_widget(GTK_WIDGET(window1), "button_quotation") );
  gtk_widget_hide(lookup_widget(GTK_WIDGET(window1), "button_clear_format") );
  gtk_widget_hide(lookup_widget(GTK_WIDGET(window1), "pRadioButtonLeft") );
  gtk_widget_hide(lookup_widget(GTK_WIDGET(window1), "pRadioButtonCenter") );
  gtk_widget_hide(lookup_widget(GTK_WIDGET(window1), "pRadioButtonRight") );
  gtk_widget_hide(lookup_widget(GTK_WIDGET(window1), "pRadioButtonFill") );

  gtk_widget_show(pg_frame);
  gtk_widget_show(pg_title);
  gtk_widget_show(pg_entry);
  gtk_widget_show(pLabelPDFMod);
  gtk_widget_set_sensitive(pLabelPDFMod, FALSE);
  gtk_widget_set_sensitive(pg_entry, TRUE);
  gtk_widget_show(pg_label);
  gtk_widget_hide(pButtonReplace);
  gtk_widget_hide(pReplaceEntry);
  gtk_widget_show(pButtonZoomIn);
  gtk_widget_show(pButtonZoomOut);
  gtk_widget_show(pbuttonZoomFitBest);
  gtk_widget_show(pg_labelHitsFrame);
  gtk_widget_show(pLabelHits);
  gtk_widget_show(pSearchEntry);
  gtk_widget_show(pNext);
  gtk_widget_show(pPrev);
}

/*
  activate/deactivate buttons in Editor stack
entry : window1 == mainWindow

*/
void misc_set_gui_in_editor_mode(GtkWidget *window1, gint prevStack)
{
  /* redio tool buttons upper toolbar */
  GtkWidget *pSearchEntry=lookup_widget(window1, "search_entry"); 
  GtkWidget *pNext=lookup_widget(window1, "buttonNextOccurrence");   
  GtkWidget *pPrev=lookup_widget(window1, "buttonPrevOccurrence");
  GtkWidget *pButtonText= lookup_widget(window1, "pRadioButtonTextSelect");
  GtkWidget *pButtonPict= lookup_widget(window1, "pRadioButtonPictureSelect");
  GtkWidget *pButtonHigh= lookup_widget(window1,"pRadioButtonHiglightSelect");
  GtkWidget *pButtonAnnot= lookup_widget(window1,"pRadioButtonHiAnnotSelect");
  GtkWidget *pButtonZoomIn=lookup_widget(window1,"buttonZoomIn");
  GtkWidget *pButtonZoomOut=lookup_widget(window1,"buttonZoomOut");
  GtkWidget *pbuttonZoomFitBest=lookup_widget(window1,"buttonZoomFitBest");
  GtkWidget *pButtonPencil=lookup_widget(window1,"button_pencil");
  GtkWidget *pLabelPDFMod=lookup_widget(window1,"PDF_modified_label");
  GtkWidget *pg_frame= lookup_widget(GTK_WIDGET(window1), "page_frame"); 
  GtkWidget *pg_title= lookup_widget(GTK_WIDGET(window1), "page_title"); 
  GtkWidget *pg_entry= lookup_widget(GTK_WIDGET(window1), "page_entry"); 
  GtkWidget *pg_label= lookup_widget(GTK_WIDGET(window1), "page_label"); 
  GtkWidget *pg_labelHitsFrame= lookup_widget(GTK_WIDGET(window1), "labelHitsFrame"); 
  GtkWidget *pLabelHits=lookup_widget(GTK_WIDGET(window1), "labelHits"); 
  /* find and change buttons, statusbar */
  GtkWidget *pButtonReplace= lookup_widget(window1, "buttonReplace");
  GtkWidget *pReplaceEntry = lookup_widget(window1, "replace_entry");
  /* remove searchentry and remove all found and highlighted text */
  gtk_entry_set_text(pSearchEntry,"");
  gtk_label_set_text(GTK_LABEL(pLabelHits), _("0 hits"));
  /* de-active widgets */
  gtk_widget_set_sensitive(pSearchEntry, TRUE);
  gtk_widget_set_sensitive(pButtonText, FALSE);
  gtk_widget_set_sensitive(pButtonPict, FALSE);
  gtk_widget_set_sensitive(pButtonHigh, FALSE);
  gtk_widget_set_sensitive(pButtonAnnot, FALSE);
  gtk_widget_set_sensitive(pButtonReplace, FALSE);
  gtk_widget_set_sensitive(pReplaceEntry, FALSE);
  gtk_widget_set_sensitive(pButtonZoomIn, FALSE);
  gtk_widget_set_sensitive(pButtonZoomOut, FALSE);
  gtk_widget_set_sensitive(pButtonPencil, FALSE);
  //gtk_widget_set_sensitive(pg_entry, FALSE);
  gtk_widget_show(lookup_widget(GTK_WIDGET(window1), "button_bold") );
  gtk_widget_show(lookup_widget(GTK_WIDGET(window1), "button_italic") );
  gtk_widget_show(lookup_widget(GTK_WIDGET(window1), "button_underline") );
  gtk_widget_show(lookup_widget(GTK_WIDGET(window1), "button_superscript") );
  gtk_widget_show(lookup_widget(GTK_WIDGET(window1), "button_subscript") );
  gtk_widget_show(lookup_widget(GTK_WIDGET(window1), "button_strikethrough") );
  gtk_widget_show(lookup_widget(GTK_WIDGET(window1), "button_highlight") );
  gtk_widget_show(lookup_widget(GTK_WIDGET(window1), "button_quotation") );
  gtk_widget_show(lookup_widget(GTK_WIDGET(window1), "button_clear_format") );
  gtk_widget_show(lookup_widget(GTK_WIDGET(window1), "pRadioButtonLeft") );
  gtk_widget_show(lookup_widget(GTK_WIDGET(window1), "pRadioButtonCenter") );
  gtk_widget_show(lookup_widget(GTK_WIDGET(window1), "pRadioButtonRight") );
  gtk_widget_show(lookup_widget(GTK_WIDGET(window1), "pRadioButtonFill") );

  gtk_widget_hide(pLabelPDFMod);
  gtk_widget_hide(pg_frame);
  gtk_widget_hide(pg_title);
  gtk_widget_hide(pg_entry);
  gtk_widget_hide(pg_label);
  gtk_widget_show(pButtonReplace);
  gtk_widget_show(pReplaceEntry);
  gtk_widget_hide(pButtonZoomIn);
  gtk_widget_hide(pButtonZoomOut);
  gtk_widget_hide(pbuttonZoomFitBest);
  gtk_widget_show(pg_labelHitsFrame);
  gtk_widget_show(pLabelHits);
  gtk_widget_show(pSearchEntry);
  gtk_widget_show(pNext);
  gtk_widget_show(pPrev);
}

/*
  activate/deactivate buttons in Editor stack
entry : window1 == mainWindow

*/
void misc_set_gui_in_sketch_mode(GtkWidget *window1, gint prevStack)
{
  GtkWidget *pSearchEntry=lookup_widget(window1, "search_entry"); 
  GtkWidget *pNext=lookup_widget(window1, "buttonNextOccurrence");   
  GtkWidget *pPrev=lookup_widget(window1, "buttonPrevOccurrence");
  /* redio tool buttons upper toolbar */
  GtkWidget *pButtonText= lookup_widget(window1, "pRadioButtonTextSelect");
  GtkWidget *pButtonPict= lookup_widget(window1, "pRadioButtonPictureSelect");
  GtkWidget *pButtonHigh= lookup_widget(window1,"pRadioButtonHiglightSelect");
  GtkWidget *pButtonAnnot= lookup_widget(window1,"pRadioButtonHiAnnotSelect");
  GtkWidget *pButtonZoomIn=lookup_widget(window1,"buttonZoomIn");
  GtkWidget *pButtonZoomOut=lookup_widget(window1,"buttonZoomOut");
  GtkWidget *pbuttonZoomFitBest=lookup_widget(window1,"buttonZoomFitBest");
  GtkWidget *pButtonPencil=lookup_widget(window1,"button_pencil");
  GtkWidget *pLabelPDFMod=lookup_widget(window1,"PDF_modified_label");
  GtkWidget *pg_frame= lookup_widget(GTK_WIDGET(window1), "page_frame"); 
  GtkWidget *pg_title= lookup_widget(GTK_WIDGET(window1), "page_title"); 
  GtkWidget *pg_entry= lookup_widget(GTK_WIDGET(window1), "page_entry"); 
  GtkWidget *pg_label= lookup_widget(GTK_WIDGET(window1), "page_label"); 
  GtkWidget *pg_labelHitsFrame= lookup_widget(GTK_WIDGET(window1), "labelHitsFrame"); 

  GtkWidget *pLabelHits=lookup_widget(GTK_WIDGET(window1), "labelHits"); 
  /* find and change buttons, statusbar */
  GtkWidget *pButtonReplace= lookup_widget(window1, "buttonReplace");
  GtkWidget *pReplaceEntry = lookup_widget(window1, "replace_entry");
  /* remove searchentry and remove all found and highlighted text */
  gtk_entry_set_text(pSearchEntry,"");
  gtk_label_set_text(GTK_LABEL(pLabelHits), _("0 hits"));
  /* de-active widgets */
  gtk_widget_set_sensitive(pSearchEntry, FALSE);
  gtk_widget_set_sensitive(pButtonText, FALSE);
  gtk_widget_set_sensitive(pButtonPict, TRUE);
  gtk_widget_set_sensitive(pButtonHigh, FALSE);
  gtk_widget_set_sensitive(pButtonAnnot, TRUE);/* to allow text notes inside sketch */
  gtk_widget_set_sensitive(pButtonReplace, FALSE);
  gtk_widget_set_sensitive(pReplaceEntry, FALSE);
  gtk_widget_set_sensitive(pButtonZoomIn, FALSE);
  gtk_widget_set_sensitive(pButtonZoomOut, FALSE);
  gtk_widget_set_sensitive(pButtonPencil, TRUE);
  gtk_widget_set_sensitive(pg_entry, FALSE);

  gtk_widget_hide(lookup_widget(GTK_WIDGET(window1), "button_bold") );
  gtk_widget_hide(lookup_widget(GTK_WIDGET(window1), "button_italic") );
  gtk_widget_hide(lookup_widget(GTK_WIDGET(window1), "button_underline") );
  gtk_widget_hide(lookup_widget(GTK_WIDGET(window1), "button_superscript") );
  gtk_widget_hide(lookup_widget(GTK_WIDGET(window1), "button_subscript") );
  gtk_widget_hide(lookup_widget(GTK_WIDGET(window1), "button_strikethrough") );
  gtk_widget_hide(lookup_widget(GTK_WIDGET(window1), "button_highlight") );
  gtk_widget_hide(lookup_widget(GTK_WIDGET(window1), "button_quotation") );
  gtk_widget_hide(lookup_widget(GTK_WIDGET(window1), "button_clear_format") );
  gtk_widget_hide(lookup_widget(GTK_WIDGET(window1), "pRadioButtonLeft") );
  gtk_widget_hide(lookup_widget(GTK_WIDGET(window1), "pRadioButtonCenter") );
  gtk_widget_hide(lookup_widget(GTK_WIDGET(window1), "pRadioButtonRight") );
  gtk_widget_hide(lookup_widget(GTK_WIDGET(window1), "pRadioButtonFill") );


  gtk_widget_hide(pg_label);
  gtk_widget_hide(pg_frame);
  gtk_widget_hide(pg_title);
  gtk_widget_hide(pg_entry);
  gtk_widget_hide(pLabelPDFMod);
  gtk_widget_hide(pButtonReplace);
  gtk_widget_hide(pReplaceEntry);
  gtk_widget_hide(pButtonZoomIn);
  gtk_widget_hide(pButtonZoomOut);
  gtk_widget_hide(pbuttonZoomFitBest);
  gtk_widget_hide(pg_labelHitsFrame);
  gtk_widget_hide(pLabelHits);
  gtk_widget_hide(pSearchEntry);
  gtk_widget_hide(pNext);
  gtk_widget_hide(pPrev);
}
/*************************************
 change betwwen red anf green in order
  to remind the user if the current
  PDF has been modifyed
*************************************/
void update_PDF_state(APP_data *data, gint state)
{
  GtkWidget *window1=data->appWindow;
  GtkWidget *pLabelPDFMod=lookup_widget(window1,"PDF_modified_label");

  gtk_widget_hide( lookup_widget(GTK_WIDGET(data->appWindow),"image_pdf_modif"));
  switch(state) {
    case PDF_NON_MODIF:{
      gtk_label_set_markup (GTK_LABEL (pLabelPDFMod), "<span background=\"green\">   </span>");
      break;
    }
    case PDF_MODIF:{
      gtk_label_set_markup (GTK_LABEL (pLabelPDFMod), "<span background=\"red\" foreground=\"white\"><b> ! </b></span>");
      gtk_widget_show( lookup_widget(GTK_WIDGET(data->appWindow),"image_pdf_modif"));
      break;
    }
    default:{
      gtk_label_set_markup (GTK_LABEL (pLabelPDFMod), "   ");
    }
  }/* end switch */   
}

void update_statusbarPDF(APP_data *data) 
{
  gchar *msg, *filename, *tmpStr=NULL;
  GtkWidget *window1=data->appWindow;
  GKeyFile *keyString=data->keystring;
  GtkWidget *statusbar = GTK_STATUSBAR(data->statusbar1);

  GtkWidget *pg_entry= lookup_widget(GTK_WIDGET(window1), "page_entry"); 
  GtkWidget *pg_label= lookup_widget(GTK_WIDGET(window1), "page_label"); 
  gtk_entry_set_text(GTK_ENTRY(pg_entry),g_strdup_printf("%d",data->curPDFpage+1));
  if(data->doc)
     gtk_widget_set_sensitive(pg_entry, TRUE);

  gtk_entry_set_text(GTK_ENTRY(pg_label),g_strdup_printf(_("of %d"),data->totalPDFpages));

  //gtk_label_set_markup (GTK_LABEL (pg_label),g_strdup_printf(_("of %d"),data->totalPDFpages));

  filename= g_key_file_get_string(keyString, "application", "current-PDF-file-basename", NULL);
  if(strlen(filename)>36) {
     tmpStr=g_strdup_printf("%s...", g_strndup (filename,36));
     g_free(filename);
     filename=tmpStr;
  }
  gtk_statusbar_pop(statusbar, 0); 
  if(data->doc) {
      msg = g_strdup_printf(_("PDF : %s"), filename);
      g_free(filename);
  }
  else
      msg=g_strdup_printf("%s",_("No PDF Document"));
  gtk_statusbar_push(statusbar, 0, msg);
  g_free(msg);
}

void update_statusbarSketch(GtkStatusbar  *statusbar) 
{
  gchar *msg;

  gtk_statusbar_pop(statusbar, 0); 
  msg=g_strdup_printf("%s",_(" "));
  gtk_statusbar_push(statusbar, 0, msg);
  g_free(msg);
}

/***************************************
  set-up various tags for 
  editor's buffer
***************************************/
void misc_setup_text_buffer_tags(GtkTextBuffer *buffer)
{
 GtkTextTag *bold_tag, *italic_tag, *underline_tag, *center_tag, *left_tag, *right_tag, *fill_tag, *std_font_tag;
 GtkTextTag *superscript_tag, *subscript_tag, *highlight_tag, *strikethrough_tag, *quotation_tag;

 /* character formating */
  bold_tag = gtk_text_buffer_create_tag (buffer, "bold",
				    "weight", PANGO_WEIGHT_BOLD,
                                    NULL);
  italic_tag = gtk_text_buffer_create_tag (buffer, "italic",
				    "style", PANGO_STYLE_ITALIC,
                                    NULL);
  underline_tag = gtk_text_buffer_create_tag (buffer, "underline",
				    "underline", PANGO_UNDERLINE_SINGLE,
                                    NULL);
  /* source : https://github.com/GNOME/gtk/blob/master/demos/gtk-demo/textview.c */
  superscript_tag = gtk_text_buffer_create_tag (buffer, "superscript",
                              "rise", 10 * PANGO_SCALE,   /* 10 pixels */
                              "size", 8 * PANGO_SCALE,    /* 8 points */
                                NULL);
  subscript_tag = gtk_text_buffer_create_tag (buffer, "subscript",
                              "rise", -10 * PANGO_SCALE,   /* 10 pixels */
                              "size", 8 * PANGO_SCALE,     /* 8 points */
                               NULL);

  highlight_tag = gtk_text_buffer_create_tag (buffer, "highlight",
                                              "background", "yellow", NULL);
  strikethrough_tag = gtk_text_buffer_create_tag (buffer, "strikethrough",
                                              "strikethrough", TRUE, NULL);
  quotation_tag =  gtk_text_buffer_create_tag (buffer, "quotation",
                              "family", "courier",
                              "size", 13 * PANGO_SCALE,
                              "foreground", "#5E5353",
                              "background", "#DCDCDC",
                              "indent", 30,
                              "left_margin", 40,
                              "right_margin", 40,
                               NULL);
  /* create tags : line formating */

  center_tag = gtk_text_buffer_create_tag (buffer, "center",
				    "justification", GTK_JUSTIFY_CENTER,
                                    NULL);
  left_tag = gtk_text_buffer_create_tag (buffer, "left",
				    "justification", GTK_JUSTIFY_LEFT,
                                    NULL);
  right_tag = gtk_text_buffer_create_tag (buffer, "right",
				    "justification", GTK_JUSTIFY_RIGHT,
                                    NULL);
  fill_tag = gtk_text_buffer_create_tag (buffer, "fill",
				    "justification", GTK_JUSTIFY_FILL,
                                    NULL);



}

/*******************************
  compute zoom value for PDF
  rendering, with the aim to fill
 horizontally the display
 port
 entry : width of PDF to render,
   width of the drawable
 output : a zoom coefficient
 (decimal) to multiply the
 original PDF's size
TODO : add a zoom factor !
********************************/
gdouble misc_get_PDF_ratio(gdouble pdf_width, gdouble draw_width)
{
  return draw_width/pdf_width;
}

/*******************************
 clear text in memory
*******************************/
void misc_clear_text(GtkTextBuffer *buffer, const gchar  *tag)
{
  GtkTextIter start, end;

  gtk_text_buffer_set_text(buffer, "", -1); /* Clear text! */
  gtk_text_buffer_get_start_iter(buffer, &start);
  gtk_text_buffer_get_end_iter (buffer, &end);
  gtk_text_buffer_apply_tag_by_name (buffer,tag, &start, &end);

}
/*******************************
  add en empty paragraph at the
  end of the buffer
******************************/
void misc_append_empty_paragraph(GtkTextBuffer *buffer, gint row, gint total)
{
  GtkTextIter end_of_buffer;

  /* if we are at the last line, we must append an empty paragraph */
  if((row==total-1)||(total==0)) {
       gtk_text_buffer_get_end_iter (buffer, &end_of_buffer);
       gtk_text_buffer_insert (buffer,
                         &end_of_buffer,
                        "\n",-1);
  }
}

/************************************
  we remove all alignment tags for
  the current selection
************************************/
void misc_remove_alignment_tags(GtkTextBuffer *buffer, GtkTextIter start, GtkTextIter end)
{
  gtk_text_buffer_remove_tag_by_name (buffer, "center", &start, &end);
  gtk_text_buffer_remove_tag_by_name (buffer, "left", &start, &end);
  gtk_text_buffer_remove_tag_by_name (buffer, "right", &start, &end);
  gtk_text_buffer_remove_tag_by_name (buffer, "fill", &start, &end);
}

/********************************
 returns a GtkTag pointer given
 an internal quadding code
*********************************/
GtkTextTag *misc_get_tag_from_code(GtkTextBuffer *buffer, gint code)
{
  GtkTextTag *tag;
  GtkTextTagTable *tagTable1;

  tagTable1 = gtk_text_buffer_get_tag_table(buffer);
  switch(code) {
    case LEFT_QUADDING:{
       tag=gtk_text_tag_table_lookup(tagTable1, "left");
       break;
    }
    case CENTER_QUADDING:{
       tag=gtk_text_tag_table_lookup(tagTable1, "center");
       break;
    }
    case RIGHT_QUADDING:{
       tag=gtk_text_tag_table_lookup(tagTable1, "right");
       break;
    }
    case FILL_QUADDING:{
       tag=gtk_text_tag_table_lookup(tagTable1, "fill");
       break;
    }
   default:tag=gtk_text_tag_table_lookup(tagTable1, "left");
  }
  return tag;
}
/********************************
 function returns quadding tag 
  for current iter
  0=left, 1=center, 2=right 3= fill
 see misc.h file
***********************************/
gint misc_get_paragraph_quadding(GtkTextBuffer *buffer, GtkTextIter iter)
{
  GtkTextTag *tag;
  GtkTextTagTable *tagTable1;

 tagTable1 = gtk_text_buffer_get_tag_table(buffer);
 tag = gtk_text_tag_table_lookup(tagTable1, "center");
 if(gtk_text_iter_has_tag (&iter, tag))
    return CENTER_QUADDING;

 tag = gtk_text_tag_table_lookup(tagTable1, "right");
 if(gtk_text_iter_has_tag (&iter, tag))
    return RIGHT_QUADDING;

 tag = gtk_text_tag_table_lookup(tagTable1, "fill");
 if(gtk_text_iter_has_tag (&iter, tag))
    return FILL_QUADDING;

 return LEFT_QUADDING;
}
/********************************
  a simple function to get 
  a Pango string in order to
  display shortcuts
  modifier : NONE =0
  1=CTRL, 2=SHIFT
********************************/
gchar *misc_get_pango_string(const gchar *key, const gint modifier)
{
  switch(modifier) {
    case 0:{
     return g_strdup_printf(" <span font=\"11\" foreground=\"white\" background=\"orange\"><b> %s </b></span>", key);
     break;
    }
    case 1:{
     return g_strdup_printf(" <span font=\"11\" foreground=\"white\" background=\"#352EDA\"><b>CTRL</b></span> + <span font=\"11\" foreground=\"white\" background=\"orange\"><b> %s </b></span>", key);
     break;
    }
   case 2:{
     return g_strdup_printf(_(" <span font=\"11\" foreground=\"white\" background=\"#2C8A3C\"><b>SHIFT</b></span> + <span font=\"11\" foreground=\"white\" background=\"orange\"><b> %s </b></span>"), key);
     break;
    }
   default:;
  }/* end switch */
}


/*
 * Internal helper: destroys pop-up menu created with right-click on results table.
 */
void undo_popup_menu(GtkWidget *attach_widget, GtkMenu *menu)
{  
  gtk_widget_destroy(GTK_WIDGET(menu));
}

/**********************
 initialize various
  values
**********************/
void misc_init_vars(APP_data *data )
{
  data->x1=0;
  data->y1=0;
  data->w=0;
  data->h=0;
  data->iAudioSmartRew=2;
  data->button_pressed=FALSE;
  data->fPdfLoaded=FALSE;
  data->fAudioLoaded=FALSE;
  data->fAudioPlaying=FALSE;
  data->PDFratio=1;
  data->clipboardMode=0; /* default : copy to clipboard the text datas */
  data->curPDFpage=0;
  data->totalPDFpages=0;
  data->doc=NULL;
  data->pdfSearch=NULL;
  data->pdfAnnotMapping=NULL;
  data->kw_paragraph_alignment=KW_ALIGNMENT_LEFT;
  data->fPencilTool=FALSE;
  data->currentStack=CURRENT_STACK_EDITOR;
  /* we prepare Sketch area display */
  data->Sketchsurface=NULL;
  data->surface=NULL;
  /* undo engine */
  data->undo.opCode=OP_NONE;
  data->undoList=NULL;
  data->undo.annotStr=NULL;
  data->undo.pix=NULL;
  data->undo.serialized_buffer=NULL;
  data->pipeline=NULL;/* GStreamer element */
  data->audio_total_duration=0;
  data->audio_current_position=0;
}

/************************
 initalize spell checker
*************************/
void misc_init_spell_checker(APP_data *data )
{
  GtkSpellChecker* spell = gtk_spell_checker_new ();
  data->spell=spell;
  gtk_spell_checker_set_language (spell, pango_language_to_string (pango_language_get_default()), NULL);
  gboolean fAttachSpell = gtk_spell_checker_attach (spell, GTK_TEXT_VIEW (data->view));
}

/**********************************
 get an extract of first 
 chars within the current document
 input : pointer on structure APP_data *data
 output : *gchar
**********************************/
gchar *misc_get_extract_from_document(APP_data *data )
{
  GtkTextBuffer *buffer;
  GtkTextIter start, end;
  
  buffer = data->buffer;
  gtk_text_buffer_get_start_iter (buffer, &start);
  gtk_text_buffer_get_iter_at_offset (buffer,&end, 300); /* we get the first 300 chars */ 
  return gtk_text_buffer_get_text(buffer,&start,&end,FALSE);
}

