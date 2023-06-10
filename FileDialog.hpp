#ifndef __FIleDir__
#define __FIabout__
#include "hello_imgui/hello_imgui.h"
#include "imgui_utilities/MarkdownHelper.h"
#include "imgui_utilities/ImGuiFile.h"

class FileWindow {
public:
    FileWindow() {}
    void gui() { guiHelp(); }
    bool *isVisible = nullptr;

private:
    void guiHelp() {

            ImGuiFileDialog::Instance()->OpenDialog("ChooseDirDlgKey", "Choose a Directory", nullptr, "/home");

            // display and action if ok
            if (ImGuiFileDialog::Instance()->Display("ChooseDirDlgKey")) {
                if (ImGuiFileDialog::Instance()->IsOk()) {
                    std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                    std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
                    std::string filter = ImGuiFileDialog::Instance()->GetCurrentFilter();
                    // here convert from string because a string was passed as a userDatas, but it can be what you want
                    std::string userDatas;
                    if (ImGuiFileDialog::Instance()->GetUserDatas())
                        userDatas = std::string((const char *) ImGuiFileDialog::Instance()->GetUserDatas());
                    auto selection = ImGuiFileDialog::Instance()->GetSelection();// multiselection

                    // action
                  // close
               //   ImGuiFileDialog::Instance()->Close();
                }
            }
        if (isVisible != nullptr && ImGui::Button(ICON_FA_THUMBS_UP " Got it"))
            *isVisible = false;
    }
};


#endif
