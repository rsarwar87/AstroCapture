# Planetary captuer software


This app was primarily made to allow fast video capture on linux aarch64 platforms. But FireCapture, a popular linux video capture JAVA-based software was difficult to run due to limited resurces on SBCs.
This lead to the making of a CPP application targeting 64-bit architecture. The application was tested in RaspPi4 and Rock5b.

Recomended hardware:
* at least four CPU cores (hex-core recommended)
* at least 4 GB RAM DDR4 (dual-channel recommended)
* OpenGL/OpenGLES3 support (hardware acceleration recommended)
* 500 GB SSD over USB3 (NVMe recommended)

For full speed download, a PCIe 2.0 NVMe is needed, these include Rock 3A and Rock 4SE and Rock 5B. It however runs okay on RPi4.

[I'd love to read your feedback!](https://github.com/rsarwar87/AstroCapture/issues)

## Testing
I only have a ASI178, a 6MP camera. Only done testing of some features. Was able to offload 25 frames per sec using an NVMe. This camera is rated for 30 fps, generating 6 MB per frame or 180 MBps. Will update this section with full results.

#### Known Issue
* Does not work well with 16-bit captures.
## Building
```
sudo apt-get install libgles2-mesa libgles2-mesa-dev xorg-dev libusb-1.0-0-dev pkg-config libudev-dev libxi-dev libxrandr-dev 
git clone git@github.com:rsarwar87/AstroCapture.git --recurse-submodules
mkdir build
cd build
cmake ../ -DIMMVISION_FETCH_OPENCV=ON -DIMGUI_BUNDLE_WITH_SDL=ON
```

If i am missing any prerequisite libraries, please let me know and i'll add it in.


## Acknowledgement

This manual uses some great libraries, which are shown below.
* [OpenCV -- statically linked](https://opencv.org/)
* [SpdLog -- statically linked](https://github.com/gabime/spdlog).
* [SDL2 -- statically linked](https://github.com/libsdl-org/SDL).
* [SERUtils -- used as a template](https://github.com/artix75/SERUtils).
* [sys_info -- source included](https://github.com/SaulBerrenson/sys_info).
* [Dear ImGui](https://github.com/ocornut/imgui)
* [Hello ImGui](https://github.com/pthom/hello_imgui)
* [ImGui Bungle](https://github.com/pthom/imgui_bundle)
* [ImmVision](https://github.com/pthom/immvision/).
* [Implot](https://traineq.org/implot_demo/src/implot_demo.html)
* [ImFileDialog](https://github.com/pthom/ImFileDialog).
* ASI driver -- statically linked

This interactive GUI was developed using [Hello ImGui](https://github.com/pthom/hello_imgui), which provided the emscripten port, as well as the assets embedding and image loading. ImGuiManual.cpp gives a good overview of [Hello Imgui API](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/hello_imgui_api.md).

See also a related demo for 
* [Dear ImGui](https://raw.githubusercontent.com/wiki/ocornut/imgui/web/v167/v167-misc.png)
* [Hello ImGui](https://github.com/pthom/hello_imgui)
* [Implot](https://traineq.org/implot_demo/src/implot_demo.html)
* [ImmVision](https://traineq.org/ImGuiBundle/emscripten/bin/demo_immvision_launcher.html).

