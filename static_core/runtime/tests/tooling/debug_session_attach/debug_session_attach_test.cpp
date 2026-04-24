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

#include <gtest/gtest.h>

#include <algorithm>
#include <atomic>
#include <string>
#include <thread>
#include <vector>

#include "libarkbase/os/mutex.h"
#include "libarkbase/utils/utf.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/method.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/tooling/tools.h"

namespace ark::tooling::test {

static std::string GetPandaFilePath()
{
    return "debug_session_attach_test.abc";
}

static std::string GetAotFilePath()
{
    return "debug_session_attach_test.an";
}

class BytecodeEventListener final : public RuntimeListener {
public:
    struct Event final {
        ManagedThread *thread;
        Method *method;
        uint32_t bcOffset;
    };

    void BytecodePcChanged(ManagedThread *thread, Method *method, uint32_t bcOffset) override
    {
        os::memory::LockHolder lock(mutex_);
        events_.push_back({thread, method, bcOffset});
    }

    auto GetEvents() const
    {
        os::memory::LockHolder lock(mutex_);
        return events_;
    }

private:
    mutable os::memory::Mutex mutex_ {};
    std::vector<Event> events_ {};
};

static constexpr int32_t MANAGED_WAIT_LEAF_RESULT = 3;
static constexpr int32_t LOOP_RESULT = 7;
static constexpr int32_t CATCHER_RESULT = 1;

class AttachDebugSessionSync final {
public:
    explicit AttachDebugSessionSync(size_t waitingThreadsCount = 1) : waitingThreadsCount_(waitingThreadsCount) {}

    void ManagedThreadWait()
    {
        ScopedNativeCodeThread nativeScope(ManagedThread::GetCurrent());
        // Atomic with relaxed order reason: ordering constraints are not required
        const auto prev = managedThreadWaitingCount_.fetch_add(1, std::memory_order_relaxed);
        if (prev == waitingThreadsCount_ - 1) {
            threadsStartEvent_.Fire();
        }
        attachEvent_.Wait();
    }

    void WaitUntilManagedThreadsWait()
    {
        threadsStartEvent_.Wait();
    }

    void ContinueManagedThreads()
    {
        attachEvent_.Fire();
    }

private:
    os::memory::Event attachEvent_ {};
    os::memory::Event threadsStartEvent_ {};
    const size_t waitingThreadsCount_ {0};
    std::atomic<size_t> managedThreadWaitingCount_ {0};
};

static AttachDebugSessionSync *g_attachDebugSessionSync = nullptr;

static int32_t AttachDebugSessionImpl(Method * /* unused */)
{
    ScopedNativeCodeThread snct(ManagedThread::GetCurrent());
    return static_cast<int32_t>(Runtime::GetCurrent()->GetTools().AttachDebugSession());
}

static void WaitForDebugSessionImpl(Method * /* unused */)
{
    ASSERT_NE(g_attachDebugSessionSync, nullptr);
    g_attachDebugSessionSync->ManagedThreadWait();
}

static void ThrowErrorImpl(Method * /* unused */)
{
    auto *extension = Runtime::GetCurrent()->GetClassLinker()->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);
    PandaString descriptor;
    auto *eClass = extension->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("E"), &descriptor));
    ASSERT_NE(eClass, nullptr);
    auto *obj = ObjectHeader::Create(eClass);
    if (LIKELY(!ManagedThread::GetCurrent()->HasPendingException())) {
        ManagedThread::GetCurrent()->SetException(obj);
    }
}

static const void *const *g_dispatchTable = nullptr;
static const void *const *g_debugDispatchTable = nullptr;

static void GetDispatchTablesImpl(Method * /* unused */)
{
    auto *thread = ManagedThread::GetCurrent();
    g_dispatchTable = thread->GetCurrentDispatchTable<false>();
    g_debugDispatchTable = thread->GetDebugDispatchTable();
}

