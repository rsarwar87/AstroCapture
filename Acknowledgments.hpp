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
        std::string help = "This manual uses some great libraries, which are shown below.";
        ImGuiMd::Render(help.c_str());
        if (isVisible != nullptr && ImGui::Button(ICON_FA_THUMBS_UP " Got it"))
            *isVisible = false;
    }
};


#endif
