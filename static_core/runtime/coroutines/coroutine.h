/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_COROUTINES_COROUTINE_H
#define PANDA_RUNTIME_COROUTINES_COROUTINE_H

#include <atomic>
#include <optional>
#include "runtime/coroutines/coroutine_events.h"
#include "runtime/include/runtime.h"
#include "runtime/include/managed_thread.h"

namespace ark {

class CoroutineContext;
class CompletionEvent;
/**
 * @brief The base class for all coroutines. Holds the managed part of the coroutine context.
 *
 * The coroutine context is splitted into managed and native parts.
 * The managed part is store in this class and its descendants. For the native part see the
 * CoroutineContext class and its descendants.
 */
class Coroutine : public ManagedThread {
public:
    NO_COPY_SEMANTIC(Coroutine);
    NO_MOVE_SEMANTIC(Coroutine);

    /**
     * Status transitions:
     *
     * +---------+                                  +----------+
     * | CREATED | -------------------------------> |          | <-------------+
     * +---------+                                  | RUNNABLE |               |
     *                                         +--- |          | <--+          |
     *                                         |    +----------+    |          |
     *                                         |                    |          |
     *                                         |    +----------+    |     +----------+
     *                                         +--> |          | ---+     |          |
     * +------------+      +-------------+          | RUNNING  |          | BLOCKED  |
     * | AWAIT_LOOP | <--- | TERMINATING | <------- |          | -------> |          |
     * +------------+      +-------------+          +----------+          +----------+
     *
     *
     * Main coroutine gets AWAIT_LOOP status once it starts the final waiting loop. After all
     * other coroutines are completed, the main coroutine exits.
     */
    enum class Status { CREATED, RUNNABLE, RUNNING, BLOCKED, TERMINATING, AWAIT_LOOP };

    /// Needed for object locking
    static constexpr ThreadId MAX_COROUTINE_ID = MarkWord::LIGHT_LOCK_THREADID_MAX_COUNT;

    /// A helper struct that aggregates all EP related data for a coroutine with a managed EP
    struct ManagedEntrypointInfo {
        // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
        CompletionEvent *completionEvent;  ///< not owned by this structure, just passed!
        // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
        Method *entrypoint;
        // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
        PandaVector<Value> &&arguments;

        /**
         * @param event an instance of CompletionEvent to be used on coroutine completion to pass the
         * return value to the point where it is needed. Also is used to unblock the coroutines that are waiting for
         * the one being created to complete.
         *
         * @param entry managed method to execute in the context of coroutine.
         *
         * @param args the array of EP method arguments
         */
        explicit ManagedEntrypointInfo(CompletionEvent *event, Method *entry, PandaVector<Value> &&args)
            : completionEvent(event), entrypoint(entry), arguments(std::move(args))
        {
            ASSERT(event != nullptr);
            ASSERT(entry != nullptr);
        }
    };

    /// a helper struct that aggregates all EP related data for a coroutine with a native EP
    struct NativeEntrypointInfo {
        using NativeEntrypointFunc = void (*)(void *);
        // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
        NativeEntrypointFunc entrypoint;
        // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
        void *param;

        /**
         * @param entry native function to execute in the context of coroutine
         *
         * @param arguments a parameter which will be passed to the entrypoint (usually some object pointer)
         */
        explicit NativeEntrypointInfo(NativeEntrypointFunc entry, void *data) : entrypoint(entry), param(data)
        {
            ASSERT(data != nullptr);
            ASSERT(entry != nullptr);
        }
    };

    using EntrypointInfo = std::variant<ManagedEntrypointInfo, NativeEntrypointInfo>;

    /**
     * The coroutine factory: creates and initializes a coroutine instance. The preferred way to create a
     * coroutine. For details see CoroutineManager::CoroutineFactory
     */
    static Coroutine *Create(Runtime *runtime, PandaVM *vm, PandaString name, CoroutineContext *context,
                             std::optional<EntrypointInfo> &&epInfo = std::nullopt);
    ~Coroutine() override;

    /// Should be called after creation in order to create native context and do other things
    virtual void Initialize();
    /**
     * Coroutine reinitialization: semantically equivalent to Initialize(),
     * but is called to prepare a cached Coroutine instance for reuse when it is needed.
     * Implies that the CleanUp() method was called before caching.
     */
    void ReInitialize(PandaString name, CoroutineContext *context, std::optional<EntrypointInfo> &&epInfo);
    /**
     * Manual destruction, applicable only to the main coro. Other ones get deleted by the coroutine manager once they
     * finish execution of their entrypoint method.
     */
    void Destroy();

    void CleanUp() override;

    bool RetrieveStackInfo(void *&stackAddr, size_t &stackSize, size_t &guardSize) override;

    static bool ThreadIsCoroutine(Thread *thread)
    {
        ASSERT(thread != nullptr);
        // NOTE(konstanting, #I67QXC): THREAD_TYPE_TASK -> THREAD_TYPE_COROUTINE and
        // remove the runtime/scheduler directory contents
        return thread->GetThreadType() == Thread::ThreadType::THREAD_TYPE_TASK;
    }

    static Coroutine *CastFromThread(Thread *thread)
    {
        ASSERT(thread != nullptr);
        ASSERT(ThreadIsCoroutine(thread));
        return static_cast<Coroutine *>(thread);
    }

