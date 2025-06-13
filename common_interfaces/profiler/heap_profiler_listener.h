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

#ifndef COMMON_INTERFACES_HEAP_PROFILER_LISTENER_H
#define COMMON_INTERFACES_HEAP_PROFILER_LISTENER_H

#include <cstddef>
#include <cstdint>
#include <functional>

namespace common {
class HeapProfilerListener {
public:
    static HeapProfilerListener &GetInstance();

    void RegisterMoveEventCb(const std::function<void(uintptr_t, uintptr_t, size_t)> &cb);
    void UnRegisterMoveEventCb();
    void OnMoveEvent(uintptr_t fromObj, uintptr_t toObj, size_t size);

private:
    std::function<void(uintptr_t, uintptr_t, size_t)> moveEventCb_;
};
}  // namespace common
#endif  // COMMON_INTERFACES_HEAP_PROFILER_LISTENER_H