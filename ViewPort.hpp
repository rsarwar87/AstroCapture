#ifndef __ViewPort__
#define __ViewPort__
#include <spdlog/spdlog.h>

#include <filesystem>
#include <memory>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "AcqusitionHandler.hpp"
#include "CameraWindow.hpp"
#include "hello_imgui/hello_imgui.h"
#include "imgui_md_wrapper/imgui_md_wrapper.h"
#include "immvision/immvision.h"
#include "inspector.hpp"
#include <iostream>

using namespace ImmVision;
class ViewPort : public IVInspector {
 public:
  ViewPort(std::shared_ptr<AcqManager> _acqManage) : acqManage(_acqManage) {}
  void gui() { guiHelp(); }

 private:
  cv::Mat mImage;
  Timer timer;
  std::shared_ptr<AcqManager> acqManage;
  void guiHelp() {
    static bool is_first = false;
    if (!is_first) {
      FillInspector();
      is_first = true;
      Inspector_Show(true);
    } else {
      UpdateInspector();
      Inspector_Show(true, &mImage);
    }
    GuiSobelParams();
  }
  void UpdateInspector() {
    std::string zoomKey = "zk";
    // process
    if (CameraWindow::pCamera != nullptr) {
      if (CameraWindow::pCamera->is_connected) {
        auto ptr = CameraWindow::pCamera->getImageFramePtr();
        if (CameraWindow::pCamera->is_running) {
          if (!CameraWindow::pCamera->is_still) {
            auto ptrS = CameraWindow::pCamera->getStreamingFramePtr();
            if (ptrS->is_active)
            {
              bool doprocess = true;
              if (mImageParams.targetFPS > 0)
              {
                if (timer.Finish() > (1/(mImageParams.targetFPS*10))*1000)
                {
                  timer.Start();
                  doprocess = true;
                }
              }
              if (doprocess && ptrS->buffer != nullptr)
              {
                auto buf = ptrS->buffer->dequeue();
                if (buf != nullptr)
                {
                  spdlog::info("Got New VFrame, {} KB {} CH, {}x{}",ptrS->size/1024,ptrS->ch, ptrS->dim[0], ptrS->dim[1]);
                  mImage = cv::Mat(ptr->dim[0], ptr->dim[1],  CV_MAKETYPE((ptr->byte_channel-1)*2,ptr->ch), buf);

                  ptrS->buffer->move_trail();
                }
              }
              
            }
          }
        } else if (ptr->is_new) {
          if (ptr->mutex.try_lock())
          {
            u_int8_t* buf = ptr->buffer.get();
            spdlog::debug("Got New Still Frame, {} KB {} CH, {}x{}",ptr->size/1024,ptr->ch, ptr->dim[0], ptr->dim[1]);
            mImage = cv::Mat(ptr->dim[0], ptr->dim[1],  CV_MAKETYPE((ptr->byte_channel-1)*2,ptr->ch), buf);
            ptr->is_new = false;
            ptr->mutex.unlock();
          }

        }
      }
    }
    auto item = mImageParams;
    item.Params.RefreshImage = true;
  }
  void FillInspector() {
    std::string zoomKey = "zk";
    mImage = cv::imread(std::filesystem::canonical("/proc/self/exe")
                            .remove_filename()
                            .string() +
                        "/assets//moon.jpg");
    Inspector_AddImage(mImage, "CapturedFrame", zoomKey);
  }
  bool GuiSobelParams() {
    bool changed = false;
    SobelParams& params = mImageParams.sParam;
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
    const char* items[] = {"None", "10 fps", "20 fps", "30 fps"};
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImmApp::EmSize() * 5);
    ImGui::Combo("PreviewFPS", &(mImageParams.targetFPS), items,
                 IM_ARRAYSIZE(items));
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImmApp::EmSize() * 5);
    ImGui::Combo("RecordingFPS", &(mImageParams.recordFPS),
                 items, IM_ARRAYSIZE(items));

    return changed;
  }
};

#endif
