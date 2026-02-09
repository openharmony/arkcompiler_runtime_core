/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <iostream>
#include <ostream>
#include <string>

#include "libarkbase/os/filesystem.h"
#include "runtime/include/runtime.h"
#include "assembly-parser.h"
#include "compiler/tests/unit_test.h"
#include "libarkbase/os/exec.h"
#include "libarkbase/os/file.h"

namespace ark::test {

class CodegenTypeIdTest : public testing::Test {
public:
    CodegenTypeIdTest()
    {
        std::string exePath = ark::os::file::File::GetExecutablePath().Value();
        auto buildPath = exePath.substr(0U, exePath.rfind('/'));
        paocPath_ = buildPath + "/bin/ark_aot";
        aotdumpPath_ = buildPath + "/bin/ark_aotdump";
        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to etsstdlib.abc" << std::endl;
            std::abort();
        }
        stdlibPath_ = stdlib;
        options_.SetBootPandaFiles({GetStdlibFile()});
        options_.SetLoadRuntimes({"ets"});
        options_.SetRunGcInPlace(true);
    }

    void SetPandaFile(const ark::arg_list_t abcPaths)
    {
        options_.SetPandaFiles(abcPaths);
    }

    void SetAotFile(const char *aotPath)
    {
        options_.SetAotFile(aotPath);
    }

    std::pair<int, std::string> ExecutePanda(const char *file, const char *entryPoint)
    {
        Runtime::Create(options_);
        auto *runtime = ark::Runtime::GetCurrent();

        std::stringstream strBuf;
        auto old = std::cerr.rdbuf(strBuf.rdbuf());
        auto reset = [&old](auto *cout) { cout->rdbuf(old); };
        auto guard = std::unique_ptr<std::ostream, decltype(reset)>(&std::cerr, reset);

        auto res = runtime->ExecutePandaFile(file, entryPoint, {});
        if (!res) {
            return {1, "error " + std::to_string((int)res.Error())};
        }
        return {res.Value(), strBuf.str()};
    }

    const char *GetPaocFile() const
    {
        return paocPath_.c_str();
    }

    const char *GetStdlibFile() const
    {
        return stdlibPath_.c_str();
    }

    ~CodegenTypeIdTest() override
    {
        Runtime::Destroy();
    }

    void CheckAotdump(const std::string &aotFilename)
    {
        compiler::TmpFile tmpfile("./aotdump.tmp");

        EXPECT_TRUE(os::IsFileExists(aotFilename)) << "AN file not found: " << os::GetAbsolutePath(aotFilename);
        auto res = os::exec::Exec(aotdumpPath_.c_str(), "--show-code=disasm", "--output-file", tmpfile.GetFileName(),
                                  aotFilename.c_str());
        ASSERT_TRUE(res) << "aotdump failed with error: " << res.Error().ToString();
        ASSERT_EQ(res.Value(), 0U) << "aotdump return error code: " << res.Value();

        // Check that EncodeLoadAndInitClassInAot was called during aot compilation for inlined ext method
        std::ifstream dumpFile(tmpfile.GetFileName());
        std::regex rgxInlineInfo(".*InlineInfo .*method: f_ext.B f_ext.A::B().*");
        bool found = false;
        for (std::string line; std::getline(dumpFile, line);) {
            if (line.rfind("InlineInfo") != std::string::npos) {
                if (std::regex_match(line, rgxInlineInfo)) {
                    found = true;
                    break;
                }
            }
        }
        ASSERT_EQ(found, true) << "TypeIds matched for LoadAndInitClass";
    }

    NO_COPY_SEMANTIC(CodegenTypeIdTest);
    NO_MOVE_SEMANTIC(CodegenTypeIdTest);

private:
    RuntimeOptions options_;
    std::string paocPath_;
    std::string aotdumpPath_;
    std::string stdlibPath_;
};

static constexpr auto MAIN_FILE_SOURCE = R"(
.language eTS

.record pad1 { i32 v }
.record pad2 { i32 v }
.record pad3 { i32 v }
.record pad4 { i32 v }
.record pad5 { i32 v }
.record pad6 { i32 v }
.record pad7 { i32 v }
.record pad8 { i32 v }
.record pad9 { i32 v }
.record pad10 { i32 v }
.record pad11

.record std.core.Class <external>

.record std.core.String <external>

.record f_main

.record f_ext.A <external>

.record f_ext.B <external> {
	i32 c <external, access.field=public>
}

