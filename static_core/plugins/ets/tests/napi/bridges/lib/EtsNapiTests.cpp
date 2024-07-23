/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "ets_napi.h"
#include <string>
#include <iostream>
#include <cassert>
#include <vector>

// NOLINTBEGIN(readability-magic-numbers)

static std::string GetString(EtsEnv *env, ets_string const &estr)
{
    const char *cstr = env->GetStringUTFChars(estr, nullptr);
    if (cstr == nullptr) {
        std::cerr << "Bad estring to cstring conversion";
        exit(1);
    }
    std::string result(cstr);
    env->ReleaseStringUTFChars(estr, cstr);
    return result;
}

template <typename ValueType>
std::vector<ValueType> EtsArrToVec(ValueType *buf, ets_size sizeArr)
{
    if (buf == nullptr) {
        std::cerr << "Array empty, failed to get array";
        exit(1);
    }
    std::vector<ValueType> vec;
    vec.reserve(sizeArr);
    for (ets_size i = 0; i < sizeArr; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        vec.push_back(buf[i]);
    }
    return vec;
}

[[maybe_unused]] static bool Equal(double a1, double a2)
{
    return std::abs(a1 - a2) < 0.000001F;
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printChar1([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_char a1)
{
    assert(a1 == 1);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printChar2([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_char a1, [[maybe_unused]] ets_char a2)
{
    assert(a1 == 1 && a2 == 2);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printChar3([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_char a1, [[maybe_unused]] ets_char a2,
                                                       [[maybe_unused]] ets_char a3)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printChar4([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_char a1, [[maybe_unused]] ets_char a2,
                                                       [[maybe_unused]] ets_char a3, [[maybe_unused]] ets_char a4)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printChar5([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_char a1, [[maybe_unused]] ets_char a2,
                                                       [[maybe_unused]] ets_char a3, [[maybe_unused]] ets_char a4,
                                                       [[maybe_unused]] ets_char a5)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printChar6([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_char a1, [[maybe_unused]] ets_char a2,
                                                       [[maybe_unused]] ets_char a3, [[maybe_unused]] ets_char a4,
                                                       [[maybe_unused]] ets_char a5, [[maybe_unused]] ets_char a6)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printChar7([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_char a1, [[maybe_unused]] ets_char a2,
                                                       [[maybe_unused]] ets_char a3, [[maybe_unused]] ets_char a4,
                                                       [[maybe_unused]] ets_char a5, [[maybe_unused]] ets_char a6,
                                                       [[maybe_unused]] ets_char a7)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5 && a6 == 6 && a7 == 7);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printChar8([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_char a1, [[maybe_unused]] ets_char a2,
                                                       [[maybe_unused]] ets_char a3, [[maybe_unused]] ets_char a4,
                                                       [[maybe_unused]] ets_char a5, [[maybe_unused]] ets_char a6,
                                                       [[maybe_unused]] ets_char a7, [[maybe_unused]] ets_char a8)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5 && a6 == 6 && a7 == 7 && a8 == 8);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printChar9([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_char a1, [[maybe_unused]] ets_char a2,
                                                       [[maybe_unused]] ets_char a3, [[maybe_unused]] ets_char a4,
                                                       [[maybe_unused]] ets_char a5, [[maybe_unused]] ets_char a6,
                                                       [[maybe_unused]] ets_char a7, [[maybe_unused]] ets_char a8,
                                                       [[maybe_unused]] ets_char a9)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5 && a6 == 6 && a7 == 7 && a8 == 8 && a9 == 9);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printChar10([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        [[maybe_unused]] ets_char a1, [[maybe_unused]] ets_char a2,
                                                        [[maybe_unused]] ets_char a3, [[maybe_unused]] ets_char a4,
                                                        [[maybe_unused]] ets_char a5, [[maybe_unused]] ets_char a6,
                                                        [[maybe_unused]] ets_char a7, [[maybe_unused]] ets_char a8,
                                                        [[maybe_unused]] ets_char a9, [[maybe_unused]] ets_char a10)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5 && a6 == 6 && a7 == 7 && a8 == 8 && a9 == 9 &&
           a10 == 10);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printChar11([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        [[maybe_unused]] ets_char a1, [[maybe_unused]] ets_char a2,
                                                        [[maybe_unused]] ets_char a3, [[maybe_unused]] ets_char a4,
                                                        [[maybe_unused]] ets_char a5, [[maybe_unused]] ets_char a6,
                                                        [[maybe_unused]] ets_char a7, [[maybe_unused]] ets_char a8,
                                                        [[maybe_unused]] ets_char a9, [[maybe_unused]] ets_char a10,
                                                        [[maybe_unused]] ets_char a11)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5 && a6 == 6 && a7 == 7 && a8 == 8 && a9 == 9 &&
           a10 == 10 && a11 == 11);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printByte1([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_byte a1)
{
    assert(a1 == 1);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printByte2([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_byte a1, [[maybe_unused]] ets_byte a2)
{
    assert(a1 == 1 && a2 == 2);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printByte3([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_byte a1, [[maybe_unused]] ets_byte a2,
                                                       [[maybe_unused]] ets_byte a3)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printByte4([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_byte a1, [[maybe_unused]] ets_byte a2,
                                                       [[maybe_unused]] ets_byte a3, [[maybe_unused]] ets_byte a4)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printByte5([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_byte a1, [[maybe_unused]] ets_byte a2,
                                                       [[maybe_unused]] ets_byte a3, [[maybe_unused]] ets_byte a4,
                                                       [[maybe_unused]] ets_byte a5)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printByte6([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_byte a1, [[maybe_unused]] ets_byte a2,
                                                       [[maybe_unused]] ets_byte a3, [[maybe_unused]] ets_byte a4,
                                                       [[maybe_unused]] ets_byte a5, [[maybe_unused]] ets_byte a6)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printByte7([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_byte a1, [[maybe_unused]] ets_byte a2,
                                                       [[maybe_unused]] ets_byte a3, [[maybe_unused]] ets_byte a4,
                                                       [[maybe_unused]] ets_byte a5, [[maybe_unused]] ets_byte a6,
                                                       [[maybe_unused]] ets_byte a7)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5 && a6 == 6 && a7 == 7);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printByte8([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_byte a1, [[maybe_unused]] ets_byte a2,
                                                       [[maybe_unused]] ets_byte a3, [[maybe_unused]] ets_byte a4,
                                                       [[maybe_unused]] ets_byte a5, [[maybe_unused]] ets_byte a6,
                                                       [[maybe_unused]] ets_byte a7, [[maybe_unused]] ets_byte a8)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5 && a6 == 6 && a7 == 7 && a8 == 8);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printByte9([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_byte a1, [[maybe_unused]] ets_byte a2,
                                                       [[maybe_unused]] ets_byte a3, [[maybe_unused]] ets_byte a4,
                                                       [[maybe_unused]] ets_byte a5, [[maybe_unused]] ets_byte a6,
                                                       [[maybe_unused]] ets_byte a7, [[maybe_unused]] ets_byte a8,
                                                       [[maybe_unused]] ets_byte a9)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5 && a6 == 6 && a7 == 7 && a8 == 8 && a9 == 9);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printByte10([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        [[maybe_unused]] ets_byte a1, [[maybe_unused]] ets_byte a2,
                                                        [[maybe_unused]] ets_byte a3, [[maybe_unused]] ets_byte a4,
                                                        [[maybe_unused]] ets_byte a5, [[maybe_unused]] ets_byte a6,
                                                        [[maybe_unused]] ets_byte a7, [[maybe_unused]] ets_byte a8,
                                                        [[maybe_unused]] ets_byte a9, [[maybe_unused]] ets_byte a10)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5 && a6 == 6 && a7 == 7 && a8 == 8 && a9 == 9 &&
           a10 == 10);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printByte11([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        [[maybe_unused]] ets_byte a1, [[maybe_unused]] ets_byte a2,
                                                        [[maybe_unused]] ets_byte a3, [[maybe_unused]] ets_byte a4,
                                                        [[maybe_unused]] ets_byte a5, [[maybe_unused]] ets_byte a6,
                                                        [[maybe_unused]] ets_byte a7, [[maybe_unused]] ets_byte a8,
                                                        [[maybe_unused]] ets_byte a9, [[maybe_unused]] ets_byte a10,
                                                        [[maybe_unused]] ets_byte a11)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5 && a6 == 6 && a7 == 7 && a8 == 8 && a9 == 9 &&
           a10 == 10 && a11 == 11);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printShort1([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        [[maybe_unused]] ets_short a1)
{
    assert(a1 == 1);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printShort2([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        [[maybe_unused]] ets_short a1, [[maybe_unused]] ets_short a2)
{
    assert(a1 == 1 && a2 == 2);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printShort3([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        [[maybe_unused]] ets_short a1, [[maybe_unused]] ets_short a2,
                                                        [[maybe_unused]] ets_short a3)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printShort4([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        [[maybe_unused]] ets_short a1, [[maybe_unused]] ets_short a2,
                                                        [[maybe_unused]] ets_short a3, [[maybe_unused]] ets_short a4)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printShort5([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        [[maybe_unused]] ets_short a1, [[maybe_unused]] ets_short a2,
                                                        [[maybe_unused]] ets_short a3, [[maybe_unused]] ets_short a4,
                                                        [[maybe_unused]] ets_short a5)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printShort6([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        [[maybe_unused]] ets_short a1, [[maybe_unused]] ets_short a2,
                                                        [[maybe_unused]] ets_short a3, [[maybe_unused]] ets_short a4,
                                                        [[maybe_unused]] ets_short a5, [[maybe_unused]] ets_short a6)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5 && a6 == 6);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printShort7([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        [[maybe_unused]] ets_short a1, [[maybe_unused]] ets_short a2,
                                                        [[maybe_unused]] ets_short a3, [[maybe_unused]] ets_short a4,
                                                        [[maybe_unused]] ets_short a5, [[maybe_unused]] ets_short a6,
                                                        [[maybe_unused]] ets_short a7)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5 && a6 == 6 && a7 == 7);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printShort8([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        [[maybe_unused]] ets_short a1, [[maybe_unused]] ets_short a2,
                                                        [[maybe_unused]] ets_short a3, [[maybe_unused]] ets_short a4,
                                                        [[maybe_unused]] ets_short a5, [[maybe_unused]] ets_short a6,
                                                        [[maybe_unused]] ets_short a7, [[maybe_unused]] ets_short a8)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5 && a6 == 6 && a7 == 7 && a8 == 8);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printShort9([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        [[maybe_unused]] ets_short a1, [[maybe_unused]] ets_short a2,
                                                        [[maybe_unused]] ets_short a3, [[maybe_unused]] ets_short a4,
                                                        [[maybe_unused]] ets_short a5, [[maybe_unused]] ets_short a6,
                                                        [[maybe_unused]] ets_short a7, [[maybe_unused]] ets_short a8,
                                                        [[maybe_unused]] ets_short a9)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5 && a6 == 6 && a7 == 7 && a8 == 8 && a9 == 9);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printShort10([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         [[maybe_unused]] ets_short a1, [[maybe_unused]] ets_short a2,
                                                         [[maybe_unused]] ets_short a3, [[maybe_unused]] ets_short a4,
                                                         [[maybe_unused]] ets_short a5, [[maybe_unused]] ets_short a6,
                                                         [[maybe_unused]] ets_short a7, [[maybe_unused]] ets_short a8,
                                                         [[maybe_unused]] ets_short a9, [[maybe_unused]] ets_short a10)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5 && a6 == 6 && a7 == 7 && a8 == 8 && a9 == 9 &&
           a10 == 10);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printShort11([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         [[maybe_unused]] ets_short a1, [[maybe_unused]] ets_short a2,
                                                         [[maybe_unused]] ets_short a3, [[maybe_unused]] ets_short a4,
                                                         [[maybe_unused]] ets_short a5, [[maybe_unused]] ets_short a6,
                                                         [[maybe_unused]] ets_short a7, [[maybe_unused]] ets_short a8,
                                                         [[maybe_unused]] ets_short a9, [[maybe_unused]] ets_short a10,
                                                         [[maybe_unused]] ets_short a11)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5 && a6 == 6 && a7 == 7 && a8 == 8 && a9 == 9 &&
           a10 == 10 && a11 == 11);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printInt1([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                      [[maybe_unused]] ets_int a1)
{
    assert(a1 == 1);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printInt2([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                      [[maybe_unused]] ets_int a1, [[maybe_unused]] ets_int a2)
{
    assert(a1 == 1 && a2 == 2);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printInt3([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                      [[maybe_unused]] ets_int a1, [[maybe_unused]] ets_int a2,
                                                      [[maybe_unused]] ets_int a3)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printInt4([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                      [[maybe_unused]] ets_int a1, [[maybe_unused]] ets_int a2,
                                                      [[maybe_unused]] ets_int a3, [[maybe_unused]] ets_int a4)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printInt5([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                      [[maybe_unused]] ets_int a1, [[maybe_unused]] ets_int a2,
                                                      [[maybe_unused]] ets_int a3, [[maybe_unused]] ets_int a4,
                                                      [[maybe_unused]] ets_int a5)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printInt6([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                      [[maybe_unused]] ets_int a1, [[maybe_unused]] ets_int a2,
                                                      [[maybe_unused]] ets_int a3, [[maybe_unused]] ets_int a4,
                                                      [[maybe_unused]] ets_int a5, [[maybe_unused]] ets_int a6)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printInt7([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                      [[maybe_unused]] ets_int a1, [[maybe_unused]] ets_int a2,
                                                      [[maybe_unused]] ets_int a3, [[maybe_unused]] ets_int a4,
                                                      [[maybe_unused]] ets_int a5, [[maybe_unused]] ets_int a6,
                                                      [[maybe_unused]] ets_int a7)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5 && a6 == 6 && a7 == 7);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printInt8([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                      [[maybe_unused]] ets_int a1, [[maybe_unused]] ets_int a2,
                                                      [[maybe_unused]] ets_int a3, [[maybe_unused]] ets_int a4,
                                                      [[maybe_unused]] ets_int a5, [[maybe_unused]] ets_int a6,
                                                      [[maybe_unused]] ets_int a7, [[maybe_unused]] ets_int a8)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5 && a6 == 6 && a7 == 7 && a8 == 8);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printInt9([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                      [[maybe_unused]] ets_int a1, [[maybe_unused]] ets_int a2,
                                                      [[maybe_unused]] ets_int a3, [[maybe_unused]] ets_int a4,
                                                      [[maybe_unused]] ets_int a5, [[maybe_unused]] ets_int a6,
                                                      [[maybe_unused]] ets_int a7, [[maybe_unused]] ets_int a8,
                                                      [[maybe_unused]] ets_int a9)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5 && a6 == 6 && a7 == 7 && a8 == 8 && a9 == 9);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printInt10([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_int a1, [[maybe_unused]] ets_int a2,
                                                       [[maybe_unused]] ets_int a3, [[maybe_unused]] ets_int a4,
                                                       [[maybe_unused]] ets_int a5, [[maybe_unused]] ets_int a6,
                                                       [[maybe_unused]] ets_int a7, [[maybe_unused]] ets_int a8,
                                                       [[maybe_unused]] ets_int a9, [[maybe_unused]] ets_int a10)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5 && a6 == 6 && a7 == 7 && a8 == 8 && a9 == 9 &&
           a10 == 10);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printInt11([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_int a1, [[maybe_unused]] ets_int a2,
                                                       [[maybe_unused]] ets_int a3, [[maybe_unused]] ets_int a4,
                                                       [[maybe_unused]] ets_int a5, [[maybe_unused]] ets_int a6,
                                                       [[maybe_unused]] ets_int a7, [[maybe_unused]] ets_int a8,
                                                       [[maybe_unused]] ets_int a9, [[maybe_unused]] ets_int a10,
                                                       [[maybe_unused]] ets_int a11)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5 && a6 == 6 && a7 == 7 && a8 == 8 && a9 == 9 &&
           a10 == 10 && a11 == 11);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printLong1([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_long a1)
{
    assert(a1 == 1);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printLong2([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_long a1, [[maybe_unused]] ets_long a2)
{
    assert(a1 == 1 && a2 == 2);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printLong3([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_long a1, [[maybe_unused]] ets_long a2,
                                                       [[maybe_unused]] ets_long a3)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printLong4([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_long a1, [[maybe_unused]] ets_long a2,
                                                       [[maybe_unused]] ets_long a3, [[maybe_unused]] ets_long a4)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printLong5([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_long a1, [[maybe_unused]] ets_long a2,
                                                       [[maybe_unused]] ets_long a3, [[maybe_unused]] ets_long a4,
                                                       [[maybe_unused]] ets_long a5)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printLong6([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_long a1, [[maybe_unused]] ets_long a2,
                                                       [[maybe_unused]] ets_long a3, [[maybe_unused]] ets_long a4,
                                                       [[maybe_unused]] ets_long a5, [[maybe_unused]] ets_long a6)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printLong7([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_long a1, [[maybe_unused]] ets_long a2,
                                                       [[maybe_unused]] ets_long a3, [[maybe_unused]] ets_long a4,
                                                       [[maybe_unused]] ets_long a5, [[maybe_unused]] ets_long a6,
                                                       [[maybe_unused]] ets_long a7)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5 && a6 == 6 && a7 == 7);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printLong8([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_long a1, [[maybe_unused]] ets_long a2,
                                                       [[maybe_unused]] ets_long a3, [[maybe_unused]] ets_long a4,
                                                       [[maybe_unused]] ets_long a5, [[maybe_unused]] ets_long a6,
                                                       [[maybe_unused]] ets_long a7, [[maybe_unused]] ets_long a8)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5 && a6 == 6 && a7 == 7 && a8 == 8);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printLong9([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_long a1, [[maybe_unused]] ets_long a2,
                                                       [[maybe_unused]] ets_long a3, [[maybe_unused]] ets_long a4,
                                                       [[maybe_unused]] ets_long a5, [[maybe_unused]] ets_long a6,
                                                       [[maybe_unused]] ets_long a7, [[maybe_unused]] ets_long a8,
                                                       [[maybe_unused]] ets_long a9)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5 && a6 == 6 && a7 == 7 && a8 == 8 && a9 == 9);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printLong10([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        [[maybe_unused]] ets_long a1, [[maybe_unused]] ets_long a2,
                                                        [[maybe_unused]] ets_long a3, [[maybe_unused]] ets_long a4,
                                                        [[maybe_unused]] ets_long a5, [[maybe_unused]] ets_long a6,
                                                        [[maybe_unused]] ets_long a7, [[maybe_unused]] ets_long a8,
                                                        [[maybe_unused]] ets_long a9, [[maybe_unused]] ets_long a10)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5 && a6 == 6 && a7 == 7 && a8 == 8 && a9 == 9 &&
           a10 == 10);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printLong11([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        [[maybe_unused]] ets_long a1, [[maybe_unused]] ets_long a2,
                                                        [[maybe_unused]] ets_long a3, [[maybe_unused]] ets_long a4,
                                                        [[maybe_unused]] ets_long a5, [[maybe_unused]] ets_long a6,
                                                        [[maybe_unused]] ets_long a7, [[maybe_unused]] ets_long a8,
                                                        [[maybe_unused]] ets_long a9, [[maybe_unused]] ets_long a10,
                                                        [[maybe_unused]] ets_long a11)
{
    assert(a1 == 1 && a2 == 2 && a3 == 3 && a4 == 4 && a5 == 5 && a6 == 6 && a7 == 7 && a8 == 8 && a9 == 9 &&
           a10 == 10 && a11 == 11);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printFloat1([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        [[maybe_unused]] ets_float a1)
{
    assert(Equal(a1, 1.1));
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printFloat2([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        [[maybe_unused]] ets_float a1, [[maybe_unused]] ets_float a2)
{
    assert(Equal(a1, 1.1) && Equal(a2, 2.2));
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printFloat3([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        [[maybe_unused]] ets_float a1, [[maybe_unused]] ets_float a2,
                                                        [[maybe_unused]] ets_float a3)
{
    assert(Equal(a1, 1.1) && Equal(a2, 2.2) && Equal(a3, 3.3));
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printFloat4([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        [[maybe_unused]] ets_float a1, [[maybe_unused]] ets_float a2,
                                                        [[maybe_unused]] ets_float a3, [[maybe_unused]] ets_float a4)
{
    assert(Equal(a1, 1.1) && Equal(a2, 2.2) && Equal(a3, 3.3) && Equal(a4, 4.4));
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printFloat5([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        [[maybe_unused]] ets_float a1, [[maybe_unused]] ets_float a2,
                                                        [[maybe_unused]] ets_float a3, [[maybe_unused]] ets_float a4,
                                                        [[maybe_unused]] ets_float a5)
{
    assert(Equal(a1, 1.1) && Equal(a2, 2.2) && Equal(a3, 3.3) && Equal(a4, 4.4) && Equal(a5, 5.5));
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printFloat6([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        [[maybe_unused]] ets_float a1, [[maybe_unused]] ets_float a2,
                                                        [[maybe_unused]] ets_float a3, [[maybe_unused]] ets_float a4,
                                                        [[maybe_unused]] ets_float a5, [[maybe_unused]] ets_float a6)
{
    assert(Equal(a1, 1.1) && Equal(a2, 2.2) && Equal(a3, 3.3) && Equal(a4, 4.4) && Equal(a5, 5.5) && Equal(a6, 6.6));
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printFloat7([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        [[maybe_unused]] ets_float a1, [[maybe_unused]] ets_float a2,
                                                        [[maybe_unused]] ets_float a3, [[maybe_unused]] ets_float a4,
                                                        [[maybe_unused]] ets_float a5, [[maybe_unused]] ets_float a6,
                                                        [[maybe_unused]] ets_float a7)
{
    assert(Equal(a1, 1.1) && Equal(a2, 2.2) && Equal(a3, 3.3) && Equal(a4, 4.4) && Equal(a5, 5.5) && Equal(a6, 6.6) &&
           Equal(a7, 7.7));
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printFloat8([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        [[maybe_unused]] ets_float a1, [[maybe_unused]] ets_float a2,
                                                        [[maybe_unused]] ets_float a3, [[maybe_unused]] ets_float a4,
                                                        [[maybe_unused]] ets_float a5, [[maybe_unused]] ets_float a6,
                                                        [[maybe_unused]] ets_float a7, [[maybe_unused]] ets_float a8)
{
    assert(Equal(a1, 1.1) && Equal(a2, 2.2) && Equal(a3, 3.3) && Equal(a4, 4.4) && Equal(a5, 5.5) && Equal(a6, 6.6) &&
           Equal(a7, 7.7) && Equal(a8, 8.8));
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printFloat9([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        [[maybe_unused]] ets_float a1, [[maybe_unused]] ets_float a2,
                                                        [[maybe_unused]] ets_float a3, [[maybe_unused]] ets_float a4,
                                                        [[maybe_unused]] ets_float a5, [[maybe_unused]] ets_float a6,
                                                        [[maybe_unused]] ets_float a7, [[maybe_unused]] ets_float a8,
                                                        [[maybe_unused]] ets_float a9)
{
    assert(Equal(a1, 1.1) && Equal(a2, 2.2) && Equal(a3, 3.3) && Equal(a4, 4.4) && Equal(a5, 5.5) && Equal(a6, 6.6) &&
           Equal(a7, 7.7) && Equal(a8, 8.8) && Equal(a9, 9.9));
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printFloat10([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         [[maybe_unused]] ets_float a1, [[maybe_unused]] ets_float a2,
                                                         [[maybe_unused]] ets_float a3, [[maybe_unused]] ets_float a4,
                                                         [[maybe_unused]] ets_float a5, [[maybe_unused]] ets_float a6,
                                                         [[maybe_unused]] ets_float a7, [[maybe_unused]] ets_float a8,
                                                         [[maybe_unused]] ets_float a9, [[maybe_unused]] ets_float a10)
{
    assert(Equal(a1, 1.1) && Equal(a2, 2.2) && Equal(a3, 3.3) && Equal(a4, 4.4) && Equal(a5, 5.5) && Equal(a6, 6.6) &&
           Equal(a7, 7.7) && Equal(a8, 8.8) && Equal(a9, 9.9) && Equal(a10, 10.1));
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printFloat11([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         [[maybe_unused]] ets_float a1, [[maybe_unused]] ets_float a2,
                                                         [[maybe_unused]] ets_float a3, [[maybe_unused]] ets_float a4,
                                                         [[maybe_unused]] ets_float a5, [[maybe_unused]] ets_float a6,
                                                         [[maybe_unused]] ets_float a7, [[maybe_unused]] ets_float a8,
                                                         [[maybe_unused]] ets_float a9, [[maybe_unused]] ets_float a10,
                                                         [[maybe_unused]] ets_float a11)
{
    assert(Equal(a1, 1.1) && Equal(a2, 2.2) && Equal(a3, 3.3) && Equal(a4, 4.4) && Equal(a5, 5.5) && Equal(a6, 6.6) &&
           Equal(a7, 7.7) && Equal(a8, 8.8) && Equal(a9, 9.9) && Equal(a10, 10.1) && Equal(a11, 11.1));
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printDouble1([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         [[maybe_unused]] ets_double a1)
{
    assert(Equal(a1, 1.1));
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printDouble2([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         [[maybe_unused]] ets_double a1, [[maybe_unused]] ets_double a2)
{
    assert(Equal(a1, 1.1) && Equal(a2, 2.2));
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printDouble3([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         [[maybe_unused]] ets_double a1, [[maybe_unused]] ets_double a2,
                                                         [[maybe_unused]] ets_double a3)
{
    assert(Equal(a1, 1.1) && Equal(a2, 2.2) && Equal(a3, 3.3));
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printDouble4([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         [[maybe_unused]] ets_double a1, [[maybe_unused]] ets_double a2,
                                                         [[maybe_unused]] ets_double a3, [[maybe_unused]] ets_double a4)
{
    assert(Equal(a1, 1.1) && Equal(a2, 2.2) && Equal(a3, 3.3) && Equal(a4, 4.4));
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printDouble5([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         [[maybe_unused]] ets_double a1, [[maybe_unused]] ets_double a2,
                                                         [[maybe_unused]] ets_double a3, [[maybe_unused]] ets_double a4,
                                                         [[maybe_unused]] ets_double a5)
{
    assert(Equal(a1, 1.1) && Equal(a2, 2.2) && Equal(a3, 3.3) && Equal(a4, 4.4) && Equal(a5, 5.5));
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printDouble6([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         [[maybe_unused]] ets_double a1, [[maybe_unused]] ets_double a2,
                                                         [[maybe_unused]] ets_double a3, [[maybe_unused]] ets_double a4,
                                                         [[maybe_unused]] ets_double a5, [[maybe_unused]] ets_double a6)
{
    assert(Equal(a1, 1.1) && Equal(a2, 2.2) && Equal(a3, 3.3) && Equal(a4, 4.4) && Equal(a5, 5.5) && Equal(a6, 6.6));
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printDouble7([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         [[maybe_unused]] ets_double a1, [[maybe_unused]] ets_double a2,
                                                         [[maybe_unused]] ets_double a3, [[maybe_unused]] ets_double a4,
                                                         [[maybe_unused]] ets_double a5, [[maybe_unused]] ets_double a6,
                                                         [[maybe_unused]] ets_double a7)
{
    assert(Equal(a1, 1.1) && Equal(a2, 2.2) && Equal(a3, 3.3) && Equal(a4, 4.4) && Equal(a5, 5.5) && Equal(a6, 6.6) &&
           Equal(a7, 7.7));
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printDouble8([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         [[maybe_unused]] ets_double a1, [[maybe_unused]] ets_double a2,
                                                         [[maybe_unused]] ets_double a3, [[maybe_unused]] ets_double a4,
                                                         [[maybe_unused]] ets_double a5, [[maybe_unused]] ets_double a6,
                                                         [[maybe_unused]] ets_double a7, [[maybe_unused]] ets_double a8)
{
    assert(Equal(a1, 1.1) && Equal(a2, 2.2) && Equal(a3, 3.3) && Equal(a4, 4.4) && Equal(a5, 5.5) && Equal(a6, 6.6) &&
           Equal(a7, 7.7) && Equal(a8, 8.8));
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printDouble9([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         [[maybe_unused]] ets_double a1, [[maybe_unused]] ets_double a2,
                                                         [[maybe_unused]] ets_double a3, [[maybe_unused]] ets_double a4,
                                                         [[maybe_unused]] ets_double a5, [[maybe_unused]] ets_double a6,
                                                         [[maybe_unused]] ets_double a7, [[maybe_unused]] ets_double a8,
                                                         [[maybe_unused]] ets_double a9)
{
    assert(Equal(a1, 1.1) && Equal(a2, 2.2) && Equal(a3, 3.3) && Equal(a4, 4.4) && Equal(a5, 5.5) && Equal(a6, 6.6) &&
           Equal(a7, 7.7) && Equal(a8, 8.8) && Equal(a9, 9.9));
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printDouble10(
    [[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls, [[maybe_unused]] ets_double a1,
    [[maybe_unused]] ets_double a2, [[maybe_unused]] ets_double a3, [[maybe_unused]] ets_double a4,
    [[maybe_unused]] ets_double a5, [[maybe_unused]] ets_double a6, [[maybe_unused]] ets_double a7,
    [[maybe_unused]] ets_double a8, [[maybe_unused]] ets_double a9, [[maybe_unused]] ets_double a10)
{
    assert(Equal(a1, 1.1) && Equal(a2, 2.2) && Equal(a3, 3.3) && Equal(a4, 4.4) && Equal(a5, 5.5) && Equal(a6, 6.6) &&
           Equal(a7, 7.7) && Equal(a8, 8.8) && Equal(a9, 9.9) && Equal(a10, 10.1));
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printDouble11(
    [[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls, [[maybe_unused]] ets_double a1,
    [[maybe_unused]] ets_double a2, [[maybe_unused]] ets_double a3, [[maybe_unused]] ets_double a4,
    [[maybe_unused]] ets_double a5, [[maybe_unused]] ets_double a6, [[maybe_unused]] ets_double a7,
    [[maybe_unused]] ets_double a8, [[maybe_unused]] ets_double a9, [[maybe_unused]] ets_double a10,
    [[maybe_unused]] ets_double a11)
{
    assert(Equal(a1, 1.1) && Equal(a2, 2.2) && Equal(a3, 3.3) && Equal(a4, 4.4) && Equal(a5, 5.5) && Equal(a6, 6.6) &&
           Equal(a7, 7.7) && Equal(a8, 8.8) && Equal(a9, 9.9) && Equal(a10, 10.1) && Equal(a11, 11.1));
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printBool1([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_boolean a1)
{
    assert(a1 == 1);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printBool2([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_boolean a1, [[maybe_unused]] ets_boolean a2)
{
    assert(a1 == 1 && a2 == 0);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printBool3([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_boolean a1, [[maybe_unused]] ets_boolean a2,
                                                       [[maybe_unused]] ets_boolean a3)
{
    assert(a1 == 1 && a2 == 0 && a3 == 1);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printBool4([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_boolean a1, [[maybe_unused]] ets_boolean a2,
                                                       [[maybe_unused]] ets_boolean a3, [[maybe_unused]] ets_boolean a4)
{
    assert(a1 == 1 && a2 == 0 && a3 == 1 && a4 == 0);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printBool5([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_boolean a1, [[maybe_unused]] ets_boolean a2,
                                                       [[maybe_unused]] ets_boolean a3, [[maybe_unused]] ets_boolean a4,
                                                       [[maybe_unused]] ets_boolean a5)
{
    assert(a1 == 1 && a2 == 0 && a3 == 1 && a4 == 0 && a5 == 1);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printBool6([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_boolean a1, [[maybe_unused]] ets_boolean a2,
                                                       [[maybe_unused]] ets_boolean a3, [[maybe_unused]] ets_boolean a4,
                                                       [[maybe_unused]] ets_boolean a5, [[maybe_unused]] ets_boolean a6)
{
    assert(a1 == 1 && a2 == 0 && a3 == 1 && a4 == 0 && a5 == 1 && a6 == 0);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printBool7([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_boolean a1, [[maybe_unused]] ets_boolean a2,
                                                       [[maybe_unused]] ets_boolean a3, [[maybe_unused]] ets_boolean a4,
                                                       [[maybe_unused]] ets_boolean a5, [[maybe_unused]] ets_boolean a6,
                                                       [[maybe_unused]] ets_boolean a7)
{
    assert(a1 == 1 && a2 == 0 && a3 == 1 && a4 == 0 && a5 == 1 && a6 == 0 && a7 == 1);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printBool8([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_boolean a1, [[maybe_unused]] ets_boolean a2,
                                                       [[maybe_unused]] ets_boolean a3, [[maybe_unused]] ets_boolean a4,
                                                       [[maybe_unused]] ets_boolean a5, [[maybe_unused]] ets_boolean a6,
                                                       [[maybe_unused]] ets_boolean a7, [[maybe_unused]] ets_boolean a8)
{
    assert(a1 == 1 && a2 == 0 && a3 == 1 && a4 == 0 && a5 == 1 && a6 == 0 && a7 == 1 && a8 == 0);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printBool9([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                       [[maybe_unused]] ets_boolean a1, [[maybe_unused]] ets_boolean a2,
                                                       [[maybe_unused]] ets_boolean a3, [[maybe_unused]] ets_boolean a4,
                                                       [[maybe_unused]] ets_boolean a5, [[maybe_unused]] ets_boolean a6,
                                                       [[maybe_unused]] ets_boolean a7, [[maybe_unused]] ets_boolean a8,
                                                       [[maybe_unused]] ets_boolean a9)
{
    assert(a1 == 1 && a2 == 0 && a3 == 1 && a4 == 0 && a5 == 1 && a6 == 0 && a7 == 1 && a8 == 0 && a9 == 1);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printBool10(
    [[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls, [[maybe_unused]] ets_boolean a1,
    [[maybe_unused]] ets_boolean a2, [[maybe_unused]] ets_boolean a3, [[maybe_unused]] ets_boolean a4,
    [[maybe_unused]] ets_boolean a5, [[maybe_unused]] ets_boolean a6, [[maybe_unused]] ets_boolean a7,
    [[maybe_unused]] ets_boolean a8, [[maybe_unused]] ets_boolean a9, [[maybe_unused]] ets_boolean a10)
{
    assert(a1 == 1 && a2 == 0 && a3 == 1 && a4 == 0 && a5 == 1 && a6 == 0 && a7 == 1 && a8 == 0 && a9 == 1 && a10 == 0);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printBool11(
    [[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls, [[maybe_unused]] ets_boolean a1,
    [[maybe_unused]] ets_boolean a2, [[maybe_unused]] ets_boolean a3, [[maybe_unused]] ets_boolean a4,
    [[maybe_unused]] ets_boolean a5, [[maybe_unused]] ets_boolean a6, [[maybe_unused]] ets_boolean a7,
    [[maybe_unused]] ets_boolean a8, [[maybe_unused]] ets_boolean a9, [[maybe_unused]] ets_boolean a10,
    [[maybe_unused]] ets_boolean a11)
{
    assert(a1 == 1 && a2 == 0 && a3 == 1 && a4 == 0 && a5 == 1 && a6 == 0 && a7 == 1 && a8 == 0 && a9 == 1 &&
           a10 == 0 && a11 == 1);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printString1([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         [[maybe_unused]] ets_string a1)
{
    std::string s1 = GetString(env, a1);
    assert(s1 == "1");
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printString2([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         [[maybe_unused]] ets_string a1, [[maybe_unused]] ets_string a2)
{
    std::string s1 = GetString(env, a1);
    std::string s2 = GetString(env, a2);
    assert(s1 == "1" && s2 == "2");
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printString3([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         [[maybe_unused]] ets_string a1, [[maybe_unused]] ets_string a2,
                                                         [[maybe_unused]] ets_string a3)
{
    std::string s1 = GetString(env, a1);
    std::string s2 = GetString(env, a2);
    std::string s3 = GetString(env, a3);
    assert(s1 == "1" && s2 == "2" && s3 == "3");
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printString4([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         [[maybe_unused]] ets_string a1, [[maybe_unused]] ets_string a2,
                                                         [[maybe_unused]] ets_string a3, [[maybe_unused]] ets_string a4)
{
    std::string s1 = GetString(env, a1);
    std::string s2 = GetString(env, a2);
    std::string s3 = GetString(env, a3);
    std::string s4 = GetString(env, a4);
    assert(s1 == "1" && s2 == "2" && s3 == "3" && s4 == "4");
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printString5([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         [[maybe_unused]] ets_string a1, [[maybe_unused]] ets_string a2,
                                                         [[maybe_unused]] ets_string a3, [[maybe_unused]] ets_string a4,
                                                         [[maybe_unused]] ets_string a5)
{
    std::string s1 = GetString(env, a1);
    std::string s2 = GetString(env, a2);
    std::string s3 = GetString(env, a3);
    std::string s4 = GetString(env, a4);
    std::string s5 = GetString(env, a5);
    assert(s1 == "1" && s2 == "2" && s3 == "3" && s4 == "4" && s5 == "5");
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printString6([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         [[maybe_unused]] ets_string a1, [[maybe_unused]] ets_string a2,
                                                         [[maybe_unused]] ets_string a3, [[maybe_unused]] ets_string a4,
                                                         [[maybe_unused]] ets_string a5, [[maybe_unused]] ets_string a6)
{
    std::string s1 = GetString(env, a1);
    std::string s2 = GetString(env, a2);
    std::string s3 = GetString(env, a3);
    std::string s4 = GetString(env, a4);
    std::string s5 = GetString(env, a5);
    std::string s6 = GetString(env, a6);
    assert(s1 == "1" && s2 == "2" && s3 == "3" && s4 == "4" && s5 == "5" && s6 == "6");
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printString7([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         [[maybe_unused]] ets_string a1, [[maybe_unused]] ets_string a2,
                                                         [[maybe_unused]] ets_string a3, [[maybe_unused]] ets_string a4,
                                                         [[maybe_unused]] ets_string a5, [[maybe_unused]] ets_string a6,
                                                         [[maybe_unused]] ets_string a7)
{
    std::string s1 = GetString(env, a1);
    std::string s2 = GetString(env, a2);
    std::string s3 = GetString(env, a3);
    std::string s4 = GetString(env, a4);
    std::string s5 = GetString(env, a5);
    std::string s6 = GetString(env, a6);
    std::string s7 = GetString(env, a7);
    assert(s1 == "1" && s2 == "2" && s3 == "3" && s4 == "4" && s5 == "5" && s6 == "6" && s7 == "7");
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printString8([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         [[maybe_unused]] ets_string a1, [[maybe_unused]] ets_string a2,
                                                         [[maybe_unused]] ets_string a3, [[maybe_unused]] ets_string a4,
                                                         [[maybe_unused]] ets_string a5, [[maybe_unused]] ets_string a6,
                                                         [[maybe_unused]] ets_string a7, [[maybe_unused]] ets_string a8)
{
    std::string s1 = GetString(env, a1);
    std::string s2 = GetString(env, a2);
    std::string s3 = GetString(env, a3);
    std::string s4 = GetString(env, a4);
    std::string s5 = GetString(env, a5);
    std::string s6 = GetString(env, a6);
    std::string s7 = GetString(env, a7);
    std::string s8 = GetString(env, a8);
    assert(s1 == "1" && s2 == "2" && s3 == "3" && s4 == "4" && s5 == "5" && s6 == "6" && s7 == "7" && s8 == "8");
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printString9([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         [[maybe_unused]] ets_string a1, [[maybe_unused]] ets_string a2,
                                                         [[maybe_unused]] ets_string a3, [[maybe_unused]] ets_string a4,
                                                         [[maybe_unused]] ets_string a5, [[maybe_unused]] ets_string a6,
                                                         [[maybe_unused]] ets_string a7, [[maybe_unused]] ets_string a8,
                                                         [[maybe_unused]] ets_string a9)
{
    std::string s1 = GetString(env, a1);
    std::string s2 = GetString(env, a2);
    std::string s3 = GetString(env, a3);
    std::string s4 = GetString(env, a4);
    std::string s5 = GetString(env, a5);
    std::string s6 = GetString(env, a6);
    std::string s7 = GetString(env, a7);
    std::string s8 = GetString(env, a8);
    std::string s9 = GetString(env, a9);
    assert(s1 == "1" && s2 == "2" && s3 == "3" && s4 == "4" && s5 == "5" && s6 == "6" && s7 == "7" && s8 == "8" &&
           s9 == "9");
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printString10(
    [[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls, [[maybe_unused]] ets_string a1,
    [[maybe_unused]] ets_string a2, [[maybe_unused]] ets_string a3, [[maybe_unused]] ets_string a4,
    [[maybe_unused]] ets_string a5, [[maybe_unused]] ets_string a6, [[maybe_unused]] ets_string a7,
    [[maybe_unused]] ets_string a8, [[maybe_unused]] ets_string a9, [[maybe_unused]] ets_string a10)
{
    std::string s1 = GetString(env, a1);
    std::string s2 = GetString(env, a2);
    std::string s3 = GetString(env, a3);
    std::string s4 = GetString(env, a4);
    std::string s5 = GetString(env, a5);
    std::string s6 = GetString(env, a6);
    std::string s7 = GetString(env, a7);
    std::string s8 = GetString(env, a8);
    std::string s9 = GetString(env, a9);
    std::string s10 = GetString(env, a10);
    assert(s1 == "1" && s2 == "2" && s3 == "3" && s4 == "4" && s5 == "5" && s6 == "6" && s7 == "7" && s8 == "8" &&
           s9 == "9" && s10 == "10");
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printString11(
    [[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls, [[maybe_unused]] ets_string a1,
    [[maybe_unused]] ets_string a2, [[maybe_unused]] ets_string a3, [[maybe_unused]] ets_string a4,
    [[maybe_unused]] ets_string a5, [[maybe_unused]] ets_string a6, [[maybe_unused]] ets_string a7,
    [[maybe_unused]] ets_string a8, [[maybe_unused]] ets_string a9, [[maybe_unused]] ets_string a10,
    [[maybe_unused]] ets_string a11)
{
    std::string s1 = GetString(env, a1);
    std::string s2 = GetString(env, a2);
    std::string s3 = GetString(env, a3);
    std::string s4 = GetString(env, a4);
    std::string s5 = GetString(env, a5);
    std::string s6 = GetString(env, a6);
    std::string s7 = GetString(env, a7);
    std::string s8 = GetString(env, a8);
    std::string s9 = GetString(env, a9);
    std::string s10 = GetString(env, a10);
    std::string s11 = GetString(env, a11);
    assert(s1 == "1" && s2 == "2" && s3 == "3" && s4 == "4" && s5 == "5" && s6 == "6" && s7 == "7" && s8 == "8" &&
           s9 == "9" && s10 == "10" && s11 == "11");
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printByteArr([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         [[maybe_unused]] ets_byteArray arr)
{
    ets_byte *buf;
    ets_size len = env->GetArrayLength(arr);
    buf = (ets_byte *)env->PinByteArray(arr);
    auto vec = EtsArrToVec<ets_byte>(buf, len);
    for (size_t i = 0; i < vec.size(); ++i) {
        assert(vec[i] == static_cast<int>(i + 1));
    }
    env->UnpinByteArray(arr);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printCharArr([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         [[maybe_unused]] ets_charArray arr)
{
    ets_char *buf;
    ets_size len = env->GetArrayLength(arr);
    buf = (ets_char *)env->PinCharArray(arr);
    auto vec = EtsArrToVec<ets_char>(buf, len);
    for (size_t i = 0; i < vec.size(); ++i) {
        assert(vec[i] == (i + 1));
    }
    env->UnpinCharArray(arr);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printShortArr([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                          [[maybe_unused]] ets_shortArray arr)
{
    ets_short *buf;
    ets_size len = env->GetArrayLength(arr);
    buf = (ets_short *)env->PinShortArray(arr);
    auto vec = EtsArrToVec<ets_short>(buf, len);
    for (size_t i = 0; i < vec.size(); ++i) {
        assert(vec[i] == static_cast<int>(i + 1));
    }
    env->UnpinShortArray(arr);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printIntArr([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                        [[maybe_unused]] ets_intArray arr)
{
    ets_int *buf;
    ets_size len = env->GetArrayLength(arr);
    buf = (ets_int *)env->PinIntArray(arr);
    auto vec = EtsArrToVec<ets_int>(buf, len);
    for (size_t i = 0; i < vec.size(); ++i) {
        assert(vec[i] == static_cast<int>(i + 1));
    }
    env->UnpinIntArray(arr);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printLongArr([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         [[maybe_unused]] ets_longArray arr)
{
    ets_long *buf;
    ets_size len = env->GetArrayLength(arr);
    buf = (ets_long *)env->PinLongArray(arr);
    auto vec = EtsArrToVec<ets_long>(buf, len);
    for (size_t i = 0; i < vec.size(); ++i) {
        assert(vec[i] == static_cast<int>(i + 1));
    }
    env->UnpinLongArray(arr);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printFloatArr([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                          [[maybe_unused]] ets_floatArray arr)
{
    ets_float *buf;
    ets_size len = env->GetArrayLength(arr);
    buf = (ets_float *)env->PinFloatArray(arr);
    auto vec = EtsArrToVec<ets_float>(buf, len);
    for (size_t i = 0; i < vec.size(); ++i) {
        assert(Equal(vec[i], 1.1 * (i + 1)));
    }
    env->UnpinFloatArray(arr);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printDoubleArr([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                           [[maybe_unused]] ets_doubleArray arr)
{
    ets_double *buf;
    ets_size len = env->GetArrayLength(arr);
    buf = (ets_double *)env->PinDoubleArray(arr);
    auto vec = EtsArrToVec<ets_double>(buf, len);
    for (size_t i = 0; i < vec.size(); ++i) {
        assert(Equal(vec[i], 1.1 * (i + 1)));
    }
    env->UnpinDoubleArray(arr);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printBoolArr([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                         [[maybe_unused]] ets_booleanArray arr)
{
    ets_boolean *buf;
    ets_size len = env->GetArrayLength(arr);
    buf = (ets_boolean *)env->PinBooleanArray(arr);
    auto vec = EtsArrToVec<ets_boolean>(buf, len);
    for (size_t i = 0; i < vec.size(); ++i) {
        assert(vec[i] == 1 * ((i + 1) % 2));
    }
    env->UnpinBooleanArray(arr);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_printStringArr([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class cls,
                                                           [[maybe_unused]] ets_objectArray arr)
{
    for (ets_size i = 0; i < env->GetArrayLength(arr); ++i) {
        auto s = static_cast<ets_string>(env->GetObjectArrayElement(arr, i));
        std::string s1 = GetString(env, s);
        assert(s1 == std::to_string(i + 1));
    }
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT ets_boolean ETS_EtsNapiTests_returnBool([[maybe_unused]] EtsEnv *env,
                                                              [[maybe_unused]] ets_class klass)
{
    ets_boolean ret = ETS_TRUE;
    return ret;
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_checkBool([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                                      [[maybe_unused]] ets_boolean value)
{
    assert(value == ETS_TRUE);
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT ets_byte ETS_EtsNapiTests_returnByte([[maybe_unused]] EtsEnv *env,
                                                           [[maybe_unused]] ets_class klass)
{
    auto ret = ets_byte(0xf1);
    return ret;
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_checkByte([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                                      [[maybe_unused]] ets_byte value)
{
    assert(value == ets_byte(0xf1));
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT ets_char ETS_EtsNapiTests_returnChar([[maybe_unused]] EtsEnv *env,
                                                           [[maybe_unused]] ets_class klass)
{
    auto ret = ets_char(0xf1ff);
    return ret;
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_checkChar([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                                      [[maybe_unused]] ets_char value)
{
    assert(value == ets_char(0xf1ff));
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT ets_short ETS_EtsNapiTests_returnShort([[maybe_unused]] EtsEnv *env,
                                                             [[maybe_unused]] ets_class klass)
{
    auto ret = ets_short(0xf2);
    return ret;
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_checkShort([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                                       [[maybe_unused]] ets_short value)
{
    assert(value == ets_short(0xf2));
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT ets_int ETS_EtsNapiTests_returnInt([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass)
{
    auto ret = ets_int(0xffff00ff);
    return ret;
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_checkInt([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                                     [[maybe_unused]] ets_int value)
{
    assert(value == ets_int(0xffff00ff));
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT ets_long ETS_EtsNapiTests_returnLong([[maybe_unused]] EtsEnv *env,
                                                           [[maybe_unused]] ets_class klass)
{
    auto ret = ets_long(0xffff00ffffff1111);
    return ret;
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_checkLong([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                                      [[maybe_unused]] ets_long value)
{
    assert(value == ets_long(0xffff00ffffff1111));
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT ets_float ETS_EtsNapiTests_returnFloat([[maybe_unused]] EtsEnv *env,
                                                             [[maybe_unused]] ets_class klass)
{
    ets_float ret = 6.54321F;
    return ret;
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_checkFloat([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                                       [[maybe_unused]] ets_float value)
{
    assert(Equal(value, 6.54321F));
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT ets_double ETS_EtsNapiTests_returnDouble([[maybe_unused]] EtsEnv *env,
                                                               [[maybe_unused]] ets_class klass)
{
    ets_double ret = 0.0123456789;
    return ret;
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_checkDouble([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                                        [[maybe_unused]] ets_double value)
{
    assert(Equal(value, 0.0123456789));
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT ets_string ETS_EtsNapiTests_returnString([[maybe_unused]] EtsEnv *env,
                                                               [[maybe_unused]] ets_class klass)
{
    std::string ret("UTF_STRING@#$");
    return env->NewStringUTF(ret.c_str());
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void ETS_EtsNapiTests_checkString([[maybe_unused]] EtsEnv *env, [[maybe_unused]] ets_class klass,
                                                        [[maybe_unused]] ets_string value)
{
    std::string ret = GetString(env, value);
    assert(ret == "UTF_STRING@#$");
}

// NOTE(a.kropacheva): enable tests when SEGV catch will be implemented
/*
static int *illgalpointer = nullptr;

int makeSEGV(int stackdepth)
{
    if (stackdepth > 0) // make some call stack with depth
        return stackdepth * makeSEGV(stackdepth - 1);
    *illgalpointer = 0;    // this produce Segmentation fault
    return 0;
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void managedThreadSEGV(EtsEnv*,  [[maybe_unused]] ets_class klass)
{
    makeSEGV(17);
    return;
}

void *thread_func([[maybe_unused]] void *dummy)
{
    *illgalpointer = 0;
    return NULL;
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" ETS_EXPORT void nativeThreadSEGV(EtsEnv*,  [[maybe_unused]] ets_class klass)
{
    illgalpointer++; // touch to prevent optimization

    p thread_t thread;
    if (p thread_create(&thread, NULL, thread_func, NULL))
        std::cout << "Error creating thread\n";
    if (p thread_join(thread, NULL))
        std::cout << "Error joining thread\n";
    return;
}
*/

// NOLINTEND(readability-magic-numbers)