class DebugSessionAttachTest : public testing::Test {
public:
    DebugSessionAttachTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        // Note that default interpreter-type is used, which is either 'irtoc' or 'llvm'
        options.SetCompilerEnableJit(false);
        options.SetDebuggerAttachAllowed(true);
        options.SetPandaFiles({GetPandaFilePath()});
        options.SetAotFile(GetAotFilePath());
        Runtime::Create(options);
    }

    ~DebugSessionAttachTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(DebugSessionAttachTest);
    NO_MOVE_SEMANTIC(DebugSessionAttachTest);

    void SetUp() override
    {
        thread_ = ManagedThread::GetCurrent();

        auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
        auto pf = panda_file::OpenPandaFileOrZip(GetPandaFilePath());
        ASSERT_NE(pf, nullptr);
        classLinker->AddPandaFile(std::move(pf));

        {
            ScopedManagedCodeThread smc(thread_);

            auto *extension = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);
            PandaString descriptor;
            globalClass_ = extension->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("_GLOBAL"), &descriptor));
            ASSERT_NE(globalClass_, nullptr);

            auto *attachDebugSessionMethod = globalClass_->GetDirectMethod(utf::CStringAsMutf8("attachDebugSession"));
            ASSERT_NE(attachDebugSessionMethod, nullptr);
            attachDebugSessionMethod->SetNativePointer(reinterpret_cast<void *>(AttachDebugSessionImpl));
            attachDebugSessionMethod->SetCompiledEntryPoint(reinterpret_cast<const void *>(AttachDebugSessionImpl));

            auto *waitForDebugSessionMethod = globalClass_->GetDirectMethod(utf::CStringAsMutf8("waitForDebugSession"));
            ASSERT_NE(waitForDebugSessionMethod, nullptr);
            waitForDebugSessionMethod->SetNativePointer(reinterpret_cast<void *>(WaitForDebugSessionImpl));
            waitForDebugSessionMethod->SetCompiledEntryPoint(reinterpret_cast<const void *>(WaitForDebugSessionImpl));

            auto *throwErrorFromNativeMethod =
                globalClass_->GetDirectMethod(utf::CStringAsMutf8("throwErrorFromNative"));
            ASSERT_NE(throwErrorFromNativeMethod, nullptr);
            throwErrorFromNativeMethod->SetNativePointer(reinterpret_cast<void *>(ThrowErrorImpl));
            throwErrorFromNativeMethod->SetCompiledEntryPoint(reinterpret_cast<const void *>(ThrowErrorImpl));

            auto *getDispatchTablesMethod = globalClass_->GetDirectMethod(utf::CStringAsMutf8("getDispatchTablesImpl"));
            ASSERT_NE(getDispatchTablesMethod, nullptr);
            getDispatchTablesMethod->SetNativePointer(reinterpret_cast<void *>(GetDispatchTablesImpl));
            getDispatchTablesMethod->SetCompiledEntryPoint(reinterpret_cast<const void *>(GetDispatchTablesImpl));
        }
    }

protected:
    ManagedThread *thread_ {nullptr};
    Class *globalClass_ {nullptr};
};

class NotAllowedDebugAttachTest : public testing::Test {
public:
    NotAllowedDebugAttachTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetCompilerEnableJit(false);
        // Check that `debugger-attach-allowed` is `false` by default and `AttachDebugSession` discards
        Runtime::Create(options);
    }

    ~NotAllowedDebugAttachTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(NotAllowedDebugAttachTest);
    NO_MOVE_SEMANTIC(NotAllowedDebugAttachTest);
};

TEST_F(NotAllowedDebugAttachTest, AttachIsDisabledByDefault)
{
    auto status = Runtime::GetCurrent()->GetTools().AttachDebugSession();
    ASSERT_EQ(status, DebugSessionAttachErrorCode::NOT_ALLOWED);
}

