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

#include <gtest/gtest.h>
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <memory>
#include <optional>
#include <sstream>
#include <string>

#include "../linker_context.h"
#include "assembler/assembly-field.h"
#include "assembler/assembly-function.h"
#include "assembler/assembly-program.h"
#include "assembler/assembly-record.h"
#include "assembler/assembly-emitter.h"
#include "assembler/assembly-parser.h"
#include "assembler/ins_create_api.h"
#include "libpandafile/file_item_container.h"
#include "libpandafile/file_writer.h"
#include "include/runtime_options.h"
#include "linker.h"
#include "runtime/include/runtime.h"
#include "source_lang_enum.h"

namespace {
using Config = panda::static_linker::Config;
using Result = panda::static_linker::Result;
using panda::static_linker::DefaultConfig;
using panda::static_linker::Link;

constexpr size_t TEST_REPEAT_COUNT = 10;

std::pair<int, std::string> ExecPanda(const std::string &file)
{
    auto opts = panda::RuntimeOptions {};
    auto boot = opts.GetBootPandaFiles();
    for (auto &a : boot) {
        a.insert(0, "../");
    }
    opts.SetBootPandaFiles(std::move(boot));

    opts.SetLoadRuntimes({"core"});

    opts.SetPandaFiles({file});
    if (!panda::Runtime::Create(opts)) {
        return {1, "can't create runtime"};
    }

    auto *runtime = panda::Runtime::GetCurrent();

    std::stringstream str_buf;
    auto old = std::cout.rdbuf(str_buf.rdbuf());
    auto reset = [&](auto *cout) { cout->rdbuf(old); };
    auto guard = std::unique_ptr<std::ostream, decltype(reset)>(&std::cout, reset);

    auto res = runtime->ExecutePandaFile(file, "_GLOBAL::main", {});
    auto ret = std::pair<int, std::string> {};
    if (!res) {
        ret = {1, "error " + std::to_string((int)res.Error())};
    } else {
        ret = {0, str_buf.str()};
    }

    if (!panda::Runtime::Destroy()) {
        return {1, "can't destroy runtime"};
    }

    return ret;
}

template <bool IS_BINARY>
bool ReadFile(const std::string &path, std::conditional_t<IS_BINARY, std::vector<char>, std::string> &out)
{
    auto f = std::ifstream(path, IS_BINARY ? std::ios_base::binary : std::ios_base::in);
    if (!f.is_open() || f.bad()) {
        return false;
    }

    out.clear();

    out.assign((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    return true;
}

// removes comments
void NormalizeGold(std::string &gold)
{
    std::string_view in = gold;
    std::string out;
    out.reserve(gold.size());
    while (!in.empty()) {
        auto nxt_nl = in.find('\n');
        if (in[0] == '#') {
            if (nxt_nl == std::string::npos) {
                break;
            }
            in = in.substr(nxt_nl + 1);
            continue;
        }
        if (nxt_nl == std::string::npos) {
            out += in;
            break;
        }
        out += in.substr(0, nxt_nl + 1);
        in = in.substr(nxt_nl + 1);
    }
    gold = std::move(out);
}

std::optional<std::string> Build(const std::string &path)
{
    std::string prog;

    if (!ReadFile<false>(path + ".pa", prog)) {
        return "can't read file " + path + ".pa";
    }
    panda::pandasm::Parser p;
    auto res = p.Parse(prog, path + ".pa");
    if (p.ShowError().err != panda::pandasm::Error::ErrorType::ERR_NONE) {
        return p.ShowError().message + "\n" + p.ShowError().whole_line;
    }

    if (!res.HasValue()) {
        return "no parsed value";
    }

    auto writer = panda::panda_file::FileWriter(path + ".abc");
    if (!panda::pandasm::AsmEmitter::Emit(&writer, res.Value())) {
        return "can't emit";
    }

    return std::nullopt;
}

void TestSingle(const std::string &path, bool is_good = true,
                const Config &conf = panda::static_linker::DefaultConfig(), bool *succeded = nullptr,
                Result *save_result = nullptr)
{
    const auto path_prefix = "data/single/";
    ASSERT_EQ(Build(path_prefix + path), std::nullopt);
    auto gold = std::string {};
    ASSERT_TRUE(ReadFile<false>(path_prefix + path + ".gold", gold));

    NormalizeGold(gold);

    const auto out = path_prefix + path + ".linked.abc";
    auto link_res = Link(conf, out, {path_prefix + path + ".abc"});

    ASSERT_EQ(link_res.errors.empty(), is_good);

    if (is_good) {
        auto res = ExecPanda(out);
        ASSERT_EQ(res.first, 0);
        ASSERT_EQ(res.second, gold);
    }

    if (succeded != nullptr) {
        *succeded = true;
    }

    if (save_result != nullptr) {
        *save_result = std::move(link_res);
    }
}

void TestMultiple(const std::string &path, std::vector<std::string> perms, bool is_good = true,
                  const Config &conf = panda::static_linker::DefaultConfig(), Result *expected = nullptr)
{
    std::sort(perms.begin(), perms.end());

    const auto path_prefix = "data/multi/" + path + "/";

    for (const auto &p : perms) {
        ASSERT_EQ(Build(path_prefix + p), std::nullopt);
    }

    auto gold = std::string {};

    if (is_good) {
        ASSERT_TRUE(ReadFile<false>(path_prefix + "out.gold", gold));
        NormalizeGold(gold);
    }

    auto out = std::string {};
    auto files = std::vector<std::string> {};

    std::optional<std::vector<char>> expected_file;

    auto perform_test = [&](size_t iteration) {
        out = path_prefix + "linked.";
        files.clear();

        for (const auto &f : perms) {
            out += f;
            out += ".";
            files.emplace_back(path_prefix + f + ".abc");
        }
        out += "it";
        out += std::to_string(iteration);
        out += ".abc";

        SCOPED_TRACE(out);

        auto link_res = Link(conf, out, files);
        if (link_res.errors.empty() != is_good) {
            auto errs = std::string();
            for (auto &err : link_res.errors) {
                errs += err;
                errs += "\n";
            }
            ASSERT_EQ(link_res.errors.empty(), is_good) << errs;
        }

        if (expected != nullptr) {
            ASSERT_EQ(link_res.stats.deduplicated_foreigners, expected->stats.deduplicated_foreigners);
        }

        if (is_good) {
            std::vector<char> got_file;
            ASSERT_TRUE(ReadFile<true>(out, got_file));
            if (!expected_file.has_value()) {
                expected_file = std::move(got_file);
            } else {
                (void)iteration;
                ASSERT_EQ(expected_file.value(), got_file) << "on iteration: " << iteration;
            }

            auto res = ExecPanda(out);
            ASSERT_EQ(res.first, 0);
            ASSERT_EQ(res.second, gold);
        }
    };

    do {
        expected_file = std::nullopt;
        for (size_t iteration = 0; iteration < TEST_REPEAT_COUNT; iteration++) {
            perform_test(iteration);
        }
    } while (std::next_permutation(perms.begin(), perms.end()));
}
}  // namespace

TEST(linkertests, HelloWorld)
{
    TestSingle("hello_world");
}

TEST(linkertests, LitArray)
{
    TestSingle("lit_array");
}

TEST(linkertests, Exceptions)
{
    TestSingle("exceptions");
}

TEST(linkertests, ForeignMethod)
{
    TestMultiple("fmethod", {"1", "2"});
}

TEST(linkertests, ForeignField)
{
    TestMultiple("ffield", {"1", "2"});
}

TEST(linkertests, BadForeignField)
{
    TestMultiple("bad_ffield", {"1", "2"}, false);
}

TEST(linkertests, BadClassRedefinition)
{
    TestMultiple("bad_class_redefinition", {"1", "2"}, false);
}

TEST(linkertests, BadFFieldType)
{
    TestMultiple("bad_ffield_type", {"1", "2"}, false);
}

TEST(linkertests, FMethodOverloaded)
{
    TestMultiple("fmethod_overloaded", {"1", "2"});
}

TEST(linkertests, FMethodOverloaded2)
{
    TestMultiple("fmethod_overloaded_2", {"1", "2", "3", "4"});
}

TEST(linkertests, BadFMethodOverloaded)
{
    TestMultiple("bad_fmethod_overloaded", {"1", "2"}, false);
}

TEST(linkertests, DeduplicatedField)
{
    auto res = Result {};
    res.stats.deduplicated_foreigners = 1;
    TestMultiple("dedup_field", {"1", "2"}, true, DefaultConfig(), &res);
}

TEST(linkertests, DeduplicatedMethod)
{
    auto res = Result {};
    res.stats.deduplicated_foreigners = 1;
    TestMultiple("dedup_method", {"1", "2"}, true, DefaultConfig(), &res);
}

TEST(linkertests, UnresolvedInGlobal)
{
    TestSingle("unresolved_global", false);
    auto conf = DefaultConfig();
    conf.remains_partial = {std::string(panda::panda_file::ItemContainer::GetGlobalClassName())};
    TestSingle("unresolved_global", true, conf);
}

TEST(linkertests, DeduplicateLineNumberNrogram)
{
    auto succ = false;
    auto res = Result {};
    TestSingle("lnp_dedup", true, DefaultConfig(), &succ, &res);
    ASSERT_TRUE(succ);
    ASSERT_EQ(res.stats.debug_count, 1);
}

TEST(linkertests, StripDebugInfo)
{
    auto succ = false;
    auto res = Result {};
    auto conf = DefaultConfig();
    conf.strip_debug_info = true;
    TestSingle("hello_world", true, conf, &succ, &res);
    ASSERT_TRUE(succ);
    ASSERT_EQ(res.stats.debug_count, 0);
}

TEST(linkertests, FieldOverload)
{
    auto conf = DefaultConfig();
    conf.partial.emplace("LFor;");
    TestMultiple("ffield_overloaded", {"1", "2"}, true, conf);
}

TEST(linkertests, ForeignBase)
{
#ifdef PANDA_WITH_ETS
    constexpr auto LANG = panda::panda_file::SourceLang::ETS;
    auto make_record = [](panda::pandasm::Program &prog, const std::string &name) {
        return &prog.record_table.emplace(name, panda::pandasm::Record(name, LANG)).first->second;
    };

    const std::string base_path = "data/multi/ForeignBase.1.abc";
    const std::string derv_path = "data/multi/ForeignBase.2.abc";

    {
        panda::pandasm::Program prog_base;
        auto base = make_record(prog_base, "Base");
        auto fld = panda::pandasm::Field(LANG);
        fld.name = "fld";
        fld.type = panda::pandasm::Type("i32", 0);
        base->field_list.push_back(std::move(fld));

        ASSERT_TRUE(panda::pandasm::AsmEmitter::Emit(base_path, prog_base));
    }

    {
        panda::pandasm::Program prog_der;
        auto base = make_record(prog_der, "Base");
        base->metadata->SetAttribute("external");

        auto derv = make_record(prog_der, "Derv");
        ASSERT_EQ(derv->metadata->SetAttributeValue("ets.extends", "Base"), std::nullopt);
        std::ignore = derv;
        auto fld = panda::pandasm::Field(LANG);
        fld.name = "fld";
        fld.type = panda::pandasm::Type("i32", 0);
        fld.metadata->SetAttribute("external");
        derv->field_list.push_back(std::move(fld));

        auto func = panda::pandasm::Function("main", LANG);
        func.regs_num = 1;
        func.return_type = panda::pandasm::Type("void", 0);
        func.AddInstruction(panda::pandasm::Create_NEWOBJ(0, "Derv"));
        func.AddInstruction(panda::pandasm::Create_LDOBJ(0, "Derv.fld"));
        func.AddInstruction(panda::pandasm::Create_RETURN_VOID());

        prog_der.function_table.emplace(func.name, std::move(func));

        ASSERT_TRUE(panda::pandasm::AsmEmitter::Emit(derv_path, prog_der));
    }

    auto res = Link(DefaultConfig(), "data/multi/ForeignBase.linked.abc", {base_path, derv_path});
    ASSERT_TRUE(res.errors.empty()) << res.errors.front();
#endif
}
