/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "plugins/ets/tests/mock/mock_test_helper.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
namespace panda::ets::test {

class CallingMethodsTestBase : public MockEtsNapiTestBaseClass {
protected:
    CallingMethodsTestBase() = default;
    explicit CallingMethodsTestBase(const char *test_bin_file_name) : MockEtsNapiTestBaseClass(test_bin_file_name) {};
};

[[maybe_unused]] static void CallVoidMethodListHelper(EtsEnv *env, ets_object obj, ets_method method_id, ...)
{
    std::va_list args;
    va_start(args, method_id);
    env->CallVoidMethodList(obj, method_id, args);
}

[[maybe_unused]] static ets_object CallObjectMethodListHelper(EtsEnv *env, ets_object obj, ets_method method_id, ...)
{
    std::va_list args;
    va_start(args, method_id);
    return env->CallObjectMethodList(obj, method_id, args);
}

[[maybe_unused]] static ets_boolean CallBooleanMethodListHelper(EtsEnv *env, ets_object obj, ets_method method_id, ...)
{
    std::va_list args;
    va_start(args, method_id);
    return env->CallBooleanMethodList(obj, method_id, args);
}

[[maybe_unused]] static ets_byte CallByteMethodListHelper(EtsEnv *env, ets_object obj, ets_method method_id, ...)
{
    std::va_list args;
    va_start(args, method_id);
    return env->CallByteMethodList(obj, method_id, args);
}

[[maybe_unused]] static ets_char CallCharMethodListHelper(EtsEnv *env, ets_object obj, ets_method method_id, ...)
{
    std::va_list args;
    va_start(args, method_id);
    return env->CallCharMethodList(obj, method_id, args);
}

[[maybe_unused]] static ets_short CallShortMethodListHelper(EtsEnv *env, ets_object obj, ets_method method_id, ...)
{
    std::va_list args;
    va_start(args, method_id);
    return env->CallShortMethodList(obj, method_id, args);
}

[[maybe_unused]] static ets_int CallIntMethodListHelper(EtsEnv *env, ets_object obj, ets_method method_id, ...)
{
    std::va_list args;
    va_start(args, method_id);
    return env->CallIntMethodList(obj, method_id, args);
}

[[maybe_unused]] static ets_long CallLongMethodListHelper(EtsEnv *env, ets_object obj, ets_method method_id, ...)
{
    std::va_list args;
    va_start(args, method_id);
    return env->CallLongMethodList(obj, method_id, args);
}

[[maybe_unused]] static ets_float CallFloatMethodListHelper(EtsEnv *env, ets_object obj, ets_method method_id, ...)
{
    std::va_list args;
    va_start(args, method_id);
    return env->CallFloatMethodList(obj, method_id, args);
}

[[maybe_unused]] static ets_double CallDoubleMethodListHelper(EtsEnv *env, ets_object obj, ets_method method_id, ...)
{
    std::va_list args;
    va_start(args, method_id);
    return env->CallDoubleMethodList(obj, method_id, args);
}

[[maybe_unused]] static void CallNonvirtualVoidMethodListHelper(EtsEnv *env, ets_object obj, ets_class cls,
                                                                ets_method method_id, ...)
{
    std::va_list args;
    va_start(args, method_id);
    env->CallNonvirtualVoidMethodList(obj, cls, method_id, args);
}

[[maybe_unused]] static ets_object CallNonvirtualObjectMethodListHelper(EtsEnv *env, ets_object obj, ets_class cls,
                                                                        ets_method method_id, ...)
{
    std::va_list args;
    va_start(args, method_id);
    return env->CallNonvirtualObjectMethodList(obj, cls, method_id, args);
}

[[maybe_unused]] static ets_boolean CallNonvirtualBooleanMethodListHelper(EtsEnv *env, ets_object obj, ets_class cls,
                                                                          ets_method method_id, ...)
{
    std::va_list args;
    va_start(args, method_id);
    return env->CallNonvirtualBooleanMethodList(obj, cls, method_id, args);
}

[[maybe_unused]] static ets_byte CallNonvirtualByteMethodListHelper(EtsEnv *env, ets_object obj, ets_class cls,
                                                                    ets_method method_id, ...)
{
    std::va_list args;
    va_start(args, method_id);
    return env->CallNonvirtualByteMethodList(obj, cls, method_id, args);
}

[[maybe_unused]] static ets_char CallNonvirtualCharMethodListHelper(EtsEnv *env, ets_object obj, ets_class cls,
                                                                    ets_method method_id, ...)
{
    std::va_list args;
    va_start(args, method_id);
    return env->CallNonvirtualCharMethodList(obj, cls, method_id, args);
}

[[maybe_unused]] static ets_short CallNonvirtualShortMethodListHelper(EtsEnv *env, ets_object obj, ets_class cls,
                                                                      ets_method method_id, ...)
{
    std::va_list args;
    va_start(args, method_id);
    return env->CallNonvirtualShortMethodList(obj, cls, method_id, args);
}

[[maybe_unused]] static ets_int CallNonvirtualIntMethodListHelper(EtsEnv *env, ets_object obj, ets_class cls,
                                                                  ets_method method_id, ...)
{
    std::va_list args;
    va_start(args, method_id);
    return env->CallNonvirtualIntMethodList(obj, cls, method_id, args);
}

[[maybe_unused]] static ets_long CallNonvirtualLongMethodListHelper(EtsEnv *env, ets_object obj, ets_class cls,
                                                                    ets_method method_id, ...)
{
    std::va_list args;
    va_start(args, method_id);
    return env->CallNonvirtualLongMethodList(obj, cls, method_id, args);
}

[[maybe_unused]] static ets_float CallNonvirtualFloatMethodListHelper(EtsEnv *env, ets_object obj, ets_class cls,
                                                                      ets_method method_id, ...)
{
    std::va_list args;
    va_start(args, method_id);
    return env->CallNonvirtualFloatMethodList(obj, cls, method_id, args);
}

[[maybe_unused]] static ets_double CallNonvirtualDoubleMethodListHelper(EtsEnv *env, ets_object obj, ets_class cls,
                                                                        ets_method method_id, ...)
{
    std::va_list args;
    va_start(args, method_id);
    return env->CallNonvirtualDoubleMethodList(obj, cls, method_id, args);
}

[[maybe_unused]] static void CallStaticVoidMethodListHelper(EtsEnv *env, ets_class cls, ets_method method_id, ...)
{
    std::va_list args;
    va_start(args, method_id);
    env->CallStaticVoidMethodList(cls, method_id, args);
}

[[maybe_unused]] static ets_object CallStaticObjectMethodListHelper(EtsEnv *env, ets_class cls, ets_method method_id,
                                                                    ...)
{
    std::va_list args;
    va_start(args, method_id);
    return env->CallStaticObjectMethodList(cls, method_id, args);
}

[[maybe_unused]] static ets_boolean CallStaticBooleanMethodListHelper(EtsEnv *env, ets_class cls, ets_method method_id,
                                                                      ...)
{
    std::va_list args;
    va_start(args, method_id);
    return env->CallStaticBooleanMethodList(cls, method_id, args);
}

[[maybe_unused]] static ets_byte CallStaticByteMethodListHelper(EtsEnv *env, ets_class cls, ets_method method_id, ...)
{
    std::va_list args;
    va_start(args, method_id);
    return env->CallStaticByteMethodList(cls, method_id, args);
}

[[maybe_unused]] static ets_char CallStaticCharMethodListHelper(EtsEnv *env, ets_class cls, ets_method method_id, ...)
{
    std::va_list args;
    va_start(args, method_id);
    return env->CallStaticCharMethodList(cls, method_id, args);
}

[[maybe_unused]] static ets_short CallStaticShortMethodListHelper(EtsEnv *env, ets_class cls, ets_method method_id, ...)
{
    std::va_list args;
    va_start(args, method_id);
    return env->CallStaticShortMethodList(cls, method_id, args);
}

[[maybe_unused]] static ets_int CallStaticIntMethodListHelper(EtsEnv *env, ets_class cls, ets_method method_id, ...)
{
    std::va_list args;
    va_start(args, method_id);
    return env->CallStaticIntMethodList(cls, method_id, args);
}

[[maybe_unused]] static ets_long CallStaticLongMethodListHelper(EtsEnv *env, ets_class cls, ets_method method_id, ...)
{
    std::va_list args;
    va_start(args, method_id);
    return env->CallStaticLongMethodList(cls, method_id, args);
}

[[maybe_unused]] static ets_float CallStaticFloatMethodListHelper(EtsEnv *env, ets_class cls, ets_method method_id, ...)
{
    std::va_list args;
    va_start(args, method_id);
    return env->CallStaticFloatMethodList(cls, method_id, args);
}

[[maybe_unused]] static ets_double CallStaticDoubleMethodListHelper(EtsEnv *env, ets_class cls, ets_method method_id,
                                                                    ...)
{
    std::va_list args;
    va_start(args, method_id);
    return env->CallStaticDoubleMethodList(cls, method_id, args);
}
// NOLINTEND(cppcoreguidelines-pro-type-vararg)

}  // namespace panda::ets::test

#endif  // PANDA_PLUGINS_ETS_TESTS_MOCK_CALLING_METHODS_TEST_HELPER_H