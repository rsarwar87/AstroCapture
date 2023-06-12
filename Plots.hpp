#ifndef __PLOTS__
#define __PLOTS__
#include <spdlog/spdlog.h>
#include "SystemInformation.hpp"
#include "circular_buffer.hpp"
#include "hello_imgui/hello_imgui.h"
#include "imgui_md_wrapper/imgui_md_wrapper.h"
#include "implot/implot.h"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "sys/sysinfo.h"
#include "sys/times.h"
#include "sys/types.h"
#include "timer.hpp"

typedef struct {
  CircularBuffer<float, 100> totalPhysMem;
  CircularBuffer<float, 100> processMemUsage;
  CircularBuffer<float, 100> totalCPUseage;
  CircularBuffer<float, 100> processCPUseage;
  CircularBuffer<float, 100> fps;
  CircularBuffer<float, 100> buffer;
  float max_phy;
  float max_fps;
  float max_buf;
} processMem_t;

class PlotWidget {
 public:
  PlotWidget() {
    timer.Start();
  }
  ~PlotWidget() {
  }
  void gui() { guiHelp(); }

 private:
  Timer timer;
  template <typename T>
  inline T RandomRange(T min, T max) {
    T scale = rand() / (T)RAND_MAX;
    return min + scale * (max - min);
  }
  void guiHelp() {
    static int offset = 30 % 100;
    offset = (offset + 1) % 100;
    static ImGuiTableFlags flags =
        ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV |
        ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable |
        ImGuiTableFlags_Reorderable;
    ImPlot::PushColormap(ImPlotColormap_Deep);
    if (ImGui::BeginTable("##table", 2, flags, ImVec2(-1, 0))) {
      ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 50.0f);
      ImGui::TableSetupColumn("Trace");
      ImGui::TableHeadersRow();
      ImPlot::PushColormap(ImPlotColormap_Cool);
      TablePlot("Total CPU", processStat.totalCPUseage.get_buffer(), 0); 
      TablePlot("Total RAM", processStat.totalPhysMem.get_buffer(), 0, processStat.max_phy); 
      TablePlot("Self CPU", processStat.processCPUseage.get_buffer(), 0); 
      TablePlot("Self RAM", processStat.processMemUsage.get_buffer(), 0, processStat.max_phy); 
      TablePlot("FPS", processStat.fps.get_buffer(), 0, 100); 
      TablePlot("FPS", processStat.fps.get_buffer(), 0, 100); 
      ImPlot::PopColormap();
      ImGui::EndTable();
    }
    if (timer.Finish() > 100) {
      update_sys_info();
      timer.Start();
    }
  }
  template <typename T>
  void TablePlot(std::string str, T* data, int row, float _max = 100) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("%s", str.c_str());
    ImGui::TableSetColumnIndex(1);
    ImGui::PushID(row);
    Sparkline("##spark", data, 100, 0, _max, 
              ImPlot::GetColormapColor(row), ImVec2(-1, 35));
    ImGui::PopID();
  }
  void Sparkline(const char* id, const float* values, int count, float min_v,
                 float max_v, const ImVec4& col,
                 const ImVec2& size) {
    ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(0, 0));
    if (ImPlot::BeginPlot(id, size,
                          ImPlotFlags_CanvasOnly | ImPlotFlags_NoChild)) {
      ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoDecorations,
                        ImPlotAxisFlags_NoDecorations);
      ImPlot::SetupAxesLimits(0, count - 1, min_v, max_v, ImGuiCond_Always);
      ImPlot::SetNextLineStyle(col);
      ImPlot::SetNextFillStyle(col, 0.25);
      ImPlot::PlotLine(id, values, count, 1, 0, ImPlotLineFlags_Shaded, 0);
      ImPlot::EndPlot();
    }
    ImPlot::PopStyleVar();
  }
  processMem_t processStat;
  ProcessInfo process;
  SystemInformation sys_info;
  void update_sys_info() {

    processStat.totalPhysMem.push(sys_info.GetTotalUsageMemory());
    processStat.totalCPUseage.push(sys_info.GetCpuTotalUsage());

    processStat.processCPUseage.push(process.GetCpuUsage());
    processStat.processMemUsage.push(process.GetMemoryUsage());

    processStat.max_phy = sys_info.GetTotalMemory();
  }

};

#endif
