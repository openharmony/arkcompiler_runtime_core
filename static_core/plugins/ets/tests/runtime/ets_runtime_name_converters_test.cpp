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
#include <string>

#include "include/class_helper.h"
#include "include/runtime.h"
#include "libarkbase/utils/utf.h"

namespace ark::ets::test {

class EtsRuntimeNameConvertersTest : public testing::Test {
public:
    EtsRuntimeNameConvertersTest()
    {
        options_.SetShouldLoadBootPandaFiles(true);
        options_.SetShouldInitializeIntrinsics(false);
        options_.SetCompilerEnableJit(false);
        options_.SetGcType("epsilon");
        options_.SetLoadRuntimes({"ets"});

        const auto *stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to mock_stdlib.abc" << std::endl;
            std::abort();
        }
        options_.SetBootPandaFiles({stdlib});

        Runtime::Create(options_);
    }

    ~EtsRuntimeNameConvertersTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsRuntimeNameConvertersTest);
    NO_MOVE_SEMANTIC(EtsRuntimeNameConvertersTest);

private:
    RuntimeOptions options_;
};

TEST_F(EtsRuntimeNameConvertersTest, DescriptorToRuntimeName)
{
    auto runtimeName = ClassHelper::GetName(utf::CStringAsMutf8("Lstd/core/Object;"));
    ASSERT_STREQ(runtimeName.c_str(), "std.core.Object");

    runtimeName = ClassHelper::GetName(utf::CStringAsMutf8("[Lstd/core/Object;"));
    ASSERT_STREQ(runtimeName.c_str(), "std.core.Object[]");

    runtimeName = ClassHelper::GetName(utf::CStringAsMutf8("LA;"));
    ASSERT_STREQ(runtimeName.c_str(), "A");

    runtimeName = ClassHelper::GetName(utf::CStringAsMutf8("LA1/%%partial-A2;"));
    ASSERT_STREQ(runtimeName.c_str(), "A1.%%partial-A2");

    runtimeName = ClassHelper::GetName(utf::CStringAsMutf8("I"));
    ASSERT_STREQ(runtimeName.c_str(), "i32");

    runtimeName = ClassHelper::GetName(utf::CStringAsMutf8("[I"));
    ASSERT_STREQ(runtimeName.c_str(), "i32[]");

    runtimeName = ClassHelper::GetName(utf::CStringAsMutf8("{U[I[J[Lstd/core/String;}"));
    ASSERT_STREQ(runtimeName.c_str(), "{Ui32[],i64[],std.core.String[]}");

    runtimeName = ClassHelper::GetName(utf::CStringAsMutf8("{ULLLL/L;LLLL/N;}"));
    ASSERT_STREQ(runtimeName.c_str(), "{ULLL.L,LLL.N}");

    runtimeName = ClassHelper::GetName(utf::CStringAsMutf8("[[{U[I[J[Lstd/core/String;}"));
    ASSERT_STREQ(runtimeName.c_str(), "{Ui32[],i64[],std.core.String[]}[][]");

    runtimeName =
        ClassHelper::GetName(utf::CStringAsMutf8("{ULstd/core/String;[{ULstd/core/String;[Lstd/core/String;}}"));
    ASSERT_STREQ(runtimeName.c_str(), "{Ustd.core.String,{Ustd.core.String,std.core.String[]}[]}");
}

TEST_F(EtsRuntimeNameConvertersTest, GetDescriptor)
{
    auto compare = [](const char *runtimeName, std::string_view descriptor) {
        PandaString classDescriptor;
        auto res = ClassHelper::GetDescriptor(utf::CStringAsMutf8(runtimeName), &classDescriptor);
        ASSERT_TRUE(res != nullptr);
        ASSERT_STREQ(classDescriptor.c_str(), descriptor.data());
        ASSERT_STREQ(runtimeName, ClassHelper::GetName(utf::CStringAsMutf8(classDescriptor.c_str())).c_str());
    };

    compare("u", "Lu;");

    compare("i", "Li;");

    compare("std.core.Object", "Lstd/core/Object;");

    compare("std.core.Object[]", "[Lstd/core/Object;");

    compare("u1", "Z");

    compare("i32", "I");

    compare("i32[]", "[I");

    compare("{Ui32[],i64[],std.core.String[]}", "{U[I[J[Lstd/core/String;}");

    compare("{ULLL.L,LLL.N}", "{ULLLL/L;LLLL/N;}");

    compare("{Ui32[],i64[],std.core.String[]}[][]", "[[{U[I[J[Lstd/core/String;}");

    compare("{U{Ustd.core.String,std.core.String[]}[],std.core.String}",
            "{U[{ULstd/core/String;[Lstd/core/String;}Lstd/core/String;}");

    compare("{Ustd.core.String,{Ustd.core.String,std.core.String[]}[]}",
            "{ULstd/core/String;[{ULstd/core/String;[Lstd/core/String;}}");

    compare("{Ustd.core.String,{Ustd.core.String,std.core.String[]},f64[]}",
            "{ULstd/core/String;{ULstd/core/String;[Lstd/core/String;}[D}");
}

}  // namespace ark::ets::test
