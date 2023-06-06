#include <gst/gst.h>

#define VIDEO_WIDTH   640
#define VIDEO_HEIGHT  480
#define VIDEO_FORMAT  "RGB16"
#define PIXEL_SIZE    2

static void cb_need_data(GstElement *appsrc, guint unused_size, gpointer user_data)
{
    static gboolean white = FALSE;
    static GstClockTime timestamp = 0;
    GstBuffer *buffer;
    guint size;
    GstFlowReturn ret;

    size = VIDEO_WIDTH * VIDEO_HEIGHT * PIXEL_SIZE;
    buffer = gst_buffer_new_allocate (NULL, size, NULL);

    /* this makes the image black/white */
    gst_buffer_memset (buffer, 0, white ? 0x44 : 0xCC, size);

    white = !white;

    GST_BUFFER_PTS (buffer) = timestamp;
    GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale_int (1, GST_SECOND, 2);

    timestamp += GST_BUFFER_DURATION (buffer);

    g_signal_emit_by_name (appsrc, "push-buffer", buffer, &ret);
    gst_buffer_unref (buffer);

    if (ret != GST_FLOW_OK)
    {
        /* something wrong, stop pushing */
        g_printerr("push-buffer fail");
    }
}

int main(int argc, char ** argv)
{
    GstStateChangeReturn ret;
    GstBus *bus;
    GstMessage *msg;
    GstElement* video_src_element;
    GstElement* conv;
    GstElement* video_display_element;
    GstElement* run_pipeline;

    gst_init(&argc, &argv);

    video_src_element = gst_element_factory_make("appsrc", "video_src_element");
    conv = gst_element_factory_make ("videoconvert", "conv");
    video_display_element = gst_element_factory_make("autovideosink", "video_display_element");

    run_pipeline = gst_pipeline_new ("run_pipeline");

    if (!video_src_element || !video_display_element || !conv || !run_pipeline)
    {
        g_printerr ("Not all elements could be created.\n");
        return -1;
    }

    gst_bin_add_many(GST_BIN(run_pipeline), video_src_element, conv,video_display_element, NULL);

    if(gst_element_link_many(video_src_element, conv, video_display_element, NULL) != TRUE)
    {
        g_printerr ("Elements could not be linked.\n");
        gst_object_unref(run_pipeline);
        return -1;
    }

    /* setup appsrc */
    g_object_set (G_OBJECT (video_src_element), "caps",
        gst_caps_new_simple ("video/x-raw",
                     "format",    G_TYPE_STRING,     VIDEO_FORMAT,
                     "width",     G_TYPE_INT,        VIDEO_WIDTH,
                     "height",    G_TYPE_INT,        VIDEO_HEIGHT,
                     "framerate", GST_TYPE_FRACTION, 0, 1,
                     NULL), NULL);
    g_object_set (G_OBJECT (video_src_element), "stream-type", 0, "format", GST_FORMAT_TIME, NULL);

    g_signal_connect (video_src_element, "need-data", G_CALLBACK (cb_need_data), NULL);

    ret = gst_element_set_state(run_pipeline, GST_STATE_PLAYING);

    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_printerr ("Unable to set the pipeline to the playing state.\n");
        gst_object_unref (run_pipeline);
        return -1;
    }

    bus = gst_element_get_bus(run_pipeline);
    msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, (GstMessageType) (GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    if(msg != NULL)
        gst_message_unref(msg);

    gst_object_unref (bus);
    gst_element_set_state (run_pipeline, GST_STATE_NULL);
    gst_object_unref (run_pipeline);

    return 0;
}
