#ifndef __camera_window__
#define __camera_window__
#include <spdlog/spdlog.h>
#include <unistd.h>

#include <charconv>
#include <iterator>
#include <map>
#include <memory>
#include <opencv2/core/core.hpp>
#include <string>
#include <vector>

#include "ImFileDialog/ImFileDialog.h"
#include "asi_ccd.hpp"
#include "hello_imgui/hello_imgui.h"

class CameraWindow {
 public:
  CameraWindow() : camcurrent(0) {
    HelloImGui::Log(HelloImGui::LogLevel::Info, "Found %d Cameras",
                    loader.cameras.size());
    camera = nullptr;
    for (auto const &[key, val] : loader.cameras) {
      names.push_back(std::to_string(key) + " " + val->getDefaultName());
      keys.push_back(key);
      HelloImGui::Log(HelloImGui::LogLevel::Info, "++ %s (%d)",
                      val->getDefaultName().c_str(), key);
      if (camera == nullptr) camera = val;
    }
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    mTSysMem = page_size * pages / 1024 / 1024;
    spdlog::debug("Total System Memory: {}", mTSysMem);
    HelloImGui::Log(HelloImGui::LogLevel::Info, "Total System Memory: %d",
                    mTSysMem);
    mSysMem = 512;
  }
  void gui() { guiHelp(); }

  std::vector<std::string> names;
  std::vector<int> keys;
  std::shared_ptr<ASICCD> camera;
  int camcurrent;
  std::vector<const char *> items_fmt;
  std::vector<const char *> items_bin;
  int mSysMem;
  size_t mTSysMem;
  std::string selectedFilename;

  std::thread thCapture;

