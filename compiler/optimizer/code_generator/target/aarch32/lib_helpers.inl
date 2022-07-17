/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

// Gcc lib-helpers:
#ifdef __arm__
// defined in libgcc_s.so
extern "C" uint64_t __aeabi_uldivmod(uint64_t numerator, uint64_t denominator);  // NOLINT(misc-definitions-in-headers)
extern "C" int64_t __aeabi_ldivmod(int64_t numerator, int64_t denominator);      // NOLINT(misc-definitions-in-headers)

extern "C" uint32_t __aeabi_uidivmod(uint32_t numerator, uint32_t denominator);  // NOLINT(misc-definitions-in-headers)
extern "C" int32_t __aeabi_idivmod(int32_t numerator, int32_t denominator);      // NOLINT(misc-definitions-in-headers)

extern "C" float __aeabi_l2f(int64_t data);     // NOLINT(misc-definitions-in-headers)
extern "C" float __aeabi_ul2f(uint64_t data);   // NOLINT(misc-definitions-in-headers)
extern "C" double __aeabi_l2d(int64_t data);    // NOLINT(misc-definitions-in-headers)
extern "C" double __aeabi_ul2d(uint64_t data);  // NOLINT(misc-definitions-in-headers)

extern "C" int64_t __aeabi_f2lz(float data);     // NOLINT(misc-definitions-in-headers)
extern "C" uint64_t __aeabi_f2ulz(float data);   // NOLINT(misc-definitions-in-headers)
extern "C" int64_t __aeabi_d2lz(double data);    // NOLINT(misc-definitions-in-headers)
extern "C" uint64_t __aeabi_d2ulz(double data);  // NOLINT(misc-definitions-in-headers)
#else
struct DivLUResult {
    uint64_t quotient;
    uint64_t remainder;
};

struct DivLSResult {
    int64_t quotient;
    int64_t remainder;
};

// NOLINTNEXTLINE(readability-identifier-naming,misc-definitions-in-headers)
DivLUResult __aeabi_uldivmod(uint64_t numerator, uint64_t denominator)
{
    DivLUResult res {0, 0};
    res.quotient = numerator / denominator;
    res.remainder = numerator % denominator;
    return res;
}
// NOLINTNEXTLINE(readability-identifier-naming,misc-definitions-in-headers)
DivLSResult __aeabi_ldivmod(int64_t numerator, int64_t denominator)
{
    DivLSResult res {0, 0};
    res.quotient = numerator / denominator;
    res.remainder = numerator % denominator;
    return res;
}

struct DivUResult {
    uint32_t quotient;
    uint32_t remainder;
};

struct DivSResult {
    int32_t quotient;
    int32_t remainder;
};

// NOLINTNEXTLINE(readability-identifier-naming,misc-definitions-in-headers)
DivUResult __aeabi_uidivmod(uint32_t numerator, uint32_t denominator)
{
    DivUResult res {0, 0};
    res.quotient = numerator / denominator;
    res.remainder = numerator % denominator;
    return res;
}
// NOLINTNEXTLINE(readability-identifier-naming,misc-definitions-in-headers)
DivSResult __aeabi_idivmod(int32_t numerator, int32_t denominator)
{
    DivSResult res {0, 0};
    res.quotient = numerator / denominator;
    res.remainder = numerator % denominator;
    return res;
}

// NOLINTNEXTLINE(readability-identifier-naming,misc-definitions-in-headers)
float __aeabi_l2f(int64_t data)
{
    return static_cast<float>(data);
}

// NOLINTNEXTLINE(readability-identifier-naming,misc-definitions-in-headers)
float __aeabi_ul2f(uint64_t data)
{
    return static_cast<float>(data);
}

// NOLINTNEXTLINE(readability-identifier-naming,misc-definitions-in-headers)
double __aeabi_l2d(int64_t data)
{
    return static_cast<double>(data);
}

// NOLINTNEXTLINE(readability-identifier-naming,misc-definitions-in-headers)
double __aeabi_ul2d(uint64_t data)
{
    return static_cast<double>(data);
}

// NOLINTNEXTLINE(readability-identifier-naming,misc-definitions-in-headers)
int64_t __aeabi_f2lz(float data)
{
    return static_cast<int64_t>(data);
}

// NOLINTNEXTLINE(readability-identifier-naming,misc-definitions-in-headers)
uint64_t __aeabi_f2ulz(float data)
{
    return static_cast<uint64_t>(data);
}

// NOLINTNEXTLINE(readability-identifier-naming,misc-definitions-in-headers)
int64_t __aeabi_d2lz(double data)
{
    return static_cast<int64_t>(data);
}

// NOLINTNEXTLINE(readability-identifier-naming,misc-definitions-in-headers)
uint64_t __aeabi_d2ulz(double data)
{
    return static_cast<uint64_t>(data);
}
#endif

// defined in libm.so, fmodf function from math.h
extern "C" float fmodf(float a, float b);  // NOLINT(misc-definitions-in-headers,readability-identifier-naming)
// defined in libm.so, fmod function from math.h
extern "C" double fmod(double a, double b);  // NOLINT(misc-definitions-in-headers,readability-identifier-naming)
