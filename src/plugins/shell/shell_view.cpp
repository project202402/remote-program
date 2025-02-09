/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "plugins/shell/shell_view.h"
#include "plugins/shell/shell_define.h"
#include "manager/view_manager.h"
#include "common/encoding.h"
#include "common/logger.h"
#include "imgui.h"

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

void ShellView::draw() {
    if (!open_){
        return ;
    }

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar;
    std::string title = "\\ " + std::to_string(getSessionId()) + " remote shell";
    ImGui::Begin(title.c_str(), &open_, window_flags);
    if(!open_) {
        stop();
    }

    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);
    std::string utf8str = EncodingConversion::GBKToUTF8(shell_result_);
    ImGui::TextUnformatted(utf8str.c_str());

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();

    ImGui::Separator();

    std::string input_path ;
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    if(InputText("##path", &input_path, ImGuiInputTextFlags_EnterReturnsTrue)){
        if(!input_path.empty()){
            input_path += "\r\n";
            //getTaskProducer()->doTask(ShellFactory::command(getViewId(), getSessionId(), shell_id_, input_path));
            call(getSessionId(), kShellPlugin, "execShell", [this](auto ec, auto result) {
                int status = as<int>(result);
            }, input_path);
        }
    }
    ImGui::PopItemWidth();
    ImGui::End();
}

void ShellView::start() {
    call(getSessionId(), kShellPlugin, "startShell", [](auto ec, auto result) {
        auto status = as<int>(result);
    }, getViewId(), getSessionId());
}

void ShellView::stop() {
    call(getSessionId(), kShellPlugin, "stopShell", [this](auto ec, auto result) {
        getTaskRunner()->postTask([this] {
                    ViewManager::views.erase(getViewId());
                });
    });
}

void ShellView::onTaskDone(Task task) {
    int command = task["command"];
    switch (command) {
        case RPC:
        {
            View::onTaskDone(task);
        }
        break;
        case SHELL_COMMAND:
        {
            const std::string& output = task["output"];
            int status = task["status"];
            shell_result_ += output;
        }
        break;
        default:break;
    }
}