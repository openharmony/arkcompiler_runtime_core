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
#include "opt.impl.hpp"

#include <iostream>

#include "opt.ReturnResult.proj.1.hpp"
#include "taihe/array.hpp"
#include "taihe/optional.hpp"
#include "taihe/string.hpp"

using namespace taihe;

namespace {
void showOptionalInt(optional_view<int32_t> x)
{
    if (x) {
        std::cout << *x << std::endl;
    } else {
        std::cout << "Null" << std::endl;
    }
}

optional<int32_t> makeOptionalInt(bool b)
{
    if (b) {
        const int optionalMakeValue = 10;
        return optional<int32_t>::make(optionalMakeValue);
    } else {
        return optional<int32_t>(nullptr);
    }
}

optional<array<int32_t>> makeOptionalArray(bool b, int32_t val, int32_t num)
{
    if (b) {
        return optional<array<int32_t>>::make(array<int32_t>::make(num, val));
    } else {
        return optional<array<int32_t>>(nullptr);
    }
}

optional<string> sendReturnResult(::opt::ReturnResult const &result)
{
    if (result.results) {
        string ret = "";
        for (auto str : *result.results) {
            ret = ret + str;
        }
        return optional<string>::make(ret);
    } else {
        return optional<string>(nullptr);
    }
}
}  // namespace

// The macros used below are automatically generated code
// NOLINTBEGIN
TH_EXPORT_CPP_API_showOptionalInt(showOptionalInt);
TH_EXPORT_CPP_API_makeOptionalInt(makeOptionalInt);
TH_EXPORT_CPP_API_makeOptionalArray(makeOptionalArray);
TH_EXPORT_CPP_API_sendReturnResult(sendReturnResult);
// NOLINTEND
