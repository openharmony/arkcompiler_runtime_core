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

#include "config_process.h"

#include "verification/public_internal.h"
#include "verification/config/context/context.h"

#include "runtime/include/mem/panda_string.h"

#include <unordered_map>

namespace {

using panda::verifier::Config;
using panda::verifier::config::Section;

bool ProcessConfigSection(Config *cfg, const Section &section, const panda::PandaString &path = "")
{
    auto &sectionHandlers = cfg->debugCfg.sectionHandlers;
    if (sectionHandlers.count(path) > 0) {
        return sectionHandlers.at(path)(cfg, section);
    }
    for (const auto &s : section.sections) {
        if (!ProcessConfigSection(cfg, s, path + "." + s.name)) {
            return false;
        }
    }
    return true;
}

}  // namespace

namespace panda::verifier::config {

void RegisterConfigHandler(Config *cfg, const PandaString &path, callable<bool(Config *, const Section &)> handler)
{
    auto &sectionHandlers = cfg->debugCfg.sectionHandlers;
    sectionHandlers[path] = handler;
}

bool ProcessConfig(Config *cfg, const Section &section)
{
    return ProcessConfigSection(cfg, section, section.name);
}

}  // namespace panda::verifier::config
