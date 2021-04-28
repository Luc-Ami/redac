#ifndef CONFIG_H
#define CONFIG_H


#include <gtk/gtk.h>
#include "misc.h"

void update_statusbar(GtkTextBuffer *buffer,
                  APP_data    *data);

void mark_set_callback(GtkTextBuffer *buffer, 
    const GtkTextIter *new_location, GtkTextMark *mark, APP_data    *data);
void copy_to_clipboard(GtkTextView *view, APP_data *data);
void paste_clipboard(GtkTextView *view, APP_data *data);
void cut_to_clipboard(GtkTextView *view, APP_data *data);
void backspace(GtkTextView *view, APP_data *data);
void delete(GtkTextView *view, GtkDeleteType type, gint count, APP_data *data);

void save_standard_file(GtkMenuItem *menuitem,  APP_data    *data);

void
on_open_clicked                 (GtkButton       *button,
                                        APP_data *data_app);

void
on_clearSketch_clicked                 (GtkButton       *button,
                                        APP_data *data_app);
void
on_saveSketch_clicked                 (GtkButton       *button,
                                        APP_data *data_app);
void
on_bold_clicked                 (GtkButton       *button,
                                        APP_data    *data);
void
on_italic_clicked                 (GtkButton       *button,
                                        APP_data    *data);
void
on_underline_clicked                 (GtkButton       *button,
                                        APP_data    *data);
void
on_superscript_clicked                 (GtkButton       *button,
                                        APP_data    *data);
void
on_subscript_clicked                 (GtkButton       *button,
                                        APP_data    *data);
void
on_highlight_clicked                 (GtkButton       *button,
                                        APP_data    *data);
void 
on_strikethrough_clicked                 (GtkButton       *button,
                                        APP_data    *data);
void  on_quotation_clicked           (GtkButton       *button,
                                        APP_data    *data);
void
on_left_justify_clicked      (APP_data *data);
void
on_center_justify_clicked    (APP_data *data);
void
on_right_justify_clicked     (APP_data *data);
void
on_fill_justify_clicked      (APP_data *data);
void
on_clear_format_clicked                 (GtkButton       *button,
                                        APP_data    *data);
void
on_undo_clicked                 (GtkButton       *button,
                                        APP_data    *data);
void
on_button_alignment_toggled   (GtkButton       *button,
                                        APP_data    *data);
gint get_current_alignment(GtkTextBuffer *buffer);
void set_alignment_button(GtkWidget *win, gint alignment);

void
on_find_changed                (GtkSearchEntry *entry, APP_data    *data);
void
on_page_entry_changed  (GtkEntry *entry, APP_data    *data);
void
on_find_next_clicked                 (GtkButton       *button,
                                       APP_data    *data);
void
on_find_prev_clicked                 (GtkButton       *button,
                                        APP_data    *data);
void
on_replace_clicked     (GtkButton       *button,
                                        APP_data    *data);
void
on_prefs_clicked  (GtkButton  *button,  APP_data *data_app);
gboolean key_event(GtkWidget *widget, GdkEventKey *event, APP_data    *data);
void clipboard_request_image(GtkClipboard *clipboard, GdkPixbuf *pixbuf, gpointer data);
void on_doc_show_menu(GtkMenuToolButton *button, GtkMenu *menu,APP_data *data_app);

void
   on_main_menu_button_toggled (GtkToggleButton *togglebutton,  APP_data *data_app);
   
void menuitem_response (GtkMenuItem *menuitem, APP_data *user_data);
void new_project(GtkMenuItem *menuitem,
                 APP_data    *user_data);
void on_stack_changed (GObject    *gobject,
                       GParamSpec *pspec,
                       APP_data    *user_data);
                       
gboolean draw_callback(GtkWidget *widget, cairo_t *cr, APP_data    *data);

gboolean on_PDF_draw_button_press_callback(GtkWidget *widget,
               GdkEvent  *event,
               APP_data *data);

gboolean on_PDF_draw_motion_event_callback(GtkWidget *widget,
               GdkEvent  *event,
               APP_data *data);
gboolean on_PDF_draw_button_release_callback(GtkWidget *widget,
               GdkEvent  *event,
               APP_data *data);
void on_button_clip_mode_toggled (GtkButton  *button,
                                        APP_data *user_data);
void on_PDF_size_changed (GtkWidget    *widget,
               GdkRectangle *allocation,
               APP_data *data_app);
void on_PDF_zoom_in_clicked  (GtkButton       *button,
                                        APP_data    *data);
void on_PDF_zoom_out_clicked  (GtkButton       *button,
                                        APP_data    *data);
void on_PDF_zoom_fit_best_clicked  (GtkButton *button, APP_data *data);
gboolean on_PDF_scroll_event (GtkWidget *widget, GdkEvent  *event, APP_data *data);

gboolean sketch_draw_callback(GtkWidget *widget, cairo_t *cr, APP_data    *data);
gboolean on_sketch_draw_button_press_callback(GtkWidget *widget,
               GdkEvent  *event,
               APP_data *data);
gboolean on_sketch_draw_button_release_callback(GtkWidget *widget,
               GdkEvent  *event,
               APP_data *data);
gboolean on_sketch_draw_motion_event_callback(GtkWidget *widget,
               GdkEvent  *event,
               APP_data *data);
void on_button_button_pencil_toggled(GtkButton       *button,
                                          APP_data *user_data);
void on_menuPasteSketch(GtkMenuItem *menuitem,
               APP_data    *user_data);
void on_menuCenteredPasteSketch(GtkMenuItem *menuitem,
               APP_data    *user_data);
void on_menuPDFEditAnnot(GtkMenuItem *menuitem,
               APP_data    *user_data);
void on_menuPDFColorAnnot(GtkMenuItem *menuitem,
               APP_data    *user_data);
void on_menuPDFRemoveAnnot(GtkMenuItem *menuitem,
               APP_data    *user_data);
gboolean timeout_quick_save( APP_data *data);
void
on_play_pause_clicked (GtkButton *button, APP_data *data);
void
on_jump_prev_clicked (GtkButton *button, APP_data *data);
void
on_jump_next_clicked (GtkButton *button, APP_data *data);
void on_go_jump_clicked(GtkButton *button, APP_data *data);
gdouble on_rewGapSpin_value_changed_event (GtkSpinButton *a_spinner, gpointer user_data);
gboolean timeout_audio_display_position( APP_data *data);
void on_audioPlaySpeed_changed(GtkComboBox     *combobox, APP_data *data);
void
on_about1_activate (GtkMenuItem  *menuitem, APP_data *data);
void on_wiki1_activate (GtkMenuItem  *menuitem, APP_data *data);

void on_keyHelp1_activate (GtkMenuItem  *menuitem, APP_data *data);
void ScrollToEnd (GtkWidget *widget, GdkRectangle *allocation, APP_data *data);
#endif /* CALLBACKS_H */
