/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include "linker_context.h"
#include "linker_context_misc.cpp"
#include "runtime/include/runtime.h"
#include "source_lang_enum.h"

namespace {
using Config = ark::static_linker::Config;
using Result = ark::static_linker::Result;
using ark::static_linker::DefaultConfig;
using ark::static_linker::Link;

#ifdef TEST_STATIC_LINKER_WITH_STS
constexpr std::string_view ABC_FILE_EXTENSION = ".abc";
constexpr size_t ABC_FILE_EXTENSION_LENGTH = 4;
#endif
constexpr size_t TEST_REPEAT_COUNT = 10;

std::pair<int, std::string> ExecPanda(const std::string &file, const std::string &loadRuntimes = "core",
                                      const std::string &entryPoint = "_GLOBAL::main")
{
    auto opts = ark::RuntimeOptions {};
    if (loadRuntimes == "ets") {
        opts.SetBootPandaFiles({"../../plugins/ets/etsstdlib.abc"});
    } else if (loadRuntimes == "core") {
        auto boot = opts.GetBootPandaFiles();
        for (auto &a : boot) {
            a.insert(0, "../");
        }
        opts.SetBootPandaFiles(std::move(boot));
    } else {
        return {1, "unknown loadRuntimes " + loadRuntimes};
    }

    opts.SetLoadRuntimes({loadRuntimes});

    opts.SetPandaFiles({file});
    if (!ark::Runtime::Create(opts)) {
        return {1, "can't create runtime"};
    }

    auto *runtime = ark::Runtime::GetCurrent();

    std::stringstream strBuf;
    auto old = std::cout.rdbuf(strBuf.rdbuf());
    auto reset = [&old](auto *cout) { cout->rdbuf(old); };
    auto guard = std::unique_ptr<std::ostream, decltype(reset)>(&std::cout, reset);

    auto res = runtime->ExecutePandaFile(file, entryPoint, {});
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

#ifdef TEST_STATIC_LINKER_WITH_STS
// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
struct StripOptions {
    bool stripUnused = false;
    std::optional<std::string> stripUnusedSkiplist;
    bool IsEmpty() const
    {
        return !stripUnused && !stripUnusedSkiplist.has_value();
    }
    StripOptions() = default;
};

struct TestStsConfig {
    std::string entryPoint = "1/ETSGLOBAL::main";
    std::string outputFileName = "out.gold";
    TestStsConfig() = default;
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

std::string BuildLinkerCommand(const std::string &path, const std::vector<std::string> &files,
                               const StripOptions &stripOptions, const std::string &targetPathPrefix)
{
    std::string linkerCommand = "../../bin/ark_link";

    if (stripOptions.stripUnused) {
        linkerCommand.append(" --strip-unused");
    }

    if (stripOptions.stripUnusedSkiplist.has_value()) {
        linkerCommand.append(" --strip-unused-skiplist=");
        linkerCommand.append(stripOptions.stripUnusedSkiplist.value());
    }

    linkerCommand.append(" --output " + targetPathPrefix + "linked.abc");
    linkerCommand.append(" -- ");
    for (const auto &f : files) {
        if (f.length() < ABC_FILE_EXTENSION_LENGTH ||
            f.substr(f.length() - ABC_FILE_EXTENSION_LENGTH) != ABC_FILE_EXTENSION) {
            linkerCommand.push_back('@');  // test filesinfo
        }
        linkerCommand.append(path + f);
        linkerCommand.push_back(' ');
    }

    return linkerCommand;
}

void TestSts(const std::string &path, const std::vector<std::string> &files, bool isGood = true,
             const StripOptions &deleteOptions = {}, const TestStsConfig &config = {})
{
    const auto sourcePathPrefix = "data/ets/" + path + "/";
    const auto targetPathPrefix = "data/output/" + path + "/";

    std::string linkerCommand = BuildLinkerCommand(sourcePathPrefix, files, deleteOptions, targetPathPrefix);
    // NOLINTNEXTLINE(cert-env33-c)
    auto linkRes = std::system(linkerCommand.c_str());

    if (isGood) {
        ASSERT_EQ(linkRes, 0);
        auto gold = std::string {};
        ASSERT_TRUE(ReadFile<false>(sourcePathPrefix + config.outputFileName, gold));
        NormalizeGold(gold);
        auto ret = ExecPanda(targetPathPrefix + "linked.abc", "ets", config.entryPoint);
        ASSERT_EQ(ret.first, 0);
        ASSERT_EQ(ret.second, gold);
    } else {
        ASSERT_NE(linkRes, 0);
    }
}
#endif

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

        progDer.functionInstanceTable.emplace(func.name, std::move(func));

        ASSERT_TRUE(ark::pandasm::AsmEmitter::Emit(dervPath, progDer));
    }

    auto res = Link(DefaultConfig(), "data/multi/ForeignBase.linked.abc", {basePath, dervPath});
    ASSERT_TRUE(res.errors.empty()) << res.errors.front();
#endif
}

TEST(linkertests, StsHelloWorld)
{
#ifdef TEST_STATIC_LINKER_WITH_STS
    TestSts("hello_world", {"1.ets.abc"});
#endif
}

TEST(linkertests, StsForeignClass)
{
#ifdef TEST_STATIC_LINKER_WITH_STS
    TestSts("fclass", {"1.ets.abc", "2.ets.abc"});
#endif
}

TEST(linkertests, StsForeignMethod)
{
#ifdef TEST_STATIC_LINKER_WITH_STS
    TestSts("fmethod", {"1.ets.abc", "2.ets.abc"});
#endif
}

TEST(linkertests, StsFMethodOverloaded)
{
#ifdef TEST_STATIC_LINKER_WITH_STS
    TestSts("fmethod_overloaded", {"1.ets.abc", "2.ets.abc"});
#endif
}

TEST(linkertests, StsFMethodOverloaded2)
{
#ifdef TEST_STATIC_LINKER_WITH_STS
    TestSts("fmethod_overloaded_2", {"1.ets.abc", "2.ets.abc", "3.ets.abc", "4.ets.abc"});
#endif
}

TEST(linkertests, FilesInfo)
{
#ifdef TEST_STATIC_LINKER_WITH_STS
    TestSts("filesinfo", {"filesinfo.txt"});
#endif
}

TEST(linkertests, Mix)
{
#ifdef TEST_STATIC_LINKER_WITH_STS
    TestSts("mix", {"1.ets.abc", "2.ets.abc"});
#endif
}

TEST(linkertests, ClassCallNoDeleteDependency)
{
#ifdef TEST_STATIC_LINKER_WITH_STS
    TestStsConfig config;
    config.entryPoint = "dependency/ETSGLOBAL::main";
    TestSts("classcall_doublefile", {"dependency.ets.abc", "bedependent.ets.abc"}, true, {}, config);
#endif
}

TEST(linkertests, ClassCallDeleteDependency)
{
#ifdef TEST_STATIC_LINKER_WITH_STS
    StripOptions opts;
    opts.stripUnused = true;
    TestStsConfig config;
    config.entryPoint = "dependency/ETSGLOBAL::main";
    TestSts("classcall_doublefile", {"dependency.ets.abc", "bedependent.ets.abc"}, true, opts, config);
#endif
}

TEST(linkertests, ClassCallDeleteDependencyFromEntryInputError)
{
#ifdef TEST_STATIC_LINKER_WITH_STS
    StripOptions opts;
    opts.stripUnused = true;
    opts.stripUnusedSkiplist = "\"1/ETSGLOBAL\"";
    TestSts("classcall_doublefile", {"dependency.ets.abc", "bedependent.ets.abc"}, false, opts);
#endif
}

TEST(linkertests, MethodCallDeleteDependencyFromEntryInputError)
{
#ifdef TEST_STATIC_LINKER_WITH_STS
    StripOptions opts;
    opts.stripUnused = true;
    opts.stripUnusedSkiplist = "\"1/ETSGLOBAL/main\"";
    TestSts("classcall_doublefile", {"dependency.ets.abc", "bedependent.ets.abc"}, false, opts);
#endif
}

TEST(linkertests, CallDeleteDependencyFromConfigFileNotExist)
{
#ifdef TEST_STATIC_LINKER_WITH_STS
    StripOptions opts;
    opts.stripUnused = true;
    opts.stripUnusedSkiplist = "@data/ets/classcall_doublefile/methodconfig1.txt";
    TestSts("classcall_doublefile", {"dependency.ets.abc", "bedependent.ets.abc"}, false, opts);
#endif
}

TEST(linkertests, CallDeleteDependencyFromEntryFileNotExist)
{
#ifdef TEST_STATIC_LINKER_WITH_STS
    StripOptions opts;
    opts.stripUnused = true;
    opts.stripUnusedSkiplist = "\"1.ets\"";
    TestSts("classcall_doublefile", {"dependency.ets.abc", "bedependent.ets.abc"}, false, opts);
#endif
}

TEST(linkertests, MultiClassCallNoDeleteDependency)
{
#ifdef TEST_STATIC_LINKER_WITH_STS
    TestStsConfig config;
    config.entryPoint = "main_dependencyUser/ETSGLOBAL::main";
    TestSts("classcall_multifile",
            {"main_dependencyUser.ets.abc", "dependency.ets.abc", "user_dependency.ets.abc", "product_user.ets.abc"},
            true, {}, config);
#endif
}

TEST(linkertests, MultiClassCallDeleteDependency)
{
#ifdef TEST_STATIC_LINKER_WITH_STS
    StripOptions opts;
    opts.stripUnused = true;
    TestStsConfig config;
    config.entryPoint = "main_dependencyUser/ETSGLOBAL::main";
    TestSts("classcall_multifile",
            {"main_dependencyUser.ets.abc", "dependency.ets.abc", "user_dependency.ets.abc", "product_user.ets.abc"},
            true, opts, config);
#endif
}

TEST(linkertests, ClassCallDeleteDependencyAll)
{
#ifdef TEST_STATIC_LINKER_WITH_STS
    StripOptions opts;
    opts.stripUnused = true;
    opts.stripUnusedSkiplist = "*";
    TestStsConfig config;
    config.entryPoint = "dependency/ETSGLOBAL::main";
    TestSts("classcall_doublefile", {"dependency.ets.abc", "bedependent.ets.abc"}, true, opts, config);
#endif
}

TEST(linkertests, CallDeleteDependencyNoStripUnusedArg)
{
#ifdef TEST_STATIC_LINKER_WITH_STS
    StripOptions opts;
    opts.stripUnusedSkiplist = "\"dependency/ETSGLOBAL/main\"";
    TestSts("classcall_doublefile", {"dependency.ets.abc", "bedependent.ets.abc"}, false, opts);
#endif
}

#ifdef TEST_STATIC_LINKER_WITH_STS
std::string GenLinkCmd(const std::string &param)
{
    std::string prefix = "../../bin/ark_link";
    std::string suffix = " > /dev/null 2>&1";
    return prefix + param + suffix;
}
#endif

TEST(linkertests, TestForCoverage)
{
#ifdef TEST_STATIC_LINKER_WITH_STS
    std::string cmd = GenLinkCmd(" --error-option");
    // NOLINTNEXTLINE(cert-env33-c)
    auto linkRes = std::system(cmd.c_str());
    ASSERT_NE(linkRes, 0);

    cmd = GenLinkCmd(" --output data/ets/sys/target.abc");
    // NOLINTNEXTLINE(cert-env33-c)
    linkRes = std::system(cmd.c_str());
    ASSERT_NE(linkRes, 0);

    cmd = GenLinkCmd(" -- data/ets/sys/1.ets.abc data/ets/sys/2.ets.abc");
    // NOLINTNEXTLINE(cert-env33-c)
    linkRes = std::system(cmd.c_str());
    ASSERT_EQ(linkRes, 0);

    cmd = GenLinkCmd(" -- data/ets/sys/no_exist.abc");
    // NOLINTNEXTLINE(cert-env33-c)
    linkRes = std::system(cmd.c_str());
    ASSERT_NE(linkRes, 0);

    cmd = GenLinkCmd(" -- data/ets/sys/1.ets.abc data/ets/sys/2.ets.abc");
    // NOLINTNEXTLINE(cert-env33-c)
    linkRes = std::system(cmd.c_str());
    ASSERT_EQ(linkRes, 0);

    cmd = GenLinkCmd(" --output data/ets/sys/target.abc -- data/ets/sys/1.ets.abc data/ets/sys/2.ets.abc");
    // NOLINTNEXTLINE(cert-env33-c)
    linkRes = std::system(cmd.c_str());
    ASSERT_EQ(linkRes, 0);

    std::string opt = " --show-stats --version --log-level info";
    std::string dst = " --output data/ets/sys/target.abc -- data/ets/sys/1.ets.abc data/ets/sys/2.ets.abc";
    cmd = GenLinkCmd(opt + dst);
    // NOLINTNEXTLINE(cert-env33-c)
    linkRes = std::system(cmd.c_str());
    ASSERT_EQ(linkRes, 0);

    cmd = GenLinkCmd(" --output data/ets/sys/target.abc -- @data/ets/sys/no_exist.txt");
    // NOLINTNEXTLINE(cert-env33-c)
    linkRes = std::system(cmd.c_str());
    ASSERT_NE(linkRes, 0);

    cmd = GenLinkCmd(" --output data/ets/sys/target.abc -- @data/ets/sys/file_list_exist.txt");
    // NOLINTNEXTLINE(cert-env33-c)
    linkRes = std::system(cmd.c_str());
    ASSERT_EQ(linkRes, 0);

    cmd = GenLinkCmd(" --output data/ets/sys/target.abc -- @data/ets/sys/file_list_noexist.txt");
    // NOLINTNEXTLINE(cert-env33-c)
    linkRes = std::system(cmd.c_str());
    ASSERT_NE(linkRes, 0);

    cmd = GenLinkCmd(" --output data/ets/sys/target.abc -- @data/ets/sys/file_list_errorformat.txt");
    // NOLINTNEXTLINE(cert-env33-c)
    linkRes = std::system(cmd.c_str());
    ASSERT_NE(linkRes, 0);
#endif
}

constexpr uint32_t I = 32;
constexpr uint64_t L = 64;
constexpr float F = 11.1;
constexpr double D = 22.2;

TEST(linkertests, TestForReprValueItem)
{
    std::string sv1Expect = "32 as int";
    std::stringstream ss1;
    auto *sv1 = new ark::panda_file::ScalarValueItem(I);
    ark::static_linker::ReprValueItem(ss1, sv1);
    std::string sv1Res = ss1.str();
    delete sv1;
    ASSERT_TRUE(sv1Res == sv1Expect);

    std::string sv2Expect = "64 as long";
    std::stringstream ss2;
    auto *sv2 = new ark::panda_file::ScalarValueItem(L);
    ark::static_linker::ReprValueItem(ss2, sv2);
    std::string sv2Res = ss2.str();
    delete sv2;
    ASSERT_TRUE(sv2Res == sv2Expect);

    std::string sv3Expect = "11.1 as float";
    std::stringstream ss3;
    auto *sv3 = new ark::panda_file::ScalarValueItem(F);
    ark::static_linker::ReprValueItem(ss3, sv3);
    std::string sv3Res = ss3.str();
    delete sv3;
    ASSERT_TRUE(sv3Res == sv3Expect);

    std::string sv4Expect = "22.2 as double";
    std::stringstream ss4;
    auto *sv4 = new ark::panda_file::ScalarValueItem(D);
    ark::static_linker::ReprValueItem(ss4, sv4);
    std::string sv4Res = ss4.str();
    delete sv4;
    ASSERT_TRUE(sv4Res == sv4Expect);

    auto *sv5p = new ark::panda_file::ScalarValueItem(D);
    auto *sv5 = new ark::panda_file::ScalarValueItem(sv5p);
    std::stringstream ss5;
    ark::static_linker::ReprValueItem(ss5, sv5);
    std::string sv5Res = ss5.str();
    delete sv5p;
    delete sv5;
    ASSERT_EQ(sv5Res, sv4Expect);
}

TEST(linkertests, TestForReprArrayValueItem)
{
    std::string av1Expect = "[1 as int, 2 as int, 3 as int]";
    std::stringstream ss6;
    std::vector<ark::panda_file::ScalarValueItem> items;
    items.emplace_back(static_cast<uint32_t>(1));
    items.emplace_back(static_cast<uint32_t>(2));
    items.emplace_back(static_cast<uint32_t>(3));
    auto *av1 = new ark::panda_file::ArrayValueItem(ark::panda_file::Type(ark::panda_file::Type::TypeId::I32), items);
    ark::static_linker::ReprValueItem(ss6, av1);
    std::string av1Res = ss6.str();
    delete av1;
    ASSERT_TRUE(av1Res == av1Expect);
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST(linkertests, TestLoopSuperClass)
{
#ifdef TEST_STATIC_LINKER_WITH_STS
    // Write panda file to memory
    ark::panda_file::ItemContainer container;

    // set current class's superclass to self
    ark::panda_file::ClassItem *classItem = container.GetOrCreateClassItem("Bar");
    classItem->SetAccessFlags(ark::ACC_PUBLIC);
    classItem->SetSuperClass(classItem);

    // Add interface
    ark::panda_file::ClassItem *ifaceItem = container.GetOrCreateClassItem("Iface");
    ifaceItem->SetAccessFlags(ark::ACC_PUBLIC);
    classItem->AddInterface(ifaceItem);

    // Add method
    ark::panda_file::StringItem *methodName = container.GetOrCreateStringItem("foo");
    ark::panda_file::PrimitiveTypeItem *retType =
        container.GetOrCreatePrimitiveTypeItem(ark::panda_file::Type::TypeId::VOID);
    std::vector<ark::panda_file::MethodParamItem> params;
    ark::panda_file::ProtoItem *protoItem = container.GetOrCreateProtoItem(retType, params);
    classItem->AddMethod(methodName, protoItem, ark::ACC_PUBLIC | ark::ACC_STATIC, params);

    // Add field
    ark::panda_file::StringItem *fieldName = container.GetOrCreateStringItem("field");
    ark::panda_file::PrimitiveTypeItem *fieldType =
        container.GetOrCreatePrimitiveTypeItem(ark::panda_file::Type::TypeId::I32);
    classItem->AddField(fieldName, fieldType, ark::ACC_PUBLIC);

    // Add source file
    ark::panda_file::StringItem *sourceFile = container.GetOrCreateStringItem("source_file");
    classItem->SetSourceFile(sourceFile);
    auto writer = ark::panda_file::FileWriter("loop_super_class.abc");
    ASSERT_TRUE(container.Write(&writer));

    std::string cmd = GenLinkCmd(" -- loop_super_class.abc");
    // NOLINTNEXTLINE(cert-env33-c)
    auto linkRes = std::system(cmd.c_str());
    ASSERT_NE(linkRes, 0);
#endif
}

}  // namespace
