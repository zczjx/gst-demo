#include <gst/gst.h>
#include <unistd.h>

#define VIDEO_WIDTH   640
#define VIDEO_HEIGHT  480
#define VIDEO_FORMAT  "RGB16"
#define PIXEL_SIZE    2

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

        // g_signal_emit_by_name (app_src, "push-buffer", buffer, &ret);
        g_printerr("appsink new_sample push-buffer!\n");

        if (ret != GST_FLOW_OK)
        {
            /* something wrong, stop pushing */
            g_printerr("push-buffer fail");;
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
    GstElement* videotest_src2;
    GstElement* app_sink;
    GstElement* app_src;
    GstElement* video_display_element;
    GstElement* pipeline_appsink;

    gst_init(&argc, &argv);

    videotest_src1 = gst_element_factory_make("videotestsrc", "video_src1");
    videotest_src2 = gst_element_factory_make("videotestsrc", "video_src2");
    app_sink = gst_element_factory_make("appsink", "app-sink");
    app_src = gst_element_factory_make("appsrc", "app-src");
    video_display_element = gst_element_factory_make("autovideosink", "video_display_element");


    pipeline_appsink = gst_pipeline_new ("pipeline_appsink");

    if (!videotest_src1 || !videotest_src2 || !app_sink
    || !app_src || !video_display_element || !pipeline_appsink)
    {
        g_printerr ("Not all elements could be created.\n");
        return -1;
    }

    gst_bin_add_many(GST_BIN(pipeline_appsink), videotest_src1, videotest_src2, app_sink, app_src, video_display_element, NULL);

    if((gst_element_link_many(videotest_src1, app_sink, NULL) != TRUE)
    || (gst_element_link_many(videotest_src2, video_display_element, NULL) != TRUE))
    {
        g_printerr ("Elements could not be linked.\n");
        gst_object_unref(pipeline_appsink);
        return -1;
    }



    /* setup videotest_src */
    g_object_set(videotest_src1, "pattern", 19, NULL);
    g_object_set(videotest_src2, "pattern", 19, NULL);

    /* setup appsrc */
    g_object_set (G_OBJECT (app_src), "is-live", TRUE, "format", GST_FORMAT_TIME, NULL);

    /* setup appsink */
    g_object_set(app_sink, "emit-signals", TRUE, NULL);
    g_signal_connect(app_sink, "new-sample", G_CALLBACK(new_sample), app_src);



    ret = gst_element_set_state(pipeline_appsink, GST_STATE_PLAYING);

    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_printerr ("Unable to set the pipeline to the playing state.\n");
        gst_object_unref (pipeline_appsink);
        return -1;
    }

    bus = gst_element_get_bus(pipeline_appsink);
    msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, (GstMessageType) (GST_MESSAGE_ERROR | GST_MESSAGE_EOS));
    if(msg != NULL)
        gst_message_unref(msg);
    gst_object_unref (bus);
    gst_element_set_state (pipeline_appsink, GST_STATE_NULL);
    gst_object_unref (pipeline_appsink);


    return 0;
}
