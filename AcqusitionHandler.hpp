#ifndef __AcquisitionManager__
#define __AcquisitionManager__
//#include <opencv2/opencv.hpp>

#include <chrono>
#include <mutex>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

#include "opencv2/imgproc.hpp"
//#include <opencv2/videoio.hpp>
#include <iostream>
#include <thread>

#include "Plots.hpp"
#include "SERProcessor.hpp"
#include "asi_base.hpp"
#include "hello_imgui/hello_imgui.h"
#include "imgui_md_wrapper/imgui_md_wrapper.h"
#include "spdlog/fmt/bundled/chrono.h"

class AcqManager {
 public:
  AcqManager() {
    abort_view = false;
    viewingThread = std::thread(AcqManager::HelperUpdateView, this);
    recordingThread = std::thread(AcqManager::HelperRecordStream, this);
  }
  void close_threads() {
    abort_view = true;
    spdlog::info("Waiting for AcqManager threads to end");
    recordingThread.join();
    viewingThread.join();
    spdlog::info("AcqManager threads to closed");
  }
  ~AcqManager() { close_threads(); }

 protected:
  cv::Mat mImage;
  std::mutex updatingFrame;
  int targetFPS = 0;
  int recordFPS = 1;

  static void HelperRecordStream(AcqManager* acq) {
    spdlog::info("RecordStream Thread started");
    acq->RecordStream();
  }
  static void HelperUpdateView(AcqManager* acq) {
    spdlog::info("UpdateView Thread started");
    acq->UpdateView();
  }

  void RecordStream() {
    auto now = std::chrono::system_clock::now();
    std::string fn;
    while (!abort_view) {
      if (CameraWindow::pCamera != nullptr) {
        if (CameraWindow::pCamera->is_connected) {
          if (CameraWindow::pCamera->is_running) {
            if (!CameraWindow::pCamera->is_still) {
              auto ptrS = CameraWindow::pCamera->getStreamingFramePtr();
              if (ptrS->do_record && ptrS->buffer != nullptr) {
                now = std::chrono::system_clock::now();
                fn = fmt::format("{}\{:%Y-%m-%d_%H-%M-%S}.ser",
                                 ptrS->selectedFilename, now);
                spdlog::info("Starting recording to {}", fn);
                ptrS->nCaptured = 0;
                std::unique_ptr<SER::SERWriter> writer =
                    std::make_unique<SER::SERWriter>(fn);
                std::array<size_t, 2> dims{ptrS->dim[0], ptrS->dim[1]};
                std::array<std::string, 3> strs{"ds", "dds", "asdwad"};
                writer->prepare_header(dims, strs, ptrS->byte_channel,
                                       ptrS->format);
                if (!writer->isOpen()) {
                  spdlog::critical("VideoWriter failed to open {}", fn);
                  continue;
                }
                while (ptrS->is_active && writer->isOpen()) {
                  auto buf = ptrS->buffer->dequeue();
                  if (buf != nullptr) {
                    if (ptrS->do_record) {
                      writer->write_frame(buf);
                      ptrS->nCaptured++;
                    }
                    ptrS->buffer->move_tail();
                  } else
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));

                  std::this_thread::sleep_for(std::chrono::milliseconds(10));
                  if (abort_view) {
                    spdlog::info("Stopped recording");
                    writer.reset();
                    break;
                  }
                }
                spdlog::info("Stopped recording");
                writer.reset();
              }
            }
          }
        }
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
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
                    timer.Finish() > (1 / ((uint32_t)(targetFPS)*10)) * 1000) {
                  timer.Start();
                  if (ptrS->buffer != nullptr) {
                    auto buf = ptrS->buffer->last();
                    if (buf != nullptr) {
                      updateImage<STILL_STREAMING_STRUCT>(ptrS, buf,
                                                          "VideoFrame");
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
              ptr->mutex.unlock();
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

  std::vector<cv::Mat> color_planes;
  std::thread recordingThread;
  std::thread viewingThread;
  template <class T = STILL_IMAGE_STRUCT>
  void updateImage(T* ptr, uint8_t* buf, std::string str = "StillFrame") {
   // static int histSize[] = {0, 256};
   // static int channels[] = {0, 1};
   // static float hranges[] = {0, 180};
   // static float sranges[] = {0, 256};
   // const float* ranges[] = {hranges, sranges};
   // cv::MatND hist;
    if (updatingFrame.try_lock()) {
      spdlog::debug("Got new {}, {} KB {} CH, {}x{} {} {}", str,
                    ptr->size / 1024, ptr->ch, ptr->dim[0], ptr->dim[1],
                    (ptr->byte_channel - 1) * 2,
                    CV_MAKETYPE((ptr->byte_channel - 1) * 2, ptr->ch));
      mImage = cv::Mat(ptr->dim[0], ptr->dim[1],
                       CV_MAKETYPE((ptr->byte_channel - 1) * 2, ptr->ch), buf);
      if (ptr->byte_channel == 2) {
        mImage = mImage / 255;
        mImage.convertTo(mImage, CV_MAKETYPE(0, ptr->ch));
      }
      //if (ptr->ch == 1) {
      //  cv::calcHist(&mImage, 1, channels, cv::Mat(),  // do not use mask
      //               hist /*processStat.hist[0]*/, 1, histSize, ranges,
      //               true,  // the histogram is uniform
      //               false);
      //  std::cout << processStat.hist[0] << hist << std::endl;
      //} else {
      //  std::cout << "s" << std::endl;
      //  cv::split(mImage, color_planes);
      //  //        cv::calcHist(&color_planes[0], 1, 0, cv::Mat(),
      //  //        processStat.histograms[0].get_buffer(), 1, 256, {0, 255},
      //  //        false, false); cv::calcHist(&color_planes[1], 1, 0, cv::Mat(),
      //  //        processStat.histograms[1].get_buffer(), 1, 256, {0, 255},
      //  //        false, false); cv::calcHist(&color_planes[2], 1, 0, cv::Mat(),
      //  //        processStat.histograms[2].get_buffer(), 1, 256, {0, 255},
      //  //        false, false);
      //}
      updatingFrame.unlock();
    }
  }
};

#endif
