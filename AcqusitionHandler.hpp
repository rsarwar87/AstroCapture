#ifndef __AcquisitionManager__
#define __AcquisitionManager__
//#include <opencv2/opencv.hpp>

#include <mutex>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <thread>

#include "hello_imgui/hello_imgui.h"
#include "imgui_md_wrapper/imgui_md_wrapper.h"

class AcqManager {
 public:
  AcqManager() {
    abort_view = false;
    viewingThread = std::thread(AcqManager::HelperUpdateView, this);
    recordingThread = std::thread(AcqManager::HelperRecordStream, this);
  }
  ~AcqManager() {
    abort_view = true;
    spdlog::info("Waiting for AcqManager threads to end");
    recordingThread.join();
    viewingThread.join();
    spdlog::info("AcqManager threads to closed");
  }

 protected:
  cv::Mat mImage;
  std::mutex updatingFrame;
  int targetFPS = 0;
  int recordFPS = 1;

  static void HelperRecordStream(AcqManager* acq) {
    spdlog::info("UpdateView Thread started");
    acq->RecordStream();
  }
  static void HelperUpdateView(AcqManager* acq) {
    spdlog::info("UpdateView Thread started");
    acq->UpdateView();
  }

  void RecordStream() {
  }

  void UpdateView() {
    while (!abort_view) {
      if (CameraWindow::pCamera != nullptr) {
        if (CameraWindow::pCamera->is_connected) {
          auto ptr = CameraWindow::pCamera->getImageFramePtr();
          if (CameraWindow::pCamera->is_running) {
            if (!CameraWindow::pCamera->is_still) {
              auto ptrS = CameraWindow::pCamera->getStreamingFramePtr();
              while (ptrS->is_active) {
                if (targetFPS == 0 ||
                    timer.Finish() > (1 / (targetFPS * 10)) * 1000) {
                  timer.Start();
                  if (ptrS->buffer != nullptr) {
                    auto buf = ptrS->buffer->last();
                    if (buf != nullptr) {
                      updateImage(ptr, buf, "VideoFrame");
                    }
                  }
                } else {
                  std::this_thread::sleep_for(
                      std::chrono::milliseconds(
                          ((1 / (targetFPS * 10)) * 1000) - timer.Finish()) *
                      0.75);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                if (abort_view) break;
              }
            }
          } else if (ptr->is_new) {
            if (ptr->mutex.try_lock()) {
              u_int8_t* buf = ptr->buffer.get();
              updateImage(ptr, buf);
              ptr->is_new = false;
            }
          }
        }
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  }

 private:
  Timer timer;
  bool abort_view = false;

  std::thread recordingThread;
  std::thread viewingThread;
  void updateImage(auto ptr, auto buf, std::string str = "StillFrame") {
    if (updatingFrame.try_lock()) {
      spdlog::debug("Got new {}, {} KB {} CH, {}x{}", str, ptr->size / 1024,
                    ptr->ch, ptr->dim[0], ptr->dim[1]);
      mImage = cv::Mat(ptr->dim[0], ptr->dim[1],
                       CV_MAKETYPE((ptr->byte_channel - 1) * 2, ptr->ch), buf);
      updatingFrame.unlock();
    }
  }
};

#endif
