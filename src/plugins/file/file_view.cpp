/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */


#include "plugins/file/file_view.h"
#include "plugins/file/file_define.h"
#include "plugins/file/file_impl.h"
#include "manager/view_manager.h"
#include "client/resource/font/IconsFontAwesome6.h"
#include "common/logger.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuiFileDialog.h"

#include <regex>
#include <filesystem>
namespace fs = std::filesystem;

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

void FileView::draw() {
    if (!open_){
        getTaskRunner()->postTask([this] {
            ViewManager::views.erase(getViewId());
        });
        return ;
    }

    static ImVector<int> selection;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar;
    std::string title = "\\ " + std::to_string(getSessionId()) + " file manager";
    ImGui::Begin(title.c_str(), &open_, window_flags);

    if (ImGui::Button(ICON_FA_UPLOAD)) {
        IGFD::FileDialogConfig config;
        config.path = ".";
        config.countSelectionMax = 5;
        ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".*", config);
    }
    // display
    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::vector<FileItem> fileItems = {};
            auto selected_files = ImGuiFileDialog::Instance()->GetSelection();
            for(auto& [name, path]:selected_files) {
                auto size = getFileSize(path);
                if(size) {
                    FileItem item{path, remote_path_, size, true};
                    fileItems.emplace_back(std::move(item));
                }
            }
            uploadFiles(fileItems);
        }
        ImGuiFileDialog::Instance()->Close();
    }

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_DOWNLOAD)) {
        IGFD::FileDialogConfig config;
        config.path = ".";
        ImGuiFileDialog::Instance()->OpenDialog("ChooseFolderDlgKey", "Choose Folder", nullptr, config);
    }
    // display
    if (ImGuiFileDialog::Instance()->Display("ChooseFolderDlgKey")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string folderPath = ImGuiFileDialog::Instance()->GetCurrentPath();
            std::vector<FileItem> fileItems = {};
            for(int i : selection) {
                auto path = (fs::path(remote_path_) / remote_files_[i].name).string();
                auto size = static_cast<std::streamsize>(std::stol(remote_files_[i].size));
                if(size) {
                    FileItem item{path, folderPath, size, false};
                    fileItems.emplace_back(std::move(item));
                }
            }
            downloadFiles(fileItems);
        }
        ImGuiFileDialog::Instance()->Close();
    }

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_REPEAT)) {
        if(remote_path_.empty()){
            driverList();
        }else{
            fileList(remote_path_);
        };
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_BACKWARD)) {
        auto back_path = file_explore_.goBack();
        if(!back_path.empty()){
            remote_path_ = back_path;
            fileList(remote_path_);
        }else if(!remote_path_.empty()){
            remote_path_ = "";
            driverList();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FORWARD)) {
        auto forward_path = file_explore_.goForward();
        if(!forward_path.empty()){
            remote_path_ = forward_path;
            fileList(remote_path_);
        }
    }

    ImGui::SameLine();
    char path_input[256] = {0};
    memcpy(path_input, remote_path_.data(), remote_path_.size());

    std::string input_path = remote_path_;
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    if(InputText("##path", &input_path, ImGuiInputTextFlags_EnterReturnsTrue)){
        if(!input_path.empty()){
            remote_path_ = input_path;
            fileList(remote_path_);
            file_explore_.navigateTo(remote_path_);
        }
    }
    ImGui::PopItemWidth();
    ImGui::Separator();

    if (ImGui::BeginTable("file_table", 4, ImGuiTableFlags_BordersInnerH)) {

        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("type");
        ImGui::TableSetupColumn("Size (KB)");
        ImGui::TableSetupColumn("Last modified");
        ImGui::TableHeadersRow();

        for (int i = 0; i < remote_files_.size(); i++) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            auto file = remote_files_[i];
            const bool item_is_selected = selection.contains(i);

            if (ImGui::Selectable(file.name.c_str(), item_is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
                if (ImGui::GetIO().KeyCtrl) {
                    if (item_is_selected)
                        selection.find_erase_unsorted(i);
                    else
                        selection.push_back(i);
                } else {
                    selection.clear();
                    selection.push_back(i);
                }
            }

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                if(remote_path_.empty() || file.type == "Directory" ){
                    auto path = fs::path(remote_path_) / file.name;
                    remote_path_ = path.string();
                    fileList(remote_path_);
                    file_explore_.navigateTo(remote_path_);
                }
            }

            if (item_is_selected && ImGui::BeginPopupContextItem()) {
                /*if (ImGui::MenuItem("Upload")) {
                    // 上传操作
                }
                if (ImGui::MenuItem("Download")) {
                    // 下载操作
                }*/
                ImGui::EndPopup();
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", file.type.c_str());
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", file.size.c_str());
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s", file.modified.c_str());
        }
        ImGui::EndTable();
    }

    auto* viewport = ImGui::GetMainViewport();
    float height = ImGui::GetFrameHeight();

    if (ImGui::BeginViewportSideBar("##MainStatusBar", viewport, ImGuiDir_Down, height, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar)) {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginTable("##footer_table", 3, ImGuiTableFlags_SizingFixedFit)) {
                ImGui::TableSetupColumn("Left", ImGuiTableColumnFlags_WidthStretch, 0.5F);
                ImGui::TableSetupColumn("Center", ImGuiTableColumnFlags_WidthStretch, 0.5F);
                ImGui::TableSetupColumn("Right", ImGuiTableColumnFlags_WidthStretch, 0.5F);
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::Text("status: normal");

                ImGui::TableNextColumn();
                if(uploader_){
                    float columnHeight = ImGui::GetContentRegionAvail().y;
                    ImVec2 progressBarSize = ImVec2(0.0f, columnHeight - 10.0f);
                    ImGui::ProgressBar(
                            static_cast<float>(uploader_->offset()) / static_cast<float>(uploader_->size()),
                            progressBarSize);
                }else if(downloader_) {
                    float columnHeight = ImGui::GetContentRegionAvail().y;
                    ImVec2 progressBarSize = ImVec2(0.0f, columnHeight - 10.0f);
                    ImGui::ProgressBar(
                            static_cast<float>(downloader_->offset()) / static_cast<float>(downloader_->size()),
                            progressBarSize);
                }
                ImGui::TableNextColumn();
                ImGui::Text("size: %zu", remote_files_.size());
            }
            ImGui::EndTable();
            ImGui::EndMenuBar();
        }
        ImGui::End();
    }

    ImGui::End();
}

