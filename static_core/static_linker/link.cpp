/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <algorithm>

#include "libpandabase/utils/pandargs.h"
#include "libpandabase/utils/logger.h"
#include "generated/link_options.h"

#include "linker.h"

namespace {

int PrintHelp(const ark::PandArgParser &paParser)
{
    const auto a = paParser.GetErrorString();
    if (!a.empty()) {
        std::cerr << "Error: " << a << std::endl;
    }

    std::cerr << "Usage:" << std::endl;
    std::cerr << "ark_link [OPTIONS] -- FILES..." << std::endl << std::endl;
    std::cerr << "Supported options:" << std::endl << std::endl;
    std::cerr << paParser.GetHelpString() << std::endl;
    return 1;
}

std::string MangleClass(std::string s)
{
    s.insert(s.begin(), 'L');
    s += ";";
    return s;
}

}  // namespace

int main(int argc, const char *argv[])
{
    ark::PandArgParser paParser;
    ark::static_linker::Options options {*argv};
    options.AddOptions(&paParser);
    paParser.EnableRemainder();

    if (!paParser.Parse(argc, argv)) {
        return PrintHelp(paParser);
    }

    ark::Logger::InitializeStdLogging(
        ark::Logger::LevelFromString(options.GetLogLevel()),
        ark::Logger::ComponentMask().set(ark::Logger::Component::STATIC_LINKER).set(ark::Logger::Component::PANDAFILE));

    const auto files = paParser.GetRemainder();
    if (files.empty()) {
        std::cerr << "must have at least one file" << std::endl;
        return PrintHelp(paParser);
    }

    if (options.GetOutput().empty()) {
        auto const &fn = files[0];
        options.SetOutput(fn.substr(0, fn.find_last_of('.')) + ".linked.abc");
    }

    auto conf = ark::static_linker::DefaultConfig();

    conf.stripDebugInfo = options.IsStripDebugInfo();

    auto classesVecToSet = [](const std::vector<std::string> &v, std::set<std::string> &s) {
        s.clear();
        std::transform(v.begin(), v.end(), std::inserter(s, s.begin()), MangleClass);
    };

    classesVecToSet(options.GetPartialClasses(), conf.partial);
    classesVecToSet(options.GetRemainsPartialClasses(), conf.remainsPartial);

    auto res = ark::static_linker::Link(conf, options.GetOutput(), files);

    size_t i = 0;
    for (const auto &s : res.errors) {
        std::cerr << "# " << ++i << "\n";
        std::cerr << s << std::endl;
    }

    if (options.IsShowStats()) {
        std::cout << "stats:\n" << res.stats << std::endl;
    }

    const auto wasError = !res.errors.empty();
    return static_cast<int>(wasError);
}
