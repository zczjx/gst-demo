#include <gst/gst.h>

int main(int argc, char ** argv)
{
    GstStateChangeReturn ret;
    GstBus *bus;
    GstMessage *msg;
    GstElement* video_src_element;
    GstElement* video_display_element;
    GstElement* run_pipeline;

    gst_init(&argc, &argv);

    video_src_element = gst_element_factory_make("videotestsrc", "video_src_element");
    video_display_element = gst_element_factory_make("autovideosink", "video_display_element");

    run_pipeline = gst_pipeline_new ("run_pipeline");

    if (!video_src_element || !video_display_element || !run_pipeline)
    {
        g_printerr ("Not all elements could be created.\n");
        return -1;
    }

    gst_bin_add_many(GST_BIN(run_pipeline), video_src_element, video_display_element, NULL);

    if(gst_element_link(video_src_element, video_display_element) != TRUE)
    {
        g_printerr ("Elements could not be linked.\n");
        gst_object_unref(run_pipeline);
        return -1;
    }

    g_object_set(video_src_element, "pattern", 19, NULL);
    ret = gst_element_set_state(run_pipeline, GST_STATE_PLAYING);

    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_printerr ("Unable to set the pipeline to the playing state.\n");
        gst_object_unref (run_pipeline);
        return -1;
    }

    bus = gst_element_get_bus(run_pipeline);
    msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

    if(msg != NULL)
        gst_message_unref(msg);

    gst_object_unref (bus);
    gst_element_set_state (run_pipeline, GST_STATE_NULL);
    gst_object_unref (run_pipeline);

    return 0;
}
