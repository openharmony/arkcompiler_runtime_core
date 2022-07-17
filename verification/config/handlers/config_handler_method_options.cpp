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

#include "verification/config/config.h"
#include "verification/config/default/default_config.h"
#include "verification/config/process/config_process.h"
#include "verification/config/parse/config_parse.h"
#include "verification/config/options/msg_set_parser.h"

#include "verifier_messages.h"

#include "literal_parser.h"

#include "runtime/include/method.h"
#include "runtime/include/runtime.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/mem/panda_containers.h"

#include "utils/logger.h"

namespace panda::verifier::debug {

using panda::verifier::config::Section;

namespace {

using Literals = PandaVector<PandaString>;

bool ProcessSectionMsg(MethodOption::MsgClass msg_class, const PandaString &items, MethodOptions *options)
{
    MessageSetContext c;

    const char *start = items.c_str();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if (!MessageSetParser()(c, start, items.c_str() + items.length())) {
        LOG(ERROR, VERIFIER) << "Unexpected set of messages: '" << items << "'";
        return false;
    }

    for (const auto msg_num : c.nums) {
        options->SetMsgClass(VerifierMessageIsValid, msg_num, msg_class);
    }

    return true;
}

bool ProcessSectionShow(const Literals &literals, MethodOptions *options)
{
    for (const auto &option : literals) {
        if (option == "context") {
            options->SetShow(MethodOption::InfoType::CONTEXT);
        } else if (option == "reg-changes") {
            options->SetShow(MethodOption::InfoType::REG_CHANGES);
        } else if (option == "cflow") {
            options->SetShow(MethodOption::InfoType::CFLOW);
        } else if (option == "jobfill") {
            options->SetShow(MethodOption::InfoType::JOBFILL);
        } else {
            LOG(ERROR, VERIFIER) << "Unexpected show option: '" << option << "'";
            return false;
        }
    }

    return true;
}

bool ProcessSectionUplevel(const Literals &uplevel_options, const MethodOptionsConfig &all_method_options,
                           MethodOptions *options)
{
    for (const auto &uplevel : uplevel_options) {
        if (!all_method_options.IsOptionsPresent(uplevel)) {
            LOG(ERROR, VERIFIER) << "Cannot find uplevel options: '" << uplevel << "'";
            return false;
        }
        options->AddUpLevel(all_method_options.GetOptions(uplevel));
    }

    return true;
}

bool ProcessSectionCheck(const Literals &checks, MethodOptions *options)
{
    auto &options_check = options->Check();
    for (const auto &c : checks) {
        if (c == "cflow") {
            options_check |= MethodOption::CheckType::CFLOW;
        } else if (c == "reg-usage") {
            options_check |= MethodOption::CheckType::REG_USAGE;
        } else if (c == "resolve-id") {
            options_check |= MethodOption::CheckType::RESOLVE_ID;
        } else if (c == "typing") {
            options_check |= MethodOption::CheckType::TYPING;
        } else if (c == "absint") {
            options_check |= MethodOption::CheckType::ABSINT;
        } else {
            LOG(ERROR, VERIFIER) << "Unexpected check type: '" << c << "'";
            return false;
        }
    }

    return true;
}

}  // namespace

const auto &MethodOptionsProcessor()
{
    static const auto PROCESS_METHOD_OPTIONS = [](const Section &section) {
        MethodOptionsConfig &all_options = Runtime::GetCurrent()->GetVerificationOptions().Debug.GetMethodOptions();
        MethodOptions &options = all_options.NewOptions(section.name);

        for (const auto &s : section.sections) {
            const PandaString &name = s.name;
            PandaString items;
            for (const auto &item : s.items) {
                items += item;
                items += " ";
            }

            if (name == "error") {
                if (!ProcessSectionMsg(MethodOption::MsgClass::ERROR, items, &options)) {
                    return false;
                }
            } else if (name == "warning") {
                if (!ProcessSectionMsg(MethodOption::MsgClass::WARNING, items, &options)) {
                    return false;
                }
            } else if (name == "hidden") {
                if (!ProcessSectionMsg(MethodOption::MsgClass::HIDDEN, items, &options)) {
                    return false;
                }
            } else {
                Literals literals;
                const char *start = items.c_str();
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                if (!LiteralsParser()(literals, start, start + items.length())) {
                    LOG(ERROR, VERIFIER) << "Failed to parse '" << name << "' under '" << section.name << "'";
                    return false;
                }

                if (name == "show") {
                    if (!ProcessSectionShow(literals, &options)) {
                        return false;
                    }
                } else if (name == "uplevel") {
                    if (!ProcessSectionUplevel(literals, all_options, &options)) {
                        return false;
                    }
                } else if (name == "check") {
                    if (!ProcessSectionCheck(literals, &options)) {
                        return false;
                    }
                } else {
                    LOG(ERROR, VERIFIER) << "Unexpected section name: '" << name << "' under '" << section.name << "'";
                    return false;
                }
            }
        }

        LOG(DEBUG, VERIFIER) << options.Image();

        return true;
    };

    return PROCESS_METHOD_OPTIONS;
}

void RegisterConfigHandlerMethodOptions()
{
    static const auto CONFIG_DEBUG_METHOD_OPTIONS_VERIFIER = [](const Section &section) {
        bool default_present = false;
        for (const auto &s : section.sections) {
            if (s.name == "default") {
                default_present = true;
                break;
            }
        }
        if (!default_present) {
            // take default section from inlined config
            Section cfg;
            if (!ParseConfig(panda::verifier::config::VerifierDebugDefaultConfig, cfg)) {
                LOG(ERROR, VERIFIER) << "Cannot parse default verifier config";
                return false;
            }
            if (!MethodOptionsProcessor()(cfg["debug"]["method_options"]["verifier"]["default"])) {
                LOG(ERROR, VERIFIER) << "Cannot parse default method options";
                return false;
            }
        }
        for (const auto &s : section.sections) {
            if (!MethodOptionsProcessor()(s)) {
                LOG(ERROR, VERIFIER) << "Cannot parse section '" << s.name << "'";
                return false;
            }
        }
        return true;
    };

    config::RegisterConfigHandler("config.debug.method_options.verifier", CONFIG_DEBUG_METHOD_OPTIONS_VERIFIER);
}

void SetDefaultMethodOptions()
{
    auto &runtime = *Runtime::GetCurrent();
    auto &verif_options = runtime.GetVerificationOptions();
    auto &options = verif_options.Debug.GetMethodOptions();
    if (!options.IsOptionsPresent("default")) {
        // take default section from inlined config
        Section cfg;
        if (!ParseConfig(panda::verifier::config::VerifierDebugDefaultConfig, cfg)) {
            LOG(FATAL, VERIFIER) << "Cannot parse default internal config. Internal error.";
        }
        if (!MethodOptionsProcessor()(cfg["debug"]["method_options"]["verifier"]["default"])) {
            LOG(FATAL, VERIFIER) << "Cannot parse default section";
        }
    }
}

}  // namespace panda::verifier::debug
