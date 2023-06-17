#include "AboutWindow.hpp"
#include "Acknowledgments.hpp"
#include "hello_imgui/hello_imgui.h"
#include "imgui_md_wrapper/imgui_md_wrapper.h"
#include "immapp/immapp.h"
#include "immapp/snippets.h"
//#include "hello_imgui/hello_imgui.h"
#include <iostream>
#include <memory>
#include <sstream>

#include "CameraWindow.hpp"
#include "HyperlinkHelper.hpp"
#include "ViewPort.hpp"
#include "Plots.hpp"
#include <memory>
// MyLoadFonts: demonstrate
// * how to load additional fonts
// * how to use assets from the local assets/ folder
//   Files in the application assets/ folder are embedded automatically
//   (on iOS/Android/Emscripten)
ImFont *gAkronimFont = nullptr;
void MyLoadFonts() {
  // First, we load the default fonts (the font that was loaded first is the
  // default font)
  HelloImGui::ImGuiDefaultSettings::LoadDefaultFont_WithFontAwesomeIcons();

  // Then we load a second font from
  // Since this font is in a local assets/ folder, it was embedded automatically
  std::string fontFilename = "fonts/Akronim-Regular.ttf";
  gAkronimFont =
      HelloImGui::LoadFontTTF_WithFontAwesomeIcons(fontFilename, 40.f);
}

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 2) {
  std::ostringstream out;
  out.precision(n);
  out << std::fixed << a_value;
  return std::move(out).str();
}

// Our Gui in the status bar
void StatusBarGui() {
  if (CameraWindow::pCamera.get() == nullptr) return;
  if (CameraWindow::pCamera->is_running) {
    if (CameraWindow::pCamera->is_still) {
      ImGui::SameLine();
      ImGui::Text("Exposure status:");
      ImGui::SameLine();
      ImGui::ProgressBar(1 - float(CameraWindow::pCamera->m_expo_escape) /
                                 float(CameraWindow::pCamera->mExposureCap->current_value),
                         HelloImGui::EmToVec2(12.f, 1.f));
      ImGui::SameLine();
      ImGui::Text("%d/%d", int(CameraWindow::pCamera->m_expo_escape),
                  int(CameraWindow::pCamera->mExposureCap->current_value));
      ImGui::SameLine();
      ImGui::Text(" ms");
    } else {
      ImGui::Text("Video buffer fullness: ");
      ImGui::SameLine();
      ImGui::ProgressBar(CameraWindow::pCamera->getStreamingFramePtr()->buffer->update_fullness(),
                         HelloImGui::EmToVec2(12.f, 1.f));

      ImGui::SameLine();
      ImGui::Text(
          "Video capture fps: %0.2f; Time Elapsed: %d ms; Dropped: %d frames",
          float(CameraWindow::pCamera->m_fps), int(CameraWindow::pCamera->m_vc_escape),
          int(CameraWindow::pCamera->m_dropped_frames));
    }
  }
}

void MenuBar(HelloImGui::RunnerParams &runnerParams) {
  runnerParams.imGuiWindowParams.showMenuBar = true;
  runnerParams.callbacks.ShowMenus = [&runnerParams] {
    HelloImGui::DockableWindow *aboutWindow =
        runnerParams.dockingParams.dockableWindowOfName("About");
    HelloImGui::DockableWindow *acknowledgmentWindow =
        runnerParams.dockingParams.dockableWindowOfName("Acknowledgments");
    if (ImGui::BeginMenu("Links and About")) {
      ImGui::TextDisabled("Links");
      if (ImGui::MenuItem("ImGui Github repository"))
        HyperlinkHelper::OpenUrl("https://github.com/ocornut/imgui");
      if (ImGui::MenuItem("ImGui wiki"))
        HyperlinkHelper::OpenUrl("https://github.com/ocornut/imgui/wiki");
      if (ImGui::MenuItem("imgui-docs: nice third party ImGui Documentation"))
        HyperlinkHelper::OpenUrl(
            "https://possiblyashrub.github.io/imgui-docs/");
      if (ImGui::MenuItem("imgui-manual"))
        HyperlinkHelper::OpenUrl("https://github.com/pthom/imgui_manual");
      if (ImGui::MenuItem("Online interactive imgui-manual"))
        HyperlinkHelper::OpenUrl(
            "https://pthom.github.io/imgui_manual_online/");

      ImGui::Separator();
      ImGui::TextDisabled("About this app");
      if (ImGui::MenuItem("About")) {
        aboutWindow->isVisible = true;
      }
      if (ImGui::MenuItem("Acknowledgments"))
        acknowledgmentWindow->isVisible = true;
      ImGui::EndMenu();
    }
  };
}

