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

#include "libarkfile/file.h"
#include "include/object_header.h"
#include "include/thread_scopes.h"
#include "intrinsics.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/utf.h"
#include "runtime/handle_scope-inl.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
#include "plugins/ets/runtime/types/ets_abc_file.h"
#include "plugins/ets/runtime/types/ets_abc_package.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/types/ets_runtime_linker.h"
#include "plugins/ets/runtime/types/ets_string.h"

namespace ark::ets::intrinsics {

static bool IsHspPath(const std::string &path)
{
    return ets::EndsWith(path, ets::HSP_SUFFIX);
}

static bool IsHapPath(const std::string &path)
{
    return ets::EndsWith(path, ets::HAP_SUFFIX);
}

/*
 * Reads a package's abc entry through the shared package reader and translates the outcome into
 * the loader's contract: a package that cannot be opened raises AbcFileNotFoundError, a readable
 * package without the entry is a plain failure with a log (the historical asymmetry, kept).
 */
static bool GetPackageAbc(const std::string &pathStr, EtsExecutionContext *executionCtx,
                          std::unique_ptr<const panda_file::File> &pf, std::string_view suffix)
{
    switch (ets::ReadPackageAbc(pathStr, suffix, pf)) {
        case ets::PackageReadResult::OK:
            return true;
        case ets::PackageReadResult::OPEN_FAILED:
            ets::ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->arkruntimeAbcFileNotFoundError,
                                   "Open failed, file: " + pathStr);
            return false;
        case ets::PackageReadResult::NO_ABC_ENTRY:
        default:
            LOG(ERROR, RUNTIME) << "Failed to get safe data from ABC package: "
                                << ets::GetAbcPathFromPackagePath(pathStr, suffix);
            return false;
    }
}

static bool GetHspPath(const std::string &pathStr, EtsExecutionContext *executionCtx,
                       std::unique_ptr<const panda_file::File> &pf)
{
    return GetPackageAbc(pathStr, executionCtx, pf, ets::HSP_SUFFIX);
}

static bool GetHapPackagePath(const std::string &pathStr, EtsExecutionContext *executionCtx,
                              std::unique_ptr<const panda_file::File> &pf)
{
    return GetPackageAbc(pathStr, executionCtx, pf, ets::HAP_SUFFIX);
}

EtsAbcFile *EtsAbcFileLoadAbcFile(EtsRuntimeLinker *runtimeLinker, EtsString *filePath)
{
    if (UNLIKELY(runtimeLinker == nullptr || filePath == nullptr)) {
        ThrowNullPointerException();
        return nullptr;
    }

    auto *ctx = runtimeLinker->GetClassLinkerContext();
    auto *executionCtx = EtsExecutionContext::GetCurrent();

    const auto path = filePath->GetMutf8();
    std::unique_ptr<const panda_file::File> pf {nullptr};

    auto pathStr = std::string(path.begin(), path.end());
    // HAP/HSP packages store the ABC under the package-specific entry, while OpenPandaFileOrZip first tries the default
    // classes.abc entry. Handle packages before the generic path to avoid redundant archive lookups.
    if (IsHspPath(pathStr) && !GetHspPath(pathStr, executionCtx, pf)) {
        return nullptr;
    }
    if (IsHapPath(pathStr) && !GetHapPackagePath(pathStr, executionCtx, pf)) {
        return nullptr;
    }
#ifndef PANDA_TARGET_OHOS
    if (pf == nullptr) {
        // Loading panda-file might be time-consuming, which would affect GC
        // unless being executed in native scope
        ScopedNativeCodeThread etsNativeScope(executionCtx->GetMT());
        pf = panda_file::OpenPandaFileOrZip(path);
    }
#endif

    if (pf == nullptr) {
        ets::ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->arkruntimeAbcFileNotFoundError,
                               PandaString("Abc file not found: ") + path);
        return nullptr;
    }
    return EtsAbcFile::CreateAbcFile(executionCtx, ctx, std::move(pf));
}

EtsAbcFile *EtsAbcFileLoadFromMemory(EtsRuntimeLinker *runtimeLinker [[maybe_unused]],
                                     ObjectHeader *rawFileArray [[maybe_unused]])
{
#ifndef PANDA_TARGET_OHOS
    if (UNLIKELY(runtimeLinker == nullptr || rawFileArray == nullptr)) {
        ThrowNullPointerException();
        return nullptr;
    }
    ASSERT(rawFileArray != nullptr);
    ASSERT(runtimeLinker != nullptr);

    auto *executionCtx = EtsExecutionContext::GetCurrent();
    auto *array = EtsByteArray::FromCoreType(rawFileArray);

    auto pf = panda_file::OpenPandaFileFromMemory(array->GetData<void>(), array->GetLength());
    if (pf == nullptr) {
        ets::ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->escompatError,
                               PandaString("Failed to load abc file from memory"));
        return nullptr;
    }

    auto *ctx = runtimeLinker->GetClassLinkerContext();
    return EtsAbcFile::CreateAbcFile(executionCtx, ctx, std::move(pf));
#else
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ets::ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->escompatError,
                           "Load abc from memory is not supported");
    return nullptr;
#endif
}

EtsClass *EtsAbcFileLoadClass(EtsAbcFile *abcFile, EtsRuntimeLinker *runtimeLinker, EtsString *clsName, EtsBoolean init)
{
    if (UNLIKELY(runtimeLinker == nullptr || clsName == nullptr)) {
        ThrowNullPointerException();
        return nullptr;
    }

    const auto name = clsName->GetMutf8();
    PandaString descriptor;
    const auto *classDescriptor = ClassHelper::GetDescriptor(utf::CStringAsMutf8(name.c_str()), &descriptor);
    if (classDescriptor == nullptr) {
        return nullptr;
    }

    const auto *pf = abcFile->GetPandaFile();
    const auto classId = pf->GetClassId(classDescriptor);
    if (!classId.IsValid() || pf->IsExternal(classId)) {
        return nullptr;
    }

    auto *executionCtx = EtsExecutionContext::GetCurrent();
    auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
    auto *ctx = runtimeLinker->GetClassLinkerContext();
    auto *linkerErrorHandler = PandaEtsVM::GetCurrent()->GetEtsClassLinkerExtension()->GetErrorHandler();
    auto *klass = classLinker->LoadClass(*pf, classId, ctx, linkerErrorHandler, true);
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

EtsString *EtsAbcFileGetFilename(EtsAbcFile *abcFile)
{
    auto filename = abcFile->GetPandaFile()->GetFilename();
    return EtsString::CreateFromMUtf8(filename.c_str());
}

}  // namespace ark::ets::intrinsics
