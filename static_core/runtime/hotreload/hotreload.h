/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_HOTRELOAD_HOTRELOAD_H_
#define PANDA_RUNTIME_HOTRELOAD_HOTRELOAD_H_

#include <string_view>

#include "libarkbase/macros.h"
#include "libarkfile/file.h"
#include "runtime/hotreload/redirect.h"
#include "runtime/include/class.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime.h"
#include "runtime/mem/rendezvous.h"

namespace ark::hotreload {

enum ChangesFlags : uint32_t {
    F_NO_STRUCT_CHANGES = 0x0000U,
    F_INHERITANCE = 0x0001U,
    F_METHOD_SIGN = 0x0002U,
    F_METHOD_DELETED = 0x0004U,
    F_INTERFACES = 0x0008U,
    F_FIELDS_TYPE = 0x0010U,
    F_FIELDS_AMOUNT = 0x0020U,
    F_ACCESS_FLAGS = 0x0040U,
    F_METHOD_ADDED = 0x0080U,
    /*
     * The dispatch tables came out a different size even though every declared method matched.
     * Deliberately has no dedicated `Error`: `GetHotreloadErrorByFlag` falls through to
     * `UNSUPPORTED_CHANGES`, which is exactly what this is --- a change the swap cannot represent,
     * with no name in the source that the developer could act on.
     */
    F_DISPATCH_TABLES = 0x0100U
};

struct ClassContainment {
    const panda_file::File *pf = nullptr;
    panda_file::File::EntityId classId {};
    const std::string className_;
    Class *loadedClass = nullptr;
    Class *tmpClass = nullptr;
    // NOTE(hotreload-mvp): must stay initialized, it is only ever read-modify-written via |=
    uint32_t fChanges = ChangesFlags::F_NO_STRUCT_CHANGES;
};

enum class Error {
    NONE,
    INTERNAL,
    CLASS_ALREADY_LOADED,
    CLASS_NOT_FOUND,
    EXTERNAL_CLASS_READ,
    INVALID_INPUT_ARG,
    UNSUPPORTED_CHANGES,
    NO_DEBUGGER_ATTACHED,
    CLASS_UNMODIFIABLE,
    WRONG_CLASS_DESCRIPTOR,
    INVALID_CLASS_FORMAT,
    CIRCULAR_CLASS,
    METHOD_ADDED,
    METHOD_DELETED,
    METHOD_SIGN,
    CLASS_MODIFIERS,
    HIERARCHY_CHANGED,
    FIELD_CHANGED,
    /*
     * `Error` ordinals are part of the exported C++ API surface (`ProcessHotreload` returns it),
     * so new values may only be APPENDED. The names below are taken from the P1 design (§8);
     * only the subset that has a real producer today is declared, to avoid unreachable ordinals.
     */
    JIT_ENABLED,
    AOT_LOADED,
    TARGET_ABC_NOT_FOUND,
    TARGET_ABC_AMBIGUOUS,
    PATCH_OPEN_FAILED,
    CLASS_REMOVED,
    RELOAD_IN_PROGRESS,
    CLASS_ERRONEOUS,
    CLASS_INITIALIZING,
    /*
     * A class load raced the transaction: either a load was still in flight on the target context
     * when the world stopped, or a class of the target file was published between the collection
     * snapshot and the commit. The transaction was refused before the point of no return, so
     * nothing was modified; like CLASS_INITIALIZING, this is retryable and says so by existing.
     */
    CONCURRENT_LOADING,
    /*
     * Authorization gate of the ETS entry (`EtsAbcRuntimeLinkerHotReload`): the entry was
     * reached from managed code whose class does not belong to the boot context -- ANI by-name
     * invocation does not check access modifiers, so a protected entry is still reachable from
     * an arbitrary native bridge; only boot-context managed callers and pure native entries
     * (the framework's shape) may run the transaction. The core machinery stays unguarded for
     * other language frontends.
     */
    CALLER_NOT_BOOT,
    /*
     * The receiver's context is not backed by an `AbcRuntimeLinker`: there is no `abcFiles` array
     * to publish the new generation into, and committing without publication would serve later
     * lazy loads from the old file -- a mixed generation. Refused before anything is swapped.
     */
    NO_ABC_RUNTIME_LINKER,
};

PANDA_PUBLIC_API const char *GetErrorString(Error err);

/*
 * Machine-readable markers in the `hotreload` log component.
 *
 * A reload can succeed and still not apply everything the developer edited: classes the swap
 * cannot handle are dropped from the transaction rather than failing it (see
 * `ValidateClassesHotreloadPossibility`). The return value alone cannot say that --- it is `0` for
 * both outcomes --- and "reported success, part of the edit did not take effect" is the worst
 * contract a debugging tool can have.
 *
 * So the two facts a tool needs are emitted under fixed prefixes it can grep for, on the
 * `hotreload` component (`hilog | grep hotreload` on a device):
 *
 *   HOTRELOAD_SKIPPED_CLASS <descriptor> reason=<token>
 *   HOTRELOAD_RESULT generation=<n> target='<path>' reloaded=<n> skipped=<n>
 *
 * `HOTRELOAD_RESULT` is emitted once per SUCCESSFUL reload --- a failure is already unambiguous
 * from the returned `Error`. `skipped=0` therefore means "everything you changed is live"; anything
 * else means the preceding `HOTRELOAD_SKIPPED_CLASS` lines name what did not take effect, and the
 * developer needs a restart for those.
 *
 * The prefixes and the `reason` tokens are part of the tooling contract: **append, never rename**.
 * The descriptor is the internal form (`Lmodule/Class;`), which is unambiguous; demangling for
 * display is the tool's business.
 */
constexpr std::string_view LOG_TAG_SKIPPED_CLASS = "HOTRELOAD_SKIPPED_CLASS";
constexpr std::string_view LOG_TAG_RESULT = "HOTRELOAD_RESULT";

/**
 * @brief Make one generation's old->new pairs visible to `ark::hotreload::RedirectMethod` / `RedirectField`.
 *
 * Defined in `redirect.cpp`, declared here because its argument type belongs to the transaction and
 * `redirect.h` is deliberately kept free of runtime container headers --- it is included on the ANI
 * fast path. Call inside the commit, after the swap.
 */
void PublishRedirects(const PandaUnorderedMap<Method *, Method *> &methods,
                      const PandaUnorderedMap<Field *, Field *> &fields);

/**
 * @brief One-transaction-at-a-time gate.
 *
 * Two reloads running at once would interleave their Prepare phases and then their commits, and the
 * second commit would swap arrays the first one is still repairing. Nothing else prevents it: the
 * "only from the main thread" convention is a convention, and the STW of one transaction does not
 * stop the other one's NATIVE-state prologue.
 *
 * The gate refuses rather than waits. A blocking gate would have to be taken in NATIVE state to
 * avoid deadlocking against the holder's STW, and a caller queued behind a reload has nothing
 * useful to do anyway --- `RELOAD_IN_PROGRESS` is a better answer than an unbounded stall.
 */
class ReloadGate {
public:
    ReloadGate();
    ~ReloadGate();

