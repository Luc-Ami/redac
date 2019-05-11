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
#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gstdio.h> /* g_fopen, etc */

#include "interface.h" /* glade requirement */
#include "support.h" /* glade requirement */
#include "mttexport.h"



/**********************
  compute twips for an
  image
  Twips = Pixels / 96 * 1440 
  96 = DPI for the image, 
  a "twip" is 1/1440 
  of an inch 
************************/
gint get_twips(gint val, gint dpi)
{
   return val*1440/dpi;
}

/******************************

  take a jpeg memory buffer
  and output an RTF image string
  i.e. in unsigned Hex
*********************************/
void output_RTF_image_string(gchar *buffer, gsize lg, FILE *outputFile, gint width, gint height)
{
  glong i, k;
  gchar *tmpstr;
 
  tmpstr= g_strdup_printf("{\\pict\\pngblip\\picw%d\\pich%d\\picwgoal%d\\picscalex%d\\pichgoal%d\\picscaley%d \n", 
              width, height,
              get_twips(width, 144), 100, get_twips(height, 144), 100);
  /* we put the image section header */
  fwrite(tmpstr, sizeof(gchar), strlen(tmpstr), outputFile);
  g_free(tmpstr);
  /* image datas in Hexadecimal \bin is useless */
  k=0;
  for(i=0;i<lg;i++) {
     tmpstr = g_strdup_printf("%02x",(unsigned char)buffer[i]); 
     fwrite(tmpstr, sizeof(gchar), strlen(tmpstr), outputFile);
     g_free(tmpstr);
     k++;
     /* 32 hex values by line */
     if(k>31) {
       k=0;
       fwrite("\n", sizeof(gchar), 1, outputFile);
     }
  }
  /* and the image trailer */
  fwrite("}\n", sizeof(gchar), 2, outputFile);
}


/******************************

  output a gchar valid pseudo
  string to an opened file
*****************************/
void output_RTF_pure_text(gchar *text, FILE *outputFile)
{
  guint16 code;
  glong i;
  gchar *tmpstr;
  glong bytes_written, bytes_read;
  GError **error;

  guint16 *uniText = g_utf8_to_utf16 (text,
                 -1,&bytes_read, &bytes_written, &error);


  for(i=0;i<bytes_written;i++) {
      code = uniText[i];
      if(code>255) {/* pure UCS16 */
         tmpstr = g_strdup_printf("\\u%d?",code); 
         fwrite(tmpstr, sizeof(gchar), strlen(tmpstr), outputFile);
         g_free(tmpstr);
      }
      else {
         if(code>127) {/* extended ASCII */
            tmpstr = g_strdup_printf("%s", "\\\'"); 
            fwrite(tmpstr, sizeof(gchar), strlen(tmpstr), outputFile);
            g_free(tmpstr);
            tmpstr = g_strdup_printf("%x",(unsigned char)code); 
            fwrite(tmpstr, sizeof(gchar), strlen(tmpstr), outputFile);
            g_free(tmpstr);
         }
         else {/* ASCII 7 bits */
           if(code>31) {
               if (code == '\\' || code == '{' || code == '}' ){
                  tmpstr = g_strdup_printf("\\%c",(char)code); 
                  fwrite(tmpstr, sizeof(gchar), 2, outputFile);
                  g_free(tmpstr);
               }
               else {
                  tmpstr = g_strdup_printf("%c",(char)code); 
                  fwrite(tmpstr, sizeof(gchar), 1, outputFile);
                  g_free(tmpstr);
               }
           }/* code >31*/
           else {/* control codes */
             switch((char)code) {
               case '\n': {
                  tmpstr = g_strdup_printf("%s","\\par\\pard "); /* new paragraph = reset paragraph formatting with \pard */
                  fwrite(tmpstr, sizeof(gchar), strlen(tmpstr), outputFile);
                  g_free(tmpstr);
                  fwrite("\\s0 ", sizeof(gchar), 3, outputFile);
                  fwrite("\\f1 ", sizeof(gchar), 3, outputFile);
                  fwrite("\\ql ", sizeof(gchar), 3, outputFile);
                  fwrite("\\fi0 " , sizeof(gchar), 4, outputFile);/* first line indent = 1/2"*/
                  fwrite("\\li0 " , sizeof(gchar), 4, outputFile);/* block paargraph left indent = 1/4 " */
                  fwrite("\\ri0 " , sizeof(gchar), 5, outputFile);/* block paragrph. right indent = 1/4 "*/
                  break;
               }
               case '\t': {
                  tmpstr = g_strdup_printf("%s","\\tab "); 
                  fwrite(tmpstr, sizeof(gchar), strlen(tmpstr), outputFile);
                  g_free(tmpstr);
                  break;
               }
               default:;
             }/* end switch */
           }/* if control codes */
         }/* if ascii 7 bits */
      }/* extended ASCII */
  }/* next */
 g_free(uniText);
}