void FileView::start() {
    driverList();
}

void FileView::driverList() {
    call(getSessionId(), kFilePlugin, "driveList", [this](auto ec, auto result) {
        auto drivers = as<std::vector<FileInfo>>(result);
        remote_files_.swap(drivers);
    });
}

void FileView::fileList(const std::string &path) {
    call(getSessionId(), kFilePlugin, "fileList", [this](auto ec, auto result) {
        auto files = as<std::vector<FileInfo>>(result);
        remote_files_.swap(files);
    }, path);
}


void FileView::uploadFiles(const std::vector<FileItem>& items) {
    bool upload_running = !upload_items_.empty();
    for(auto& item: items) {
        upload_items_.emplace(item);
    }
    if(!upload_running) {
        startUpload();
    }
}

void FileView::startUpload() {
    if (upload_items_.empty()) {
        uploader_.reset();
        return;
    }
    auto task = upload_items_.front();
    upload_items_.pop();
    auto& path = task.source_path;
    auto name = fs::path(path).filename();
    std::string target = (fs::path(task.target_folder) / name).string();

    if(uploader_) {
        ZLOG_ERROR << "last error " ;
    }

    uploader_ = std::make_unique<FileUploader>(path, kChunkSize);
    call(getSessionId(), kFilePlugin, "startUpload", [this](auto ec, auto result) {
        int status = as<int>(result);
        if(status == SUCCESS) {
            uploadFile();
            return;
        }
        startUpload();
    }, target, uploader_->size());
}

void FileView::uploadFile() {
    auto chunk = uploader_->readChunk();
    if(chunk.empty()) {
        startUpload();
        return;
    }

    call(getSessionId(), kFilePlugin, "uploadFile", [this](auto ec, auto result) {
        int status = as<int>(result);
        if(status == SUCCESS) {
            uploadFile();
            return;
        }
        startUpload();
    }, chunk);
}

void FileView::downloadFiles(const std::vector<FileItem>& items) {
    bool download_running = !download_items_.empty();
    for(auto& item: items) {
        download_items_.emplace(item);
    }
    if(!download_running) {
        startDownload();
    }
}

void FileView::startDownload() {
    if (download_items_.empty()) {
        downloader_.reset();
        return;
    }
    auto task = download_items_.front();
    download_items_.pop();

    auto& path = task.source_path;
    auto name = fs::path(path).filename();
    std::string target = (fs::path(task.target_folder) / name).string();

    if(downloader_) {
        ZLOG_ERROR << "last error " ;
    }
    downloader_ = std::make_unique<FileDownloader>(target, task.size);
    call(getSessionId(), kFilePlugin, "startDownload", [this](auto ec, auto result) {
        int status = as<int>(result);
        if(status == SUCCESS) {
            downloadFile();
            return;
        }
        startDownload();
    }, path);

}

void FileView::downloadFile() {
    call(getSessionId(), kFilePlugin, "downloadFile", [this](auto ec, auto result) {
        auto data = as<std::vector<char>>(result);
        if(data.size()) {
            downloader_->writeChunk(data);
            downloadFile();
            return;
        }
        startDownload();
    });
}
