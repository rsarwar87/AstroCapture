#ifndef __acknowlege__
#define __acknowlege__
#include "hello_imgui/hello_imgui.h"
#include "imgui_md_wrapper/imgui_md_wrapper.h"

class Acknowledgments {
public:
    Acknowledgments() {}
    void gui() { guiHelp(); }
    bool *isVisible = nullptr;

private:
    void guiHelp() {
        std::string help = R"(
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
)";
        ImGuiMd::Render(help.c_str());
        if (isVisible != nullptr && ImGui::Button(ICON_FA_THUMBS_UP " Got it"))
            *isVisible = false;
    }
};


#endif
