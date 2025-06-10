/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMMON_INTERFACES_OBJECTS_READONLY_HANDLE_H
#define COMMON_INTERFACES_OBJECTS_READONLY_HANDLE_H

#include "common_interfaces/objects/base_object.h"

namespace common {
template <typename T>
class ReadOnlyHandle {
public:
    inline explicit ReadOnlyHandle(uintptr_t slot) : address_(slot)
    {
        DCHECK_CC(slot != 0);
        T::Cast(*reinterpret_cast<BaseObject **>(slot));
    }
    inline ReadOnlyHandle() : address_(reinterpret_cast<uintptr_t>(nullptr)) {}
    ~ReadOnlyHandle() = default;
    DEFAULT_NOEXCEPT_MOVE_SEMANTIC_CC(ReadOnlyHandle);
    DEFAULT_COPY_SEMANTIC_CC(ReadOnlyHandle);

    uintptr_t GetAddress() const
    {
        return address_;
    }

    template <typename S>
    explicit ReadOnlyHandle(const ReadOnlyHandle<S> &handle) : address_(handle.GetAddress())
    {
        T::Cast(handle.GetBaseObject());
    }

    template <typename S>
    static ReadOnlyHandle<T> Cast(const ReadOnlyHandle<S> &handle)
    {
        T::Cast(handle.GetTaggedValue().GetTaggedObject());
        return ReadOnlyHandle<T>(handle.GetAddress());
    }

    BaseObject* GetBaseObject() const
    {
        if (GetAddress() == 0U) {
            return nullptr;
        }
        // temporarily add ReadBarrier for JSHandle
        return *reinterpret_cast<BaseObject **>(GetAddress());
    }

    T *operator*() const
    {
        return T::Cast(GetBaseObject());
    }

    T *operator->() const
    {
        return T::Cast(GetBaseObject());
    }

    bool operator==(const ReadOnlyHandle<T> &other) const
    {
        return GetBaseObject() == other.GetBaseObject();
    }

    bool operator!=(const ReadOnlyHandle<T> &other) const
    {
        return GetBaseObject() != other.GetBaseObject();
    }

    bool IsEmpty() const
    {
        return GetAddress() == 0U;
    }

    template <typename R>
    R *GetObject() const
    {
        return reinterpret_cast<R *>(GetBaseObject());
    }
private:
    uintptr_t address_;
};
}

#endif //COMMON_INTERFACES_OBJECTS_READONLY_HANDLE_H
