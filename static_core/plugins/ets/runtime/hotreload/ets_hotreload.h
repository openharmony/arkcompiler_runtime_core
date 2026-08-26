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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_HOTRELOAD_ETS_HOTRELOAD_H_
#define PANDA_PLUGINS_ETS_RUNTIME_HOTRELOAD_ETS_HOTRELOAD_H_

#include "libarkbase/utils/utf.h"
#include "runtime/hotreload/hotreload.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/thread_scopes.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/types/ets_array.h"

namespace ark::ets {
class EtsClassLinkerExtension;
class EtsAbcFile;
}  // namespace ark::ets

namespace ark::ets::hotreload {

using Error = ark::hotreload::Error;

/**
 * @brief ETS hotreload facade: file resolution, patch reading, publication into `abcFiles`.
 *
 * Errors are returned as `Error`, never thrown (an OOM during preparation stays pending).
 * Must be constructed in NATIVE state; `managedScope_` switches to MANAGED for the transaction.
 */
class EtsHotreload final : public ark::hotreload::ArkHotreloadBase {
public:
    explicit EtsHotreload(EtsCoroutine *coro);
    ~EtsHotreload() override;

    NO_COPY_SEMANTIC(EtsHotreload);
    NO_MOVE_SEMANTIC(EtsHotreload);

    /**
     * @brief Replace an already loaded abc file by a new version of the same file.
     * @param targetAbcPath path the target abc was loaded with. A `.hap` / `.hsp` package path is
     *                      accepted and resolved the same way the loader resolves it.
     * @param patchAbcPath  path of the new version: a plain `.abc`, or a `.hqf` / `.hap` / `.hsp`
     *                      package holding `ets/modules_static.abc`.
     * @param expectedContext if not null, the target must belong to this class linker context;
     *                        used by the `AbcRuntimeLinker` entry to reject "wrong linker asked".
     * @return Error::NONE on success; on any error nothing in the VM has been modified.
     */
    Error ReplaceAbc(const PandaString &targetAbcPath, const PandaString &patchAbcPath,
                     ClassLinkerContext *expectedContext = nullptr);

private:
    Error LangSpecificValidateClasses() override;
    void LangSpecificHotreloadPart() override;
    Error LangSpecificQuiesceCheck() const override;

    Error CheckNoPendingLoads(ClassLinker *classLinker) const;
    Error CheckNoLatePublishedClass(EtsClassLinkerExtension *ext) const;
    Error CheckAbcFilesUnchanged() const;

    /**
     * @brief Re-point every live function-reference class at the replacement of its target method.
     *
     * `EtsClass::typeMetaData_` caches the `Method *` named by the class's `FunctionReference`
     * annotation, resolved once at class load. The swap does not reach it. It feeds lambda equality,
     * not dispatch, so what it costs is that two values denoting the same function compare unequal
     * when one of their synthesised classes was loaded before the reload and the other after.
     */
    void RedirectFunctionReferenceTargets(EtsClassLinkerExtension *ext);

    /**
     * @brief Find the current, non-superseded panda file behind @a derivedPath.
     * @param derivedPath the target path in its loaded (derived) form, see `DeriveLoadedAbcPath`
     * @param expectedContext when present, consider only files loaded by that linker context
     * @returns the authoritative file for the next generation. Superseded files may still own
     *          published skipped classes, but do not participate in target selection.
     */
    Error FindTargetPandaFile(const PandaString &derivedPath, ClassLinkerContext *expectedContext,
                              const panda_file::File **outPf);

    /**
     * @brief Open the patch, unpacking it first if it is a `.hqf` / `.hap` / `.hsp` package.
     * @param patchPath   where to read the new code from
     * @param derivedPath the name to open it under: the target's derived path, so that the caller's
     *                    `targetPath` still resolves to it on the next reload
     * @returns the owned file, or nullptr if it cannot be read. Never throws a managed exception
     *          and never falls back to a different file.
     */
    const panda_file::File *OpenPatchPandaFile(const PandaString &patchPath, const PandaString &derivedPath);

    Error CollectClasses(const panda_file::File *currentPf, const panda_file::File *newPf,
                         ClassLinkerContext *expectedContext);

    /** @brief For each target class, find its match in the patch and load it as a temporary class. */
    Error LoadTemporaryClasses(const panda_file::File *newPf, const PandaVector<Class *> &targets,
                               ClassLinker *classLinker, ClassLinkerContext *context);

    /**
     * @brief Pre-allocate the wrapper and grown array for the commit's publication.
     *
     * The commit inserts the patch in front of the target's `abcFiles` slot (keeping the old
     * file behind, so incremental patches fall through). Nothing may allocate under STW, so both
     * objects are created here and rooted by the member handle scope.
     *
     * @return Error::NONE, or Error::INTERNAL on OOM (transaction aborts, nothing modified).
     */
    Error CheckPublicationTarget(const PandaString &targetAbcPath) const;
    Error PrepareAbcFilePublication();

    /*
     * The two files of the transaction, kept for `LangSpecificHotreloadPart()`.
     *
     * They cannot be recovered from `classes_` there: `AddObsoleteClassesToRuntime()` clears that
     * vector, and it runs BEFORE the language hook. Raw `panda_file::File *` are not managed
     * objects, so holding them across the transaction needs no handle.
     */
    const panda_file::File *oldPf_ = nullptr;
    const panda_file::File *newPf_ = nullptr;

    /*
     * NATIVE -> MANAGED, must be constructed BEFORE every member that touches managed state:
     * `publicationScope_` below creates and destroys a handle scope, which must not happen in
     * native state -- a native-state thread is not paused during STW and holds no mutator lock,
     * so it would race the GC's handle scan (asserted in handle_scope-inl.h). Symmetrically the
     * scope must be declared AFTER those members so that it is destroyed last, keeping the
     * thread managed while they are torn down.
     */
    ScopedManagedCodeThread managedScope_;

    // Publication state, prepared before PONR and consumed by the commit under STW.
    EtsHandleScope publicationScope_;
    EtsHandle<EtsAbcFile> publicationWrapperHandle_ {};
    EtsHandle<EtsObjectArray> publicationArrayHandle_ {};
    uint32_t publicationSlot_ = 0;
    uint32_t publicationLength_ = 0;

    /*
     * Descriptors of the classes `CollectClasses` saw as the target's live set. The quiesce check
     * replays the same enumeration under stop-the-world and refuses the transaction when a
     * target-file class is published that this snapshot does not contain. Raw descriptors are
     * fine here: they belong to published classes, which stay alive for the rest of the process.
     */
    PandaUnorderedSet<const uint8_t *, utf::Mutf8Hash, utf::Mutf8Equal> collectedClasses_ {};
};

}  // namespace ark::ets::hotreload

#endif  // PANDA_PLUGINS_ETS_RUNTIME_HOTRELOAD_ETS_HOTRELOAD_H_
