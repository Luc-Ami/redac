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


static void undo_free_last_data (APP_data *data)
{
   GList *l = g_list_last (data->undoList);
   undo_datas *tmp_undo_datas;
   tmp_undo_datas = (undo_datas *)l->data;
//printf("avant annot dans free last \n");
   if(tmp_undo_datas->annotStr != NULL)
         g_free (tmp_undo_datas->annotStr);
//printf("apres annot dans free last et avant serial\n");
   if(tmp_undo_datas->serialized_buffer != NULL) {
     g_free (tmp_undo_datas->serialized_buffer);
     tmp_undo_datas->serialized_buffer = NULL;
   }
   if(tmp_undo_datas->pix != NULL)
         g_object_unref (tmp_undo_datas->pix);  

//printf("apres serial dans free last \n");
   g_free (tmp_undo_datas);
   data->undoList = g_list_delete_link (data->undoList, l);  
}


static gint get_undo_current_op_code(APP_data *data)
{
  gint ret=OP_NONE;
  undo_datas *tmp_undo_datas;

  GList *l = g_list_last(data->undoList);
  tmp_undo_datas = (undo_datas *)l->data;
  ret = tmp_undo_datas->opCode;
  return ret;
}


static void update_undo_tooltip( gint op, APP_data *data)
{
 if(g_list_length (data->undoList) > 0) {
  /* we change the [undo] button tooltip text according to the current "push" */
  switch(op) {
    case OP_SET_BOLD:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Undo set text to bold"));
     break;
    }
    case OP_UNSET_BOLD:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Redo set text to bold"));
     break;
    }
    case OP_SET_UNDERLINE:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Undo set text to underline"));
     break;
    }
    case OP_UNSET_UNDERLINE:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Redo set text to underline"));
     break;
    }
    case OP_SET_ITALIC:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Undo set text to italic"));
     break;
    }
    case OP_UNSET_ITALIC:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Redo set text to italic"));
     break;
    }
    case OP_SET_SUPER:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Undo set text to superscript"));
     break;
    }
    case OP_UNSET_SUPER:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Redo set text to superscript"));
     break;
    }
    case OP_SET_SUB:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Undo set text to subscript"));
     break;
    }
    case OP_UNSET_SUB:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Redo set text to subscript"));
     break;
    }
    case OP_SET_HIGH:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Undo set text to highlight"));
     break;
    }
    case OP_UNSET_HIGH:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Redo set text to highlight"));
     break;
    }
    case OP_SET_STRIKE:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Undo set text to strikethrought"));
     break;
    }
    case OP_UNSET_STRIKE:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Redo set text to strikethrought"));
     break;
    }
    case OP_SET_QUOTE:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Undo set paragraph to quotation style"));
     break;
    }
    case OP_REPLACE_TEXT:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Undo replace text"));
     break;
    }
    case OP_TOGGLE_CASE:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Undo set selection to upper or lower case"));
     break;
    }
    case OP_UNSET_QUOTE:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Redo set paragraph to quotation style"));
     break;
    }
    case OP_ALIGN_LEFT:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Undo set paragraph to left alignment, and \ngo back to previous alignment."));
     break;
    }
    case OP_ALIGN_CENTER:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Undo set paragraph to center alignment, and \ngo back to previous alignment"));
     break;
    }
    case OP_ALIGN_RIGHT:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Undo set paragraph to right alignment, and \ngo back to previous alignment"));
     break;
    }
    case OP_ALIGN_FILL:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Undo set paragraph to fill alignment, and \ngo back to previous alignment"));
     break;
    }
    case OP_INS_CHAR:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Undo insert char"));
     break;
    }
    case OP_DEL_CHAR:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Undo delete char"));
     break;
    }
    case OP_INS_BLOCK:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Undo insert block of text"));
     break;
    }
    case OP_DEL_BLOCK:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Undo delete block of text"));
     break;
    }
    case OP_INS_IMG:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Undo insert image"));
     break;
    }
    case OP_DEL_IMG:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Undo delete image"));
     break;
    }
    case OP_SET_TEXT_ANNOT:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Undo new PDF text annot"));
     break;
    }
    case OP_SET_HIGHLIGHT_ANNOT:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Undo set highlighting inside PDF"));
     break;
    }
    case OP_SET_ANNOT_COLOR:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Undo set color of current annotation inside PDF"));
     break;
    }
    case OP_SET_ANNOT_STR:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Undo set current annotation text content inside PDF"));
     break;
    }
    case OP_REMOVE_ANNOT:{
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Undo remove annotation inside PDF"));
     break;
    }
    case OP_SET_POINT: {
      gtk_widget_set_tooltip_text (data->pBtnUndo, _("Undo last drawing inside sketch"));
      break;
    }
    case OP_PASTE_PIXBUF: {
      gtk_widget_set_tooltip_text (data->pBtnUndo, _("Undo last image pasting inside sketch"));
      break;
    }
    case OP_SKETCH_ANNOT: {
      gtk_widget_set_tooltip_text (data->pBtnUndo, _("undo last text annot paste inside sketch - TODO"));
      break;
    }
    default:{
       gtk_widget_set_tooltip_text (data->pBtnUndo, "There is a strange alien to undo ;-)");
    }
  }/* end switch */
 }
 else {
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("Nothing to undo"));
 }
}

