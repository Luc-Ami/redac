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

#include "interface.h" /* glade requirement */
#include "support.h" /* glade requirement */
#include "mttimport.h"

/*****************************************
 close file parsed with a correct trailer
 from Searchmonkey  misc.c module
******************************************/
void misc_close_file(FILE *outputFile)
{
  gchar end_sign[]={0x0a,0};
  fwrite(end_sign, sizeof(gchar),strlen(end_sign), outputFile); 
  fclose(outputFile);
}

/******************************************
  function to recognize the file type
  entry = path to file
  output = a gint code to switch
 from Searchmonkey misc.c module
*****************************************/
static gint get_file_type_by_signature(gchar *path_to_file)
{
  FILE *inputFile;
  gint retval = iUnknownFile;
  gchar *buffer;
  glong fileSize;
  gchar gtk_rtf_sign[]="GTKTEXTBUFFERCONTENTS";
  gchar rtf_sign[]="{\\rtf";
  gchar start_sign[]={0x00, 0xD9, 0x00, 0x00, 0x00, 0x00};
  gchar old_word_sign[]={0xdb,0xa5,0};
  gchar write_sign[]={0x31,0xBE,0};
  gchar ole_sign[]={0xD0,0xCF,0x11,0xE0,0xA1,0xB1,0x1A,0xE1,0};
  gchar zip_sign[]="PK\003\004";
  gchar abiword_sign[]={0x3C,0x3F,0x78,0x6D,0};
  gchar pdf_sign[]={0x25, 0x50, 0x44, 0x46, 0};

  inputFile = fopen(path_to_file,"rb");
  if(inputFile==NULL) {
          printf("* ERROR : impossible to open file:%s to check signature *\n", path_to_file);
          return NULL;
  }
  /* we compute the size before dynamically allocate buffer */
   glong prev = ftell(inputFile);   
   fseek(inputFile, 0L, SEEK_END);
   glong sz = ftell(inputFile);
   fseek(inputFile, prev, SEEK_SET);
   if(sz>127)
     sz = 127;
   /* we allocate the buffer */
   buffer = g_malloc0(128);
   /* we start the file reading in Binary mode : it's better for future parsing */
   fileSize = fread(buffer, sizeof(gchar), sz, inputFile);
   fclose(inputFile);
   /* now we attempt to recognize signature */
   if (strncmp(buffer,&gtk_rtf_sign,21)==0)
      return iGtkRtfFile;
   if (strncmp(buffer,&write_sign,2)==0)
      return iMsWriteFile;
   if (strncmp(buffer,&old_word_sign,2)==0)
      return iOldMSWordFile;
   if (strncmp(buffer,&ole_sign,8)==0)
      return iOleMsWordFile;
   if (strncmp(buffer,&rtf_sign,4)==0)
      return iRtfFile;
   if (strncmp(buffer,&pdf_sign,4)==0)
      return iPdfFile;
   if (strncmp(buffer, &zip_sign,4) == 0) {
     /* two cases : MS XML or Oasis XML */
     if(strncmp ((const gchar*)buffer+54,(const gchar *)"oasis", 5)==0) {
        /* now we switch between OASIS files signatures */
       if(strncmp ((const gchar*)buffer+73,(const gchar *)"textPK", 6)==0) {
          return iXmlOdtFile;}
       if(strncmp ((const gchar*)buffer+73,(const gchar *)"presen", 6)==0){
          return iXmlOdpFile;}
       if(strncmp ((const gchar*)buffer+73,(const gchar *)"spread", 6)==0){
          return iXmlOdsFile;}
       else
          return iUnknownFile;
     }
     else {/* dirty */
        /* it's a good Idea to switch between DOC-X, PPT-X and XLS-X */
        return iXmlMsWordFile;
     }
   }
   if (strncmp(buffer, &abiword_sign,4) == 0)
      if(strncmp ((const gchar*)buffer+0x31,(const gchar *)"abiwo", 5)==0)
           return iAbiwordFile;
   return retval;
}

