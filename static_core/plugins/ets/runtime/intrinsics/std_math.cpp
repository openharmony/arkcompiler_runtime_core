/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
constexpr int INT_MAX_SIZE = 63;
constexpr double ROUND_BIAS = 0.5;

int32_t ToInt32(double x)
{
    if (std::isinf(x)) {
        return 0;
    }

    if ((std::isfinite(x)) && (x <= INT_MAX) && (x >= INT_MIN)) {
        return static_cast<int32_t>(x);
    }

    double intPart = 0.0;
    std::modf(x, &intPart);
    double int64Max = std::pow(2, INT_MAX_SIZE);
    intPart = std::fmod(intPart, int64Max);
    return static_cast<int32_t>(static_cast<int64_t>(intPart));
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
    if (std::signbit(val) && val >= -ROUND_BIAS) {
        return -0.0;
    }
    double res = std::ceil(val);
    if (res - val > ROUND_BIAS) {
        res -= 1.0;
    }
    return res;
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
