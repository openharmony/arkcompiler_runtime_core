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

#ifndef PANDA_VERIFICATION_OPTIONS_H__
#define PANDA_VERIFICATION_OPTIONS_H__

#include "utils/pandargs.h"
#include "runtime/include/runtime_options.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"
#include "verification/cflow/cflow_check_options.h"
#include "verification/config/options/method_options_config.h"

#include <string>
#include <unordered_map>

namespace panda::verifier {

struct VerificationOptions {
    std::string ConfigFile = "default";
    VerificationMode Mode = VerificationMode::DISABLED;
    struct {
        bool Status = false;
    } Show;
    CflowOptions Cflow;
    bool VerifyRuntimeLibraries = false;
    bool SyncOnClassInitialization = false;
    size_t VerificationThreads = 1;
    struct {
        std::string File;
        bool UpdateOnExit = false;
    } Cache;
    struct {
        struct {
            bool RegChanges = false;
            bool Context = false;
            bool TypeSystem = false;
        } Show;
        struct {
            bool UndefinedClass = false;
            bool UndefinedMethod = false;
            bool UndefinedField = false;
            bool UndefinedType = false;
            bool UndefinedString = false;
            bool MethodAccessViolation = false;
            bool ErrorInExceptionHandler = false;
            bool PermanentRuntimeException = false;
            bool FieldAccessViolation = false;
            bool WrongSubclassingInMethodArgs = false;
        } Allow;
        MethodOptionsConfig *MethodOptions = nullptr;
        MethodOptionsConfig &GetMethodOptions()
        {
            return *MethodOptions;
        }
        const MethodOptionsConfig &GetMethodOptions() const
        {
            return *MethodOptions;
        }
    } Debug;
    void Initialize(const panda::RuntimeOptions &runtime_options);
    void Destroy();

    bool IsEnabled() const
    {
        return Mode != VerificationMode::DISABLED;
    }

    bool IsOnlyVerify() const
    {
        return Mode == VerificationMode::AHEAD_OF_TIME || Mode == VerificationMode::DEBUG;
    }
};

}  // namespace panda::verifier

#endif  // !PANDA_VERIFICATION_OPTIONS_H__
