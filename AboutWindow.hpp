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
This app was primarily made to allow fast video capture on linux aarch64 platforms. But FireCapture, a popular linux video capture JAVA-based software was difficult to run due to limited resurces on SBCs.
This lead to the making of a CPP application targeting 64-bit architecture. The application was tested in RaspPi4 and Rock5b.

Recomended hardware:
* at least four CPU cores (hex-core recommended)
* at least 4 GB RAM DDR4 (dual-channel recommended)
* OpenGL/OpenGLES3 support (hardware acceleration recommended)
* 500 GB SSD (NVMe recommended)

For full speed download, a PCIe 2.0 NVMe is needed, these include Rock 3A and Rock 4SE and Rock 5B. It however runs okay on RPi4.

[I'd love to read your feedback!](https://github.com/rsarwar87/AstroCapture/issues)
)";
        ImGuiMd::Render(help.c_str());
        if (isVisible != nullptr && ImGui::Button(ICON_FA_THUMBS_UP " Got it"))
            *isVisible = false;
    }
};


#endif
