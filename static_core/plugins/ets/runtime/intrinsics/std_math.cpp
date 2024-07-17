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

#include <climits>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <random>
#include <cmath>

#include "libpandabase/utils/bit_utils.h"
#include "macros.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_vm.h"

namespace ark::ets::intrinsics {

namespace {
constexpr uint64_t SIGN_MASK = 0x8000'0000'0000'0000;
constexpr uint64_t EXPONENT_MASK = 0x7FF0'0000'0000'0000;
constexpr uint64_t SIGNIFICANT_MASK = 0x000F'FFFF'FFFF'FFFF;
constexpr uint64_t WORD_MASK = 0xFFFF'FFFFUL;
constexpr uint64_t HIDDEN_BIT = 0x0010'0000'0000'0000;
constexpr uint64_t PHYSICAL_SIGNIFICAND_SIZE = 52;
constexpr int MAX_EXPONENT = 31;
constexpr int SIGNIFICAND_SIZE = 53;
constexpr int EXPONENT_BIAS = 0x3FF + PHYSICAL_SIGNIFICAND_SIZE;
constexpr int DENORMAL_EXPONENT = -EXPONENT_BIAS + 1;

bool IsDenormal(uint64_t d64)
{
    return (d64 & EXPONENT_MASK) == 0;
}

int Sign(uint64_t d64)
{
    return (d64 & SIGN_MASK) == 0 ? 1 : -1;
}

int Exponent(uint64_t d64)
{
    if (IsDenormal(d64)) {
        return DENORMAL_EXPONENT;
    }
    int biasedE = static_cast<int>((d64 & EXPONENT_MASK) >> PHYSICAL_SIGNIFICAND_SIZE);
    return biasedE - EXPONENT_BIAS;
}

uint64_t Significand(uint64_t d64)
{
    uint64_t significand = d64 & SIGNIFICANT_MASK;
    if (!IsDenormal(d64)) {
        return significand + HIDDEN_BIT;
    }
    return significand;
}

int32_t ToInt32(double x)
{
    if ((std::isfinite(x)) && (x <= INT_MAX) && (x >= INT_MIN)) {
        return static_cast<int32_t>(x);
    }

    uint64_t d64;
    // NOTE(kirill-mitkin): can be __builtin_bit_cast() if defined
    if (memcpy_s(&d64, sizeof(uint64_t), &x, sizeof(uint64_t)) != 0) {
        UNREACHABLE();
    }

    int exponent = Exponent(d64);
    uint64_t significand = Significand(d64);
    uint64_t bits;
    if (exponent < 0) {
        if (exponent <= SIGNIFICAND_SIZE) {
            return 0;
        }
        bits = significand >> static_cast<uint64_t>(-exponent);
    } else {
        if (exponent > MAX_EXPONENT) {
            return 0;
        }
        bits = (significand << static_cast<uint64_t>(exponent)) & WORD_MASK;
    }
    return static_cast<int32_t>(Sign(d64) * static_cast<int64_t>(bits));
}

uint32_t ToUint32(double x)
{
    return static_cast<uint32_t>(ToInt32(x));
}
}  // namespace

extern "C" double StdMathRandom()
{
    std::uniform_real_distribution<double> urd(0.0, 1.0);
    return urd(EtsCoroutine::GetCurrent()->GetPandaVM()->GetRandomEngine());
}

extern "C" double StdMathAcos(double val)
{
    return std::acos(val);
}

extern "C" double StdMathAcosh(double val)
{
    return std::acosh(val);
}

extern "C" double StdMathAsin(double val)
{
    return std::asin(val);
}

extern "C" double StdMathAsinh(double val)
{
    return std::asinh(val);
}

extern "C" double StdMathAtan2(double val1, double val2)
{
    return std::atan2(val1, val2);
}

extern "C" double StdMathAtanh(double val)
{
    return std::atanh(val);
}

extern "C" double StdMathAtan(double val)
{
    return std::atan(val);
}

extern "C" double StdMathSinh(double val)
{
    return std::sinh(val);
}

extern "C" double StdMathCosh(double val)
{
    return std::cosh(val);
}

extern "C" double StdMathFloor(double val)
{
    return std::floor(val);
}

extern "C" double StdMathRound(double val)
{
    return std::round(val);
}

extern "C" double StdMathTrunc(double val)
{
    return std::trunc(val);
}

extern "C" double StdMathCbrt(double val)
{
    return std::cbrt(val);
}

extern "C" double StdMathTan(double val)
{
    return std::tan(val);
}

extern "C" double StdMathTanh(double val)
{
    return std::tanh(val);
}

extern "C" double StdMathExp(double val)
{
    return std::exp(val);
}

extern "C" double StdMathLog10(double val)
{
    return std::log10(val);
}

extern "C" double StdMathExpm1(double val)
{
    return std::expm1(val);
}

extern "C" double StdMathCeil(double val)
{
    return std::ceil(val);
}

extern "C" int32_t StdMathClz64(int64_t val)
{
    if (val != 0) {
        return Clz(static_cast<uint64_t>(val));
    }
    return std::numeric_limits<uint64_t>::digits;
}

extern "C" int32_t StdMathClz32(int32_t val)
{
    if (val != 0) {
        return Clz(static_cast<uint32_t>(val));
    }
    return std::numeric_limits<uint32_t>::digits;
}

extern "C" double StdMathClz32Double(double val)
{
    auto intValue = ToUint32(val);
    if (intValue != 0) {
        return static_cast<double>(Clz(intValue));
    }
    return std::numeric_limits<uint32_t>::digits;
}

extern "C" double StdMathLog(double val)
{
    return std::log(val);
}

extern "C" double StdMathRem(double val, double val2)
{
    return std::remainder(val, val2);
}

extern "C" double StdMathMod(double val, double val2)
{
    return std::fmod(val, val2);
}

extern "C" bool StdMathSignbit(double val)
{
    return std::signbit(val);
}

extern "C" int StdMathImul(double val, double val2)
{
    if (!std::isfinite(val) || !std::isfinite(val2)) {
        return 0;
    }
    return static_cast<int32_t>(static_cast<int64_t>(val) * static_cast<int64_t>(val2));
}

extern "C" double StdMathFround(double val)
{
    if (std::isnan(val)) {
        return std::numeric_limits<float>::quiet_NaN();
    }

    return static_cast<float>(val);
}
extern "C" double StdMathHypot(double val1, double val2)
{
    return std::hypot(val1, val2);
}
}  // namespace ark::ets::intrinsics
