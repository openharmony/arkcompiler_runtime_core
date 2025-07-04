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

#include "libpandabase/macros.h"
#include "plugins/ets/runtime/ani/ani.h"
#include "plugins/ets/runtime/ani/ani_checkers.h"
#include "plugins/ets/runtime/ets_napi_env.h"
#include "plugins/ets/runtime/ani/scoped_objects_fix.h"

namespace ark::ets::test {

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

// CC-OFFNXT(G.PRE.02) should be with define
#define CHECK_PTR_ARG(arg) ANI_CHECK_RETURN_IF_EQ(arg, nullptr, ANI_INVALID_ARGS)

// CC-OFFNXT(G.PRE.02) should be with define
#define CHECK_ENV(env)                                                                           \
    do {                                                                                         \
        ANI_CHECK_RETURN_IF_EQ(env, nullptr, ANI_INVALID_ARGS);                                  \
        bool hasPendingException = ::ark::ets::PandaEnv::FromAniEnv(env)->HasPendingException(); \
        ANI_CHECK_RETURN_IF_EQ(hasPendingException, true, ANI_PENDING_ERROR);                    \
    } while (false)

// NOLINTEND(cppcoreguidelines-macro-usage)

class EtsNapiTestBaseClass : public testing::Test {
public:
    template <typename R, typename... Args>
    void CallEtsFunction(R *ret, std::string_view className, std::string_view methodName, Args &&...args)
    {
        ani_module module;
        auto status = env_->FindModule(className.data(), &module);
        ASSERT_EQ(status, ANI_OK);
        ani_function fn;
        status = env_->Module_FindFunction(module, methodName.data(), nullptr, &fn);
        ASSERT_EQ(status, ANI_OK);

        // NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
        if constexpr (std::is_same_v<R, ani_boolean>) {
            status = env_->Function_Call_Boolean(fn, ret, args...);
        } else if constexpr (std::is_same_v<R, ani_byte>) {
            status = env_->Function_Call_Byte(fn, ret, args...);
        } else if constexpr (std::is_same_v<R, ani_char>) {
            status = env_->Function_Call_Char(fn, ret, args...);
        } else if constexpr (std::is_same_v<R, ani_short>) {
            status = env_->Function_Call_Short(fn, ret, args...);
        } else if constexpr (std::is_same_v<R, ani_int>) {
            status = env_->Function_Call_Int(fn, ret, args...);
        } else if constexpr (std::is_same_v<R, ani_long>) {
            status = env_->Function_Call_Long(fn, ret, args...);
        } else if constexpr (std::is_same_v<R, ani_float>) {
            status = env_->Function_Call_Float(fn, ret, args...);
        } else if constexpr (std::is_same_v<R, ani_double>) {
            status = env_->Function_Call_Double(fn, ret, args...);
        } else if constexpr (std::is_same_v<R, ani_ref *> || std::is_same_v<R, ani_array> ||
                             std::is_same_v<R, ani_arraybuffer>) {
            status = env_->Function_Call_Ref(fn, reinterpret_cast<ani_ref *>(ret), args...);
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

        // Create boot-panda-files options
        std::string bootFileString = "--ext:boot-panda-files=" + std::string(stdlib);
        const char *abcPath = std::getenv("ANI_GTEST_ABC_PATH");
        if (abcPath != nullptr) {
            bootFileString += ":";
            bootFileString += abcPath;
        }

        // clang-format off
        std::vector<ani_option> optionsVector{
            {bootFileString.data(), nullptr},
            {"--ext:compiler-enable-jit=false", nullptr},
        };
        // clang-format on
        ani_options optionsPtr = {optionsVector.size(), optionsVector.data()};

        ASSERT_EQ(ANI_CreateVM(&optionsPtr, ANI_VERSION_1, &vm_), ANI_OK) << "Cannot create ETS VM";
        ASSERT_EQ(vm_->GetEnv(ANI_VERSION_1, &env_), ANI_OK) << "Cannot get ani env";
    }

    void TearDown() override
    {
        ASSERT_EQ(vm_->DestroyVM(), ANI_OK) << "Cannot destroy ETS VM";
    }

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    const char *abcPath_ {nullptr};
    ani_env *env_ {nullptr};
    ani_vm *vm_ {nullptr};
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

using AniFinalize = void (*)(void *, void *);

// CC-OFFNXT(G.FUD.06) solid logic, ODR
inline ani_status DoCreateExternalArrayBuffer(ani_env *env, void *externalData, size_t length, AniFinalize finalizeCB,
                                              void *finalizeHint, ani_arraybuffer *resultBuffer)
{
    CHECK_ENV(env);
    CHECK_PTR_ARG(externalData);
    ANI_CHECK_RETURN_IF_GT(length, static_cast<size_t>(std::numeric_limits<ani_int>::max()), ANI_INVALID_ARGS);
    CHECK_PTR_ARG(finalizeCB);
    CHECK_PTR_ARG(resultBuffer);

    ani::ScopedManagedCodeFix s(env);
    EtsCoroutine *coro = s.GetCoroutine();

    EtsEscompatArrayBuffer *internalArrayBuffer =
        EtsEscompatArrayBuffer::Create(coro, externalData, length, finalizeCB, finalizeHint);
    ANI_CHECK_RETURN_IF_EQ(internalArrayBuffer, nullptr, ANI_OUT_OF_MEMORY);
    ASSERT(coro->HasPendingException() == false);

    ani_ref arrayObject = nullptr;
    ani_status ret = s.AddLocalRef(internalArrayBuffer, &arrayObject);
    ANI_CHECK_RETURN_IF_NE(ret, ANI_OK, ret);
    ASSERT(arrayObject != nullptr);
    *resultBuffer = reinterpret_cast<ani_arraybuffer>(arrayObject);
    return ret;
}

inline ani_status ArrayBufferDetach(ani_env *env, ani_arraybuffer buffer)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(buffer);
    ani::ScopedManagedCodeFix s(env);
    auto *internalArrayBuffer = s.ToInternalType(buffer);
    if (!internalArrayBuffer->IsDetachable()) {
        return ANI_INVALID_ARGS;
    }
    internalArrayBuffer->Detach();
    return ANI_OK;
}

inline ani_status ArrayBufferIsDetached(ani_env *env, ani_arraybuffer buffer, bool *result)
{
    ANI_DEBUG_TRACE(env);
    CHECK_ENV(env);
    CHECK_PTR_ARG(buffer);
    CHECK_PTR_ARG(result);
    ani::ScopedManagedCodeFix s(env);
    auto *internalArrayBuffer = s.ToInternalType(buffer);
    *result = internalArrayBuffer->WasDetached();
    return ANI_OK;
}

inline void CheckUnhandledError(ani_env *env, ani_boolean shouldExist = ANI_FALSE)
{
    ani_boolean exists = ANI_FALSE;
    ASSERT_EQ(env->ExistUnhandledError(&exists), ANI_OK);
    ASSERT_EQ(exists, shouldExist);
}

}  // namespace ark::ets::test

#endif  // PANDA_PLUGINS_ETS_TESTS_NATIVE_NATIVE_TEST_HELPER_H
