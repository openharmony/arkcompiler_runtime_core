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

#ifndef COMMON_INTERFACES_HEAP_HEAP_H
#define COMMON_INTERFACES_HEAP_HEAP_H

#include <cstdint>

#include "objects/base_object.h"

namespace panda {
enum class GcType : uint8_t {
    ASYNC,
    SYNC,
    FULL,  // Waiting finish
};

namespace common {
using HeapVisitor = const std::function<void(BaseObject*)>;

class Heap {
public:
    Heap() = default;
    ~Heap() = default;

    void RequestGC(GcType type);
    bool ForEachObj(HeapVisitor& visitor, bool safe);
    size_t GetRegionSize();  // for serializer now
};
}  // namespace common
}  // namespace panda
#endif  // COMMON_INTERFACES_HEAP_HEAP_H
