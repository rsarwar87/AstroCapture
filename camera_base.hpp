/*
    ASI CCD Driver

    Copyright (C) 2015-2021 Jasem Mutlaq (mutlaqja@ikarustech.com)
    Copyright (C) 2018 Leonard Bottleman (leonard@whiteweasel.net)
    Copyright (C) 2021 Pawel Soja (kernel32.pl@gmail.com)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
   USA
*/

#pragma once

#include <libasi/ASICamera2.h>
#include <spdlog/spdlog.h>

#include <atomic>
#include <chrono>
#include <cmath>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include "Plots.hpp"
#include "SERProcessor.hpp"
#include "asi_helpers.hpp"
#include "circular_buffer.hpp"
#include "hello_imgui/hello_imgui.h"
#include "timer.hpp"
#include "Plots.hpp"
#include "SERProcessor.hpp"
#include "circular_buffer.hpp"
#include "hello_imgui/hello_imgui.h"
#include "timer.hpp"
typedef struct CONTROL_CAPS_CAST {
  char Name[64];          // the name of the Control like Exposure, Gain etc..
  char Description[128];  // description of this control
  long MaxValue;
  long MinValue;
  long DefaultValue;
  ASI_BOOL IsAutoSupported;  // support auto set 1, don't support 0
  ASI_BOOL IsWritable;  // some control like temperature can only be read by
                        // some cameras
  ASI_CONTROL_TYPE
  ControlType;  // this is used to get value and set value of the control
  long current_value;
  bool current_isauto;
  char Unused[16];
} CONTROL_CAPS_CAST;
typedef struct _RESOLUTION_STRUCT {
  char Name[64];  // the name of the Control like Exposure, Gain etc..
  /*uint16_t*/ int MaxValue;
  /*uint16_t*/ int MinValue = 0;
  /*uint16_t*/ int DefaultValue;
  /*uint16_t*/ int CurrentValue;
  /*uint16_t*/ int AxisOffset;
  /*uint16_t*/ int BinnedValue;
  /*uint16_t*/ int BinndedAxisOffset;
  uint16_t Bin;
} RESOLUTION_STRUCT;
typedef struct _STILL_IMAGE_STRUCT {
  std::unique_ptr<uint8_t[]>
      buffer;  // using a smart pointer is safer (and we don't
  ASI_IMG_TYPE currentFormat;
  size_t size = 0;
  size_t ch = 1;
  size_t byte_channel = 1;
  SER::BAYER format;
  std::array<size_t, 3> dim;

  std::mutex mutex;
  std::atomic_bool is_new = false;
} STILL_IMAGE_STRUCT;
typedef struct _STILL_STREAMING_STRUCT {
  std::shared_ptr<Circular_Buffer<uint8_t>> buffer =
      nullptr;  // using a smart pointer is safer (and we don't
  ASI_IMG_TYPE currentFormat;
  size_t size = 0;
  size_t ch = 1;
  size_t byte_channel = 1;
  SER::BAYER format;
  std::array<size_t, 3> dim;

  bool do_record = false;
  std::atomic_bool is_recording = false;
  std::atomic_bool is_active = false;
  std::string selectedFilename =
      "/home/rsarwar/workspace/wkspace1/asi_planet/AstroCapture/build2/";
  std::atomic_uint32_t nCaptured;
  size_t fSpace = 0;
  size_t aSpace = 0;
} STILL_STREAMING_STRUCT;

typedef struct _ASI_CONTROL_CAPS_CAST {
  char Name[64];          // the name of the Control like Exposure, Gain etc..
  char Description[128];  // description of this control
  long MaxValue;
  long MinValue;
  long DefaultValue;
  ASI_BOOL IsAutoSupported;  // support auto set 1, don't support 0
  ASI_BOOL IsWritable;  // some control like temperature can only be read by
                        // some cameras
  ASI_CONTROL_TYPE
  ControlType;  // this is used to get value and set value of the control
  long current_value;
  bool current_isauto;
  char Unused[16];
} ASI_CONTROL_CAPS_CAST;

class CameraBase {
 public:
  CameraBase(std::string _vendor) {
    is_connected = false;
    is_running = false;
    is_still = false;
    do_abort = false;
    mVendorName = _vendor;
  }

  ~CameraBase() {  }

 public:
  std::string getVendorName() { return mVendorName; };
  std::string getDefaultName() { return mCameraName; };
  std::string getDevName() { return mCameraName; }



  STILL_IMAGE_STRUCT stillFrame;
  STILL_STREAMING_STRUCT streamingFrames;

  STILL_IMAGE_STRUCT *getImageFramePtr() { return &stillFrame; };
  STILL_STREAMING_STRUCT *getStreamingFramePtr() { return &streamingFrames; };
  std::string mVendorName;
  std::string mCameraName;
  size_t max_buffer_size = 512;
  ASI_IMG_TYPE mCurrentStillFormat;

  std::atomic_bool is_running;
  std::atomic_bool is_connected;
  std::atomic_bool is_still;
  std::atomic_bool do_abort;
  std::atomic_uint32_t m_dropped_frames;
  std::atomic<float> m_fps;

  std::array<RESOLUTION_STRUCT, 2> m_frame;
  std::vector<std::string> m_supportedFormat_str;
  std::vector<ASI_IMG_TYPE> m_supportedFormat;
  std::vector<std::string> m_supportedBin;
  uint8_t BinNumber = 1;

  std::vector<CONTROL_CAPS_CAST> mControlCaps;
  CONTROL_CAPS_CAST *mExposureCap;

};
