
#pragma once

#include <libasi/ASICamera2.h>
#include "camera_base.hpp"

#include "asi_helpers.hpp"

class ASIBase : public CameraBase {
 public:
  ASIBase(ASI_CAMERA_INFO _CameraInfo) : CameraBase("ZWO") {
    mCameraInfo = _CameraInfo;
    print_camera_info();

    for (size_t i = 0; i < 8; i++) {
      if (mCameraInfo.SupportedVideoFormat[i] == ASI_IMG_END) break;
      m_supportedFormat.push_back(mCameraInfo.SupportedVideoFormat[i]);
      m_supportedFormat_str.push_back(
          ASIHelpers::toPrettyString(mCameraInfo.SupportedVideoFormat[i]));
    }
    for (size_t i = 0; i < 16; i++) {
      if (mCameraInfo.SupportedBins[i] > 0)
        m_supportedBin.push_back(std::to_string(mCameraInfo.SupportedBins[i]));
    }
  }

  ~ASIBase() { Disconnect(); }

  uint32_t getDeviceID() { return mCameraID; };

  bool Disconnect() {
    if (!is_connected) return true;
    spdlog::info("Attempting to Close {}...", mCameraName);
    if (is_running) {
      AbortExposure();
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    spdlog::info("Attempting to Close {}...", mCameraName);
    StopVideoCapture();
    StopExposure();
    ASI_ERROR_CODE ret = ASICloseCamera(mCameraInfo.CameraID);
    if (ret != ASI_SUCCESS) {
      spdlog::critical("Failed to close {}: {}", mCameraName,
                       ASIHelpers::toString(ret));
      return false;
    }
    spdlog::info("Camera is offline.");
    is_connected = false;
    return true;
  }
  bool Connect() {
    spdlog::info("Attempting to open {}...", mCameraName);
    ASI_ERROR_CODE ret = ASI_SUCCESS;
    mCameraID = mCameraInfo.CameraID;

    ret = ASIOpenCamera(mCameraID);
    if (ret != ASI_SUCCESS) {
      spdlog::critical("Failed to open {}: {}", mCameraName,
                       ASIHelpers::toString(ret));
      return false;
    }
    ret = ASIInitCamera(mCameraID);
    if (ret != ASI_SUCCESS) {
      spdlog::critical("Error Initializing the CCD {}: {}", mCameraName,
                       ASIHelpers::toString(ret));
      return false;
    }
    spdlog::info("Setting intital bandwidth to 40 on connection.");
    if ((ret = SetControlValue(ASI_BANDWIDTHOVERLOAD, 40, ASI_FALSE)) !=
        ASI_SUCCESS) {
      spdlog::critical("Failed to set initial bandwidth (%s).",
                       ASIHelpers::toString(ret));
    }
    spdlog::info("Stopping old instructions.");
    StopVideoCapture();
    StopExposure();

    spdlog::info("Successfully opened {}...", mCameraName);

    stillFrame.buffer = std::make_unique<uint8_t[]>(
        mCameraInfo.MaxHeight * mCameraInfo.MaxWidth * 3 * 2);
    spdlog::debug("Created still buffer {}...", mCameraName);
    is_connected = true;
    return true;
  }
  bool UpdateControls() {
    spdlog::info("Attempting to update controls for {}...", mCameraName);
    for (auto &cap : mControlCaps) {
      if (!cap.IsWritable) continue;
      CONTROL_CAPS_CAST *rcap =
          reinterpret_cast<CONTROL_CAPS_CAST *>(&cap);
      ASI_ERROR_CODE ret = ASI_SUCCESS;
      if (cap.ControlType == ASI_EXPOSURE) {
        ret = SetControlValue(
            rcap->ControlType, rcap->current_value * 1000,
            rcap->IsAutoSupported ? rcap->current_isauto : false);
      }
      else
      {
      ret =
          SetControlValue(rcap->ControlType, rcap->current_value,
                          rcap->IsAutoSupported ? rcap->current_isauto : false);
      }
      if (ret != ASI_SUCCESS) {
        spdlog::critical("Failed to set value for {} ({}).", rcap->Name,
                         ASIHelpers::toString(ret));
        return false;
      }
    }

    spdlog::info("Successfully update controls for {}...", mCameraName);
    return true;
  }
  bool RetrieveControls(bool is_create = false) {
    spdlog::info("Attempting to retrieve controls for {}...", mCameraName);
    for (auto &cap : mControlCaps) {
      if (cap.IsWritable == ASI_FALSE) continue;
      CONTROL_CAPS_CAST *rcap =
          reinterpret_cast<CONTROL_CAPS_CAST *>(&cap);
      auto curr = GetControlValue(rcap->ControlType);
      if (std::get<0>(curr) != ASI_SUCCESS)
        spdlog::critical("Failed to get value for {} ({}).", rcap->Name,
                         ASIHelpers::toString(std::get<0>(curr)));
      else {
        rcap->current_value = std::get<1>(curr);
        rcap->current_isauto = std::get<2>(curr);
      }
      if (cap.ControlType == ASI_EXPOSURE) {
        mExposureCap = rcap;
        if (is_create) mExposureCap->MinValue /= 1000.0;
        if (is_create) mExposureCap->MaxValue /= 1000.0;
        mExposureCap->Description[14] = 'm';
        mExposureCap->current_value = mExposureCap->current_value / 1000.0;
      }
    }

    int width = 0, height = 0, bin = 1;
    mCurrentStillFormat = ASI_IMG_RAW8;

    ASI_ERROR_CODE ret;
    ret =
        ASIGetROIFormat(mCameraID, &width, &height, &bin, &mCurrentStillFormat);

    if (ret != ASI_SUCCESS) {
      spdlog::critical("Failed to get ROI format (%s).",
                       ASIHelpers::toString(ret));
      return false;
    }
    spdlog::info(
        "CCD ID: {} Width: {} Height: {} Binning: {}x{} Image Type: {}",
        mCameraID, width, height, bin, bin,
        ASIHelpers::toString(mCurrentStillFormat));
    m_frame[0].DefaultValue = mCameraInfo.MaxHeight;
    m_frame[1].DefaultValue = mCameraInfo.MaxWidth;
    m_frame[0].MaxValue = mCameraInfo.MaxHeight;
    m_frame[1].MaxValue = mCameraInfo.MaxWidth;
    m_frame[0].CurrentValue = height * bin;
    m_frame[1].CurrentValue = width * bin;
    m_frame[0].BinnedValue = height;
    m_frame[1].BinnedValue = width;
    m_frame[0].Bin = bin;
    m_frame[1].Bin = bin;
    ret = ASIGetStartPos(mCameraID, &width, &height);
    m_frame[0].AxisOffset = height;
    m_frame[1].AxisOffset = width;
    m_frame[0].BinndedAxisOffset = height / bin;
    m_frame[1].BinndedAxisOffset = width / bin;
    if (ret != ASI_SUCCESS) {
      spdlog::critical("Failed to get ROI format (%s).",
                       ASIHelpers::toString(ret));
      return false;
    }
    spdlog::info("CCD ID: {} StartPos: {}x{}", mCameraID, width, height);
    spdlog::info("Successfully retrieved controls for {}...", mCameraName);
    return true;
  }
  bool CreateControls() {
    spdlog::info("Attempting to create controls for {}...", mCameraName);
    int piNumberOfControls = 0;
    ASI_ERROR_CODE ret;
    ret = ASIGetNumOfControls(mCameraID, &piNumberOfControls);
    if (ret != ASI_SUCCESS) {
      spdlog::critical("Failed to get number of controls ({}).",
                       ASIHelpers::toString(ret));
      return false;
    }
    mControlCaps.clear();
    mControlCaps.resize(piNumberOfControls);
    int i = 0;
    for (auto &cap : mControlCaps) {
      ret = ASIGetControlCaps(mCameraID, i++, reinterpret_cast<ASI_CONTROL_CAPS*>(&cap));
      if (ret != ASI_SUCCESS) {
        spdlog::critical("Failed to get control information ({}).",
                         ASIHelpers::toString(ret));
        return false;
      }

      CONTROL_CAPS_CAST *rcap =
          reinterpret_cast<CONTROL_CAPS_CAST *>(&cap);
      auto curr = GetControlValue(rcap->ControlType);
      if (std::get<0>(curr) != ASI_SUCCESS)
        spdlog::critical("Failed to get value ({}).",
                         ASIHelpers::toString(std::get<0>(curr)));
      else {
        rcap->current_value = std::get<1>(curr);
        rcap->current_isauto = std::get<2>(curr);
      }
      spdlog::debug(
          "Control #{}: name ({}), Descp ({}), Min ({}), Max ({}), Default "
          "Value ({}), IsAutoSupported ({}), "
          "isWritale ({}) ControlType ({}) CurrentValue ({}) IsAuto ({})",
          i, rcap->Name, rcap->Description, rcap->MinValue, rcap->MaxValue,
          rcap->DefaultValue, rcap->IsAutoSupported ? "True" : "False",
          rcap->IsWritable ? "True" : "False", rcap->ControlType,
          rcap->current_value, rcap->current_isauto ? "True" : "False");

      // Update Min/Max exposure as supported by the camera
    }

    spdlog::info("Successfully created controls for {}...", mCameraName);
    return true;
  }
  void AbortExposure() {
    if (!is_running) {
      spdlog::warn("Camera not runing...");
    }
    do_abort = true;
    spdlog::debug("Aaborted signal submitted...");
  }

  bool UpdateExposure() {
    ASI_ERROR_CODE ret = SetControlValue<float>(
        ASI_EXPOSURE, mExposureCap->current_value * 1000);
    if (ret != ASI_SUCCESS) {
      spdlog::critical("Failed to set exposure control ({}).",
                       ASIHelpers::toString(ret));
      return false;
    }
    spdlog::debug("Exposure Updated to {} ms", mExposureCap->current_value);
    return true;
  }
  bool DoVideoCapture() {
    if (is_running) {
      spdlog::debug("camera is busy IsRunning: {} IsStill: {}", is_running,
                    is_still);
      HelloImGui::Log(HelloImGui::LogLevel::Error, "camera is busy");
      return false;
    }
    SetCCDBin(BinNumber);
    if (!SetCCDROI()) {
      spdlog::critical("Failed to set ROI");
      return false;
    }
    if (!UpdateExposure()) {
      spdlog::critical("Failed to update Exposure");
      return false;
    }

    Timer escaped;
    Timer timer;
    ASI_ERROR_CODE ret = ASIStartVideoCapture(mCameraID);
    if (ret != ASI_SUCCESS) {
      spdlog::critical("Failed to start video capture {}",
                       ASIHelpers::toString(ret));
    }
    spdlog::info("Started video capture {} MB", max_buffer_size);
    HelloImGui::Log(HelloImGui::LogLevel::Debug, "Started video capture");

    timer.Start();
    escaped.Start();
    size_t count = 0;
    int waitMS = (mExposureCap->current_value) * 2 + 500;
    auto imgFormat = getImageFormat(mCurrentStillFormat);
    size_t nTotalBytes = std::get<1>(imgFormat)[0] * std::get<1>(imgFormat)[1] *
                         std::get<1>(imgFormat)[2] * std::get<2>(imgFormat);

    streamingFrames.buffer.reset();
    streamingFrames.buffer = std::make_shared<Circular_Buffer<uint8_t>>(
        max_buffer_size * 1024 * 1024 / nTotalBytes, nTotalBytes);
    streamingFrames.size = nTotalBytes;
    streamingFrames.ch = std::get<1>(imgFormat)[2];
    streamingFrames.byte_channel = std::get<2>(imgFormat);
    streamingFrames.format = SER::BAYER::COLOR_RGB;
    streamingFrames.currentFormat = mCurrentStillFormat;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    is_still = false;
    is_running = true;

    if (streamingFrames.ch == 1) {
      if (streamingFrames.ch)
        streamingFrames.format = SER::BAYER::COLOR_MONO;
      else {
        if (std::get<0>(imgFormat) == ASIHelpers::RGGB)
          streamingFrames.format = SER::BAYER::COLOR_BAYER_RGGB;
        else if (std::get<0>(imgFormat) == ASIHelpers::BGGR)
          streamingFrames.format = SER::BAYER::COLOR_BAYER_BGGR;
        else if (std::get<0>(imgFormat) == ASIHelpers::GRBG)
          streamingFrames.format = SER::BAYER::COLOR_BAYER_GRBG;
        else if (std::get<0>(imgFormat) == ASIHelpers::GBRG)
          streamingFrames.format = SER::BAYER::COLOR_BAYER_GBRG;
      }
    }

    streamingFrames.dim = {std::get<1>(imgFormat)[0], std::get<1>(imgFormat)[1],
                           std::get<1>(imgFormat)[2]};

    streamingFrames.is_active = true;
    int droppedcount = 0;
    uint8_t *targetFrame = nullptr;
    bool get_new_buffer = true;

    while (true) {
      if (do_abort) {
        spdlog::info("aborting .");
        StopVideoCapture();
        is_running = false;
        do_abort = false;
        streamingFrames.do_record = false;
        streamingFrames.is_active = false;
        spdlog::info("Exiting {}.", __func__);
        return true;
      }

      ASIGetDroppedFrames(mCameraID, &droppedcount);
      m_dropped_frames = droppedcount;
      if (timer.Finish() > 500) {
        m_fps = float(count) * 500. / float(timer.Finish());
        m_vc_escape = escaped.Finish();
        timer.Start();
        spdlog::debug("Capturing at {} fps. Dropped frame {}", m_fps,
                      m_dropped_frames);
        processStat.fps.push(m_fps);
        count = 0;
      }

      if (get_new_buffer) 
        targetFrame = streamingFrames.buffer->get_new_buffer();
      else
        get_new_buffer = true;
      if (targetFrame == nullptr) {
        std::this_thread::sleep_for(std::chrono::milliseconds(waitMS));
        continue;
      }

      ret = ASIGetVideoData(mCameraID, targetFrame, nTotalBytes, waitMS);
      if (ret != ASI_SUCCESS) {
        if (ret != ASI_ERROR_TIMEOUT) {
          spdlog::critical("ASIGetVideoData status timed out ({})",
                           ASIHelpers::toString(ret));
          StopVideoCapture();
          is_running = false;
          streamingFrames.do_record = false;
          streamingFrames.is_active = false;
          return false;
        }
        spdlog::critical("ASIGetVideoData status timed out ({})",
                         ASIHelpers::toString(ret));
        get_new_buffer = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        continue;
      }
      if (streamingFrames.currentFormat == ASI_IMG_RGB24)
        sort_rgb24(targetFrame, imgFormat);

      count++;
      //std::this_thread::sleep_for(std::chrono::milliseconds(10));

      // if (mCurrentVideoFormat == ASI_IMG_RGB24)
      //   for (uint32_t i = 0; i < totalBytes; i += 3)
      //       std::swap(targetFrame[i], targetFrame[i + 2]);
    }
    spdlog::info("Capture completed .");
    is_running = false;
    streamingFrames.do_record = false;
    while (streamingFrames.is_recording)
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    streamingFrames.is_active = false;

    return true;
  }
  bool DoCapture(/*boo subs_dark,*/ bool is_dark = false, size_t retry = 3) {
    if (is_running) {
      spdlog::debug("camera is busy IsRunning: {} IsStill: {}", is_running,
                    is_still);
      HelloImGui::Log(HelloImGui::LogLevel::Error, "camera is busy");
      return false;
    }

    SetCCDBin(BinNumber);
    if (!SetCCDROI()) {
      spdlog::critical("Failed to set ROI");
      return false;
    }
    if (!UpdateExposure()) {
      spdlog::critical("Failed to update Exposure");
      return false;
    }
    if (stillFrame.is_new) {
      stillFrame.is_new = false;
      spdlog::warn(
          "Previous frame not cleared, waiting 0.5 sec for things to "
          "settle...");
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    stillFrame.mutex.lock();
    int32_t expo_ms = mExposureCap->current_value;
    spdlog::debug("Starting {} msec exposure...", expo_ms);
    HelloImGui::Log(HelloImGui::LogLevel::Debug, "Starting %d msec exposure...",
                    expo_ms);
    stillFrame.currentFormat = mCurrentStillFormat;
    auto imgFormat = getImageFormat(stillFrame.currentFormat);
    size_t nTotalBytes = std::get<1>(imgFormat)[0] * std::get<1>(imgFormat)[1] *
                         std::get<1>(imgFormat)[2] * std::get<2>(imgFormat);

    Timer escaped;
    ASI_ERROR_CODE ret = ASI_SUCCESS;
    for (size_t i = 0; i < retry; i++) {
      ret = ASIStartExposure(mCameraID, is_dark ? ASI_TRUE : ASI_FALSE);
      if (ret == ASI_SUCCESS) break;
      spdlog::warn("Attempt {} to start exposure failed", i);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    if (ret != ASI_SUCCESS) {
      spdlog::critical("Failed to start exposure control ({}).",
                       ASIHelpers::toString(ret));
      return false;
    }
    spdlog::debug("Started {} sec exposure...", expo_ms / 1000.);

    escaped.Start();
    ASI_EXPOSURE_STATUS status = ASI_EXP_IDLE;
    m_expo_escape = static_cast<uint32_t>(expo_ms);
    int statRetry = 0;
    is_running = true;
    is_still = true;

    while (true) {
      if (do_abort) {
        spdlog::info("aborting .", mExposureRetry);
        StopExposure();
        is_running = false;
        do_abort = false;
        return true;
      }

      m_expo_escape = std::max(static_cast<int32_t>(expo_ms - escaped.Finish()),
                               static_cast<int32_t>(1));

      ret = ASIGetExpStatus(mCameraID, &status);
      if (ret != ASI_SUCCESS) {
        spdlog::warn("Failed to get exposure status (%s)",
                     ASIHelpers::toString(ret));
        if (++statRetry < 10) {
          std::this_thread::sleep_for(std::chrono::milliseconds(50));
          continue;
        }

        spdlog::critical("Exposure status timed out (%s)",
                         ASIHelpers::toString(ret));
        StopExposure();
        is_running = false;
        return false;
      }

      if (status == ASI_EXP_FAILED) {
        spdlog::critical("Exposure failed .", mExposureRetry);
        StopExposure();
        is_running = false;
        return false;
      }

      if (status == ASI_EXP_SUCCESS) break;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    spdlog::debug("Exposure successful .", mExposureRetry);

    ret = ASIGetDataAfterExp(mCameraID, stillFrame.buffer.get(), nTotalBytes);
    if (ret != ASI_SUCCESS) {
      spdlog::critical(
          "Failed to get data after exposure ({}x{} #{} channels, {} bytes) "
          "({}).",
          std::get<1>(imgFormat)[0], std::get<1>(imgFormat)[1],
          std::get<1>(imgFormat)[2], std::get<2>(imgFormat),
          ASIHelpers::toString(ret));
      return 0;
    }
    if (stillFrame.currentFormat == ASI_IMG_RGB24)
      sort_rgb24(stillFrame.buffer.get(), imgFormat);
    spdlog::debug(
        "Downloaded exposure... ({}x{} #{} channels, {} bytes; total: {} {}) ",
        std::get<1>(imgFormat)[0], std::get<1>(imgFormat)[1],
        std::get<1>(imgFormat)[2], std::get<2>(imgFormat), nTotalBytes, 
        ASIHelpers::toString(mCurrentStillFormat));
    stillFrame.size = nTotalBytes;
    stillFrame.ch = std::get<1>(imgFormat)[2];
    stillFrame.byte_channel = std::get<2>(imgFormat);
    stillFrame.format = SER::BAYER::COLOR_RGB;
    if (stillFrame.ch == 1) {
      if (stillFrame.ch)
        stillFrame.format = SER::BAYER::COLOR_MONO;
      else {
        if (std::get<0>(imgFormat) == ASIHelpers::RGGB)
          stillFrame.format = SER::BAYER::COLOR_BAYER_RGGB;
        else if (std::get<0>(imgFormat) == ASIHelpers::BGGR)
          stillFrame.format = SER::BAYER::COLOR_BAYER_BGGR;
        else if (std::get<0>(imgFormat) == ASIHelpers::GRBG)
          stillFrame.format = SER::BAYER::COLOR_BAYER_GRBG;
        else if (std::get<0>(imgFormat) == ASIHelpers::GBRG)
          stillFrame.format = SER::BAYER::COLOR_BAYER_GBRG;
      }
    }
    stillFrame.dim = {std::get<1>(imgFormat)[0], std::get<1>(imgFormat)[1],
                      std::get<1>(imgFormat)[2]};
    spdlog::debug("unlock ...");
    stillFrame.mutex.unlock();
    stillFrame.is_new = true;

    is_running = false;

    return true;
  }
  void sort_rgb24(
      uint8_t *ptr,
      std::tuple<ASIHelpers::PIXEL_FORMAT, std::array<uint16_t, 3>, size_t>
          imgFormat) {
    uint8_t *dstR = ptr;
    uint8_t *dstG = ptr + std::get<1>(imgFormat)[0] * std::get<1>(imgFormat)[1];
    uint8_t *dstB =
        ptr + std::get<1>(imgFormat)[0] * std::get<1>(imgFormat)[1] * 2;

    const uint8_t *src = ptr;
    const uint8_t *end =
        ptr + std::get<1>(imgFormat)[0] * std::get<1>(imgFormat)[1] * 3;

    while (src != end) {
      *dstB++ = *src++;
      *dstG++ = *src++;
      *dstR++ = *src++;
    }
  }
  bool SetCCDBin(uint8_t bin) {
    if (bin < 1) {
      spdlog::critical("Invalid bin request : {}", bin);
      return false;
    }
    for (size_t i = 0; i < 8; i++)
      if (bin == mCameraInfo.SupportedBins[i]) {
        m_frame[0].Bin = bin;
        m_frame[1].Bin = bin;
        m_frame[0].BinnedValue = m_frame[0].CurrentValue / bin;
        m_frame[1].BinnedValue = m_frame[1].CurrentValue / bin;
        m_frame[0].BinndedAxisOffset = m_frame[0].AxisOffset / bin;
        m_frame[1].BinndedAxisOffset = m_frame[1].AxisOffset / bin;
        spdlog::debug("Bin Set to: {}", bin);
        spdlog::debug("Bined Resolution to: {} x {}", m_frame[0].BinnedValue,
                      m_frame[1].BinnedValue);
        spdlog::debug("Bined Resolution to: {} x {}",
                      m_frame[0].BinndedAxisOffset,
                      m_frame[1].BinndedAxisOffset);
        return true;
      }
    spdlog::critical("Invalid bin request : {}", bin);

    return false;
  }
  bool SetCCDROI() {
    uint32_t binX = m_frame[1].Bin;
    uint32_t binY = m_frame[0].Bin;
    uint32_t subX = m_frame[1].BinndedAxisOffset;
    uint32_t subY = m_frame[0].BinndedAxisOffset;
    uint32_t subW = m_frame[1].BinnedValue;
    uint32_t subH = m_frame[0].BinnedValue;
    if (subW + subX > static_cast<uint32_t>(mCameraInfo.MaxWidth / binX)) {
      spdlog::critical("Invalid width request : {} @ offset {} ({})", subW,
                       subX, mCameraInfo.MaxWidth / binX);
      return false;
    }
    if (subH + subY > static_cast<uint32_t>(mCameraInfo.MaxHeight / binY)) {
      spdlog::critical("Invalid width request : {} @ offset {} (){})", subH,
                       subY, mCameraInfo.MaxHeight / binY);
      return false;
    }

    // ZWO rules are this: width%8 = 0, height%2 = 0
    // if this condition is not met, we set it internally to slightly smaller
    // values
    subW -= subW % 8;
    subH -= subH % 2;
    m_frame[1].BinnedValue = subW;
    m_frame[0].BinnedValue = subH;

    spdlog::debug("Frame ROI x:{} y:{} w:{} h:{}", subX, subY, subW, subH);

    ASI_ERROR_CODE ret;

    ret = ASISetROIFormat(mCameraID, subW, subH, binX, mCurrentStillFormat);
    if (ret != ASI_SUCCESS) {
      spdlog::critical("Failed to set ROI ({}).", ASIHelpers::toString(ret));
      return false;
    }

    ret = ASISetStartPos(mCameraID, subX, subY);
    if (ret != ASI_SUCCESS) {
      spdlog::critical("Failed to set start position ({}).",
                       ASIHelpers::toString(ret));
      return false;
    }

    // Set UNBINNED coords

    // Total bytes required for image buffer
    return true;
  }

  std::tuple<ASIHelpers::PIXEL_FORMAT, std::array<uint16_t, 3>, size_t>
  getImageFormat(ASI_IMG_TYPE type) {
    ASIHelpers::PIXEL_FORMAT pixel = ASIHelpers::pixelFormat(
        type, mCameraInfo.BayerPattern, mCameraInfo.IsColorCam);
    uint8_t dim = 3;
    if (pixel == ASIHelpers::PIXEL_FORMAT::MONO8 ||
        pixel == ASIHelpers::PIXEL_FORMAT::MONO16)
      dim = 1;
    size_t sz = 1;
    if (type == ASI_IMG_RAW16) sz = 2;

    return std::make_tuple(
        pixel,
        std::array<uint16_t, 3>{static_cast<uint16_t>(m_frame[0].BinnedValue),
                                static_cast<uint16_t>(m_frame[1].BinnedValue),
                                dim},
        sz);
  }

  bool StopExposure() {
    if (!is_running && !is_still) return true;
    ASI_ERROR_CODE ret = ASIStopExposure(mCameraInfo.CameraID);
    if (ret != ASI_SUCCESS) {
      spdlog::critical("Failed to stop exposure {}: {}", mCameraName,
                       ASIHelpers::toString(ret));
      return false;
    }
    spdlog::debug("Exposure aborted... ");
    return true;
  }
  bool StopVideoCapture() {
    ASI_ERROR_CODE ret = ASIStopVideoCapture(mCameraInfo.CameraID);
    //if (!is_running && is_still) return true;
    if (ret != ASI_SUCCESS) {
      spdlog::critical("Failed to stop video capture {}: {}", mCameraName,
                       ASIHelpers::toString(ret));
      return false;
    }
    spdlog::debug("VideoCapture aborted... ");
    return true;
  }

 private:
  std::tuple<ASI_ERROR_CODE, uint32_t, bool> GetControlValue(
      ASI_CONTROL_TYPE _type) {
    std::tuple<ASI_ERROR_CODE, uint32_t, bool> ret;
    ASI_BOOL isAuto = ASI_FALSE;
    long value = 0;
    ASI_ERROR_CODE _ret = ASIGetControlValue(mCameraID, _type, &value, &isAuto);
    bool _auto = false;
    isAuto == ASI_TRUE ? _auto = true : _auto = false;
    return std::make_tuple(_ret, value, _auto);
  }
  template <class T = uint32_t>
  ASI_ERROR_CODE SetControlValue(ASI_CONTROL_TYPE _type, T val,
                                 bool _auto = false) {
    return ASISetControlValue(mCameraID, _type, val,
                              _auto ? ASI_TRUE : ASI_FALSE);
  }

  void print_camera_info() {
    spdlog::debug("Name: {}; ID: {}", mCameraInfo.Name, mCameraInfo.CameraID);
    spdlog::debug("Resolution: {} x {}", mCameraInfo.MaxHeight,
                  mCameraInfo.MaxWidth);
    spdlog::debug("IsColor: {} BayerPattern {}",
                  mCameraInfo.IsColorCam ? "True" : "False",
                  ASIHelpers::toString(mCameraInfo.BayerPattern));
    spdlog::debug("SupportedBins: {} {} {} {} {}",
                  mCameraInfo.SupportedBins[0],  // 16
                  mCameraInfo.SupportedBins[1], mCameraInfo.SupportedBins[2],
                  mCameraInfo.SupportedBins[3], mCameraInfo.SupportedBins[4]);
    spdlog::debug("SupportedVideoFormat: {} {} {} {} {}",  // 8
                  ASIHelpers::toString(mCameraInfo.SupportedVideoFormat[0]),
                  ASIHelpers::toString(mCameraInfo.SupportedVideoFormat[1]),
                  ASIHelpers::toString(mCameraInfo.SupportedVideoFormat[2]),
                  ASIHelpers::toString(mCameraInfo.SupportedVideoFormat[3]),
                  ASIHelpers::toString(mCameraInfo.SupportedVideoFormat[4]));
    spdlog::debug("PixelSize: {}; BitDepth: {}", mCameraInfo.PixelSize,
                  mCameraInfo.BitDepth);
  }

 public:
  uint32_t mCameraID;
  ASI_CAMERA_INFO mCameraInfo;
  uint8_t mExposureRetry{0};
  ASI_IMG_TYPE mCurrentStillFormat;

  std::atomic_uint32_t m_expo_escape, m_vc_escape;


};
