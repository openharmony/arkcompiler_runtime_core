/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_RUNTIME_H_
#define PANDA_RUNTIME_RUNTIME_H_

#include <atomic>
#include <memory>
#include <string>
#include <csignal>
#include <vector>

#include "libpandabase/mem/arena_allocator.h"
#include "libpandabase/os/mutex.h"
#include "libpandabase/utils/expected.h"
#include "libpandafile/file_items.h"
#include "libpandafile/literal_data_accessor.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_smart_pointers.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/method.h"
#include "runtime/include/relayout_profiler.h"
#include "runtime/include/runtime_options.h"
#include "runtime/include/gc_task.h"
#include "runtime/include/tooling/debug_interface.h"
#include "runtime/mem/allocator_adapter.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/gc_trigger.h"
#include "runtime/mem/memory_manager.h"
#include "runtime/monitor_pool.h"
#include "runtime/string_table.h"
#include "runtime/thread_manager.h"
#include "verification/verification_options.h"
#include "libpandabase/os/native_stack.h"
#include "libpandabase/os/library_loader.h"
#include "runtime/include/loadable_agent.h"

namespace panda {

class DProfiler;
class CompilerInterface;
class ClassHierarchyAnalysis;
class RuntimeController;
class PandaVM;
class RuntimeNotificationManager;
class Trace;

namespace tooling {
class Debugger;
}  // namespace tooling

using UnwindStackFn = os::unix::native_stack::FUNC_UNWINDSTACK;

class Runtime {
public:
    using ExitHook = void (*)(int32_t status);
    using AbortHook = void (*)();

    enum class Error {
        PANDA_FILE_LOAD_ERROR,
        INVALID_ENTRY_POINT,
        CLASS_NOT_FOUND,
        CLASS_NOT_INITIALIZED,
        METHOD_NOT_FOUND,
        CLASS_LINKER_EXTENSION_NOT_FOUND
    };

    class DebugSession final {
    public:
        explicit DebugSession(Runtime &runtime);
        ~DebugSession();

        tooling::DebugInterface &GetDebugger();

    private:
        Runtime &runtime_;
        bool is_jit_enabled_;
        os::memory::LockHolder<os::memory::Mutex> lock_;
        PandaUniquePtr<tooling::DebugInterface> debugger_;

        NO_COPY_SEMANTIC(DebugSession);
        NO_MOVE_SEMANTIC(DebugSession);
    };

    using DebugSessionHandle = std::shared_ptr<DebugSession>;

    LanguageContext GetLanguageContext(const std::string &runtime_type);
    LanguageContext GetLanguageContext(const Method &method);
    LanguageContext GetLanguageContext(const Class &cls);
    LanguageContext GetLanguageContext(const BaseClass &cls);
    LanguageContext GetLanguageContext(panda_file::ClassDataAccessor *cda);
    LanguageContext GetLanguageContext(panda_file::SourceLang lang);

    static bool CreateInstance(const RuntimeOptions &options, mem::InternalAllocatorPtr internal_allocator);

    PANDA_PUBLIC_API static bool Create(const RuntimeOptions &options);

    // Deprecated. Only for capability with ets_runtime.
    PANDA_PUBLIC_API static bool Create(const RuntimeOptions &options, const std::vector<LanguageContextBase *> &ctxs);

    static bool DestroyUnderLockHolder();

    PANDA_PUBLIC_API static bool Destroy();

    PANDA_PUBLIC_API static Runtime *GetCurrent();

    template <typename Handler>
    static auto GetCurrentSync(Handler &&handler)
    {
        os::memory::LockHolder<os::memory::Mutex> lock(mutex);
        return handler(*GetCurrent());
    }

    ClassLinker *GetClassLinker() const
    {
        return class_linker_;
    }

    RuntimeNotificationManager *GetNotificationManager() const
    {
        return notification_manager_;
    }

    static const RuntimeOptions &GetOptions()
    {
        return options_;
    }

    void SetZygoteNoThreadSection(bool val)
    {
        zygote_no_threads_ = val;
    }

    coretypes::String *ResolveString(PandaVM *vm, const Method &caller, panda_file::File::EntityId id);

    coretypes::String *ResolveStringFromCompiledCode(PandaVM *vm, const Method &caller, panda_file::File::EntityId id);

