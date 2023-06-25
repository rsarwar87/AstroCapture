=== Planetary captuer software


This app was primarily made to allow fast video capture on linux aarch64 platforms. But FireCapture, a popular linux video capture JAVA-based software was difficult to run due to limited resurces on SBCs.
This lead to the making of a CPP application targeting 64-bit architecture. The application was tested in RaspPi4 and Rock5b.

Recomended hardware:
* at least four CPU cores (hex-core recommended)
* at least 4 GB RAM DDR4 (dual-channel recommended)
* OpenGL/OpenGLES3 support (hardware acceleration recommended)
* 500 GB SSD (NVMe recommended)

For full speed download, a PCIe 2.0 NVMe is needed, these include Rock 3A and Rock 4SE and Rock 5B. It however runs okay on RPi4.

[I'd love to read your feedback!](https://github.com/rsarwar87/AstroCapture/issues)

== Building
'''
git clone
'''
== Acknowledgement

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

