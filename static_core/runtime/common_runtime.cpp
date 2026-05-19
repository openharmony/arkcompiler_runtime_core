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

#include "runtime/common_runtime.h"

#include "common_interfaces/base_runtime.h"
#include "runtime/include/runtime.h"

using namespace ark::mem;

namespace ark::common_runtime {

void InitCommonRuntime()
{
    auto &runtimeOptions = Runtime::GetCurrent()->GetOptions();
    auto *baseRuntime = ark::common_vm::BaseRuntime::GetInstance();
    ASSERT(baseRuntime != nullptr);
    auto param = ark::common_vm::BaseRuntime::GetDefaultParam();
    // Single pass compaction should be enabled explicitly
    param.gcParam.singlePassCompactionEnabled =
        runtimeOptions.WasSetG1SinglePassCompactionEnabled() && runtimeOptions.IsG1SinglePassCompactionEnabled();
    baseRuntime->Init(param);
}

void FinishCommonRuntime()
{
    auto *baseRuntime = ark::common_vm::BaseRuntime::GetInstance();
    ASSERT(baseRuntime != nullptr);
    baseRuntime->Fini();
    ark::common_vm::BaseRuntime::DestroyInstance();
}

void SetMutatorLock(MutatorLock *mutatorLock)
{
    common_vm::BaseRuntime::GetInstance()->SetMutatorLock(mutatorLock);
}

}  // namespace ark::common_runtime
