/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_ANI_GTEST_H
#define PANDA_PLUGINS_ETS_ANI_GTEST_H

#include <gtest/gtest.h>
#include <cstdlib>
#include <vector>

#include "libpandabase/macros.h"
#include "plugins/ets/runtime/ani/ani.h"
#include "plugins/ets/runtime/napi/ets_napi.h"

namespace ark::ets::ani::testing {

class AniTest : public ::testing::Test {
public:
    void SetUp() override
    {
        const char *stdlib = std::getenv("ARK_ETS_STDLIB_PATH");
        ASSERT_NE(stdlib, nullptr);

        // Create boot-panda-files options
        std::string bootFileString = "--ext:boot-panda-files=" + std::string(stdlib);
        const char *abcPath = std::getenv("ANI_GTEST_ABC_PATH");
        if (abcPath != nullptr) {
            bootFileString += ":";
            bootFileString += abcPath;
        }

        ani_option bootFileOption = {bootFileString.data(), nullptr};

        std::vector<ani_option> options;
        options.push_back(bootFileOption);

        // Add extra test-specific ANI options
        for (auto &aniOpt : GetExtraAniOptions()) {
            options.push_back(aniOpt);
        }

        ani_options optionsPtr = {options.size(), options.data()};
        ASSERT_TRUE(ANI_CreateVM(&optionsPtr, ANI_VERSION_1, &vm_) == ANI_OK);

        // Get ANI API
        ASSERT_TRUE(vm_->GetEnv(ANI_VERSION_1, &env_) == ANI_OK) << "Cannot get ani env";
        uint32_t aniVersion;
        ASSERT_TRUE(env_->GetVersion(&aniVersion) == ANI_OK) << "Cannot get ani version";
        ASSERT_TRUE(aniVersion == ANI_VERSION_1) << "Incorrect ani version";

        // Get ETS Env
        ets_size nVms = 0;
        ASSERT_TRUE(ETS_GetCreatedVMs(&etsVm_, 1, &nVms) == ETS_OK) << "Cannot get ETS napi vm";
        ASSERT_TRUE(etsVm_->GetEnv(&etsEnv_, ETS_NAPI_VERSION_1_0) == ETS_OK) << "Cannot get ETS napi env";
    }

    void TearDown() override
    {
        ASSERT_TRUE(vm_->DestroyVM() == ANI_OK) << "Cannot destroy ANI VM";
    }

    template <typename R, typename... Args>
    R CallEtsClassStaticMethod(const std::string &className, const std::string &fnName, Args &&...args)
    {
        std::optional<R> result;
        CallEtsFunctionImpl(&result, className, fnName, std::forward<Args>(args)...);
        if constexpr (!std::is_same_v<R, void>) {
            return result.value();
        }
    }

