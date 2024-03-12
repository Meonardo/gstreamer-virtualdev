/// gstreamer headers
#include <gio/gio.h>
#include <glib.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>
#include <gst/gst.h>
#include <gst/rtsp/gstrtspmessage.h>
#include <gst/sdp/gstsdpmessage.h>
#include <gst/video/video-info.h>

#include <inttypes.h>
#include <signal.h>

#include <cstdlib>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>

#include "local-debug.h"

/* Logging */
int log_level = LOG_VERB;
gboolean log_timestamps = TRUE;
gboolean log_colors = TRUE, disable_colors = FALSE;

static void PrintElements(GstRank rank, GstElementFactoryListType type) {
  GList *elements, *l;
  elements = gst_element_factory_list_get_elements(type, rank);
  for (l = elements; l != nullptr; l = l->next) {
    GstPluginFeature* feature = GST_PLUGIN_FEATURE(l->data);
    GstElementFactory* factory =
        GST_ELEMENT_FACTORY(gst_plugin_feature_load(feature));
    if (factory) {
      LOGD("%s", GST_OBJECT_NAME(factory));
      gst_object_unref(factory);
    }
  }
  gst_plugin_feature_list_free(elements);
}

struct App {
  GstElement* pipeline = nullptr;
  GMainContext* context = nullptr;
  GMainLoop* loop = nullptr;
  GstState pipeline_state = GST_STATE_NULL;

  GstElement* app_sink = nullptr;
};

static bool update_pipeline_state(App* app, GstState new_state) {
  if (app->pipeline_state == new_state) {
    LOGI(
        "the original state is the same as the new state, no need to change\n");
    return true;
  }
  if (app->pipeline == nullptr) {
    LOGE("pipeline is null, please check again\n");
    return false;
  }

  // set the state of the pipeline to the desired state
  auto ret = gst_element_set_state(app->pipeline, new_state);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    LOGE("unable to set the pipeline to the state: %d, old state: %d\n",
         new_state, app->pipeline_state);
    return false;
  }
  LOGI("pipeline state changed to %s\n", gst_element_state_get_name(new_state));

  // remember the current state
  app->pipeline_state = new_state;

  return true;
}

static void on_bus_message(App* app, GstBus* bus, GstMessage* msg) {
  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_ERROR: {  // error message
      GError* err;
      gchar* debug_info;
      gchar* message_string;

      gst_message_parse_error(msg, &err, &debug_info);
      message_string = g_strdup_printf("error received from element %s: %s",
                                       GST_OBJECT_NAME(msg->src), err->message);
      g_clear_error(&err);
      g_free(debug_info);

      LOGE("app playing with error: %s\n", message_string);

      g_free(message_string);
      update_pipeline_state(app, GST_STATE_NULL);

      break;
    }
    case GST_MESSAGE_EOS: {  // end-of-stream
      LOGW("end of stream reached.\n");
      update_pipeline_state(app, GST_STATE_NULL);

      break;
    }
    case GST_MESSAGE_STATE_CHANGED: {
      GstState old_state, new_state, pending_state;
      gst_message_parse_state_changed(msg, &old_state, &new_state,
                                      &pending_state);
      // only pay attention to messages coming from the pipeline, not its
      // children
      if (GST_MESSAGE_SRC(msg) == GST_OBJECT(app->pipeline)) {
        gchar* message = g_strdup_printf("pipeline state changed to %s",
                                         gst_element_state_get_name(new_state));
        LOGW("App %s [cb]\n", message);
        g_free(message);
      }
      break;
    }
    default:
      break;
  }
}

static GstFlowReturn on_new_sample(GstElement* sink, App* app) {
  GstSample* sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));
  if (sample == nullptr) {
    LOGE("failed to get sample from appsink\n");
    return GST_FLOW_ERROR;
  }

  GstBuffer* buffer = gst_sample_get_buffer(sample);
  if (buffer == nullptr) {
    LOGE("failed to get buffer from sample\n");
    gst_sample_unref(sample);
    return GST_FLOW_ERROR;
  }

  GstMapInfo map;
  if (!gst_buffer_map(buffer, &map, GST_MAP_READ)) {
    LOGE("failed to map buffer\n");
    gst_sample_unref(sample);
    return GST_FLOW_ERROR;
  }

  LOGI("got buffer with size: %d\n", map.size);

  gst_buffer_unmap(buffer, &map);
  gst_sample_unref(sample);

  return GST_FLOW_OK;
}

