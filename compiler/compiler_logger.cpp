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

#include "compiler_logger.h"

namespace panda::compiler {
std::bitset<CompilerLoggerComponents::LOG_COMPONENTS_NUM> CompilerLogger::components(0);

// clang-format off
/* static */
void CompilerLogger::SetComponents(const std::vector<std::string>& args)
{
    components.reset();
    for (const auto& arg : args) {
        if (arg == "all") {
            components.set();
            break;
        }
        if (arg == "none") {
            components.reset();
            break;
        }
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEF(COMPONENT, NAME)                \
            if (NAME == arg) {              \
                components.set(COMPONENT);  \
                continue;                   \
            }
        COMPILER_LOG_COMPONENTS(DEF)
#undef DEF

        UNREACHABLE();
    }
}
// clang-format on
}  // namespace panda::compiler
