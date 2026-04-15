/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "ets_platform_types.h"
#include "runtime/include/managed_thread.h"
#include "libarkfile/file.h"
#include "include/object_header.h"
#include "intrinsics.h"
#include "libarkbase/os/mutex.h"
#include "plugins/ets/runtime/ets_class_linker_context.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/types/ets_abc_file.h"
#include "plugins/ets/runtime/types/ets_abc_runtime_linker.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/types/ets_string.h"

namespace ark::ets::intrinsics {

void EtsAbcRuntimeLinkerAddNewAbcFiles(EtsAbcRuntimeLinker *runtimeLinker, ObjectHeader *newAbcFilesArray)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    [[maybe_unused]] EtsHandleScope hs(executionCtx);
    EtsHandle newAbcFilesHandle(executionCtx, EtsTypedObjectArray<EtsAbcFile>::FromCoreType(newAbcFilesArray));
    EtsHandle linkerHandle(executionCtx, runtimeLinker);
    auto *ctx = EtsClassLinkerContext::FromCoreType(linkerHandle->GetClassLinkerContext());

    os::memory::LockHolder lock(ctx->GetAbcFilesMutex());
    EtsHandle currentAbcFilesHandle(executionCtx, linkerHandle->GetAbcFiles());
    auto currentLength = currentAbcFilesHandle->GetLength();
    auto resultLength = newAbcFilesHandle->GetLength() + currentLength;
    EtsHandle resultAbcFilesHandle(
        executionCtx, EtsObjectArray::Create(PlatformTypes(executionCtx)->arkruntimeAbcFile, resultLength));
    if (UNLIKELY(resultAbcFilesHandle.GetPtr() == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return;
    }

    currentAbcFilesHandle->CopyDataTo(resultAbcFilesHandle.GetPtr());
    for (size_t start = currentLength, i = start; i < resultLength; ++i) {
        resultAbcFilesHandle->Set(i, newAbcFilesHandle->Get(i - start));
    }
    linkerHandle->SetAbcFiles(resultAbcFilesHandle.GetPtr());
}

EtsClass *EtsAbcRuntimeLinkerLoadClassFromAbcFiles(EtsAbcRuntimeLinker *runtimeLinker, EtsString *clsName,
                                                   EtsBoolean init)
{
    auto clsNameUtf8 = clsName->GetMutf8();
    PandaString descriptor;
    const auto *classDescriptor = ClassHelper::GetDescriptor(utf::CStringAsMutf8(clsNameUtf8.c_str()), &descriptor);
    if (UNLIKELY(classDescriptor == nullptr)) {
        return nullptr;
    }

    auto *executionCtx = EtsExecutionContext::GetCurrent();
    auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
    auto *errorHandler = PandaEtsVM::GetCurrent()->GetEtsClassLinkerExtension()->GetErrorHandler();
    auto *ctx = reinterpret_cast<EtsClassLinkerContext *>(runtimeLinker->GetClassLinkerContext());
    os::memory::LockHolder rlock(ctx->GetAbcFilesMutex());

    auto *abcFiles = runtimeLinker->GetAbcFiles();
    for (size_t i = 0, end = abcFiles->GetLength(); i < end; ++i) {
        auto *currentFile = abcFiles->Get(i);
        ASSERT(currentFile != nullptr);
        auto *pf = EtsAbcFile::FromEtsObject(currentFile)->GetPandaFile();
        const auto classId = pf->GetClassId(classDescriptor);
        if (!classId.IsValid() || pf->IsExternal(classId)) {
            continue;
        }

        auto *klass = classLinker->LoadClass(*pf, classId, ctx, errorHandler, true);
        if (UNLIKELY(klass == nullptr)) {
            ASSERT(executionCtx->GetMT()->HasPendingException());
            return nullptr;
        }

        if (UNLIKELY(init != 0 && !klass->IsInitialized())) {
            if (UNLIKELY(!classLinker->InitializeClass(executionCtx->GetMT(), klass))) {
                ASSERT(executionCtx->GetMT()->HasPendingException());
                return nullptr;
            }
        }
        return EtsClass::FromRuntimeClass(klass);
    }
    return nullptr;
}

}  // namespace ark::ets::intrinsics
