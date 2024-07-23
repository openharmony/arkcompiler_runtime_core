/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

namespace ark::verifier {

void VerificationOptions::Initialize()
{
    debug.methodOptions = new (mem::AllocatorAdapter<MethodOptionsConfig>().allocate(1)) MethodOptionsConfig {};
    ASSERT(debug.methodOptions != nullptr);
}

void VerificationOptions::Destroy()
{
    if (debug.methodOptions != nullptr) {
        debug.methodOptions->~MethodOptionsConfig();
        mem::AllocatorAdapter<MethodOptionsConfig>().deallocate(debug.methodOptions, 1);
    }
    debug.methodOptions = nullptr;
}

}  // namespace ark::verifier
