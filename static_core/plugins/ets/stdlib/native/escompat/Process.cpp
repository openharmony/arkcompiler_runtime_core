/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <cmath>
#include <thread>
#include <set>

#include "libpandabase/macros.h"
#include "libpandabase/os/exec.h"
#include "libpandabase/os/filesystem.h"
#include "libpandabase/os/kill.h"
#include "libpandabase/os/pipe.h"
#include "libpandabase/os/system_environment.h"
#include "libpandabase/os/time.h"
#include "plugins/ets/stdlib/native/escompat/Process.h"

namespace ark::ets::stdlib {

enum Signals : uint32_t { SIG_INT = 2, SIG_QUIT = 3, SIG_KILL = 9, SIG_TERM = 15 };

static auto GetPipeHandler(int stdOutFdIn, int stdOutFdOut, int stdErrFdIn, int stdErrFdOut)
{
    auto handlePipes = [stdOutFdIn, stdOutFdOut, stdErrFdIn, stdErrFdOut] {
        std::pair<ark::os::unique_fd::UniqueFd, ark::os::unique_fd::UniqueFd> out {stdOutFdIn, stdOutFdOut};
        std::pair<ark::os::unique_fd::UniqueFd, ark::os::unique_fd::UniqueFd> err {stdErrFdIn, stdErrFdOut};
        out.first.Reset();
        err.first.Reset();
        ark::os::unique_fd::UniqueFd defaultStdOut {1};
        ark::os::unique_fd::UniqueFd defaultStdErr {2};
        os::Dup2(out.second, defaultStdOut);
        os::Dup2(err.second, defaultStdErr);
        out.second.Release();
        err.second.Release();
        defaultStdOut.Release();
        defaultStdErr.Release();
    };
    return handlePipes;
}

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
os::memory::Mutex g_terminatedChildSetLock {};
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static std::set<ets_double> g_terminatedChildSet GUARDED_BY(g_terminatedChildSetLock) {};

static void SpawnChildProcess(EtsEnv *env, ets_object child, ets_string cmd, ets_int timeout, ets_int signal)
{
    auto stdOutFd = os::CreatePipe();
    if (!stdOutFd.first.IsValid()) {
        env->ThrowErrorNew(env->FindClass("std/core/RuntimeException"), "Failed to create a child process");
        return;
    }
    auto stdErrFd = os::CreatePipe();
    if (!stdErrFd.first.IsValid()) {
        env->ThrowErrorNew(env->FindClass("std/core/RuntimeException"), "Failed to create a child process");
        return;
    }

    auto pipeHandler =
        GetPipeHandler(stdOutFd.first.Get(), stdOutFd.second.Get(), stdErrFd.first.Get(), stdErrFd.second.Get());

    const char *cmdString = env->GetStringUTFChars(cmd, nullptr);
    auto result = ark::os::exec::ExecWithCallbackNoWait(pipeHandler, "/bin/sh", "-c", cmdString);
    env->ReleaseStringUTFChars(cmd, cmdString);
    stdOutFd.second.Reset();
    stdErrFd.second.Reset();

    auto childKlass = env->GetObjectClass(child);
    if (result.HasValue()) {
        env->SetDoubleField(child, env->Getp_field(childKlass, "pid", "D"), result.Value());
    } else {
        stdOutFd.first.Reset();
        stdErrFd.first.Reset();
        env->ThrowErrorNew(env->FindClass("std/core/RuntimeException"), "Failed to create a child process");
        return;
    }

    env->SetDoubleField(child, env->Getp_field(childKlass, "ppid", "D"), ark::os::thread::GetPid());
    env->SetIntField(child, env->Getp_field(childKlass, "outFd", "I"), stdOutFd.first.Release());
    env->SetIntField(child, env->Getp_field(childKlass, "errorFd", "I"), stdErrFd.first.Release());

    if (timeout == 0) {
        return;
    }

    auto pidToTerminate = env->GetDoubleField(child, env->Getp_field(childKlass, "pid", "D"));
    auto terminator = [timeout, signal, pidToTerminate] {
        ark::os::thread::NativeSleep(timeout);
        os::memory::LockHolder lock {g_terminatedChildSetLock};
        if (g_terminatedChildSet.count(pidToTerminate) == 0 && kill(pidToTerminate, signal) == 0) {
            auto signalType = std::array {SIG_INT, SIG_QUIT, SIG_KILL, SIG_TERM};
            if (std::find(signalType.begin(), signalType.end(), signal) != signalType.end()) {
                g_terminatedChildSet.insert(pidToTerminate);
            }
        }
    };

    std::thread timeoutListener {terminator};
    timeoutListener.detach();
}

static void ReadFinalizer(EtsEnv *env, ets_object child, bool isStdErr)
{
    bool terminated = false;
    auto childKlass = env->GetObjectClass(child);
    if (env->GetDoubleField(child, env->Getp_field(childKlass, "exitCode", "D")) > -1) {
        terminated = true;
    } else {
        os::memory::LockHolder lock {g_terminatedChildSetLock};
        auto pidId = env->Getp_field(childKlass, "pid", "D");
        terminated = g_terminatedChildSet.count(env->GetDoubleField(child, pidId)) != 0U;
    }

    if (terminated) {
        std::string fdName = isStdErr ? "errorFd" : "outFd";
        ets_field fdId = env->Getp_field(childKlass, fdName.c_str(), "I");
        auto fd = env->GetIntField(child, fdId);
        ark::os::unique_fd::UniqueFd out {fd};
        out.Reset();
        env->SetIntField(child, fdId, -1);
    }
}

static void ReadChildProcessStdOut(EtsEnv *env, ets_object child)
{
    auto childKlass = env->GetObjectClass(child);
    ets_field outFdId = env->Getp_field(childKlass, "outFd", "I");
    auto fd = env->GetIntField(child, outFdId);
    if (fd == -1) {
        return;
    }

    auto bufferObject = env->GetObjectField(child, env->Getp_field(childKlass, "outBuffer", "Lescompat/ArrayBuffer;"));
    auto arrayBufferClass = env->GetObjectClass(bufferObject);
    auto arrayBufferDataId = env->Getp_field(arrayBufferClass, "data", "[B");
    ASSERT(arrayBufferDataId);
    while (true) {
        auto outBytesRead = env->GetIntField(child, env->Getp_field(childKlass, "outBytesRead", "I"));
        auto outBufferLength = env->GetIntField(bufferObject, env->Getp_field(arrayBufferClass, "_byteLength", "I"));
        if (outBytesRead >= outBufferLength) {
            break;
        }

        constexpr int MAX_SIZE = 1024;
        std::array<int8_t, MAX_SIZE> buffer {};
        int bytesRead = read(fd, buffer.data(), sizeof(buffer.size()) - 1);
        if (bytesRead == 0) {
            ReadFinalizer(env, child, false);
            break;
        }

        if (bytesRead + outBytesRead > outBufferLength) {
            bytesRead = outBufferLength - outBytesRead;
            ark::os::unique_fd::UniqueFd out {fd};
            out.Reset();
            env->SetIntField(child, outFdId, -1);
        }
        auto outBuffer = reinterpret_cast<ets_byteArray>(env->GetObjectField(bufferObject, arrayBufferDataId));
        env->SetByteArrayRegion(outBuffer, outBytesRead, bytesRead, buffer.data());
        env->SetIntField(child, env->Getp_field(childKlass, "outBytesRead", "I"), outBytesRead + bytesRead);
    }
}

static void ReadChildProcessStdErr(EtsEnv *env, ets_object child)
{
    auto childKlass = env->GetObjectClass(child);
    ets_field errFdId = env->Getp_field(childKlass, "errorFd", "I");
    auto fd = env->GetIntField(child, errFdId);
    if (fd == -1) {
        return;
    }

    auto bufferObject = env->GetObjectField(child, env->Getp_field(childKlass, "errBuffer", "Lescompat/ArrayBuffer;"));
    auto arrayBufferClass = env->GetObjectClass(bufferObject);
    auto arrayBufferDataId = env->Getp_field(arrayBufferClass, "data", "[B");
    ASSERT(arrayBufferDataId);
    while (true) {
        auto errBytesRead = env->GetIntField(child, env->Getp_field(childKlass, "errBytesRead", "I"));
        auto errBufferLength = env->GetIntField(bufferObject, env->Getp_field(arrayBufferClass, "_byteLength", "I"));
        if (errBytesRead >= errBufferLength) {
            break;
        }

        constexpr int MAX_SIZE = 1024;
        std::array<int8_t, MAX_SIZE> buffer {};
        int bytesRead = read(fd, buffer.data(), sizeof(buffer.size()) - 1);
        if (bytesRead == 0) {
            ReadFinalizer(env, child, true);
            break;
        }

        if (bytesRead + errBytesRead > errBufferLength) {
            bytesRead = errBufferLength - errBytesRead;

            ark::os::unique_fd::UniqueFd out {fd};
            out.Reset();
            env->SetIntField(child, errFdId, -1);
        }
        auto errBuffer = reinterpret_cast<ets_byteArray>(env->GetObjectField(bufferObject, arrayBufferDataId));

        env->SetByteArrayRegion(errBuffer, errBytesRead, bytesRead, buffer.data());
        env->SetIntField(child, env->Getp_field(childKlass, "errBytesRead", "I"), errBytesRead + bytesRead);
    }
}

static void WaitChildProcess(EtsEnv *env, ets_object child)
{
    auto childKlass = env->GetObjectClass(child);
    auto exitCodeId = env->Getp_field(childKlass, "exitCode", "D");
    auto pidId = env->Getp_field(childKlass, "pid", "D");
    auto pid = env->GetDoubleField(child, pidId);

    if (env->GetDoubleField(child, exitCodeId) < 0) {
        auto result = ark::os::exec::Wait(pid, false);
        if (result.HasValue()) {
            env->SetDoubleField(child, exitCodeId, result.Value());
        } else {
            auto exceptionClass = env->FindClass("std/core/RuntimeException");
            env->ThrowErrorNew(exceptionClass, "Wait failed");
            return;
        }
    }

    ReadChildProcessStdOut(env, child);
    ReadChildProcessStdErr(env, child);

    {
        os::memory::LockHolder lock {g_terminatedChildSetLock};
        g_terminatedChildSet.erase(pid);
    }
}

static void KillChildProcess(EtsEnv *env, ets_object child, ets_int signal)
{
    auto signalType = std::array {SIG_INT, SIG_QUIT, SIG_KILL, SIG_TERM};

    auto intSignal = static_cast<int>(signal);
    auto childKlass = env->GetObjectClass(child);
    auto pidId = env->Getp_field(childKlass, "pid", "D");
    auto pid = env->GetDoubleField(child, pidId);
    if (ark::os::kill_process::Kill(pid, intSignal) == 0) {
        if (std::find(signalType.begin(), signalType.end(), intSignal) != signalType.end()) {
            os::memory::LockHolder lock {g_terminatedChildSetLock};
            g_terminatedChildSet.insert(pid);
        }
        auto killedId = env->Getp_field(childKlass, "killed", "Z");
        env->SetBooleanField(child, killedId, 1U);
        return;
    }

    auto exceptionClass = env->FindClass("std/core/RuntimeException");
    env->ThrowErrorNew(exceptionClass, "Kill failed");
}

static void CloseChildProcess(EtsEnv *env, ets_object child)
{
    auto childKlass = env->GetObjectClass(child);
    auto exitCodeId = env->Getp_field(childKlass, "exitCode", "D");
    auto pidId = env->Getp_field(childKlass, "pid", "D");
    auto pid = env->GetDoubleField(child, pidId);

    if (env->GetDoubleField(child, exitCodeId) > -1) {
        return;
    }

    {
        os::memory::LockHolder lock {g_terminatedChildSetLock};

        if (g_terminatedChildSet.count(pid) != 0U) {
            return;
        }
    }

    auto status = ark::os::kill_process::Close(pid);
    if (LIKELY(status != -1)) {
        env->SetDoubleField(child, exitCodeId, status);
        return;
    }

    auto exceptionClass = env->FindClass("std/core/RuntimeException");
    env->ThrowErrorNew(exceptionClass, "Close failed");
}

static ets_double PManagerGetUidForName(EtsEnv *env, [[maybe_unused]] ets_object process, ets_string name)
{
    const char *str = env->GetStringUTFChars(name, nullptr);
    auto result = ark::os::system_environment::GetUidForName(str);
    env->ReleaseStringUTFChars(name, str);

    return result;
}

static ets_double PManagerGetThreadPriority([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_object process,
                                            ets_double tid)
{
    return ark::os::thread::GetPriority(tid);
}

static ets_double PManagerGetSystemConfig([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_object process,
                                          ets_double name)
{
    return ark::os::system_environment::GetSystemConfig(name);
}

static ets_string PManagerGetEnvironmentVar(EtsEnv *env, [[maybe_unused]] ets_object process, ets_string name)
{
    const char *str = env->GetStringUTFChars(name, nullptr);
    auto result = env->NewStringUTF(ark::os::system_environment::GetEnvironmentVar(std::string(str).c_str()).c_str());
    env->ReleaseStringUTFChars(name, str);

    return result;
}

static void PManagerExit([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_object process, ets_double code)
{
    std::exit(code);
}

static ets_boolean PManagerKill(EtsEnv *env, [[maybe_unused]] ets_object process, ets_double signal, ets_double pid)
{
    int integerPid = static_cast<int>(pid);
    auto ownPid = ark::os::thread::GetPid();
    if (integerPid == 0 || integerPid == -1 || integerPid == ownPid || integerPid == -ownPid) {
        auto exceptionClass = env->FindClass("std/core/IllegalArgumentException");
        env->ThrowErrorNew(exceptionClass, "Invalid pid argument");

        return 0U;
    }

    constexpr int MIN_SIGNAL_VALUE = 1;
    constexpr int MAX_SINAL_VALUE = 64;

    if (std::trunc(signal) != signal || signal < MIN_SIGNAL_VALUE || signal > MAX_SINAL_VALUE) {
        auto exceptionClass = env->FindClass("std/core/IllegalArgumentException");
        env->ThrowErrorNew(exceptionClass, "Invalid signal argument");

        return 0U;
    }

    return static_cast<ets_boolean>(ark::os::kill_process::Kill(integerPid, signal) == 0);
}

static ets_double GetTid([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_object process)
{
    return ark::os::thread::GetCurrentThreadId();
}

static ets_double GetPid([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_object process)
{
    return ark::os::thread::GetPid();
}

static ets_double GetPPid([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_object process)
{
    return ark::os::thread::GetPPid();
}

static ets_double GetUid([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_object process)
{
    return ark::os::thread::GetUid();
}

static ets_double GetEuid([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_object process)
{
    return ark::os::thread::GetEuid();
}

static ets_double GetGid([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_object process)
{
    return ark::os::thread::GetGid();
}

static ets_double GetEgid([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_object process)
{
    return ark::os::thread::GetEgid();
}

static ets_doubleArray GetGroupIDs(EtsEnv *env, [[maybe_unused]] ets_object process)
{
    auto groups = ark::os::thread::GetGroups();
    auto groupIds = std::vector<ets_double>(groups.begin(), groups.end());

    auto result = env->NewDoubleArray(groups.size());

    if (groups.empty()) {
        auto exceptionClass = env->FindClass("std/core/RuntimeException");
        env->ThrowErrorNew(exceptionClass, "Failed to get process groups");
        return result;
    }

    env->SetDoubleArrayRegion(result, 0, groups.size(), groupIds.data());

    return result;
}

static ets_boolean Is64BitProcess([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_object process)
{
    constexpr int SIZE_OF_64_BIT_PTR = 8;
    return static_cast<ets_boolean>(sizeof(char *) == SIZE_OF_64_BIT_PTR);
}

static ets_double GetProcessStartRealTime([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_object process)
{
    return ark::os::time::GetStartRealTime<std::chrono::milliseconds>();
}

static ets_double GetProcessPastCpuTime([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_object process)
{
    constexpr int PROCESS_CLOCK = 2;
    return ark::os::time::GetClockTime<std::chrono::milliseconds>(PROCESS_CLOCK);
}

static void AbortProcess([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_object process)
{
    std::abort();
}

static ets_string GetCurrentWorkingDirectory(EtsEnv *env, [[maybe_unused]] ets_object process)
{
    return env->NewStringUTF(ark::os::GetCurrentWorkingDirectory().c_str());
}

static void ChangeCurrentWorkingDirectory(EtsEnv *env, [[maybe_unused]] ets_object process, ets_string path)
{
    const char *str = env->GetStringUTFChars(path, nullptr);
    os::ChangeCurrentWorkingDirectory(str);
    env->ReleaseStringUTFChars(path, str);
}

static ets_double GetSystemUptime([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_object process)
{
    constexpr int BOOTTIME_CLOCK = 7;
    return ark::os::time::GetClockTime<std::chrono::milliseconds>(BOOTTIME_CLOCK);
}

void RegisterProcessNativeMethods(EtsEnv *env)
{
    const auto childProcessImpls =
        std::array {EtsNativeMethod {"readOutput", nullptr, reinterpret_cast<void *>(ReadChildProcessStdOut)},
                    EtsNativeMethod {"readErrorOutput", nullptr, reinterpret_cast<void *>(ReadChildProcessStdErr)},
                    EtsNativeMethod {"close", nullptr, reinterpret_cast<void *>(CloseChildProcess)},
                    EtsNativeMethod {"killImpl", nullptr, reinterpret_cast<void *>(KillChildProcess)},
                    EtsNativeMethod {"spawn", nullptr, reinterpret_cast<void *>(SpawnChildProcess)},
                    EtsNativeMethod {"waitImpl", nullptr, reinterpret_cast<void *>(WaitChildProcess)}};

    const auto processManagerImpls = std::array {
        EtsNativeMethod {"getUidForName", nullptr, reinterpret_cast<void *>(PManagerGetUidForName)},
        EtsNativeMethod {"getThreadPriority", nullptr, reinterpret_cast<void *>(PManagerGetThreadPriority)},
        EtsNativeMethod {"getSystemConfig", nullptr, reinterpret_cast<void *>(PManagerGetSystemConfig)},
        EtsNativeMethod {"getEnvironmentVar", nullptr, reinterpret_cast<void *>(PManagerGetEnvironmentVar)},
        EtsNativeMethod {"exit", nullptr, reinterpret_cast<void *>(PManagerExit)},
        EtsNativeMethod {"kill", nullptr, reinterpret_cast<void *>(PManagerKill)},
    };

    const auto processImpls =
        std::array {EtsNativeMethod {"getTidImpl", nullptr, reinterpret_cast<void *>(GetTid)},
                    EtsNativeMethod {"getPidImpl", nullptr, reinterpret_cast<void *>(GetPid)},
                    EtsNativeMethod {"getPpidImpl", nullptr, reinterpret_cast<void *>(GetPPid)},
                    EtsNativeMethod {"getUidImpl", nullptr, reinterpret_cast<void *>(GetUid)},
                    EtsNativeMethod {"getEuidImpl", nullptr, reinterpret_cast<void *>(GetEuid)},
                    EtsNativeMethod {"getGidImpl", nullptr, reinterpret_cast<void *>(GetGid)},
                    EtsNativeMethod {"getEgidImpl", nullptr, reinterpret_cast<void *>(GetEgid)},
                    EtsNativeMethod {"getGroupsImpl", nullptr, reinterpret_cast<void *>(GetGroupIDs)},
                    EtsNativeMethod {"is64Bit", nullptr, reinterpret_cast<void *>(Is64BitProcess)},
                    EtsNativeMethod {"getStartRealtime", nullptr, reinterpret_cast<void *>(GetProcessStartRealTime)},
                    EtsNativeMethod {"getPastCpuTime", nullptr, reinterpret_cast<void *>(GetProcessPastCpuTime)},
                    EtsNativeMethod {"abort", nullptr, reinterpret_cast<void *>(AbortProcess)},
                    EtsNativeMethod {"cwd", nullptr, reinterpret_cast<void *>(GetCurrentWorkingDirectory)},
                    EtsNativeMethod {"chdir", nullptr, reinterpret_cast<void *>(ChangeCurrentWorkingDirectory)},
                    EtsNativeMethod {"uptime", nullptr, reinterpret_cast<void *>(GetSystemUptime)}};

    auto childProcessKlass = env->FindClass("escompat/ChildProcess");
    env->RegisterNatives(childProcessKlass, childProcessImpls.data(), childProcessImpls.size());

    auto processManagerKlass = env->FindClass("escompat/ProcessManager");
    env->RegisterNatives(processManagerKlass, processManagerImpls.data(), processManagerImpls.size());

    auto processKlass = env->FindClass("escompat/process");
    env->RegisterNatives(processKlass, processImpls.data(), processImpls.size());
}

}  // namespace ark::ets::stdlib
