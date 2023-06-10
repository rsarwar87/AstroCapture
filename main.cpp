#include "immapp/immapp.h"
#include "hello_imgui/hello_imgui.h"
#include "immapp/snippets.h"
#include "imgui_md_wrapper/imgui_md_wrapper.h"
#include "AboutWindow.hpp"
#include "Acknowledgments.hpp"
//#include "hello_imgui/hello_imgui.h"
#include "HyperlinkHelper.hpp"
#include "CameraWindow.hpp"


#include <sstream>

// Struct that holds the application's state
struct AppState {
    float f = 0.0f;
    int counter = 0;

    enum class RocketState {
        Init,
        Preparing,
        Launched
    };
    float rocket_progress = 0.f;
    RocketState rocketState = RocketState::Init;
};

// MyLoadFonts: demonstrate
// * how to load additional fonts
// * how to use assets from the local assets/ folder
//   Files in the application assets/ folder are embedded automatically
//   (on iOS/Android/Emscripten)
ImFont *gAkronimFont = nullptr;
void MyLoadFonts() {
    // First, we load the default fonts (the font that was loaded first is the default font)
    HelloImGui::ImGuiDefaultSettings::LoadDefaultFont_WithFontAwesomeIcons();

    // Then we load a second font from
    // Since this font is in a local assets/ folder, it was embedded automatically
    std::string fontFilename = "fonts/Akronim-Regular.ttf";
    gAkronimFont = HelloImGui::LoadFontTTF_WithFontAwesomeIcons(fontFilename, 40.f);
}


// CommandGui: the widgets on the left panel
void CommandGui(AppState &state) {
    ImGui::PushFont(gAkronimFont);
    ImGui::Text("Hello  " ICON_FA_SMILE);
    HelloImGui::ImageFromAsset("world.jpg");
    ImGui::PopFont();
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "The custom font and the globe image below were loaded \n"
            "from the application assets folder\n"
            "(those files are embedded automatically).");
    }

    ImGui::Separator();

    // Edit 1 float using a slider from 0.0f to 1.0f
    if (ImGui::SliderFloat("float", &state.f, 0.0f, 1.0f))
        HelloImGui::Log(HelloImGui::LogLevel::Warning, "state.f was changed to %f", state.f);

    // Buttons return true when clicked (most widgets return true when edited/activated)
    if (ImGui::Button("Button")) {
        state.counter++;
        HelloImGui::Log(HelloImGui::LogLevel::Info, "Button was pressed", state.f);
    }
    ImGui::SameLine();
    ImGui::Text("counter = %d", state.counter);

    switch (state.rocketState) {
        case AppState::RocketState::Init:
            if (ImGui::Button(ICON_FA_ROCKET " Launch rocket")) {
                state.rocketState = AppState::RocketState::Preparing;
                HelloImGui::Log(HelloImGui::LogLevel::Warning, "Rocket is being prepared");
            }
            break;
        case AppState::RocketState::Preparing:
            ImGui::Text(ICON_FA_ROCKET " Please Wait");
            state.rocket_progress += 0.003f;
            if (state.rocket_progress >= 1.f) {
                state.rocketState = AppState::RocketState::Launched;
                HelloImGui::Log(HelloImGui::LogLevel::Warning, "Rocket was launched!");
            }
            break;
        case AppState::RocketState::Launched:
            ImGui::Text(ICON_FA_ROCKET " Rocket Launched");
            if (ImGui::Button("Reset Rocket")) {
                state.rocketState = AppState::RocketState ::Init;
                state.rocket_progress = 0.f;
            }
            break;
    }
}


template<typename T>
std::string to_string_with_precision(const T a_value, const int n = 2) {
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return std::move(out).str();
}

// Our Gui in the status bar
void StatusBarGui(const AppState &appState) {
    if (appState.rocketState == AppState::RocketState::Preparing) {
        ImGui::Text("Video buffer status: ");
        ImGui::SameLine();
        ImGui::ProgressBar(appState.rocket_progress, HelloImGui::EmToVec2(12.f, 1.f));

        ImGui::SameLine();
        ImGui::Text("Video capture fps: ");
        ImGui::SameLine();
        ImGui::ProgressBar(appState.rocket_progress, HelloImGui::EmToVec2(12.f, 1.f));

        ImGui::SameLine();
        ImGui::Text("Exposure status: ");
        ImGui::SameLine();
        ImGui::Text(to_string_with_precision(appState.rocket_progress).c_str());
        ImGui::SameLine();
        ImGui::Text(" ms");
    }
}


