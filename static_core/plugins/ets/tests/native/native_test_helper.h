/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

#ifndef PANDA_PLUGINS_ETS_TESTS_NATIVE_NATIVE_TEST_HELPER_H
#define PANDA_PLUGINS_ETS_TESTS_NATIVE_NATIVE_TEST_HELPER_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "plugins/ets/runtime/napi/ets_scoped_objects_fix.h"

namespace ark::ets::test {

class EtsNapiTestBaseClass : public testing::Test {
public:
    template <typename R, typename... Args>
    void CallEtsFunction(R *ret, std::string_view className, std::string_view methodName, Args &&...args)
    {
        std::string etsGlobalName(std::string(className) + "/ETSGLOBAL");
        ets_class cls = env_->FindClass(etsGlobalName.data());
        ASSERT_NE(cls, nullptr);
        ets_method fn = env_->GetStaticp_method(cls, methodName.data(), nullptr);

        // NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
        if constexpr (std::is_same_v<R, ets_boolean>) {
            *ret = env_->CallStaticBooleanMethod(cls, fn, args...);
        } else if constexpr (std::is_same_v<R, ets_byte>) {
            *ret = env_->CallStaticByteMethod(cls, fn, args...);
        } else if constexpr (std::is_same_v<R, ets_char>) {
            *ret = env_->CallStaticCharMethod(cls, fn, args...);
        } else if constexpr (std::is_same_v<R, ets_short>) {
            *ret = env_->CallStaticShortMethod(cls, fn, args...);
        } else if constexpr (std::is_same_v<R, ets_int>) {
            *ret = env_->CallStaticIntMethod(cls, fn, args...);
        } else if constexpr (std::is_same_v<R, ets_long>) {
            *ret = env_->CallStaticLongMethod(cls, fn, args...);
        } else if constexpr (std::is_same_v<R, ets_float>) {
            *ret = env_->CallStaticFloatMethod(cls, fn, args...);
        } else if constexpr (std::is_same_v<R, ets_double>) {
            *ret = env_->CallStaticDoubleMethod(cls, fn, args...);
        } else if constexpr (std::is_same_v<R, ets_object> || std::is_same_v<R, ets_array> ||
                             std::is_same_v<R, ets_arraybuffer>) {
            *ret = static_cast<R>(env_->CallStaticObjectMethod(cls, fn, args...));
        } else if constexpr (std::is_same_v<R, void>) {
            // do nothing
        } else {
            enum { INCORRECT_TEMPLATE_TYPE = false };
            static_assert(INCORRECT_TEMPLATE_TYPE, "Incorrect template type");
        }
        // NOLINTEND(cppcoreguidelines-pro-type-vararg)
    }

protected:
    void SetUp() override
    {
        const char *stdlib = std::getenv("ARK_ETS_STDLIB_PATH");
        ASSERT_NE(stdlib, nullptr);

        // clang-format off
        std::vector<EtsVMOption> optionsVector{
            {EtsOptionType::ETS_GC_TYPE, "epsilon"},
            {EtsOptionType::ETS_NO_JIT, nullptr},
            {EtsOptionType::ETS_BOOT_FILE, stdlib}
        };
        // clang-format on

        abcPath_ = std::getenv("ANI_GTEST_ABC_PATH");
        if (abcPath_ != nullptr) {
            optionsVector.push_back({EtsOptionType::ETS_BOOT_FILE, abcPath_});
        }

        EtsVMInitArgs vmArgs;
        vmArgs.version = ETS_NAPI_VERSION_1_0;
        vmArgs.options = optionsVector.data();
        vmArgs.nOptions = static_cast<ets_int>(optionsVector.size());

        ASSERT_TRUE(ETS_CreateVM(&vm_, &env_, &vmArgs) == ETS_OK) << "Cannot create ETS VM";
    }

    void TearDown() override
    {
        ASSERT_TRUE(vm_->DestroyEtsVM() == ETS_OK) << "Cannot destroy ETS VM";
    }

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    const char *abcPath_ {nullptr};
    EtsEnv *env_ {nullptr};
    EtsVM *vm_ {nullptr};
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

}  // namespace ark::ets::test

#endif  // PANDA_PLUGINS_ETS_TESTS_NATIVE_NATIVE_TEST_HELPER_H
