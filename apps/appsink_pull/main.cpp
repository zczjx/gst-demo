#include <gst/gst.h>
#include <unistd.h>

#define VIDEO_WIDTH   640
#define VIDEO_HEIGHT  480
#define VIDEO_FORMAT  "RGB16"
#define PIXEL_SIZE    2


gpointer push_data(gpointer data)
{
    GstBuffer *buffer;
    guint size;
    GstFlowReturn ret;
    GstElement *appsrc = (GstElement *) data;

    size = VIDEO_WIDTH * VIDEO_HEIGHT * PIXEL_SIZE;
    buffer = gst_buffer_new_allocate (NULL, size, NULL);
    gst_buffer_memset (buffer, 0, 0x00, size);

    GST_BUFFER_PTS (buffer) = GST_CLOCK_TIME_NONE;
    GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale_int (1, GST_SECOND, 5);

    // Push the buffer into AppSrc
    g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);
    g_printerr("appsrc put new data!\n");
    if (ret != GST_FLOW_OK)
    {
        /* something wrong, stop pushing */
        g_printerr("appsrc: push-buffer fail");
        goto exit;
    }

    while(1)
    {
        sleep(1);
    }
    // Free the buffer
    gst_buffer_unref(buffer);

exit:
    g_thread_exit(0);
}

/* The appsink has received a buffer */
static GstFlowReturn new_sample(GstElement *sink, GstElement *app_src)
{
    GstSample *sample;
    GstFlowReturn ret = GST_FLOW_OK;
    /* Retrieve the buffer */
    g_signal_emit_by_name(sink, "pull-sample", &sample);
    if (sample)
    {
        /* The only thing we do in this example is print a * to indicate a received buffer */

        GstBuffer *buffer = gst_sample_get_buffer(sample);

        g_signal_emit_by_name (app_src, "push-buffer", buffer, &ret);
        g_printerr("appsink new_sample push-buffer!\n");

        if (ret != GST_FLOW_OK)
        {
            /* something wrong, stop pushing */
            g_printerr("push-buffer fail");
        }

        gst_sample_unref(sample);
        return GST_FLOW_OK;
    }

    return GST_FLOW_ERROR;
}


int main(int argc, char ** argv)
{
    GstStateChangeReturn ret;
    GstBus *bus;
    GstMessage *msg;
    GstElement* videotest_src1;
    GstElement* app_sink;
    GstElement* app_src;
    GstElement* conv;
    GstElement* video_display_element;
    GstElement* pipeline_appsink;
    GThread * appsrc_thread;

    gst_init(&argc, &argv);

    videotest_src1 = gst_element_factory_make("videotestsrc", "video_src1");
    app_sink = gst_element_factory_make("appsink", "app-sink");
    app_src = gst_element_factory_make("appsrc", "app-src");
    conv = gst_element_factory_make ("videoconvert", "conv");
    video_display_element = gst_element_factory_make("autovideosink", "video_display_element");


    pipeline_appsink = gst_pipeline_new ("pipeline_appsink");

    if (!videotest_src1 || !app_sink || !app_src
    || !video_display_element || !pipeline_appsink)
    {
        g_printerr ("Not all elements could be created.\n");
        return -1;
    }

    gst_bin_add_many(GST_BIN(pipeline_appsink), videotest_src1, app_sink, app_src, conv, video_display_element, NULL);

    if((gst_element_link_many(videotest_src1, app_sink, NULL) != TRUE)
    || (gst_element_link_many(app_src, conv, video_display_element, NULL) != TRUE))
    {
        g_printerr ("Elements could not be linked.\n");
        gst_object_unref(pipeline_appsink);
        return -1;
    }

    /* setup videotest_src */
    g_object_set(videotest_src1, "pattern", 19, NULL);
    GstPadTemplate *video_src_pad_templ = gst_element_get_pad_template(videotest_src1, "src");
    GstCaps *video_src_pad_caps = 	gst_pad_template_get_caps(video_src_pad_templ);

    /* setup appsrc */
    g_object_set (G_OBJECT (app_src), "caps",
        gst_caps_new_simple ("video/x-raw",
                     "format",    G_TYPE_STRING,     VIDEO_FORMAT,
                     "width",     G_TYPE_INT,        VIDEO_WIDTH,
                     "height",    G_TYPE_INT,        VIDEO_HEIGHT,
                     "framerate", GST_TYPE_FRACTION, 0, 1,
                     NULL), NULL);
    g_object_set (G_OBJECT (app_src), "is-live", TRUE, "format", GST_FORMAT_TIME, NULL);

    /* setup appsink */
    g_object_set(app_sink, "emit-signals", TRUE, NULL);
    g_signal_connect(app_sink, "new-sample", G_CALLBACK(new_sample), app_src);



    ret = gst_element_set_state(pipeline_appsink, GST_STATE_PLAYING);

    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_printerr ("Unable to set the pipeline to the playing state.\n");
        gst_object_unref (pipeline_appsink);
        return GST_STATE_CHANGE_FAILURE;
    }


    appsrc_thread = g_thread_new("appsrc_thread", push_data, app_src);

    if (!appsrc_thread)
    {
        g_printerr ("create appsrc_thread thread fail.\n");
        ret = GST_STATE_CHANGE_FAILURE;
    }

    bus = gst_element_get_bus(pipeline_appsink);
    msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, (GstMessageType) (GST_MESSAGE_ERROR | GST_MESSAGE_EOS));
    if(msg != NULL)
        gst_message_unref(msg);
    g_thread_join (appsrc_thread);
    gst_object_unref (bus);
    gst_element_set_state (pipeline_appsink, GST_STATE_NULL);
    gst_object_unref (pipeline_appsink);


    return 0;
}