int main(int, char **) {
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Part 1: Define the application state, fill the status and menu bars, and
  // load additional font
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  PlotWidget plotWidget;
  ViewPort viewPort;
  CameraWindow cameraWindow;
  AboutWindow aboutWindow;
  Acknowledgments acknowledgments;
  // Our application state

  // Hello ImGui params (they hold the settings as well as the Gui callbacks)
  HelloImGui::RunnerParams runnerParams;

  runnerParams.appWindowParams.windowTitle = "AstroCapture";
  runnerParams.appWindowParams.windowGeometry.size = {800, 600};
  runnerParams.appWindowParams.restorePreviousGeometry = true;

  //
  // Status bar
  //
  // We use the default status bar of Hello ImGui
  runnerParams.imGuiWindowParams.showStatusBar = true;
  // uncomment next line in order to hide the FPS in the status bar
  // runnerParams.imGuiWindowParams.showStatus_Fps = false;
  runnerParams.callbacks.ShowStatus = [] {
    StatusBarGui();
  };

  MenuBar(runnerParams);

  // Custom load fonts
  runnerParams.callbacks.LoadAdditionalFonts = MyLoadFonts;

  // optional native events handling

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Part 2: Define the application layout and windows
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

  //
  //    2.1 Define the docking splits,
  //    i.e. the way the screen space is split in different target zones for the
  //    dockable windows
  //     We want to split "MainDockSpace" (which is provided automatically) into
  //     three zones, like this:
  //
  //    ___________________________________________
  //    |        |                                |
  //    | Left   |                                |
  //    | Space  |    MainDockSpace               |
  //    |        |                                |
  //    |        |                                |
  //    |        |                                |
  //    -------------------------------------------
  //    |     BottomSpace                         |
  //    -------------------------------------------
  //

  // First, tell HelloImGui that we want full screen dock space (this will
  // create "MainDockSpace")
  runnerParams.imGuiWindowParams.defaultImGuiWindowType =
      HelloImGui::DefaultImGuiWindowType::ProvideFullScreenDockSpace;
  // In this demo, we also demonstrate multiple viewports.
  // you can drag windows outside out the main window in order to put their
  // content into new native windows
  runnerParams.imGuiWindowParams.enableViewports = true;

  // Then, add a space named "BottomSpace" whose height is 25% of the app
  // height. This will split the preexisting default dockspace "MainDockSpace"
  // in two parts.
  HelloImGui::DockingSplit splitMainBottom;
  splitMainBottom.initialDock = "MainDockSpace";
  splitMainBottom.newDock = "BottomSpace";
  splitMainBottom.direction = ImGuiDir_Down;
  splitMainBottom.ratio = 0.25f;

  // Then, add a space to the left which occupies a column whose width is 25% of
  // the app width
  HelloImGui::DockingSplit splitMainLeft;
  splitMainLeft.initialDock = "MainDockSpace";
  splitMainLeft.newDock = "LeftSpace";
  splitMainLeft.direction = ImGuiDir_Left;
  splitMainLeft.ratio = 0.25f;

  // Finally, transmit these splits to HelloImGui
  runnerParams.dockingParams.dockingSplits = {splitMainBottom, splitMainLeft};

  //
  // 2.1 Define our dockable windows : each window provide a Gui callback, and
  // will be displayed
  //     in a docking split.
  //
  // Our gui providers for the different windows

  // A Command panel named "Commands" will be placed in "LeftSpace". Its Gui is
  // provided calls "CommandGui"
  // HelloImGui::DockableWindow pCameraWindow;
  //{
  //    pCameraWindow.label = "Camera";
  //    pCameraWindow.dockSpaceName = "LeftSpace";
  //    //pCameraWindow.GuiFunction = [&appState]() { CommandGui(appState); };
  //}
  // A Command panel named "Commands" will be placed in "LeftSpace". Its Gui is
  // provided calls "CommandGui"
  HelloImGui::DockableWindow commandsWindow;
  {
    commandsWindow.label = "Commands";
    commandsWindow.dockSpaceName = "LeftSpace";
    commandsWindow.GuiFunction = [&cameraWindow] { cameraWindow.gui(); };
  }
  // A Log  window named "Logs" will be placed in "BottomSpace". It uses the
  // HelloImGui logger gui
  HelloImGui::DockableWindow plotWindow;
  {
    plotWindow.label = "Statistics";
    plotWindow.dockSpaceName = "BottomSpace";
    plotWindow.GuiFunction = [&plotWidget] { plotWidget.gui(); };
  }
  HelloImGui::DockableWindow logsWindow;
  {
    logsWindow.label = "Logs";
    logsWindow.dockSpaceName = "BottomSpace";
    logsWindow.GuiFunction = [] { HelloImGui::LogGui(); };
  }
  // A Window named "Dear ImGui Demo" will be placed in "MainDockSpace"
  HelloImGui::DockableWindow captureWindow;
  {
    captureWindow.label = "ViewPort";
    captureWindow.dockSpaceName = "MainDockSpace";
    captureWindow.GuiFunction = [&viewPort] { viewPort.gui(); };
  }
  HelloImGui::DockableWindow dock_acknowledgments;
  {
    dock_acknowledgments.label = "Acknowledgments";
    dock_acknowledgments.dockSpaceName = "MainDockSpace";
    dock_acknowledgments.isVisible = false;
    dock_acknowledgments.includeInViewMenu = false;
    dock_acknowledgments.GuiFunction = [&acknowledgments] {
      acknowledgments.gui();
    };
  };
  HelloImGui::DockableWindow dock_about;
  {
    dock_about.label = "About";
    dock_about.dockSpaceName = "MainDockSpace";
    dock_about.isVisible = false;
    dock_about.includeInViewMenu = false;
    dock_about.GuiFunction = [&aboutWindow] { aboutWindow.gui(); };
  };
  // Finally, transmit these windows to HelloImGui
  runnerParams.dockingParams.dockableWindows = {
      dock_about, commandsWindow, logsWindow, plotWindow, 
      dock_acknowledgments, captureWindow};
  aboutWindow.isVisible =
      &(runnerParams.dockingParams.dockableWindowOfName("About")->isVisible);
  acknowledgments.isVisible =
      &(runnerParams.dockingParams.dockableWindowOfName("Acknowledgments")
            ->isVisible);
  //
  // Menu bar
  //
  // We use the default menu of Hello ImGui, to which we add some more items

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Part 3: Run the app
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  auto addons = ImmApp::AddOnsParams();
  addons.withMarkdown = true;
  addons.withNodeEditor = true;
  addons.withMarkdown = true;
  addons.withImplot = true;
  addons.withTexInspect = true;
          // Also clear ImmVision cache at exit (and before OpenGl is uninitialized)
  auto oldBeforeExitCopy = runnerParams.callbacks.BeforeExit;
  auto newBeforeExit = [&runnerParams, oldBeforeExitCopy]() {
      if (oldBeforeExitCopy)
          oldBeforeExitCopy();
      ImmVision::ClearTextureCache();
  };
  runnerParams.callbacks.BeforeExit = newBeforeExit;
  ImmApp::Run(runnerParams, addons);

  return 0;
u
