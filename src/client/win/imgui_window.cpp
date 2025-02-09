/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "client/win/imgui_window.h"
#include "client/win/imgui_logger.h"
#include "client/resource/font/IconsFontAwesome6.h"
#include "manager/view_manager.h"
#include "common/logger.h"
#include "utils/binary_overlay.h"

#include <Windows.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "GLFW/glfw3.h"
#include <cstdio>
#include <string>
#include <filesystem>

#include "manager/setting_manager.h"
#include "ImGuiFileDialog.h"

namespace fs = std::filesystem;

static void RestartProcess() {
    char exePath[MAX_PATH];
    if (GetModuleFileNameA(NULL, exePath, MAX_PATH) == 0) {
        return;
    }
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    if (CreateProcessA(
        exePath,
        NULL,
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi
    )) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}

static int InputTextCallback(ImGuiInputTextCallbackData* data) {
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        auto* str = (std::string*)data->UserData;
        IM_ASSERT(data->Buf == str->c_str());
        str->resize(data->BufTextLen);
        data->Buf = const_cast<char *>(str->c_str());
    }
    return 0;
}

static bool InputText(const char* label, std::string* str, ImGuiInputTextFlags flags = 0) {
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    return ImGui::InputText(label, (char*)str->c_str(), str->capacity() + 1, flags | ImGuiInputTextFlags_CallbackResize, InputTextCallback, (void*)str);
}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

std::string getExecutablePath() {
    char buffer[1024];
#ifdef _WIN32
    GetModuleFileNameA(NULL, buffer, sizeof(buffer));
#else
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer));
    if (len == -1) {
        throw std::runtime_error("Error getting executable path");
    }
    buffer[len] = '\0';
#endif
    return std::string(buffer);
}

std::string getAssetFolderPath() {
    std::filesystem::path exePath = getExecutablePath();
    std::filesystem::path exeDir = exePath.parent_path();
    std::filesystem::path assetDir = exeDir / "resource/";
    return assetDir.string();
}


std::string GetSourceDirectory() {
    std::string filePath = __FILE__;
    fs::path sourcePath(filePath);
    return sourcePath.parent_path().string();
}


ImguiWindow::ImguiWindow(std::shared_ptr<TaskRunner> task_runner):
    task_runner_(std::move(task_runner))
{
    ZLOG_INFO << "Ctor";
    auto config = SettingManager::getInstance().getConfig();
    listen_ip_ = config["listen_ip"];
    listen_port_ = config["listen_port"];
    server_ip_ = config["server_ip"];
    server_port_ = config["server_port"];

    glfwSetErrorCallback(glfw_error_callback);
    bool result = glfwInit();
    assert(result);

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    //  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

    //  Create window with graphics context
    window_ = glfwCreateWindow(1280, 720, "zzrat", nullptr, nullptr);
    assert(window_);

    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);  // Enable vsync
#ifdef DEBUG
    std::string icon_path = GetSourceDirectory() + "/../resource/icon.png";
#else
    std::string icon_path = getAssetFolderPath() + "icon.png";
#endif
    GLFWimage images[1];
    images[0].pixels = stbi_load(icon_path.c_str(), &images[0].width, &images[0].height, 0, 4);  // rgba channels
    glfwSetWindowIcon(window_, 1, images);
    stbi_image_free(images[0].pixels);

    //  Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    io.ConfigViewportsNoAutoMerge = true;
    //  io.ConfigViewportsNoTaskBarIcon = true;
    io.IniFilename = "client.ini";

    //  Setup Dear ImGui style
    //  ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();
    //  Setup fonts
    //  ImFontConfig fontConf;
    //  io.Fonts->AddFontDefault(&fontConf);

#ifdef DEBUG
    std::string msyh_font_path = GetSourceDirectory() + "/../resource/font/msyh.ttc";
#else
    std::string msyh_font_path = getAssetFolderPath() + "font/msyh.ttc";
#endif
    io.Fonts->AddFontFromFileTTF(msyh_font_path.c_str(), 18.0F, nullptr, io.Fonts->GetGlyphRangesChineseFull());

    //  Merge fontawesom fonts
    static constexpr ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;

#ifdef DEBUG
    std::string fa_solid_font_path = GetSourceDirectory() + "/../resource/font/fa-solid-900.ttf";
#else
    std::string fa_solid_font_path = getAssetFolderPath() + "font/fa-solid-900.ttf";
#endif
    io.Fonts->AddFontFromFileTTF(fa_solid_font_path.c_str(), 18.0f, &icons_config, icons_ranges);

    //  When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

}

ImguiWindow::~ImguiWindow() {
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window_);
    glfwTerminate();
}

bool ImguiWindow::draw() {
    if (glfwWindowShouldClose(window_)) {
        return false;
    }

    if (idx_frame_ == 3){
        glfwShowWindow(window_);
    }

    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    drawInner();

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window_, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    ImGuiIO& io = ImGui::GetIO();
    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
    //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }

    glfwSwapBuffers(window_);
    idx_frame_ += 1;
    return true;
}