/*********************************************
  various functions to test command words
*********************************************/
gboolean is_font_family(gchar *buffer)
{
   if((g_ascii_strncasecmp (buffer,"fnil",4*sizeof(gchar))==0) ||
       (g_ascii_strncasecmp (buffer,"from",4*sizeof(gchar))==0)
       ||(g_ascii_strncasecmp (buffer,"fswi",4*sizeof(gchar))==0)
       ||(g_ascii_strncasecmp (buffer,"fmod",4*sizeof(gchar))==0)
       ||(g_ascii_strncasecmp (buffer,"fdec",4*sizeof(gchar))==0)
       ||(g_ascii_strncasecmp (buffer,"fscr",4*sizeof(gchar))==0)
       ||(g_ascii_strncasecmp (buffer,"fbid",4*sizeof(gchar))==0)
       ||(g_ascii_strncasecmp (buffer,"ftec",4*sizeof(gchar))==0) )
   return TRUE;
   else return FALSE;
}


/*********************************************
  test character formating
*********************************************/

/*********************************************
   ms-RTF parser
 from Searchmonkey search.c module
 retuens path to a temporary pure text file
*********************************************/
gint RTFCheckFile(gchar *path_to_file, GtkTextBuffer *textBuffer)
{
  FILE *inputFile;
  gint  i, j, fileSize = 0, openedBraces = 0, stylebraces=0, fontbraces=0;
  gchar *buffer = NULL;
  GError **error;
  glong bytes_written, bytes_read;
  gint hexa_char;
  gchar buf_hexa[32];
  GString *str=g_string_new("");

  inputFile = fopen(path_to_file,"rb");
  if(inputFile==NULL) {
          printf("* ERROR : can't open RTF file:%s *\n", path_to_file);
          return -1;
  }
  /* we compute the size before dynamically allocate buffer */
  glong prev = ftell(inputFile);   
  fseek(inputFile, 0L, SEEK_END);
  glong sz = ftell(inputFile);
  fseek(inputFile, prev, SEEK_SET);

  /* we allocate the buffer */
  buffer = g_malloc0(sz*sizeof(gchar)+sizeof(gchar));
  /* we start the file reading in Binary mode : it's better for future parsing */
  fileSize = fread(buffer, sizeof(gchar), sz, inputFile);
  fclose(inputFile);
  i=0;

   while(i<=fileSize) {
    /* is it a command char ? */
    switch(buffer[i])
     {
       case '\\':{
         i++;/* next char */        
         switch(buffer[i])
          {
            case '*':{openedBraces = 1;/* destination control word */
               while( (i<fileSize)&&(openedBraces>0)){
                /* we skip all chars */
                i++;
                        if(buffer[i]=='}')
                           openedBraces--;
                        if(buffer[i]=='{')
                           openedBraces++;
               }
              i--;
              break;           
            }
            case '~':{
              str= g_string_append(str, " ");
              break;
            }
            case '\\':{
              str= g_string_append (str, "\\");
              break;
            }
            case '{':{
              str= g_string_append (str, "{");
              break;
            }
            case '}':{
              str= g_string_append (str, "}");
              break;
            }
            case '-': case '_':{
              str= g_string_append (str, "-");
              break;
            }
            case '\'':{/* char >127 in Hexa mode */
                          buf_hexa[0] = buffer[i+1];
                          buf_hexa[1] = buffer[i+2];
                          buf_hexa[2]=0;
                          buf_hexa[3]=0;
                          sscanf(buf_hexa, "%x", &hexa_char);
                          buf_hexa[0] = hexa_char;
                          buf_hexa[1]=0;   
                          str= g_string_append (str,  g_convert_with_fallback ((gchar *)buf_hexa, 1, "UTF8", "WINDOWS-1252",
                                           NULL, &bytes_read, &bytes_written, &error));                    
                          i=i+2;

              break;
            }
            case 'u':{/* char >127 in decimal mode */
                         i++;
                         j=0;
                         while((i<fileSize) && ( (buffer[i]>='0') && (buffer[i]<='9')))
                           {
                             buf_hexa[j]= buffer[i];
                             i++;
                             j++;
                         }/* wend digits in U chars */
                         if(j>0) {
                           buf_hexa[j]=0;
                           sscanf(buf_hexa, "%d", &hexa_char);
                           if(hexa_char>255) {
                               buf_hexa[0]= hexa_char-(256*(hexa_char/256));
                               buf_hexa[1]= hexa_char/256;
                               buf_hexa[2]=0;
                               buf_hexa[3]=0;
                               buf_hexa[4]=0;
                               if(j>0) {
                                  str= g_string_append (str, g_convert_with_fallback ((gchar *)buf_hexa, 2, "UTF8", "UTF16",
                                           NULL, &bytes_read, &bytes_written, &error));
                               }

                           }/* >255 */
                           else {
                             buf_hexa[0] = hexa_char;/* be careful for chars >255 !!! */
                             buf_hexa[1]=0;
                             if(j>0) {
                                str= g_string_append (str, g_convert_with_fallback ((gchar *)buf_hexa, 1, "UTF8", "WINDOWS-1252",
                                           NULL, &bytes_read, &bytes_written, &error));
                             }
                           }/* <256 */
                           /* we now check if there is an alternate coding */
                           if(i<fileSize-2) {
                             if((buffer[i]=='\\') && (buffer[i+1]=='\'')){
                              i=i+4;/* warning works only for 8 bits Hex values  */
                             }
                           }
                         }
                         else {
                            while( (i<fileSize)&&(buffer[i]!=' ')&&(buffer[i]!='\\')&&(buffer[i]!='}')&&(buffer[i]!='{')) {
                               i++;
                            }
                         }      
              i--;
              break;
            }
            default:{/* ? a true control ? */
              /* at least, we must check if it's a paragraph command 
                  \pard resets any previous paragraph formatting, 
                  \plain resets any previous character formatting*/
              if( g_ascii_strncasecmp ((gchar*)&buffer[i],"pard",4*sizeof(gchar))==0) {
                      i=i+4;
                      /* we should reset properties of paraggraph */
              }
              else
                if(g_ascii_strncasecmp ((gchar*)&buffer[i],"par",3*sizeof(gchar))==0) {
                      i=i+3;
                      str= g_string_append (str, "\n");
                }
                /* we must add the case of \pict controls ! */
                else
                  if(g_ascii_strncasecmp ((gchar*)&buffer[i],"pict",4*sizeof(gchar))==0){
                     /* 2 cases ; simple picture, then no braces in other cases we have \* and braces */
                     i=i+4;
                     openedBraces = 1;
                     while((i<fileSize)&&(openedBraces>0)) {
                        /* we skip all picture's hexa codes */
                        i++;
                        if(buffer[i]=='}')
                           openedBraces--;
                        if(buffer[i]=='{')
                           openedBraces++;
                     }/* wend */
                     i++;
                   }
                   else /* we must remove fonts definitions */
                      if(is_font_family((gchar*)&buffer[i])){
                        i=i+4;
                        while((i<fileSize)&&(buffer[i]!='}')) {
                          i++;
                        }/* wend */
                        i++;
                      }
                      else /* stylesheets */
                        if(g_ascii_strncasecmp ((gchar*)&buffer[i],"stylesheet",10*sizeof(gchar))==0 ) {
                          i=i+10;printf("bingo style sheet \n");
                          stylebraces=1;
                          while((i<fileSize)&&(stylebraces>0)) {
                            if(buffer[i]=='}')
                               stylebraces--;
                            if(buffer[i]=='{')
                               stylebraces++;
                            i++;
                          }/* wend */
                          stylebraces=0;
                          i++;
                        }/* fonttbl = fontable */
                        else
                          if(g_ascii_strncasecmp ((gchar*)&buffer[i],"fonttbl",7*sizeof(gchar))==0) {
                             i=i+7;printf("bingo font table \n");
                             fontbraces=1;
                             while((i<fileSize)&&(fontbraces>0)) {
                               if(buffer[i]=='}')
                                  fontbraces--;
                               if(buffer[i]=='{')
                                  fontbraces++;
                               i++;
                             }/* wend */
                             fontbraces=0;
                             i++;
                          }
                          else {/* others control words - TODO minus sign before digits */
                            while((i<fileSize)&&(buffer[i]!=' ')&&(buffer[i]!='\\')&&(buffer[i]!='}')&&(buffer[i]!='{') &&(buffer[i]!='\n')) {
                              /* we skip all chars */
                              i++;
                            }/* wend */
                          }/* elseif */
             i--;
            }/* case default */
          }/* end switch first char after control */
         break;
       }
       case '{': case '}': case '\n':{/* a Parser MUST ignore LF */
         break;
       }      
       default:{/* plain text */
            str= g_string_append_c (str, buffer[i]);
       }
     }/* end switch */
    i++;
   }/* wend all along the file */   
 g_free(buffer);
 /* we insert text */
 gtk_text_buffer_insert_at_cursor (textBuffer, str->str, strlen(str->str));
 g_string_free(str, TRUE);
 return 0;
}

