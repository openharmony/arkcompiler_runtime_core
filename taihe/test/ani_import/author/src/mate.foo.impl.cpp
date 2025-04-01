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
#include "mate.foo.impl.hpp"

#include "mate.MateType.proj.1.hpp"
#include "nova.NovaType.proj.2.hpp"
#include "pura.PuraType.proj.0.hpp"
#include "stdexcept"
// Please delete <stdexcept> include when you implement
using namespace taihe::core;
namespace {
void testMate(::mate::MateType const &mate)
{
    throw std::runtime_error("Function testMate Not implemented");
}
void testNova(::nova::weak::NovaType nova)
{
    throw std::runtime_error("Function testNova Not implemented");
}
void testPura(::pura::PuraType pura)
{
    throw std::runtime_error("Function testPura Not implemented");
}
}  // namespace
TH_EXPORT_CPP_API_testMate(testMate);
TH_EXPORT_CPP_API_testNova(testNova);
TH_EXPORT_CPP_API_testPura(testPura);
