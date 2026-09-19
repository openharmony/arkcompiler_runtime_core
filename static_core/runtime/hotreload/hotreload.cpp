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

#include <algorithm>
#include <array>

#include "runtime/include/mem/allocator.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/monitor.h"
#include "compiler/aot/aot_manager.h"
#include "libarkfile/field_data_accessor.h"
#include "libarkfile/panda_cache.h"

#include "runtime/hotreload/hotreload.h"

namespace ark::hotreload {

const char *GetErrorString(Error err)
{
    static constexpr std::array<const char *, 30> NAMES = {
        "NONE",
        "INTERNAL",
        "CLASS_ALREADY_LOADED",
        "CLASS_NOT_FOUND",
        "EXTERNAL_CLASS_READ",
        "INVALID_INPUT_ARG",
        "UNSUPPORTED_CHANGES",
        "NO_DEBUGGER_ATTACHED",
        "CLASS_UNMODIFIABLE",
        "WRONG_CLASS_DESCRIPTOR",
        "INVALID_CLASS_FORMAT",
        "CIRCULAR_CLASS",
        "METHOD_ADDED",
        "METHOD_DELETED",
        "METHOD_SIGN",
        "CLASS_MODIFIERS",
        "HIERARCHY_CHANGED",
        "FIELD_CHANGED",
        "JIT_ENABLED",
        "AOT_LOADED",
        "TARGET_ABC_NOT_FOUND",
        "TARGET_ABC_AMBIGUOUS",
        "PATCH_OPEN_FAILED",
        "CLASS_REMOVED",
        "RELOAD_IN_PROGRESS",
        "CLASS_ERRONEOUS",
        "CLASS_INITIALIZING",
        "CONCURRENT_LOADING",
        "CALLER_NOT_BOOT",
        "NO_ABC_RUNTIME_LINKER",
    };
    auto idx = static_cast<size_t>(err);
    return idx < NAMES.size() ? NAMES[idx] : "UNKNOWN";
}

/*
 * The reload gate, see `ReloadGate` in the header. An atomic rather than a mutex on purpose: there
 * is nothing to wait for, and this way there is no lock to reason about against STW.
 */
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static std::atomic<bool> g_reloadInProgress {false};

// Counts committed transactions in this process. Only ever read for logging.
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static std::atomic<uint64_t> g_generation {0};

ReloadGate::ReloadGate()
    // Atomic with acq_rel order reason: the winner's transaction must not be reordered outside the
    // section this exchange opens
    : acquired_(!g_reloadInProgress.exchange(true, std::memory_order_acq_rel))
{
}

ReloadGate::~ReloadGate()
{
    if (acquired_) {
        // Atomic with release order reason: everything the transaction did must be visible to the
        // next holder of the gate
        g_reloadInProgress.store(false, std::memory_order_release);
    }
}

static Error GetHotreloadErrorByFlag(uint32_t flag)
{
    if (flag == ChangesFlags::F_NO_STRUCT_CHANGES) {
        return Error::NONE;
    }
    if ((flag & ChangesFlags::F_ACCESS_FLAGS) != 0U) {
        return Error::CLASS_MODIFIERS;
    }
    if ((flag & ChangesFlags::F_FIELDS_TYPE) != 0U || (flag & ChangesFlags::F_FIELDS_AMOUNT) != 0U) {
        return Error::FIELD_CHANGED;
    }
    if ((flag & ChangesFlags::F_INHERITANCE) != 0U || (flag & ChangesFlags::F_INTERFACES) != 0U) {
        return Error::HIERARCHY_CHANGED;
    }
    if ((flag & ChangesFlags::F_METHOD_SIGN) != 0U) {
        return Error::METHOD_SIGN;
    }
    if ((flag & ChangesFlags::F_METHOD_ADDED) != 0U) {
        return Error::METHOD_ADDED;
    }
    if ((flag & ChangesFlags::F_METHOD_DELETED) != 0U) {
        return Error::METHOD_DELETED;
    }

    return Error::UNSUPPORTED_CHANGES;
}

#ifndef NDEBUG
/*
 * Commit-phase invariant: this thread holds the mutator WRITE lock, i.e. the world is stopped.
 *
 * This replaces `ASSERT(!GetThreadManager()->IsRunningThreadExist())`, which cannot be used here:
 * `StackfulCoroutineManager::IsRunningThreadExist()` is `UNREACHABLE()` (its semantics for
 * coroutines were never defined, see `stackful_coroutine_manager.cpp`), so under the ETS coroutine
 * manager the assertion aborted every debug-build reload. The write lock is the property we
 * actually depend on, and it is directly observable.
 */
static bool IsWorldStopped()
{
    auto *rendezvous = PandaVM::GetCurrent()->GetRendezvous();
    return rendezvous != nullptr && rendezvous->GetMutatorLock()->GetState() == MutatorLock::WRLOCK;
}
#endif  // !NDEBUG

// ---------------------------------------------------------
// ------------------ Hotreload Class API ------------------
// ---------------------------------------------------------

const panda_file::File *ArkHotreloadBase::ReadAndOwnPandaFileFromFile(const char *location)
{
    ASSERT_MANAGED_CODE();
    ASSERT(thread_ == ManagedThread::GetCurrent());

    std::unique_ptr<const panda_file::File> pf = panda_file::OpenPandaFile(location);
    // A failed open must not be pushed: `AddPandaFile` asserts on and dereferences the pointer.
    if (pf == nullptr) {
        LOG(ERROR, HOTRELOAD) << "Cannot open panda file '" << location << "'";
        return nullptr;
    }
    const panda_file::File *ptr = pf.get();
    pandaFiles_.push_back(std::move(pf));
    return ptr;
}

const panda_file::File *ArkHotreloadBase::ReadAndOwnPandaFileFromMemory(const void *buffer, size_t buffSize)
{
    ASSERT_MANAGED_CODE();
    ASSERT(thread_ == ManagedThread::GetCurrent());

    if (buffer == nullptr || buffSize == 0) {
        LOG(ERROR, HOTRELOAD) << "Empty buffer passed for hotreload";
        return nullptr;
    }
    std::unique_ptr<const panda_file::File> pf = panda_file::OpenPandaFileFromMemory(buffer, buffSize);
    if (pf == nullptr) {
        LOG(ERROR, HOTRELOAD) << "Cannot open panda file from memory";
        return nullptr;
    }
    auto ptr = pf.get();
    pandaFiles_.push_back(std::move(pf));
    return ptr;
}

const panda_file::File *ArkHotreloadBase::OwnPandaFile(std::unique_ptr<const panda_file::File> &&pf)
{
    ASSERT_MANAGED_CODE();
    ASSERT(thread_ == ManagedThread::GetCurrent());

    if (pf == nullptr) {
        return nullptr;
    }
    const panda_file::File *ptr = pf.get();
    pandaFiles_.push_back(std::move(pf));
    return ptr;
}

Error ArkHotreloadBase::AdmissionCheck() const
{
    if (!gate_.IsAcquired()) {
        LOG(ERROR, HOTRELOAD) << "Another hotreload transaction is already in flight";
        return Error::RELOAD_IN_PROGRESS;
    }
    /*
     * JIT must be checked by the STARTUP OPTION, not by the dynamic `Runtime::IsJitEnabled()` flag:
     * the latter can be turned off temporarily (debug sessions do that), while compiled code and
     * CHA state are produced based on the startup option.
     */
    if (Runtime::GetOptions().IsCompilerEnableJit()) {
        LOG(ERROR, HOTRELOAD) << "JIT is enabled for this process, hotreload is not available";
        return Error::JIT_ENABLED;
    }
    if (Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->HasAotFiles()) {
        LOG(ERROR, HOTRELOAD) << "AOT files are loaded, hotreload is not available";
        return Error::AOT_LOADED;
    }
    return Error::NONE;
}

Error ArkHotreloadBase::QuiesceCheck() const
{
    ASSERT(IsWorldStopped());

    Error err = AdmissionCheck();
    if (err != Error::NONE) {
        return err;
    }

    /*
     * Initialization state is only meaningful under STW. Outside it, a class can enter or leave
     * `INITIALIZING` between the test and its use.
     *
     * A class another thread is initializing right now must not be swapped: `ClassInitializer`
     * looks the `<cctor>` up and then invokes it WITHOUT holding the class lock, so depending on
     * where that thread was suspended either the old or the new `<cctor>` would run, and there is
     * no way to tell which. Both errors below are RETRYABLE and say so by existing: the caller is
     * being told that the VM moved between Prepare and the commit, not that the patch is bad.
     * `ERRONEOUS` is normally handled by dropping the class in the Prepare phase; reaching it here
     * means the `<cctor>` failed inside that window.
     */
    for (const auto &hCls : classes_) {
        if (hCls.loadedClass->IsInitializing()) {
            LOG(ERROR, HOTRELOAD) << "Class " << hCls.className_ << " is being initialized by another thread";
            return Error::CLASS_INITIALIZING;
        }
        if (hCls.loadedClass->IsErroneous()) {
            LOG(ERROR, HOTRELOAD) << "Class " << hCls.className_ << " is in ERRONEOUS state, restart is required";
            return Error::CLASS_ERRONEOUS;
        }
    }

    /*
     * The language layer re-runs its own race gates here: stop-the-world stops mutators, but a
     * class that was published after the collection snapshot, or a load that is still in flight,
     * is invisible to everything above and would silently miss the swap.
     */
    return LangSpecificQuiesceCheck();
}

Error ArkHotreloadBase::ProcessHotreload()
{
    ASSERT_MANAGED_CODE();
    ASSERT(thread_ == ManagedThread::GetCurrent());

    Error err = AdmissionCheck();
    if (err != Error::NONE) {
        return err;
    }

    if (classes_.empty()) {
        LOG(ERROR, HOTRELOAD) << "Empty hotreload transaction";
        return Error::INVALID_INPUT_ARG;
    }

    err = ValidateClassesHotreloadPossibility();
    if (err != Error::NONE) {
        return err;
    }

    for (auto &hCls : classes_) {
        auto changesType = RecognizeHotreloadType(&hCls);
        if (changesType == Type::STRUCTURAL) {
            LOG(ERROR, HOTRELOAD) << "Class " << hCls.className_
                                  << " has structural changes. Structural changes is unsafe for hotreload.";
            return GetHotreloadErrorByFlag(hCls.fChanges);
        }
    }

    {
        ScopedSuspendAllThreadsRunning ssat(PandaVM::GetCurrent()->GetRendezvous());
        ASSERT_MANAGED_CODE();

        /*
         * Last exit. Everything below this line mutates shared state and cannot be undone, so the
         * gates get their authoritative run here, where the world is stopped and their answers are
         * stable. Returning from inside the scope resumes the world with nothing modified.
         */
        err = QuiesceCheck();
        if (err != Error::NONE) {
            return err;
        }

        /* ---------------------------- point of no return ---------------------------- */

        ClearInterpreterCaches();

        auto classLinker = Runtime::GetCurrent()->GetClassLinker();
        for (auto &hCls : classes_) {
            ReloadClassNormal(&hCls);
        }
        // After the last swap, so that no entry cached while swapping can survive it
        ClearAllPandaCaches(classLinker);

        /*
         * `UpdateTables` has left the temporary classes holding the OLD methods, which are exactly
         * the keys of `methodsTable_`, so repairing them would put the new ones straight back.
         * `UpdateDispatchTables` excludes them explicitly, reading them out of `classes_` --- which
         * `AddObsoleteClassesToRuntime` clears. Hence this order.
         */
        UpdateDispatchTables(classLinker);
        AddLoadedPandaFilesToRuntime(classLinker);
        AddObsoleteClassesToRuntime(classLinker);

        /*
         * Raw pointers that already left the runtime cannot be found and repaired, so they are
         * translated on the way back in instead. Published inside STW, so that no mutator can
         * resume into a window where the swap has happened but the translation is not available.
         */
        PublishRedirects(methodsTable_, fieldsRedirect_);

        LangSpecificHotreloadPart();
    }
    // Atomic with relaxed order reason: diagnostic counter, no data depends on it
    g_generation.fetch_add(1, std::memory_order_relaxed);
    ReportResult();
    return Error::NONE;
}

ArkHotreloadBase::ArkHotreloadBase(ManagedThread *mthread, panda_file::SourceLang lang) : lang_(lang), thread_(mthread)
{
    /*
     * Scoped object that switch to managed code is language dependent
     * So is should be constructed in superclass
     */
    ASSERT_NATIVE_CODE();
    ASSERT(thread_ != nullptr);
    ASSERT(thread_ == ManagedThread::GetCurrent());
    ASSERT(lang_ == thread_->GetThreadLang());
}

/* virtual */
ArkHotreloadBase::~ArkHotreloadBase()
{
    ASSERT_NATIVE_CODE();
    ASSERT(thread_ == ManagedThread::GetCurrent());

    auto classLinker = Runtime::GetCurrent()->GetClassLinker();
    for (auto &hCls : classes_) {
        if (hCls.tmpClass != nullptr) {
            classLinker->FreeClass(hCls.tmpClass);
        }
    }
}

// ---------------------------------------------------------
// ----------------------- Validators ----------------------
// ---------------------------------------------------------

Error ArkHotreloadBase::ValidateClassesHotreloadPossibility()
{
    ASSERT_MANAGED_CODE();
    ASSERT(thread_ == ManagedThread::GetCurrent());

    auto err = this->LangSpecificValidateClasses();
    if (err != Error::NONE) {
        return err;
    }

    /*
     * Classes the swap cannot handle are DROPPED from the transaction, not treated as a failure.
     *
     * Whole-file replacement collects every loaded class of the target abc, so a single interface
     * anywhere in the module used to make the module permanently un-reloadable --- and a real
     * ArkTS module has interfaces. Dropping such a class leaves it on the old panda file, which is
     * the same well-understood state a superseded generation is in.
     *
     * The price is that the reload then succeeds without applying everything the developer edited,
     * which the return value cannot express. Each dropped class is therefore named under
     * `LOG_TAG_SKIPPED_CLASS`, and `ReportResult` counts them, so a tool can say which edits did
     * not take effect instead of reporting an unqualified success.
     */
    PandaVector<ClassContainment> reloadable;
    reloadable.reserve(classes_.size());
    for (auto &hCls : classes_) {
        const char *skipReason = nullptr;
        Error returnErr = ValidateClassForHotreload(hCls, &skipReason);
        if (returnErr != Error::NONE) {
            return returnErr;
        }
        if (skipReason != nullptr) {
            LOG(WARNING, HOTRELOAD) << LOG_TAG_SKIPPED_CLASS << " " << hCls.className_ << " reason=" << skipReason;
            continue;
        }
        reloadable.push_back(hCls);
    }

    skippedCount_ = classes_.size() - reloadable.size();
    if (skippedCount_ != 0) {
        /*
         * The dropped classes' temporary copies must be freed, and `~ArkHotreloadBase` only frees
         * what is still in `classes_`. Swap the survivors in, then free what is left over.
         */
        PandaVector<ClassContainment> dropped;
        dropped.swap(classes_);
        classes_ = std::move(reloadable);

        auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
        PandaUnorderedSet<Class *> kept;
        for (const auto &hCls : classes_) {
            kept.insert(hCls.tmpClass);
        }
        for (const auto &hCls : dropped) {
            if (hCls.tmpClass != nullptr && kept.find(hCls.tmpClass) == kept.end()) {
                classLinker->FreeClass(hCls.tmpClass);
            }
        }
    }

    if (classes_.empty()) {
        LOG(ERROR, HOTRELOAD) << "No class of the target abc can be reloaded";
        return Error::CLASS_UNMODIFIABLE;
    }

    reloadedCount_ = classes_.size();
    return Error::NONE;
}

void ArkHotreloadBase::ReportResult() const
{
    /*
     * One line, always emitted on success, so that a tool never has to infer completeness from the
     * absence of warnings. `skipped=0` is the "everything you edited is live" answer.
     */
    // Atomic with relaxed order reason: diagnostic counter, no data depends on it
    LOG(INFO, HOTRELOAD) << LOG_TAG_RESULT << " generation=" << g_generation.load(std::memory_order_relaxed)
                         << " target='" << targetName_ << "'"
                         << " reloaded=" << reloadedCount_ << " skipped=" << skippedCount_;
    if (skippedCount_ != 0) {
        LOG(WARNING, HOTRELOAD) << skippedCount_ << " class(es) could not be swapped and stay on the previous abc; "
                                << "changes to them will NOT take effect until the process restarts";
    }
}

Error ArkHotreloadBase::ValidateClassForHotreload(const ClassContainment &hCls, const char **skipReason)
{
    ASSERT(skipReason != nullptr && *skipReason == nullptr);

    Class *clazz = hCls.tmpClass;
    Class *runtimeClass = hCls.loadedClass;
    if (clazz == nullptr) {
        LOG(ERROR, HOTRELOAD) << "Class " << hCls.className_ << " are failed to be initialized";
        return Error::INTERNAL;
    }

    /*
     * A not-yet-initialized class is reloaded as-is; it is NOT initialized here.
     *
     * The original code called `InitializeClass()` at this point. That is both unnecessary and
     * actively harmful:
     *   - unnecessary, because the swap only needs the class to be LINKED (vtable/IMT/field layout,
     *     all built at load time). An uninitialized class has no static state to preserve, and its
     *     `<cctor>` will simply run from the patch when it is initialized later --- which is the
     *     more correct outcome;
     *   - harmful, because initializing runs verification of the class methods, and under the
     *     default `--verification-mode=ahead-of-time` that RESOLVES the classes those methods
     *     reference, loading them from the OLD panda file. That happens in the Prepare phase, i.e.
     *     before the new file is published, so those classes stay stuck on the old code forever.
     *     It is also an observable side effect of a transaction that may still fail, which breaks
     *     the "a failed hotreload changes nothing" contract.
     *
     * The self-deadlock guard that used to sit next to it (`Monitor::HoldsLock` on the class
     * object) went away with the initialization it was protecting. It could not have stayed
     * anyway: `Monitor::HoldsLock` starts with `ASSERT(MTManagedThread::GetCurrent() != nullptr)`,
     * and under the ETS coroutine manager the current thread is a `Coroutine`, so that assertion
     * fails on every reload in a debug build --- the same shape of problem as
     * `IsRunningThreadExist()`.
     *
     * A class whose static initializer already failed is ERRONEOUS, and stays uninitializable for
     * the rest of the process no matter what its body is replaced with. It is dropped rather than
     * swapped --- and dropped rather than treated as a failure, because failing here would mean one
     * class that threw in its `<cctor>` once, possibly inside a swallowed `try`, disables hot
     * reload for the whole module until the process restarts.
     *
     * A class another thread is INITIALIZING is a different matter and is not decided here: outside
     * STW that answer is a guess that can change before it is used, and unlike ERRONEOUS it is
     * transient, so dropping the class would silently leave it stale for a condition that is about
     * to clear. `QuiesceCheck()` refuses the transaction instead, which tells the caller to retry.
     */
    if (!runtimeClass->IsInitialized()) {
        LOG(DEBUG, HOTRELOAD) << "Class " << hCls.className_ << " is not initialized yet, reloading it anyway";
    }
    if (runtimeClass->IsErroneous()) {
        *skipReason = "erroneous";
        return Error::NONE;
    }

    /*
     * Shapes the swap does not handle. None of them is a reason to fail the transaction: they say
     * "this particular class stays as it is", and the caller drops it. See the rationale in
     * `ValidateClassesHotreloadPossibility`.
     *
     * Interfaces are the one that matters in practice. Their default-method bodies are copied into
     * every implementor's method array as COPIED methods, which `MethodChangesCheck` does not walk
     * and `methodsTable_` therefore does not cover, so swapping an interface would leave those
     * copies pointing into the obsolete generation with nothing to repair them. Supporting it means
     * extending the redirect to copied methods first.
     */
    if (clazz->IsInterface()) {
        *skipReason = "interface";
    } else if (clazz->IsProxy()) {
        *skipReason = "proxy";
    } else if (clazz->IsArrayClass()) {
        *skipReason = "array-class";
    } else if (clazz->IsStringClass()) {
        *skipReason = "string-class";
    } else if (clazz->IsPrimitive()) {
        *skipReason = "primitive";
    }

    return Error::NONE;
}

Type ArkHotreloadBase::RecognizeHotreloadType(ClassContainment *hCls)
{
    /*
     * Checking for the changes:
     *  - Inheritance changed
     *  - Access flags changed
     *  - Field added/deleted
     *  - Field type changed
     *  - Method added/deleted
     *  - Method signature changed
     *
     * In case there are any of changes above Type is Structural
     * Otherwise it's normal changes
     */
    if (InheritanceChangesCheck(hCls) == Type::STRUCTURAL) {
        return Type::STRUCTURAL;
    }
    if (FlagsChangesCheck(hCls) == Type::STRUCTURAL) {
        return Type::STRUCTURAL;
    }
    if (FieldChangesCheck(hCls) == Type::STRUCTURAL) {
        return Type::STRUCTURAL;
    }
    if (MethodChangesCheck(hCls) == Type::STRUCTURAL) {
        return Type::STRUCTURAL;
    }
    if (DispatchTableChangesCheck(hCls) == Type::STRUCTURAL) {
        return Type::STRUCTURAL;
    }

    return Type::NORMAL;
}

/*
 * The last check, and the only one that inspects a DERIVED property rather than a declared one.
 *
 * `UpdateTables` exchanges the vtable and the IMT with `std::swap_ranges`, which walks the length
 * of the runtime class's table and writes the same number of slots into the temporary class's. Both
 * tables live INSIDE the `Class` allocation, immediately ahead of the static field storage, so a
 * length disagreement is not a wrong answer --- it is a write past the end of one of the two class
 * objects, into whatever the allocator put there.
 *
 * Nothing above establishes equal lengths. The checks constrain what the source DECLARES: the same
 * method count, and for each method the same name, prototype and access flags. Table lengths are
 * what the vtable builder DERIVES from those declarations, and the builder is order-sensitive ---
 * `VTableBuilderBase::AddClassMethods` walks the class's methods in reverse and lets each one claim
 * a base slot greedily, so under ETS variance rules two declarations that differ only in their
 * order in the metadata can leave a different number of slots unclaimed, hence appended. The check
 * above cannot see that, because it matches methods as an unordered set --- which is right, since
 * reordering methods is otherwise none of its business.
 *
 * Rather than reason about which frontend emits which order, ask the tables directly. They are
 * already built at this point: the temporary class is fully linked, that is what makes the swap
 * possible at all.
 *
 * This refuses the whole transaction instead of dropping the class. A length that the declared
 * methods do not explain means the model this validation is built on does not hold for this class,
 * and the honest response to that is to stop, not to continue with one class quietly left behind.
 * The `ASSERT`s in `UpdateTables` stay as documentation of the invariant this establishes; they are
 * compiled out in Release, which is exactly why they could not be the guard.
 *
 * NO TEST DRIVES THIS, and that is a finding rather than an omission --- see `04-testing.md` §5.
 * Neither available frontend can express the input: es2panda refuses the source outright
 * (`ESE602430`, both declarations override the same base method), and `ark_asm` NORMALISES method
 * order, so two hand-written `.pa` files that differ only in the order of two same-name overloads
 * assemble to byte-identical method arrays --- verified by disassembling both. That is what the
 * earlier "identical vtable either way" measurement was actually measuring; it was not evidence
 * about this builder.
 *
 * The builder itself IS order-sensitive in length, so this check is not dead code. `ProcessClassMethod`
 * lets one class method claim EVERY compatible base slot, and a method processed later whose only
 * compatible slot is already claimed is appended instead. With a base declaring two same-name
 * methods, and a derived one whose parameter is wide enough to override both (ETS parameters are
 * contravariant), the two declaration orders leave a different number of slots appended. Reaching
 * it needs a producer that emits method order independently of the method set, which is a property
 * of today's toolchain rather than of the format.
 */
Type ArkHotreloadBase::DispatchTableChangesCheck(ClassContainment *hCls)
{
    Class *tmpClass = hCls->tmpClass;
    Class *runtimeClass = hCls->loadedClass;

    if (tmpClass->GetVTableSize() != runtimeClass->GetVTableSize() ||
        tmpClass->GetIMTSize() != runtimeClass->GetIMTSize()) {
        LOG(ERROR, HOTRELOAD) << "Class " << hCls->className_ << " dispatch table size changed: vtable "
                              << runtimeClass->GetVTableSize() << " -> " << tmpClass->GetVTableSize() << ", imt "
                              << runtimeClass->GetIMTSize() << " -> " << tmpClass->GetIMTSize()
                              << ". Every declared method matched, so this is a declaration ORDER change the "
                              << "vtable builder resolved differently";
        hCls->fChanges |= ChangesFlags::F_DISPATCH_TABLES;
        return Type::STRUCTURAL;
    }

    return Type::NORMAL;
}

Type ArkHotreloadBase::InheritanceChangesCheck(ClassContainment *hCls)
{
    Class *tmpClass = hCls->tmpClass;
    Class *runtimeClass = hCls->loadedClass;
    if (tmpClass->GetBase() != runtimeClass->GetBase()) {
        hCls->fChanges |= ChangesFlags::F_INHERITANCE;
        return Type::STRUCTURAL;
    }

    auto newIfaces = tmpClass->GetInterfaces();
    auto oldIfaces = runtimeClass->GetInterfaces();
    if (newIfaces.size() != oldIfaces.size()) {
        hCls->fChanges |= ChangesFlags::F_INTERFACES;
        return Type::STRUCTURAL;
    }

    PandaUnorderedSet<Class *> ifaces;
    for (auto iface : oldIfaces) {
        ifaces.insert(iface);
    }
    for (auto iface : newIfaces) {
        if (ifaces.find(iface) == ifaces.end()) {
            hCls->fChanges |= ChangesFlags::F_INTERFACES;
            return Type::STRUCTURAL;
        }
        ifaces.erase(iface);
    }
    if (!ifaces.empty()) {
        hCls->fChanges |= ChangesFlags::F_INTERFACES;
        return Type::STRUCTURAL;
    }

    return Type::NORMAL;
}

Type ArkHotreloadBase::FlagsChangesCheck(ClassContainment *hCls)
{
    Class *tmpClass = hCls->tmpClass;
    Class *runtimeClass = hCls->loadedClass;

    // NOTE(m.strizhak) research that maybe there are flags that can be changed keeping normal type
    if (tmpClass->GetRuntimeFlags() != runtimeClass->GetRuntimeFlags()) {
        hCls->fChanges |= ChangesFlags::F_ACCESS_FLAGS;
        return Type::STRUCTURAL;
    }

    if (tmpClass->GetAccessFlags() != runtimeClass->GetAccessFlags()) {
        hCls->fChanges |= ChangesFlags::F_ACCESS_FLAGS;
        return Type::STRUCTURAL;
    }

    return Type::NORMAL;
}

/*
 * Do the two fields describe the SAME STORAGE, byte for byte and type for type?
 *
 * Not "are they the same declaration" --- the swap installs the patch's `Field` array on the live
 * class and the offset lives on the `Field`, while every existing instance keeps the bytes it
 * already has. So what has to hold is that reading through the new field lands on exactly what the
 * old field described.
 *
 * Two things are compared, and both are needed:
 *
 *   - The OFFSET, because the layout algorithm assigns offsets in declaration order within each
 *     width bucket. Swapping two same-width fields in the source is a legal edit that changes
 *     nothing about names, types, count or flags, and exchanges the two offsets. Without this,
 *     every live object silently answers with the other field's value.
 *
 *   - For references, the DESCRIPTOR, because `GetTypeId()` only distinguishes the primitive
 *     widths from the single `REFERENCE` category. Every reference field compares equal to every
 *     other one, so retyping a field from `A` to `B` passes as "same type" while the bytes in the
 *     slot are still an `A`, which the patch's bytecode then uses as a `B`.
 *
 * Compared as strings out of the field metadata, deliberately: `ResolveTypeClass` would LOAD the
 * referenced class, and this runs in the Prepare phase, where loading resolves out of the OLD panda
 * file and pins that class to the superseded generation. That is bug D2 in `02-implementation.md`,
 * removed once already, and it must not come back through the field check.
 */
static bool SameFieldStorage(const Field &oldField, const Field &newField)
{
    if (oldField.GetOffset() != newField.GetOffset() || oldField.GetTypeId() != newField.GetTypeId()) {
        return false;
    }
    if (oldField.GetTypeId() != panda_file::Type::TypeId::REFERENCE) {
        return true;
    }

    const auto *oldPf = oldField.GetPandaFile();
    const auto *newPf = newField.GetPandaFile();
    auto oldDescr = oldPf->GetStringData(panda_file::FieldDataAccessor::GetTypeId(*oldPf, oldField.GetFileId()));
    auto newDescr = newPf->GetStringData(panda_file::FieldDataAccessor::GetTypeId(*newPf, newField.GetFileId()));
    return oldDescr == newDescr;
}

Type ArkHotreloadBase::FieldChangesCheck(ClassContainment *hCls)
{
    Class *tmpClass = hCls->tmpClass;
    Class *runtimeClass = hCls->loadedClass;

    auto oldFields = runtimeClass->GetFields();
    auto newFields = tmpClass->GetFields();
    if (newFields.size() != oldFields.size()) {
        hCls->fChanges |= ChangesFlags::F_FIELDS_AMOUNT;
        return Type::STRUCTURAL;
    }

    PandaUnorderedMap<PandaString, Field *> fieldsTable;
    for (auto &oldField : oldFields) {
        PandaString fieldName(utf::Mutf8AsCString(oldField.GetName().data));
        fieldsTable.insert({fieldName, &oldField});
    }

    for (auto &newField : newFields) {
        PandaString fieldName(utf::Mutf8AsCString(newField.GetName().data));
        auto oldIt = fieldsTable.find(fieldName);
        if (oldIt == fieldsTable.end()) {
            hCls->fChanges |= ChangesFlags::F_FIELDS_AMOUNT;
            return Type::STRUCTURAL;
        }

        if (oldIt->second->GetAccessFlags() != newField.GetAccessFlags() ||
            !SameFieldStorage(*oldIt->second, newField)) {
            hCls->fChanges |= ChangesFlags::F_FIELDS_TYPE;
            return Type::STRUCTURAL;
        }

        // Same pair, by pointer, for the ANI / reflection redirect. Recorded here because this is
        // the only place the two `Field` objects are known to be each other's counterpart.
        fieldsRedirect_[oldIt->second] = &newField;
        fieldsTable.erase(fieldName);
    }

    if (!fieldsTable.empty()) {
        hCls->fChanges |= ChangesFlags::F_FIELDS_AMOUNT;
        return Type::STRUCTURAL;
    }

    return Type::NORMAL;
}

static inline uint32_t GetFileAccessFlags(const Method &method)
{
    return method.GetAccessFlags() & ACC_FILE_MASK;
}

Type ArkHotreloadBase::MethodChangesCheck(ClassContainment *hCls)
{
    Class *tmpClass = hCls->tmpClass;
    Class *runtimeClass = hCls->loadedClass;

    auto oldMethods = runtimeClass->GetMethods();
    auto newMethods = tmpClass->GetMethods();
    if (newMethods.size() > oldMethods.size() ||
        tmpClass->GetNumVirtualMethods() > runtimeClass->GetNumVirtualMethods()) {
        hCls->fChanges |= ChangesFlags::F_METHOD_ADDED;
        return Type::STRUCTURAL;
    }

    if (newMethods.size() < oldMethods.size() ||
        tmpClass->GetNumVirtualMethods() < runtimeClass->GetNumVirtualMethods()) {
        hCls->fChanges |= ChangesFlags::F_METHOD_DELETED;
        return Type::STRUCTURAL;
    }

    for (auto &newMethod : newMethods) {
        bool isNameFound = false;
        bool isExactFound = false;
        for (auto &oldMethod : oldMethods) {
            PandaString oldName = utf::Mutf8AsCString(oldMethod.GetName().data);
            PandaString newName = utf::Mutf8AsCString(newMethod.GetName().data);
            if (oldName != newName) {
                continue;
            }
            isNameFound = true;
            if (oldMethod.GetProto() == newMethod.GetProto() &&
                GetFileAccessFlags(oldMethod) == GetFileAccessFlags(newMethod)) {
                methodsTable_[&oldMethod] = &newMethod;
                isExactFound = true;
                break;
            }
        }

        if (isNameFound) {
            if (isExactFound) {
                continue;
            }
            hCls->fChanges |= ChangesFlags::F_METHOD_SIGN;
            return Type::STRUCTURAL;
        }
        hCls->fChanges |= ChangesFlags::F_METHOD_ADDED;
        return Type::STRUCTURAL;
    }
    return Type::NORMAL;
}

// ---------------------------------------------------------
// ----------------------- Reloaders -----------------------
// ---------------------------------------------------------

// This method is used under assert. So in release build it's unused
[[maybe_unused]] static bool VerifyClassConsistency(const Class *cls)
{
    ASSERT(cls);

    const auto pf = cls->GetPandaFile();
    auto methods = cls->GetMethods();
    auto fields = cls->GetFields();
    const uint8_t *descriptor = cls->GetDescriptor();

    if (cls->GetFileId().GetOffset() != pf->GetClassId(descriptor).GetOffset()) {
        return false;
    }

    for (const auto &method : methods) {
        panda_file::MethodDataAccessor mda(*pf, method.GetFileId());
        panda_file::ProtoDataAccessor pda(*pf, mda.GetProtoId());
        if (mda.GetName() != method.GetName()) {
            return false;
        }
        if (method.GetClass() != cls) {
            return false;
        }
    }

    for (const auto &field : fields) {
        panda_file::FieldDataAccessor fda(*pf, field.GetFileId());
        if (field.GetClass() != cls) {
            return false;
        }
    }

    return true;
}

static void UpdateClassPtrInMethods(Span<Method> methods, Class *cls)
{
    for (auto &method : methods) {
        method.SetClass(cls);
    }
}

static void UpdateClassPtrInFields(Span<Field> fields, Class *cls)
{
    for (auto &field : fields) {
        field.SetClass(cls);
    }
}

static void UpdatePandaFileInClass(Class *runtimeClass, const panda_file::File *pf)
{
    const uint8_t *descriptor = runtimeClass->GetDescriptor();
    panda_file::File::EntityId classId = pf->GetClassId(descriptor);
    runtimeClass->SetPandaFile(pf);
    runtimeClass->SetFileId(classId);
    runtimeClass->SetClassIndex(pf->GetClassIndex(classId));
    runtimeClass->SetMethodIndex(pf->GetMethodIndex(classId));
    runtimeClass->SetFieldIndex(pf->GetFieldIndex(classId));
}

/*
 * Obsolete methods should be saved by temporary class 'cause it might be continue executing
 * Updating class pointers in methods to keep it consistent with class and panda file
 */
static void UpdateMethods(Class *runtimeClass, Class *tmpClass)
{
    auto newMethods = tmpClass->GetMethodsWithCopied();
    auto oldMethods = runtimeClass->GetMethodsWithCopied();
    uint32_t numVmethods = tmpClass->GetNumVirtualMethods();
    uint32_t numCmethods = tmpClass->GetNumCopiedMethods();
    uint32_t numSmethods = newMethods.size() - numVmethods - numCmethods;
    UpdateClassPtrInMethods(newMethods, runtimeClass);
    UpdateClassPtrInMethods(oldMethods, tmpClass);
    runtimeClass->SetMethods(newMethods, numVmethods, numSmethods);
    tmpClass->SetMethods(oldMethods, numVmethods, numSmethods);
}

/*
 * Obsolete fields should be saved by temporary class 'cause it might be used by obselete methods
 * Updating class pointers in fields to keep it consistent with class and panda file
 */
static void UpdateFields(Class *runtimeClass, Class *tmpClass)
{
    auto newFields = tmpClass->GetFields();
    auto oldFields = runtimeClass->GetFields();
    uint32_t numSfields = tmpClass->GetNumStaticFields();
    UpdateClassPtrInFields(newFields, runtimeClass);
    UpdateClassPtrInFields(oldFields, tmpClass);
    runtimeClass->SetFields(newFields, numSfields);
    tmpClass->SetFields(oldFields, numSfields);
}

static void UpdateIfaces(Class *runtimeClass, Class *tmpClass)
{
    auto newIfaces = tmpClass->GetInterfaces();
    auto oldIfaces = runtimeClass->GetInterfaces();
    runtimeClass->SetInterfaces(newIfaces);
    tmpClass->SetInterfaces(oldIfaces);
}

/*
 * All three dispatch tables are EXCHANGED, the same way the method and field arrays are.
 *
 * The runtime class needs the temporary class's tables, which were built from the patch. The
 * temporary class needs the runtime class's tables, which name the methods it has just taken
 * ownership of --- it used to keep the new ones, so an obsolete class dispatched into the live
 * generation while its own methods claimed to belong to it. Nothing depended on that, because
 * objects keep pointing at the runtime class and virtual dispatch goes through them, but a class
 * whose method array and vtable disagree is a trap for the next reader.
 *
 * The exchange is also what lets `UpdateDispatchTables` stop making an exception for the reloaded
 * classes: after it, the temporary class holds old methods and must not be visited, which is
 * exactly what "obsolete" already means.
 */
static void UpdateTables(Class *runtimeClass, Class *tmpClass)
{
    ASSERT(tmpClass->GetIMTSize() == runtimeClass->GetIMTSize());
    ASSERT(tmpClass->GetVTableSize() == runtimeClass->GetVTableSize());
    ITable oldItable = runtimeClass->GetITable();
    Span<Method *> oldVtable = runtimeClass->GetVTable();
    Span<Method *> newVtable = tmpClass->GetVTable();
    Span<Method *> oldImt = runtimeClass->GetIMT();
    Span<Method *> newImt = tmpClass->GetIMT();
    runtimeClass->SetITable(tmpClass->GetITable());
    tmpClass->SetITable(oldItable);
    if (!oldVtable.empty()) {
        std::swap_ranges(oldVtable.begin(), oldVtable.end(), newVtable.begin());
    }
    if (!oldImt.empty()) {
        std::swap_ranges(oldImt.begin(), oldImt.end(), newImt.begin());
    }
}

void ArkHotreloadBase::ClearInterpreterCaches()
{
    ASSERT(thread_->GetVM()->GetThreadManager() != nullptr);
    PandaVM::GetCurrent()->GetThreadManager()->EnumerateThreads([this](ManagedThread *thread) {
        (void)this;  // [[maybe_unused]] in lambda capture list is not possible
        ASSERT(thread->GetThreadLang() == lang_);
        thread->GetInterpreterCache()->Clear();
        return true;
    });
}

void ArkHotreloadBase::ReloadClassNormal(const ClassContainment *hCls)
{
    ASSERT(IsWorldStopped());

    /*
     * Update runtime class header:
     *   - Panda file and its id
     *   - methods
     *   - fields
     *   - ifaces
     *   - tables
     *
     * Then adding obsolete classes to special area in class linker
     * to be able to continue executing obsolete methods after hotreloading
     *
     * No class monitor is taken here on purpose (design D10): the world is already stopped, and
     * waiting for a monitor that a suspended mutator holds would deadlock. STW is the mutual
     * exclusion; contending threads cannot observe the intermediate state.
     */
    Class *tmpClass = hCls->tmpClass;
    Class *runtimeClass = hCls->loadedClass;

    const panda_file::File *newPf = hCls->pf;
    const panda_file::File *oldPf = runtimeClass->GetPandaFile();

    // The caches of ALL files, this one included, are dropped by `ClearAllPandaCaches` once every
    // class has been swapped --- clearing only the target's is not enough, see its comment

    reloadedClasses_.insert(runtimeClass);
    UpdatePandaFileInClass(tmpClass, oldPf);
    UpdatePandaFileInClass(runtimeClass, newPf);
    UpdateMethods(runtimeClass, tmpClass);
    InheritNativeBindings(tmpClass);
    UpdateFields(runtimeClass, tmpClass);
    UpdateIfaces(runtimeClass, tmpClass);
    UpdateTables(runtimeClass, tmpClass);

    ASSERT(VerifyClassConsistency(runtimeClass));
    ASSERT(VerifyClassConsistency(tmpClass));
}

void ArkHotreloadBase::InheritNativeBindings(Class *obsoleteClass)
{
    ASSERT(IsWorldStopped());

    for (auto &oldMethod : obsoleteClass->GetMethods()) {
        /*
         * `pointer_` is a union: it only holds the native pointer for native and proxy methods, and
         * profiling data for everything else. Reading it for a non-native method would copy garbage.
         */
        if (!oldMethod.IsNative()) {
            continue;
        }
        void *nativePointer = oldMethod.GetNativePointer();
        if (nativePointer == nullptr) {
            continue;
        }
        // Exact name + prototype + access flags match, established by `MethodChangesCheck`
        auto it = methodsTable_.find(&oldMethod);
        if (it == methodsTable_.end()) {
            continue;
        }
        it->second->SetNativePointer(nativePointer);
    }
}

void ArkHotreloadBase::ClearAllPandaCaches(ClassLinker *classLinker)
{
    ASSERT(IsWorldStopped());

    auto clearCache = [](const panda_file::File &pf) {
        auto *cache = pf.GetPandaCache();
        if (cache->HasCachedEntries()) {
            cache->Clear();
        }
    };

    /*
     * The transaction's own files first, because the enumeration below cannot reach them: they are
     * handed to the class linker only later, in `AddLoadedPandaFilesToRuntime`. Their caches are
     * not empty, though. Loading the temporary classes during Prepare resolves entities of the
     * patch file against the generation this commit is retiring --- a function-reference class
     * resolves its `FunctionReference` annotation through `ClassLinker::GetMethod(pf, id)`, which
     * writes the cache --- so a method entry here names a `Method` that the swap has just moved
     * onto an obsolete class. Only entries written before the swap are poisonous; anything cached
     * afterwards resolves by name against the live classes, so nothing has to be reordered around
     * `AddLoadedPandaFilesToRuntime`.
     */
    for (const auto &pf : pandaFiles_) {
        clearCache(*pf);
    }

    /*
     * The retiring generation's file can sit outside the class linker's registry as well: an
     * embedder that opened the abc itself and loaded its classes with `LoadClass` never calls
     * `AddPandaFile`, so the enumeration below cannot reach that file, and its cache would keep
     * naming the `Method`s the swap has just moved onto obsolete classes. After the swap the
     * temporary class is the one that names the retiring file --- `ReloadClassNormal` points it
     * back at the old abc --- so reach the file through it. Every entry that reaches the commit
     * went through `ReloadClassNormal`; the dropped classes were removed from `classes_` during
     * validation. `clearCache` is idempotent, so the file shared by several reloaded classes is
     * simply skipped after its first visit.
     */
    for (const ClassContainment &hCls : classes_) {
        if (hCls.tmpClass != nullptr) {
            clearCache(*hCls.tmpClass->GetPandaFile());
        }
    }

    /*
     * Every loaded file has to be considered, and that includes every generation this process has
     * already retired --- their files stay registered for good. `Clear()` re-materialises three
     * full-size tables, so doing it unconditionally would make each reload cost time proportional
     * to the number of reloads before it. A retired generation is cleared exactly once and never
     * writes to its cache again, so skipping the ones that have nothing cached keeps the sweep
     * proportional to the number of files that are actually in use.
     */
    classLinker->EnumeratePandaFiles([&clearCache](const panda_file::File &pf) {
        clearCache(pf);
        return true;
    });
}

void ArkHotreloadBase::UpdateDispatchTables(ClassLinker *classLinker)
{
    ASSERT(IsWorldStopped());

    auto *ext = classLinker->GetExtension(lang_);

    /*
     * Two kinds of class must keep their tables:
     *  - obsolete classes of PREVIOUS generations, which have to go on dispatching within their own
     *    generation;
     *  - this transaction's temporary classes, which `UpdateTables` has just handed the OLD methods
     *    and the OLD tables. They are still in `createdClasses_` at this point --- they only leave
     *    it in `AddObsoleteClassesToRuntime`, which runs after this --- so the enumeration below
     *    does reach them, and patching them would put the new methods straight back.
     */
    PandaUnorderedSet<Class *> untouchable;
    ext->EnumerateObsoleteClasses([&untouchable](Class *cls) {
        untouchable.insert(cls);
        return true;
    });
    for (const auto &hCls : classes_) {
        if (hCls.tmpClass != nullptr) {
            untouchable.insert(hCls.tmpClass);
        }
    }

    auto repair = [this](Method **slot) {
        if (*slot == nullptr) {
            return;
        }
        auto it = methodsTable_.find(*slot);
        if (it != methodsTable_.end()) {
            *slot = it->second;
        }
    };

    /*
     * Reloaded classes are NOT skipped, unlike in the original implementation.
     *
     * Their tables came from the temporary class, which the class linker built during Prepare ---
     * while every class it inherits from still had its old methods. So a class that inherits from
     * another class of the same abc got its inherited vtable and ITable slots filled with that
     * class's methods as they were BEFORE the swap, and calling an inherited method through it ran
     * obsolete code, silently and for good. Their own slots hold the new methods, which are not
     * keys of `methodsTable_`, so running the repair over them is a no-op.
     */
    ext->EnumerateClasses([&untouchable, &repair](Class *cls) {
        if (untouchable.find(cls) != untouchable.end()) {
            return true;
        }
        for (auto &methodPtr : cls->GetVTable()) {
            repair(&methodPtr);
        }
        // Interface method table: the fast path of interface dispatch, populated independently of
        // the vtable, so it goes stale independently too
        for (auto &methodPtr : cls->GetIMT()) {
            repair(&methodPtr);
        }
        // Full interface table: one method array per implemented interface, each one owned by this
        // class (`ITable::Entry::Copy`), so no entry is shared with another class
        auto itable = cls->GetITable();
        for (size_t i = 0; i < itable.Size(); ++i) {
            for (auto &methodPtr : itable[i].GetMethods()) {
                repair(&methodPtr);
            }
        }
        return true;
    });
}

void ArkHotreloadBase::AddLoadedPandaFilesToRuntime(ClassLinker *classLinker)
{
    ASSERT(IsWorldStopped());

    /*
     * `context_` must be the context the reloaded classes belong to (design D2). Handing the file
     * over with a null context registers it as a BOOT panda file, after which the boot context can
     * resolve application classes out of it.
     */
    for (auto &ptrPf : pandaFiles_) {
        classLinker->AddPandaFile(std::move(ptrPf), context_);
    }
    pandaFiles_.clear();
}

void ArkHotreloadBase::AddObsoleteClassesToRuntime(ClassLinker *classLinker)
{
    ASSERT(IsWorldStopped());

    /*
     * Sending all classes in one vector to avoid holding lock for every single class
     * It should be faster because all threads are still stopped
     */
    PandaVector<Class *> obsoleteClasses;
    obsoleteClasses.reserve(classes_.size());
    for (const auto &hCls : classes_) {
        if (hCls.tmpClass != nullptr) {
            ASSERT(hCls.tmpClass->GetSourceLang() == lang_);
            /*
             * `LoadClass(..., addToRuntime = false)` leaves the class in `createdClasses_` because it
             * never reaches the publishing branch. Without this it would live in BOTH `createdClasses_`
             * and `obsoleteClasses_` (design D3) and be freed twice on extension teardown.
             */
            classLinker->RemoveCreatedClassInExtension(hCls.tmpClass);
            /*
             * The obsolete class stays reachable: the methods that were swapped out now belong to
             * it, and anything still holding one of them (a suspended frame, a coroutine that has
             * not started yet) can reach its class and try to initialize it. Class initialization
             * takes `ClassLock`, which looks the class up in the context's mutex table with `.at()`
             * --- and an obsolete class was never published there, because it is deliberately not
             * resolvable by descriptor. Without this registration that lookup throws
             * `std::out_of_range` and aborts the process.
             */
            ASSERT(context_ != nullptr);
            context_->RegisterClassMutex(hCls.tmpClass);
            obsoleteClasses.push_back(hCls.tmpClass);
        }
    }
    classLinker->GetExtension(lang_)->AddObsoleteClass(obsoleteClasses);
    /*
     * Ownership of the obsolete classes moved to the extension: clear the transaction so that the
     * destructor does not free them (`~ArkHotreloadBase` frees every non-null `tmpClass`).
     */
    classes_.clear();
}

}  // namespace ark::hotreload
