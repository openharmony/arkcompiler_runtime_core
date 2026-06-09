/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "ark_mutator_container.h"

#include "runtime/include/mutator.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime.h"

namespace ark::common_vm {

ArkMutatorContainer::ArkMutatorContainer()
{
    auto *runtime = Runtime::GetCurrent();
    auto *vm = runtime->GetPandaVM();
    auto allocator = runtime->GetInternalAllocator();
    while (vm == nullptr) {
        vm = runtime->GetPandaVM();
    }
    mutator_ = allocator->New<ark::Mutator>(vm, ark::Mutator::MutatorType::GC);
    ASSERT(mutator_ != nullptr);
    auto *gc = vm->GetGC();
    ASSERT(ark::Mutator::GetCurrent() == nullptr);
    gc->OnMutatorCreate(mutator_);
    ark::Mutator::SetCurrent(mutator_);
}

ArkMutatorContainer::~ArkMutatorContainer()
{
    auto *runtime = Runtime::GetCurrent();
    auto allocator = runtime->GetInternalAllocator();
    auto *vm = runtime->GetPandaVM();
    auto *gc = vm->GetGC();
    gc->OnMutatorTerminate(mutator_, ark::mem::MutatorUnregistrationMode::UNREGISTER,
                           ark::mem::BuffersKeepingFlag::DELETE);
    allocator->Delete(mutator_);
    ark::Mutator::SetCurrent(nullptr);
}

}  // namespace ark::common_vm
