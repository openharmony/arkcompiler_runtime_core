/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/coldreload/ets_coldreload.h"

#include <memory>
#include <string>
#include <string_view>

#include "plugins/ets/runtime/ets_class_linker_context.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/types/ets_abc_file.h"
#include "plugins/ets/runtime/types/ets_abc_package.h"
#include "plugins/ets/runtime/types/ets_abc_runtime_linker.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "compiler/aot/aot_manager.h"
#include "runtime/include/runtime.h"

namespace ark::ets::coldreload {

namespace {

/**
 * @returns the suffix of @a path if it names a package cold reload accepts, an empty view
 * otherwise. The suffixes and the entry names live in the shared package reader
 * (`types/ets_abc_package.h`); which of them a caller accepts is the caller's policy -- the
 * loader handles `.hap` / `.hsp` only, cold reload also handles the `.hqf` patch format.
 */
std::string_view PackageSuffixOf(const PandaString &path)
{
    for (auto suffix : {ets::HAP_SUFFIX, ets::HSP_SUFFIX, ets::HQF_SUFFIX}) {
        if (ets::EndsWith(path, suffix)) {
            return suffix;
        }
    }
    return {};
}

/*
 * Opens the patch through the shared package reader: a plain `.abc` directly, a `.hap` / `.hsp` /
 * `.hqf` package through its abc entry. A readable package without the entry (a native-only
 * patch carrying just `.so` files) sets @a noAbcEntry: cold reload only reorders class lookup,
 * so there is nothing for it to do. The secure-memory mapping behind a package entry outlives
 * the extractor (FileMapper does not unmap it), so the returned file stays readable for the
 * lifetime of the process.
 */
const panda_file::File *OpenPatchFile(const PandaString &patchPath, bool *noAbcEntry)
{
    *noAbcEntry = false;
    const auto suffix = PackageSuffixOf(patchPath);
    if (suffix.empty()) {
        // Plain `.abc` patch: opened through the shared libarkfile entry, like any other abc.
        auto pf = panda_file::OpenPandaFile(patchPath.c_str());
        if (pf == nullptr) {
            return nullptr;
        }
        return pf.release();
    }

    const std::string packagePath(patchPath.data(), patchPath.size());
    std::unique_ptr<const panda_file::File> pf;
    switch (ets::ReadPackageAbc(packagePath, suffix, pf)) {
        case ets::PackageReadResult::OK:
            return pf.release();
        case ets::PackageReadResult::NO_ABC_ENTRY:
            // The `.so` files such a patch carries are the framework's business; class lookup has
            // nothing to reorder.
            LOG(INFO, COLDRELOAD) << "coldreload: patch package '" << patchPath << "' has no '"
                                  << ets::PACKAGE_ABC_ENTRY << "' entry, nothing to prepend";
            *noAbcEntry = true;
            return nullptr;
        case ets::PackageReadResult::OPEN_FAILED:
        default:
            LOG(ERROR, COLDRELOAD) << "coldreload: cannot open patch package '" << patchPath << "'";
            return nullptr;
    }
}

}  // namespace

Error EtsColdReload::Reload(EtsCoroutine *coro, const PandaString &patchPath, EtsAbcRuntimeLinker *runtimeLinker)
{
    if (UNLIKELY(coro == nullptr || patchPath.empty() || runtimeLinker == nullptr)) {
        return Error::INVALID_INPUT_ARG;
    }

    // Create the handle before anything else: it protects the raw receiver pointer across
    // every later GC point (the native scope's exit below can move the linker).
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    [[maybe_unused]] EtsHandleScope hs(executionCtx);
    EtsHandle<EtsAbcRuntimeLinker> linkerHandle(executionCtx, runtimeLinker);

    /*
     * AOT code may embed old classes (an inlined method of a patched class), and is not
     * invalidated by the patch, so the entry is refused while any AOT file is loaded.
     */
    if (UNLIKELY(Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->HasAotFiles())) {
        LOG(ERROR, COLDRELOAD) << "coldreload: AOT files are loaded, cold reload is not available";
        return Error::AOT_LOADED;
    }

    // The contract is "startup, before any user code has run": once the receiver's context has
    // loaded classes, the prepend could no longer give them a consistent patched view.
    if (UNLIKELY(linkerHandle->GetClassLinkerContext()->NumLoadedClasses() != 0)) {
        LOG(ERROR, COLDRELOAD) << "coldreload: the module already has loaded classes, cold reload is not available";
        return Error::CLASSES_ALREADY_LOADED;
    }

    // Read and validate the patch in native scope; none of these steps runs managed code.
    {
        ScopedNativeCodeThread nativeScope(coro);
        bool noAbcEntry = false;
        const panda_file::File *raw = OpenPatchFile(patchPath, &noAbcEntry);
        if (noAbcEntry) {
            return Error::NONE;
        }
        if (UNLIKELY(raw == nullptr)) {
            return Error::PATCH_OPEN_FAILED;
        }
        patchFile_.reset(raw);
    }

    // Wrap the patch in a managed EtsAbcFile the way the loader does; the class
    // linker takes ownership of the File, the unique_ptr still covers the error paths.
    EtsHandle<EtsAbcFile> patchAbcFileHandle(
        executionCtx,
        EtsAbcFile::CreateAbcFile(executionCtx, linkerHandle->GetClassLinkerContext(), std::move(patchFile_)));
    if (UNLIKELY(patchAbcFileHandle.GetPtr() == nullptr)) {
        // Allocation failed with a pending exception; the unique_ptr released the file to the caller.
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return Error::INTERNAL;
    }

    // Prepend to abcFiles under the mutex.
    if (UNLIKELY(!linkerHandle->PrependAbcFile(executionCtx, patchAbcFileHandle.GetPtr()))) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return Error::INTERNAL;
    }

    LOG(INFO, COLDRELOAD) << "coldreload: patch '" << patchPath << "' prepended to abcFiles";
    return Error::NONE;
}

}  // namespace ark::ets::coldreload
