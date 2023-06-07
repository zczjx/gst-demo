
#include <iostream>
#include <string>
#include <gst/gst.h>

int main(int argc, char ** argv)
{
    GstStateChangeReturn ret;
    GstBus *bus;
    GstMessage *msg;
    GstElement* video_src_element;
    GstElement* conv;
    GstElement* video_display_element;
    GstElement* run_pipeline;
    GstElement* app_src;
    int32_t val;
    gchar *caps_string;

    gst_init(&argc, &argv);

    video_src_element = gst_element_factory_make("videotestsrc", "video_src_element");
    app_src = gst_element_factory_make("appsrc", "app_src");
    conv = gst_element_factory_make ("videoconvert", "conv");
    video_display_element = gst_element_factory_make("autovideosink", "video_display_element");

    run_pipeline = gst_pipeline_new ("run_pipeline");

    if (!video_src_element || !conv || !video_display_element || !run_pipeline)
    {
        g_printerr ("Not all elements could be created.\n");
        return -1;
    }

    gst_bin_add_many(GST_BIN(run_pipeline), video_src_element, conv, video_display_element, NULL);

    if(gst_element_link_many(video_src_element, conv, video_display_element, NULL) != TRUE)
    {
        g_printerr ("Elements could not be linked.\n");
        gst_object_unref(run_pipeline);
        return -1;
    }

    g_object_set(video_src_element, "pattern", 19, NULL);
    g_object_get(video_src_element, "background-color", &val, NULL);
    g_printerr ("videotestsrc background-color: %u\n", val);

    GstPadTemplate *video_src_pad_templ = gst_element_get_pad_template(video_src_element, "src");
    GstCaps *video_src_pad_caps = 	gst_pad_template_get_caps(video_src_pad_templ);
    caps_string = gst_caps_to_string(video_src_pad_caps);
    g_printerr ("videotestsrc caps : %s\n", caps_string);

    g_printerr ("\n---------------------------------------------------------------\n");

    GstPadTemplate *app_src_pad_templ = gst_element_get_pad_template(app_src, "src");
    GstCaps *app_src_pad_caps = 	gst_pad_template_get_caps(app_src_pad_templ);
    caps_string = gst_caps_to_string(app_src_pad_caps);
    g_printerr ("app_src caps : %s\n", caps_string);
    g_object_set(G_OBJECT (app_src), "caps", video_src_pad_caps, NULL);

    GstCaps *temp_caps;
    g_object_get(app_src, "caps", temp_caps, NULL);
    caps_string = gst_caps_to_string(temp_caps);
    g_printerr ("app_src set caps : %s\n", caps_string);

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
