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

#include "ets_handle.h"
#include "include/mem/panda_containers.h"
#include "runtime/runtime_helpers.h"
#include "types/ets_method.h"
#include "types/ets_stacktrace_element.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"

#include "runtime/include/stack_walker.h"
#include "runtime/include/thread.h"
#include "runtime/interpreter/runtime_interface.h"
#include "runtime/handle_scope.h"
#include "runtime/handle_scope-inl.h"

namespace ark::ets::intrinsics {

EtsStackTraceElement *CreateStackTraceElement(StackWalker *stack)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);

    EtsMethod *method = EtsMethod::FromRuntimeMethod(stack->GetMethod());
    auto className = EtsHandle<EtsString>(coroutine, method->GetClass()->GetName());
    auto methodName = EtsHandle<EtsString>(coroutine, method->GetNameString());
    const auto lineNumber = method->GetLineNumFromBytecodeOffset(stack->GetBytecodePc());
    auto *sourceFile = reinterpret_cast<const char *>(method->GetClassSourceFile().data);
    if (sourceFile == nullptr) {
        sourceFile = "<unknown>";
    }

    auto element = EtsHandle<EtsStackTraceElement>(coroutine, EtsStackTraceElement::Create(coroutine));
    element->SetClassName(className.GetPtr());
    element->SetMethodName(methodName.GetPtr());

    EtsString *sourceFileName = EtsString::CreateFromMUtf8(sourceFile);
    element->SetSourceFileName(sourceFileName);
    element->SetLineNumber(lineNumber);
    return element.GetPtr();
}

extern "C" EtsObjectArray *StdCoreStackTraceProvisionStackTrace()
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);

    auto linker = coroutine->GetPandaVM()->GetClassLinker();
    auto stackTraceElementClass = linker->GetClass(panda_file_items::class_descriptors::STACK_TRACE_ELEMENT.data());

    auto thread = ManagedThread::GetCurrent();
    auto walker = StackWalker::Create(thread);

    PandaVector<EtsHandle<EtsStackTraceElement>> stackTraceElements;
    for (auto stack = StackWalker::Create(thread); stack.HasFrame(); stack.NextFrame()) {
        auto element = EtsHandle<EtsStackTraceElement>(coroutine, CreateStackTraceElement(&stack));
        stackTraceElements.push_back(element);
    }

    const auto linesSize = static_cast<uint32_t>(stackTraceElements.size());

    EtsHandle<EtsObjectArray> resultArrayHandle(coroutine, EtsObjectArray::Create(stackTraceElementClass, linesSize));
    for (uint32_t i = 0; i < linesSize; i++) {
        resultArrayHandle.GetPtr()->Set(i, stackTraceElements[i]->AsObject());
    }
    return resultArrayHandle.GetPtr();
}

}  // namespace ark::ets::intrinsics
