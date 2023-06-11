#ifndef __ViewPort__
#define __ViewPort__
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "AcqusitionHandler.hpp"
#include "hello_imgui/hello_imgui.h"
#include "imgui_md_wrapper/imgui_md_wrapper.h"
#include "inspector.hpp"
#include <spdlog/spdlog.h>
#include <memory>
#include "immvision/immvision.h"
#include <filesystem>

using namespace ImmVision;
class ViewPort  : public IVInspector {
 public:
  ViewPort(std::shared_ptr<AcqManager> _acqManage)
      : acqManage(_acqManage) {
    spdlog::info("Initializing: {}... {}", __func__, ResourcesDir() + "/house.jpg");

  }
  void gui() { guiHelp(); }

 private:
  std::shared_ptr<AcqManager> acqManage;
  void guiHelp(bool show_list = false) { 
    static bool is_first = false;
    if (!is_first) FillInspector();
    is_first = true;
    Inspector_Show(show_list); 
  }
std::string ResourcesDir()
{
    std::filesystem::path this_file(__FILE__);
    return ("/home/rsarwar/workspace/AstroCapture/build/resources");
}
void FillInspector()
{
    std::string zoomKey = "zk";
    auto image = cv::imread(ResourcesDir() + "/house.jpg");
    Inspector_AddImage(image, "Original", zoomKey);

    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    Inspector_AddImage(gray, "Gray", zoomKey);

    cv::Mat blur;
    cv::GaussianBlur(gray, blur, cv::Size(), 7.);
    Inspector_AddImage(blur, "Blur", zoomKey);

    cv::Mat floatMat;
    blur.convertTo(floatMat, CV_64FC1);
    floatMat = floatMat / 255.f;
    Inspector_AddImage(floatMat, "FloatMat", zoomKey);

}
};

#endif
