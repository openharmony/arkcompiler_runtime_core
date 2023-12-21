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
#include "compilation.h"
#include "compiler_options.h"
#include "irtoc_options.h"
#include "generated/base_options.h"

namespace panda::irtoc {

int Run(int argc, const char **argv)
{
    panda::PandArgParser pa_parser;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    panda::irtoc::OPTIONS.AddOptions(&pa_parser);
    base_options::Options base_options("");
    base_options.AddOptions(&pa_parser);
    panda::compiler::OPTIONS.AddOptions(&pa_parser);
#ifdef PANDA_LLVMAOT
    panda::llvmaot::OPTIONS.AddOptions(&pa_parser);
#endif
    if (!pa_parser.Parse(argc, argv)) {
        std::cerr << "Error: " << pa_parser.GetErrorString() << "\n";
        return -1;
    }

    Logger::Initialize(base_options);

    panda::compiler::CompilerLogger::SetComponents(panda::compiler::OPTIONS.GetCompilerLog());

    if (std::getenv("IRTOC_VERBOSE") != nullptr) {
        Logger::SetLevel(Logger::Level::DEBUG);
        Logger::ResetComponentMask();
        Logger::EnableComponent(Logger::Component::IRTOC);
    }

    auto res = Compilation().Run();
    if (!res.HasValue()) {
        std::cerr << res.Error() << std::endl;
        return 1;
    }
    return 0;
}

}  // namespace panda::irtoc

int main(int argc, const char **argv)
{
    return panda::irtoc::Run(argc, argv);
}