    coretypes::String *ResolveString(PandaVM *vm, const panda_file::File &pf, panda_file::File::EntityId id,
                                     const LanguageContext &ctx);

    coretypes::String *ResolveString(PandaVM *vm, const uint8_t *mutf8, uint32_t length, const LanguageContext &ctx);

    Class *GetClassRootForLiteralTag(const ClassLinkerExtension &ext, panda_file::LiteralTag tag) const;

    static bool GetLiteralTagAndValue(const panda_file::File &pf, uint32_t id, panda_file::LiteralTag *tag,
                                      panda_file::LiteralDataAccessor::LiteralValue *value);

    uintptr_t GetPointerToConstArrayData(const panda_file::File &pf, uint32_t id) const;

    coretypes::Array *ResolveLiteralArray(PandaVM *vm, const Method &caller, uint32_t id);
    coretypes::Array *ResolveLiteralArray(PandaVM *vm, const panda_file::File &pf, uint32_t id,
                                          const LanguageContext &ctx);

    void PreZygoteFork();

    void PostZygoteFork();

    Expected<int, Error> ExecutePandaFile(std::string_view filename, std::string_view entry_point,
                                          const std::vector<std::string> &args);

    int StartDProfiler(std::string_view app_name);

    Expected<int, Error> Execute(std::string_view entry_point, const std::vector<std::string> &args);

    int StartDProfiler(const PandaString &app_name);

    bool IsDebugMode() const
    {
        return is_debug_mode_;
    }

    void SetDebugMode(bool is_debug_mode)
    {
        is_debug_mode_ = is_debug_mode;
    }

    bool IsDebuggerConnected() const
    {
        return is_debugger_connected_;
    }

    void SetDebuggerConnected(bool dbg_connected_state)
    {
        is_debugger_connected_ = dbg_connected_state;
    }

    bool IsProfileableFromShell() const
    {
        return is_profileable_from_shell_;
    }

    void SetProfileableFromShell(bool profileable_from_shell)
    {
        is_profileable_from_shell_ = profileable_from_shell;
    }

    PandaVector<PandaString> GetBootPandaFiles();

    PandaVector<PandaString> GetPandaFiles();

    // Returns true if profile saving is enabled.
    bool SaveProfileInfo() const;

    std::string GetPandaPath()
    {
        return panda_path_string_;
    }

    static const std::string &GetRuntimeType()
    {
        return runtime_type_;
    }

    void UpdateProcessState(int state);

    bool IsZygote() const
    {
        return is_zygote_;
    }

    bool IsInitialized() const
    {
        return is_initialized_;
    }

    // TODO: lack NativeBridgeAction action
    void InitNonZygoteOrPostFork(bool is_system_server, const char *isa, bool profile_system_server = false);

    static const char *GetVersion()
    {
        // TODO(chenmudan): change to the correct version when we have one;
        return "1.0.0";
    }

    PandaString GetFingerprint()
    {
        return fingerPrint_;
    }

    [[noreturn]] static void Halt(int32_t status);

    void SetExitHook(ExitHook exit_hook)
    {
        ASSERT(exit_ == nullptr);
        exit_ = exit_hook;
    }

    void SetAbortHook(AbortHook abort_hook)
    {
        ASSERT(abort_ == nullptr);
        abort_ = abort_hook;
    }

    [[noreturn]] static void Abort(const char *message = nullptr);

    Expected<Method *, Error> ResolveEntryPoint(std::string_view entry_point);

    void RegisterSensitiveThread() const;

    // Deprecated.
    // Get VM instance from the thread. In multi-vm runtime this method returns
    // the first VM. It is undeterminated which VM will be first.
    PandaVM *GetPandaVM() const
    {
        return panda_vm_;
    }

    ClassHierarchyAnalysis *GetCha() const
    {
        return cha_;
    }

    const panda::verifier::VerificationOptions &GetVerificationOptions() const
    {
        return VerificationOptions_;
    }

    panda::verifier::VerificationOptions &GetVerificationOptions()
    {
        return VerificationOptions_;
    }

    bool IsDebuggerAttached()
    {
        return debug_session_.use_count() > 0;
    }

    void DumpForSigQuit(std::ostream &os);

    bool IsDumpNativeCrash()
    {
        return is_dump_native_crash_;
    }

    bool IsChecksSuspend() const
    {
        return checks_suspend_;
    }

    bool IsChecksStack() const
    {
        return checks_stack_;
    }

