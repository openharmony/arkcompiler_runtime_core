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

#include "linker_context.h"
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
using Config = ark::static_linker::Config;
using Result = ark::static_linker::Result;
using ark::static_linker::DefaultConfig;
using ark::static_linker::Link;

constexpr size_t TEST_REPEAT_COUNT = 10;

std::pair<int, std::string> ExecPanda(const std::string &file)
{
    auto opts = ark::RuntimeOptions {};
    auto boot = opts.GetBootPandaFiles();
    for (auto &a : boot) {
        a.insert(0, "../");
    }
    opts.SetBootPandaFiles(std::move(boot));

    opts.SetLoadRuntimes({"core"});

    opts.SetPandaFiles({file});
    if (!ark::Runtime::Create(opts)) {
        return {1, "can't create runtime"};
    }

    auto *runtime = ark::Runtime::GetCurrent();

    std::stringstream strBuf;
    auto old = std::cout.rdbuf(strBuf.rdbuf());
    auto reset = [&old](auto *cout) { cout->rdbuf(old); };
    auto guard = std::unique_ptr<std::ostream, decltype(reset)>(&std::cout, reset);

    auto res = runtime->ExecutePandaFile(file, "_GLOBAL::main", {});
    auto ret = std::pair<int, std::string> {};
    if (!res) {
        ret = {1, "error " + std::to_string((int)res.Error())};
    } else {
        ret = {0, strBuf.str()};
    }

    if (!ark::Runtime::Destroy()) {
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
        auto nxtNl = in.find('\n');
        if (in[0] == '#') {
            if (nxtNl == std::string::npos) {
                break;
            }
            in = in.substr(nxtNl + 1);
            continue;
        }
        if (nxtNl == std::string::npos) {
            out += in;
            break;
        }
        out += in.substr(0, nxtNl + 1);
        in = in.substr(nxtNl + 1);
    }
    gold = std::move(out);
}

std::optional<std::string> Build(const std::string &path)
{
    std::string prog;

    if (!ReadFile<false>(path + ".pa", prog)) {
        return "can't read file " + path + ".pa";
    }
    ark::pandasm::Parser p;
    auto res = p.Parse(prog, path + ".pa");
    if (p.ShowError().err != ark::pandasm::Error::ErrorType::ERR_NONE) {
        return p.ShowError().message + "\n" + p.ShowError().wholeLine;
    }

    if (!res.HasValue()) {
        return "no parsed value";
    }

    auto writer = ark::panda_file::FileWriter(path + ".abc");
    if (!ark::pandasm::AsmEmitter::Emit(&writer, res.Value())) {
        return "can't emit";
    }

    return std::nullopt;
}

void TestSingle(const std::string &path, bool isGood = true, const Config &conf = ark::static_linker::DefaultConfig(),
                bool *succeded = nullptr, Result *saveResult = nullptr)
{
    const auto pathPrefix = "data/single/";
    ASSERT_EQ(Build(pathPrefix + path), std::nullopt);
    auto gold = std::string {};
    ASSERT_TRUE(ReadFile<false>(pathPrefix + path + ".gold", gold));

    NormalizeGold(gold);

    const auto out = pathPrefix + path + ".linked.abc";
    auto linkRes = Link(conf, out, {pathPrefix + path + ".abc"});

    ASSERT_EQ(linkRes.errors.empty(), isGood);

    if (isGood) {
        auto res = ExecPanda(out);
        ASSERT_EQ(res.first, 0);
        ASSERT_EQ(res.second, gold);
    }

    if (succeded != nullptr) {
        *succeded = true;
    }

    if (saveResult != nullptr) {
        *saveResult = std::move(linkRes);
    }
}

struct TestData {
    std::string pathPrefix;
    bool isGood = false;
    Result *expected = nullptr;
    std::string gold;
};

void PerformTest(TestData *data, const std::vector<std::string> &perms, const Config &conf,
                 std::optional<std::vector<char>> expectedFile, size_t iteration)
{
    auto out = data->pathPrefix + "linked.";
    auto files = std::vector<std::string> {};

    for (const auto &f : perms) {
        out += f;
        out += ".";
        files.emplace_back(data->pathPrefix + f + ".abc");
    }
    out += "it";
    out += std::to_string(iteration);
    out += ".abc";

    SCOPED_TRACE(out);

    auto linkRes = Link(conf, out, files);
    if (linkRes.errors.empty() != data->isGood) {
        auto errs = std::string();
        for (auto &err : linkRes.errors) {
            errs += err;
            errs += "\n";
        }
        ASSERT_EQ(linkRes.errors.empty(), data->isGood) << errs;
    }

    if (data->expected != nullptr) {
        ASSERT_EQ(linkRes.stats.deduplicatedForeigners, data->expected->stats.deduplicatedForeigners);
    }

    if (data->isGood) {
        std::vector<char> gotFile;
        ASSERT_TRUE(ReadFile<true>(out, gotFile));
        if (!expectedFile.has_value()) {
            expectedFile = std::move(gotFile);
        } else {
            (void)iteration;
            ASSERT_EQ(expectedFile.value(), gotFile) << "on iteration: " << iteration;
        }

        auto res = ExecPanda(out);
        ASSERT_EQ(res.first, 0);
        ASSERT_EQ(res.second, data->gold);
    }
}

