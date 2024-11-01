/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <atomic>
#include "libpandabase/os/time.h"
#include "plugins/ets/runtime/types/ets_primitives.h"

namespace ark::ets::intrinsics::taskpool {

static std::atomic<EtsLong> g_taskId = 1;
static std::atomic<EtsLong> g_taskGroupId = 1;
static std::atomic<EtsLong> g_seqRunnerId = 1;

extern "C" EtsLong GenerateTaskId()
{
    return g_taskId++;
}

extern "C" EtsLong GenerateGroupId()
{
    return g_taskGroupId++;
}

extern "C" EtsLong GenerateSeqRunnerId()
{
    return g_seqRunnerId++;
}

}  // namespace ark::ets::intrinsics::taskpool
