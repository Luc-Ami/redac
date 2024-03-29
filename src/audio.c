/***************************
  functions to manage Audio
 files and audio playing
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
#include <gst/gst.h>
#include "interface.h" /* glade requirement */
#include "support.h" /* glade requirement */
#include "audio.h"



/******************************************************** 

converts an HH:MM:SS humain readable time to GST_time

********************************************************/
gint64 audio_time_to_gst_time (gint64 hours, gint64 minutes, gint64 seconds)
{
  gint64 vals = 0, val=0; /* security value */

  if(hours<0 || hours>23 || minutes<0 || minutes>59 || seconds<0 || seconds>59)
    return val;

  vals = (hours*3600) + (minutes*60)+seconds;

  return vals;
}

/***********************************
 seek backward
  relative seeking from current
 position
 all time values are in GST_TIME
 nanoseconds except first parameter,
 "time_nanoseconds" in human seconds
************************************/
void audio_seek_backward(GstElement *pipeline,
          gint64  time_nanoseconds, gint64 time_current_pos, gint64 duration_time_nanoseconds)
{
  gint64 newPos;

  if(time_nanoseconds>0) {
    newPos=time_current_pos-(time_nanoseconds*GST_SECOND);
    if(newPos<0)
 	newPos=0;
    if (!gst_element_seek (pipeline, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
                         GST_SEEK_TYPE_SET, newPos,
                         GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE)) {
      g_print ("* Audio Seek failed! *\n");
    } 
  }
}

/***********************************
 seek forward
 relative seeking from current
 position
 all time values are in GST_TIME
 nanoseconds except first parameter,
 "time_nanoseconds" in human seconds
************************************/
void audio_seek_forward(GstElement *pipeline,
          gint64  time_nanoseconds, gint64 time_current_pos, gint64 duration_time_nanoseconds)
{
  gint64 newPos;

  if(time_nanoseconds>0) {
    newPos=time_current_pos+(time_nanoseconds*GST_SECOND);
    if(newPos>duration_time_nanoseconds)
 	newPos=duration_time_nanoseconds-1*GST_SECOND;/* avoid crashes : auto rewind -1 human second! */

    if (!gst_element_seek (pipeline, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
                         GST_SEEK_TYPE_SET, newPos,
                         GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE)) {
      g_print ("* Audio Seek failed! *\n");
    } 
  }
}
/**************************
  goto specified position
  (seeking)
  source = https://gstreamer.freedesktop.org/documentation/application-development/advanced/queryevents.html
**************************/
void audio_seek_to_time (GstElement *pipeline,
                    gint64 seconds, gint64 duration_time_nanoseconds)
{
  gint64 newPos, curPos, endPos, shift, val1;
  gboolean ret=FALSE;

  /* please note : player must be PAUDED ou PLATING */
  while(!ret) {
     ret=gst_element_query_position (pipeline, GST_FORMAT_TIME, &curPos);
  }/* wend */
  ret=FALSE;
  /* please note : player mus be PAUDED ou PLATING */
  while(!ret) {
     ret=gst_element_query_duration (pipeline, GST_FORMAT_TIME, &endPos);
  }/* wend */
  /* we concert current GST time position in human seconds */
  val1=curPos/GST_SECOND; /* human seconds */
  endPos=endPos/GST_SECOND; /* human seconds */
  shift = seconds-val1;

  if((val1+shift) >= endPos) {
    printf("******* çà va planter !!!! \n");
    shift=endPos-val1-1;
  }
  newPos=curPos+(shift*GST_SECOND);
  if(newPos<0)
    newPos=0;
  if(newPos>duration_time_nanoseconds)
    newPos=duration_time_nanoseconds-1;
  
  if (!gst_element_seek (pipeline, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
                         GST_SEEK_TYPE_SET, newPos,
                         GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE)) {
   g_print ("* Audio Seek failed! *\n");
  }
}
/******************************
  get total duration for
  current stream
  returns a string


gboolean
gst_element_query_duration (GstElement *element,
                            GstFormat format,
                            gint64 *duration);
******************************/
void audio_get_duration (GstElement *pipeline, gint64 *len)
{
  gboolean ret=FALSE;

  gst_element_set_state (pipeline, GST_STATE_PAUSED);/* required - Playbin must be pre-rolled or playing */
  while(!ret) {
     ret = gst_element_query_duration (pipeline, GST_FORMAT_TIME, len);
  }/* wend */
}

/************************************
 get current position ; like
 duration, GStreamer requires that we
 waits until a response
 Please note : the pipeline
 must be in PAUSED or PLAYING state
*************************************/
void audio_get_position (GstElement *pipeline, gint64 *pos)
{
  gboolean ret = FALSE;

 // gst_element_set_state (pipeline, GST_STATE_PAUSED);/* required - Playbin must be pre-rolled or playing */
  while(!ret) {
     ret = gst_element_query_position (pipeline, GST_FORMAT_TIME, pos);
  }/* wend */
}
/*************************************
 converts Gst_Time to human time
 and store results in 3 gdouble
 passed as pointers

*************************************/
void audio_gst_time_to_gdouble (gint64 time_value, gdouble *hh, gdouble *mm, gdouble *ss )
{
   gint64 hour, min, sec, total;

   total = time_value/GST_SECOND; /* total duration in human seconds */
   hour  = total/3600;
   min   = (total-(hour*3600))/60;
   sec   = total-(hour*3600)-(min*60);
   *hh = hour;
   *mm = min;
   *ss = sec;
}

/**************************************

  converts a GST_TIME guint64
 value to human readable string
*************************************/
gchar *audio_gst_time_to_str (gint64 time_value)
{
   gint64 hour, min, sec, total;
   gchar *tmpStr = NULL;

   total = time_value/GST_SECOND; /* total duration in human seconds */
   hour  = total/3600;
   min   = (total-(hour*3600))/60;
   sec   = total-(hour*3600)-(min*60);
   tmpStr = g_strdup_printf ("%02u:%02u:%02u", hour, min, sec);
   return tmpStr;
}