class NotAllowedDebugAttachWithJitTest : public testing::Test {
public:
    NotAllowedDebugAttachWithJitTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetDebuggerAttachAllowed(true);
        // Check that `AttachDebugSession` discards when JIT is enabled
        options.SetCompilerEnableJit(true);
        Runtime::Create(options);
    }

    ~NotAllowedDebugAttachWithJitTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(NotAllowedDebugAttachWithJitTest);
    NO_MOVE_SEMANTIC(NotAllowedDebugAttachWithJitTest);
};

TEST_F(NotAllowedDebugAttachWithJitTest, AttachIsNotAllowedWithJit)
{
    auto status = Runtime::GetCurrent()->GetTools().AttachDebugSession();
    ASSERT_EQ(status, DebugSessionAttachErrorCode::NOT_ALLOWED);
}

// ---------------------------------------------------------------------------
// Basic attach: debug mode is enabled, debug session created
// ---------------------------------------------------------------------------
TEST_F(DebugSessionAttachTest, AttachEnablesDebugMode)
{
    auto *runtime = Runtime::GetCurrent();

    BytecodeEventListener listener;
    auto *notificationManager = runtime->GetNotificationManager();
    notificationManager->AddListener(&listener, RuntimeNotificationManager::BYTECODE_PC_CHANGED);

    Method *simpleMethod = globalClass_->GetDirectMethod(utf::CStringAsMutf8("simpleMethod"));
    ASSERT_NE(simpleMethod, nullptr);

    EXPECT_FALSE(runtime->IsDebugMode());

    auto status = runtime->GetTools().AttachDebugSession();
    ASSERT_EQ(status, DebugSessionAttachErrorCode::OK);

    EXPECT_TRUE(runtime->IsDebugMode());

    {
        ScopedManagedCodeThread smc(thread_);
        simpleMethod->Invoke(thread_, nullptr);
    }

    notificationManager->RemoveListener(&listener, RuntimeNotificationManager::BYTECODE_PC_CHANGED);

    ASSERT_EQ(listener.GetEvents().size(), 2U);
    for (const auto &event : listener.GetEvents()) {
        EXPECT_EQ(event.thread, thread_);
        EXPECT_EQ(event.method, simpleMethod);
    }
}

// ---------------------------------------------------------------------------
// Second attach is rejected
// ---------------------------------------------------------------------------
TEST_F(DebugSessionAttachTest, DoubleAttachReturnsAlreadyAttached)
{
    auto status1 = Runtime::GetCurrent()->GetTools().AttachDebugSession();
    EXPECT_EQ(status1, DebugSessionAttachErrorCode::OK);

    auto status2 = Runtime::GetCurrent()->GetTools().AttachDebugSession();
    EXPECT_EQ(status2, DebugSessionAttachErrorCode::ALREADY_ATTACHED);
}

// ---------------------------------------------------------------------------
// Dispatch table is swapped from release to debug stubs
// ---------------------------------------------------------------------------
TEST_F(DebugSessionAttachTest, DispatchTableSwitch)
{
    // Check that thread initially has unset dispatch tables
    ASSERT_EQ(thread_->GetCurrentDispatchTable<false>(), nullptr);
    ASSERT_EQ(thread_->GetDebugDispatchTable(), nullptr);

    {
        ScopedManagedCodeThread smc(thread_);

        auto *getDispatchTables = globalClass_->GetDirectMethod(utf::CStringAsMutf8("getDispatchTables"));
        ASSERT_NE(getDispatchTables, nullptr);
        getDispatchTables->Invoke(thread_, nullptr);
        ASSERT_FALSE(thread_->HasPendingException());

        // Check that thread's dispatch tables were set back after Invoke
        ASSERT_NE(g_dispatchTable, nullptr);
        ASSERT_NE(g_debugDispatchTable, nullptr);
        ASSERT_EQ(thread_->GetCurrentDispatchTable<false>(), nullptr);
        ASSERT_EQ(thread_->GetDebugDispatchTable(), nullptr);

        thread_->SetCurrentDispatchTable<false>(g_dispatchTable);
        thread_->SetDebugDispatchTable(g_debugDispatchTable);

        auto *simpleMethod = globalClass_->GetDirectMethod(utf::CStringAsMutf8("simpleMethod"));
        ASSERT_NE(simpleMethod, nullptr);
        simpleMethod->Invoke(thread_, nullptr);
        ASSERT_FALSE(thread_->HasPendingException());

        // Check that thread's dispatch tables were set back after Invoke
        ASSERT_EQ(thread_->GetCurrentDispatchTable<false>(), g_dispatchTable);
        ASSERT_EQ(thread_->GetDebugDispatchTable(), g_debugDispatchTable);
    }

    auto status = Runtime::GetCurrent()->GetTools().AttachDebugSession();
    ASSERT_EQ(status, DebugSessionAttachErrorCode::OK);

    // Check that thread's non-debug dispatch table now points to debug dispatch table
    ASSERT_EQ(thread_->GetCurrentDispatchTable<false>(), g_debugDispatchTable);
    ASSERT_EQ(thread_->GetDebugDispatchTable(), g_debugDispatchTable);

    g_dispatchTable = nullptr;
    g_debugDispatchTable = nullptr;
}

