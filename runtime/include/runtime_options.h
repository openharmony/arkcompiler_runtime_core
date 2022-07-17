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
#ifndef PANDA_RUNTIME_OPTIONS_H_
#define PANDA_RUNTIME_OPTIONS_H_

#include "generated/runtime_options_gen.h"
#include "utils/logger.h"
#include "runtime/plugins.h"

namespace panda {
/**
 * \brief Verification mode
 */
enum class VerificationMode {
    DISABLED,       // No verification
    ON_THE_FLY,     // Verify methods before they are executed (used by panda/ark executable)
    AHEAD_OF_TIME,  // Verify methods at startup (used by verifier executable)
    DEBUG           // Debug verification by enabling breakpoints (used by verifier executable)
};

/**
 * \brief Class represents runtime options
 *
 * It extends Options that represents public options (that described in options.yaml) and
 * adds some private options related to runtime initialization that cannot be controlled
 * via command line tools. Now they are used in unit tests to create minimal runtime for
 * testing.
 *
 * To control private options from any class/function we need make it friend for this class.
 */
class RuntimeOptions : public Options {
public:
    explicit RuntimeOptions(const std::string &exe_path = "") : Options(exe_path) {}

    bool ShouldLoadBootPandaFiles() const
    {
        return should_load_boot_panda_files_;
    }

    bool ShouldInitializeIntrinsics() const
    {
        return should_initialize_intrinsics_;
    }

    void *GetMobileLog()
    {
        return mlog_buf_print_ptr_;
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
        return verification_mode_;
    }

    void SetVerificationMode(VerificationMode in)
    {
        verification_mode_ = in;
    }

    bool IsVerifyRuntimeLibraries() const
    {
        return verify_runtime_libraries_;
    }

    void SetVerifyRuntimeLibraries(bool in)
    {
        verify_runtime_libraries_ = in;
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
        crash_connect_ = reinterpret_cast<char *>(in);
    }

    void *GetCrashConnect() const
    {
        return crash_connect_;
    }

    void SetMobileLog(void *mlog_buf_print_ptr)
    {
        mlog_buf_print_ptr_ = mlog_buf_print_ptr;
        Logger::SetMobileLogPrintEntryPointByPtr(mlog_buf_print_ptr);
    }

    void SetForSnapShotStart()
    {
        should_load_boot_panda_files_ = false;
        should_initialize_intrinsics_ = false;
    }

    void SetShouldLoadBootPandaFiles(bool value)
    {
        should_load_boot_panda_files_ = value;
    }

    void SetShouldInitializeIntrinsics(bool value)
    {
        should_initialize_intrinsics_ = value;
    }

    bool UseMallocForInternalAllocations() const
    {
        bool use_malloc = false;
        auto option = GetInternalAllocatorType();
        if (option == "default") {
#ifdef NDEBUG
            use_malloc = true;
#else
            use_malloc = false;
#endif
        } else if (option == "malloc") {
            use_malloc = true;
        } else if (option == "panda_allocators") {
            use_malloc = false;
        } else {
            UNREACHABLE();
        }
        return use_malloc;
    }

    bool IsG1TrackFreedObjects() const
    {
        bool track = false;
        auto option = GetG1TrackFreedObjects();
        if (option == "default") {
#ifdef NDEBUG
            track = true;
#else
            track = false;
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
            std::vector<std::string> load_runtimes = GetLoadRuntimes();
            std::vector<std::string> class_spaces;
            std::string runtime_type = "core";

            if (WasSetBootClassSpaces()) {
                class_spaces = GetBootClassSpaces();
                std::unique_copy(load_runtimes.begin(), load_runtimes.end(), std::back_inserter(class_spaces));
            } else {
                class_spaces = load_runtimes;
            }

            if (WasSetRuntimeType()) {
                runtime_type = GetRuntimeType();
            } else {
                // Select first non-core runtime
                for (auto &runtime : load_runtimes) {
                    if (runtime != "core") {
                        runtime_type = runtime;
                        break;
                    }
                }
            }

            SetRuntimeType(runtime_type);
            SetBootClassSpaces(class_spaces);
        } else {
            std::vector<std::string> intrinsic_spaces = GetBootIntrinsicSpaces();
            std::vector<std::string> class_spaces = GetBootClassSpaces();
            std::vector<std::string> load_runtimes;
            if (intrinsic_spaces.size() < class_spaces.size()) {
                load_runtimes = intrinsic_spaces;
            } else {
                load_runtimes = class_spaces;
            }
            SetLoadRuntimes(load_runtimes);
        }
    }

private:
    // Fix default value for possible missing plugins.
    void CheckAndFixIntrinsicSpaces()
    {
        bool intr_set = WasSetBootIntrinsicSpaces();
        std::vector<std::string> spaces = GetBootIntrinsicSpaces();
        for (auto it = spaces.begin(); it != spaces.end();) {
            if (panda::plugins::HasRuntime(*it)) {
                ++it;
                continue;
            }

            if (intr_set) {
                LOG(FATAL, RUNTIME) << "Missing runtime for intrinsic space " << *it;
            }
            it = spaces.erase(it);
        }

        SetBootIntrinsicSpaces(spaces);
    }

    bool should_load_boot_panda_files_ {true};
    bool should_initialize_intrinsics_ {true};
    void *mlog_buf_print_ptr_ {nullptr};
    std::string fingerPrint_ {"unknown"};
    void *unwindstack_ {nullptr};
    void *crash_connect_ {nullptr};
    VerificationMode verification_mode_ {VerificationMode::DISABLED};
    bool verify_runtime_libraries_ {false};
};
}  // namespace panda

#endif  // PANDA_RUNTIME_OPTIONS_H_
