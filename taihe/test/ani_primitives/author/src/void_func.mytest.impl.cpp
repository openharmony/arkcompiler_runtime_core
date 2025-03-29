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
#include "void_func.mytest.impl.hpp"

#include "core/optional.hpp"
#include "core/string.hpp"
#include "stdexcept"
#include "void_func.mytest.BarTest.proj.0.hpp"
// Please delete <stdexcept> include when you implement
using namespace taihe::core;
namespace {
void myfunc1()
{
    std::cout << "myfunc1 is true " << std::endl;
}
void myfunc2(int8_t option1, int16_t option2)
{
    std::cout << "myfunc2 is option1  " << (int)option1 << std::endl;
    std::cout << "myfunc2 is option2  " << option2 << std::endl;
}
void myfunc3(int32_t option1, bool option2)
{
    if (option2) {
        std::cout << "myfunc3 is option1  " << option1 << std::endl;
        std::cout << "myfunc3 is option2  " << option2 << std::endl;
    } else {
        std::cout << "myfunc3 is option1  " << option1 << std::endl;
        std::cout << "myfunc3 is option2  " << option2 << std::endl;
    }
}
void myfunc4(int32_t option1, int64_t option2)
{
    std::cout << "myfunc4 is option1  " << option1 << std::endl;
    std::cout << "myfunc4 is option2  " << option2 << std::endl;
}
void myfunc5(int32_t option1, string_view option2)
{
    std::cout << "myfunc5 is option1  " << option1 << std::endl;
    std::cout << "myfunc5 is option2  " << option2 << std::endl;
}
void myfunc6(int64_t option1, bool option2)
{
    if (option2) {
        std::cout << "myfunc6 is option1  " << option1 << std::endl;
        std::cout << "myfunc6 is option2  " << option2 << std::endl;
    } else {
        std::cout << "myfunc6 is option1  " << option1 << std::endl;
        std::cout << "myfunc6 is option2  " << option2 << std::endl;
    }
}
void myfunc7(int64_t option1, float option2)
{
    std::cout << "myfunc7 is option1  " << option1 << std::endl;
    std::cout << "myfunc7 is option2  " << option2 << std::endl;
}
void myfunc8(int64_t option1, double option2)
{
    std::cout << "myfunc8 is option1  " << option1 << std::endl;
    std::cout << "myfunc8 is option2  " << option2 << std::endl;
}
void myfunc9(float option1, bool option2)
{
    if (option2) {
        std::cout << "myfunc9 is option1  " << option1 << std::endl;
        std::cout << "myfunc9 is option2  " << option2 << std::endl;
    } else {
        std::cout << "myfunc9 is option1  " << option1 << std::endl;
        std::cout << "myfunc9 is option2  " << option2 << std::endl;
    }
}
void myfunc10(float option1, string_view option2)
{
    std::cout << "myfunc10 is option1  " << option1 << std::endl;
    std::cout << "myfunc10 is option2  " << option2 << std::endl;
}
void myfunc11(double option1, string_view option2)
{
    std::cout << "myfunc11 is option1  " << option1 << std::endl;
    std::cout << "myfunc11 is option2  " << option2 << std::endl;
}
void myfunc12(optional_view<int32_t> option1, optional_view<int64_t> option2)
{
    if (option1) {
        std::cout << *option1 << std::endl;
    } else if (option2) {
        std::cout << *option2 << std::endl;
    } else {
        std::cout << "Null" << std::endl;
    }
}
void myfunc13(optional_view<float> option1, optional_view<double> option2)
{
    if (option1) {
        std::cout << *option1 << std::endl;
    } else if (option2) {
        std::cout << *option2 << std::endl;
    } else {
        std::cout << "Null" << std::endl;
    }
}
void myfunc14(optional_view<string> option1, optional_view<bool> option2)
{
    if (option1) {
        std::cout << *option1 << std::endl;
    } else if (option2) {
        std::cout << *option2 << std::endl;
    } else {
        std::cout << "Null" << std::endl;
    }
}
void myfunc15(optional_view<int16_t> option1, optional_view<int64_t> option2)
{
    if (option1) {
        std::cout << *option1 << std::endl;
    } else if (option2) {
        std::cout << *option2 << std::endl;
    } else {
        std::cout << "Null" << std::endl;
    }
}
void myfunc16(optional_view<int16_t> option1, ::void_func::mytest::BarTest const &option2)
{
    switch (option2.get_key()) {
        case ::void_func::mytest::BarTest::key_t::A:
            std::cout << static_cast<int32_t>(option2.get_key()) << " A: " << std::endl;
            break;
        case ::void_func::mytest::BarTest::key_t::B:
            std::cout << static_cast<int32_t>(option2.get_key()) << " B: " << std::endl;
            break;
        case ::void_func::mytest::BarTest::key_t::C:
            std::cout << static_cast<int32_t>(option2.get_key()) << " C: " << std::endl;
            break;
    }
}
}  // namespace
TH_EXPORT_CPP_API_myfunc1(myfunc1);
TH_EXPORT_CPP_API_myfunc2(myfunc2);
TH_EXPORT_CPP_API_myfunc3(myfunc3);
TH_EXPORT_CPP_API_myfunc4(myfunc4);
TH_EXPORT_CPP_API_myfunc5(myfunc5);
TH_EXPORT_CPP_API_myfunc6(myfunc6);
TH_EXPORT_CPP_API_myfunc7(myfunc7);
TH_EXPORT_CPP_API_myfunc8(myfunc8);
TH_EXPORT_CPP_API_myfunc9(myfunc9);
TH_EXPORT_CPP_API_myfunc10(myfunc10);
TH_EXPORT_CPP_API_myfunc11(myfunc11);
TH_EXPORT_CPP_API_myfunc12(myfunc12);
TH_EXPORT_CPP_API_myfunc13(myfunc13);
TH_EXPORT_CPP_API_myfunc14(myfunc14);
TH_EXPORT_CPP_API_myfunc15(myfunc15);
TH_EXPORT_CPP_API_myfunc16(myfunc16);
