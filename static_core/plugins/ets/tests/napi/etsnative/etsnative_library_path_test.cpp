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

#include "ets_vm.h"
#include "include/mem/panda_string.h"
#include "plugins/ets/runtime/napi/ets_napi.h"

#include <cstdlib>
#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>
#include <ios>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

namespace ark::ets::test {

struct TestParams {
    bool expected;
    std::string libraryPath;
    std::string libraryFile;
    bool permissionCheck;
};

inline std::ostream &operator<<(std::ostream &out, const TestParams &val)
{
    std::ios_base::fmtflags f = out.flags();
    out << std::boolalpha                                          //-
        << "libraryPath is '" << val.libraryPath << "'"            //-
        << ", libraryFile is '" << val.libraryFile << "'"          //-
        << ", permissionCheck is '" << val.permissionCheck << "'"  //-
        << ", expected is " << std::boolalpha << val.expected;
    out.setf(f);
    return out;
}

class EtsNativeLibraryPathTest : public testing::TestWithParam<TestParams> {
protected:
    void SetUp() override
    {
        std::vector<EtsVMOption> optionsVector {
            {EtsOptionType::ETS_BOOT_FILE, std::getenv("ARK_ETS_STDLIB_PATH")},
            {EtsOptionType::ETS_NATIVE_LIBRARY_PATH, GetParam().libraryPath.data()},
        };
        EtsVMInitArgs vmArgs {
            ETS_NAPI_VERSION_1_0,
            static_cast<ets_int>(optionsVector.size()),
            optionsVector.data(),
        };
        ASSERT_TRUE(ETS_CreateVM(&vm_, &env_, &vmArgs) == ETS_OK) << "Cannot create ETS VM";
    }

    void TearDown() override
    {
        ASSERT_TRUE(vm_->DestroyEtsVM() == ETS_OK) << "Cannot destroy ETS VM";
    }

    inline EtsEnv *GetEnv()
    {
        return env_;
    }

    inline EtsVM *GetVm()
    {
        return vm_;
    }

private:
    EtsEnv *env_ {nullptr};
    EtsVM *vm_ {nullptr};
};

TEST_P(EtsNativeLibraryPathTest, EtsNativeTestLibraryPath)
{
    auto pandaVm = EtsCoroutine::GetCurrent()->GetPandaVM();
    ASSERT_EQ(pandaVm, GetVm());
    ASSERT_EQ(GetParam().expected, pandaVm->LoadNativeLibrary(GetEnv(), ConvertToString(GetParam().libraryFile),
                                                              GetParam().permissionCheck, ""))
        << GetParam();
}

constexpr auto TEST_LIBRARY_FILE = "libetsnapi_lib.so";

static std::string GetLibraryPath()
{
    const auto path = std::getenv("ARK_ETS_LIBRARY_PATH");
    return {path != nullptr ? path : ""};
}

INSTANTIATE_TEST_SUITE_P(  //-
    LibraryPathTests, EtsNativeLibraryPathTest,
    ::testing::Values(
        TestParams {
            true,
            GetLibraryPath(),
            TEST_LIBRARY_FILE,
            false,
        },
        TestParams {
            true,
            []() {
                std::stringstream ss {};
                ss << GetLibraryPath() << "/not_exists_dir"
                   << ":"  //-
                   << GetLibraryPath();
                return ss.str();
            }(),
            TEST_LIBRARY_FILE,
            false,
        },
        TestParams {
            false,
            {},
            TEST_LIBRARY_FILE,
            false,
        },
        TestParams {
            true,
            GetLibraryPath(),
            TEST_LIBRARY_FILE,
            true,
        }));

}  // namespace ark::ets::test
