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
#include "verification/config/options/method_options_config.h"

#include <string>
#include <unordered_map>

namespace panda::verifier {

struct VerificationOptions {
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    std::string config_file = "default";
    VerificationMode mode = VerificationMode::DISABLED;
    struct {
        bool status = false;
    } show;
    bool verify_runtime_libraries = false;
    bool sync_on_class_initialization = false;
    size_t verification_threads = 1;
    struct {
        std::string file;
        bool update_on_exit = false;
    } cache;
    struct {
        struct {
            bool reg_changes = false;
            bool context = false;
            bool type_system = false;
        } show;
        struct {
            bool undefined_class = false;
            bool undefined_method = false;
            bool undefined_field = false;
            bool undefined_type = false;
            bool undefined_string = false;
            bool method_access_violation = false;
            bool error_in_exception_handler = false;
            bool permanent_runtime_exception = false;
            bool field_access_violation = false;
            bool wrong_subclassing_in_method_args = false;
        } allow;
        MethodOptionsConfig *method_options = nullptr;
        MethodOptionsConfig &GetMethodOptions()
        {
            return *method_options;
        }
        const MethodOptionsConfig &GetMethodOptions() const
        {
            return *method_options;
        }
    } debug;
    // NOLINTEND(misc-non-private-member-variables-in-classes)

    void Initialize();
    void Destroy();

    bool IsEnabled() const
    {
        return mode != VerificationMode::DISABLED;
    }

    bool IsOnlyVerify() const
    {
        return mode == VerificationMode::AHEAD_OF_TIME || mode == VerificationMode::DEBUG;
    }
};

}  // namespace panda::verifier

#endif  // !PANDA_VERIFICATION_OPTIONS_H__
