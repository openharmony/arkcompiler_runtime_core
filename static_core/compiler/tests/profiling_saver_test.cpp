/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "unit_test.h"
#include "panda_runner.h"
#include "libpandabase/os/filesystem.h"
#include "utils/string_helpers.h"

namespace ark::test {
// NOLINTBEGIN(readability-magic-numbers)
class ProfilingSaverTest : public ::testing::Test {};

static constexpr auto SOURCE = R"(
.record E {}
.record A {}
.record B <extends=A> {}
.record C <extends=A> {}

.function i32 A.bar(A a0, i32 a1) {
	ldai 0x10
	jge a1, jump_label_0
	lda a1
	return
jump_label_0:
	newobj v0, E
    throw v0
}

.function i32 A.foo(A a0) {
	ldai 0x1
	return
}

.function i32 B.foo(B a0) {
	ldai 0x2
	return
}

.function i32 C.foo(C a0) {
    ldai 0x3
    return
}

.function void test() <static> {
    movi v0, 0x20
	movi v1, 0x0
	newobj v4, A
    newobj v5, B
    newobj v6, C
jump_label_4:
	lda v1
	jge v0, jump_label_3
	call.virt A.foo, v4
	call.virt B.foo, v5
	call.virt C.foo, v6
try_begin:
	call.short A.bar, v4, v1
try_end:
	jmp handler_begin_label_0_0
handler_begin_label_0_0:
	inci v1, 0x1
	jmp jump_label_4
jump_label_3:
	return.void

.catchall try_begin, try_end, handler_begin_label_0_0
}

.function void main() <static> {
    call.short test
    return.void
}
)";

std::vector<uint8_t> HexStringToBytes(const std::string &hex)
{
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i + 1 < hex.size(); i += 2) {  // CC-OFF(G.CNS.02-CPP) step length is 2
        std::string byteStr = hex.substr(i, 2);
        bytes.push_back(static_cast<uint8_t>(std::stoul(byteStr, nullptr, 16)));  // CC-OFF(G.CNS.02-CPP) hexadecimal
    }
    return bytes;
}

std::string ReadHexFile(const std::string &path)
{
    std::ifstream file(path);
    std::string hex;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        for (size_t i = 0; i + 1 < line.size(); ++i) {
            if (std::isxdigit(line[i]) != 0 && std::isxdigit(line[i + 1]) != 0) {
                hex += line[i];
                hex += line[i + 1];
                ++i;  // CC-OFF(G.EXP.41-CPP) necessary operation
            }
        }
    }
    return hex;
}

std::vector<uint8_t> ReadBinaryFile(const std::string &path)
{
    std::ifstream file(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(file), {}};
}

bool CompareBinaryAndHexFiles(const std::string &binaryPath, const std::string &hexPath)
{
    auto binary = ReadBinaryFile(binaryPath);
    auto hex = HexStringToBytes(ReadHexFile(hexPath));
    return binary == hex;
}

TEST_F(ProfilingSaverTest, ProfilingFileSaveCpp)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetShouldLoadBootPandaFiles(false);
    runner.GetRuntimeOptions().SetShouldInitializeIntrinsics(false);
    runner.GetRuntimeOptions().SetCompilerProfilingThreshold(0U);
    runner.GetRuntimeOptions().SetInterpreterType("cpp");

    uint32_t tid = os::thread::GetCurrentThreadId();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::string pgoFileName = helpers::string::Format("tmpfile1_%04x.ap", tid);
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static const std::string LOCATION = "/tmp";
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static const std::string PGO_FILE_PATH = LOCATION + "/" + pgoFileName;

    runner.GetRuntimeOptions().SetProfilesaverEnabled(true);
    runner.GetRuntimeOptions().SetProfileOutput(PGO_FILE_PATH);

    auto runtime = runner.CreateRuntime();
    runner.Run(runtime, SOURCE, std::vector<std::string> {});
    Runtime::Destroy();
    ASSERT_TRUE(os::IsFileExists(PGO_FILE_PATH));
    std::filesystem::path currentFile = __FILE__;
    static const std::string PGO_HEX_FILE_PATH = currentFile.parent_path() / "expected_profile.ap.hex";
    ASSERT_TRUE(CompareBinaryAndHexFiles(PGO_FILE_PATH, PGO_HEX_FILE_PATH));
}

TEST_F(ProfilingSaverTest, ProfilingFileSave)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetShouldLoadBootPandaFiles(false);
    runner.GetRuntimeOptions().SetShouldInitializeIntrinsics(false);
    runner.GetRuntimeOptions().SetCompilerProfilingThreshold(0U);

    uint32_t tid = os::thread::GetCurrentThreadId();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::string pgoFileName = helpers::string::Format("tmpfile2_%04x.ap", tid);
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static const std::string LOCATION = "/tmp";
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static const std::string PGO_FILE_PATH = LOCATION + "/" + pgoFileName;
    runner.GetRuntimeOptions().SetProfilesaverEnabled(true);
    runner.GetRuntimeOptions().SetProfileOutput(PGO_FILE_PATH);

    auto runtime = runner.CreateRuntime();
    runner.Run(runtime, SOURCE, std::vector<std::string> {});
    Runtime::Destroy();
    ASSERT_TRUE(os::IsFileExists(PGO_FILE_PATH));
    std::filesystem::path currentFile = __FILE__;
    static const std::string PGO_HEX_FILE_PATH = currentFile.parent_path() / "expected_profile.ap.hex";
    ASSERT_TRUE(CompareBinaryAndHexFiles(PGO_FILE_PATH, PGO_HEX_FILE_PATH));
}
// NOLINTEND(readability-magic-numbers)
}  // namespace ark::test
