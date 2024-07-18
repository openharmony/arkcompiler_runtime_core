/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_OPTIONS_H_
#define PANDA_RUNTIME_OPTIONS_H_

#include "generated/runtime_options_gen.h"
#include "utils/logger.h"
#include "runtime/plugins.h"

namespace ark {
/// @brief Verification mode
enum class VerificationMode {
    DISABLED,       // No verification
    ON_THE_FLY,     // Verify methods before they are executed (used by panda/ark executable)
    AHEAD_OF_TIME,  // Verify methods at startup (used by verifier executable)
    DEBUG           // Debug verification by enabling breakpoints (used by verifier executable)
};

static inline bool IsEnabled(VerificationMode mode)
{
    return mode != VerificationMode::DISABLED;
}

static inline VerificationMode VerificationModeFromString(const std::string &mode)
{
    if (mode == "on-the-fly") {
        return VerificationMode::ON_THE_FLY;
    }
    if (mode == "ahead-of-time") {
        return VerificationMode::AHEAD_OF_TIME;
    }
    if (mode == "debug") {
        return VerificationMode::DEBUG;
    }
    return VerificationMode::DISABLED;
}

/**
 * @brief Class represents runtime options
 *
 * It extends Options that represents public options (that described in options.yaml) and
 * adds some private options related to runtime initialization that cannot be controlled
 * via command line tools. Now they are used in unit tests to create minimal runtime for
 * testing.
 *
 * To control private options from any class/function we need make it friend for this class.
 */
class PANDA_PUBLIC_API RuntimeOptions : public Options {
public:
    explicit RuntimeOptions(const std::string &exePath = "") : Options(exePath) {}

    bool ShouldLoadBootPandaFiles() const
    {
        return shouldLoadBootPandaFiles_;
    }

    bool ShouldInitializeIntrinsics() const
    {
        return shouldInitializeIntrinsics_;
    }

    void *GetMobileLog()
    {
        return mlogBufPrintPtr_;
    }

    const std::string &GetFingerprint() const
    {
        return fingerPrint_;
    }

    void SetFingerprint(const std::string &in)
    {
        fingerPrint_.assign(in);
    }

    VerificationMode GetVerificationMode() const
    {
        return verificationMode_;
    }

    void SetVerificationMode(VerificationMode in)
    {
        verificationMode_ = in;
    }

    bool IsVerifyRuntimeLibraries() const
    {
        return verifyRuntimeLibraries_;
    }

    void SetVerifyRuntimeLibraries(bool in)
    {
        verifyRuntimeLibraries_ = in;
    }

    void SetUnwindStack(void *in)
    {
        unwindstack_ = reinterpret_cast<char *>(in);
    }

    void *GetUnwindStack() const
    {
        return unwindstack_;
    }

    void SetCrashConnect(void *in)
    {
        crashConnect_ = reinterpret_cast<char *>(in);
    }

    void *GetCrashConnect() const
    {
        return crashConnect_;
    }

    void SetMobileLog(void *mlogBufPrintPtr)
    {
        mlogBufPrintPtr_ = mlogBufPrintPtr;
        Logger::SetMobileLogPrintEntryPointByPtr(mlogBufPrintPtr);
    }

    void SetForSnapShotStart()
    {
        shouldLoadBootPandaFiles_ = false;
        shouldInitializeIntrinsics_ = false;
    }

    void SetShouldLoadBootPandaFiles(bool value)
    {
        shouldLoadBootPandaFiles_ = value;
    }

    void SetShouldInitializeIntrinsics(bool value)
    {
        shouldInitializeIntrinsics_ = value;
    }

    bool UseMallocForInternalAllocations() const
    {
        bool useMalloc = false;
        auto option = GetInternalAllocatorType();
        if (option == "default") {
#ifdef NDEBUG
            useMalloc = true;
#else
            useMalloc = false;
#endif
        } else if (option == "malloc") {
            useMalloc = true;
        } else if (option == "panda_allocators") {
            useMalloc = false;
        } else {
            UNREACHABLE();
        }
        return useMalloc;
    }

    bool IsG1TrackFreedObjects() const
    {
        bool track = false;
        auto option = GetG1TrackFreedObjects();
        if (option == "default") {
#ifdef NDEBUG
            track = false;
#else
            track = true;
#endif
        } else if (option == "true") {
            track = true;
        } else if (option == "false") {
            track = false;
        } else {
            UNREACHABLE();
        }
        return track;
    }

    void InitializeRuntimeSpacesAndType()
    {
        CheckAndFixIntrinsicSpaces();
        if (WasSetLoadRuntimes()) {
            std::vector<std::string> loadRuntimes = GetLoadRuntimes();
            std::string runtimeType = "core";

            // Select first non-core runtime
            for (auto &runtime : loadRuntimes) {
                if (runtime != "core") {
                    runtimeType = runtime;
                    break;
                }
            }

            SetRuntimeType(runtimeType);
            SetBootClassSpaces(loadRuntimes);
            SetBootIntrinsicSpaces(loadRuntimes);
        }
    }

private:
    // Fix default value for possible missing plugins.
    void CheckAndFixIntrinsicSpaces()
    {
        bool intrSet = WasSetBootIntrinsicSpaces();
        std::vector<std::string> spaces = GetBootIntrinsicSpaces();
        for (auto it = spaces.begin(); it != spaces.end();) {
            if (ark::plugins::HasRuntime(*it)) {
                ++it;
                continue;
            }

            if (intrSet) {
                LOG(FATAL, RUNTIME) << "Missing runtime for intrinsic space " << *it;
            }
            it = spaces.erase(it);
        }

        SetBootIntrinsicSpaces(spaces);
    }

    bool shouldLoadBootPandaFiles_ {true};
    bool shouldInitializeIntrinsics_ {true};
    void *mlogBufPrintPtr_ {nullptr};
    std::string fingerPrint_ {"unknown"};
    void *unwindstack_ {nullptr};
    void *crashConnect_ {nullptr};
    VerificationMode verificationMode_ {VerificationMode::DISABLED};
    bool verifyRuntimeLibraries_ {false};
};
}  // namespace ark

#endif  // PANDA_RUNTIME_OPTIONS_H_
