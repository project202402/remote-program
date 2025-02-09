/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */


#include "plugins/screen/screen_view.h"
#include "plugins/screen/screen_control.h"
#include "plugins/screen/screen_define.h"
#include "manager/view_manager.h"
#include "common/logger.h"

#include "imgui.h"
#include "imgui_impl_opengl3_loader.h"
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromMemory(const void* data, size_t data_size, GLuint* out_texture, int* out_width, int* out_height)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load_from_memory((const unsigned char*)data, (int)data_size, &image_width, &image_height, NULL, 4);
    if (image_data == nullptr)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload pixels into texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}


void ScreenView::draw() {
    if(!open_) {
        return;
    }

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar;
    std::string title = "\\ " + std::to_string(getSessionId()) + " screen shot";
    ImGui::Begin(title.c_str(), &open_, window_flags);
    if(!open_) {
        //getTaskProducer()->doTask(ScreenFactory::stop(getViewId(), getSessionId()));
        stopCapture();
    }

    std::string sliderLabel = "Quality = %d | size = " +
                          std::to_string(screen_width_) + " x " + std::to_string(screen_height_);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::SliderInt("##quality", &screen_quality_, 10, 100, sliderLabel.c_str());
    ImGui::SameLine();
    ImGui::PopItemWidth();
    ImGui::Separator();

    float window_width = ImGui::GetWindowSize().x;
    float padding = (window_width - screen_width_) / 2.0f;
    if (padding > 0.0f) {
        ImGui::SetCursorPosX(padding);
    }
    ImGui::Image((void*)(intptr_t)screen_texture_, ImVec2(screen_width_, screen_height_));
    ImGui::End();

}

void ScreenView::start() {
    startCapture();
}

void ScreenView::startCapture() {
    call(getSessionId(), kScreenPlugin, "startCapture", [this](auto ec, auto result) {
        int status = as<int>(result);
        captureScreen();
    });
}

void ScreenView::stopCapture() {
    call(getSessionId(), kScreenPlugin, "stopCapture", [this](auto ec, auto result) {
        int status = as<int>(result);
        getTaskRunner()->postTask([this] {
                    ViewManager::views.erase(getViewId());
                });
    });
}

void ScreenView::captureScreen() {
    call(getSessionId(), kScreenPlugin, "captureScreen", [this](auto ec, auto result) {
        auto data = as<std::vector<char>>(result);
        data_.swap(data);
        bool ret = LoadTextureFromMemory(data_.data(), data_.size(), &screen_texture_, &screen_width_, &screen_height_);
        if (open_)
            captureScreen();
    }, screen_quality_);
}