/**********************************

  decode tags 
  it's the core function

**********************************/
void decode_tags(GtkTextBuffer *buffer, FILE *outputFile)
{
  GtkTextIter start, iter, infIter; 
  GtkTextTagTable *tagTable1;
  GtkTextTag *tag;
  gchar *tmpStr =NULL;
  gboolean ok;
  gboolean fBold=FALSE, fItalic=FALSE, fUnderline =FALSE, fStrike = FALSE, fHighlight=FALSE;
  gboolean fSuper=FALSE, fSub =FALSE, fQuote =FALSE;
  gboolean fLeft=FALSE, fCenter=FALSE, fRight=FALSE, fFill =FALSE;
  GdkPixbuf *pixbuf;
  gchar *pixBuffer;
  gsize pixSize;
  GError *pixError = NULL;
  gint k, width, height;

  /* we get the absolute bounds */
  gtk_text_buffer_get_start_iter(buffer, &start);

  /* we start decoding in memory */
  tagTable1 = gtk_text_buffer_get_tag_table(buffer);
  iter = start;
  infIter = start;/* infIter & supIter are the bounds of text zone */

  ok=TRUE;
  k=0;
  while(ok) {
     /* is there is a tag ? */
     /* left alignment */
     if(gtk_text_iter_has_tag (&iter,gtk_text_tag_table_lookup(tagTable1, "left"))) {
       if(!fLeft) {
         fLeft=TRUE;
         fwrite("{\\ql ", sizeof(gchar), 5, outputFile);
       }
     }
     else {
       if(fLeft) {
          fLeft =FALSE;
          fwrite("}", sizeof(gchar), 1, outputFile);/* for others WP */
       }
     }
     /* right alignment */
     if(gtk_text_iter_has_tag (&iter,gtk_text_tag_table_lookup(tagTable1, "right"))) {
       if(!fRight) {
         fRight=TRUE;
         fwrite("{\\qr ", sizeof(gchar), 5, outputFile);
       }
     }
     else {
       if(fRight) {
          fRight =FALSE;
          fwrite("}", sizeof(gchar), 1, outputFile);/* for others WP */
       }
     }
    /* center alignment */
     if(gtk_text_iter_has_tag (&iter,gtk_text_tag_table_lookup(tagTable1, "center"))) {
       if(!fCenter) {
         fCenter=TRUE;
         fwrite("{\\qc ", sizeof(gchar), 5, outputFile);
       }
     }
     else {
       if(fCenter) {
          fCenter =FALSE; 
          fwrite("}", sizeof(gchar), 1, outputFile);/* for others WP */
       }
     }
    /* fill alignment */
     if(gtk_text_iter_has_tag (&iter,gtk_text_tag_table_lookup(tagTable1, "fill"))) {
       if(!fFill) {
         fFill=TRUE;
         fwrite("{\\qj ", sizeof(gchar), 5, outputFile);
       }
     }
     else {
       if(fFill) {
          fFill =FALSE;  
          fwrite("}", sizeof(gchar), 1, outputFile);/* for others WP */
       }
     }

     if(gtk_text_iter_has_tag (&iter,gtk_text_tag_table_lookup(tagTable1, "bold"))) {
       if(!fBold) {
         fBold=TRUE;
         /* we add the RTF bold tag on file : \b  */
         fwrite("{\\b ", sizeof(gchar), 4, outputFile);
       }
     }
     else {/* if the Bold tag is already armed, then we change format ! so we must also output text */
       if(fBold) {
          fBold =FALSE;
          fwrite("}", sizeof(gchar), 1, outputFile);
       }
     }     
     if(gtk_text_iter_has_tag (&iter,gtk_text_tag_table_lookup(tagTable1, "italic"))) {
       if(!fItalic) {
         fItalic=TRUE;
         fwrite("{\\i ", sizeof(gchar), 4, outputFile);
       }
     }
     else {/* if the Bold tag is already armed, then we change format ! so we must also output text */
       if(fItalic) {
          fItalic =FALSE;
          fwrite("}", sizeof(gchar), 1, outputFile);
       }
     }  
     if(gtk_text_iter_has_tag (&iter,gtk_text_tag_table_lookup(tagTable1, "underline"))) {
       if(!fUnderline) {
         fUnderline=TRUE;
         fwrite("{\\ul ", sizeof(gchar), 5, outputFile);
       }
     }
     else {
       if(fUnderline) {
          fUnderline =FALSE;
          fwrite("}", sizeof(gchar), 1, outputFile);
       }
     } 
     /* strikethrough */
     if(gtk_text_iter_has_tag (&iter,gtk_text_tag_table_lookup(tagTable1, "strikethrough"))) {
       if(!fStrike) {
         fStrike=TRUE;
         fwrite("{\\strike ", sizeof(gchar), 9, outputFile);
       }
     }
     else {/* if the Bold tag is already armed, then we change format ! so we must also output text */
       if(fStrike) {
          fStrike =FALSE;
          fwrite("}", sizeof(gchar), 1, outputFile);
       }
     }
     /* superscript */
     if(gtk_text_iter_has_tag (&iter,gtk_text_tag_table_lookup(tagTable1, "superscript"))) {
       if(!fSuper) {
         fSuper=TRUE;
         fwrite("{\\super ", sizeof(gchar), 8, outputFile);
       }
     }
     else {
       if(fSuper) {
          fSuper =FALSE;
          fwrite("}", sizeof(gchar), 1, outputFile);/* for others WP */
       }
     }
     /* subscript */
     if(gtk_text_iter_has_tag (&iter,gtk_text_tag_table_lookup(tagTable1, "subscript"))) {
       if(!fSub) {
         fSub=TRUE;
         fwrite("{\\sub ", sizeof(gchar), 6, outputFile);
       }
     }
     else {
       if(fSub) {
          fSub =FALSE;
          fwrite("}", sizeof(gchar), 1, outputFile);/* for others WP */
       }
     }
     /* highlighting - don't work with Abiword */
     if(gtk_text_iter_has_tag (&iter,gtk_text_tag_table_lookup(tagTable1, "highlight"))) {
       if(!fHighlight) {
         fHighlight=TRUE;
         //fwrite("\\cb3 ", sizeof(gchar), 5, outputFile);
         fwrite("{\\highlight3 ", sizeof(gchar), 13,outputFile);
         /* and the trick ! we must add all others formatings in order to keep them */
       }
     }
     else {/* if the Bold tag is already armed, then we change format ! so we must also output text */
       if(fHighlight) {
          fHighlight =FALSE;
          //fwrite("\\cb2 ", sizeof(gchar), 5, outputFile);
         fwrite("\\highlight0 ", sizeof(gchar), 12,outputFile);
         fwrite("}\n", sizeof(gchar), 2, outputFile);
       }
     }
     /* quotation */
     if(gtk_text_iter_has_tag (&iter,gtk_text_tag_table_lookup(tagTable1, "quotation"))) {
       if(!fQuote) {
         fQuote=TRUE;
         /* we add the RTF bold tag on file : \b  */
       //  fwrite("\\par ", sizeof(gchar), 5, outputFile);
        // fwrite("\\pard ", sizeof(gchar), 6, outputFile);

         fwrite("{", sizeof(gchar), 1, outputFile);
         fwrite("\\s7", sizeof(gchar), 3, outputFile);
         fwrite("\\fi720" , sizeof(gchar), 6, outputFile);/* first line indent = 1/2"*/
         fwrite("\\li360" , sizeof(gchar), 6, outputFile);/* block paargraph left indent = 1/4 " */
         fwrite("\\ri360" , sizeof(gchar), 6, outputFile);/* block paragrph. right indent = 1/4 "*/
         fwrite("\\highlight4 ", sizeof(gchar), 12,outputFile);/* for LibreOffice */
         fwrite("\\qj ", sizeof(gchar), 3, outputFile);/* justify */
         fwrite("\\cb4 ", sizeof(gchar), 4, outputFile);/* background color 4 = light grey - for Abiword*/
         fwrite("\\f0 ", sizeof(gchar), 4, outputFile);/* font 0 */

       }
     }
     else {/* if the Bold tag is already armed, then we change format ! so we must also output text */
       if(fQuote) {
          fQuote =FALSE;
          fwrite("}", sizeof(gchar), 1, outputFile);
          //fwrite("\\par}\n", sizeof(gchar), 6, outputFile);
//fwrite("\\cb0", sizeof(gchar), 4, outputFile);/* background color 4 = light grey - for Abiword*/
//fwrite("\\highlight0 ", sizeof(gchar), 12,outputFile);
//fwrite("\\par ", sizeof(gchar), 5, outputFile);
//fwrite("\\pard ", sizeof(gchar), 6, outputFile);

          fwrite("\\s0 ", sizeof(gchar), 4, outputFile);
          fwrite("\\f1 ", sizeof(gchar), 4, outputFile);
          fwrite("\\ql ", sizeof(gchar), 4, outputFile);
          fwrite("\\fi0 " , sizeof(gchar), 5, outputFile);/* first line indent = 1/2"*/
          fwrite("\\li0 " , sizeof(gchar), 5, outputFile);/* block paargraph left indent = 1/4 " */
          fwrite("\\ri0 " , sizeof(gchar), 5, outputFile);/* block paragrph. right indent = 1/4 "*/
       }
     }
     /* next char - keep in mind that a picture is a char ! */
     ok = gtk_text_iter_forward_char (&iter);
     /* we must check if we are an image at the current infIter position */
     pixbuf = NULL;
     pixbuf = gtk_text_iter_get_pixbuf (&infIter);

     if(pixbuf) {
          width = gdk_pixbuf_get_width (pixbuf);
          height = gdk_pixbuf_get_height (pixbuf);
          gdk_pixbuf_save_to_buffer(pixbuf, &pixBuffer, &pixSize, "png", &pixError, NULL);
          /* we output to file the jpeg */
          output_RTF_image_string(pixBuffer, pixSize, outputFile, width, height);
          /* don't use g_object_unref !!! */
          g_free(pixBuffer);
     }

     /* if not, we output to file the current char */
     output_RTF_pure_text(gtk_text_buffer_get_text (buffer, &infIter, &iter, FALSE), outputFile);
     infIter=iter;
     k++;
  }/* wend */
}

