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

#include "plugins/ets/runtime/ani/ani.h"
#include <regex>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::test {

class StackTraceTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        if (std::getenv("PANDA_STD_LIB") == nullptr) {
            std::cerr << "PANDA_STD_LIB not set" << std::endl;
            std::abort();
        }
    }

    void SetUp() override
    {
        std::string bootPandaFilesOption = std::string("--ext:boot-panda-files=") + std::getenv("PANDA_STD_LIB");
        ani_option option {bootPandaFilesOption.c_str(), nullptr};
        ani_options opts {1, &option};
        ASSERT_EQ(ANI_CreateVM(&opts, ANI_VERSION_1, &vm_), ANI_OK) << "Cannot create ETS VM";
        ASSERT_EQ(vm_->GetEnv(ANI_VERSION_1, &env_), ANI_OK) << "Cannot get ani env";
    }

    void TearDown() override
    {
        ASSERT_EQ(vm_->DestroyVM(), ANI_OK) << "Cannot destroy ETS VM";
    }

    ani_env *GetEnv() const
    {
        return env_;
    }

private:
    ani_env *env_ {};
    ani_vm *vm_ {};
};

TEST_F(StackTraceTest, GetMainThreadStackTrace)
{
    static constexpr const char *CLASS_DESCRIPTOR = "std.dfx.StackTraceHelper";
    static constexpr const char *METHOD_NAME = "getMainThreadStackTrace";
    static constexpr const char *SIGNATURE = ":C{std.core.String}";

    ani_class klass {};
    auto *env = GetEnv();
    ASSERT_EQ(env->FindClass(CLASS_DESCRIPTOR, &klass), ANI_OK);

    ani_ref aniRef {};
    ASSERT_EQ(env->Class_CallStaticMethodByName_Ref(klass, METHOD_NAME, SIGNATURE, &aniRef), ANI_OK);

    auto aniString = reinterpret_cast<ani_string>(aniRef);
    std::string stack;
    ani_size sz {};
    ASSERT_EQ(env->String_GetUTF8Size(aniString, &sz), ANI_OK);
    stack.resize(sz + 1);
    ASSERT_EQ(env->String_GetUTF8SubString(aniString, 0, sz, stack.data(), stack.size(), &sz), ANI_OK);
    stack.resize(sz);

    std::stringstream ss(stack);
    std::regex pattern(R"(\s*at .*(\d+):(\d+)\))");
    std::string token;

    while (std::getline(ss, token, '\n')) {
        ASSERT_TRUE(std::regex_match(token, pattern));
    }
}

}  // namespace ark::ets::test
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
