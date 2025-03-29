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
#include "struct_test.impl.hpp"

#include "stdexcept"
// Please delete this include when you implement
using namespace taihe::core;
namespace {

string concatStruct(::struct_test::Data const &data)
{
    return concat(concat(data.a, data.b), to_string(data.c));
}

::struct_test::Data makeStruct(string_view a, string_view b, int32_t c)
{
    return {a, b, c};
}

}  // namespace

TH_EXPORT_CPP_API_concatStruct(concatStruct);
TH_EXPORT_CPP_API_makeStruct(makeStruct);
