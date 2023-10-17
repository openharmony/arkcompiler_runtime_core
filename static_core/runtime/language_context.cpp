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
#include "runtime/include/language_context.h"

#include "macros.h"
#include "runtime/core/core_itable_builder.h"
#include "runtime/core/core_vm.h"
#include "runtime/core/core_vtable_builder.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/language_config.h"
#include "runtime/include/method.h"
#include "runtime/include/runtime.h"
#include "runtime/include/stack_walker.h"
#include "runtime/include/thread.h"
#include "runtime/include/vtable_builder-inl.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/vm_handle.h"
#include "runtime/tooling/default_inspector_extension.h"

namespace panda {
std::pair<Method *, uint32_t> LanguageContextBase::GetCatchMethodAndOffset(Method *method, ManagedThread *thread) const
{
    uint32_t catch_offset = 0;
    Method *catch_method = method;
    auto stack = StackWalker::Create(thread);
    while (stack.HasFrame()) {
        catch_method = stack.GetMethod();
        if (catch_method->GetPandaFile() == nullptr) {
            stack.NextFrame();
            continue;
        }
        if (stack.IsCFrame()) {
            stack.NextFrame();
            continue;
        }
        catch_offset = catch_method->FindCatchBlock(thread->GetException()->ClassAddr<Class>(), stack.GetBytecodePc());

        if (catch_offset != panda_file::INVALID_OFFSET) {
            break;
        }
        stack.NextFrame();
    }

    return std::make_pair(catch_method, catch_offset);
}

std::unique_ptr<ClassLinkerExtension> LanguageContextBase::CreateClassLinkerExtension() const
{
    return nullptr;
}

std::unique_ptr<tooling::InspectorExtension> LanguageContextBase::CreateInspectorExtension() const
{
    std::unique_ptr<tooling::InspectorExtension> result;

    if (GetLanguageType() == LangTypeT::LANG_TYPE_STATIC) {
        result = std::make_unique<tooling::StaticDefaultInspectorExtension>(GetLanguage());
    } else {
        result = std::make_unique<tooling::DynamicDefaultInspectorExtension>();
    }
    return result;
}

PandaUniquePtr<tooling::PtLangExt> LanguageContextBase::CreatePtLangExt() const
{
    return nullptr;
}

void LanguageContextBase::ThrowException([[maybe_unused]] ManagedThread *thread,
                                         [[maybe_unused]] const uint8_t *mutf8_name,
                                         [[maybe_unused]] const uint8_t *mutf8_msg) const
{
}

void LanguageContextBase::SetExceptionToVReg(
    [[maybe_unused]] interpreter::AccVRegister &vreg,  // NOLINTNEXTLINE(google-runtime-references)
    [[maybe_unused]] ObjectHeader *obj) const
{
}

}  // namespace panda
