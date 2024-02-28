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

#include "config_load.h"

#include "verification/public.h"
#include "verification/config/default/default_config.h"
#include "verification/config/parse/config_parse.h"
#include "verification/config/process/config_process.h"
#include "verification/config/context/context.h"
#include "verification/config/handlers/config_handlers.h"

#if !PANDA_TARGET_WINDOWS
#include "securec.h"
#endif
#include "utils/logger.h"
#include "libpandabase/os/file.h"

namespace {

bool ProcessConfigFile(panda::verifier::Config *cfg, const char *text)
{
    panda::verifier::debug::RegisterConfigHandlerBreakpoints(cfg);
    panda::verifier::debug::RegisterConfigHandlerWhitelist(cfg);
    panda::verifier::debug::RegisterConfigHandlerOptions(cfg);
    panda::verifier::debug::RegisterConfigHandlerMethodOptions(cfg);
    panda::verifier::debug::RegisterConfigHandlerMethodGroups(cfg);

    panda::verifier::config::Section section;

    bool result =
        panda::verifier::config::ParseConfig(text, section) && panda::verifier::config::ProcessConfig(cfg, section);
    if (result) {
        LOG(DEBUG, VERIFIER) << "Verifier debug configuration: \n" << section.Image();
        panda::verifier::debug::SetDefaultMethodOptions(cfg);
    }

    return result;
}

}  // namespace

namespace panda::verifier::config {

bool LoadConfig(Config *cfg, std::string_view filename)
{
    using panda::os::file::Mode;
    using panda::os::file::Open;

    bool result = false;

    if (filename == "default") {
        result = ProcessConfigFile(cfg, VERIFIER_DEBUG_DEFAULT_CONFIG);
    } else {
        do {
            auto file = Open(filename, Mode::READONLY);
            if (!file.IsValid()) {
                break;
            }
            auto size = file.GetFileSize();
            if (!size.HasValue()) {
                file.Close();
                break;
            }
            char *text = new char[1 + *size];
            memset_s(text, 1 + *size, 0x00, 1 + *size);
            if (!file.ReadAll(text, *size)) {
                file.Close();
                delete[] text;
                break;
            }
            text[*size] = 0;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            file.Close();
            result = ProcessConfigFile(cfg, text);
            delete[] text;
        } while (false);
    }

    if (!result) {
        LOG(DEBUG, VERIFIER) << "Failed to load verifier debug config file '" << filename << "'";
    }

    return result;
}

}  // namespace panda::verifier::config
