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

#include "plugins/ets/runtime/hotreload/ets_hotreload.h"

#include <fstream>
#include <optional>

#include "libarkbase/os/mem.h"
#include "libarkbase/utils/math_helpers.h"

#include "libarkbase/utils/utf.h"
#include "libziparchive/extractortool/extractor.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_class_linker_context.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/types/ets_abc_file.h"
#include "plugins/ets/runtime/types/ets_abc_runtime_linker.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "runtime/class_linker_context.h"
#include "runtime/hotreload/redirect.h"
#include "runtime/include/class_linker-inl.h"
#include "runtime/include/runtime.h"

namespace ark::ets::hotreload {

namespace {

/**
 * Class linker error handler that never throws a managed exception.
 *
 * The default ETS handler raises ETS errors, which would escape the transaction and violate the
 * "a failed hotreload changes nothing observable" contract. Loading the temporary classes must stay
 * a pure `Error` return path.
 */
class SilentErrorHandler final : public ClassLinkerErrorHandler {
public:
    void OnError(ClassLinker::Error error, const PandaString &message) override
    {
        LOG(ERROR, HOTRELOAD) << "class linker error (" << static_cast<int>(error) << "): " << message;
    }
};

/*
 * Package handling, kept in sync with `arkruntime_AbcFile.cpp`.
 *
 * It is duplicated on purpose rather than shared: the loader's version throws managed exceptions
 * and falls back to `OpenPandaFileOrZip` when the package cannot be read. Neither is acceptable
 * inside a transaction that must stay a pure `Error` return path and must never silently load
 * something other than the patch that was asked for.
 *
 * `.hqf` is a patch package; the user confirmed it is structurally identical to hap/hsp, so it goes
 * through the same reader with the same entry name.
 */
constexpr std::string_view HSP_SUFFIX = ".hsp";
constexpr std::string_view HAP_SUFFIX = ".hap";
constexpr std::string_view HQF_SUFFIX = ".hqf";
constexpr std::string_view PACKAGE_ABC_PATH = "/ets/modules_static.abc";
constexpr std::string_view PACKAGE_ABC_ENTRY = "ets/modules_static.abc";

bool EndsWith(const PandaString &path, std::string_view suffix)
{
    return path.length() >= suffix.length() &&
           path.compare(path.length() - suffix.length(), suffix.length(), suffix.data(), suffix.length()) == 0;
}

/// @returns the suffix of @a path if it names a package, an empty view otherwise.
std::string_view PackageSuffixOf(const PandaString &path)
{
    for (auto suffix : {HAP_SUFFIX, HSP_SUFFIX, HQF_SUFFIX}) {
        if (EndsWith(path, suffix)) {
            return suffix;
        }
    }
    return {};
}

/// @returns the whole content of @a path, or nothing if it cannot be read.
std::optional<PandaVector<uint8_t>> LoadWholeFile(const PandaString &path)
{
    std::ifstream stream(std::string(path.data(), path.size()), std::ios::binary | std::ios::ate);
    if (!stream) {
        return std::nullopt;
    }
    const auto size = stream.tellg();
    if (size <= 0) {
        return std::nullopt;
    }
    PandaVector<uint8_t> buffer(static_cast<size_t>(size));
    stream.seekg(0, std::ios::beg);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    if (!stream.read(reinterpret_cast<char *>(buffer.data()), size)) {
        return std::nullopt;
    }
    return buffer;
}

/**
 * @brief Open a plain abc file under a name of our choosing.
 *
 * `OpenPandaFile(location)` always records `location` as the file name, and
 * `OpenPandaFileFromMemory`'s third argument is a memory TAG, not a name --- it derives the name
 * from the mapping address. Neither can give the new generation the target's identity, so the
 * content is copied into an owned anonymous mapping and opened explicitly.
 */
std::unique_ptr<const panda_file::File> OpenAbcAs(const PandaString &path, std::string_view filename)
{
    auto buffer = LoadWholeFile(path);
    if (!buffer.has_value()) {
        return nullptr;
    }
    const size_t sizeToMmap = AlignUp(buffer->size(), ark::os::mem::GetPageSize());
    void *mem = ark::os::mem::MapRWAnonymousRaw(sizeToMmap, false);
    if (mem == nullptr) {
        return nullptr;
    }
    if (memcpy_s(mem, sizeToMmap, buffer->data(), buffer->size()) != 0) {
        ark::os::mem::UnmapRaw(mem, sizeToMmap);
        return nullptr;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ark::os::mem::ConstBytePtr ptr(reinterpret_cast<std::byte *>(mem), sizeToMmap, ark::os::mem::MmapDeleter);
    return panda_file::File::OpenFromMemory(std::move(ptr), filename);
}

/// @returns `<package path without suffix>/ets/modules_static.abc`, or @a path itself if it is a plain abc.
PandaString DeriveLoadedAbcPath(const PandaString &path)
{
    const auto suffix = PackageSuffixOf(path);
    if (suffix.empty()) {
        return path;
    }
    PandaString derived = path.substr(0, path.length() - suffix.length());
    derived.append(PACKAGE_ABC_PATH.data(), PACKAGE_ABC_PATH.length());
    return derived;
}

/*
 * @returns the context's `EtsRuntimeLinker`, or null when there is none: the boot context is not
 * an `EtsClassLinkerContext` at all and has no linker (a transaction over boot-context classes is
 * refused by `LangSpecificValidateClasses`; publication steps skip it). Never call
 * `EtsClassLinkerContext::FromCoreType` on a boot context -- it asserts and reinterpret-casts
 * wrongly.
 */
EtsRuntimeLinker *GetContextLinker(ClassLinkerContext *context)
{
    if (context->IsBootContext()) {
        return nullptr;
    }
    return EtsClassLinkerContext::FromCoreType(context)->GetRuntimeLinker();
}

}  // namespace

EtsHotreload::EtsHotreload(EtsCoroutine *coro)
    : ArkHotreloadBase(coro, panda_file::SourceLang::ETS),
      managedScope_(coro),
      publicationScope_(EtsExecutionContext::GetCurrent())
{
}

EtsHotreload::~EtsHotreload() = default;

Error EtsHotreload::LangSpecificQuiesceCheck() const
{
    ASSERT(oldPf_ != nullptr);
    ASSERT(context_ != nullptr);

    auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
    auto *ext = EtsClassLinkerExtension::FromCoreType(classLinker->GetExtension(lang_));

    Error err = CheckNoPendingLoads(classLinker);
    if (err != Error::NONE) {
        return err;
    }
    err = CheckNoLatePublishedClass(ext);
    if (err != Error::NONE) {
        return err;
    }
    return CheckAbcFilesUnchanged();
}

// Any in-flight load, in ANY context: a child-context builder (Derived extends Base)
// reads target-context method pointers into its vtable, so a context-local check misses it.
Error EtsHotreload::CheckNoPendingLoads(ClassLinker *classLinker) const
{
    if (classLinker->GetPendingLoads() != 0) {
        LOG(ERROR, HOTRELOAD) << "A class load is in flight in this VM, retry when it finishes";
        return Error::CONCURRENT_LOADING;
    }
    return Error::NONE;
}

// A class published after the collection snapshot: it is neither swapped nor in
// `methodsTable_`, so dispatch repair cannot reach it.
Error EtsHotreload::CheckNoLatePublishedClass(EtsClassLinkerExtension *ext) const
{
    const auto filename = oldPf_->GetFilename();
    const Class *late = nullptr;
    ext->EnumerateClasses([this, ext, &late, &filename](Class *cls) {
        // Array and union classes cannot be swap targets (see CollectClasses); one first created
        // inside the transaction window is not a late-published target class.
        if (cls->IsArrayClass() || cls->IsUnionClass()) {
            return true;
        }
        if (cls->GetLoadContext() != context_) {
            return true;
        }
        const auto *pf = cls->GetPandaFile();
        if (pf == nullptr || pf->GetFilename() != filename || (pf != oldPf_ && !ext->IsPandaFileSuperseded(pf))) {
            return true;
        }
        // Unpublished (obsolete, or mid-load) classes do not identify a generation
        if (context_->FindClass(cls->GetDescriptor()) != cls) {
            return true;
        }
        if (collectedClasses_.find(cls->GetDescriptor()) == collectedClasses_.cend()) {
            late = cls;
            return false;
        }
        return true;
    });
    if (late != nullptr) {
        LOG(ERROR, HOTRELOAD) << "Class " << utf::Mutf8AsCString(late->GetDescriptor())
                              << " was loaded after the transaction snapshot, retry to include it";
        return Error::CONCURRENT_LOADING;
    }
    return Error::NONE;
}

// The array must not have changed since `PrepareAbcFilePublication` and no `addAbcFiles`
// writer may be mid-flight (it holds the mutex and would publish a stale snapshot after the
// commit). TryLock is non-blocking, so it is safe under STW.
Error EtsHotreload::CheckAbcFilesUnchanged() const
{
    if (publicationArrayHandle_.GetPtr() == nullptr) {
        return Error::NONE;
    }
    auto *runtimeLinker = GetContextLinker(context_);
    if (runtimeLinker != nullptr && runtimeLinker->IsInstanceOf(PlatformTypes()->coreAbcRuntimeLinker)) {
        auto *linker = EtsAbcRuntimeLinker::FromEtsObject(runtimeLinker);
        auto *abcFiles = linker->GetAbcFiles();
        if (abcFiles == nullptr || abcFiles->GetLength() != publicationLength_ ||
            EtsAbcFile::FromEtsObject(abcFiles->Get(publicationSlot_))->GetPandaFile() != oldPf_) {
            LOG(ERROR, HOTRELOAD) << "abcFiles changed since preparation, retry with the updated array";
            return Error::CONCURRENT_LOADING;
        }
    }
    if (context_->IsBootContext()) {
        return Error::NONE;
    }
    auto &mutex = EtsClassLinkerContext::FromCoreType(context_)->GetAbcFilesMutex();
    if (!mutex.TryLock()) {
        LOG(ERROR, HOTRELOAD) << "An abcFiles writer (addAbcFiles) is in flight, retry when it finishes";
        return Error::CONCURRENT_LOADING;
    }
    mutex.Unlock();
    return Error::NONE;
}

Error EtsHotreload::LangSpecificValidateClasses()
{
    ASSERT_MANAGED_CODE();

    for (const auto &hCls : classes_) {
        // A fully loaded class always has its context set (runtime invariant)
        ASSERT(hCls.loadedClass->GetLoadContext() != nullptr);
        if (hCls.loadedClass->GetLoadContext()->IsBootContext()) {
            LOG(ERROR, HOTRELOAD) << "Class " << hCls.className_ << " belongs to the boot context, cannot reload";
            return Error::CLASS_UNMODIFIABLE;
        }
    }
    return Error::NONE;
}

/*
 * A function value's synthesised class caches its target `Method *` in `EtsClass::typeMetaData_`
 * (resolved once from the FunctionReference annotation). The swap leaves that slot alone, which
 * breaks lambda equality across a reload; re-point it at the new method. Obsolete classes are
 * excluded: they are self-consistent within their own generation.
 */
void EtsHotreload::RedirectFunctionReferenceTargets(EtsClassLinkerExtension *ext)
{
    PandaUnorderedSet<Class *> obsolete;
    ext->EnumerateObsoleteClasses([&obsolete](Class *cls) {
        obsolete.insert(cls);
        return true;
    });

    ext->EnumerateClasses([&obsolete](Class *cls) {
        if (obsolete.find(cls) != obsolete.end()) {
            return true;
        }
        auto *etsClass = EtsClass::FromRuntimeClass(cls);
        if (!etsClass->IsFunctionReference()) {
            return true;
        }
        auto *target = reinterpret_cast<Method *>(etsClass->GetTypeMetaData());
        if (target == nullptr) {
            return true;
        }
        auto *live = ark::hotreload::RedirectMethod(target);
        if (live != target) {
            etsClass->SetTypeMetaData(reinterpret_cast<EtsLong>(live));
        }
        return true;
    });
}

void EtsHotreload::LangSpecificHotreloadPart()
{
    ASSERT(oldPf_ != nullptr);
    ASSERT(newPf_ != nullptr);

    /*
     * The swap has passed its point of no return: this generation can no longer identify the
     * current target, even when a skipped class remains published from it. Record that before any
     * language-specific early return. Failed transactions never reach this hook.
     */
    auto *ext = EtsClassLinkerExtension::FromCoreType(Runtime::GetCurrent()->GetClassLinker()->GetExtension(lang_));
    ext->MarkPandaFileSuperseded(oldPf_);

    // Also before the early return below, and for the same reason: it is part of the commit, not
    // part of publishing the file
    RedirectFunctionReferenceTargets(ext);

    /*
     * Insert the patch in front of the target's slot in `abcFiles`, keeping the old file behind:
     * lazy lookups walk the array first-hit, so a class the patch omits falls through to the old
     * file. Runs under STW (no allocation; `GetAbcFilesMutex` deliberately not taken, D10).
     */
    auto *runtimeLinker = GetContextLinker(context_);
    if (runtimeLinker == nullptr || !runtimeLinker->IsInstanceOf(PlatformTypes()->coreAbcRuntimeLinker)) {
        // Not an `AbcRuntimeLinker` context: it has no `abcFiles` array to update
        LOG(WARNING, HOTRELOAD) << "Context has no AbcRuntimeLinker, new panda file is not published";
        return;
    }
    auto *linker = EtsAbcRuntimeLinker::FromEtsObject(runtimeLinker);

    if (publicationArrayHandle_.GetPtr() != nullptr) {
        auto *abcFiles = linker->GetAbcFiles();
        // The quiesce check validated the array in this same STW scope; the re-check is defensive.
        if (abcFiles != nullptr && abcFiles->GetLength() == publicationLength_ &&
            EtsAbcFile::FromEtsObject(abcFiles->Get(publicationSlot_))->GetPandaFile() == oldPf_) {
            auto *publicationArray = publicationArrayHandle_.GetPtr();
            for (uint32_t i = 0; i < publicationSlot_; ++i) {
                publicationArray->Set(i, abcFiles->Get(i));
            }
            publicationArray->Set(publicationSlot_, publicationWrapperHandle_.GetPtr()->AsObject());
            for (uint32_t i = publicationSlot_; i < publicationLength_; ++i) {
                publicationArray->Set(i + 1, abcFiles->Get(i));
            }
            linker->SetAbcFiles(publicationArray);
            LOG(DEBUG, HOTRELOAD) << "Published new panda file in front of AbcFile slot " << publicationSlot_;
            return;
        }
        LOG(WARNING, HOTRELOAD) << "abcFiles changed since the quiesce validation (defensive check)";
        return;
    }

    // Unprepared (target not owned by this linker's `abcFiles`): past PONR, warn only.
    LOG(WARNING, HOTRELOAD) << "No prepared publication and no in-place fallback, "
                            << "lazily loaded classes will still come from the old file";
}

Error EtsHotreload::FindTargetPandaFile(const PandaString &derivedPath, ClassLinkerContext *expectedContext,
                                        const panda_file::File **outPf)
{
    ASSERT_MANAGED_CODE();

    /*
     * Resolve the target by its live classes: the file list stays populated with superseded
     * generations, so scanning it would find the previous generation, not the current one.
     * Published classes of files marked superseded identify the target's lineage but not its
     * current generation; `expectedContext` restricts both steps to the caller's linker.
     */
    const std::string_view wanted(derivedPath.data(), derivedPath.size());
    auto *ext = EtsClassLinkerExtension::FromCoreType(Runtime::GetCurrent()->GetClassLinker()->GetExtension(lang_));

    // Collect first, filter later: `FindClass` and `EnumerateClasses` share a non-recursive lock
    PandaVector<Class *> candidates;
    ext->EnumerateClasses([&candidates, wanted, expectedContext](Class *cls) {
        if (expectedContext != nullptr && cls->GetLoadContext() != expectedContext) {
            return true;
        }
        const auto *pf = cls->GetPandaFile();
        if (pf != nullptr && pf->GetFilename() == wanted) {
            candidates.push_back(cls);
        }
        return true;
    });

    const panda_file::File *found = nullptr;
    for (auto *cls : candidates) {
        // A fully loaded class always has its context set (runtime invariant)
        ASSERT(cls->GetLoadContext() != nullptr);
        // Unpublished (obsolete, or mid-load) classes do not identify a generation
        if (cls->GetLoadContext()->FindClass(cls->GetDescriptor()) != cls) {
            continue;
        }
        const auto *pf = cls->GetPandaFile();
        if (ext->IsPandaFileSuperseded(pf)) {
            continue;
        }
        if (found == nullptr) {
            found = pf;
        } else if (found != pf) {
            LOG(ERROR, HOTRELOAD) << "Target abc '" << derivedPath << "' resolves to more than one live panda file";
            return Error::TARGET_ABC_AMBIGUOUS;
        }
    }

    if (found == nullptr) {
        LOG(ERROR, HOTRELOAD) << "Target abc '" << derivedPath << "' has no live loaded classes in this VM";
        return Error::TARGET_ABC_NOT_FOUND;
    }

    *outPf = found;
    return Error::NONE;
}

const panda_file::File *EtsHotreload::OpenPatchPandaFile(const PandaString &patchPath, const PandaString &derivedPath)
{
    ASSERT_MANAGED_CODE();

    // Open under the target's derived name so the caller's stable targetPath keeps resolving
    // across generations (generations also collide in DebugInf by name; debugger coexistence
    // is out of scope).
    const std::string pfName(derivedPath.data(), derivedPath.size());

    const auto suffix = PackageSuffixOf(patchPath);
    if (suffix.empty()) {
        // Plain `.abc` file: the host-test shape. Read it and re-open under the target's name.
        auto pf = OpenAbcAs(patchPath, pfName);
        if (pf == nullptr) {
            LOG(ERROR, HOTRELOAD) << "Cannot open patch file '" << patchPath << "'";
            return nullptr;
        }
        return OwnPandaFile(std::move(pf));
    }

    const std::string packagePath(patchPath.data(), patchPath.size());
    auto extractor = std::make_shared<ark::extractor::Extractor>(packagePath);
    if (!extractor->Init()) {
        LOG(ERROR, HOTRELOAD) << "Cannot open patch package '" << patchPath << "'";
        return nullptr;
    }

    /*
     * `GetSafeData` (the hap variant) verifies entry integrity via `IsEntryDataConsistent`, which
     * `GetSafeDataForHsp` does not. A patch is pushed onto the device from outside, so the check is
     * kept for `.hqf` as well; only genuine `.hsp` targets use the unchecked reader.
     */
    const PandaString derivedAbcPath = DeriveLoadedAbcPath(patchPath);
    const std::string hspEntry(derivedAbcPath.data(), derivedAbcPath.size());
    auto safeData = (suffix == HSP_SUFFIX) ? extractor->GetSafeDataForHsp(hspEntry)
                                           : extractor->GetSafeData(std::string(PACKAGE_ABC_ENTRY));
    if (safeData == nullptr) {
        LOG(ERROR, HOTRELOAD) << "Patch package '" << patchPath << "' has no readable entry '" << PACKAGE_ABC_ENTRY
                              << "'";
        return nullptr;
    }

    /*
     * The file name given here becomes `File::GetFilename()` of the new generation. It must be the
     * target's derived name, just like the plain-abc path above, so the caller can keep using one
     * stable target identity. `DebugInf::AddCodeMetaInfo` therefore sees generations collide by
     * file name; that is accepted because debugger coexistence is outside hot-reload's scope.
     *
     * `~FileMapper` deliberately does not unmap `SAFE_ABC` data, which is why the empty deleter
     * inside `OpenPandaFileFromSecureMemory` is safe. The flip side is that the mapping lives until
     * the process exits --- see the "repeated reload memory growth" acceptance item.
     */
    auto pf = panda_file::OpenPandaFileFromSecureMemory(safeData->GetDataPtr(), safeData->GetDataLen(), pfName);
    if (pf == nullptr) {
        LOG(ERROR, HOTRELOAD) << "Entry of patch package '" << patchPath << "' is not a valid abc file";
        return nullptr;
    }
    return OwnPandaFile(std::move(pf));
}

Error EtsHotreload::CollectClasses(const panda_file::File *currentPf, const panda_file::File *newPf,
                                   ClassLinkerContext *expectedContext)
{
    ASSERT_MANAGED_CODE();

    auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
    auto *ext = EtsClassLinkerExtension::FromCoreType(classLinker->GetExtension(lang_));

    /*
     * Collect first, filter later: `ClassLinkerContext::FindClass` takes the same non-recursive lock
     * as `ClassLinkerContext::EnumerateClasses`, so it must not be called from inside the callback.
     * Keep the receiver-context filter here as well as in target lookup: superseded files from an
     * independent linker may have the same name, but are not part of this target's lineage.
     */
    PandaVector<Class *> candidates;
    const auto filename = currentPf->GetFilename();
    ext->EnumerateClasses([&candidates, ext, currentPf, filename, expectedContext](Class *cls) {
        // Array and union classes are synthesised from their component classes: they are never
        // declared in a patch and are never swap targets, only their component classes are.
        if (cls->IsArrayClass() || cls->IsUnionClass()) {
            return true;
        }
        if (expectedContext != nullptr && cls->GetLoadContext() != expectedContext) {
            return true;
        }
        const auto *pf = cls->GetPandaFile();
        if (pf != nullptr && pf->GetFilename() == filename && (pf == currentPf || ext->IsPandaFileSuperseded(pf))) {
            candidates.push_back(cls);
        }
        return true;
    });

    PandaVector<Class *> targets;
    targets.reserve(candidates.size());
    ClassLinkerContext *context = nullptr;
    for (auto *cls : candidates) {
        // A fully loaded class always has its context set (runtime invariant)
        ASSERT(cls->GetLoadContext() != nullptr);
        auto *ctx = cls->GetLoadContext();
        // Drops obsolete classes of a previous reload and classes not published in their context
        if (ctx->FindClass(cls->GetDescriptor()) != cls) {
            continue;
        }
        if (context == nullptr) {
            context = ctx;
        } else if (context != ctx) {
            LOG(ERROR, HOTRELOAD) << "Target abc is loaded into more than one class linker context";
            return Error::TARGET_ABC_AMBIGUOUS;
        }
        targets.push_back(cls);
    }

    if (targets.empty()) {
        LOG(ERROR, HOTRELOAD) << "No loaded classes found for the target abc";
        return Error::CLASS_NOT_FOUND;
    }

    // The quiesce check replays the collection under stop-the-world and compares against this
    collectedClasses_.reserve(targets.size());
    for (const auto *cls : targets) {
        collectedClasses_.insert(cls->GetDescriptor());
    }

    return LoadTemporaryClasses(newPf, targets, classLinker, context);
}

Error EtsHotreload::LoadTemporaryClasses(const panda_file::File *newPf, const PandaVector<Class *> &targets,
                                         ClassLinker *classLinker, ClassLinkerContext *context)
{
    SilentErrorHandler errorHandler;
    classes_.reserve(targets.size());
    context_ = context;

    for (auto *cls : targets) {
        const uint8_t *descriptor = cls->GetDescriptor();
        std::string className(utf::Mutf8AsCString(descriptor));

        auto classId = newPf->GetClassId(descriptor);
        if (!classId.IsValid() || newPf->IsExternal(classId)) {
            /*
             * A class of the target that the patch does not carry. With incremental compilation
             * this is the common case: the patch holds only the classes that changed, and every
             * untouched class is simply absent. Keeping the class on the old abc is the correct
             * outcome for both readings of the absence --- "unchanged" (nothing to do) and
             * "deleted" (the framework can report it, the runtime keeps the old code live so
             * that existing objects keep working). Either way the class is NOT skipped by the
             * drop mechanism below: it simply does not participate in this transaction.
             */
            LOG(DEBUG, HOTRELOAD) << "Class " << className << " is not in the patch abc, keeping old version";
            continue;
        }

        // addToRuntime = false: load as temporary, never publish by descriptor
        auto *tmpClass = classLinker->LoadClass(*newPf, classId, context, &errorHandler, false);
        if (tmpClass == nullptr) {
            LOG(ERROR, HOTRELOAD) << "Cannot load class " << className << " from the patch abc";
            return Error::INVALID_CLASS_FORMAT;
        }

        LOG(DEBUG, HOTRELOAD) << "Prepared " << className << ": loaded=" << cls << " tmp=" << tmpClass;
        classes_.push_back(ark::hotreload::ClassContainment {newPf, classId, std::move(className), cls, tmpClass,
                                                             ark::hotreload::ChangesFlags::F_NO_STRUCT_CHANGES});
    }

    /*
     * NOTE(hotreload-mvp): only classes that are already LOADED are compared. Classes that exist
     * only in the patch, and classes of the target that nothing has touched yet, are not examined
     * at all --- and once the commit publishes the new file they are simply loaded from it. So a
     * structural change confined to an unloaded class is not detected. Catching it means diffing
     * whole-file metadata instead of loaded classes.
     */
    return Error::NONE;
}

Error EtsHotreload::ReplaceAbc(const PandaString &targetAbcPath, const PandaString &patchAbcPath,
                               ClassLinkerContext *expectedContext)
{
    ASSERT_MANAGED_CODE();

    // Gates must run before anything is opened or loaded
    Error err = AdmissionCheck();
    if (err != Error::NONE) {
        return err;
    }

    if (targetAbcPath.empty() || patchAbcPath.empty()) {
        return Error::INVALID_INPUT_ARG;
    }

    // Keyed on the string the CALLER passed, not the derived one: that is what the tool watching
    // the log recognises as "the module I asked to reload"
    targetName_ = targetAbcPath;

    /*
     * The caller passes the path it originally handed to `addAbcFiles`, which on a device is a
     * `.hap` / `.hsp` PACKAGE path. The loader does not store that string: it derives
     * `<package path without suffix>/ets/modules_static.abc` and that derived string is what ends
     * up in `File::GetFilename()` (see `arkruntime_AbcFile.cpp`). Everything below therefore works
     * on the derived form, and each new generation is opened under it as well, so the caller's
     * `targetPath` keeps identifying the same slot no matter how many reloads have happened.
     *
     * NOTE(hotreload-mvp): this reproduces the loader's rule instead of recording the original
     * path. The robust fix is an `srcPath_` field on `EtsAbcFile`, which also gives the hap/hsp
     * provenance gate a place to live.
     */
    const PandaString derivedTargetPath = DeriveLoadedAbcPath(targetAbcPath);

    const panda_file::File *oldPf = nullptr;
    err = FindTargetPandaFile(derivedTargetPath, expectedContext, &oldPf);
    if (err != Error::NONE) {
        return err;
    }

    const panda_file::File *newPf = OpenPatchPandaFile(patchAbcPath, derivedTargetPath);
    if (newPf == nullptr) {
        return Error::PATCH_OPEN_FAILED;
    }
    if (newPf == oldPf) {
        LOG(ERROR, HOTRELOAD) << "Patch and target resolve to the same panda file";
        return Error::INVALID_INPUT_ARG;
    }

    err = CollectClasses(oldPf, newPf, expectedContext);
    if (err != Error::NONE) {
        return err;
    }

    /*
     * `CollectClasses` derived the context from the loaded classes themselves. When the caller came
     * in through a specific `AbcRuntimeLinker`, that derived context must be the linker's own,
     * otherwise the caller is asking to patch an abc that belongs to somebody else.
     */
    if (expectedContext != nullptr && context_ != expectedContext) {
        LOG(ERROR, HOTRELOAD) << "Target abc '" << targetAbcPath << "' does not belong to the given runtime linker";
        return Error::TARGET_ABC_NOT_FOUND;
    }

    err = CheckPublicationTarget(targetAbcPath);
    if (err != Error::NONE) {
        return err;
    }

    oldPf_ = oldPf;
    newPf_ = newPf;

    // Last thing that can fail or allocate: the commit is pure mutation
    err = PrepareAbcFilePublication();
    if (err != Error::NONE) {
        return err;
    }

    LOG(INFO, HOTRELOAD) << "Reloading " << classes_.size() << " classes: '" << targetAbcPath << "' -> '"
                         << patchAbcPath << "'";
    return ProcessHotreload();
}

/*
 * Publication targets: the boot context has no linker at all (its classes are also refused
 * per class in `LangSpecificValidateClasses`), and a non-boot context without an
 * `AbcRuntimeLinker` has no `abcFiles` to publish into --- committing without publication would
 * serve later lazy loads from the old file, a mixed generation. Both are refused up front,
 * before anything is swapped.
 */
Error EtsHotreload::CheckPublicationTarget(const PandaString &targetAbcPath) const
{
    if (context_->IsBootContext()) {
        LOG(ERROR, HOTRELOAD) << "Target abc '" << targetAbcPath << "' belongs to the boot context, cannot reload";
        return Error::CLASS_UNMODIFIABLE;
    }
    auto *runtimeLinker = GetContextLinker(context_);
    if (runtimeLinker == nullptr || !runtimeLinker->IsInstanceOf(PlatformTypes()->coreAbcRuntimeLinker)) {
        LOG(ERROR, HOTRELOAD) << "Target abc '" << targetAbcPath << "' belongs to a context without an"
                              << " AbcRuntimeLinker, nowhere to publish";
        return Error::NO_ABC_RUNTIME_LINKER;
    }
    return Error::NONE;
}

Error EtsHotreload::PrepareAbcFilePublication()
{
    ASSERT_MANAGED_CODE();
    ASSERT(oldPf_ != nullptr);
    ASSERT(newPf_ != nullptr);
    ASSERT(context_ != nullptr);

    auto *runtimeLinker = GetContextLinker(context_);
    if (runtimeLinker == nullptr || !runtimeLinker->IsInstanceOf(PlatformTypes()->coreAbcRuntimeLinker)) {
        // Not an `AbcRuntimeLinker` context; the commit warns and publishes nothing
        return Error::NONE;
    }
    auto *abcFiles = EtsAbcRuntimeLinker::FromEtsObject(runtimeLinker)->GetAbcFiles();
    if (abcFiles == nullptr) {
        return Error::NONE;
    }

    // The slot is matched by panda file IDENTITY, not by path, so it is exact by construction
    uint32_t slot = abcFiles->GetLength();
    for (uint32_t i = 0, end = abcFiles->GetLength(); i < end; ++i) {
        if (EtsAbcFile::FromEtsObject(abcFiles->Get(i))->GetPandaFile() == oldPf_) {
            slot = i;
            break;
        }
    }
    if (slot == abcFiles->GetLength()) {
        // The commit re-checks by identity and warns; the target belongs to another owner
        return Error::NONE;
    }

    auto *executionCtx = EtsExecutionContext::GetCurrent();

    // Root the live array first: the allocations below can trigger a GC that would move it.
    EtsHandle<EtsObjectArray> abcFilesHandle(executionCtx, abcFiles);

    // Movable allocations; each object is rooted the moment it is allocated, so the GC a later
    // allocation may trigger cannot leave an earlier pointer stale. The member handles hold
    // slots in `publicationScope_`, which lives for the whole transaction.
    publicationWrapperHandle_ = EtsHandle<EtsAbcFile>(
        executionCtx,
        EtsAbcFile::FromEtsObject(EtsObject::Create(executionCtx, PlatformTypes(executionCtx)->arkruntimeAbcFile)));
    if (publicationWrapperHandle_.GetPtr() == nullptr) {
        // OOM: the transaction is aborted (nothing modified). The pending exception is left on
        // the thread so it propagates like any other allocation failure.
        return Error::INTERNAL;
    }
    publicationArrayHandle_ =
        EtsHandle<EtsObjectArray>(executionCtx, EtsObjectArray::Create(PlatformTypes(executionCtx)->arkruntimeAbcFile,
                                                                       abcFilesHandle->GetLength() + 1));
    if (publicationArrayHandle_.GetPtr() == nullptr) {
        // OOM: the transaction is aborted (nothing modified). The pending exception is left on
        // the thread so it propagates like any other allocation failure.
        return Error::INTERNAL;
    }
    publicationWrapperHandle_->SetPandaFile(newPf_);

    publicationSlot_ = slot;
    publicationLength_ = abcFilesHandle->GetLength();
    return Error::NONE;
}

}  // namespace ark::ets::hotreload
