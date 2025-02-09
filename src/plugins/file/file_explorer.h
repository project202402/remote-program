/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */


#pragma once

#include <stack>
#include <string>

class FileExplorer {
public:
    void navigateTo(const std::string& path) {
        forward_stack_.emplace(current_path_);
        current_path_ = path;
        while (!backward_stack_.empty()) {
            backward_stack_.pop();
        }
    }

    std::string goBack() {
        if (canGoBack()) {
            backward_stack_.emplace(current_path_);
            current_path_ = forward_stack_.top();
            forward_stack_.pop();
            return current_path_;
        }
        return "";
    }

    std::string goForward() {
        if (canGoForward()) {
            forward_stack_.emplace(current_path_);
            current_path_ = backward_stack_.top();
            backward_stack_.pop();
            return current_path_;
        }
        return "";
    }

private:
    bool canGoBack() const {
        return !forward_stack_.empty();
    }

    bool canGoForward() const {
        return !backward_stack_.empty();
    }

    std::string current_path_;
    std::stack<std::string> forward_stack_;
    std::stack<std::string> backward_stack_;
};
