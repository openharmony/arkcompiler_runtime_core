/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <array>
#include <iostream>

#include "ets_napi.h"

// NOLINTBEGIN(readability-magic-numbers)

// CC-OFFNXT(G.PRE.02-CPP) testing macro with source line info for finding errors
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_EQUAL(a, b)                                                                                 \
    {                                                                                                     \
        auto aValue = (a);                                                                                \
        auto bValue = (b);                                                                                \
        if (aValue != bValue) {                                                                           \
            ++errorCount;                                                                                 \
            std::cerr << "LINE " << __LINE__ << ", " << #a << ": " << aValue << " != " << bValue << '\n'; \
        }                                                                                                 \
    }

// CC-OFFNXT(G.FUN.01-CPP, readability-function-size_parameters) function for testing
static ets_int Direct(ets_int im1, ets_byte i0, ets_short i1, ets_int i2, ets_long i3, ets_float f0, ets_double f1,
                      ets_double f2, ets_int i4, ets_int i5, ets_int i6, ets_long i7, ets_int i8, ets_short i9,
                      ets_int i10, ets_int i11, ets_int i12, ets_int i13, ets_int i14, ets_float f3, ets_float f4,
                      ets_double f5, ets_double f6, ets_double f7, ets_double f8, ets_double f9, ets_double f10,
                      ets_double f11, ets_double f12, ets_double f13, ets_double f14)
{
    ets_int errorCount = 0L;

    CHECK_EQUAL(im1, -1L);
    CHECK_EQUAL(static_cast<ets_int>(i0), 0L);
    CHECK_EQUAL(i1, 1L);
    CHECK_EQUAL(i2, 2L);
    CHECK_EQUAL(i3, 3L);
    CHECK_EQUAL(i4, 4L);
    CHECK_EQUAL(i5, 5L);
    CHECK_EQUAL(i6, 6L);
    CHECK_EQUAL(i7, 7L);
    CHECK_EQUAL(i8, 8L);
    CHECK_EQUAL(i9, 9L);
    CHECK_EQUAL(i10, 10L);
    CHECK_EQUAL(i11, 11L);
    CHECK_EQUAL(i12, 12L);
    CHECK_EQUAL(i13, 13L);
    CHECK_EQUAL(i14, 14L);

    CHECK_EQUAL(f0, 0.0L);
    CHECK_EQUAL(f1, 1.0L);
    CHECK_EQUAL(f2, 2.0L);
    CHECK_EQUAL(f3, 3.0L);
    CHECK_EQUAL(f4, 4.0L);
    CHECK_EQUAL(f5, 5.0L);
    CHECK_EQUAL(f6, 6.0L);
    CHECK_EQUAL(f7, 7.0L);
    CHECK_EQUAL(f8, 8.0L);
    CHECK_EQUAL(f9, 9.0L);
    CHECK_EQUAL(f10, 10.0L);
    CHECK_EQUAL(f11, 11.0L);
    CHECK_EQUAL(f12, 12.0L);
    CHECK_EQUAL(f13, 13.0L);
    CHECK_EQUAL(f14, 14.0L);

    return errorCount;
}

extern "C" ets_int EtsNapiOnLoad(EtsEnv *env)
{
    ets_class nativeModuleClass = env->FindClass("NativeModule");
    if (nativeModuleClass == nullptr) {
        return ETS_ERR;
    }

    const std::array methods = {EtsNativeMethod {"direct", nullptr, reinterpret_cast<void *>(Direct)}};
    ets_int errCode = env->RegisterNatives(nativeModuleClass, methods.data(), methods.size());
    return errCode == ETS_OK ? ETS_NAPI_VERSION_1_0 : errCode;
}

// NOLINTEND(readability-magic-numbers)
