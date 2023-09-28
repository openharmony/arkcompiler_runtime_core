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

#include <algorithm>

#include "libpandabase/utils/pandargs.h"
#include "libpandabase/utils/logger.h"
#include "generated/link_options.h"

#include "linker.h"

namespace {

int PrintHelp(const panda::PandArgParser &pa_parser)
{
    const auto a = pa_parser.GetErrorString();
    if (!a.empty()) {
        std::cerr << "Error: " << a << std::endl;
    }

    std::cerr << "Usage:" << std::endl;
    std::cerr << "ark_link [OPTIONS] -- FILES..." << std::endl << std::endl;
    std::cerr << "Supported options:" << std::endl << std::endl;
    std::cerr << pa_parser.GetHelpString() << std::endl;
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
    panda::PandArgParser pa_parser;
    panda::static_linker::Options options {*argv};
    options.AddOptions(&pa_parser);
    pa_parser.EnableRemainder();

    if (!pa_parser.Parse(argc, argv)) {
        return PrintHelp(pa_parser);
    }

    panda::Logger::InitializeStdLogging(panda::Logger::LevelFromString(options.GetLogLevel()),
                                        panda::Logger::ComponentMask()
                                            .set(panda::Logger::Component::STATIC_LINKER)
                                            .set(panda::Logger::Component::PANDAFILE));

    const auto files = pa_parser.GetRemainder();

    if (files.empty()) {
        std::cerr << "must have at least one file" << std::endl;
        return PrintHelp(pa_parser);
    }

    if (options.GetOutput().empty()) {
        auto const &fn = files[0];
        options.SetOutput(fn.substr(0, fn.find_last_of('.')) + ".linked.abc");
    }

    auto conf = panda::static_linker::DefaultConfig();

    conf.strip_debug_info = options.IsStripDebugInfo();

    auto classes_vec_to_set = [](const std::vector<std::string> &v, std::set<std::string> &s) {
        s.clear();
        std::transform(v.begin(), v.end(), std::inserter(s, s.begin()), MangleClass);
    };

    classes_vec_to_set(options.GetParitalClasses(), conf.partial);
    classes_vec_to_set(options.GetRemainsPartialClasses(), conf.remains_partial);

    auto res = panda::static_linker::Link(conf, options.GetOutput(), files);

    size_t i = 0;
    for (const auto &s : res.errors) {
        std::cerr << "# " << ++i << "\n";
        std::cerr << s << std::endl;
    }

    if (options.IsShowStats()) {
        std::cout << "stats:\n" << res.stats << std::endl;
    }

    const auto was_error = !res.errors.empty();
    return static_cast<int>(was_error);
}