    bool IsAcquired() const
    {
        return acquired_;
    }

    NO_COPY_SEMANTIC(ReloadGate);
    NO_MOVE_SEMANTIC(ReloadGate);

private:
    bool acquired_;
};

enum class Type { STRUCTURAL, NORMAL, INVALID };

class ArkHotreloadBase {
public:
    /*
     * There is no API for adding classes for hotreload
     * 'cause its signature is language-dependent
     * this API should be declared in superclass
     */
    PANDA_PUBLIC_API Error ProcessHotreload();

    PANDA_PUBLIC_API const panda_file::File *ReadAndOwnPandaFileFromFile(const char *location);
    PANDA_PUBLIC_API const panda_file::File *ReadAndOwnPandaFileFromMemory(const void *buffer, size_t size);

    /**
     * @brief Take ownership of a panda file the caller opened itself.
     *
     * Needed by language subclasses that open the patch through their own package reader (secure
     * memory, archive entry, ...) instead of one of the two readers above. Ownership follows the
     * same path: `AddLoadedPandaFilesToRuntime()` hands it over to the class linker on commit, the
     * destructor drops it if the transaction fails.
     *
     * @returns the borrowed pointer, or nullptr if @a pf was null.
     */
    PANDA_PUBLIC_API const panda_file::File *OwnPandaFile(std::unique_ptr<const panda_file::File> &&pf);