static bool HasEventAtNonZeroOffset(const std::vector<BytecodeEventListener::Event> &events, ManagedThread *thread,
                                    Method *method)
{
    return std::find_if(events.begin(), events.end(), [thread, method](const auto &event) {
               return event.thread == thread && event.method == method && event.bcOffset != 0;
           }) != events.end();
}

TEST_F(DebugSessionAttachTest, AttachInMainThread)
{
    Value result;
    {
        ScopedManagedCodeThread smc(thread_);

        auto *getDispatchTables = globalClass_->GetDirectMethod(utf::CStringAsMutf8("getDispatchTables"));
        ASSERT_NE(getDispatchTables, nullptr);
        getDispatchTables->Invoke(thread_, nullptr);
        ASSERT_FALSE(thread_->HasPendingException());

        // Check that thread's dispatch tables were set back after Invoke
        ASSERT_NE(g_dispatchTable, nullptr);
        ASSERT_NE(g_debugDispatchTable, nullptr);
        ASSERT_EQ(thread_->GetCurrentDispatchTable<false>(), nullptr);
        ASSERT_EQ(thread_->GetDebugDispatchTable(), nullptr);

        thread_->SetCurrentDispatchTable<false>(g_dispatchTable);
        thread_->SetDebugDispatchTable(g_debugDispatchTable);

        auto *attachInMainThreadMethod = globalClass_->GetDirectMethod(utf::CStringAsMutf8("attachInMainThread"));
        ASSERT_NE(attachInMainThreadMethod, nullptr);

        result = attachInMainThreadMethod->Invoke(thread_, nullptr);
        ASSERT_FALSE(thread_->HasPendingException());
    }

    // Check that thread's non-debug dispatch table now points to debug dispatch table
    ASSERT_EQ(thread_->GetCurrentDispatchTable<false>(), g_debugDispatchTable);
    ASSERT_EQ(thread_->GetDebugDispatchTable(), g_debugDispatchTable);

    g_dispatchTable = nullptr;
    g_debugDispatchTable = nullptr;

    auto attachStatus = static_cast<DebugSessionAttachErrorCode>(result.GetAs<int32_t>());
    EXPECT_EQ(attachStatus, DebugSessionAttachErrorCode::OK);
}

