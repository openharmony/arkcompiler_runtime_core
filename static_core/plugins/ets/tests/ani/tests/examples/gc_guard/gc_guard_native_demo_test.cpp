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

#include <cstdlib>
#include <iostream>
#include <string>

#ifdef PANDA_TARGET_UNIX
#include <sys/wait.h>
#include <unistd.h>

#include <array>
#endif

#include "ani.h"
#include <gtest/gtest.h>

namespace ark::ets::ani::testing {
namespace {

constexpr const char *MODULE_NAME = "gc_guard_native_demo";
constexpr const char *FUNCTION_NAME = "callNativeFunc";
constexpr size_t CHILD_OUTPUT_BUFFER_SIZE = 256U;

[[noreturn]] void Fail(const char *message)
{
    std::cerr << message << std::endl;
    std::abort();
}

void InvokeNativeFunction()
{
    const char *stdlib = std::getenv("ARK_ETS_STDLIB_PATH");
    const char *testAbc = std::getenv("ANI_GTEST_ABC_PATH");
    if (stdlib == nullptr || testAbc == nullptr) {
        Fail("ANI test bytecode paths are not configured");
    }

    const std::string bootFiles = "--ext:boot-panda-files=" + std::string(stdlib) + ":" + testAbc;
    const ani_option bootFileOption = {bootFiles.c_str(), nullptr};
    const ani_options options = {1U, &bootFileOption};

    ani_vm *vm = nullptr;
    if (ANI_CreateVM(&options, ANI_VERSION_1, &vm) != ANI_OK) {
        Fail("Cannot create ANI VM");
    }

    ani_env *env = nullptr;
    if (vm->GetEnv(ANI_VERSION_1, &env) != ANI_OK) {
        Fail("Cannot obtain ANI environment");
    }

    ani_module module = nullptr;
    if (env->FindModule(MODULE_NAME, &module) != ANI_OK) {
        Fail("Cannot find GC guard demo ETS module");
    }

    ani_function function = nullptr;
    if (env->Module_FindFunction(module, FUNCTION_NAME, nullptr, &function) != ANI_OK) {
        Fail("Cannot find GC guard demo ETS function");
    }

    // The native function must not return: it enters GC while GCGuard is alive.
    if (env->Function_Call_Void(function) != ANI_OK) {
        Fail("Native GC guard demo did not enter the expected FATAL path");
    }
    Fail("Native GC guard demo unexpectedly returned");
}

}  // namespace

TEST(GcGuardNativeDemoTest, EtsNativeFunctionAbortsWhenGcRunsInsideGuard)
{
#ifdef PANDA_TARGET_UNIX
    // Do not use EXPECT_DEATH here. The test runner forces the "threadsafe"
    // death-test style, which re-executes the target binary directly. That
    // fails with ENOEXEC when an ARM target is run through qemu-user.
    int pipeFd[2];
    ASSERT_EQ(pipe(pipeFd), 0);

    pid_t pid = fork();
    ASSERT_NE(pid, -1);
    if (pid == 0) {
        if (close(pipeFd[0]) != 0 || dup2(pipeFd[1], STDERR_FILENO) == -1) {
            _exit(EXIT_FAILURE);
        }
        (void)close(pipeFd[1]);

        // Create the VM only in the child so the parent never forks a running VM.
        InvokeNativeFunction();
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
    ASSERT_TRUE(WIFSIGNALED(status)) << "Native GC guard demo did not abort: " << childOutput;
    EXPECT_EQ(WTERMSIG(status), SIGABRT);
    EXPECT_NE(childOutput.find("Executing garbage collection inside a DISALLOW_GARBAGE_COLLECTION"), std::string::npos)
        << "Unexpected child output: " << childOutput;
#else
    EXPECT_DEATH(InvokeNativeFunction(), "Executing garbage collection inside a DISALLOW_GARBAGE_COLLECTION");
#endif
}

}  // namespace ark::ets::ani::testing
