/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_TESTS_MOCK_CALLING_METHODS_TEST_HELPER_H
#define PANDA_PLUGINS_ETS_TESTS_MOCK_CALLING_METHODS_TEST_HELPER_H

#include <gtest/gtest.h>

#include "libpandabase/utils/utils.h"
#include "plugins/ets/tests/mock/mock_test_helper.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
namespace ark::ets::test {

class CallingMethodsTestBase : public MockEtsNapiTestBaseClass {
protected:
    CallingMethodsTestBase() = default;
    explicit CallingMethodsTestBase(const char *testBinFileName) : MockEtsNapiTestBaseClass(testBinFileName) {};
};

[[maybe_unused]] static void CallVoidMethodListHelper(EtsEnv *env, ets_object obj, ets_method methodId, ...)
{
    std::va_list args;
    va_start(args, methodId);
    env->CallVoidMethodList(obj, methodId, args);
}

[[maybe_unused]] static ets_object CallObjectMethodListHelper(EtsEnv *env, ets_object obj, ets_method methodId, ...)
{
    std::va_list args;
    va_start(args, methodId);
    return env->CallObjectMethodList(obj, methodId, args);
}

[[maybe_unused]] static ets_boolean CallBooleanMethodListHelper(EtsEnv *env, ets_object obj, ets_method methodId, ...)
{
    std::va_list args;
    va_start(args, methodId);
    return env->CallBooleanMethodList(obj, methodId, args);
}

[[maybe_unused]] static ets_byte CallByteMethodListHelper(EtsEnv *env, ets_object obj, ets_method methodId, ...)
{
    std::va_list args;
    va_start(args, methodId);
    return env->CallByteMethodList(obj, methodId, args);
}

[[maybe_unused]] static ets_char CallCharMethodListHelper(EtsEnv *env, ets_object obj, ets_method methodId, ...)
{
    std::va_list args;
    va_start(args, methodId);
    return env->CallCharMethodList(obj, methodId, args);
}

[[maybe_unused]] static ets_short CallShortMethodListHelper(EtsEnv *env, ets_object obj, ets_method methodId, ...)
{
    std::va_list args;
    va_start(args, methodId);
    return env->CallShortMethodList(obj, methodId, args);
}

[[maybe_unused]] static ets_int CallIntMethodListHelper(EtsEnv *env, ets_object obj, ets_method methodId, ...)
{
    std::va_list args;
    va_start(args, methodId);
    return env->CallIntMethodList(obj, methodId, args);
}

[[maybe_unused]] static ets_long CallLongMethodListHelper(EtsEnv *env, ets_object obj, ets_method methodId, ...)
{
    std::va_list args;
    va_start(args, methodId);
    return env->CallLongMethodList(obj, methodId, args);
}

[[maybe_unused]] static ets_float CallFloatMethodListHelper(EtsEnv *env, ets_object obj, ets_method methodId, ...)
{
    std::va_list args;
    va_start(args, methodId);
    return env->CallFloatMethodList(obj, methodId, args);
}

[[maybe_unused]] static ets_double CallDoubleMethodListHelper(EtsEnv *env, ets_object obj, ets_method methodId, ...)
{
    std::va_list args;
    va_start(args, methodId);
    return env->CallDoubleMethodList(obj, methodId, args);
}

[[maybe_unused]] static void CallNonvirtualVoidMethodListHelper(EtsEnv *env, ets_object obj, ets_class cls,
                                                                ets_method methodId, ...)
{
    std::va_list args;
    va_start(args, methodId);
    env->CallNonvirtualVoidMethodList(obj, cls, methodId, args);
}

[[maybe_unused]] static ets_object CallNonvirtualObjectMethodListHelper(EtsEnv *env, ets_object obj, ets_class cls,
                                                                        ets_method methodId, ...)
{
    std::va_list args;
    va_start(args, methodId);
    return env->CallNonvirtualObjectMethodList(obj, cls, methodId, args);
}

[[maybe_unused]] static ets_boolean CallNonvirtualBooleanMethodListHelper(EtsEnv *env, ets_object obj, ets_class cls,
                                                                          ets_method methodId, ...)
{
    std::va_list args;
    va_start(args, methodId);
    return env->CallNonvirtualBooleanMethodList(obj, cls, methodId, args);
}

[[maybe_unused]] static ets_byte CallNonvirtualByteMethodListHelper(EtsEnv *env, ets_object obj, ets_class cls,
                                                                    ets_method methodId, ...)
{
    std::va_list args;
    va_start(args, methodId);
    return env->CallNonvirtualByteMethodList(obj, cls, methodId, args);
}

[[maybe_unused]] static ets_char CallNonvirtualCharMethodListHelper(EtsEnv *env, ets_object obj, ets_class cls,
                                                                    ets_method methodId, ...)
{
    std::va_list args;
    va_start(args, methodId);
    return env->CallNonvirtualCharMethodList(obj, cls, methodId, args);
}

[[maybe_unused]] static ets_short CallNonvirtualShortMethodListHelper(EtsEnv *env, ets_object obj, ets_class cls,
                                                                      ets_method methodId, ...)
{
    std::va_list args;
    va_start(args, methodId);
    return env->CallNonvirtualShortMethodList(obj, cls, methodId, args);
}

[[maybe_unused]] static ets_int CallNonvirtualIntMethodListHelper(EtsEnv *env, ets_object obj, ets_class cls,
                                                                  ets_method methodId, ...)
{
    std::va_list args;
    va_start(args, methodId);
    return env->CallNonvirtualIntMethodList(obj, cls, methodId, args);
}

[[maybe_unused]] static ets_long CallNonvirtualLongMethodListHelper(EtsEnv *env, ets_object obj, ets_class cls,
                                                                    ets_method methodId, ...)
{
    std::va_list args;
    va_start(args, methodId);
    return env->CallNonvirtualLongMethodList(obj, cls, methodId, args);
}

[[maybe_unused]] static ets_float CallNonvirtualFloatMethodListHelper(EtsEnv *env, ets_object obj, ets_class cls,
                                                                      ets_method methodId, ...)
{
    std::va_list args;
    va_start(args, methodId);
    return env->CallNonvirtualFloatMethodList(obj, cls, methodId, args);
}

[[maybe_unused]] static ets_double CallNonvirtualDoubleMethodListHelper(EtsEnv *env, ets_object obj, ets_class cls,
                                                                        ets_method methodId, ...)
{
    std::va_list args;
    va_start(args, methodId);
    return env->CallNonvirtualDoubleMethodList(obj, cls, methodId, args);
}

[[maybe_unused]] static void CallStaticVoidMethodListHelper(EtsEnv *env, ets_class cls, ets_method methodId, ...)
{
    std::va_list args;
    va_start(args, methodId);
    env->CallStaticVoidMethodList(cls, methodId, args);
}

[[maybe_unused]] static ets_object CallStaticObjectMethodListHelper(EtsEnv *env, ets_class cls, ets_method methodId,
                                                                    ...)
{
    std::va_list args;
    va_start(args, methodId);
    return env->CallStaticObjectMethodList(cls, methodId, args);
}

[[maybe_unused]] static ets_boolean CallStaticBooleanMethodListHelper(EtsEnv *env, ets_class cls, ets_method methodId,
                                                                      ...)
{
    std::va_list args;
    va_start(args, methodId);
    return env->CallStaticBooleanMethodList(cls, methodId, args);
}

[[maybe_unused]] static ets_byte CallStaticByteMethodListHelper(EtsEnv *env, ets_class cls, ets_method methodId, ...)
{
    std::va_list args;
    va_start(args, methodId);
    return env->CallStaticByteMethodList(cls, methodId, args);
}

[[maybe_unused]] static ets_char CallStaticCharMethodListHelper(EtsEnv *env, ets_class cls, ets_method methodId, ...)
{
    std::va_list args;
    va_start(args, methodId);
    return env->CallStaticCharMethodList(cls, methodId, args);
}

[[maybe_unused]] static ets_short CallStaticShortMethodListHelper(EtsEnv *env, ets_class cls, ets_method methodId, ...)
{
    std::va_list args;
    va_start(args, methodId);
    return env->CallStaticShortMethodList(cls, methodId, args);
}

[[maybe_unused]] static ets_int CallStaticIntMethodListHelper(EtsEnv *env, ets_class cls, ets_method methodId, ...)
{
    std::va_list args;
    va_start(args, methodId);
    return env->CallStaticIntMethodList(cls, methodId, args);
}

[[maybe_unused]] static ets_long CallStaticLongMethodListHelper(EtsEnv *env, ets_class cls, ets_method methodId, ...)
{
    std::va_list args;
    va_start(args, methodId);
    return env->CallStaticLongMethodList(cls, methodId, args);
}

[[maybe_unused]] static ets_float CallStaticFloatMethodListHelper(EtsEnv *env, ets_class cls, ets_method methodId, ...)
{
    std::va_list args;
    va_start(args, methodId);
    return env->CallStaticFloatMethodList(cls, methodId, args);
}

[[maybe_unused]] static ets_double CallStaticDoubleMethodListHelper(EtsEnv *env, ets_class cls, ets_method methodId,
                                                                    ...)
{
    std::va_list args;
    va_start(args, methodId);
    return env->CallStaticDoubleMethodList(cls, methodId, args);
}
// NOLINTEND(cppcoreguidelines-pro-type-vararg)

}  // namespace ark::ets::test

#endif  // PANDA_PLUGINS_ETS_TESTS_MOCK_CALLING_METHODS_TEST_HELPER_H
