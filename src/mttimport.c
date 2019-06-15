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
          return -1;
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
static gboolean is_font_family(gchar *buffer)
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

/********************************************

  converts a dec-coded char in UTF8
for example : \u2824

length of number ? YES, it's
countdec-1 parameter
*******************************************/
static gchar *rtf_convert_dec_to_utf8(gchar *buffer, gint i, gint countdec)
{
  gchar buf_hexa[32];
  gchar *str=NULL;
  gint hexa_char, j;
  glong bytes_written, bytes_read;
  GError **error;

  for(j=0;j<=31;j++)
     buf_hexa[j]= 0;
  for(j=0;j<countdec;j++) {
      buf_hexa[j]= buffer[i+j+1];
  }/* next */

  sscanf(buf_hexa, "%d", &hexa_char);
  if(hexa_char>255) {
          buf_hexa[0]= hexa_char-(256*(hexa_char/256));
          buf_hexa[1]= hexa_char/256;
          buf_hexa[2]=0;
          buf_hexa[3]=0;
          buf_hexa[4]=0;
          str=  g_convert_with_fallback ((gchar *)buf_hexa, 2, "UTF8", "UTF16",
                                           NULL, &bytes_read, &bytes_written, &error);
  }/* >255 */
  else {
         buf_hexa[0] = hexa_char;/* be careful for chars >255 !!! */
         buf_hexa[1]=0;
         str= g_convert_with_fallback ((gchar *)buf_hexa, 1, "UTF8", "WINDOWS-1252",
                                           NULL, &bytes_read, &bytes_written, &error);
        }/* <256 */
  return str;
}
/********************************************

  converts an hex-coded char in UTF8
  for example : \'ea
*******************************************/
static gchar *rtf_convert_hex_to_utf8(gchar *buffer, gint i)
{
  gchar buf_hexa[32];
  gchar *str=NULL;
  gint hexa_char;
  glong bytes_written, bytes_read;
  GError **error;

  buf_hexa[0] = buffer[i+1];
  buf_hexa[1] = buffer[i+2];
  buf_hexa[2]=0;
  buf_hexa[3]=0;
  sscanf(buf_hexa, "%x", &hexa_char);
  buf_hexa[0] = hexa_char;
  buf_hexa[1]=0;    
  str= g_convert_with_fallback ((gchar *)buf_hexa, 1, "UTF8", "WINDOWS-1252",
                                           NULL, &bytes_read, &bytes_written, error);  
  return str;
}
/************************************************
 function to 'skip' all datas in a 'destination' 
 control sequence, i.e. a command \*
 Input : buffer, current position, filesize
 output : new position where to skip
********************************************/
static gint rtf_skip_destination_section(gchar *buffer, gint position, gint fileSize )
{
 gint j, openedBraces = 1;

 j=position;

 while((j<fileSize)&&(openedBraces>0)){  /* we skip all chars */
    j++;
    if(buffer[j]=='}')
        openedBraces--;
    if(buffer[j]=='{')
        openedBraces++;
 }/* wend */
 return j;
}
/**********************************************
  tokenize commands - only for useful commands
  input : a gchar with the supposed command
  output : a gint
**********************************************/
static gint rtf_tokenize_command(gchar *command)
{
  gint ret=0;
  //printf("command =%s \n", command);
  if(strlen(command)==1) {
      if(command[0]=='i') {
          return fmtItalic;
      }
      if(command[0]=='b') {
          return fmtBold;
      }
  }
  if(strlen(command)==2) {
      if((command[0]=='u')&&(command[1]=='l')) {
          return fmtUnder;
      }  
  }
  if(strlen(command)>2) {
      if((command[0]=='u')&&(command[1]>='0')&&(command[1]<='9')) {
          return cmdDecChar;
      }  
  }
  if(strlen(command)==3) {
      if((command[0]=='s')&&(command[1]=='u')&&(command[2]=='b')) {
          return fmtSub;
      }  
      if((command[0]=='p')&&(command[1]=='a')&&(command[2]=='r')) {
          return cmdPar;
      } 
      if(command[0]=='\'') {
          return cmdHexChar;/* tricky, no ? */
      }
  }
  if(strlen(command)==4) { 
      if((command[0]=='p')&&(command[1]=='a')&&(command[2]=='r')&&(command[3]=='d')) {
          return cmdParD;
      } 
  }
  if(strlen(command)==5) {
      if((command[0]=='s')&&(command[1]=='u')&&(command[2]=='p')&&(command[3]=='e')&&(command[4]=='r')) {
          return fmtSuper;
      }  
  }
  if(strlen(command)==6) {
      if((command[0]=='s')&&(command[1]=='t')&&(command[2]=='r')&&(command[3]=='i')&&(command[4]=='k')&&(command[5]=='e')) {
          return fmtStrike;
      }
     if((command[0]=='a')&&(command[1]=='u')&&(command[2]=='t')&&(command[3]=='h')&&(command[4]=='o')&&(command[5]=='r')) {
          return cmdAuthor;
      }  
  }
  return ret;
}
/*********************************************
   ms-RTF parser
 from Searchmonkey search.c module
 retuens path to a temporary pure text file
*********************************************/
gint RTFCheckFile(gchar *path_to_file, GtkTextBuffer *textBuffer)
{
  FILE *inputFile;
  gint  ret, counthexa, countdec, i, j, posmem, fileSize = 0;
  gboolean fHexa=FALSE, fDec=FALSE;
  gchar *buffer = NULL;
  GError **error;
  glong bytes_written, bytes_read;
  gchar command[32];
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
         switch(buffer[i]) {
            case '*':{/* destination control word */
              i=rtf_skip_destination_section(buffer, i, fileSize );
              //printf("sq * buffer %c%c%c\n", buffer[i-1], buffer[i], buffer[i+1]);
              if(buffer[i+1]!=';')
                  i--;
                else i++;
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
            default:{posmem=i;/* printf("Mot clef à %d:", posmem);*/
                counthexa=0;/* for special chars, like 'EA */
                countdec=0; /* special chars in decimal mode */
                fHexa=FALSE;
                fDec=FALSE;
                j=0;
                while( (i<fileSize)&&(buffer[i]!=' ')&&(buffer[i]!='\\')&&(buffer[i]!='}')&&(buffer[i]!='{')&&(buffer[i]!='\n')) {
                     if(buffer[i]=='\'')
                        fHexa=TRUE;
                     if((buffer[i]=='u')&&(buffer[i+1]>='0')&&(buffer[i+1]<='9'))
                        fDec=TRUE;
                     if(fHexa && counthexa>2)
                          break;
                     command[j]=buffer[i];
                     j++;
                     counthexa++;
                     countdec++;
                     i++;
                }/* wend */
                command[j]=0;
                ret=rtf_tokenize_command(&command[0]);
                //printf(">%i<\n", ret);
                /* here we have discovered the command word - it's between posmem and current i index */
                if(buffer[i]=='\\')
                   i--;
                /* style ? font ? */
                if(((buffer[posmem]=='s') || (buffer[posmem]=='f') )  &&(buffer[posmem+1]>='0')&&(buffer[posmem+1]<='9') ) {
                   i=posmem;
                   while( (i<fileSize)&&(buffer[i]!='}')&&(buffer[i]!='{')&&(buffer[i]!='\n')) {
                      i++;
                   }
                }/* if style / font */
                else {
                    switch(ret) {
                      case cmdHexChar:{
                           str= g_string_append (str, rtf_convert_hex_to_utf8(buffer, posmem));
                           i=posmem+2;
                        break;
                      }
                      case cmdDecChar:{
                           str= g_string_append (str, rtf_convert_dec_to_utf8(buffer, posmem, countdec));
                           i=posmem+countdec;
                           /* jump alternate  Hex char ? */
                           if((buffer[i]=='\\')&&(buffer[i+1]=='\''))
                               i=i+3;
                           /* jump alternate  unknown [?]  char ? */
                           if(buffer[i-1]=='?')
                               i--;
                        break;
                      }
                      case cmdAuthor:{/* we skip all chars until found a closing brace */
                        while( (i<fileSize)&&(buffer[i]!='}')) {
                           i++;
                        }
                        break;
                      }
                      case cmdPar:{
                        str= g_string_append_c (str, '\n');
                        break;
                      }
                      default:{
                        // printf("* RTF control/command not managed *\n");
                      }
                    }/* end switch posmem */
                }/* elseif */
                   
            } /* case : default */        
          }/* end switch command */
         break;
       }/* case control */
       case '{': case '}':{ 
         break;
       }      
       case '\n':{/* a Parser MUST ignore LF */
         //printf("LF inattendu etat buffer après=%c%c%c\n", buffer[i+1],buffer[i+2],buffer[i+3]);
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

  GtkWidget *dialog = create_loadFileDialog(data_app, _("Open Redac file ..."));
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


 GtkWidget *dialog = create_loadFileDialog(data_app, _("Open Rich Text Format file ..."));
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
