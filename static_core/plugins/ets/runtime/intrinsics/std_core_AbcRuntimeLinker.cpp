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
#include "runtime/include/stack_walker.h"
#include "libarkfile/file.h"
#include "include/object_header.h"
#include "intrinsics.h"
#include "libarkbase/os/mutex.h"
#include "plugins/ets/runtime/coldreload/ets_coldreload.h"
#include "plugins/ets/runtime/ets_class_linker_context.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/hotreload/ets_hotreload.h"
#include "runtime/include/thread_scopes.h"
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

/**
 * @brief `std.core.AbcRuntimeLinker.hotReload(targetPath: string, patchPath: string): int`
 *
 * The entry the application framework calls: validate arguments, copy the paths out of managed
 * memory, and hand over to `EtsHotreload`. Errors come back as the return value; an OOM during
 * preparation stays pending and propagates.
 *
 * @returns 0 on success, otherwise the ordinal of `ark::hotreload::Error`.
 */
extern "C" EtsInt EtsAbcRuntimeLinkerHotReload(EtsAbcRuntimeLinker *runtimeLinker, EtsString *targetPath,
                                               EtsString *patchPath)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);

    if (UNLIKELY(targetPath == nullptr || patchPath == nullptr)) {
        auto *executionCtx = EtsExecutionContext::GetCurrent();
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreNullPointerError,
                          "targetPath or patchPath is null");
        return static_cast<EtsInt>(ark::hotreload::Error::INVALID_INPUT_ARG);
    }

    /*
     * Root the managed arguments before anything below that may move objects (the stack walk of
     * the caller check, the string reads): a moving GC would leave the raw argument pointers
     * stale, while the handles are updated in place.
     */
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    [[maybe_unused]] EtsHandleScope hs(executionCtx);
    EtsHandle<EtsString> targetPathHandle(executionCtx, targetPath);
    EtsHandle<EtsString> patchPathHandle(executionCtx, patchPath);
    EtsHandle<EtsAbcRuntimeLinker> runtimeLinkerHandle(executionCtx, runtimeLinker);

    /*
     * Authorization: ANI by-name invocation does not check access modifiers, so a protected entry
     * is still reachable from an arbitrary native bridge with a managed frame below it. Only a
     * boot-context managed caller may run the transaction; a pure native entry (the framework's
     * shape, no managed frames at all) passes.
     */
    for (auto stack = StackWalker::Create(coro); stack.HasFrame(); stack.NextFrame()) {
        auto *method = stack.GetMethod();
        if (method == nullptr) {
            continue;
        }
        auto *ctx = method->GetClass()->GetLoadContext();
        if (ctx == nullptr || !ctx->IsBootContext()) {
            LOG(ERROR, HOTRELOAD) << "hotReload reached from a non-boot caller " << method->GetFullName();
            return static_cast<EtsInt>(ark::hotreload::Error::CALLER_NOT_BOOT);
        }
        break;
    }

    /*
     * Read everything that lives in managed memory NOW, while still in managed state: the
     * transaction below runs with no handle on these objects. The handle slots are checked
     * explicitly: the raw arguments were null-checked above, but `operator->` cannot prove
     * non-nullness of the slot to the static analyzer.
     */
    auto *targetStr = targetPathHandle.GetPtr();
    auto *patchStr = patchPathHandle.GetPtr();
    auto *linkerPtr = runtimeLinkerHandle.GetPtr();
    if (UNLIKELY(targetStr == nullptr || patchStr == nullptr || linkerPtr == nullptr)) {
        return static_cast<EtsInt>(ark::hotreload::Error::INVALID_INPUT_ARG);
    }
    PandaString target = targetStr->GetMutf8();
    PandaString patch = patchStr->GetMutf8();
    auto *expectedContext = linkerPtr->GetClassLinkerContext();

    ark::hotreload::Error err;
    {
        // `ArkHotreloadBase` must be constructed in NATIVE state; its own managed scope switches back
        ScopedNativeCodeThread nativeScope(coro);
        ets::hotreload::EtsHotreload reload(coro);
        err = reload.ReplaceAbc(target, patch, expectedContext);
    }

    if (err != ark::hotreload::Error::NONE) {
        LOG(ERROR, HOTRELOAD) << "hotReload failed: " << ark::hotreload::GetErrorString(err);
    } else {
        LOG(INFO, HOTRELOAD) << "hotReload succeeded: '" << target << "' -> '" << patch << "'";
    }
    return static_cast<EtsInt>(err);
}

}  // namespace ark::ets::intrinsics
