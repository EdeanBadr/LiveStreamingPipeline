#define GST_USE_UNSTABLE_API
#include <gst/gst.h>
#include <gst/webrtc/webrtc.h>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/json.hpp>
#include <iostream>
#include <thread>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;
using namespace boost::json;

#define STUN_SERVER "stun://stun.l.google.com:19302"
#define SERVER_PORT 8000

class GStreamerPipeline {
public:
    GStreamerPipeline();
    ~GStreamerPipeline();

    void initialize(boost::beast::websocket::stream<boost::asio::ip::tcp::socket>* ws);
    void handle_offer(const std::string& sdp);
    void add_ice_candidate(guint mlineindex, const std::string& candidate);

private:
    static void on_negotiation_needed(GstElement *webrtc, gpointer user_data);
    static void on_ice_candidate(GstElement *webrtc, guint mlineindex,
                                gchar *candidate, gpointer user_data);
    static void on_answer_created(GstPromise *promise, gpointer user_data);
    static void on_set_remote_description(GstPromise *promise, gpointer user_data);

    GstElement *pipeline;
    GstElement *webrtcbin;
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket>* ws_stream;
};