static int init(App* app) {
  // create the main loop
  app->context = g_main_context_new();

  // create the pipeline
  auto pipeline_desc = g_strdup_printf(
      "videotestsrc is-live=true ! videoconvert ! queue !  "
      "video/x-raw,width=1920,height=1080,framerate=30/1 ! queue ! "
      "appsink name=appsink");
  GError* error = nullptr;
  app->pipeline = gst_parse_launch(pipeline_desc, &error);
  g_free(pipeline_desc);
  if (app->pipeline == nullptr || error != nullptr) {
    LOGE("failed to create pipeline, erorr: %s\n", error->message);
    return -11;
  }

  // register appsink callback
  app->app_sink = gst_bin_get_by_name(GST_BIN(app->pipeline), "appsink");
  if (app->app_sink == nullptr) {
    LOGE("app sink not found\n");
    return -12;
  }
  g_object_set(app->app_sink, "emit-signals", TRUE, nullptr);
  g_object_set(app->app_sink, "max-buffers", 1, NULL);
  g_object_set(app->app_sink, "drop", TRUE, NULL);

  g_signal_connect(app->app_sink, "new-sample", G_CALLBACK(on_new_sample), app);

  // set the pipeline to READY and return
  if (!update_pipeline_state(app, GST_STATE_READY)) {
    LOGE("failed to set pipeline to ready state\n");
    return -13;
  }
  return 0;  // success
}

static void deinit(App* app) {
  // quit main loop
  g_main_loop_quit(app->loop);
  // reset the pipeline state to NULL
  update_pipeline_state(app, GST_STATE_NULL);
  // free resources
  g_main_loop_unref(app->loop);
  app->loop = nullptr;
  gst_object_unref(app->app_sink);
  app->app_sink = nullptr;
  g_main_context_unref(app->context);
  app->context = nullptr;
  gst_object_unref(app->pipeline);
  app->pipeline = nullptr;
}

static void run(App* app) {
  // instruct the bus to emit signals for each received message,
  // and connect to the interesting signals
  GstBus* bus = gst_element_get_bus(app->pipeline);
  gst_bus_add_signal_watch(bus);
  g_signal_connect(
      G_OBJECT(bus), "message",
      G_CALLBACK(+[](GstBus* bus, GstMessage* message, App* ctx) -> void {
        on_bus_message(ctx, bus, message);
      }),
      app);

  // create the main loop thread
  LOGI("===============> App entering main loop...\n");
  app->loop = g_main_loop_new(app->context, FALSE);

  // start to receive data
  update_pipeline_state(app, GST_STATE_PLAYING);

  // run the main loop
  g_main_loop_run(app->loop);

  LOGI("<=============== App exited main loop\n");
  // clean up
  gst_bus_remove_signal_watch(bus);
  gst_object_unref(bus);
}

// app
static App* main_app = nullptr;

// signal handler
static volatile gint stop = 0;
static void handle_signal(int signum) {
  LOG(LOG_INFO, "Stopping...\n");
  if (g_atomic_int_compare_and_exchange(&stop, 0, 1)) {
    deinit(main_app);
  } else {
    g_atomic_int_inc(&stop);
    if (g_atomic_int_get(&stop) > 2)
      exit(1);
  }
}

int main() {
  /* Handle SIGINT (CTRL-C), SIGTERM (from service managers) */
  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);

  LOGI("GStreamer version: %s\n", gst_version_string());
  LOGI("GStreamer init...\n");

  // init gstreamer
  gst_init(nullptr, nullptr);

  LOGI("GStreamer initialized\n");

  // create app
  App app;
  main_app = &app;

  LOGI("App init...\n");
  // init app
  int ret = init(&app);
  if (ret != 0) {
    // can not init the app, exit it.
    return ret;
  }

  LOGI("App initialized\n");

  // create mainloop and run it
  run(&app);

  // destroy gstreamer
  gst_deinit();

  LOGI("App shutdown\n");

  return 0;
}