static void undo_free_first_data(APP_data *data)
{
   GList *l= g_list_first (data->undoList);
   undo_datas *tmp_undo_datas;
   tmp_undo_datas = (undo_datas *)l->data;
// printf("avant annot dans free first \n");
   if(tmp_undo_datas->annotStr != NULL)
         g_free (tmp_undo_datas->annotStr);
// printf("apres annot dans free first et avant serial\n");
   if(tmp_undo_datas->serialized_buffer != NULL) {
     g_free (tmp_undo_datas->serialized_buffer);
     tmp_undo_datas->serialized_buffer = NULL;
   }
   if(tmp_undo_datas->pix != NULL)
         g_object_unref (tmp_undo_datas->pix);  

// printf("apres serial dans free first \n");
   g_free (tmp_undo_datas);
   data->undoList = g_list_delete_link (data->undoList, l);  
}


static void undo_push_editor (gint op, APP_data *data)
{
  gint undo_list_len = g_list_length (data->undoList);

  /* if current list's length==max allowed length for undo buffer, we delete 
     the first element */
  if( undo_list_len >= MAX_UNDO_OPERATIONS) {
    printf("* Redac : Undo buffer is full, I delete the first element *\n");
    // remove first element 
    if(data->undoList != NULL)
       undo_free_first_data (data);
  }
  data->undo.opCode = op;
  /* we add datas to the GList */
  undo_datas *tmp_undo_datas;
  tmp_undo_datas = g_malloc (sizeof (undo_datas));
  /* store values from quasi global var */
  *tmp_undo_datas = data->undo;
  data->undoList = g_list_append (data->undoList, tmp_undo_datas);
  update_undo_tooltip (op, data);
  /* vérifier longueur */
 // printf("list annul contient %d éléments j'ai pushé %d qui est traduit par %d \n", 
   //                   g_list_length ( data->undoList), 
     //                 data->undo.opCode, tmp_undo_datas->opCode);

}

static void undo_push_PDF (gint op, PopplerAnnot *tmpAnnot, APP_data *data)
{
//  PopplerAnnot my_annot;	
  gint undo_list_len = g_list_length (data->undoList);

  /* if current list's length==max allowed length for undo buffer, we delete 
     the first element */
  if (undo_list_len >= MAX_UNDO_OPERATIONS) {
    printf("* Redac : Undo buffer is full, I delete the first element *\n");
    // remove first element 
    if(data->undoList != NULL) {
       undo_free_first_data (data);
    }
  }

  data->undo.opCode = op;
  /* we add datas to the GList */
  undo_datas *tmp_undo_datas;
  tmp_undo_datas = g_malloc (sizeof(undo_datas));
  /* store values from quasi global var */
  *tmp_undo_datas = data->undo;
  tmp_undo_datas->annotStr = g_strdup_printf("%s", data->undo.annotStr);
//  my_annot = g_malloc(sizeof(PopplerAnnot));
  tmp_undo_datas->annot = tmpAnnot;
  data->undoList = g_list_append (data->undoList, tmp_undo_datas);
  update_undo_tooltip (op, data);
}

static void undo_push_sketch (gint op, APP_data *data)
{
  gint undo_list_len = g_list_length (data->undoList);

  /* if current list's length==max allowed length for undo buffer, we delete 
     the first element */
  if (undo_list_len >= MAX_UNDO_OPERATIONS) {
    printf("* Redac : Undo buffer is full, I delete the first element *\n");
    // remove first element 
    if(data->undoList != NULL)
       undo_free_first_data (data);
  }

  data->undo.opCode = op;
  /* we add datas to the GList */
  undo_datas *tmp_undo_datas;
  tmp_undo_datas = g_malloc (sizeof(undo_datas));
  /* store values from quasi global var */
  *tmp_undo_datas = data->undo;
  tmp_undo_datas->curStack = CURRENT_STACK_SKETCH;
  data->undoList = g_list_append (data->undoList, tmp_undo_datas);
  update_undo_tooltip (op, data);
}

