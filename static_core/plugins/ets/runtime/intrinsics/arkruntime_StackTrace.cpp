/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "ets_handle.h"
#include "include/mem/panda_containers.h"
#include "runtime/runtime_helpers.h"
#include "types/ets_method.h"
#include "types/ets_stacktrace_element.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"

#include "runtime/include/stack_walker.h"
#include "runtime/interpreter/runtime_interface.h"
#include "runtime/handle_scope.h"
#include "runtime/handle_scope-inl.h"

#include "plugins/ets/runtime/intrinsics/helpers/ets_intrinsics_helpers.h"

namespace ark::ets::intrinsics {

EtsStackTraceElement *CreateStackTraceElement(StackWalker *stack)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(executionCtx);

    helpers::StackTraceElementInfo info;
    if (!GetStackTraceElementBasicInfo(stack, &info)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return nullptr;
    }

    EtsHandle<EtsString> classNameHandle(executionCtx, EtsString::CreateFromMUtf8(info.className.c_str()));
    if (UNLIKELY(classNameHandle.GetPtr() == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return nullptr;
    }
    EtsHandle<EtsString> methodNameHandle(executionCtx, EtsString::CreateFromMUtf8(info.methodName.c_str()));
    if (UNLIKELY(methodNameHandle.GetPtr() == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return nullptr;
    }

    auto *stackTraceElement = EtsStackTraceElement::Create(executionCtx);
    if (UNLIKELY(stackTraceElement == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return nullptr;
    }
    auto element = EtsHandle<EtsStackTraceElement>(executionCtx, stackTraceElement);
    element->SetClassName(classNameHandle.GetPtr());
    element->SetMethodName(methodNameHandle.GetPtr());

    EtsString *sourceFileName = EtsString::CreateFromMUtf8(info.sourceFile);
    if (UNLIKELY(sourceFileName == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return nullptr;
    }
    element->SetSourceFileName(sourceFileName);
    element->SetLineNumber(info.lineNumber);
    return element.GetPtr();
}

extern "C" EtsObjectArray *ArkRuntimeStackTraceProvisionStackTrace()
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(executionCtx);

    auto stackTraceElementClass = PlatformTypes(executionCtx)->arkruntimeStackTraceElement;

    uint32_t linesSize = 0;
    for (auto stack = StackWalker::Create(executionCtx->GetMT()); stack.HasFrame(); stack.NextFrame()) {
        linesSize++;
    }

    auto *resultArray = EtsObjectArray::Create(stackTraceElementClass, linesSize);
    if (UNLIKELY(resultArray == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return nullptr;
    }
    EtsHandle<EtsObjectArray> resultArrayHandle(executionCtx, resultArray);
    uint32_t i = 0;
    for (auto stack = StackWalker::Create(executionCtx->GetMT()); stack.HasFrame() && i < linesSize;
         stack.NextFrame()) {
        auto element = CreateStackTraceElement(&stack);
        if (UNLIKELY(element == nullptr)) {
            ASSERT(executionCtx->GetMT()->HasPendingException());
            return nullptr;
        }
        resultArrayHandle.GetPtr()->Set(i, element->AsObject());
        i++;
    }
    return resultArrayHandle.GetPtr();
}

}  // namespace ark::ets::intrinsics
