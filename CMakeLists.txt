cmake_minimum_required(VERSION 3.12)
project(helloworld_with_helloimgui)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
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
set(HELLOIMGUI_USE_SDL_OPENGL3 ON CACHE STRING "" FORCE)
set(IMGUI_BUNDLE_FETCH_OPENCV OFF)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_module/")
include(GNUInstallDirs)
find_package(ASI REQUIRED)
find_package(spdlog REQUIRED)

##########################################################
# Build your app
##########################################################
# Call imgui_bundle_add_app
FILE(GLOB sources_imgui_astrocapture
    ${CMAKE_CURRENT_LIST_DIR}/*.h
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
    )
imgui_bundle_add_app(astrocapture ${sources_imgui_astrocapture})
target_link_libraries(astrocapture PRIVATE ASICamera2 spdlog fmt)

# Now you can build your app with
#     mkdir build && cd build && cmake .. && cmake --build .