/*****************************
 main function (public) to
 call undo engine

*******************************/
void undo_push (gint current_stack, gint op, PopplerAnnot *annot, APP_data *data)
{
  gtk_widget_set_sensitive (data->pBtnUndo, TRUE);
  switch(current_stack) {
    case CURRENT_STACK_EDITOR: {
      undo_push_editor (op, data);
      break;
    }
    case CURRENT_STACK_PDF: {
      undo_push_PDF (op, annot, data);
      break;
    }
    case CURRENT_STACK_SKETCH: {
      undo_push_sketch (op, data);
      break;
    }
    default: {
      printf("* Redac : something is wrong with UNDO engine source *\n");
    }
  }/* end switch */
  /* now we add last datas to undo GList */
}

static void undo_pop_editor (gint op, APP_data *data)
{
  GtkTextTag *tag;
  GtkTextTagTable *tagTable1; 
  undo_datas *tmp_undo_datas;
  GtkTextIter iter, start, end;
  gsize length;
  gboolean flag;

  GList *l = g_list_last (data->undoList);
  tmp_undo_datas = (undo_datas *)l->data;

  tagTable1 = gtk_text_buffer_get_tag_table (data->buffer);

  if(op > 0 && op < 50) {
     switch(op) {
        case OP_SET_BOLD:{
          tag = gtk_text_tag_table_lookup (tagTable1, "bold");
          gtk_text_buffer_remove_tag (data->buffer, tag, &tmp_undo_datas->start_sel, &tmp_undo_datas->end_sel);
          break;
        }
        case OP_UNSET_BOLD:{
          tag = gtk_text_tag_table_lookup (tagTable1, "bold");
          gtk_text_buffer_apply_tag (data->buffer, tag, &tmp_undo_datas->start_sel, &tmp_undo_datas->end_sel);
          break;
        }
        case OP_SET_ITALIC:{
          tag = gtk_text_tag_table_lookup (tagTable1, "italic");
          gtk_text_buffer_remove_tag (data->buffer, tag, &tmp_undo_datas->start_sel, &tmp_undo_datas->end_sel);
          break;
        }
        case OP_UNSET_ITALIC:{
          tag = gtk_text_tag_table_lookup (tagTable1, "italic");
          gtk_text_buffer_apply_tag (data->buffer, tag, &tmp_undo_datas->start_sel, &tmp_undo_datas->end_sel);
          break;
        }
        case OP_SET_UNDERLINE:{
          tag = gtk_text_tag_table_lookup (tagTable1, "underline");
          gtk_text_buffer_remove_tag (data->buffer, tag, &tmp_undo_datas->start_sel, &tmp_undo_datas->end_sel);
          break;
        }
        case OP_UNSET_UNDERLINE:{
          tag = gtk_text_tag_table_lookup (tagTable1, "underline");
          gtk_text_buffer_apply_tag (data->buffer, tag, &tmp_undo_datas->start_sel, &tmp_undo_datas->end_sel);
          break;
        }
        case OP_SET_SUPER:{
          tag = gtk_text_tag_table_lookup (tagTable1, "superscript");
          gtk_text_buffer_remove_tag (data->buffer, tag, &tmp_undo_datas->start_sel, &tmp_undo_datas->end_sel);
          break;
        }
        case OP_UNSET_SUPER:{
          tag = gtk_text_tag_table_lookup (tagTable1, "superscript");
          gtk_text_buffer_apply_tag (data->buffer, tag, &tmp_undo_datas->start_sel, &tmp_undo_datas->end_sel);
          break;
        }
        case OP_SET_SUB:{
          tag = gtk_text_tag_table_lookup (tagTable1, "subscript");
          gtk_text_buffer_remove_tag (data->buffer, tag, &tmp_undo_datas->start_sel, &tmp_undo_datas->end_sel);
          break;
        }
        case OP_UNSET_SUB:{
          tag = gtk_text_tag_table_lookup (tagTable1, "subscript");
          gtk_text_buffer_apply_tag (data->buffer, tag, &tmp_undo_datas->start_sel, &tmp_undo_datas->end_sel);
          break;
        }
        case OP_SET_HIGH:{
          tag = gtk_text_tag_table_lookup (tagTable1, "highlight");
          gtk_text_buffer_remove_tag (data->buffer, tag, &tmp_undo_datas->start_sel, &tmp_undo_datas->end_sel);
          break;
        }
        case OP_UNSET_HIGH:{
          tag = gtk_text_tag_table_lookup (tagTable1, "highlight");
          gtk_text_buffer_apply_tag (data->buffer, tag, &tmp_undo_datas->start_sel, &tmp_undo_datas->end_sel);
          break;
        }
        case OP_SET_STRIKE:{
          tag = gtk_text_tag_table_lookup (tagTable1, "strikethrough");
          gtk_text_buffer_remove_tag (data->buffer, tag, &tmp_undo_datas->start_sel, &tmp_undo_datas->end_sel);
          break;
        }
        case OP_UNSET_STRIKE:{
          tag = gtk_text_tag_table_lookup (tagTable1, "strikethrough");
          gtk_text_buffer_apply_tag (data->buffer, tag, &tmp_undo_datas->start_sel, &tmp_undo_datas->end_sel);
          break;
        }
        case OP_SET_QUOTE:{
          tag = gtk_text_tag_table_lookup (tagTable1, "quotation");
          gtk_text_buffer_remove_tag (data->buffer, tag, &tmp_undo_datas->start_sel, &tmp_undo_datas->end_sel);
          break;
        }
        case OP_UNSET_QUOTE:{
          tag = gtk_text_tag_table_lookup (tagTable1, "quotation");
          gtk_text_buffer_apply_tag (data->buffer, tag, &tmp_undo_datas->start_sel, &tmp_undo_datas->end_sel);
          break;
        }
        case OP_REPLACE_TEXT:{
          /* remove text inside marked area */
          gtk_text_buffer_get_iter_at_mark (data->buffer, &start, tmp_undo_datas->beforeMark);
          gtk_text_buffer_get_iter_at_mark (data->buffer, &end, tmp_undo_datas->undoMark);
          flag = gtk_text_iter_backward_chars (&start, tmp_undo_datas->str_len);
          gtk_text_buffer_delete (data->buffer, &start, &end);
          /* paste previous rich text */
          GdkAtom format = gtk_text_buffer_register_deserialize_tagset (data->buffer, "application/x-gtk-text-buffer-rich-text");
          gtk_text_buffer_get_iter_at_mark (data->buffer,&start, tmp_undo_datas->beforeMark);
          flag = gtk_text_buffer_deserialize (data->buffer, data->buffer, format, &start, 
                                                  tmp_undo_datas->serialized_buffer, tmp_undo_datas->buffer_length, NULL);
          /* clear some datas */
          gtk_text_buffer_delete_mark (data->buffer, tmp_undo_datas->undoMark); 
          gtk_text_buffer_delete_mark (data->buffer, tmp_undo_datas->beforeMark); 
          break;
        } 
        case OP_INSERT_TIME:{ 
          /* remove text inside marked area */
          gtk_text_buffer_get_iter_at_mark (data->buffer, &start, tmp_undo_datas->beforeMark);
          gtk_text_buffer_get_iter_at_mark (data->buffer, &end, tmp_undo_datas->undoMark);
          flag = gtk_text_iter_backward_chars (&start, tmp_undo_datas->str_len);
          gtk_text_buffer_delete (data->buffer, &start, &end);
          /* clear some datas */
          gtk_text_buffer_delete_mark (data->buffer, tmp_undo_datas->undoMark); 
          gtk_text_buffer_delete_mark (data->buffer, tmp_undo_datas->beforeMark); 
          break;
        }        
               
        case OP_TOGGLE_CASE:{
          /* remove text inside marked area */
          gtk_text_buffer_get_iter_at_mark (data->buffer, &start, tmp_undo_datas->beforeMark);
          gtk_text_buffer_get_iter_at_mark (data->buffer, &end, tmp_undo_datas->undoMark);
          flag=gtk_text_iter_backward_chars (&start, tmp_undo_datas->str_len);
          gtk_text_buffer_delete (data->buffer, &start,&end);
          /* paste previous rich text */
          GdkAtom format = gtk_text_buffer_register_deserialize_tagset (data->buffer, "application/x-gtk-text-buffer-rich-text");
          gtk_text_buffer_get_iter_at_mark (data->buffer, &start, tmp_undo_datas->beforeMark);
          flag = gtk_text_buffer_deserialize (data->buffer, data->buffer, format, &start, 
                                                  tmp_undo_datas->serialized_buffer, tmp_undo_datas->buffer_length, NULL);
          /* clear some datas */
          gtk_text_buffer_delete_mark (data->buffer, tmp_undo_datas->undoMark); 
          gtk_text_buffer_delete_mark (data->buffer, tmp_undo_datas->beforeMark); 
          break;
        }
        case OP_ALIGN_LEFT:
        case OP_ALIGN_CENTER:
        case OP_ALIGN_RIGHT:
        case OP_ALIGN_FILL:{
          tag = misc_get_tag_from_code (data->buffer, tmp_undo_datas->prevQuadding);
          misc_remove_alignment_tags (data->buffer, tmp_undo_datas->start_sel, tmp_undo_datas->end_sel);
          gtk_text_buffer_apply_tag (data->buffer, tag, &tmp_undo_datas->start_sel, &tmp_undo_datas->end_sel);
          break;
        }
        case OP_INS_IMG:
        case OP_INS_CHAR:{
          gtk_text_buffer_get_iter_at_mark (data->buffer, &start, tmp_undo_datas->beforeMark);
          //gtk_text_buffer_get_iter_at_mark (data->buffer,&start,tmp_undo_datas->beforeMark);
          end = start;
          flag = gtk_text_iter_backward_char (&start);
          //printf("char=%s<<\n", gtk_text_buffer_get_text(data->buffer,&start,&end, FALSE) );
          gtk_text_buffer_delete (data->buffer, &start, &end);
          gtk_text_buffer_delete_mark (data->buffer, tmp_undo_datas->beforeMark); 
          gtk_text_buffer_delete_mark (data->buffer, tmp_undo_datas->undoMark); 
          break;
        }
        case OP_DEL_CHAR:{
          GdkAtom format = gtk_text_buffer_register_deserialize_tagset (data->buffer, "application/x-gtk-text-buffer-rich-text");
          gtk_text_buffer_get_iter_at_mark (data->buffer, &iter, tmp_undo_datas->undoMark); 
          if(!tmp_undo_datas->fIsStart) {
             flag = gtk_text_iter_forward_char (&iter);
          }
          else {
             gtk_text_buffer_get_start_iter (data->buffer, &iter);
          }
          gtk_text_buffer_place_cursor (data->buffer,&iter);
          flag = gtk_text_buffer_deserialize (data->buffer, data->buffer, format, &iter, 
                                                  tmp_undo_datas->serialized_buffer, tmp_undo_datas->buffer_length, NULL);
          gtk_text_buffer_delete_mark (data->buffer, tmp_undo_datas->undoMark);      
          break;
        }
        case OP_INS_BLOCK:{  
          gtk_text_buffer_get_iter_at_mark (data->buffer, &end, tmp_undo_datas->undoMark);
          gtk_text_buffer_get_iter_at_mark (data->buffer, &start, tmp_undo_datas->beforeMark);
          flag = gtk_text_iter_forward_char (&start);
          if(tmp_undo_datas->fIsStart) {
                gtk_text_buffer_get_start_iter (data->buffer, &start);
          }
          gtk_text_buffer_delete (data->buffer, &start, &end);
          gtk_text_buffer_delete_mark (data->buffer, tmp_undo_datas->beforeMark); 
          gtk_text_buffer_delete_mark (data->buffer, tmp_undo_datas->undoMark); 
          break;
        }
        case OP_DEL_BLOCK:{
          GdkAtom format = gtk_text_buffer_register_deserialize_tagset (data->buffer, "application/x-gtk-text-buffer-rich-text");
          gtk_text_buffer_get_iter_at_mark (data->buffer, &iter, tmp_undo_datas->undoMark);
          flag = gtk_text_buffer_deserialize (data->buffer, data->buffer, format, &iter, 
                                                  tmp_undo_datas->serialized_buffer, tmp_undo_datas->buffer_length, NULL);
//printf("ava \n");
          gtk_text_buffer_delete_mark (data->buffer, tmp_undo_datas->undoMark);      
//printf("apr \n");
          break;
        }
        default: printf ("* Redac : Unknown UNDO operation request for editor *\n");
     }/* end switch */
  }
}

