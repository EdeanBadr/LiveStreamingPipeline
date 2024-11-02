// gstreamer_pipeline.cpp
#include "gstreamer_pipeline.hpp"
#include <iostream>
#include <gstreamer-1.0/gst/gstelement.h>


#define STUN_SERVER "stun://stun.l.google.com:19302"
namespace websocket = beast::websocket;
namespace net = boost::asio;

GStreamerPipeline::GStreamerPipeline() : pipeline(nullptr), webrtcbin(nullptr), ws_stream(nullptr) {}

GStreamerPipeline::~GStreamerPipeline() {
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
    }
}

void GStreamerPipeline::initialize(boost::beast::websocket::stream<boost::asio::ip::tcp::socket>* ws) {
    ws_stream = ws;
    GstStateChangeReturn ret;
    GError *error = nullptr;

    pipeline = gst_pipeline_new("pipeline");
    GstElement *v4l2src = gst_element_factory_make("v4l2src", "source");
    GstElement *videoconvert = gst_element_factory_make("videoconvert", "convert");
    GstElement *queue = gst_element_factory_make("queue", "queue");
    GstElement *vp8enc = gst_element_factory_make("vp8enc", "encoder");
    GstElement *rtpvp8pay = gst_element_factory_make("rtpvp8pay", "pay");
    webrtcbin = gst_element_factory_make("webrtcbin", "sendrecv");

    if (!pipeline || !v4l2src || !videoconvert || !queue || !vp8enc || 
        !rtpvp8pay || !webrtcbin) {
        g_printerr("Not all elements could be created.\n");
        return;
    }

    g_object_set(v4l2src, "device", "/dev/video0", nullptr);
    g_object_set(vp8enc, "deadline", 1, nullptr);

    gst_bin_add_many(GST_BIN(pipeline), v4l2src, videoconvert, queue, 
                     vp8enc, rtpvp8pay, webrtcbin, nullptr);

    if (!gst_element_link_many(v4l2src, videoconvert, queue, vp8enc, 
                              rtpvp8pay, nullptr)) {
        g_printerr("Elements could not be linked.\n");
        gst_object_unref(pipeline);
        return;
    }

    GstPad *rtp_src_pad = gst_element_get_static_pad(rtpvp8pay, "src");
    GstPad *webrtc_sink_pad = gst_element_get_request_pad(webrtcbin, "sink_%u");
    gst_pad_link(rtp_src_pad, webrtc_sink_pad);
    gst_object_unref(rtp_src_pad);
    gst_object_unref(webrtc_sink_pad);

    g_signal_connect(webrtcbin, "on-negotiation-needed", 
                    G_CALLBACK(on_negotiation_needed), this);
    g_signal_connect(webrtcbin, "on-ice-candidate", 
                    G_CALLBACK(on_ice_candidate), this);

    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the pipeline to the playing state.\n");
        gst_object_unref(pipeline);
        return;
    }

    std::cout << "GStreamer pipeline set to playing" << std::endl;
}

void GStreamerPipeline::handle_offer(const std::string& sdp) {
    GstSDPMessage *sdp_message;
    gst_sdp_message_new_from_text(sdp.c_str(), &sdp_message);
    GstWebRTCSessionDescription *offer = 
        gst_webrtc_session_description_new(GST_WEBRTC_SDP_TYPE_OFFER, sdp_message);
    
    GstPromise *promise = 
        gst_promise_new_with_change_func(on_set_remote_description, this, nullptr);
    g_signal_emit_by_name(webrtcbin, "set-remote-description", offer, promise);
    gst_webrtc_session_description_free(offer);
    
    std::cout << "Setting remote description" << std::endl;
}

void GStreamerPipeline::add_ice_candidate(guint mlineindex, const std::string& candidate) {
    g_signal_emit_by_name(webrtcbin, "add-ice-candidate", mlineindex, candidate.c_str());
    std::cout << "Added ICE candidate" << std::endl;
}

void GStreamerPipeline::on_negotiation_needed(GstElement *webrtc, gpointer user_data) {
    std::cout << "Negotiation needed" << std::endl;
}

void GStreamerPipeline::on_ice_candidate(GstElement *webrtc, guint mlineindex, 
                                       gchar *candidate, gpointer user_data) {
    auto *pipeline = static_cast<GStreamerPipeline*>(user_data);
    std::cout << "ICE candidate generated: mlineindex=" << mlineindex 
              << ", candidate=" << candidate << std::endl;

    object ice_json;
    ice_json["candidate"] = candidate;
    ice_json["sdpMLineIndex"] = mlineindex;

    object msg_json;
    msg_json["type"] = "candidate";
    msg_json["ice"] = ice_json;

    std::string text = serialize(msg_json);
    pipeline->ws_stream->write(net::buffer(text));
}

void GStreamerPipeline::on_answer_created(GstPromise *promise, gpointer user_data) {
    auto *pipeline = static_cast<GStreamerPipeline*>(user_data);
    GstWebRTCSessionDescription *answer = nullptr;
    const GstStructure *reply = gst_promise_get_reply(promise);
    gst_structure_get(reply, "answer", GST_TYPE_WEBRTC_SESSION_DESCRIPTION, 
                      &answer, nullptr);
    
    GstPromise *local_promise = gst_promise_new();
    g_signal_emit_by_name(pipeline->webrtcbin, "set-local-description", 
                         answer, local_promise);

    object sdp_json;
    sdp_json["type"] = "answer";
    sdp_json["sdp"] = gst_sdp_message_as_text(answer->sdp);
    std::string text = serialize(sdp_json);
    pipeline->ws_stream->write(net::buffer(text));

    std::cout << "Local description set and answer sent: " << text << std::endl;
    gst_webrtc_session_description_free(answer);
}

void GStreamerPipeline::on_set_remote_description(GstPromise *promise, gpointer user_data) {
    std::cout << "Remote description set, creating answer" << std::endl;

    auto *pipeline = static_cast<GStreamerPipeline*>(user_data);
    GstPromise *answer_promise = 
        gst_promise_new_with_change_func(on_answer_created, pipeline, nullptr);
    g_signal_emit_by_name(pipeline->webrtcbin, "create-answer", nullptr, answer_promise);
}