    NO_COPY_SEMANTIC(ArkHotreloadBase);
    NO_MOVE_SEMANTIC(ArkHotreloadBase);

protected:
    explicit ArkHotreloadBase(ManagedThread *mthread, panda_file::SourceLang lang);
    virtual ~ArkHotreloadBase();

    virtual Error LangSpecificValidateClasses() = 0;
    virtual void LangSpecificHotreloadPart() = 0;

    /**
     * @brief Language-specific race gate, re-run under stop-the-world right before the PONR.
     *
     * Stop-the-world stops mutators, but it is not class-loading quiescence: a loader that was
     * executing when the world stopped may have already published a class the transaction's
     * snapshot never saw, and one that is in flight (native state, or suspended past its own
     * last safepoint with the old file pointer already read) can publish one after the commit.
     * A language whose loading can race the transaction overrides this and refuses with a
     * retryable Error; the default is for languages without concurrent loading paths.
     */
    virtual Error LangSpecificQuiesceCheck() const
    {
        return Error::NONE;
    }

    /*
     * Refusal gates that must run before anything is loaded or modified.
     * NOTE(hotreload-mvp): P1 adds profiler / sampler / method-trace gates here.
     */
    Error AdmissionCheck() const;

    /**
     * @brief Re-run the refusal gates once the world is stopped, before anything is modified.
     *
     * The Prepare phase runs with mutators live, so everything it checked can have changed by the
     * time the commit starts: an AOT file can be loaded, and a class can enter `INITIALIZING` or
     * `ERRONEOUS`. Under STW those properties are stable, so this is the check that actually holds.
     *
     * It is the last point at which the transaction may still be rejected --- nothing has been
     * modified yet, so returning an error here keeps the "a failed hotreload changes nothing"
     * contract. Everything after it is past the point of no return.
     */
    Error QuiesceCheck() const;

    Error ValidateClassesHotreloadPossibility();

    /**
     * @brief Decide what to do with one candidate class.
     * @param[out] skipReason set to a stable reason token when the class cannot be swapped but must
     *                        not fail the transaction; left untouched when the class is reloadable.
     *                        The token is part of the tooling contract, see `LOG_TAG_SKIPPED_CLASS`.
     * @returns Error::NONE if the class is either reloadable or skippable.
     */
    Error ValidateClassForHotreload(const ClassContainment &hCls, const char **skipReason);
    Type RecognizeHotreloadType(ClassContainment *hCls);

    Type InheritanceChangesCheck(ClassContainment *hCls);
    Type FlagsChangesCheck(ClassContainment *hCls);
    Type FieldChangesCheck(ClassContainment *hCls);
    Type MethodChangesCheck(ClassContainment *hCls);
    Type DispatchTableChangesCheck(ClassContainment *hCls);

    void ReloadClassNormal(const ClassContainment *hCls);

    /** @brief Clear interpreter caches of all threads matching this transaction's language. */
    void ClearInterpreterCaches();

    /**
     * @brief Carry native bindings over from the methods that were just swapped out.
     *
     * A native method's implementation pointer is stored on the `Method` object itself, by
     * `Class_BindNativeMethods` / `RegisterNative`. Since the commit exchanges whole method arrays,
     * the replacements start out unbound, and the next call to one would raise
     * `UnresolvedMethodException` even though the patch did not touch the native side at all.
     *
     * @param obsoleteClass the temporary class, which by now carries the OLD methods.
     */
    void InheritNativeBindings(Class *obsoleteClass);

    /**
     * @brief Re-point every dispatch slot in the VM that still names a swapped-out method.
     *
     * Covers all three tables --- vtable, IMT and ITable --- of every live class, INCLUDING the
     * reloaded ones. A reloaded class is not "already actual": its tables were built while the
     * classes it inherits from still had their old methods, so every slot it inherits from another
     * class of the same abc points at that class's obsolete method.
     *
     * Obsolete classes are excluded: they must keep dispatching within their own generation.
     */
    void UpdateDispatchTables(ClassLinker *classLinker);