static PopplerAnnot *find_annot_at_coordinates (gint x1, gint y1, gint x2, gint y2, GList *PDFmap)
{
  GList *l = NULL;
  PopplerAnnotMapping *current_annot_map;
  PopplerAnnot *current_annot = NULL;

// printf("x1=%d y1=%d x2=%d y2=%d \n", x1, y1, x2, y2);
  l = PDFmap;
  for(l; l!=NULL; l=l->next) {
    current_annot_map = (PopplerAnnotMapping *)l->data;
// printf("dans datas x1=%d y1=%d x2=%d y2=%d \n", (gint)current_annot_map->area.x1, (gint)current_annot_map->area.y1, (gint)current_annot_map->area.x2, (gint)current_annot_map->area.y2);
    if(x1>=(gint)current_annot_map->area.x1 && y1>=(gint)current_annot_map->area.y1 
       && x2<=(gint)current_annot_map->area.x2 && y2<=(gint)current_annot_map->area.y2) {
       // printf("bingo ! coinc for annotype dans UNDO =%d \n", poppler_annot_get_annot_type (current_annot_map->annot));
        current_annot = current_annot_map->annot;
    }
  }
  return current_annot;
}




static void undo_remove_highlight_annot (APP_data *data_user, undo_datas *data)
{
  PopplerRectangle rect_annot;
  GArray  *quads_array;

  rect_annot.x1 = data->x1;
  rect_annot.y1 = data->y1;
  rect_annot.x2 = data->x2;
  rect_annot.y2 = data->y2;
  quads_array = pgd_annots_create_quads_array_for_rectangle (&rect_annot);
  PopplerAnnot *my_annot = poppler_annot_text_markup_new_highlight (data_user->doc, &rect_annot, quads_array);
  g_array_free (quads_array, TRUE);
  PopplerColor *my_color = poppler_color_new();
  /* color */
  my_color->red = 65535*data->color.red;
  my_color->green = 65535*data->color.green;
  my_color->blue = 65535*data->color.blue;
  poppler_annot_set_color (my_annot,my_color);  
  poppler_page_add_annot (poppler_document_get_page (data_user->doc, data->PDFpage),my_annot);
  poppler_color_free (my_color);
}

