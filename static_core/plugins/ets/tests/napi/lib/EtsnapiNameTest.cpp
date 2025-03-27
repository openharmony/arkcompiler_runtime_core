/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "EtsnapiNameTest.h"
#include "libpandabase/macros.h"
#include <iostream>

#include "plugins/ets/runtime/ani/ani.h"

// NOLINTBEGIN(readability-magic-numbers, readability-named-parameter)

[[maybe_unused]] static bool Equal(double a1, double a2)
{
    return std::abs(a1 - a2) < 0.000001F;
}

extern "C" {
ETS_EXPORT ets_int ETS_CALL ETS_EtsnapiNameTest_ETSGLOBAL_nativeInGlobal([[maybe_unused]] EtsEnv *,
                                                                         [[maybe_unused]] ets_class)
{
    return 1;
}

ETS_EXPORT ets_int ETS_CALL ETS_EtsnapiNameTest_EtsnapiNameTest_objectArg([[maybe_unused]] EtsEnv *,
                                                                          [[maybe_unused]] ets_class,
                                                                          [[maybe_unused]] ets_object)
{
    return 0;
}

ETS_EXPORT ets_string ETS_CALL ETS_EtsnapiNameTest_EtsnapiNameTest__04e2d_06587_051fd_065702_04e0d_05e26_053c2_06570(
    [[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_object)
{
    const std::string ret {"passed"};
    return (env->NewStringUTF(ret.c_str()));
}

ETS_EXPORT ets_int ETS_CALL
ETS_EtsnapiNameTest_EtsnapiNameTest_methodOverloaded__Lstd_core_Object_2Lstd_core_String_2_3D(
    [[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class, [[maybe_unused]] ets_object obj1,
    [[maybe_unused]] ets_string s, [[maybe_unused]] ets_object obj2)
{
    ASSERT(obj1 == nullptr && s != nullptr && obj2 != nullptr);
    return 4U;
}

ETS_EXPORT ets_int ETS_CALL ETS_EtsnapiNameTest_EtsnapiNameTest_methodOverloaded__I([[maybe_unused]] EtsEnv *,
                                                                                    [[maybe_unused]] ets_class,
                                                                                    [[maybe_unused]] ets_int i)
{
    ASSERT(i == 2U);
    return 2U;
}

ETS_EXPORT ets_int ETS_CALL ETS_EtsnapiNameTest_EtsnapiNameTest_methodOverloaded__ZBCDFIJS(
    [[maybe_unused]] EtsEnv *, [[maybe_unused]] ets_class, [[maybe_unused]] ets_boolean boo,
    [[maybe_unused]] ets_byte b, [[maybe_unused]] ets_char c, [[maybe_unused]] ets_double d,
    [[maybe_unused]] ets_float f, [[maybe_unused]] ets_int i, [[maybe_unused]] ets_long l, [[maybe_unused]] ets_short s)
{
    ASSERT(boo && b == 1 && Equal(d, 2.2F) && Equal(f, 3.3F) && i == 4U && l == 5L && s == 6U);
    return 3U;
}

ETS_EXPORT ets_int ETS_CALL ETS_EtsnapiNameTest_EtsnapiNameTest_methodOverloaded__([[maybe_unused]] EtsEnv *,
                                                                                   [[maybe_unused]] ets_class)
{
    return 1;
}

extern "C" ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    (void)vm;
    *result = ANI_VERSION_1;
    // No need to do anything, symbols are already mangled and can be called.
    return ANI_OK;
}

}  // extern "C"

// NOLINTEND(readability-magic-numbers, readability-named-parameter)
