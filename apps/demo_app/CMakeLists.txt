cmake_minimum_required(VERSION 3.5)
project(demo_app)

# Default to C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()


# Find ffmpeg libraries
find_package(PkgConfig  REQUIRED)
pkg_search_module(PC_GST REQUIRED gstreamer-1.0) # equals `pkg-config --cflags --libs gstreamer-1.0`
pkg_search_module(PC_GST_APP REQUIRED gstreamer-app-1.0) # `pkg-config --cflags --libs gstreamer-app-1.0`


add_executable(
  demo_gst_app
  main.cpp
)


include_directories(
  ${PC_GST_INCLUDE_DIRS}
  ${PC_GST_APP_INCLUDE_DIRS}
)

target_link_libraries(
  demo_gst_app
  ${PC_GST_LDFLAGS}
  ${PC_GST_APP_LDFLAGS}
)
