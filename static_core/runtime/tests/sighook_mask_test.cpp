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
#include <csignal>
#include <atomic>
#include <chrono>
#include <thread>
#include <ucontext.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "libarkbase/os/sighook.h"
#include "runtime/include/runtime.h"
#include "runtime/include/managed_thread.h"

namespace ark::test {

static constexpr auto SIGNAL_DELIVERY_WAIT = std::chrono::microseconds(1000);
static std::atomic<bool> g_handlerCalled {false};
static sigset_t g_capturedMask = []() {
    sigset_t s;
    sigemptyset(&s);
    return s;
}();

static void ResetState()
{
    // Atomic with release order reason: Reset the completion flag with the same
    // release/acquire synchronization contract used by this test.
    g_handlerCalled.store(false, std::memory_order_release);
    sigemptyset(&g_capturedMask);
}

static inline std::string Separator()
{
#ifdef _WIN32
    return "\\";
#else
    return "/";
#endif
}

class SighookMaskTest : public testing::Test {
public:
    SighookMaskTest()
    {
        RuntimeOptions options;
        options.SetLoadRuntimes({"core"});
        auto execPath = ark::os::file::File::GetExecutablePath();
        std::string pandaStdLib =
            execPath.Value() + Separator() + ".." + Separator() + "pandastdlib" + Separator() + "arkstdlib.abc";
        options.SetBootPandaFiles({pandaStdLib});
        options.SetGcType("epsilon");
        options.SetCompilerEnableJit(false);
        options.SetCompilerEnableOsr(false);
        Runtime::Create(options);
    }

    ~SighookMaskTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(SighookMaskTest);
    NO_MOVE_SEMANTIC(SighookMaskTest);
};

static void OldActionHandler([[maybe_unused]] int signo, [[maybe_unused]] siginfo_t *info, [[maybe_unused]] void *ctx)
{
    pthread_sigmask(SIG_SETMASK, nullptr, &g_capturedMask);
    // Atomic with release order reason: Publish that g_capturedMask was already
    // filled before the test thread observes handler completion.
    g_handlerCalled.store(true, std::memory_order_release);
}

static bool HookThatReturnsFalse([[maybe_unused]] int signo, [[maybe_unused]] siginfo_t *info,
                                 [[maybe_unused]] void *ctx)
{
    return false;
}

TEST_F(SighookMaskTest, CallOldActionMergesMaskAndUcSigmask)
{
    ResetState();

    // Step 1: install an old handler with sa_mask = {SIGWINCH}, SA_SIGINFO, no SA_NODEFER
    struct sigaction oldAct {};
    oldAct.sa_sigaction = OldActionHandler;
    oldAct.sa_flags = SA_SIGINFO;  // no SA_NODEFER -> signo should be auto-blocked
    sigemptyset(&oldAct.sa_mask);
    sigaddset(&oldAct.sa_mask, SIGWINCH);

    struct sigaction prevAct {};
    ASSERT_EQ(sigaction(SIGHUP, &oldAct, &prevAct), 0);

    // Step 2: register hook (returns false → falls through to CallOldAction)
    sigset_t hookMask;
    sigfillset(&hookMask);
    SigchainAction hookAction = {HookThatReturnsFalse, hookMask, 0};
    AddSpecialSignalHandlerFn(SIGHUP, &hookAction);

    // Step 3: block SIGALRM in current thread so uc_sigmask will contain it
    sigset_t blockSet;
    sigemptyset(&blockSet);
    sigaddset(&blockSet, SIGALRM);
    sigset_t savedMask;
    pthread_sigmask(SIG_BLOCK, &blockSet, &savedMask);

    // Step 4: send signal to self
    pthread_kill(pthread_self(), SIGHUP);

    // Give a tiny bit of time for synchronous signal delivery
    std::this_thread::sleep_for(SIGNAL_DELIVERY_WAIT);

    // Atomic with acquire order reason: Synchronize with the handler's release
    // store before checking the captured signal mask.
    ASSERT_TRUE(g_handlerCalled.load(std::memory_order_acquire));

    // Step 5: verify captured mask
    EXPECT_EQ(sigismember(&g_capturedMask, SIGWINCH), 1) << "sa_mask (SIGWINCH) must be in mask";
    EXPECT_EQ(sigismember(&g_capturedMask, SIGALRM), 1) << "uc_sigmask (SIGALRM) must be in mask";
    EXPECT_EQ(sigismember(&g_capturedMask, SIGHUP), 1) << "signo (SIGHUP) must be blocked";

    // Cleanup
    RemoveSpecialSignalHandlerFn(SIGHUP, HookThatReturnsFalse);
    pthread_sigmask(SIG_SETMASK, &savedMask, nullptr);
    sigaction(SIGHUP, &prevAct, nullptr);
}

}  // namespace ark::test
