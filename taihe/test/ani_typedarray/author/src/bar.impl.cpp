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
#include "bar.impl.hpp"
#include "bar.proj.hpp"
#include "stdexcept"
#include "taihe/runtime.hpp"

#include <numeric>

using namespace taihe;

namespace {
// To be implemented.

int8_t sumUint8Array(array_view<uint8_t> v)
{
    return std::accumulate(v.begin(), v.end(), 0);
}

array<uint8_t> newUint8Array(int64_t n, int8_t v)
{
    array<uint8_t> result(n);
    std::fill(result.begin(), result.end(), v);
    return result;
}

int16_t sumUint16Array(array_view<uint16_t> v)
{
    return std::accumulate(v.begin(), v.end(), 0);
}

array<uint16_t> newUint16Array(int64_t n, int16_t v)
{
    array<uint16_t> result(n);
    std::fill(result.begin(), result.end(), v);
    return result;
}

int32_t sumUint32Array(array_view<uint32_t> v)
{
    return std::accumulate(v.begin(), v.end(), 0);
}

array<uint32_t> newUint32Array(int64_t n, int32_t v)
{
    array<uint32_t> result(n);
    std::fill(result.begin(), result.end(), v);
    return result;
}

int64_t sumBigUint64Array(array_view<uint64_t> v)
{
    return std::accumulate(v.begin(), v.end(), 0);
}

array<uint64_t> newBigUint64Array(int64_t n, int64_t v)
{
    array<uint64_t> result(n);
    std::fill(result.begin(), result.end(), v);
    return result;
}

int8_t sumInt8Array(array_view<int8_t> v)
{
    return std::accumulate(v.begin(), v.end(), 0);
}

array<int8_t> newInt8Array(int64_t n, int8_t v)
{
    array<int8_t> result(n);
    std::fill(result.begin(), result.end(), v);
    return result;
}

int16_t sumInt16Array(array_view<int16_t> v)
{
    return std::accumulate(v.begin(), v.end(), 0);
}

array<int16_t> newInt16Array(int64_t n, int16_t v)
{
    array<int16_t> result(n);
    std::fill(result.begin(), result.end(), v);
    return result;
}

int32_t sumInt32Array(array_view<int32_t> v)
{
    return std::accumulate(v.begin(), v.end(), 0);
}

array<int32_t> newInt32Array(int64_t n, int32_t v)
{
    array<int32_t> result(n);
    std::fill(result.begin(), result.end(), v);
    return result;
}

int64_t sumBigInt64Array(array_view<int64_t> v)
{
    return std::accumulate(v.begin(), v.end(), 0);
}

array<int64_t> newBigInt64Array(int64_t n, int64_t v)
{
    array<int64_t> result(n);
    std::fill(result.begin(), result.end(), v);
    return result;
}

float sumFloat32Array(array_view<float> v)
{
    return std::accumulate(v.begin(), v.end(), 0.0f);
}

array<float> newFloat32Array(int64_t n, float v)
{
    array<float> result(n);
    std::fill(result.begin(), result.end(), v);
    return result;
}

double sumFloat64Array(array_view<double> v)
{
    return std::accumulate(v.begin(), v.end(), 0.0);
}

array<double> newFloat64Array(int64_t n, double v)
{
    array<double> result(n);
    std::fill(result.begin(), result.end(), v);
    return result;
}
}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_sumUint8Array(sumUint8Array);
TH_EXPORT_CPP_API_newUint8Array(newUint8Array);
TH_EXPORT_CPP_API_sumUint16Array(sumUint16Array);
TH_EXPORT_CPP_API_newUint16Array(newUint16Array);
TH_EXPORT_CPP_API_sumUint32Array(sumUint32Array);
TH_EXPORT_CPP_API_newUint32Array(newUint32Array);
TH_EXPORT_CPP_API_sumBigUint64Array(sumBigUint64Array);
TH_EXPORT_CPP_API_newBigUint64Array(newBigUint64Array);
TH_EXPORT_CPP_API_sumInt8Array(sumInt8Array);
TH_EXPORT_CPP_API_newInt8Array(newInt8Array);
TH_EXPORT_CPP_API_sumInt16Array(sumInt16Array);
TH_EXPORT_CPP_API_newInt16Array(newInt16Array);
TH_EXPORT_CPP_API_sumInt32Array(sumInt32Array);
TH_EXPORT_CPP_API_newInt32Array(newInt32Array);
TH_EXPORT_CPP_API_sumBigInt64Array(sumBigInt64Array);
TH_EXPORT_CPP_API_newBigInt64Array(newBigInt64Array);
TH_EXPORT_CPP_API_sumFloat32Array(sumFloat32Array);
TH_EXPORT_CPP_API_newFloat32Array(newFloat32Array);
TH_EXPORT_CPP_API_sumFloat64Array(sumFloat64Array);
TH_EXPORT_CPP_API_newFloat64Array(newFloat64Array);
// NOLINTEND