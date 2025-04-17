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
#include "bigint_new.impl.hpp"
#include <cstdint>
#include "bigint_new.proj.hpp"
#include "stdexcept"
#include "taihe/runtime.hpp"

#include <iostream>

using namespace taihe;

namespace {
// To be implemented.

array<uint64_t> processBigInt(array_view<uint64_t> a)
{
    array<uint64_t> res(a.size() + 1);
    res[0] = 0;
    for (int i = 0; i < a.size(); i++) {
        res[i + 1] = a[i];
        std::cerr << "arr[" << i << "] = " << a[i] << std::endl;
    }
    return res;
}
}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_processBigInt(processBigInt);
// NOLINTEND