void ImguiWindow::wakeup()
{
    glfwPostEmptyEvent();
}

void ImguiWindow::waitEvents(double timeout)
{
    glfwWaitEventsTimeout(timeout);
}

void ImguiWindow::drawInner() {
    drawMenus();
    drawSettings();
    drawBuild();
    drawAbout();
    drawViews();
    drawLogs();
}

void ImguiWindow::drawMenus() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit")) {
                task_runner_->postQuit();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            if (ImGui::BeginMenu("Colors"))
            {
                if (ImGui::MenuItem("Light", nullptr, selected_light_)) {
                    ImGui::StyleColorsLight();
                    selected_light_ = true;
                    selected_dark_ = false;
                    selected_classic_ = false;
                }
                if (ImGui::MenuItem("Dark", nullptr, selected_dark_)) {
                    ImGui::StyleColorsDark();
                    selected_light_ = false;
                    selected_dark_ =  true;
                    selected_classic_ = false;
                }
                if (ImGui::MenuItem("Classic", nullptr, selected_classic_)) {
                    ImGui::StyleColorsClassic();
                    selected_light_ = false;
                    selected_dark_ = false;
                    selected_classic_ = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Settings")) {
            open_settings_ = true;
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Build")) {
            open_build_ = true;
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("About")) {
            open_about_ = true;
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void ImguiWindow::drawViews() {
    auto dockspace_id = ImGui::DockSpaceOverViewport();
    for (auto &[id, view]: ViewManager::views) {
        ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);
        view->draw();
    }
}

void ImguiWindow::drawLogs() {
    ImGui::Begin("logs");
    LoggerWindow::singleton()->draw();
    ImGui::End();
}

void ImguiWindow::drawSettings() {
    if (open_settings_) {
        ImGui::OpenPopup("Settings");
        open_settings_ = false;
    }
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        ImGui::InputInt("##port", &listen_port_);
        ImGui::PopStyleVar();
        ImGui::SameLine();
        if (ImGui::Button("Listen")) {
            ImGui::OpenPopup("restart");
        }

        bool unused_open = true;
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal("restart", &unused_open, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Do you want to restart now to apply the changes?");
            float windowWidth = ImGui::GetContentRegionAvail().x;
            float buttonWidth = windowWidth / 2.0f - ImGui::GetStyle().ItemSpacing.x / 2.0f;
            if (ImGui::Button("OK", ImVec2(buttonWidth, 0))) {
                IMGUI_INFO_FMT("Listen Port: %d", listen_port_);
                SettingManager::getInstance().setField("listen_port", listen_port_);
                SettingManager::getInstance().saveConfig();
                task_runner_->postQuit();
                RestartProcess();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Close", ImVec2(buttonWidth, 0)))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
        float windowWidth = ImGui::GetContentRegionAvail().x;
        float buttonWidth = windowWidth / 2.0f - ImGui::GetStyle().ItemSpacing.x / 2.0f;
        if (ImGui::Button("Save", ImVec2(buttonWidth, 0))) {
            SettingManager::getInstance().setField("listen_port", listen_port_);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void ImguiWindow::drawBuild() {
    if (open_build_) {
        ImGui::OpenPopup("Build");
        open_build_ = false;
    }
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Build", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        InputText("##ip", &server_ip_);
        ImGui::InputInt("##port", &server_port_);
        ImGui::PopStyleVar();

        float windowWidth = ImGui::GetContentRegionAvail().x;
        float buttonWidth = windowWidth / 2.0f - ImGui::GetStyle().ItemSpacing.x / 2.0f;
        if (ImGui::Button("OK", ImVec2(buttonWidth, 0))) {
            IGFD::FileDialogConfig config;
            config.path = ".";
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFolder", "Choose Folder", nullptr, config);
            ImGui::CloseCurrentPopup();
        }

        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    if (ImGuiFileDialog::Instance()->Display("ChooseFolder")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string folderPath = ImGuiFileDialog::Instance()->GetCurrentPath();

            auto &setting = SettingManager::getInstance();
            setting.setField("server_ip", server_ip_);
            setting.setField("server_port", server_port_);

            nlohmann::json config;
            config["server_ip"] = server_ip_;
            config["server_port"] = server_port_;

            auto src = setting.getField<std::string>("server_path", "server.dat");
            if(BinaryOverlay::writeOverlay(src, folderPath + "/server.exe", config.dump())) {
                IMGUI_INFO_FMT("IP: %s, Port: %d build success", server_ip_.c_str(), server_port_);
            }else{
                IMGUI_INFO_FMT("IP: %s, Port: %d build failed", server_ip_.c_str(), server_port_);
            }
        }
        ImGuiFileDialog::Instance()->Close();
    }
}

void ImguiWindow::drawAbout() {
    if (open_about_) {
        ImGui::OpenPopup("About");
        open_about_ = false;
    }
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("About", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Author: zz \nmail: 3875657991@qq.com");
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
