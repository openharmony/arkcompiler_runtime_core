/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef COMMON_RUNTIME_COMMON_COMPONENTS_MUTATOR_THREAD_LOCAL_H
#define COMMON_RUNTIME_COMMON_COMPONENTS_MUTATOR_THREAD_LOCAL_H

#include <cstdint>

namespace ark::common_vm {
class AllocationBuffer;
class Mutator;

enum class ThreadType { ARK_PROCESSOR = 0, GC_THREAD, FP_THREAD, HOT_UPDATE_THREAD };

// Backend and ArkThread will use external tls var through offset calculation, so external tls
// must in the first place, followed by the internal tls.
struct ThreadLocalData {
    // External thread local var.
    AllocationBuffer *buffer;
    // Internal thread local var.
    ThreadType threadType;
};

class ThreadLocal {  // merge this to ThreadLocalData.
public:
    static ThreadLocalData *GetThreadLocalData();

    static AllocationBuffer *GetAllocBuffer()
    {
        return GetThreadLocalData()->buffer;
    }
    static void ClearAllocBufferRegion();

    static void SetAllocBuffer(AllocationBuffer *buffer)
    {
        GetThreadLocalData()->buffer = buffer;
    }

    static void SetThreadType(ThreadType type)
    {
        GetThreadLocalData()->threadType = type;
    }

    static ThreadType GetThreadType()
    {
        return GetThreadLocalData()->threadType;
    }
};
}  // namespace ark::common_vm

#endif  // COMMON_RUNTIME_COMMON_COMPONENTS_MUTATOR_THREAD_LOCAL_H
