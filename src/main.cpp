// main.cpp
#include "webrtc_server.hpp"
#include "gstreamer_pipeline.hpp"
#include <glib.h> 


int main(int argc, char *argv[])
{
    gst_init(&argc, &argv);
    std::cout << "Starting server on port " << SERVER_PORT << std::endl;
    
    WebRTCServer server;
    std::thread server_thread(&WebRTCServer::start, &server);

    GMainLoop *loop = g_main_loop_new(nullptr, FALSE);
    g_main_loop_run(loop);

    server_thread.join();
    gst_deinit();

    return 0;
}