/**********************************
  callback : button open clicked 
  Please Note : we merge
 from Gtk Rich text files

************************************/
void
on_merge_clicked (GtkButton *button, APP_data *data_app)
{
  GKeyFile *keyString;
  GtkTextBuffer *buffer;
  GtkTextIter start, end;
  guint8 *txtbuffer;
  gchar *path_to_file, *filename;
  FILE *inputFile;
  glong fileSize;
  gint ret, sign;
  GtkFileFilter *filter = gtk_file_filter_new ();
  gtk_file_filter_add_pattern (filter, "*.kw");
  gtk_file_filter_set_name (filter, _("Redac notes"));

  buffer = data_app->buffer;
  GtkWidget *window1 = data_app->appWindow;
  keyString = g_object_get_data(G_OBJECT(window1), "config");

  GdkAtom format = gtk_text_buffer_register_deserialize_tagset(buffer, "application/x-gtk-text-buffer-rich-text");

  GtkWidget *dialog = create_loadFileDialog(data_app);
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), g_get_home_dir());
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
  /* run dialog */
  ret=gtk_dialog_run(GTK_DIALOG(dialog));
  if(ret == GTK_RESPONSE_OK) {
    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
    gtk_widget_destroy (dialog); 
    sign=get_file_type_by_signature(filename);

    if(sign==iGtkRtfFile) { 
       printf("OK, signature Gtk Rtf \n");
       /* first, we save once more time the current file */
       path_to_file = g_key_file_get_string(keyString, "application", "current-file", NULL);
       ret = save_gtk_rich_text(path_to_file, buffer);
       g_free(path_to_file);
       /* TODO we import datas at current insertion point */
       /* we open the file */
       inputFile = fopen(filename,"rb");
       if(inputFile==NULL) {
          printf("* ERROR : can't open for merging with Redac file:%s *\n", filename);
          return;
       }
       /* we compute the size before dynamically allocate buffer */
       glong prev = ftell(inputFile);   
       fseek(inputFile, 0L, SEEK_END);
       glong sz = ftell(inputFile);
       fseek(inputFile, prev, SEEK_SET);
       /* we allocate the buffer */
       if(sz<=0)
         return;
       txtbuffer = g_malloc0(sz*sizeof(guint8)+sizeof(guint8));
       fileSize = fread(txtbuffer, sizeof(guint8), sz, inputFile);
       fclose(inputFile);

       gtk_text_buffer_get_end_iter (buffer, &end);
       gboolean deserialized = gtk_text_buffer_deserialize(buffer, buffer, format, &end, txtbuffer, fileSize, NULL);/* NULL mandatory ? ! */
       g_free(txtbuffer);
    }/* endif iRtfFile RTF file signature */ 
    g_free(filename);
  }/* endif ret */
  else   gtk_widget_destroy (dialog); ;
}
/**********************************
  callback : button open clicked 
  Please Note : we only reload pure
 text from ms-RTF files.

************************************/
void
on_import_clicked (GtkButton *button, APP_data *data_app)
{
  GKeyFile *keyString;
  GtkTextBuffer *buffer;
  gchar *path_to_file, *filename;
  gint ret, sign, ret_conv;
  GtkFileFilter *filter = gtk_file_filter_new ();
  gtk_file_filter_add_pattern (filter, "*.rtf");
  gtk_file_filter_set_name (filter, _("Word processor files"));

  buffer = data_app->buffer;
  GtkWidget *window1 = data_app->appWindow;
  keyString = g_object_get_data(G_OBJECT(window1), "config");


  GtkWidget *dialog = create_loadFileDialog(data_app);
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), g_get_home_dir());
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
  /* run dialog */
  ret=gtk_dialog_run(GTK_DIALOG(dialog));
  if(ret == GTK_RESPONSE_OK) {
    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
    gtk_widget_destroy (dialog); 
    sign=get_file_type_by_signature(filename);

    if(sign==iRtfFile) { 
       /* first, we save once more time the current file */
       path_to_file = g_key_file_get_string(keyString, "application", "current-file", NULL);
       ret = save_gtk_rich_text(path_to_file, buffer);
       g_free(path_to_file);
       /* we import datas at current insertion point */
       ret_conv=RTFCheckFile(filename, buffer);
    }/* endif iRtfFile RTF file signature */ 
    g_free(filename);
  }/* endif ret */
  else   gtk_widget_destroy (dialog); ;
}