static void undo_remove_text_annot (APP_data *data_user, undo_datas *data)
{
  PopplerRectangle rect_annot;

  rect_annot.x1 = data->x1;
  rect_annot.y1 = data->y1;
  rect_annot.x2 = data->x2;
  rect_annot.y2 = data->y2;

  PopplerAnnot *my_annot = poppler_annot_text_new (data_user->doc, &rect_annot);
  PopplerColor *my_color = poppler_color_new();
  /* color */
  my_color->red = 65535*data->color.red;
  my_color->green = 65535*data->color.green;
  my_color->blue = 65535*data->color.blue;
  poppler_annot_set_color (my_annot,my_color);
  poppler_annot_set_contents (my_annot,data->annotStr);
  poppler_annot_text_set_icon (POPPLER_ANNOT_TEXT(my_annot), POPPLER_ANNOT_TEXT_ICON_COMMENT);
  poppler_annot_text_set_is_open (POPPLER_ANNOT_TEXT(my_annot), FALSE);
  
  poppler_page_add_annot (poppler_document_get_page (data_user->doc, data->PDFpage),my_annot);
  poppler_color_free (my_color);
}

/********************************
 * local function to 'pop' i.e.
 * undo an operation in PDF
 * document
 * *****************************/
static void undo_pop_PDF (gint op, APP_data *data)
{
  gint index;
  PopplerPage *pPage;
  undo_datas *tmp_undo_datas;
  GList *PDFmap, *l;
  PopplerAnnot *current_annot = NULL;

  if(data->doc == NULL) {
    return;
  }
  
  
printf ("lg liste undo =%d \n", g_list_length (data->undoList));  

  l = g_list_last (data->undoList);
  tmp_undo_datas = (undo_datas *)l->data;

  if(op>49 && op<100) {
     switch(op) {
        case OP_SET_TEXT_ANNOT: {
          poppler_page_remove_annot (poppler_document_get_page (data->doc, tmp_undo_datas->PDFpage),
                           tmp_undo_datas->annot);
          break;			
			
	    }
	    case OP_SET_HIGHLIGHT_RECTANGLE_ANNOT:
        case OP_SET_HIGHLIGHT_ANNOT: {
		  index = tmp_undo_datas->undo_index;
		  printf ("annul undo index = %d\n", index);
		//  while(index>0) {
             poppler_page_remove_annot (poppler_document_get_page (data->doc, tmp_undo_datas->PDFpage),
                           tmp_undo_datas->annot);
                        //   undo_free_last_data (data);
           //  l = g_list_last (data->undoList);
           //  tmp_undo_datas = (undo_datas *)l->data;              
           //  index--;
          
          break;
        }
        case OP_SET_ANNOT_COLOR:{
          PopplerColor *color;
          color = poppler_color_new ();
          pPage = poppler_document_get_page (data->doc, data->undo.PDFpage);
          PDFmap = poppler_page_get_annot_mapping (pPage);
          if(PDFmap) {
            color->red   = 65535*tmp_undo_datas->color.red;
            color->green = 65535*tmp_undo_datas->color.green;
            color->blue  = 65535*tmp_undo_datas->color.blue;
            current_annot = find_annot_at_coordinates (tmp_undo_datas->x1, tmp_undo_datas->y1, 
                                                       tmp_undo_datas->x2, tmp_undo_datas->y2, PDFmap);
            if(current_annot) {
                 poppler_annot_set_color (current_annot, color);
            }
            poppler_color_free (color);
            poppler_page_free_annot_mapping (PDFmap);
          }
          g_object_unref (pPage);
          break;
        }
        case OP_SET_ANNOT_STR:{
          pPage = poppler_document_get_page (data->doc, data->undo.PDFpage);
          PDFmap = poppler_page_get_annot_mapping (pPage);
          if(PDFmap) {
             current_annot = find_annot_at_coordinates (tmp_undo_datas->x1, tmp_undo_datas->y1, 
                                                        tmp_undo_datas->x2, tmp_undo_datas->y2, PDFmap);
             if(current_annot) {
                 poppler_annot_set_contents (current_annot,tmp_undo_datas->annotStr);
             }
             poppler_page_free_annot_mapping (PDFmap);
          }
          g_object_unref (pPage);
          break;
        }
        case OP_REMOVE_ANNOT: {          
          switch (tmp_undo_datas->annotType ) {
            case POPPLER_ANNOT_TEXT:{
               // printf("* ask to undo remove annot type text=%d\n", tmp_undo_datas->annotType);
               undo_remove_text_annot (data, tmp_undo_datas);
               break;
            }
            case POPPLER_ANNOT_HIGHLIGHT:{
               // printf("* ask to undo remove annot type highlight=%d\n", tmp_undo_datas->annotType);
               undo_remove_highlight_annot (data, tmp_undo_datas);
               break;
            }
            default:printf("* Redac : Poppler annotation type=%d isn't managed for now, sorry, can't UNDO *\n", tmp_undo_datas->annotType);
          }/* end switch */
          break;
        }
        default:;
     }/* end switch */
  }/* endif */
  PDF_display_page (data->PDFScrollable, data->curPDFpage, data->doc, data);
}

