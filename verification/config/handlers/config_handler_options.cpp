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

#include "verification/config/debug_breakpoint/breakpoint_private.h"
#include "verification/config/process/config_process.h"
#include "verification/util/parser/parser.h"
#include "verification/util/struct_field.h"

#include "literal_parser.h"

#include "verifier_messages.h"

#include "runtime/include/method.h"
#include "runtime/include/runtime.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"

#include "utils/logger.h"

#include <cstring>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace panda::verifier::debug {

namespace {

template <typename M>
PandaString GetKeys(const M &map)
{
    PandaString keys;
    for (const auto &p : map) {
        if (keys.empty()) {
            keys += "'";
            keys += p.first;
            keys += "'";
        } else {
            keys += ", '";
            keys += p.first;
            keys += "'";
        }
    }
    return keys;
}

}  // namespace

using panda::verifier::config::Section;

void RegisterConfigHandlerOptions()
{
    static const auto CONFIG_DEBUG_OPTIONS_VERIFIER = [](const Section &section) {
        using bool_field = struct_field<VerificationOptions, bool>;
        using flags = PandaUnorderedMap<PandaString, bool_field>;
        using flags_section = PandaUnorderedMap<PandaString, flags>;

        const flags_section FLAGS = {
            {"show",
             {{"context", bool_field {offsetof(VerificationOptions, Debug.Show.Context)}},
              {"reg-changes", bool_field {offsetof(VerificationOptions, Debug.Show.RegChanges)}},
              {"typesystem", bool_field {offsetof(VerificationOptions, Debug.Show.TypeSystem)}},
              {"status", bool_field {offsetof(VerificationOptions, Show.Status)}}}},
            {"allow",
             {{"undefined-class", bool_field {offsetof(VerificationOptions, Debug.Allow.UndefinedClass)}},
              {"undefined-method", bool_field {offsetof(VerificationOptions, Debug.Allow.UndefinedMethod)}},
              {"undefined-field", bool_field {offsetof(VerificationOptions, Debug.Allow.UndefinedField)}},
              {"undefined-type", bool_field {offsetof(VerificationOptions, Debug.Allow.UndefinedType)}},
              {"undefined-string", bool_field {offsetof(VerificationOptions, Debug.Allow.UndefinedString)}},
              {"method-access-violation",
               bool_field {offsetof(VerificationOptions, Debug.Allow.MethodAccessViolation)}},
              {"field-access-violation", bool_field {offsetof(VerificationOptions, Debug.Allow.FieldAccessViolation)}},
              {"wrong-subclassing-in-method-args",
               bool_field {offsetof(VerificationOptions, Debug.Allow.WrongSubclassingInMethodArgs)}},
              {"error-in-exception-handler",
               bool_field {offsetof(VerificationOptions, Debug.Allow.ErrorInExceptionHandler)}},
              {"permanent-runtime-exception",
               bool_field {offsetof(VerificationOptions, Debug.Allow.PermanentRuntimeException)}}}},
            {"cflow",
             {{"body-to-handler", bool_field {offsetof(VerificationOptions, Cflow.AllowJmpBodyToHandler)}},
              {"body-into-handler", bool_field {offsetof(VerificationOptions, Cflow.AllowJmpBodyIntoHandler)}},
              {"handler-to-handler", bool_field {offsetof(VerificationOptions, Cflow.AllowJmpHandlerToHandler)}},
              {"handler-into-handler", bool_field {offsetof(VerificationOptions, Cflow.AllowJmpHandlerIntoHandler)}},
              {"handler-into-body", bool_field {offsetof(VerificationOptions, Cflow.AllowJmpHandlerIntoBody)}}}}};

        auto &verif_opts = Runtime::GetCurrent()->GetVerificationOptions();
        for (const auto &s : section.sections) {
            if (FLAGS.count(s.name) == 0) {
                LOG_VERIFIER_DEBUG_CONFIG_WRONG_OPTIONS_SECTION(s.name, GetKeys(FLAGS));
                return false;
            }
            const auto &section_flags = FLAGS.at(s.name);
            for (const auto &i : s.items) {
                PandaVector<PandaString> c;
                const char *start = i.c_str();
                const char *end = i.c_str() + i.length();  // NOLINT
                if (!LiteralsParser()(c, start, end)) {
                    LOG_VERIFIER_DEBUG_CONFIG_WRONG_OPTIONS_LINE(i);
                    return false;
                }
                if (!c.empty()) {
                    for (const auto &l : c) {
                        if (section_flags.count(l) == 0) {
                            LOG_VERIFIER_DEBUG_CONFIG_WRONG_OPTION_FOR_SECTION(l, s.name, GetKeys(section_flags));
                            return false;
                        }
                        section_flags.at(l).of(verif_opts) = true;
                        LOG_VERIFIER_DEBUG_CONFIG_OPTION_IS_ACTIVE_INFO(s.name, l);
                    }
                }
            }
        }
        return true;
    };

    config::RegisterConfigHandler("config.debug.options.verifier", CONFIG_DEBUG_OPTIONS_VERIFIER);
}

}  // namespace panda::verifier::debug
