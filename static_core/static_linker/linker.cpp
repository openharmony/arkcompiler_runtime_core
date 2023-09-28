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

#include <chrono>

#include "linker.h"
#include "linker_context.h"

namespace {

void PrintTime(std::ostream &o, uint64_t micros)
{
    auto f = [&](uint64_t d, const auto suffix) {
        if (d == 1 || micros >= d) {
            auto full = micros / d;
            auto rem = double(micros - full * d) / d;
            o << double(full) + rem << " " << suffix;
            return true;
        }
        return false;
    };
    constexpr uint64_t MICRO_IN_SEC = 1'000'000;
    constexpr uint64_t MICRO_IN_MILLI = 1'000;
    f(MICRO_IN_SEC, "seconds") || f(MICRO_IN_MILLI, "milli seconds") || f(1, "micro seconds");
}

}  // namespace

namespace panda::static_linker {
Config DefaultConfig()
{
    return Config {};
}

Result Link(const Config &conf, const std::string &output, const std::vector<std::string> &input)
{
    auto ctx = Context(conf);

    using Clock = std::chrono::high_resolution_clock;

    auto t_start = Clock::now();

    ctx.Read(input);  // concurrent
    if (ctx.HasErrors()) {
        return ctx.GetResult();
    }

    auto t_read = Clock::now();

    ctx.Merge();  // sync
    if (ctx.HasErrors()) {
        return ctx.GetResult();
    }

    auto t_merge = Clock::now();

    ctx.Parse();  // could be semi concurrent
    if (ctx.HasErrors()) {
        return ctx.GetResult();
    }

    auto t_parse = Clock::now();

    ctx.ComputeLayout();  // sync
    if (ctx.HasErrors()) {
        return ctx.GetResult();
    }

    auto t_layout = Clock::now();

    ctx.Patch();  // concurrent
    if (ctx.HasErrors()) {
        return ctx.GetResult();
    }

    auto t_patch = Clock::now();

    ctx.Write(output);  // sync

    auto t_end = std::chrono::high_resolution_clock::now();

    auto res = ctx.GetResult();

    auto delta = [](const auto &s, const auto &e) {
        return std::chrono::duration_cast<std::chrono::microseconds>(e - s).count();
    };

    res.stats.elapsed.read = delta(t_start, t_read);
    res.stats.elapsed.merge = delta(t_read, t_merge);
    res.stats.elapsed.parse = delta(t_merge, t_parse);
    res.stats.elapsed.layout = delta(t_parse, t_layout);
    res.stats.elapsed.patch = delta(t_layout, t_patch);
    res.stats.elapsed.write = delta(t_patch, t_end);
    res.stats.elapsed.total = delta(t_start, t_end);

    return res;
}

std::ostream &operator<<(std::ostream &o, const Result::Stats &s)
{
    o << "total: ";
    PrintTime(o, s.elapsed.total);
    o << "\n";
    auto print_time_hist = [&](std::string_view name, const uint64_t t) {
        constexpr size_t MAX_NAME_SIZE = 10;
        o << std::left << std::setw(MAX_NAME_SIZE) << name << std::internal;

        o << "|";

        constexpr uint64_t PARTS = 30;
        auto dots = s.elapsed.total == 0 ? 0 : t * PARTS / s.elapsed.total;
        using CharType = std::remove_reference_t<decltype(o)>::char_type;
        std::fill_n(std::ostream_iterator<CharType>(o), dots, '#');
        std::fill_n(std::ostream_iterator<CharType>(o), PARTS - dots, ' ');

        o << "| ";
        PrintTime(o, t);
        o << "\n";
    };
    print_time_hist("read", s.elapsed.read);
    print_time_hist("merge", s.elapsed.merge);
    print_time_hist("parse", s.elapsed.parse);
    print_time_hist("layout", s.elapsed.layout);
    print_time_hist("patch", s.elapsed.patch);
    print_time_hist("write", s.elapsed.write);

    o << "items: " << s.items_count << "\n";
    o << "classes: " << s.class_count << "\n";

    o << "code items: " << s.code_count << "\n";
    o << "debug data: " << s.debug_count << "\n";

    return o;
}

}  // namespace panda::static_linker