/****************************
  save a file in standard rich
  text format (Ms RTF)
*******************************/
gint save_RTF_rich_text(gchar *filename, APP_data *data_app)
{
  gchar *tmpFileName;
  gchar *fntFamily=NULL;
  const gchar *page_size_a4_portrait="\\paperh16834 \\paperw11909 \\margl1440 \\margr1900 \\margt1800 \\margb1800 \\portrait\n";
  gchar *fonts_header;
  const gchar *color_header="{\\colortbl;\\red255\\green0\\blue0;\\red0\\green0\\blue255;\\red243\\green242\\blue25;\\red241\\green241\\blue241;}\\widowctrl\\s0\\f1\\ql\\fs24\n";
  const gchar *styles_header="{\\stylesheet{\\s0\\f1\\ql redac-Normal;}{\\s7\\fi720\\li360\\ri360\\qj\\cb4\\f0 redac-Quotation;}}\n";
  const gchar *init_paragr_header="\\pard\\plain\\ql\\s0\\f1 ";
  const gchar *rtf_header = "{\\rtf1\\ansi\\ansicpg1252\\deff0\n";
  const gchar *rtf_trailer = "}}";
  gint i, ret, fntSize=12;
  FILE *outputFile;
  GKeyFile *keyString;
  GtkTextBuffer *buffer=data_app->buffer;
  PangoFontDescription *desc;
  /* get main font description */

  keyString = g_object_get_data(G_OBJECT(data_app->appWindow), "config"); 
  desc = pango_font_description_from_string (g_key_file_get_string(keyString, "editor", "font", NULL));
  if (desc != NULL) {
          fntFamily= pango_font_description_get_family (desc);
          fntSize=pango_font_description_get_size(desc)/1000;
  }
  fonts_header=g_strdup_printf("{\\fonttbl{\\f0 Courier;}{\\f1 %s;}{\\f2\\fswiss\\fprq2\\fcharset222 Arial;}}\n", fntFamily );
  /* we check if filename has the .rtf extension */
  if( !g_str_has_suffix (filename, ".rtf" )) {
    /* we correct the filename */
    tmpFileName = g_strconcat(filename, ".rtf", NULL);
  }
  else {
    tmpFileName = g_strdup_printf("%s", filename);/* force to allocate memory */
  }
  ret = 0;

  /* we save as internal Gtk riche text format not the standard RTF format */
  outputFile = fopen(tmpFileName, "w");
  fwrite(rtf_header, sizeof(gchar), strlen(rtf_header), outputFile);
  fwrite(fonts_header, sizeof(gchar), strlen(fonts_header), outputFile);
  fwrite(color_header, sizeof(gchar), strlen(color_header), outputFile);
  fwrite(styles_header, sizeof(gchar), strlen(styles_header), outputFile);
  fwrite(page_size_a4_portrait, sizeof(gchar), strlen(page_size_a4_portrait), outputFile);
  fwrite(init_paragr_header, sizeof(gchar), strlen(init_paragr_header), outputFile);

  decode_tags(buffer, outputFile);

  fwrite(rtf_trailer, sizeof(gchar), strlen(rtf_trailer), outputFile);


  fclose(outputFile);
  g_free(fonts_header);
  g_free(tmpFileName);
  return ret;
}

