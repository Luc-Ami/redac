/*
 * File: audio.c header
 * Description: audio file an playing management
 *
 *
 *
 */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef AUDIO_H
#define AUDIO_H

#include <gtk/gtk.h>
#include "misc.h"
#include <gst/gst.h>

/* Local macros */
#define CONFIG_DISABLE_SAVE_CONFIG_STRING "CONFIG_DISABLE_SAVE_CONFIG" /* Used by config->restart to check for reset */
#define CONFIG_DISABLE_SAVE_CONFIG 1 /* Used by config->restart to check for reset */
#define CURRENT_VERSION "0.0.1"  /*Current version of Kilowriter */
#define MASTER_OPTIONS_DATA "options1" /* Storage for the Phase One data inside the g_object */
#define MAX_RECENT_FILES 10

/* functions */

gint64 audio_time_to_gst_time(gint64 hours, gint64 minutes, gint64 seconds);
void audio_seek_forward(GstElement *pipeline,
          gint64  time_nanoseconds, gint64 time_current_pos, gint64 duration_time_nanoseconds);
void audio_seek_backward(GstElement *pipeline,
          gint64  time_nanoseconds, gint64 time_current_pos, gint64 duration_time_nanoseconds);
void
audio_seek_to_time (GstElement *pipeline, gint64  seconds, gint64 duration_time_nanoseconds);
//gint64 audio_get_duration(GstElement *pipeline );
void audio_get_duration(GstElement *pipeline, gint64 *len );
void audio_get_position(GstElement *pipeline, gint64 *pos);
gchar *audio_gst_time_to_str(gint64 time_value);
void audio_gst_time_to_gdouble(gint64 time_value, gdouble *hh, gdouble *mm, gdouble *ss );
#endif /* AUDIO_H */