void MenuBar(const AppState &appState, HelloImGui::RunnerParams &runnerParams) {
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
                HyperlinkHelper::OpenUrl("https://possiblyashrub.github.io/imgui-docs/");
            if (ImGui::MenuItem("imgui-manual"))
                HyperlinkHelper::OpenUrl("https://github.com/pthom/imgui_manual");
            if (ImGui::MenuItem("Online interactive imgui-manual"))
                HyperlinkHelper::OpenUrl("https://pthom.github.io/imgui_manual_online/");


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
    // Part 1: Define the application state, fill the status and menu bars, and load additional font
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    CameraWindow cameraWindow;
    AboutWindow aboutWindow;
    Acknowledgments acknowledgments;
    // Our application state
    AppState appState;

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
    runnerParams.callbacks.ShowStatus = [&appState] { StatusBarGui(appState); };

    MenuBar(appState, runnerParams);

    // Custom load fonts
    runnerParams.callbacks.LoadAdditionalFonts = MyLoadFonts;

    // optional native events handling

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Part 2: Define the application layout and windows
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //
    //    2.1 Define the docking splits,
    //    i.e. the way the screen space is split in different target zones for the dockable windows
    //     We want to split "MainDockSpace" (which is provided automatically) into three zones, like this:
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

    // First, tell HelloImGui that we want full screen dock space (this will create "MainDockSpace")
    runnerParams.imGuiWindowParams.defaultImGuiWindowType = HelloImGui::DefaultImGuiWindowType::ProvideFullScreenDockSpace;
    // In this demo, we also demonstrate multiple viewports.
    // you can drag windows outside out the main window in order to put their content into new native windows
    runnerParams.imGuiWindowParams.enableViewports = true;

    // Then, add a space named "BottomSpace" whose height is 25% of the app height.
    // This will split the preexisting default dockspace "MainDockSpace" in two parts.
    HelloImGui::DockingSplit splitMainBottom;
    splitMainBottom.initialDock = "MainDockSpace";
    splitMainBottom.newDock = "BottomSpace";
    splitMainBottom.direction = ImGuiDir_Down;
    splitMainBottom.ratio = 0.25f;

    // Then, add a space to the left which occupies a column whose width is 25% of the app width
    HelloImGui::DockingSplit splitMainLeft;
    splitMainLeft.initialDock = "MainDockSpace";
    splitMainLeft.newDock = "LeftSpace";
    splitMainLeft.direction = ImGuiDir_Left;
    splitMainLeft.ratio = 0.25f;

    // Finally, transmit these splits to HelloImGui
    runnerParams.dockingParams.dockingSplits = {splitMainBottom, splitMainLeft};

    //
    // 2.1 Define our dockable windows : each window provide a Gui callback, and will be displayed
    //     in a docking split.
    //
    // Our gui providers for the different windows

    // A Command panel named "Commands" will be placed in "LeftSpace". Its Gui is provided calls "CommandGui"
    //HelloImGui::DockableWindow cameraWindow;
    //{
    //    cameraWindow.label = "Camera";
    //    cameraWindow.dockSpaceName = "LeftSpace";
    //    //cameraWindow.GuiFunction = [&appState]() { CommandGui(appState); };
    //}
    // A Command panel named "Commands" will be placed in "LeftSpace". Its Gui is provided calls "CommandGui"
    HelloImGui::DockableWindow commandsWindow;
    {
        commandsWindow.label = "Commands";
        commandsWindow.dockSpaceName = "LeftSpace";
        commandsWindow.GuiFunction = [&cameraWindow] { cameraWindow.gui(); };
    }
    // A Log  window named "Logs" will be placed in "BottomSpace". It uses the HelloImGui logger gui
    HelloImGui::DockableWindow logsWindow;
    {
        logsWindow.label = "Logs";
        logsWindow.dockSpaceName = "BottomSpace";
        logsWindow.GuiFunction = [] { HelloImGui::LogGui(); };
    }
    // A Window named "Dear ImGui Demo" will be placed in "MainDockSpace"
    HelloImGui::DockableWindow captureWindow;
    {
        captureWindow.label = "CaptureWindow";
        captureWindow.dockSpaceName = "MainDockSpace";
        //captureWindow.GuiFunction = [] { ImGui::ShowDemoWindow(); };
    }
    HelloImGui::DockableWindow dock_acknowledgments;
    {
        dock_acknowledgments.label = "Acknowledgments";
        dock_acknowledgments.dockSpaceName = "MainDockSpace";
        dock_acknowledgments.isVisible = false;
        dock_acknowledgments.includeInViewMenu = false;
        dock_acknowledgments.GuiFunction = [&acknowledgments] { acknowledgments.gui(); };
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
    runnerParams.dockingParams.dockableWindows = {dock_about, /*cameraWindow,*/ commandsWindow, logsWindow, dock_acknowledgments, captureWindow};
    aboutWindow.isVisible = &(runnerParams.dockingParams.dockableWindowOfName("About")->isVisible);
    acknowledgments.isVisible = &(runnerParams.dockingParams.dockableWindowOfName("Acknowledgments")->isVisible);
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
    ImmApp::Run(runnerParams, addons);

    return 0;
}
