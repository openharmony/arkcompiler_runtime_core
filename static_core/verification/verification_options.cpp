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

void VerificationOptions::Initialize()
{
    debug.method_options = new (mem::AllocatorAdapter<MethodOptionsConfig>().allocate(1)) MethodOptionsConfig {};
    ASSERT(debug.method_options != nullptr);
}

void VerificationOptions::Destroy()
{
    if (debug.method_options != nullptr) {
        debug.method_options->~MethodOptionsConfig();
        mem::AllocatorAdapter<MethodOptionsConfig>().deallocate(debug.method_options, 1);
    }
    debug.method_options = nullptr;
}

}  // namespace panda::verifier