static void undo_pop_sketch (gint op, APP_data *data)
{
  cairo_t *cr;
  undo_datas *tmp_undo_datas;

  cr = cairo_create (data->Sketchsurface);

  GList *l = g_list_last (data->undoList);
  tmp_undo_datas = (undo_datas *)l->data;

  switch(op) {
   case OP_SET_POINT: {
      gdk_cairo_set_source_pixbuf (cr, tmp_undo_datas->pix, tmp_undo_datas->x1, tmp_undo_datas->y1);
      break;
    }
    case OP_PASTE_PIXBUF: {
      gdk_cairo_set_source_pixbuf (cr, tmp_undo_datas->pix, tmp_undo_datas->x1, tmp_undo_datas->y1);
      break;
    }
    case OP_SKETCH_ANNOT: {
      gdk_cairo_set_source_pixbuf (cr, tmp_undo_datas->pix, tmp_undo_datas->x1, tmp_undo_datas->y1);
      break;
    }
    default:{
      printf("* Redac : Something is wrong with Sketch UNDO engine *\n");
    }   

  }/* end switch */

  cairo_paint (cr);
  cairo_destroy (cr);
  gtk_widget_queue_draw (data->SketchDrawable);

}


/*******************************
 main function to 'POP' i.e.
 cancel last operation
********************************/
void undo_pop (gint current_stack, APP_data *data)
{
  gint op;
  if(g_list_length (data->undoList)<1) {
     printf("* Redac : can't do that *, UNDO list empty ! \n");
     return;
  }

  op = get_undo_current_op_code (data);

  if(op>0 && op<50) {     
      undo_pop_editor (op, data);  
  }
  if(op>49 && op<100) {
     undo_pop_PDF (op, data);
  }
  if(op>99) {
     undo_pop_sketch (op, data);
  }
  
  data->undo.opCode = OP_NONE;
  
  /* we must free last data on undo list */
  if (data->undoList != NULL) {
     undo_free_last_data (data);
  }
  /* if #elements ==0 then lock button */
  if (g_list_length (data->undoList) == 0) {
     gtk_widget_set_sensitive (data->pBtnUndo, FALSE);  
     gtk_widget_set_tooltip_text (data->pBtnUndo, _("There is nothing to undo."));
  }
  else {
    GList *l = g_list_last (data->undoList);
    undo_datas *tmp_undo_datas;
    tmp_undo_datas = (undo_datas *)l->data;
    update_undo_tooltip (tmp_undo_datas->opCode, data);
  }  
}
/***********************************
  remove all undo datas from memory
  only for PDF stack ; it's used
  when we load another PDF document 
  in the same session
***********************************/
void undo_free_all_PDF_ops(APP_data *data)
{
  gint nvalPDF = 0, nbvalRem = 0;
  gboolean found;
  GList *l = g_list_first (data->undoList);

  for(l; l!=NULL; l=l->next) {
     undo_datas *tmp_undo_datas;
     tmp_undo_datas = (undo_datas *)l->data;
     if(tmp_undo_datas->curStack == CURRENT_STACK_PDF) {
        nvalPDF++;
     }
  }
  /* now we remove "nvalPDF" datas */  
  while(nvalPDF>0) {
     l = g_list_first (data->undoList);
     found = FALSE;
     while(found==FALSE && l!=NULL) {
        undo_datas *tmp_undo_datas;
        tmp_undo_datas = (undo_datas *)l->data;
        if(tmp_undo_datas->curStack == CURRENT_STACK_PDF) {
           if(tmp_undo_datas->annotStr != NULL)
              g_free (tmp_undo_datas->annotStr);   
           g_free (tmp_undo_datas);
           nbvalRem++;
           nvalPDF--;
           found = TRUE;
           data->undoList = g_list_delete_link (data->undoList, l); 
        }/* endif PDF */
        l = l->next;
     }/* wend */
  }/* wend */

 
// printf("trouvé PDF ds blc rien %d elim.=%d\n", nvalPDF, nbvalRem);
  
  if (g_list_length (data->undoList) == 0)
      gtk_widget_set_sensitive (data->pBtnUndo, FALSE);   
}

