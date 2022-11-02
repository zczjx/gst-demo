#include <gst/gst.h>
#include <string>
#include <iostream>

int main(int argc, char ** argv)
{
    GstElement* pipeline;
    GError* error = NULL;
    GstStateChangeReturn ret;
    GstBus *bus;
    GstMessage *msg;

    gst_init(&argc, &argv);
    std::string m_strPipeline("videotestsrc pattern=19 ! autovideosink");

    pipeline = gst_parse_launch (m_strPipeline.c_str(), &error);

    if ( error != NULL )
    {
         printf ("Could not construct pipeline: %s", error->message);
         g_clear_error (&error);
    }

    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);

    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_printerr ("Unable to set the pipeline to the playing state.\n");
        gst_object_unref (pipeline);
        return -1;
    }

    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, (GstMessageType) (GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    if(msg != NULL)
        gst_message_unref(msg);

    gst_object_unref (bus);
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (pipeline);

    return 0;
}
