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
#include "pura.impl.hpp"

#include "mate.bar.BarType.proj.1.hpp"
#include "nova.NovaType.proj.2.hpp"
#include "stdexcept"
// Please delete <stdexcept> include when you implement
using namespace taihe;

namespace {
void testBar(::mate::bar::BarType const &bar)
{
    throw std::runtime_error("Function testBar Not implemented");
}

void testNova(::nova::weak::NovaType nove)
{
    throw std::runtime_error("Function testNova Not implemented");
}
}  // namespace

TH_EXPORT_CPP_API_testBar(testBar);
TH_EXPORT_CPP_API_testNova(testNova);