/***********************************
  remove all undo datas from memory
  only for PDF stack ; it's used
  when we load another PDF document 
  in the same session
***********************************/
void undo_free_all_sketch_ops(APP_data *data)
{
  gint nvalSKETCH = 0, nbvalRem = 0, count = 0;
  gboolean found;
  GList *l = g_list_first (data->undoList);

  for(l; l!=NULL; l=l->next) {
     undo_datas *tmp_undo_datas;
     tmp_undo_datas = (undo_datas *)l->data;
// printf("count=%d  stack=%d\n", count, tmp_undo_datas->curStack);
     if(tmp_undo_datas->curStack == CURRENT_STACK_SKETCH) {
        nvalSKETCH++;
     }
  }
// printf("il y a %d sketches à éliminer \n", nvalSKETCH);
  /* now we remove "nvalSKETCH" datas */  
  while(nvalSKETCH > 0) {
     l = g_list_first (data->undoList);
     found = FALSE;
     while(found == FALSE && l != NULL) {
        undo_datas *tmp_undo_datas;
        tmp_undo_datas = (undo_datas *)l->data;
        if(tmp_undo_datas->curStack == CURRENT_STACK_SKETCH) {printf("trouvé SKETCH \n");
           if(tmp_undo_datas->pix != NULL)
              g_object_unref (tmp_undo_datas->pix);   
           g_free (tmp_undo_datas);
           nbvalRem++;
           nvalSKETCH--;
           found = TRUE;
           data->undoList = g_list_delete_link (data->undoList, l); 
        }/* endif PDF */
        l = l->next;
     }/* wend */
  }/* wend */

 
// printf("trouvé SKETCH ds blc rien %d elim.=%d\n", nvalSKETCH, nbvalRem);
  
  if( g_list_length (data->undoList) == 0)
      gtk_widget_set_sensitive (data->pBtnUndo, FALSE);   
}
/***********************************
  remove all undo datas from memory
  for example when we quit the
  program or call 'new' menu
***********************************/
void undo_free_all (APP_data *data)
{
  GList *l = g_list_first (data->undoList);

  for(l; l!=NULL; l=l->next) {
     undo_datas *tmp_undo_datas;
     tmp_undo_datas = (undo_datas *)l->data;
     if(tmp_undo_datas->annotStr != NULL)
         g_free (tmp_undo_datas->annotStr);
     if(tmp_undo_datas->serialized_buffer != NULL)
         g_free (tmp_undo_datas->serialized_buffer);
     if(tmp_undo_datas->pix != NULL)
         g_object_unref (tmp_undo_datas->pix);  

     g_free (tmp_undo_datas);
  }
  g_list_free (data->undoList);
  data->undoList = NULL;
}
/*****************************
  reset serialized buffer in
  order to avoid memory
  wasting ;-)
****************************/
void undo_reset_serialized_buffer(APP_data *data)
{
return;
  if(data->undo.serialized_buffer != NULL) {printf("avt free serial \n");
     g_free (data->undo.serialized_buffer);
printf("apres free serial \n");
     data->undo.serialized_buffer = NULL;
  }
}