    bool IsChecksNullptr() const
    {
        return checks_nullptr_;
    }

    bool IsStacktrace() const
    {
        return is_stacktrace_;
    }

    bool IsJitEnabled() const
    {
        return is_jit_enabled_;
    }

    void ForceEnableJit()
    {
        is_jit_enabled_ = true;
    }

    void ForceDisableJit()
    {
        is_jit_enabled_ = false;
    }

    static mem::GCType GetGCType(const RuntimeOptions &options, panda_file::SourceLang lang);

    static void SetDaemonMemoryLeakThreshold(uint32_t daemon_memory_leak_threshold);

    DebugSessionHandle StartDebugSession();

    mem::InternalAllocatorPtr GetInternalAllocator() const
    {
        return internal_allocator_;
    }

    PandaString GetMemoryStatistics();
    PandaString GetFinalStatistics();

    Expected<LanguageContext, Error> ExtractLanguageContext(const panda_file::File *pf, std::string_view entry_point);

    UnwindStackFn GetUnwindStackFn() const
    {
        return unwind_stack_fn_;
    }

    void SetUnwindStackFn(UnwindStackFn unwind_stack_fn)
    {
        unwind_stack_fn_ = unwind_stack_fn;
    }

    RelayoutProfiler *GetRelayoutProfiler()
    {
        return relayout_profiler_;
    }

private:
    void NotifyAboutLoadedModules();

    std::optional<Error> CreateApplicationClassLinkerContext(std::string_view filename, std::string_view entry_point);

    bool LoadVerificationConfig();

    bool CreatePandaVM(std::string_view runtime_type);

    bool InitializePandaVM();

    bool HandleAotOptions();

    void HandleJitOptions();

    bool CheckOptionsConsistency();

    void SetPandaPath();

    bool Initialize();

    bool Shutdown();

    bool LoadBootPandaFiles(panda_file::File::OpenMode open_mode);

    static void BlockSignals();

    inline void InitializeVerificationResultCache();

    Runtime(const RuntimeOptions &options, mem::InternalAllocatorPtr internal_allocator);

    ~Runtime();

    static Runtime *instance;
    static RuntimeOptions options_;
    static std::string runtime_type_;
    static os::memory::Mutex mutex;

    // TODO(dtrubenk): put all of it in the permanent space
    mem::InternalAllocatorPtr internal_allocator_;
    RuntimeNotificationManager *notification_manager_;
    ClassLinker *class_linker_;
    ClassHierarchyAnalysis *cha_;
    DProfiler *dprofiler_ = nullptr;

    PandaVM *panda_vm_ = nullptr;

    // For IDE is real connected.
    bool is_debug_mode_ {false};
    bool is_debugger_connected_ {false};
    bool is_profileable_from_shell_ {false};
    os::memory::Mutex debug_session_creation_mutex_ {};
    os::memory::Mutex debug_session_uniqueness_mutex_ {};
    DebugSessionHandle debug_session_ {};

    // For saving class path.
    std::string panda_path_string_;

    AbortHook abort_ = nullptr;
    ExitHook exit_ = nullptr;

    bool zygote_no_threads_;
    bool is_zygote_;
    bool is_initialized_ {false};

    bool save_profiling_info_;

    bool checks_suspend_ {false};
    bool checks_stack_ {true};
    bool checks_nullptr_ {true};
    bool is_stacktrace_ {false};
    bool is_jit_enabled_ {false};

    bool is_dump_native_crash_ {true};

    PandaString fingerPrint_ = "unknown";

    // Verification
    panda::verifier::VerificationOptions VerificationOptions_;

    struct AppContext {
        ClassLinkerContext *ctx {nullptr};
        std::optional<panda_file::SourceLang> lang;
    };
    AppContext app_context_ {};

    RuntimeController *runtime_controller_ {nullptr};
    UnwindStackFn unwind_stack_fn_ {nullptr};

    RelayoutProfiler *relayout_profiler_ {nullptr};

    NO_COPY_SEMANTIC(Runtime);
    NO_MOVE_SEMANTIC(Runtime);
};

inline mem::AllocatorAdapter<void> GetInternalAllocatorAdapter(const Runtime *runtime)
{
    return runtime->GetInternalAllocator()->Adapter();
}

void InitSignals();

}  // namespace panda

#endif  // PANDA_RUNTIME_RUNTIME_H_