    /**
     * @brief Drop the resolution caches of every loaded panda file, the transaction's own included.
     *
     * The target's own cache is not enough. Any other file that resolved a method or a static field
     * of a reloaded class holds that raw pointer in its `PandaCache`, and would keep handing out
     * the obsolete one for the rest of the process. Reloads are rare and the caches refill lazily,
     * so clearing all of them is cheaper than tracking which entries are affected.
     *
     * The patch files in `pandaFiles_` are swept explicitly rather than through the class linker:
     * they are not registered with it yet, but Prepare has already written entries into their
     * caches, resolved against the generation this commit retires.
     */
    void ClearAllPandaCaches(ClassLinker *classLinker);

    void AddLoadedPandaFilesToRuntime(ClassLinker *classLinker);
    void AddObsoleteClassesToRuntime(ClassLinker *classLinker);

    /**
     * @brief Emit the `HOTRELOAD_RESULT` line a tool greps to tell a complete reload from a partial one.
     *
     * Emitted by `ProcessHotreload` itself, on success only, so that no language layer can forget
     * it. The counts come from the Prepare phase, which is why they are recorded there ---
     * `classes_` is empty by the time the commit is over.
     */
    void ReportResult() const;

    /*
     * Declared first so that it is taken before anything else exists and released after everything
     * else is gone --- in particular after `~ArkHotreloadBase` has freed the temporary classes.
     * Its constructor runs while the calling thread is still in NATIVE state, which is the state
     * the subclass must construct this object in anyway.
     */
    ReloadGate gate_;              // NOLINT(misc-non-private-member-variables-in-classes)
    panda_file::SourceLang lang_;  // NOLINT(misc-non-private-member-variables-in-classes)
    ManagedThread *thread_;        // NOLINT(misc-non-private-member-variables-in-classes)
    /*
     * Context the reloaded classes belong to. Set by the language subclass while filling `classes_`.
     * The new panda files must be handed over to the class linker under this very context, otherwise
     * they silently become boot files (design D2).
     */
    ClassLinkerContext *context_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)
    /*
     * What the caller named as the abc to replace, verbatim. Only used to key `HOTRELOAD_RESULT`,
     * so that a tool watching several modules can tell which one a line belongs to. The language
     * subclass sets it; an empty string is acceptable and simply yields an empty `target=''`.
     */
    PandaString targetName_;                 // NOLINT(misc-non-private-member-variables-in-classes)
    PandaVector<ClassContainment> classes_;  // NOLINT(misc-non-private-member-variables-in-classes)
    PandaVector<std::unique_ptr<const panda_file::File>>
        pandaFiles_;                                      // NOLINT(misc-non-private-member-variables-in-classes)
    PandaUnorderedMap<Method *, Method *> methodsTable_;  // NOLINT(misc-non-private-member-variables-in-classes)
    /*
     * old -> new `Field *`, the field counterpart of `methodsTable_`. Only the ANI / reflection
     * redirect uses it: nothing inside the runtime caches a `Field *` across a reload, but native
     * code is encouraged to cache `ani_static_field`, and a stale one addresses the OBSOLETE
     * class's static storage --- a silent read and write of the wrong memory.
     */
    PandaUnorderedMap<Field *, Field *> fieldsRedirect_;  // NOLINT(misc-non-private-member-variables-in-classes)
    PandaUnorderedSet<Class *> reloadedClasses_;          // NOLINT(misc-non-private-member-variables-in-classes)
    /*
     * What `HOTRELOAD_RESULT` reports. Recorded during Prepare because that is where the decision
     * is made, and because `classes_` is cleared by the commit.
     */
    size_t reloadedCount_ = 0;  // NOLINT(misc-non-private-member-variables-in-classes)
    size_t skippedCount_ = 0;   // NOLINT(misc-non-private-member-variables-in-classes)
};

}  // namespace ark::hotreload

#endif  // PANDA_RUNTIME_HOTRELOAD_HOTRELOAD_H_
