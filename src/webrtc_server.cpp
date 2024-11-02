// webrtc_server.cpp
#include "webrtc_server.hpp"
#include "gstreamer_pipeline.hpp"

void WebRTCServer::start() {
    try {
        net::io_context ioc{1};
        tcp::acceptor acceptor{ioc, tcp::endpoint{tcp::v4(), SERVER_PORT}};
        for (;;) {
            tcp::socket socket{ioc};
            acceptor.accept(socket);
            std::cout << "Accepted new TCP connection" << std::endl;
            std::thread{&WebRTCServer::handle_websocket_session, this, std::move(socket)}.detach();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

void WebRTCServer::send_ice_candidate_message(websocket::stream<tcp::socket>& ws, 
                                            guint mlineindex, 
                                            gchar *candidate) {
    std::cout << "Sending ICE candidate: mlineindex=" << mlineindex 
              << ", candidate=" << candidate << std::endl;

    object ice_json;
    ice_json["candidate"] = candidate;
    ice_json["sdpMLineIndex"] = mlineindex;

    object msg_json;
    msg_json["type"] = "candidate";
    msg_json["ice"] = ice_json;

    std::string text = serialize(msg_json);
    ws.write(net::buffer(text));

    std::cout << "ICE candidate sent" << std::endl;
}

void WebRTCServer::handle_websocket_session(tcp::socket socket) {
    try {
        websocket::stream<tcp::socket> ws{std::move(socket)};
        ws.accept();
        std::cout << "WebSocket connection accepted" << std::endl;

        GStreamerPipeline pipeline;
        pipeline.initialize(&ws);

        // Handle WebSocket messages
        for (;;) {
            beast::flat_buffer buffer;
            ws.read(buffer);

            auto text = beast::buffers_to_string(buffer.data());
            value jv = parse(text);
            object obj = jv.as_object();
            std::string type = obj["type"].as_string().c_str();

            if (type == "offer") {
                std::cout << "Received offer: " << text << std::endl;
                std::string sdp = obj["sdp"].as_string().c_str();
                pipeline.handle_offer(sdp);
            }
            else if (type == "candidate") {
                std::cout << "Received ICE candidate: " << text << std::endl;
                object ice = obj["ice"].as_object();
                std::string candidate = ice["candidate"].as_string().c_str();
                guint sdpMLineIndex = ice["sdpMLineIndex"].as_int64();
                pipeline.add_ice_candidate(sdpMLineIndex, candidate);
            }
        }
    }
    catch (const beast::system_error& se) {
        if (se.code() != websocket::error::closed)
            std::cerr << "Error: " << se.code().message() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}
