#ifndef __ViewPort__
#define __ViewPort__
#include <spdlog/spdlog.h>

#include <filesystem>
#include <memory>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "AcqusitionHandler.hpp"
#include "hello_imgui/hello_imgui.h"
#include "imgui_md_wrapper/imgui_md_wrapper.h"
#include "immvision/immvision.h"
#include "inspector.hpp"

using namespace ImmVision;
class ViewPort : public IVInspector {
 public:
  ViewPort(std::shared_ptr<AcqManager> _acqManage) : acqManage(_acqManage) {
    spdlog::info("Initializing: {}... {}", __func__,
                 ResourcesDir() + "/house.jpg");
  }
  void gui() { guiHelp(); }

 private:
  std::shared_ptr<AcqManager> acqManage;
  void guiHelp() {
    static bool is_first = false;
    if (!is_first) {
      FillInspector();
      is_first = true;
      Inspector_Show(true);
    } else {
      Inspector_Show(true, &blur);
    }
    GuiSobelParams();
  }
  std::string ResourcesDir() {
    std::filesystem::path this_file(__FILE__);
    return ("/home/rsarwar/workspace/wkspace1/asi_planet/AstroCapture" +  std::string("/assets/"));
  }
  cv::Mat* UpdateInspector() {
    std::string zoomKey = "zk";
    auto image = cv::imread(ResourcesDir() + "/moon.jpg");
    static cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    auto item = s_Inspector_ImagesAndParams;
    item.Params.RefreshImage = true;
    return &gray;
  }
  cv::Mat blur;
  void FillInspector() {
    std::string zoomKey = "zk";
    auto image = cv::imread(ResourcesDir() + "/moon.jpg");
    Inspector_AddImage(image, "Original", zoomKey);

    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    Inspector_AddImage(gray, "Gray", zoomKey);

    cv::GaussianBlur(gray, blur, cv::Size(), 7.);
    Inspector_AddImage(blur, "Blur", zoomKey);

    cv::Mat floatMat;
    blur.convertTo(floatMat, CV_64FC1);
    floatMat = floatMat / 255.f;
    Inspector_AddImage(floatMat, "FloatMat", zoomKey);
    blur = image;
  }
  bool GuiSobelParams() {
    bool changed = false;
    SobelParams& params = s_Inspector_ImagesAndParams.sParam;
    // Blur size
    ImGui::SetNextItemWidth(ImmApp::EmSize() * 5);
    if (ImGui::SliderFloat("Blur size", &params.blur_size, 0.5f, 10.0f)) {
      changed = true;
    }
    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();

    // Deriv order
    ImGui::Text("Deriv order");
    ImGui::SameLine();
    for (int deriv_order = 1; deriv_order <= 4; ++deriv_order) {
      if (ImGui::RadioButton(std::to_string(deriv_order).c_str(),
                             params.deriv_order == deriv_order)) {
        changed = true;
        params.deriv_order = deriv_order;
      }
      ImGui::SameLine();
    }

    ImGui::Text(" | ");
    ImGui::SameLine();

    ImGui::Text("Orientation");
    ImGui::SameLine();
    if (ImGui::RadioButton("Horizontal",
                           params.orientation == Orientation::Horizontal)) {
      changed = true;
      params.orientation = Orientation::Horizontal;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Vertical",
                           params.orientation == Orientation::Vertical)) {
      changed = true;
      params.orientation = Orientation::Vertical;
    }
    const char *items[] = {"None", "10 fps", "20 fps", "30 fps"};
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImmApp::EmSize() * 5);
    ImGui::Combo("PreviewFPS",
                 &(s_Inspector_ImagesAndParams.targetFPS), items,
                 IM_ARRAYSIZE(items));
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImmApp::EmSize() * 5);
    ImGui::Combo("RecordingFPS",
                 &(s_Inspector_ImagesAndParams.recordFPS), items,
                 IM_ARRAYSIZE(items));

    return changed;
  }
};

#endif
