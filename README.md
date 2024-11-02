# LiveStreamingPipeline
A live streaming pipeline built in C++ leveraging Boost.Beast for WebSocket communication and WebRTC for real-time video streaming.

![Interface Screenshot](/interface.png)  <!-- Replace with the actual path to your interface image -->

## Key Features
- Real-Time Streaming: Utilizes WebRTC for low-latency video streaming.
- WebSocket Integration: Employs Boost.Beast for handling WebSocket connections, enabling efficient two-way communication.
- Scalable Architecture: Designed to support multiple simultaneous video streams.
- Cross-Platform Compatibility: Built with C++ for compatibility across different operating systems.

## Installation

### Prerequisites
- C++11 or higher
- CMake
- Boost libraries (with Boost.Beast)
- WebRTC libraries
- GStreamer libraries

### Install GStreamer Libraries
Before building the project, install the required GStreamer libraries by running:
```bash
sudo apt-get update
sudo apt-get install -y \
    libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev \
    libgstreamer-plugins-bad1.0-dev \
    libnice-dev \
    gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
    gstreamer1.0-nice
```

### Getting Started
1. Clone the Repository:
   ```bash
   git clone https://github.com/EdeanBadr/LiveStreamingPipeline.git
   cd LiveStreamingPipeline
   ```
2. Create a Build Directory:
   ```bash
   mkdir build
   cd build
   ```
3. Configure the Project with CMake:
   ```bash
   cmake ..
   ```
4. Compile the Project:
   ```bash
   make
   ```

## Usage
To start the live streaming server, run:
```bash
./webrtc_server
```
Connect to the server using a compatible WebRTC client or browser-based application to begin streaming.
4. To serve the HTML interface, run:
   ```bash
   cd ..
   python3 -m http.server 9999
   ```


## Contributing
Contributions are welcome! If you have suggestions for improvements or want to add new features, please open an issue or submit a pull request.

## Acknowledgements
- [Boost.Beast](https://www.boost.org/doc/libs/release/libs/beast/doc/html/index.html)
- [WebRTC](https://webrtc.org/)
