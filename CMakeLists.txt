cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(webrtc_server VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-webrtc-1.0 gstreamer-sdp-1.0)
set(SOURCES
    src/main.cpp
    src/webrtc_server.cpp
    src/gstreamer_pipeline.cpp
)
set(HEADERS
    include/webrtc_server.hpp
    include/gstreamer_pipeline.hpp
)
find_package(Boost 1.65 REQUIRED COMPONENTS system filesystem json)

# Include directories
include_directories(${GST_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})


# Link libraries
target_link_libraries(webrtc_server ${GST_LIBRARIES} Boost::system Boost::filesystem Boost::json)

# Set properties
set_target_properties(webrtc_server PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)
include_directories(${CMAKE_SOURCE_DIR}/include)

# Specify additional directories for the linker
link_directories(${GST_LIBRARY_DIRS})

# Print project info
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Boost Libraries: ${Boost_LIBRARIES}")
message(STATUS "GStreamer Libraries: ${GST_LIBRARIES}")
