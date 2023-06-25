cmake_minimum_required(VERSION 3.12)
project(helloworld_with_helloimgui)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g3")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

##########################################################
# Prepare imgui_bundle during configure time
##########################################################
# Download imgui_bundle
include(FetchContent)
Set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(
    imgui_bundle
    GIT_REPOSITORY https://github.com/pthom/imgui_bundle.git
    GIT_PROGRESS TRUE
    # Enter the desired git tag below
    GIT_TAG main  
)
FetchContent_MakeAvailable(imgui_bundle)

# Make cmake function `imgui_bundle_add_app` available
list(APPEND CMAKE_MODULE_PATH ${IMGUIBUNDLE_CMAKE_PATH})
include(imgui_bundle_add_app)

# Uncomment the next line if you which to also automatically fetch and compile OpenCV for immvision support
set(IMGUI_BUNDLE_WITH_SDL ON CACHE STRING "" FORCE)
set(HELLOIMGUI_USE_SDL_OPENGL3 OFF CACHE STRING "" FORCE)
set(IMGUI_BUNDLE_WITH_SDL OFF CACHE STRING "" FORCE)
set(HELLOIMGUI_USE_SDL_OPENGL3 OFF CACHE STRING "" FORCE)
set(IMGUI_BUNDLE_FETCH_OPENCV ON CACHE STRING "" FORCE)
set(IMMVISION_FETCH_OPENCV_FULL OFF CACHE STRING "" FORCE)
set(IMMVISION_FETCH_OPENCV ON CACHE STRING "" FORCE)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_module/")
include(GNUInstallDirs)
#find_package(ASI REQUIRED)
#find_package(spdlog REQUIRED)
find_package(PkgConfig REQUIRED)
#list(APPEND PKG_CONFIG_EXECUTABLE "--static")
    pkg_check_modules(libusb REQUIRED libusb-1.0)
    pkg_check_modules(libudev REQUIRED libudev)
add_subdirectory(external/libasi)
add_subdirectory(external/libfmt)
add_subdirectory(external/spdlog)
#pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
#    libavdevice
#    libavfilter
#    libavformat
#    libavcodec
#    libswresample
#    libswscale
#    libavutil
#)

#ADD_LIBRARY(libudev SHARED IMPORTED)
#SET_TARGET_PROPERTIES(libudev PROPERTIES IMPORTED_LOCATION /usr/./lib/x86_64-linux-gnu/libudev.so)
#ADD_LIBRARY(libusb STATIC IMPORTED)
#SET_TARGET_PROPERTIES(libusb PROPERTIES IMPORTED_LOCATION /usr/./lib/x86_64-linux-gnu/libusb-1.0.a)

#ADD_LIBRARY(libopencv_core STATIC IMPORTED)
#SET_TARGET_PROPERTIES(libopencv_core PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/build2/_deps/opencv_fetch-install/lib/libopencv_core.a")
#ADD_LIBRARY(libopencv_videoio STATIC IMPORTED)
#SET_TARGET_PROPERTIES(libopencv_videoio PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/build2/_deps/opencv_fetch-install/lib/libopencv_videoio.a")
#ADD_LIBRARY(libopencv_gapi STATIC IMPORTED)
#SET_TARGET_PROPERTIES(libopencv_gapi PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/build2/_deps/opencv_fetch-install/lib/libopencv_gapi.a")

##########################################################
# Build your app
##########################################################
# Call imgui_bundle_add_app
FILE(GLOB sources_imgui_astrocapture
    ${CMAKE_CURRENT_LIST_DIR}/*.hpp
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
    )
imgui_bundle_add_app(astrocapture ${sources_imgui_astrocapture})
#target_link_libraries(astrocapture PRIVATE ASICamera spdlog fmt::fmt udev usb-1.0 libopencv_core libopencv_gapi libopencv_videoio PkgConfig::LIBAV)
target_link_libraries(astrocapture PRIVATE ASICamera spdlog fmt::fmt udev usb-1.0 )

# Now you can build your app with
#     mkdir build && cd build && cmake .. && cmake --build .
