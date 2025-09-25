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
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
#include "runtime/include/signature_utils.h"
#include "plugins/ets/runtime/types/ets_abc_file.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/types/ets_runtime_linker.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "runtime/mem/local_object_handle.h"

namespace ark::ets::intrinsics {

static EtsAbcFile *CreateAbcFile(EtsCoroutine *coro, ClassLinkerContext *ctx,
                                 std::unique_ptr<const panda_file::File> &&pf)
{
    auto *abcFile = EtsAbcFile::FromEtsObject(EtsObject::Create(coro, PlatformTypes(coro)->coreAbcFile));
    abcFile->SetPandaFile(pf.get());

    Runtime::GetCurrent()->GetClassLinker()->AddPandaFile(std::move(pf), ctx);
    return abcFile;
}

static bool IsHspPath(const std::string &path)
{
    std::string suffix = ".hsp";
    return std::equal(suffix.rbegin(), suffix.rend(), path.rbegin());
}

static bool CheckExtractor(const std::shared_ptr<ark::extractor::Extractor> &extractor, EtsCoroutine *coro,
                           const std::string &path)
{
    if (!extractor || !extractor->Init()) {
        ets::ThrowEtsException(coro, PlatformTypes(coro)->coreAbcFileNotFoundError, "Open failed, file: " + path);
        return false;
    }
    return true;
}

EtsAbcFile *EtsAbcFileLoadAbcFile(EtsRuntimeLinker *runtimeLinker, EtsString *filePath)
{
    if (UNLIKELY(runtimeLinker == nullptr || filePath == nullptr)) {
        ThrowNullPointerException();
        return nullptr;
    }

    auto *ctx = runtimeLinker->GetClassLinkerContext();
    auto *coro = EtsCoroutine::GetCurrent();

    const auto path = filePath->GetMutf8();
    std::unique_ptr<const panda_file::File> pf {nullptr};
    {
        // Loading panda-file might be time-consuming, which would affect GC
        // unless being executed in native scope
        ScopedNativeCodeThread etsNativeScope(coro);
        pf = panda_file::OpenPandaFileOrZip(path);
    }

    auto pathStr = std::string(path.begin(), path.end());
    if (pf == nullptr && IsHspPath(pathStr)) {
        // get hsp path
        std::string hspPath = pathStr;
        std::shared_ptr<ark::extractor::Extractor> extractor = std::make_shared<ark::extractor::Extractor>(hspPath);
        if (!CheckExtractor(extractor, coro, pathStr)) {
            return nullptr;
        }
        std::string abcPath = hspPath.substr(0, hspPath.length() - 4).append("/ets/modules_static.abc");
        auto safeData = extractor->GetSafeDataForHsp(abcPath);
        if (safeData == nullptr) {
            LOG(ERROR, RUNTIME) << "Failed to get safe data from HSP archive: " << abcPath;
            return nullptr;
        }
        pf = panda_file::OpenPandaFileFromSecureMemory(safeData->GetDataPtr(), safeData->GetDataLen(), abcPath);
    }

    if (pf == nullptr) {
        // get hap path
        size_t pos = pathStr.rfind("/ets/");
        if (pos == std::string::npos) {
            ets::ThrowEtsException(coro, PlatformTypes(coro)->coreAbcFileNotFoundError,
                                   PandaString("Open failed, file: ") + path);
            return nullptr;
        }
        std::string hapPath = pathStr.substr(0, pos);
        hapPath += ".hap";

        std::shared_ptr<ark::extractor::Extractor> extractor = std::make_shared<ark::extractor::Extractor>(hapPath);
        if (!CheckExtractor(extractor, coro, pathStr)) {
            return nullptr;
        }
        auto safeData = extractor->GetSafeData(pathStr);
        if (safeData == nullptr) {
            LOG(ERROR, RUNTIME) << "Failed to get safe data from HAP archive: " << pathStr;
            return nullptr;
        }
        pf = panda_file::OpenPandaFileFromSecureMemory(safeData->GetDataPtr(), safeData->GetDataLen(), pathStr);
    }

    if (pf == nullptr) {
        ets::ThrowEtsException(coro, PlatformTypes(coro)->coreAbcFileNotFoundError,
                               PandaString("Abc file not found: ") + path);
        return nullptr;
    }
    return CreateAbcFile(coro, ctx, std::move(pf));
}

EtsAbcFile *EtsAbcFileLoadFromMemory(EtsRuntimeLinker *runtimeLinker, ObjectHeader *rawFileArray)
{
    if (UNLIKELY(runtimeLinker == nullptr || rawFileArray == nullptr)) {
        ThrowNullPointerException();
        return nullptr;
    }

    auto *ctx = runtimeLinker->GetClassLinkerContext();
    auto *coro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope hs(coro);
    EtsHandle<EtsByteArray> arrayHandle(coro, EtsByteArray::FromEtsObject(EtsObject::FromCoreType(rawFileArray)));

    auto length = arrayHandle->GetLength();
    size_t sizeToMmap = AlignUp(length, ark::os::mem::GetPageSize());
    void *mmapedMem = nullptr;
    {
        // mmap might be time-consuming, which would affect GC unless being executed in native scope
        [[maybe_unused]] ScopedNativeCodeThread ns(coro);
        mmapedMem = os::mem::MapRWAnonymousRaw(sizeToMmap, false);
    }
    if (UNLIKELY(mmapedMem == nullptr)) {
        ThrowEtsException(coro, PlatformTypes(coro)->escompatError,
                          PandaString("Failed to allocate in-memory AbcFile"));
        return nullptr;
    }
    if (UNLIKELY(memcpy_s(mmapedMem, sizeToMmap, arrayHandle->GetData<void>(), length) != 0)) {
        PLOG(FATAL, RUNTIME) << "Failed to copy buffer into mem";
    }
    os::mem::ConstBytePtr ptr(static_cast<std::byte *>(mmapedMem), sizeToMmap, os::mem::MmapDeleter);

    auto pf = panda_file::File::OpenFromMemory(std::move(ptr));
    if (pf == nullptr) {
        ThrowEtsException(coro, PlatformTypes(coro)->escompatError, PandaString("Failed to load abc file from memory"));
        return nullptr;
    }

    return CreateAbcFile(coro, ctx, std::move(pf));
}

EtsClass *EtsAbcFileLoadClass(EtsAbcFile *abcFile, EtsRuntimeLinker *runtimeLinker, EtsString *clsName, EtsBoolean init)
{
    if (UNLIKELY(runtimeLinker == nullptr || clsName == nullptr)) {
        ThrowNullPointerException();
        return nullptr;
    }

    const auto name = clsName->GetMutf8();
    auto normNameOpt = signature::NormalizePackageSeparators<PandaString, '.'>(name, 0, name.size());
    if (UNLIKELY(!normNameOpt.has_value())) {
        return nullptr;
    }

    PandaString descriptor;
    const auto *classDescriptor =
        ClassHelper::GetDescriptor(utf::CStringAsMutf8(normNameOpt.value().c_str()), &descriptor);
    if (classDescriptor == nullptr) {
        return nullptr;
    }

    const auto *pf = abcFile->GetPandaFile();
    const auto classId = pf->GetClassId(classDescriptor);
    if (!classId.IsValid() || pf->IsExternal(classId)) {
        return nullptr;
    }

    auto *coro = EtsCoroutine::GetCurrent();
    auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
    auto *ctx = runtimeLinker->GetClassLinkerContext();
    auto *linkerErrorHandler = PandaEtsVM::GetCurrent()->GetEtsClassLinkerExtension()->GetErrorHandler();
    auto *klass = classLinker->LoadClass(*pf, classId, ctx, linkerErrorHandler, true);
    if (UNLIKELY(klass == nullptr)) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }

    if (UNLIKELY(init != 0 && !klass->IsInitialized())) {
        if (UNLIKELY(!classLinker->InitializeClass(coro, klass))) {
            ASSERT(coro->HasPendingException());
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
