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

#include "utils/logger.h"
#include "verification_options.h"
#include "utils/hash.h"
#include "runtime/include/method.h"
#include "runtime/include/mem/allocator.h"

#include "macros.h"

#include <cstdint>
#include <string>

namespace panda::verifier {

void VerificationOptions::Initialize(const panda::RuntimeOptions &runtime_options)
{
    Debug.MethodOptions = new (mem::AllocatorAdapter<MethodOptionsConfig>().allocate(1)) MethodOptionsConfig {};
    ASSERT(Debug.MethodOptions != nullptr);

    Mode = runtime_options.GetVerificationMode();

    ConfigFile = runtime_options.GetVerificationConfigFile();

    Cache.File = runtime_options.GetVerificationCacheFile();
    Cache.UpdateOnExit = runtime_options.IsVerificationUpdateCache();

    VerifyRuntimeLibraries = runtime_options.IsVerifyRuntimeLibraries();
    SyncOnClassInitialization = runtime_options.IsVerificationSyncOnClassInitialization();
    VerificationThreads = runtime_options.GetVerificationThreads();
}

void VerificationOptions::Destroy()
{
    if (Debug.MethodOptions != nullptr) {
        Debug.MethodOptions->~MethodOptionsConfig();
        mem::AllocatorAdapter<MethodOptionsConfig>().deallocate(Debug.MethodOptions, 1);
    }
    Debug.MethodOptions = nullptr;
}

}  // namespace panda::verifier
