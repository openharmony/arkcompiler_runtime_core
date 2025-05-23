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
#include "foo.impl.hpp"
#include "stdexcept"
#include "taihe/array.hpp"
#include "taihe/runtime.hpp"
using namespace taihe;

namespace {
void inArrayBuffer(array_view<uint16_t> v)
{
    std::cout << "inArrayBuffer:" << std::endl;
    int i = 0;
    for (auto x : v) {
        std::cout << "v[" << i << "] = " << x << std::endl;
    }
}

array<uint16_t> outArrayBuffer()
{
    return {0, 1, 2, 3};
}
}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_inArrayBuffer(inArrayBuffer);
TH_EXPORT_CPP_API_outArrayBuffer(outArrayBuffer);
// NOLINTEND
