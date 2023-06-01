#include <gst/gst.h>
#include <unistd.h>

#define VIDEO_WIDTH   640
#define VIDEO_HEIGHT  480
#define VIDEO_FORMAT  "RGB16"
#define PIXEL_SIZE    2

static gboolean push_data(GstElement *appsrc) {
    GstBuffer *buffer;
    guint size;
    GstFlowReturn ret;

    size = VIDEO_WIDTH * VIDEO_HEIGHT * PIXEL_SIZE;
    buffer = gst_buffer_new_allocate (NULL, size, NULL);

    GST_BUFFER_PTS (buffer) = GST_CLOCK_TIME_NONE;
    GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale_int (1, GST_SECOND, 5);

    gst_buffer_memset (buffer, 0, g_random_int(), size);
    // Push the buffer into AppSrc
    g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);

    // Free the buffer
    gst_buffer_unref(buffer);

    if (ret != GST_FLOW_OK) {
        // Something went wrong, stop pushing data
        return FALSE;
    }

    return TRUE;
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
    g_object_set (G_OBJECT (video_src_element), "is-live", TRUE, "format", GST_FORMAT_TIME, NULL);

    ret = gst_element_set_state(run_pipeline, GST_STATE_PLAYING);

    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_printerr ("Unable to set the pipeline to the playing state.\n");
        gst_object_unref (run_pipeline);
        return -1;
    }

    while(push_data(video_src_element))
    {
        sleep(1);
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
