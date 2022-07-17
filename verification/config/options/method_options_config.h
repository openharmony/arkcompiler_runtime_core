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

#ifndef PANDA_VERIFIER_DEBUG_OPTIONS_METHOD_OPTIONS_CONFIG_H_
#define PANDA_VERIFIER_DEBUG_OPTIONS_METHOD_OPTIONS_CONFIG_H_

#include "method_options.h"

#include <regex>

namespace panda::verifier {

class MethodOptionsConfig {
public:
    MethodOptions &NewOptions(const PandaString &name)
    {
        return Config.emplace(name, name).first->second;
    }
    const MethodOptions &GetOptions(const PandaString &name) const
    {
        ASSERT(IsOptionsPresent(name));
        return Config.at(name);
    }
    bool IsOptionsPresent(const PandaString &name) const
    {
        return Config.count(name) > 0;
    }
    const MethodOptions &operator[](const PandaString &method_name) const
    {
        for (const auto &g : MethodGroups) {
            const auto &regex = g.first;
            if (std::regex_match(method_name, regex)) {
                return g.second;
            }
        }
        return GetOptions("default");
    }
    bool AddOptionsForGroup(const PandaString &group_regex, const PandaString &options_name)
    {
        if (!IsOptionsPresent(options_name)) {
            return false;
        }
        MethodGroups.emplace_back(
            std::regex {group_regex, std::regex_constants::basic | std::regex_constants::optimize |
                                         std::regex_constants::nosubs | std::regex_constants::icase},
            GetOptions(options_name));
        return true;
    }

private:
    PandaUnorderedMap<PandaString, MethodOptions> Config;
    PandaVector<std::pair<std::regex, const MethodOptions &>> MethodGroups;
};

}  // namespace panda::verifier

#endif  //! PANDA_VERIFIER_DEBUG_OPTIONS_METHOD_OPTIONS_CONFIG_H_