 private:
  void guiHelp() {
    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
      if (cameraState == CameraState::Connected ||
          cameraState == CameraState::Running)
        ImGui::BeginDisabled();
      if (ImGui::Combo("Camera", &camcurrent,
                       reinterpret_cast<const char **>(names.data()),
                       names.size())) {
        if (camera.get() == nullptr) {
          HelloImGui::Log(HelloImGui::LogLevel::Error, "No Camera was found");
          return;
        }
        HelloImGui::Log(
            HelloImGui::LogLevel::Info, "Selected %s Camera",
            loader.cameras[keys[camcurrent]]->getDefaultName().c_str());
        camera = loader.cameras[keys[camcurrent]];
        spdlog::info("Initializeded {} ", __func__);
        spdlog::debug("Selected {} Camera ",
                      loader.cameras[keys[camcurrent]]->getDefaultName());
      }

      if (cameraState == CameraState::Connected ||
          cameraState == CameraState::Running)
        ImGui::EndDisabled();
      switch (cameraState) {
        case CameraState::Disconnected:
          if (ImGui::Button(ICON_FA_LINK " Connect")) {
            if (camera.get() == nullptr) {
              HelloImGui::Log(HelloImGui::LogLevel::Error,
                              "No Camera was found");
              spdlog::error("No Camera was found");
              return;
            }
            camera->Connect();
            cameraState = CameraState::Connected;
            camera->CreateControls();
            camera->RetrieveControls(true);

            items_fmt.clear();
            items_fmt.reserve(camera->m_supportedFormat_str.size());
            for (size_t index = 0; index < camera->m_supportedFormat_str.size();
                 ++index) {
              items_fmt.push_back(camera->m_supportedFormat_str[index].c_str());
            }
            items_bin.clear();
            items_bin.reserve(camera->m_supportedBin.size());
            for (size_t index = 0; index < camera->m_supportedBin.size();
                 ++index) {
              items_bin.push_back(camera->m_supportedBin[index].c_str());
            }

            HelloImGui::Log(HelloImGui::LogLevel::Info,
                            "Camera %s (%d) connected",
                            names[camcurrent].c_str(), camcurrent);
          }
          break;
        case CameraState::Connected:
          if (ImGui::Button(ICON_FA_STOP_CIRCLE " Disconnect")) {
            camera->Disconnect();
            // camera.reset();
            cameraState = CameraState::Disconnected;
            HelloImGui::Log(HelloImGui::LogLevel::Info,
                            "Camera %s (%d) disconnected",
                            names[camcurrent].c_str(), camcurrent);
          }
          if (camera->is_running) cameraState = CameraState::Running;
          break;
        case CameraState::Running:
          ImGui::BeginDisabled();
          ImGui::Text(ICON_FA_ROCKET " Acquiring");
          ImGui::EndDisabled();
          if (!camera->is_running) cameraState = CameraState::Connected;
          break;
      }
    }
    if (camera.get() == nullptr) return;
    if (names.size() > 0)
      if (ImGui::CollapsingHeader("Camera info",
                                  ImGuiTreeNodeFlags_DefaultOpen))
        guiInfo();
    if (camera->is_connected) {
      if (ImGui::CollapsingHeader("Camera Control",
                                  ImGuiTreeNodeFlags_DefaultOpen))
        guiControl();
    }
    if (camera->is_connected) {
      if (ImGui::CollapsingHeader("Camera Resolution Control",
                                  ImGuiTreeNodeFlags_DefaultOpen))
        guiResolution();
    }
    if (camera->is_connected) {
      if (ImGui::CollapsingHeader(ICON_FA_WRENCH "Configuration",
                                  ImGuiTreeNodeFlags_DefaultOpen))
        guiAcquisition();
    }
  }
  enum class CameraState { Connected, Disconnected, Running };
  CameraState cameraState = CameraState::Disconnected;

  void guiInfo() {
    if (camera.get() == nullptr) return;
    ImGui::Text("Camera: %s", names[camcurrent].c_str());
    ImGui::Text("Resolution: %ld x %ld", camera->mCameraInfo.MaxHeight,
                camera->mCameraInfo.MaxWidth);
    ImGui::Text("IsColor: %s",
                camera->mCameraInfo.IsColorCam ? "True" : "False");
    if (camera->mCameraInfo.IsColorCam)
      ImGui::Text("BayerPattern: %s",
                  ASIHelpers::toString(camera->mCameraInfo.BayerPattern));
    ImGui::Text("PixelSize: %0.3f", camera->mCameraInfo.PixelSize);
    ImGui::Text("BitDepth: %d", camera->mCameraInfo.BitDepth);
  }
  void guiControl() {
    for (auto &cap : camera->mControlCaps) {
      if (cap.IsWritable == ASI_FALSE) continue;
      ASI_CONTROL_CAPS_CAST *rcap =
          reinterpret_cast<ASI_CONTROL_CAPS_CAST *>(&cap);
      // ImGui::SameLine();
      // ImGui::Text(": ");
      if (rcap->ControlType == ASI_FLIP) {
        const char *items[] = {"None", "Horizontal", "Vertical", "Both"};
        ImGui::Combo(rcap->Name,
                     reinterpret_cast<int *>(&(rcap->current_value)), items,
                     IM_ARRAYSIZE(items));
      } else if (rcap->ControlType == ASI_HIGH_SPEED_MODE ||
                 rcap->ControlType == ASI_HARDWARE_BIN) {
        ImGui::Checkbox(rcap->Name,
                        reinterpret_cast<bool *>(&rcap->current_value));
      } else if (rcap->ControlType == ASI_EXPOSURE) {
        ImGui::SliderInt(rcap->Name,
                         reinterpret_cast<int *>(&rcap->current_value),
                         rcap->MinValue / 1000, rcap->MaxValue / 1000);
      } else {
        ImGui::SliderInt(rcap->Name,
                         reinterpret_cast<int *>(&rcap->current_value),
                         rcap->MinValue, rcap->MaxValue);
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", rcap->Description);
      }
      if (rcap->IsAutoSupported) {
        ImGui::SameLine();
        ImGui::Checkbox("Auto", &rcap->current_isauto);
      }
    }

    // const char **items_fmt = reinterpret_cast<const char
    // **>(camera->m_supportedFormat_str.data()) ;
    if (camera->is_connected) {
      if (ImGui::Button(ICON_FA_THUMBS_UP " Apply Settings")) {
        if (!camera->UpdateControls())
          HelloImGui::Log(HelloImGui::LogLevel::Error,
                          "Failed to update settings.");
      }
      ImGui::SameLine();
      if (ImGui::Button(ICON_FA_THUMBS_DOWN " Revert Settings")) {
        if (!camera->RetrieveControls())
          HelloImGui::Log(HelloImGui::LogLevel::Error,
                          "Failed to update settings.");
      }
    }
  }
  void guiResolution() {
    ImGui::Text("Current Resolution {binned}: %dx%d",
                camera->m_frame[1].CurrentValue / camera->m_frame[1].Bin,
                camera->m_frame[0].CurrentValue / camera->m_frame[0].Bin);
    ImGui::Text("Current Offset {binned}: %dx%d",
                camera->m_frame[1].AxisOffset / camera->m_frame[1].Bin,
                camera->m_frame[0].AxisOffset / camera->m_frame[0].Bin);
    if (camera->is_running) ImGui::BeginDisabled();
    ImGui::SliderInt(
        "Width (X)", reinterpret_cast<int *>(&camera->m_frame[1].CurrentValue),
        camera->m_frame[1].MinValue,
        camera->m_frame[1].MaxValue - camera->m_frame[1].AxisOffset);
    ImGui::SliderInt(
        "Height (Y)", reinterpret_cast<int *>(&camera->m_frame[0].CurrentValue),
        camera->m_frame[0].MinValue,
        camera->m_frame[0].MaxValue - camera->m_frame[0].AxisOffset);
    ImGui::SliderInt(
        "Offset (X)", reinterpret_cast<int *>(&camera->m_frame[1].AxisOffset),
        camera->m_frame[1].MinValue,
        camera->m_frame[1].MaxValue - camera->m_frame[1].CurrentValue);
    ImGui::SliderInt(
        "Offset (Y)", reinterpret_cast<int *>(&camera->m_frame[0].AxisOffset),
        camera->m_frame[0].MinValue,
        camera->m_frame[0].MaxValue - camera->m_frame[0].CurrentValue);

    static int bin = 0, fmt = 0;
    if (ImGui::Combo("Format", &fmt, &items_fmt[0], items_fmt.size())) {
      HelloImGui::Log(
          HelloImGui::LogLevel::Info, "Format %s",
          ASIHelpers::toPrettyString(camera->m_supportedFormat[fmt]));
      camera->mCurrentStillFormat = camera->m_supportedFormat[fmt];
    }
    if (ImGui::Combo("Binning", &bin, &items_bin[0], items_bin.size())) {
      int number = 0;
      std::from_chars(camera->m_supportedBin[bin].data(),
                      camera->m_supportedBin[bin].data() +
                          camera->m_supportedBin[bin].size(),
                      number);
      HelloImGui::Log(HelloImGui::LogLevel::Info, "Binning %d", number);
      camera->SetCCDBin(number);
    }

    if (camera->is_running) ImGui::EndDisabled();
  }
  void guiAcquisition() {
    if (camera->is_running) ImGui::BeginDisabled();
    ImGui::SliderInt("Buffer Size", &mSysMem, 256, mTSysMem * 0.6);
    ImGui::InputText("Directory", &selectedFilename[0], 512);
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_SAVE "...")) {
      ifd::FileDialog::Instance().Open("DirectoryOpenDialog",
                                       "Open a directory", "");
    }
    if (camera->is_running) ImGui::EndDisabled();
    if (ifd::FileDialog::Instance().IsDone("DirectoryOpenDialog")) {
      if (ifd::FileDialog::Instance().HasResult())
        selectedFilename = ifd::FileDialog::Instance().GetResult().string();
      HelloImGui::Log(HelloImGui::LogLevel::Info, "Directory: %s",
                      selectedFilename.c_str());
      ifd::FileDialog::Instance().Close();
    }
    if (!camera->is_running) {
      if (ImGui::Button(ICON_FA_TV " Capture Frame")) {
        camera->DoCaptureHelper();
      }
      ImGui::SameLine();
      if (ImGui::Button(ICON_FA_ROCKET " Capture Video")) {
        camera->DoVCaptureHelper(mSysMem);
      }
    } else {
      if (ImGui::Button(ICON_FA_STOP " Abort")) {
        camera->AbortExposure();
      }
    }
  }
};

#endif
