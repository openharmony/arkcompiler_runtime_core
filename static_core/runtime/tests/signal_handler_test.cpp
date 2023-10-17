/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <string>
#include <vector>

#include "libpandafile/file_items.h"
#include "libpandafile/value.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread.h"
#include "libpandabase/macros.h"
#include "os/thread.h"
#include "runtime/include/managed_thread.h"
#include <sys/syscall.h>
#include "assembly-parser.h"
#include "assembly-emitter.h"

namespace panda::test {

inline std::string Separator()
{
#ifdef _WIN32
    return "\\";
#else
    return "/";
#endif
}

static int COUNTER = 0;

void SigProfSamplingProfilerHandler([[maybe_unused]] int signum)
{
    COUNTER++;
    std::array<volatile int, 100> a {};  // NOLINT(readability-magic-numbers)
    for (int i = 0; i < 100; i++) {      // NOLINT(readability-magic-numbers)
        a[i] = i;
    }
}

class Sampler final {
public:
    Sampler() = default;
    ~Sampler() = default;

    bool Start()
    {
        managed_thread_ = os::thread::GetCurrentThreadId();
        is_active_ = true;

        // All prepairing actions should be done before this thread is started
        sampler_thread_ = std::make_unique<std::thread>(&Sampler::SamplerThreadEntry, this);
        return true;
    }

    void Stop()
    {
        if (!sampler_thread_->joinable()) {
            LOG(FATAL, PROFILER) << "Sampling profiler thread unexpectedly disappeared";
            UNREACHABLE();
        }

        is_active_ = false;

        sampler_thread_->join();
    }

private:
    void SamplerThreadEntry()
    {
        struct sigaction action {};
        action.sa_handler = SigProfSamplingProfilerHandler;
        action.sa_flags = 0;
        // Clear signal set
        sigemptyset(&action.sa_mask);
        // Ignore incoming sigprof if handler isn't completed
        sigaddset(&action.sa_mask, SIGPROF);

        struct sigaction old_action {};

        if (sigaction(SIGPROF, &action, &old_action) == -1) {
            LOG(FATAL, PROFILER) << "Sigaction failed, can't start profiling";
            UNREACHABLE();
        }

        auto pid = getpid();
        // Atomic with relaxed order reason: data race with is_active_
        while (is_active_.load(std::memory_order_relaxed)) {
            {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                if (syscall(SYS_tgkill, pid, managed_thread_, SIGPROF) != 0) {
                    LOG(ERROR, PROFILER) << "Can't send signal to thread";
                }
            }
            os::thread::NativeSleepUS(sample_interval_);
        }
    }

    std::unique_ptr<std::thread> sampler_thread_ {nullptr};

    std::atomic<bool> is_active_ = false;

    std::chrono::microseconds sample_interval_ {200};  // NOLINT(readability-magic-numbers)

    os::thread::ThreadId managed_thread_ = 0;

    NO_COPY_SEMANTIC(Sampler);
    NO_MOVE_SEMANTIC(Sampler);
};

class SignalHandlerTest : public testing::Test {
public:
    SignalHandlerTest()
    {
        RuntimeOptions options;
        options.SetLoadRuntimes({"core"});
        auto exec_path = panda::os::file::File::GetExecutablePath();
        std::string panda_std_lib =
            exec_path.Value() + Separator() + ".." + Separator() + "pandastdlib" + Separator() + "arkstdlib.abc";
        options.SetBootPandaFiles({panda_std_lib});
        options.SetGcType("epsilon");
        options.SetCompilerEnableJit(false);
        options.SetCompilerEnableOsr(false);
        Runtime::Create(options);

        thread_ = MTManagedThread::GetCurrent();
    }

    ~SignalHandlerTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(SignalHandlerTest);
    NO_MOVE_SEMANTIC(SignalHandlerTest);

private:
    MTManagedThread *thread_;
};

TEST_F(SignalHandlerTest, CallTest)
{
    auto source = R"(
    .record Math <external>
    .function f64 Math.pow (f64 a0, f64 a1) <external>
    .function f64 Math.absF64 (f64 a0) <external>

    .function void main_short(){
        movi v0, 100000           #loop_counter
        call.short main, v0
        return.void
    }

    .function void main_long(){
        movi v0, 10000000          #loop_counter
        call.short main, v0
        return.void
    }

    .function i32 main(i64 a0){
    movi v5, 0
    loop:
        lda v5
        jeq a0, loop_exit
        fmovi.64 v0, 2.5
        fmovi.64 v1, 24.7052942201
        fmovi.64 v2, 1e-6
        fmovi.64 v3, 3.5
        call.short Math.pow, v0, v3
        fsub2.64 v1
        sta.64 v1
        call.short Math.absF64, v1, v1
        fcmpl.64 v2
        jgez exit_failure
        inci v5, 1
        jmp loop
    loop_exit:
        ldai 0
        return
    exit_failure:
        ldai 1
        return
    }
    )";

    pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(res);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    Runtime::GetCurrent()->GetClassLinker()->AddPandaFile(std::move(pf));

    Sampler sp;
    ASSERT_EQ(sp.Start(), true);
#ifdef PANDA_TARGET_ARM32
    Runtime::GetCurrent()->Execute("_GLOBAL::main_short", {});
#else
    Runtime::GetCurrent()->Execute("_GLOBAL::main_long", {});
#endif

    sp.Stop();
    ASSERT_NE(COUNTER, 0);
}

}  // namespace panda::test
