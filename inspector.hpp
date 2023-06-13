#ifndef __IVInspector__
#define __IVInspector__

#include "immapp/immapp.h"
#include "imgui.h"
#include "immvision/image.h"
//#include "immvision/inspector.h"
#include "immvision/internal/cv/zoom_pan_transform.h"
#include "immvision/internal/image_cache.h"
#include "immvision/internal/imgui/image_widgets.h"
#include <opencv2/core.hpp>
#include <string>
#include <iostream>
#ifndef IMMVISION_API
#define IMMVISION_API
#endif
#include <spdlog/spdlog.h>
using namespace ImmVision;
enum class Orientation
{
    Horizontal,
    Vertical
};

struct SobelParams
{
    float blur_size = 1.25f;
    int deriv_order = 1;  // order of the derivative
    int k_size = 7;  // size of the extended Sobel kernel it must be 1, 3, 5, or 7 (or -1 for Scharr)
    Orientation orientation = Orientation::Vertical;
};

struct Inspector_ImageAndParams {
  ImageCache::KeyType id;
  std::string Label;
  cv::Mat Image;
  ImageParams Params;

  cv::Point2d InitialZoomCenter = cv::Point2d();
  double InitialZoomRatio = 1.;
  bool WasSentToTextureCache = false;
  SobelParams sParam;
  int targetFPS = 0; 
  int recordFPS = 1; 
};

class IVInspector {
 public:
  IVInspector() {
    gInspectorImageSize = ImVec2(0., 0.);
    spdlog::info("Initializing: {}...",  __func__);
  }

  ImVec2 gInspectorImageSize;

  Inspector_ImageAndParams mImageParams;
  size_t s_Inspector_CurrentIndex = 0;


  void Inspector_AddImage(const cv::Mat& image, const std::string& legend,
        const std::string& zoomKey = "",
        const std::string& colormapKey = "",
        const cv::Point2d & zoomCenter = cv::Point2d(),
        double zoomRatio = -1.,
        bool isColorOrderBGR = true
        ) {
    ImageParams params;
    params.IsColorOrderBGR = isColorOrderBGR;
    params.ZoomKey = zoomKey;
    params.ColormapKey = colormapKey;
    params.ShowOptionsPanel = true;
    params.RefreshImage = true;

    if (gInspectorImageSize.x > 0.f)
      params.ImageDisplaySize =
          cv::Size((int)gInspectorImageSize.x, (int)gInspectorImageSize.y);

    std::string label =
        legend ;
    SobelParams sparam;
    mImageParams = 
        {0, label, image.clone(), params, zoomCenter, zoomRatio};

  }

  void priv_Inspector_ImageSize(bool showOptionsColumn) {
    ImVec2 imageSize;

    float emSize = ImGui::GetFontSize();
    float x_margin = emSize * 2.f;
    float y_margin = emSize / 3.f;
    float image_info_height = ImGui::GetFontSize() * 10.f;
      const auto& params = mImageParams.Params;
      if (!params.ShowImageInfo) image_info_height -= emSize * 1.5f;
      if (!params.ShowPixelInfo) image_info_height -= emSize * 1.5f;
    float image_options_width =
        showOptionsColumn ? ImGui::GetFontSize() * 19.f : 0.f;
    ImVec2 winSize = ImGui::GetWindowSize();
    imageSize = ImVec2(winSize.x - x_margin - image_options_width,
                       winSize.y - y_margin - image_info_height);
    if (imageSize.x < 1.f) imageSize.x = 1.f;
    if (imageSize.y < 1.f) imageSize.y = 1.f;

    gInspectorImageSize = imageSize;
    auto& i = mImageParams;
    {
      // Force image size
      i.Params.ImageDisplaySize = cv::Size((int)imageSize.x, (int)imageSize.y);
    }
  };

   void Inspector_Show(bool showOptionsColumn, cv::Mat * img = nullptr) {
    ImageWidgets::s_CollapsingHeader_CacheState_Sync = true;

    priv_Inspector_ImageSize(showOptionsColumn);
    {

        auto& imageAndParams = mImageParams;
        Image(imageAndParams.Label, img == nullptr ? imageAndParams.Image : *img,
              &imageAndParams.Params);
    }

    ImageWidgets::s_CollapsingHeader_CacheState_Sync = false;
  }

};
#endif
