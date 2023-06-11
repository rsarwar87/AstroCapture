#ifndef __AcquisitionManager__
#define __AcquisitionManager__
//#include <opencv2/opencv.hpp>
#include "immdebug/immdebug.h"
#include <GL/gl.h>

#include "hello_imgui/hello_imgui.h"
#include "imgui_md_wrapper/imgui_md_wrapper.h"

class AcqManager {
 public:
  AcqManager() {}
  void gui() { guiHelp(); }

  std::tuple<intptr_t, ImVec2> getTexturePtr()
  {
      return std::make_tuple(static_cast<intptr_t>(texture), dim);
  }

 private:
  cv::Mat image;
  GLuint texture;
  ImVec2 dim;

  void guiHelp() {}
};

#endif