    /// Call function with name `fnName` from ETSGLOBAL
    template <typename R, typename... Args>
    R CallEtsFunction(const std::string &moduleName, const std::string &fnName, Args &&...args)
    {
        std::string etsGlobalName(moduleName + "/ETSGLOBAL");
        return CallEtsClassStaticMethod<R>(etsGlobalName, fnName, std::forward<Args>(args)...);
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
    R CallEtsNativeMethod(const std::string &moduleName, const NativeFunction &fn, Args &&...args)
    {
        std::optional<R> result;

        CallEtsNativeMethodImpl(&result, moduleName, fn, std::forward<Args>(args)...);

        if constexpr (!std::is_same_v<R, void>) {
            return result.value();
        }
    }

    void GetStdString(ani_string str, std::string &result)
    {
        GetStdString(env_, str, result);
    }

    static void GetStdString(ani_env *env, ani_string str, std::string &result)
    {
        ani_size sz {};
        ASSERT_EQ(env->String_GetUTF8Size(str, &sz), ANI_OK);

        result.resize(sz + 1);
        ASSERT_EQ(env->String_GetUTF8SubString(str, 0, sz, result.data(), result.size(), &sz), ANI_OK);
        result.resize(sz);
    }

protected:
    virtual std::vector<ani_option> GetExtraAniOptions()
    {
        return {};
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
    void CallEtsFunctionImpl(std::optional<R> *result, const std::string &className, const std::string &fnName,
                             Args &&...args)
    {
        ets_class cls = etsEnv_->FindClass(className.c_str());
        ASSERT_NE(cls, nullptr) << GetFindClassFailureMsg(className) << " " << fnName;

        ets_method fn = etsEnv_->GetStaticp_method(cls, fnName.data(), nullptr);
        ASSERT_NE(fn, nullptr) << GetFindMethodFailureMsg(className, fnName);

        *result = DoCallEtsMethod<R>(cls, fn, std::forward<Args>(args)...);
    }

    template <typename R, typename... Args>
    void CallEtsNativeMethodImpl(std::optional<R> *result, const std::string &moduleName, const NativeFunction &fn,
                                 Args &&...args)
    {
        auto functionName = fn.GetName();
        std::string className(moduleName + "/ETSGLOBAL");
        auto cls = etsEnv_->FindClass(className.c_str());
        ASSERT_NE(cls, nullptr) << GetFindClassFailureMsg(className);

        auto mtd = etsEnv_->GetStaticp_method(cls, functionName, nullptr);
        ASSERT_NE(mtd, nullptr) << GetFindMethodFailureMsg(className, functionName);

        std::array<EtsNativeMethod, 1> method = {{{functionName, nullptr, fn.GetNativePtr()}}};

        ASSERT_EQ(etsEnv_->RegisterNatives(cls, method.data(), method.size()), ETS_OK)
            << "Failed to register native function " << functionName << ".";

        *result = DoCallEtsMethod<R>(cls, mtd, std::forward<Args>(args)...);

        ASSERT_EQ(etsEnv_->UnregisterNatives(cls), ETS_OK)
            << "Failed to unregister native function " << functionName << ".";
    }

    template <typename R, typename... Args>
    std::optional<R> DoCallEtsMethod(ets_class cls, ets_method mtd, Args &&...args)
    {
        // NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
        if constexpr (std::is_same_v<R, ets_boolean>) {
            return etsEnv_->CallStaticBooleanMethod(cls, mtd, std::forward<Args>(args)...);
        } else if constexpr (std::is_same_v<R, ets_byte>) {
            return etsEnv_->CallStaticByteMethod(cls, mtd, std::forward<Args>(args)...);
        } else if constexpr (std::is_same_v<R, ets_char>) {
            return etsEnv_->CallStaticCharMethod(cls, mtd, std::forward<Args>(args)...);
        } else if constexpr (std::is_same_v<R, ets_short>) {
            return etsEnv_->CallStaticShortMethod(cls, mtd, std::forward<Args>(args)...);
        } else if constexpr (std::is_same_v<R, ets_int>) {
            return etsEnv_->CallStaticIntMethod(cls, mtd, std::forward<Args>(args)...);
        } else if constexpr (std::is_same_v<R, ets_long>) {
            return etsEnv_->CallStaticLongMethod(cls, mtd, std::forward<Args>(args)...);
        } else if constexpr (std::is_same_v<R, ets_float>) {
            return etsEnv_->CallStaticFloatMethod(cls, mtd, std::forward<Args>(args)...);
        } else if constexpr (std::is_same_v<R, ets_double>) {
            return etsEnv_->CallStaticDoubleMethod(cls, mtd, std::forward<Args>(args)...);
        } else if constexpr (std::is_same_v<R, void>) {
            etsEnv_->CallStaticVoidMethod(cls, mtd, args...);
            return std::nullopt;
        } else if constexpr (std::is_same_v<R, ani_ref> || std::is_same_v<R, ani_tuple_value> ||
                             std::is_same_v<R, ani_object>) {
            return reinterpret_cast<R>(etsEnv_->CallStaticObjectMethod(cls, mtd, std::forward<Args>(args)...));
        } else {
            enum { INCORRECT_TEMPLATE_TYPE = false };
            static_assert(INCORRECT_TEMPLATE_TYPE, "Incorrect template type");
        }
        // NOLINTEND(cppcoreguidelines-pro-type-vararg)
        UNREACHABLE();
    }

protected:
    EtsEnv *etsEnv_ {nullptr};  // NOLINT(misc-non-private-member-variables-in-classes)
    ani_env *env_ {nullptr};    // NOLINT(misc-non-private-member-variables-in-classes)
    ani_vm *vm_ {nullptr};      // NOLINT(misc-non-private-member-variables-in-classes)

private:
    EtsVM *etsVm_ {nullptr};
};

}  // namespace ark::ets::ani::testing

#endif  // PANDA_PLUGINS_ETS_ANI_GTEST_H
