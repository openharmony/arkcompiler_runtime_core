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

#include <gtest/gtest.h>
#include <string>

#include "runtime/assert_gc_scope.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_options.h"
#include "runtime/tests/test_utils.h"

#ifdef PANDA_TARGET_UNIX
#include <sys/wait.h>
#include <unistd.h>

#include <array>
#endif

namespace ark::mem {

constexpr size_t CHILD_OUTPUT_BUFFER_SIZE = 256U;

// The GC guard is a developer-facing RAII interface: instantiating AssertGCScopeT (or using the
// DISALLOW_GARBAGE_COLLECTION macro) marks the current code region as "expected not to run GC".
// While such an object is alive, running a garbage collection aborts the process (LOG(FATAL)).
// It works in any build (debug or release) - there is no enable switch, the "switch" is simply
// whether the developer instantiates the guard or not.

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,-warnings-as-errors)
class AssertGCScopeTest : public testing::Test {
public:
    AssertGCScopeTest() = default;

    ~AssertGCScopeTest() override
    {
        // Only tear down if a Runtime was created on this (parent) process. The death-test case
        // creates its Runtime inside the EXPECT_DEATH child, so there is none to destroy here.
        if (Runtime::GetCurrent() != nullptr) {
            [[maybe_unused]] bool success = Runtime::Destroy();
            ASSERT(success);
        }
        Logger::Destroy();
    }

    NO_COPY_SEMANTIC(AssertGCScopeTest);
    NO_MOVE_SEMANTIC(AssertGCScopeTest);

    void SetupRuntime() const
    {
        ark::Logger::ComponentMask componentMask;
        componentMask.set(Logger::Component::GC);
        Logger::InitializeStdLogging(Logger::Level::FATAL, componentMask);

        RuntimeOptions options;
        options.SetLoadRuntimes({"core"});
        options.SetGcType("g1-gc");
        options.SetRunGcInPlace(true);
        options.SetCompilerEnableJit(false);
        options.SetGcWorkersCount(0);
        options.SetGcTriggerType("debug-never");
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        [[maybe_unused]] bool success = Runtime::Create(options);
        ASSERT(success);
    }
};

/// @brief Outside a guarded region GC is allowed: no guard, no effect on the normal path.
TEST_F(AssertGCScopeTest, AllowedOutsideGuard)
{
    SetupRuntime();
    EXPECT_TRUE(ark::AssertGCScopeT::IsAllowed());
}

/// @brief Inside a guarded region(RAII object alive) GC is not allowed.* /
TEST_F(AssertGCScopeTest, NotAllowedInsideGuard)
{
    SetupRuntime();
    ASSERT_TRUE(ark::AssertGCScopeT::IsAllowed());
    {
        AssertGCScopeT gcScope;
        EXPECT_FALSE(ark::AssertGCScopeT::IsAllowed());
    }
    // Back outside -> allowed again.
    EXPECT_TRUE(ark::AssertGCScopeT::IsAllowed());
}

/// @brief The DISALLOW_GARBAGE_COLLECTION macro must behave identically to the RAII class.
TEST_F(AssertGCScopeTest, MacroEquivalentClass)
{
    SetupRuntime();
    {
        DISALLOW_GARBAGE_COLLECTION;
        EXPECT_FALSE(ark::AssertGCScopeT::IsAllowed());
    }
    EXPECT_TRUE(ark::AssertGCScopeT::IsAllowed());
    // The legacy misspelled macro is an alias and must also work.
    {
        DISALLOW_GARBAGE_COLLECTION;
        EXPECT_FALSE(ark::AssertGCScopeT::IsAllowed());
    }
}

/// @brief Nested guards are supported; the relief happens only when the outermost guard exits.
TEST_F(AssertGCScopeTest, NestedGuards)
{
    SetupRuntime();
    {
        AssertGCScopeT outer;
        EXPECT_FALSE(ark::AssertGCScopeT::IsAllowed());
        {
            AssertGCScopeT inner;
            EXPECT_FALSE(ark::AssertGCScopeT::IsAllowed());
        }
        EXPECT_FALSE(ark::AssertGCScopeT::IsAllowed());
    }
    EXPECT_TRUE(ark::AssertGCScopeT::IsAllowed());
}

/// @brief The check used at the GC entry point aborts while a guard is alive.
TEST_F(AssertGCScopeTest, GcInsideGuardAborts)
{
#ifdef PANDA_TARGET_UNIX
    // The abort is verified with a plain fork() instead of EXPECT_DEATH: the
    // "threadsafe" death-test mode re-executes the test binary via execv(),
    // which fails with "Exec format error" in qemu-user builds, so the checked
    // statement would never run there. A plain fork() works in every supported
    // environment.
    int pipeFd[2];
    ASSERT_EQ(pipe(pipeFd), 0);

    pid_t pid = fork();
    ASSERT_NE(pid, -1);
    if (pid == 0) {
        // Child: a GC entered inside the guarded region must abort the process
        // with a FATAL error. All logging output is redirected into the pipe.
        if (close(pipeFd[0]) != 0 || dup2(pipeFd[1], STDERR_FILENO) == -1) {
            _exit(EXIT_FAILURE);
        }
        (void)close(pipeFd[1]);
        ark::Logger::ComponentMask gcMask;
        gcMask.set(Logger::Component::GC);
        Logger::InitializeStdLogging(Logger::Level::FATAL, gcMask);
        DISALLOW_GARBAGE_COLLECTION;
        DCHECK_ALLOW_GARBAGE_COLLECTION;
        // The check above must abort the child, reaching this point is a failure.
        _exit(EXIT_FAILURE);
    }
    ASSERT_EQ(close(pipeFd[1]), 0);

    std::string childOutput;
    std::array<char, CHILD_OUTPUT_BUFFER_SIZE> buffer {};
    ssize_t bytesRead;
    do {
        bytesRead = read(pipeFd[0], buffer.data(), buffer.size());
        if (bytesRead > 0) {
            childOutput.append(buffer.data(), static_cast<size_t>(bytesRead));
        }
    } while (bytesRead > 0);
    ASSERT_EQ(close(pipeFd[0]), 0);

    int status = 0;
    ASSERT_EQ(waitpid(pid, &status, 0), pid);
    ASSERT_TRUE(WIFSIGNALED(status)) << "The GC guard check did not abort the process";
    EXPECT_EQ(WTERMSIG(status), SIGABRT);
    EXPECT_TRUE(childOutput.find("Executing garbage collection inside a DISALLOW_GARBAGE_COLLECTION") !=
                std::string::npos)
        << "Unexpected child output: " << childOutput;
#endif  // PANDA_TARGET_UNIX
}

}  // namespace ark::mem