// ---------------------------------------------------------------------------
// Methods with fake AOT-compiled code are deoptimized by AttachDebugSession
// ---------------------------------------------------------------------------
TEST_F(DebugSessionAttachTest, DeoptimizesCompiledMethods)
{
    Method *methodA = nullptr;
    Method *methodB = nullptr;
    {
        ScopedManagedCodeThread smc(thread_);

        auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
        auto *extension = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);
        PandaString descriptor;
        auto *klassA = extension->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("A"), &descriptor));
        ASSERT_NE(klassA, nullptr);
        auto *klassB = extension->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("B"), &descriptor));
        ASSERT_NE(klassB, nullptr);

        methodA = klassA->GetDirectMethod(utf::CStringAsMutf8("aCompiledMethod"));
        ASSERT_NE(methodA, nullptr);
        methodB = klassB->GetDirectMethod(utf::CStringAsMutf8("bCompiledMethod"));
        ASSERT_NE(methodB, nullptr);

        ASSERT_TRUE(methodA->HasCompiledCode());
        ASSERT_TRUE(methodB->HasCompiledCode());
    }

    auto status = Runtime::GetCurrent()->GetTools().AttachDebugSession();
    ASSERT_EQ(status, DebugSessionAttachErrorCode::OK);

    // InvalidateCompiledEntryPoint must have reset both methods to the
    // interpreter bridge, so HasCompiledCode() returns false.
    EXPECT_FALSE(methodA->HasCompiledCode()) << "A.aCompiledMethod should be deoptimized after debug attach";
    EXPECT_EQ(methodA->GetCompilationStatus(), Method::NOT_COMPILED);
    EXPECT_FALSE(methodB->HasCompiledCode()) << "B.bCompiledMethod should be deoptimized after debug attach";
    EXPECT_EQ(methodB->GetCompilationStatus(), Method::NOT_COMPILED);
}

static DebugSessionAttachErrorCode AttachDebugSessionConcurrently(AttachDebugSessionSync &sync)
{
    auto *thread = MTManagedThread::Create(Runtime::GetCurrent(), Runtime::GetCurrent()->GetPandaVM());
    sync.WaitUntilManagedThreadsWait();
    DebugSessionAttachErrorCode nativeAttachStatus = Runtime::GetCurrent()->GetTools().AttachDebugSession();
    sync.ContinueManagedThreads();
    thread->Destroy();
    return nativeAttachStatus;
}

// ----------------------------------------------------------
// Test that switch to debug dispatch table happens on return
// ----------------------------------------------------------
TEST_F(DebugSessionAttachTest, SwitchToDebugOnReturn)
{
    auto *runtime = Runtime::GetCurrent();
    auto *notificationManager = runtime->GetNotificationManager();

    BytecodeEventListener listener;
    notificationManager->AddListener(&listener, RuntimeNotificationManager::BYTECODE_PC_CHANGED);

    AttachDebugSessionSync sync;
    g_attachDebugSessionSync = &sync;

    DebugSessionAttachErrorCode nativeAttachStatus = DebugSessionAttachErrorCode::INVALID;
    std::thread nativeThread(
        [&sync, &nativeAttachStatus] { nativeAttachStatus = AttachDebugSessionConcurrently(sync); });

    Method *managedWaitMainMethod = nullptr;
    Method *managedWaitMidMethod = nullptr;
    Value result;
    {
        ScopedManagedCodeThread smc(thread_);

        managedWaitMainMethod = globalClass_->GetDirectMethod(utf::CStringAsMutf8("managedWaitMain"));
        ASSERT_NE(managedWaitMainMethod, nullptr);
        managedWaitMidMethod = globalClass_->GetDirectMethod(utf::CStringAsMutf8("managedWaitMid"));
        ASSERT_NE(managedWaitMidMethod, nullptr);

        result = managedWaitMainMethod->Invoke(thread_, nullptr);
    }

    nativeThread.join();
    g_attachDebugSessionSync = nullptr;
    notificationManager->RemoveListener(&listener, RuntimeNotificationManager::BYTECODE_PC_CHANGED);

    auto events = listener.GetEvents();

    EXPECT_EQ(nativeAttachStatus, DebugSessionAttachErrorCode::OK);
    EXPECT_TRUE(runtime->IsDebugMode());
    EXPECT_EQ(result.GetAs<int32_t>(), MANAGED_WAIT_LEAF_RESULT);
    ASSERT_FALSE(events.empty());
    ASSERT_TRUE(HasEventAtNonZeroOffset(events, thread_, managedWaitMidMethod));
    ASSERT_TRUE(HasEventAtNonZeroOffset(events, thread_, managedWaitMainMethod));
}

