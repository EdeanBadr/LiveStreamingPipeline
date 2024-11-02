// webrtc_server.hpp
#pragma once

#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/json.hpp>
#include <iostream>
#include <thread>
#include <gst/gst.h>
#include <gst/webrtc/webrtc.h>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;
using namespace boost::json;

#define SERVER_PORT 8000

class WebRTCServer {
public:
    WebRTCServer() = default;
    void start();

private:
    void handle_websocket_session(tcp::socket socket);
    void send_ice_candidate_message(websocket::stream<tcp::socket>& ws, 
                                  guint mlineindex, 
                                  gchar *candidate);
};
