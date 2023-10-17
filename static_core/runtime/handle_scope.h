/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_RUNTIME_HANDLE_SCOPE_H
#define PANDA_RUNTIME_HANDLE_SCOPE_H

#include "runtime/handle_base.h"
#include "runtime/include/coretypes/tagged_value.h"

namespace panda {
/*
 * Handles are only valid within a HandleScope. When a handle is created for an object a cell is allocated in the
 * current HandleScope.
 */
template <typename T>
class PANDA_PUBLIC_API HandleScope {
public:
    inline explicit HandleScope(ManagedThread *thread);

    virtual inline ~HandleScope();

    inline uint32_t GetBeginIndex() const
    {
        return begin_index_;
    }

    inline uint32_t GetHandleCount() const
    {
        return handle_count_;
    }

    uintptr_t NewHandle(T value);

protected:
    inline HandleScope(ManagedThread *thread, T value);

    inline ManagedThread *GetThread() const;

    uint32_t begin_index_ {0};  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    uint32_t handle_count_ {0};
    ManagedThread *thread_ {nullptr};

    NO_COPY_SEMANTIC(HandleScope);
    NO_MOVE_SEMANTIC(HandleScope);
};

/*
 * EscapeHandleScope: Support the user to return a handle to prev handlescope.
 */
template <typename T>
class EscapeHandleScope final : public HandleScope<T> {
public:
    inline explicit EscapeHandleScope(ManagedThread *thread);

    inline ~EscapeHandleScope() override
    {
        ASSERT(already_escape_);
    }

    NO_COPY_SEMANTIC(EscapeHandleScope);
    NO_MOVE_SEMANTIC(EscapeHandleScope);

    inline HandleBase Escape(HandleBase handle)
    {
        ASSERT(!already_escape_);
        already_escape_ = true;
        *(reinterpret_cast<T *>(escape_handle_.GetAddress())) = *(reinterpret_cast<T *>(handle.GetAddress()));
        return escape_handle_;
    }

private:
    bool already_escape_ = false;
    HandleBase escape_handle_;
};
}  // namespace panda
#endif  // PANDA_RUNTIME_HANDLE_SCOPE_H