    static Coroutine *GetCurrent()
    {
        Thread *thread = Thread::GetCurrent();
        ASSERT(thread != nullptr);
        if (ThreadIsCoroutine(thread)) {
            return CastFromThread(thread);
        }
        return nullptr;
    }

    /// Get coroutine status. It is independent from ThreadStatus.
    Status GetCoroutineStatus() const;
    /// Get coroutine name.
    PandaString GetName() const;
    /// Get unique coroutine ID
    uint32_t GetCoroutineId() const
    {
        return coroutineId_;
    }

    /**
     * Suspend a coroutine, so its status becomes either Status::RUNNABLE or Status::BLOCKED, depending on the suspend
     * reason.
     */
    virtual void RequestSuspend(bool getsBlocked);
    /// Resume the suspended coroutine, so its status becomes Status::RUNNING.
    virtual void RequestResume();
    /// Unblock the blocked coroutine, setting its status to Status::RUNNABLE
    virtual void RequestUnblock();
    /**
     * @brief Indicate that coroutine entrypoint execution is finished. Propagates the coroutine
     * return value to language level objects.
     */
    virtual void RequestCompletion(Value returnValue);

    /// Get the CompletionEvent instance
    CompletionEvent *GetCompletionEvent()
    {
        return std::get<ManagedEntrypointData>(entrypoint_).completionEvent;
    }

    bool HasManagedEntrypoint() const
    {
        return std::holds_alternative<ManagedEntrypointData>(entrypoint_);
    }

    /// Get coroutine's managed entrypoint method.
    Method *GetManagedEntrypoint() const
    {
        ASSERT(HasManagedEntrypoint());
        return std::get<ManagedEntrypointData>(entrypoint_).entrypoint;
    }

    /// Get coroutine's managed entrypoint args if any.
    PandaVector<Value> &GetManagedEntrypointArguments()
    {
        ASSERT(HasManagedEntrypoint());
        return std::get<ManagedEntrypointData>(entrypoint_).arguments;
    }

    const PandaVector<Value> &GetManagedEntrypointArguments() const
    {
        ASSERT(HasManagedEntrypoint());
        return std::get<ManagedEntrypointData>(entrypoint_).arguments;
    }

    bool HasNativeEntrypoint() const
    {
        return std::holds_alternative<NativeEntrypointData>(entrypoint_);
    }

    /// Get coroutine's native entry function (if this is a "native" coroutine)
    NativeEntrypointInfo::NativeEntrypointFunc GetNativeEntrypoint() const
    {
        ASSERT(HasNativeEntrypoint());
        return std::get<NativeEntrypointData>(entrypoint_).entrypoint;
    }

    /// Get coroutine's native entry function parameter (if this is a "native" coroutine)
    void *GetNativeEntrypointParam() const
    {
        ASSERT(HasNativeEntrypoint());
        return std::get<NativeEntrypointData>(entrypoint_).param;
    }

    template <class T>
    T *GetContext() const
    {
        return static_cast<T *>(context_);
    }

    bool IsSuspendOnStartup() const
    {
        return startSuspended_;
    }

protected:
    // We would like everyone to use the factory to create a Coroutine, thus ctor is protected
    explicit Coroutine(ThreadId id, mem::InternalAllocatorPtr allocator, PandaVM *vm,
                       ark::panda_file::SourceLang threadLang, PandaString name, CoroutineContext *context,
                       std::optional<EntrypointInfo> &&epInfo);

    void SetCoroutineStatus(Status newStatus);

private:
    /// a converter function that stores the data from EntrypointInfo in the member variables
    void SetEntrypointData(std::optional<EntrypointInfo> &&epInfo);

    PandaString name_;
    uint32_t coroutineId_ = 0;

    /// contains managed entrypoint parameters if the coroutine an EP and is "managed"
    struct ManagedEntrypointData {
        NO_COPY_SEMANTIC(ManagedEntrypointData);
        NO_MOVE_SEMANTIC(ManagedEntrypointData);

        // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
        CompletionEvent *completionEvent = nullptr;  ///< is owned by this structure
        // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
        Method *entrypoint = nullptr;
        // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
        PandaVector<Value> arguments;

        explicit ManagedEntrypointData(CompletionEvent *event, Method *entry, PandaVector<Value> &&args)
            : completionEvent(event), entrypoint(entry), arguments(std::move(args))
        {
            ASSERT(event != nullptr);
            ASSERT(entry != nullptr);
        }

        ~ManagedEntrypointData();
    };
    /// contains native entrypoint parameters if the coroutine has an EP and is "native"
    struct NativeEntrypointData {
        // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
        NativeEntrypointInfo::NativeEntrypointFunc entrypoint;
        // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
        void *param;

        explicit NativeEntrypointData(NativeEntrypointInfo::NativeEntrypointFunc entry, void *data)
            : entrypoint(entry), param(data)
        {
            ASSERT(data != nullptr);
            ASSERT(entry != nullptr);
        }
    };
    std::variant<std::monostate, ManagedEntrypointData, NativeEntrypointData> entrypoint_;

    CoroutineContext *context_ = nullptr;
    // NOTE(konstanting, #I67QXC): check if we still need this functionality
    bool startSuspended_ = false;

    // Allocator calls our protected ctor
    friend class mem::Allocator;
    friend class CoroutineContext;
};

std::ostream &operator<<(std::ostream &os, Coroutine::Status status);

}  // namespace ark

#endif  // PANDA_RUNTIME_COROUTINES_COROUTINE_H
