/*
 * Copyright (c) 2024, zz
 * All rights reserved.
 *
 * Author: zz <3875657991@qq.com>
 */

#include "common/task_runner.h"
#include "common/macros_magic.h"

class DeleteHelper
{
public:
    DeleteHelper(void(*deleter)(const void*), const void* object)
        : deleter_(deleter),
          object_(object)
    {
        // Nothing
    }

    ~DeleteHelper()
    {
        doDelete();
    }

    void doDelete()
    {
        if (deleter_ && object_)
        {
            deleter_(object_);

            deleter_ = nullptr;
            object_ = nullptr;
        }
    }

private:
    void(*deleter_)(const void*);
    const void* object_;

    DISALLOW_COPY_AND_ASSIGN(DeleteHelper);
};

//--------------------------------------------------------------------------------------------------
void TaskRunner::deleteSoonInternal(void(*deleter)(const void*), const void* object)
{
    postTask(
        std::bind(&DeleteHelper::doDelete, std::make_shared<DeleteHelper>(deleter, object)));
}