// -------------------------------------------------------------------
// Test that switch to debug dispatch table happens on backward branch
// -------------------------------------------------------------------
TEST_F(DebugSessionAttachTest, SwitchToDebugOnBackwardBranch)
{
    auto *runtime = Runtime::GetCurrent();
    auto *notificationManager = runtime->GetNotificationManager();

    BytecodeEventListener listener;
    notificationManager->AddListener(&listener, RuntimeNotificationManager::BYTECODE_PC_CHANGED);

    AttachDebugSessionSync sync;
    g_attachDebugSessionSync = &sync;

    DebugSessionAttachErrorCode nativeAttachStatus = DebugSessionAttachErrorCode::INVALID;
    std::thread nativeThread(
        [&sync, &nativeAttachStatus] { nativeAttachStatus = AttachDebugSessionConcurrently(sync); });

    Method *loopMethod = nullptr;
    Value result;
    {
        ScopedManagedCodeThread smc(thread_);
        loopMethod = globalClass_->GetDirectMethod(utf::CStringAsMutf8("loopWithDebugAttach"));
        ASSERT_NE(loopMethod, nullptr);
        result = loopMethod->Invoke(thread_, nullptr);
    }

    nativeThread.join();
    g_attachDebugSessionSync = nullptr;
    notificationManager->RemoveListener(&listener, RuntimeNotificationManager::BYTECODE_PC_CHANGED);

    auto events = listener.GetEvents();

    EXPECT_EQ(nativeAttachStatus, DebugSessionAttachErrorCode::OK);
    EXPECT_TRUE(runtime->IsDebugMode());
    EXPECT_EQ(result.GetAs<int32_t>(), LOOP_RESULT);
    ASSERT_TRUE(HasEventAtNonZeroOffset(events, thread_, loopMethod));
}

std::pair<Method *, Method *> GetManagedWaitMethods(ManagedThread *thread, Class *klass)
{
    ScopedManagedCodeThread smc(thread);
    auto *managedWaitMainMethod = klass->GetDirectMethod(utf::CStringAsMutf8("managedWaitMain"));
    auto *managedWaitMidMethod = klass->GetDirectMethod(utf::CStringAsMutf8("managedWaitMid"));
    return {managedWaitMainMethod, managedWaitMidMethod};
}