void TestMultiple(const std::string &path, std::vector<std::string> perms, bool isGood = true,
                  const Config &conf = ark::static_linker::DefaultConfig(), Result *expected = nullptr)
{
    std::sort(perms.begin(), perms.end());

    const auto pathPrefix = "data/multi/" + path + "/";

    for (const auto &p : perms) {
        ASSERT_EQ(Build(pathPrefix + p), std::nullopt);
    }

    auto gold = std::string {};

    if (isGood) {
        ASSERT_TRUE(ReadFile<false>(pathPrefix + "out.gold", gold));
        NormalizeGold(gold);
    }

    std::optional<std::vector<char>> expectedFile;

    do {
        expectedFile = std::nullopt;
        TestData data;
        data.pathPrefix = pathPrefix;
        data.isGood = isGood;
        data.expected = expected;
        data.gold = gold;
        for (size_t iteration = 0; iteration < TEST_REPEAT_COUNT; iteration++) {
            PerformTest(&data, perms, conf, expectedFile, iteration);
        }
    } while (std::next_permutation(perms.begin(), perms.end()));
}

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
    res.stats.deduplicatedForeigners = 1;
    TestMultiple("dedup_field", {"1", "2"}, true, DefaultConfig(), &res);
}

TEST(linkertests, DeduplicatedMethod)
{
    auto res = Result {};
    res.stats.deduplicatedForeigners = 1;
    TestMultiple("dedup_method", {"1", "2"}, true, DefaultConfig(), &res);
}

TEST(linkertests, UnresolvedInGlobal)
{
    TestSingle("unresolved_global", false);
    auto conf = DefaultConfig();
    conf.remainsPartial = {std::string(ark::panda_file::ItemContainer::GetGlobalClassName())};
    TestSingle("unresolved_global", true, conf);
}

TEST(linkertests, DeduplicateLineNumberNrogram)
{
    auto succ = false;
    auto res = Result {};
    TestSingle("lnp_dedup", true, DefaultConfig(), &succ, &res);
    ASSERT_TRUE(succ);
    ASSERT_EQ(res.stats.debugCount, 1);
}

TEST(linkertests, StripDebugInfo)
{
    auto succ = false;
    auto res = Result {};
    auto conf = DefaultConfig();
    conf.stripDebugInfo = true;
    TestSingle("hello_world", true, conf, &succ, &res);
    ASSERT_TRUE(succ);
    ASSERT_EQ(res.stats.debugCount, 0);
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
    constexpr auto LANG = ark::panda_file::SourceLang::ETS;
    auto makeRecord = [](ark::pandasm::Program &prog, const std::string &name) {
        return &prog.recordTable.emplace(name, ark::pandasm::Record(name, LANG)).first->second;
    };

    const std::string basePath = "data/multi/ForeignBase.1.abc";
    const std::string dervPath = "data/multi/ForeignBase.2.abc";

    {
        ark::pandasm::Program progBase;
        auto base = makeRecord(progBase, "Base");
        auto fld = ark::pandasm::Field(LANG);
        fld.name = "fld";
        fld.type = ark::pandasm::Type("i32", 0);
        base->fieldList.push_back(std::move(fld));

        ASSERT_TRUE(ark::pandasm::AsmEmitter::Emit(basePath, progBase));
    }

    {
        ark::pandasm::Program progDer;
        auto base = makeRecord(progDer, "Base");
        base->metadata->SetAttribute("external");

        auto derv = makeRecord(progDer, "Derv");
        ASSERT_EQ(derv->metadata->SetAttributeValue("ets.extends", "Base"), std::nullopt);
        std::ignore = derv;
        auto fld = ark::pandasm::Field(LANG);
        fld.name = "fld";
        fld.type = ark::pandasm::Type("i32", 0);
        fld.metadata->SetAttribute("external");
        derv->fieldList.push_back(std::move(fld));

        auto func = ark::pandasm::Function("main", LANG);
        func.regsNum = 1;
        func.returnType = ark::pandasm::Type("void", 0);
        func.AddInstruction(ark::pandasm::Create_NEWOBJ(0, "Derv"));
        func.AddInstruction(ark::pandasm::Create_LDOBJ(0, "Derv.fld"));
        func.AddInstruction(ark::pandasm::Create_RETURN_VOID());

        progDer.functionTable.emplace(func.name, std::move(func));

        ASSERT_TRUE(ark::pandasm::AsmEmitter::Emit(dervPath, progDer));
    }

    auto res = Link(DefaultConfig(), "data/multi/ForeignBase.linked.abc", {basePath, dervPath});
    ASSERT_TRUE(res.errors.empty()) << res.errors.front();
#endif
}
}  // namespace
