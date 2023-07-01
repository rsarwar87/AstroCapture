#ifndef __camera_window__
#define __camera_window__
#include <signal.h>
#include <spdlog/spdlog.h>
#include <unistd.h>

#include <charconv>
#include <cstdlib>
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
    for (auto const &[key, val] : loader.cameras) {
      names.push_back(std::to_string(key) + " " + val->getDefaultName());
      keys.push_back(key);
      HelloImGui::Log(HelloImGui::LogLevel::Info, "++ %s (%d)",
                      val->getDefaultName().c_str(), key);
      if (pCamera == nullptr) pCamera = val;
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
  static std::shared_ptr<ASICCD> pCamera;
  int camcurrent;
  std::vector<const char *> items_fmt;
  std::vector<const char *> items_bin;
  int mSysMem;
  size_t mTSysMem;

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
        if (pCamera.get() == nullptr) {
          HelloImGui::Log(HelloImGui::LogLevel::Error, "No Camera was found");
          return;
        }
        HelloImGui::Log(
            HelloImGui::LogLevel::Info, "Selected %s Camera",
            loader.cameras[keys[camcurrent]]->getDefaultName().c_str());
        pCamera = loader.cameras[keys[camcurrent]];
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
            if (pCamera.get() == nullptr) {
              HelloImGui::Log(HelloImGui::LogLevel::Error,
                              "No Camera was found");
              spdlog::error("No Camera was found");
              return;
            }
            pCamera->Connect();
            cameraState = CameraState::Connected;
            pCamera->CreateControls();
            pCamera->RetrieveControls(true);

            items_fmt.clear();
            items_fmt.reserve(pCamera->m_supportedFormat_str.size());
            for (size_t index = 0;
                 index < pCamera->m_supportedFormat_str.size(); ++index) {
              items_fmt.push_back(
                  pCamera->m_supportedFormat_str[index].c_str());
            }
            items_bin.clear();
            items_bin.reserve(pCamera->m_supportedBin.size());
            for (size_t index = 0; index < pCamera->m_supportedBin.size();
                 ++index) {
              items_bin.push_back(pCamera->m_supportedBin[index].c_str());
            }

            HelloImGui::Log(HelloImGui::LogLevel::Info,
                            "Camera %s (%d) connected",
                            names[camcurrent].c_str(), camcurrent);
          }
          break;
        case CameraState::Connected:
          if (ImGui::Button(ICON_FA_STOP_CIRCLE " Disconnect")) {
            pCamera->Disconnect();
            // camera.reset();
            cameraState = CameraState::Disconnected;
            HelloImGui::Log(HelloImGui::LogLevel::Info,
                            "Camera %s (%d) disconnected",
                            names[camcurrent].c_str(), camcurrent);
          }
          if (pCamera->is_running) cameraState = CameraState::Running;
          break;
        case CameraState::Running:
          ImGui::BeginDisabled();
          ImGui::Text(ICON_FA_ROCKET " Acquiring");
          ImGui::EndDisabled();
          if (!pCamera->is_running) cameraState = CameraState::Connected;
          break;
      }
    }
    if (pCamera.get() == nullptr) return;
    if (names.size() > 0)
      if (ImGui::CollapsingHeader("Camera info",
                                  ImGuiTreeNodeFlags_DefaultOpen))
        guiInfo();
    if (pCamera->is_connected) {
      if (ImGui::CollapsingHeader("Camera Control",
                                  ImGuiTreeNodeFlags_DefaultOpen))
        guiControl();
    }
    if (pCamera->is_connected) {
      if (ImGui::CollapsingHeader("Camera Resolution Control",
                                  ImGuiTreeNodeFlags_DefaultOpen))
        guiResolution();
    }
    if (pCamera->is_connected) {
      if (ImGui::CollapsingHeader(ICON_FA_WRENCH "Configuration",
                                  ImGuiTreeNodeFlags_DefaultOpen))
        guiAcquisition();
    }
  }
  enum class CameraState { Connected, Disconnected, Running };
  CameraState cameraState = CameraState::Disconnected;

  void guiInfo() {
    if (pCamera.get() == nullptr) return;
    ImGui::Text("Camera: %s", names[camcurrent].c_str());
    ImGui::Text("Resolution: %ld x %ld", pCamera->mCameraInfo.MaxHeight,
                pCamera->mCameraInfo.MaxWidth);
    ImGui::Text("IsColor: %s",
                pCamera->mCameraInfo.IsColorCam ? "True" : "False");
    if (pCamera->mCameraInfo.IsColorCam)
      ImGui::Text("BayerPattern: %s",
                  ASIHelpers::toString(pCamera->mCameraInfo.BayerPattern));
    ImGui::Text("PixelSize: %0.3f", pCamera->mCameraInfo.PixelSize);
    ImGui::Text("BitDepth: %d", pCamera->mCameraInfo.BitDepth);
  }
  void guiControl() {
    for (auto &cap : pCamera->mControlCaps) {
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
                         1, rcap->MaxValue / 100);
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
    if (pCamera->is_connected) {
      if (ImGui::Button(ICON_FA_THUMBS_UP " Apply Settings")) {
        if (!pCamera->UpdateControls())
          HelloImGui::Log(HelloImGui::LogLevel::Error,
                          "Failed to update settings.");
      }
      ImGui::SameLine();
      if (ImGui::Button(ICON_FA_THUMBS_DOWN " Revert Settings")) {
        if (!pCamera->RetrieveControls())
          HelloImGui::Log(HelloImGui::LogLevel::Error,
                          "Failed to update settings.");
      }
    }
  }
  void guiResolution() {
    ImGui::Text("Current Resolution {binned}: %dx%d",
                pCamera->m_frame[1].CurrentValue / pCamera->m_frame[1].Bin,
                pCamera->m_frame[0].CurrentValue / pCamera->m_frame[0].Bin);
    ImGui::Text("Current Offset {binned}: %dx%d",
                pCamera->m_frame[1].AxisOffset / pCamera->m_frame[1].Bin,
                pCamera->m_frame[0].AxisOffset / pCamera->m_frame[0].Bin);
    if (pCamera->is_running) ImGui::BeginDisabled();
    ImGui::SliderInt(
        "Width (X)", reinterpret_cast<int *>(&pCamera->m_frame[1].CurrentValue),
        pCamera->m_frame[1].MinValue,
        pCamera->m_frame[1].MaxValue - pCamera->m_frame[1].AxisOffset);
    ImGui::SliderInt(
        "Height (Y)",
        reinterpret_cast<int *>(&pCamera->m_frame[0].CurrentValue),
        pCamera->m_frame[0].MinValue,
        pCamera->m_frame[0].MaxValue - pCamera->m_frame[0].AxisOffset);
    ImGui::SliderInt(
        "Offset (X)", reinterpret_cast<int *>(&pCamera->m_frame[1].AxisOffset),
        pCamera->m_frame[1].MinValue,
        pCamera->m_frame[1].MaxValue - pCamera->m_frame[1].CurrentValue);
    ImGui::SliderInt(
        "Offset (Y)", reinterpret_cast<int *>(&pCamera->m_frame[0].AxisOffset),
        pCamera->m_frame[0].MinValue,
        pCamera->m_frame[0].MaxValue - pCamera->m_frame[0].CurrentValue);

    static int bin = 0, fmt = 0;
    if (ImGui::Combo("Format", &fmt, &items_fmt[0], items_fmt.size())) {
      HelloImGui::Log(
          HelloImGui::LogLevel::Info, "Format %s",
          ASIHelpers::toPrettyString(pCamera->m_supportedFormat[fmt]));
      pCamera->mCurrentStillFormat = pCamera->m_supportedFormat[fmt];
    }
    if (ImGui::Combo("Binning", &bin, &items_bin[0], items_bin.size())) {
      std::from_chars(pCamera->m_supportedBin[bin].data(),
                      pCamera->m_supportedBin[bin].data() +
                          pCamera->m_supportedBin[bin].size(),
                       pCamera->BinNumber);
      HelloImGui::Log(HelloImGui::LogLevel::Info, "Binning %d", pCamera->BinNumber);
      pCamera->SetCCDBin(pCamera->BinNumber);
    }

    if (pCamera->is_running) ImGui::EndDisabled();
  }
  void guiAcquisition() {
    namespace fs = std::filesystem;
    if (pCamera->is_running) ImGui::BeginDisabled();
    ImGui::SliderInt("Buffer Size", &mSysMem, 256, mTSysMem * 0.6);
    ImGui::InputText("Directory",
                     &(pCamera->getStreamingFramePtr()->selectedFilename[0]),
                     512);
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_SAVE "...")) {
      ifd::FileDialog::Instance().Open("DirectoryOpenDialog",
                                       "Open a directory", "");
    }
    if (pCamera->is_running) ImGui::EndDisabled();
    if (ifd::FileDialog::Instance().IsDone("DirectoryOpenDialog")) {
      std::string path_rec = "";
      if (ifd::FileDialog::Instance().HasResult())
        path_rec = ifd::FileDialog::Instance().GetResult().string();
      HelloImGui::Log(
          HelloImGui::LogLevel::Info, "Directory: %s",
          pCamera->getStreamingFramePtr()->selectedFilename.c_str());
      ifd::FileDialog::Instance().Close();
      //fs::perms p = fs::status(path_rec).permissions();
      static std::error_code ec;
      if (!fs::exists(path_rec)) {
        spdlog::error("{} does not exists", path_rec);
      } else if (!fs::is_directory(path_rec)) {
        spdlog::error("{} is not a directory", path_rec);
      //} else if ((fs::perms::none == (p & fs::perms::owner_write)) ||
      //           (fs::perms::none == (p & fs::perms::owner_write))) {
      //  spdlog::error("{} is not have read/write permission", path_rec);
      } else if (std::filesystem::space(path_rec, ec).available / 1024 / 1024 /
                     1024 <
                 1) {
        spdlog::error("{} needs atleast 1GB of free space. Currently has {} MB",
                      path_rec,
                      std::filesystem::space(path_rec, ec).available / 1024 / 1024);
      } else {
        pCamera->getStreamingFramePtr()->selectedFilename = path_rec;
        pCamera->getStreamingFramePtr()->aSpace =
            std::filesystem::space(path_rec, ec).capacity / 1024 / 1024;
        pCamera->getStreamingFramePtr()->fSpace =
            std::filesystem::space(path_rec, ec).available / 1024 / 1024;
      }
    }
    if (!pCamera->is_running) {
      if (ImGui::Button(ICON_FA_TV " Capture Frame")) {
        pCamera->DoCaptureHelper();
      }
      ImGui::SameLine();
      if (ImGui::Button(ICON_FA_ROCKET " Capture Video")) {
        pCamera->DoVCaptureHelper(mSysMem);
      }
    } else {
      if (ImGui::Button(ICON_FA_STOP " Abort")) {
        pCamera->AbortExposure();
      }
      if (pCamera->getStreamingFramePtr()->fSpace > 1) {
        ImGui::SameLine();
        ImGui::Checkbox(ICON_FA_STOP " Record",
                        &(pCamera->getStreamingFramePtr()->do_record));
      }
    }
  }
};
std::shared_ptr<ASICCD> CameraWindow::pCamera = nullptr;

#endif
