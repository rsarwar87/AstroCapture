#ifndef __about__
#define __about__
#include "hello_imgui/hello_imgui.h"
#include "imgui_md_wrapper/imgui_md_wrapper.h"

class AboutWindow {
public:
    AboutWindow() {}
    void gui() { guiHelp(); }
    bool *isVisible = nullptr;

private:
    void guiHelp() {
        std::string help = R"(
Note: an online playground provided with this manual enables you to test ImGui without any installation:
* [see a demo](https://youtu.be/FJgObNNmuzo)
* [launch the playground](https://gitpod.io/#https://github.com/pthom/imgui_manual).

This interactive manual was developed using [Hello ImGui](https://github.com/pthom/hello_imgui), which provided the emscripten port, as well as the assets embedding and image loading. ImGuiManual.cpp gives a good overview of [Hello Imgui API](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui/hello_imgui_api.md).

See also a [related demo for Implot](https://traineq.org/implot_demo/src/implot_demo.html).

[I'd love to read your feedback!](https://github.com/pthom/imgui_manual/issues/1)
)";
        ImGuiMd::Render(help.c_str());
        if (isVisible != nullptr && ImGui::Button(ICON_FA_THUMBS_UP " Got it"))
            *isVisible = false;
    }
};


#endif
