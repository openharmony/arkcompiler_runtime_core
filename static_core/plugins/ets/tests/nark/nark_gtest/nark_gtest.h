/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_NARK_GTEST_H
#define PANDA_PLUGINS_ETS_NARK_GTEST_H

#include <gtest/gtest.h>
#include <cstdlib>

#include "libpandabase/macros.h"
#include "plugins/ets/runtime/napi/ets_napi.h"

namespace ark::ets::nark::testing {

class NarkTest : public ::testing::Test {
public:
    void SetUp() override
    {
        const char *stdlib = std::getenv("ARK_ETS_STDLIB_PATH");
        ASSERT_NE(stdlib, nullptr);

        std::vector<EtsVMOption> optionsVector {{EtsOptionType::ETS_BOOT_FILE, stdlib}};

        abcPath_ = std::getenv("NARK_GTEST_ABC_PATH");
        if (abcPath_.empty()) {
            std::cerr << "NARK_GTEST_ABC_PATH must be set" << std::endl;
            UNREACHABLE();
        }
        optionsVector.push_back({EtsOptionType::ETS_BOOT_FILE, abcPath_.c_str()});

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

    /// Call function with name `fnName` from ETSGLOBAL
    template <typename R, typename... Args>
    R CallEtsFunction(const std::string &fnName, Args &&...args)
    {
        std::optional<R> result;

        CallEtsFunctionImpl(&result, fnName, std::forward<Args>(args)...);

        if constexpr (!std::is_same_v<R, void>) {
            return result.value();
        }
    }

    class NativeFunction {
    public:
        template <typename FuncT>
        NativeFunction(const char *functionName, FuncT nativeFunction)
            : functionName_(functionName), nativeFunction_(reinterpret_cast<void *>(nativeFunction))
        {
        }

        const char *GetName() const
        {
            return functionName_;
        }

        void *GetNativePtr() const
        {
            return nativeFunction_;
        }

    private:
        const char *functionName_ {nullptr};
        void *nativeFunction_ {nullptr};
    };

    template <typename R, typename... Args>
    R CallEtsNativeMethod(const NativeFunction &fn, Args &&...args)
    {
        std::optional<R> result;

        CallEtsNativeMethodImpl(&result, fn, std::forward<Args>(args)...);

        if constexpr (!std::is_same_v<R, void>) {
            return result.value();
        }
    }

private:
    static std::string GetFindClassFailureMsg(const std::string &className)
    {
        std::stringstream ss;
        ss << "Failed to find class " << className << ".";
        return ss.str();
    }

    static std::string GetFindMethodFailureMsg(const std::string &className, const std::string &methodName)
    {
        std::stringstream ss;
        ss << "Failed to find method `" << methodName << "` in " << className << ".";
        return ss.str();
    }

    template <typename R, typename... Args>
    void CallEtsFunctionImpl(std::optional<R> *result, const std::string &fnName, Args &&...args)
    {
        auto className = "ETSGLOBAL";

        ets_class cls = env_->FindClass(className);
        ASSERT_NE(cls, nullptr) << GetFindClassFailureMsg(className);

        ets_method fn = env_->GetStaticp_method(cls, fnName.data(), nullptr);
        ASSERT_NE(fn, nullptr) << GetFindMethodFailureMsg(className, fnName);

        *result = DoCallEtsMethod<R>(cls, fn, std::forward<Args>(args)...);
    }

    template <typename R, typename... Args>
    void CallEtsNativeMethodImpl(std::optional<R> *result, const NativeFunction &fn, Args &&...args)
    {
        auto functionName = fn.GetName();
        auto className = "ETSGLOBAL";

        auto cls = env_->FindClass(className);
        ASSERT_NE(cls, nullptr) << GetFindClassFailureMsg(className);

        auto mtd = env_->GetStaticp_method(cls, functionName, nullptr);
        ASSERT_NE(mtd, nullptr) << GetFindMethodFailureMsg(className, functionName);

        std::array<EtsNativeMethod, 1> method = {{{functionName, nullptr, fn.GetNativePtr()}}};

        ASSERT_EQ(env_->RegisterNatives(cls, method.data(), method.size()), ETS_OK)
            << "Failed to register native function " << functionName << ".";

        *result = DoCallEtsMethod<R>(cls, mtd, std::forward<Args>(args)...);

        ASSERT_EQ(env_->UnregisterNatives(cls), ETS_OK)
            << "Failed to unregister native function " << functionName << ".";
    }

    template <typename R, typename... Args>
    std::optional<R> DoCallEtsMethod(ets_class cls, ets_method mtd, Args &&...args)
    {
        // NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
        if constexpr (std::is_same_v<R, ets_boolean>) {
            return env_->CallStaticBooleanMethod(cls, mtd, std::forward<Args>(args)...);
        } else if constexpr (std::is_same_v<R, ets_byte>) {
            return env_->CallStaticByteMethod(cls, mtd, std::forward<Args>(args)...);
        } else if constexpr (std::is_same_v<R, ets_char>) {
            return env_->CallStaticCharMethod(cls, mtd, std::forward<Args>(args)...);
        } else if constexpr (std::is_same_v<R, ets_short>) {
            return env_->CallStaticShortMethod(cls, mtd, std::forward<Args>(args)...);
        } else if constexpr (std::is_same_v<R, ets_int>) {
            return env_->CallStaticIntMethod(cls, mtd, std::forward<Args>(args)...);
        } else if constexpr (std::is_same_v<R, ets_long>) {
            return env_->CallStaticLongMethod(cls, mtd, std::forward<Args>(args)...);
        } else if constexpr (std::is_same_v<R, ets_float>) {
            return env_->CallStaticFloatMethod(cls, mtd, std::forward<Args>(args)...);
        } else if constexpr (std::is_same_v<R, ets_double>) {
            return env_->CallStaticDoubleMethod(cls, mtd, std::forward<Args>(args)...);
        } else if constexpr (std::is_same_v<R, void>) {
            env_->CallStaticVoidMethod(cls, mtd, args...);
            return std::nullopt;
        } else if constexpr (std::is_same_v<R, ets_object> || std::is_same_v<R, ets_array>) {
            return static_cast<R>(env_->CallStaticObjectMethod(cls, mtd, std::forward<Args>(args)...));
        } else {
            enum { INCORRECT_TEMPLATE_TYPE = false };
            static_assert(INCORRECT_TEMPLATE_TYPE, "Incorrect template type");
        }
        // NOLINTEND(cppcoreguidelines-pro-type-vararg)
        UNREACHABLE();
    }

protected:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    EtsEnv *env_ {nullptr};

private:
    std::string abcPath_;
    EtsVM *vm_ {nullptr};
};

}  // namespace ark::ets::nark::testing

#endif  // PANDA_PLUGINS_ETS_NARK_GTEST_H
