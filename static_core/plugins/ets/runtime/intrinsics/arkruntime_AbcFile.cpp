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
#include "libziparchive/extractortool/extractor.h"
#include "runtime/handle_scope-inl.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
#include "plugins/ets/runtime/types/ets_abc_file.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/types/ets_runtime_linker.h"
#include "plugins/ets/runtime/types/ets_string.h"

namespace ark::ets::intrinsics {

static EtsAbcFile *CreateAbcFile(EtsExecutionContext *executionCtx, ClassLinkerContext *ctx,
                                 std::unique_ptr<const panda_file::File> &&pf)
{
    auto *abcFile =
        EtsAbcFile::FromEtsObject(EtsObject::Create(executionCtx, PlatformTypes(executionCtx)->arkruntimeAbcFile));
    abcFile->SetPandaFile(pf.get());

    Runtime::GetCurrent()->GetClassLinker()->AddPandaFile(std::move(pf), ctx);
    return abcFile;
}

static bool IsHspPath(const std::string &path)
{
    std::string suffix = ".hsp";
    return std::equal(suffix.rbegin(), suffix.rend(), path.rbegin());
}

static bool CheckExtractor(const std::shared_ptr<ark::extractor::Extractor> &extractor,
                           EtsExecutionContext *executionCtx, const std::string &path)
{
    if (!extractor || !extractor->Init()) {
        ets::ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->arkruntimeAbcFileNotFoundError,
                               "Open failed, file: " + path);
        return false;
    }
    return true;
}

static bool GetHspPath(const std::string &pathStr, EtsExecutionContext *executionCtx,
                       std::unique_ptr<const panda_file::File> &pf)
{
    std::string hspPath = pathStr;
    std::shared_ptr<ark::extractor::Extractor> extractor = std::make_shared<ark::extractor::Extractor>(hspPath);
    if (!CheckExtractor(extractor, executionCtx, pathStr)) {
        return false;
    }
    std::string abcPath = hspPath.substr(0, hspPath.length() - 4).append("/ets/modules_static.abc");
    auto safeData = extractor->GetSafeDataForHsp(abcPath);
    if (safeData == nullptr) {
        LOG(ERROR, RUNTIME) << "Failed to get safe data from HSP archive: " << abcPath;
        return false;
    }
    pf = panda_file::OpenPandaFileFromSecureMemory(safeData->GetDataPtr(), safeData->GetDataLen(), abcPath);
    return true;
}

static bool GetHapPath(const std::string &pathStr, EtsExecutionContext *executionCtx,
                       std::unique_ptr<const panda_file::File> &pf)
{
    size_t pos = pathStr.rfind("/ets/");
    if (pos == std::string::npos) {
        std::string message = std::string("Open failed, file: ") + pathStr;
        ets::ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->arkruntimeAbcFileNotFoundError, message);
        return false;
    }
    std::string hapPath = pathStr.substr(0, pos);
    hapPath += ".hap";

    std::shared_ptr<ark::extractor::Extractor> extractor = std::make_shared<ark::extractor::Extractor>(hapPath);
    if (!CheckExtractor(extractor, executionCtx, pathStr)) {
        return false;
    }
    auto safeData = extractor->GetSafeData(pathStr);
    if (safeData == nullptr) {
        LOG(ERROR, RUNTIME) << "Failed to get safe data from HAP archive: " << pathStr;
        return false;
    }
    pf = panda_file::OpenPandaFileFromSecureMemory(safeData->GetDataPtr(), safeData->GetDataLen(), pathStr);
    return true;
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
    {
        // Loading panda-file might be time-consuming, which would affect GC
        // unless being executed in native scope
        ScopedNativeCodeThread etsNativeScope(executionCtx->GetMT());
        pf = panda_file::OpenPandaFileOrZip(path);
    }

    auto pathStr = std::string(path.begin(), path.end());
    if (pf == nullptr && IsHspPath(pathStr) && !GetHspPath(pathStr, executionCtx, pf)) {
        return nullptr;
    }
    if (pf == nullptr && !GetHapPath(pathStr, executionCtx, pf)) {
        return nullptr;
    }
    if (pf == nullptr) {
        ets::ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->arkruntimeAbcFileNotFoundError,
                               PandaString("Abc file not found: ") + path);
        return nullptr;
    }
    return CreateAbcFile(executionCtx, ctx, std::move(pf));
}

EtsAbcFile *EtsAbcFileLoadFromMemory(EtsRuntimeLinker *runtimeLinker, ObjectHeader *rawFileArray)
{
    if (UNLIKELY(runtimeLinker == nullptr || rawFileArray == nullptr)) {
        ThrowNullPointerException();
        return nullptr;
    }
    ASSERT(rawFileArray != nullptr);
    ASSERT(runtimeLinker != nullptr);

    auto *ctx = runtimeLinker->GetClassLinkerContext();
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    [[maybe_unused]] EtsHandleScope hs(executionCtx);
    EtsHandle<EtsByteArray> arrayHandle(executionCtx,
                                        EtsByteArray::FromEtsObject(EtsObject::FromCoreType(rawFileArray)));

    auto length = arrayHandle->GetLength();
    size_t sizeToMmap = AlignUp(length, ark::os::mem::GetPageSize());
    void *mmapedMem = nullptr;
    {
        // mmap might be time-consuming, which would affect GC unless being executed in native scope
        [[maybe_unused]] ScopedNativeCodeThread ns(executionCtx->GetMT());
        mmapedMem = os::mem::MapRWAnonymousRaw(sizeToMmap, false);
    }
    if (UNLIKELY(mmapedMem == nullptr)) {
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreError,
                          PandaString("Failed to allocate in-memory AbcFile"));
        return nullptr;
    }
    if (UNLIKELY(memcpy_s(mmapedMem, sizeToMmap, arrayHandle->GetData<void>(), length) != 0)) {
        PLOG(FATAL, RUNTIME) << "Failed to copy buffer into mem";
    }
    os::mem::ConstBytePtr ptr(static_cast<std::byte *>(mmapedMem), sizeToMmap, os::mem::MmapDeleter);

    auto pf = panda_file::File::OpenFromMemory(std::move(ptr));
    if (pf == nullptr) {
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreError,
                          PandaString("Failed to load abc file from memory"));
        return nullptr;
    }

    return CreateAbcFile(executionCtx, ctx, std::move(pf));
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
