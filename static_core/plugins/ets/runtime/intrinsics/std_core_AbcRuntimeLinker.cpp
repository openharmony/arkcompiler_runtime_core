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
#include "plugins/ets/runtime/coldreload/ets_coldreload.h"
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

/**
 * @brief `std.core.AbcRuntimeLinker.coldReload(patchPath: string): int`
 *
 * Inserts the patch abc at the front of the linker's abcFiles array, so that
 * subsequent class lookups find patch classes first. Throws NullPointerException on a null
 * patchPath; every other failure comes back as an `ets::coldreload::Error` value.
 */
extern "C" EtsInt EtsAbcRuntimeLinkerColdReload(EtsAbcRuntimeLinker *runtimeLinker, EtsString *patchPath)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);

    if (UNLIKELY(patchPath == nullptr)) {
        auto *executionCtx = EtsExecutionContext::GetCurrent();
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreNullPointerError, "patchPath is null");
        return static_cast<EtsInt>(coldreload::Error::INVALID_INPUT_ARG);
    }

    // Read the path out of managed memory while still in managed state.
    PandaString patch = patchPath->GetMutf8();

    // Unlike `EtsHotreload`, whose base class must be constructed in native state,
    // `EtsColdReload` runs in managed state throughout; it enters a native scope
    // internally for the file read.
    coldreload::Error err;
    {
        ets::coldreload::EtsColdReload coldReload;
        err = coldReload.Reload(coro, patch, runtimeLinker);
    }

    if (err != coldreload::Error::NONE) {
        LOG(ERROR, COLDRELOAD) << "coldReload failed: " << ets::coldreload::GetErrorString(err);
    } else {
        LOG(INFO, COLDRELOAD) << "coldReload succeeded: patch '" << patch << "'";
    }
    return static_cast<EtsInt>(err);
}

}  // namespace ark::ets::intrinsics