.function i32 f_main.main_1() <static, access.function=public> {
    call.short f_ext.A.B:()
    sta.obj v0
    call.short f_ext.B.foo:()
    sta v1
    lda.type f_ext.B
    call.acc.short std.core.Class.getName:(std.core.Class), v0, 0x0
    call.acc.short std.core.String.%%get-length:(std.core.String), v0, 0x0
    sta v2
    sub v1, v2
    sta v1
    ldobj.v v0, v0, f_ext.B.c
    sub v1, v0
    return
}

.function f_ext.B f_ext.A.B() <external, static, access.function=public>

.function i32 f_ext.B.foo() <external, static, access.function=public>

.function std.core.String std.core.Class.getName(std.core.Class a0) <external, access.function=public>

.function i32 std.core.String.%%get-length(std.core.String a0) <external, access.function=public>
)";

static constexpr auto EXTERNAL_FILE_SOURCE = R"(
.language eTS

.record std.core.Class <external>

.record std.core.Int <external>

.record std.core.Object <external>

.record std.core.String <external>

.record f_ext.A <access.record=public> {
}

.record f_ext.B <access.record=public> {
    i32 c <access.field=public>
}

.record f_ext

.function f_ext.B f_ext.A.B() <static, access.function=public> {
    initobj.short f_ext.B._ctor_:(f_ext.B)
    return.obj
}

.function i32 f_ext.B.foo() <static, access.function=public> {
    lda.type f_ext.B
    call.acc.short std.core.Class.getName:(std.core.Class), v0, 0x0
    call.acc.short std.core.String.%%get-length:(std.core.String), v0, 0x0
    return
}

.function std.core.String std.core.Class.getName(std.core.Class a0) <external, access.function=public>

.function void std.core.Object._ctor_(std.core.Object a0) <ctor, external, access.function=public>

.function i32 std.core.String.%%get-length(std.core.String a0) <external, access.function=public>

.function void f_ext.A._ctor_(f_ext.A a0) <ctor, access.function=public> {
    call.short std.core.Object._ctor_:(std.core.Object), a0
    return.void
}

.function void f_ext.B._ctor_(f_ext.B a0) <ctor, access.function=public> {
    call.short std.core.Object._ctor_:(std.core.Object), a0
    ldai 0x0
    stobj a0, f_ext.B.c
    return.void
}

)";

TEST_F(CodegenTypeIdTest, CodegenTypeIdComparisonConflict)
{
    compiler::TmpFile mainFile("test_main.abc");
    compiler::TmpFile externalFile("external_test.abc");
    // Without "./" there is strange behavior on ARM64 of ark_aotdump loading this file - it fails on dlopen
    compiler::TmpFile aotFile("./test_main.an");

    // Main file
    {
        pandasm::Parser p;
        auto res = p.Parse(MAIN_FILE_SOURCE);
        auto err = p.ShowError();
        ASSERT_EQ(err.err, pandasm::Error::ErrorType::ERR_NONE)
            << err.message << " (" << err.verbose << "); line: " << err.lineNumber;
        ASSERT_TRUE(pandasm::AsmEmitter::Emit(mainFile.GetFileName(), res.Value()))
            << pandasm::AsmEmitter::GetLastError();
    }

    // External file
    {
        pandasm::Parser p;
        auto res = p.Parse(EXTERNAL_FILE_SOURCE);
        auto err = p.ShowError();
        ASSERT_EQ(err.err, pandasm::Error::ErrorType::ERR_NONE)
            << err.message << " (" << err.verbose << "); line: " << err.lineNumber;
        ASSERT_TRUE(pandasm::AsmEmitter::Emit(externalFile.GetFileName(), res.Value()))
            << pandasm::AsmEmitter::GetLastError();
    }

    // Compile AOT
    {
        auto res =
            os::exec::Exec(GetPaocFile(), "--boot-panda-files", GetStdlibFile(), "--load-runtimes=ets",
                           "--paoc-panda-files", mainFile.GetFileName(), "--panda-files", externalFile.GetFileName(),
                           "--paoc-output", aotFile.GetFileName(), "--log-error=runtime");
        ASSERT_TRUE(res) << "paoc failed with error: " << res.Error().ToString();
        ASSERT_EQ(res.Value(), 0U);
        CheckAotdump(aotFile.GetFileName());
    }

    SetPandaFile({externalFile.GetFileName(), mainFile.GetFileName()});
    SetAotFile(aotFile.GetFileName());

    testing::internal::CaptureStderr();
    auto ret = ExecutePanda(mainFile.GetFileName(), "f_main::main_1");
    std::string log = testing::internal::GetCapturedStderr();
    ASSERT_EQ(ret.first, 0) << ret.second << log;
}

}  // namespace ark::test