// -----------------------------------------------------------------
// Test that multiple waiting managed threads switch to debug mode
// -----------------------------------------------------------------
TEST_F(DebugSessionAttachTest, SwitchesMultipleWaitingThreadsToDebug)
{
    auto *runtime = Runtime::GetCurrent();
    auto *notificationManager = runtime->GetNotificationManager();

    BytecodeEventListener listener;
    notificationManager->AddListener(&listener, RuntimeNotificationManager::BYTECODE_PC_CHANGED);

    auto [managedWaitMainMethod, managedWaitMidMethod] = GetManagedWaitMethods(thread_, globalClass_);
    ASSERT_NE(managedWaitMainMethod, nullptr);
    ASSERT_NE(managedWaitMidMethod, nullptr);

    AttachDebugSessionSync sync(2U);
    g_attachDebugSessionSync = &sync;

    DebugSessionAttachErrorCode nativeAttachStatus = DebugSessionAttachErrorCode::INVALID;
    std::thread nativeThread(
        [&sync, &nativeAttachStatus] { nativeAttachStatus = AttachDebugSessionConcurrently(sync); });

    ManagedThread *managedThread1 = nullptr;
    ManagedThread *managedThread2 = nullptr;
    int32_t result1 = 0;
    int32_t result2 = 0;
    std::thread thread1([&managedThread1, &result1, m = managedWaitMainMethod] {
        auto *thread = MTManagedThread::Create(Runtime::GetCurrent(), Runtime::GetCurrent()->GetPandaVM());
        managedThread1 = thread;
        {
            ScopedManagedCodeThread smc(thread);
            result1 = m->Invoke(thread, nullptr).GetAs<int32_t>();
        }
        thread->Destroy();
    });
    std::thread thread2([&managedThread2, &result2, m = managedWaitMidMethod] {
        auto *thread = MTManagedThread::Create(Runtime::GetCurrent(), Runtime::GetCurrent()->GetPandaVM());
        managedThread2 = thread;
        {
            ScopedManagedCodeThread smc(thread);
            result2 = m->Invoke(thread, nullptr).GetAs<int32_t>();
        }
        thread->Destroy();
    });

    thread1.join();
    thread2.join();
    nativeThread.join();
    g_attachDebugSessionSync = nullptr;
    notificationManager->RemoveListener(&listener, RuntimeNotificationManager::BYTECODE_PC_CHANGED);

    auto events = listener.GetEvents();

    EXPECT_EQ(nativeAttachStatus, DebugSessionAttachErrorCode::OK);
    EXPECT_TRUE(runtime->IsDebugMode());
    EXPECT_EQ(result1, MANAGED_WAIT_LEAF_RESULT);
    EXPECT_EQ(result2, MANAGED_WAIT_LEAF_RESULT);
    ASSERT_TRUE(HasEventAtNonZeroOffset(events, managedThread1, managedWaitMainMethod));
    ASSERT_TRUE(HasEventAtNonZeroOffset(events, managedThread2, managedWaitMidMethod));
}

// --------------------------------------------------------------------
// Test that switch to debug dispatch table happens on thrown exception
// --------------------------------------------------------------------
void TestSwitchToDebugOnException(ManagedThread *currentThread, Class *globalClass, const uint8_t *catchMethodName)
{
    ASSERT_EQ(currentThread, ManagedThread::GetCurrent());

    auto *runtime = Runtime::GetCurrent();
    auto *notificationManager = runtime->GetNotificationManager();

    BytecodeEventListener listener;
    notificationManager->AddListener(&listener, RuntimeNotificationManager::BYTECODE_PC_CHANGED);

    AttachDebugSessionSync sync;
    g_attachDebugSessionSync = &sync;

    DebugSessionAttachErrorCode nativeAttachStatus = DebugSessionAttachErrorCode::INVALID;
    std::thread nativeThread(
        [&sync, &nativeAttachStatus] { nativeAttachStatus = AttachDebugSessionConcurrently(sync); });

    Method *catcherMethod = nullptr;
    Value result;
    {
        ScopedManagedCodeThread smc(currentThread);
        catcherMethod = globalClass->GetDirectMethod(catchMethodName);
        ASSERT_NE(catcherMethod, nullptr);
        result = catcherMethod->Invoke(currentThread, nullptr);
    }

    nativeThread.join();
    g_attachDebugSessionSync = nullptr;
    notificationManager->RemoveListener(&listener, RuntimeNotificationManager::BYTECODE_PC_CHANGED);

    EXPECT_EQ(nativeAttachStatus, DebugSessionAttachErrorCode::OK);
    EXPECT_TRUE(runtime->IsDebugMode());
    EXPECT_EQ(result.GetAs<int32_t>(), CATCHER_RESULT);
    EXPECT_TRUE(HasEventAtNonZeroOffset(listener.GetEvents(), currentThread, catcherMethod));
}

TEST_F(DebugSessionAttachTest, SwitchToDebugOnExceptionThrownFromManaged)
{
    TestSwitchToDebugOnException(thread_, globalClass_, utf::CStringAsMutf8("catchFromManagedThrow"));
}

TEST_F(DebugSessionAttachTest, SwitchToDebugOnExceptionThrownFromNative)
{
    TestSwitchToDebugOnException(thread_, globalClass_, utf::CStringAsMutf8("catchFromNativeThrow"));
}

}  // namespace ark::tooling::test
