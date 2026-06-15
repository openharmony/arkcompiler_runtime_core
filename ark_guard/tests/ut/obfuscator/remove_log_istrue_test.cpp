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

#include <filesystem>
#include <limits>
#include <string>

#include <gtest/gtest.h>

#include "abckit_wrapper/file_view.h"
#include "configuration/configuration.h"
#include "libabckit/c/abckit.h"
#include "libabckit/src/metadata_inspect_impl.h"
#include "obfuscator/obfuscator.h"
#include "static_core/assembler/assembly-program.h"

using namespace testing::ext;

namespace {
constexpr const char *ABC_PATH = ARK_GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/remove_log_istrue_test.abc";
constexpr const char *OBF_ABC_PATH =
    ARK_GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/remove_log_istrue_test_updated.abc";
constexpr const char *NAME_CACHE_PATH = ARK_GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/remove_log_istrue_test.json";
// Matches compiler ACC_REG_ID (INVALID_REG_ID - 1) without pulling in compiler headers.
constexpr uint16_t ACC_REG_ID = static_cast<uint16_t>(std::numeric_limits<uint8_t>::max()) - 1U;

// After obfuscation names may change, so scan all functions for ets.istrue.
void VerifyAllEtsIstrueUseVregNotAcc(ark::pandasm::Program *prog)
{
    size_t totalIstrueCount = 0;
    for (const auto &[key, func] : prog->functionStaticTable) {
        for (const auto &ins : func.ins) {
            if (ins.opcode != ark::pandasm::Opcode::ETS_ISTRUE) {
                continue;
            }
            ++totalIstrueCount;
            ASSERT_EQ(ins.RegSize(), 1U) << key;
            const uint16_t reg = ins.GetReg(0U);
            EXPECT_NE(reg, ACC_REG_ID) << "ets.istrue must read vreg, not acc: " << key;
            EXPECT_LT(reg, func.regsNum) << "ets.istrue vreg out of range: " << key;
        }
    }
    ASSERT_GE(totalIstrueCount, 1U) << "expected at least one ets.istrue in obfuscated abc";
}

void VerifyObfuscatedAbcIstrueBytecode(const std::string &abcPath)
{
    auto api = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
    ASSERT_NE(api, nullptr);
    AbckitFile *file = api->openAbc(abcPath.c_str(), abcPath.size());
    ASSERT_NE(file, nullptr);
    ASSERT_EQ(api->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *prog = reinterpret_cast<AbckitFile *>(file)->GetStaticProgram();
    VerifyAllEtsIstrueUseVregNotAcc(prog);

    api->closeFile(file);
    ASSERT_EQ(api->getLastError(), ABCKIT_STATUS_NO_ERROR);
}
}  // namespace

/**
 * @tc.name: remove_log_istrue_test_001
 * @tc.desc: remove log must keep ets.istrue reading object vreg after graph rewrite
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(RemoveLogIstrueTest, remove_log_istrue_test_001, TestSize.Level1)
{
    std::filesystem::remove(OBF_ABC_PATH);
    std::filesystem::remove(NAME_CACHE_PATH);

    abckit_wrapper::FileView fileView;
    ASSERT_EQ(fileView.Init(ABC_PATH), AbckitWrapperErrorCode::ERR_SUCCESS);

    ark::guard::Configuration config {};
    config.defaultNameCachePath = NAME_CACHE_PATH;
    config.obfuscationRules.removeLog = true;

    ark::guard::Obfuscator obfuscator(config);
    ASSERT_TRUE(obfuscator.Execute(fileView));

    fileView.Save(OBF_ABC_PATH);
    ASSERT_TRUE(std::filesystem::exists(OBF_ABC_PATH));

    VerifyObfuscatedAbcIstrueBytecode(OBF_ABC_PATH);
}
