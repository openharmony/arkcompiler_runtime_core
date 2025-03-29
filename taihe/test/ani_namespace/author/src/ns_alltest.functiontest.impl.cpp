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
#include "ns_alltest.functiontest.impl.hpp"

#include <iostream>

#include "core/array.hpp"
#include "core/map.hpp"
#include "core/optional.hpp"
#include "core/runtime.hpp"
#include "core/string.hpp"
#include "ns_alltest.functiontest.Color.proj.0.hpp"
#include "ns_alltest.functiontest.Data.proj.1.hpp"

using namespace taihe::core;
namespace {
class testNameSpace {
    string testStr_ {"testNameSpace"};

public:
    void baseFunctionTest43()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }
    int8_t baseFunctionTest44(int8_t param1)
    {
        // 检查结果是否超出 int8_t 的范围
        int32_t result = static_cast<int32_t>(param1) + 100;
        if (result > INT8_MAX || result < INT8_MIN) {
            taihe::core::set_error("baseFunctionTest44: result exceeds int8_t range");
        }
        // 返回结果
        return static_cast<int8_t>(result);
    }
    int16_t baseFunctionTest45(int16_t param1)
    {
        // 检查结果是否超出 int16_t 的范围
        int32_t result = static_cast<int32_t>(param1) * 100;
        if (result > INT16_MAX || result < INT16_MIN) {
            taihe::core::set_error("baseFunctionTest45: result exceeds int16_t range");
        }
        // 返回结果
        return static_cast<int16_t>(result);
    }
    int32_t baseFunctionTest46(int32_t param1)
    {
        // 检查结果是否超出 int32_t 的范围
        int64_t result = static_cast<int64_t>(param1) * 100;
        std::cout << "NameSpaceImpl: " << __func__ << " result " << result << std::endl;
        if (result > INT32_MAX || result < INT32_MIN) {
            taihe::core::set_error("baseFunctionTest18: result exceeds int32_t range");
        }
        // 返回结果
        return static_cast<int32_t>(result);
    }
    int64_t baseFunctionTest47(int64_t param1)
    {
        // 检查结果是否超出 int64_t 的范围
        int64_t result = param1 * 100;
        std::cout << "NameSpaceImpl: " << __func__ << " result " << result << std::endl;
        if (result > INT64_MAX || result < INT64_MIN) {
            taihe::core::set_error("baseFunctionTest19: result exceeds int64_t range");
        }
        // 返回结果
        return static_cast<int64_t>(result);
    }
    float baseFunctionTest48(float param1)
    {
        return param1 - 10;
    }
    double baseFunctionTest49(double param1)
    {
        return param1 - 100;
    }
    string baseFunctionTest50(string_view param1)
    {
        return std::string(param1) + "baseFunctionTest50";
    }
    bool baseFunctionTest51(bool param1)
    {
        if (param1) {
            return false;
        } else {
            return true;
        }
    }
    array<uint8_t> baseFunctionTest52(array_view<uint8_t> param1)
    {
        array<uint8_t> result = array<uint8_t>::make(param1.size());
        for (int i = 0; i < param1.size(); i++) {
            result[i] = param1[i] + 10;
        }
        return result;
    }
    optional<int8_t> baseFunctionTest53(optional_view<int8_t> param1)
    {
        if (param1) {
            // 检查结果是否超出 int8_t 的范围
            int32_t result = static_cast<int32_t>(*param1) - 100;
            std::cout << "NameSpaceImpl: " << __func__ << " result " << result << std::endl;
            if (result > INT8_MAX || result < INT8_MIN) {
                taihe::core::set_error("baseFunctionTest53: result exceeds int8_t range");
            }
            // 返回结果
            return optional<int8_t>::make(result);
        } else {
            std::cout << "NameSpaceImpl: " << __func__ << " param1 is null" << std::endl;
            return optional<int8_t>(nullptr);
        }
    }
    optional<int32_t> baseFunctionTest54(optional_view<int32_t> param1)
    {
        if (param1) {
            // 检查结果是否超出 int32_t 的范围
            int32_t result = static_cast<int32_t>(*param1) - 100;
            std::cout << "NameSpaceImpl: " << __func__ << " result " << result << std::endl;
            if (result > INT32_MAX || result < INT32_MIN) {
                taihe::core::set_error("baseFunctionTest54: result exceeds int32_t range");
            }
            // 返回结果
            return optional<int32_t>::make(result);
        } else {
            std::cout << "NameSpaceImpl: " << __func__ << " param1 is null" << std::endl;
            return optional<int32_t>(nullptr);
        }
    }
    map<int16_t, ::ns_alltest::functiontest::Data> baseFunctionTest55(
        map_view<int16_t, ::ns_alltest::functiontest::Data> param1)
    {
        map<int16_t, ::ns_alltest::functiontest::Data> m;
        // for (int i = 0; i < param1.size(); ++i) {
        //     m.emplace(i, param1);
        // }
        // return m;
        return m;
    }
    int64_t baseFunctionTest56(int32_t param1)
    {
        int64_t result = static_cast<int64_t>(param1) * 100;
        return result;
    }
    int32_t baseFunctionTest57(int8_t param1)
    {
        int32_t result = static_cast<int32_t>(param1) * 100;
        return result;
    }
    string getTestStr()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " " << testStr_ << std::endl;
        return testStr_;
    }
    ::ns_alltest::functiontest::ErrorResponse baseFunctionTest58(bool param1)
    {
        if (param1) {
            return {param1, 10000, "test58"};
        } else {
            return {false, 100, std::string("test581")};
        }
    }
};

class testInterfacePerformance1 {
public:
    void basePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }
    int8_t basePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + 5) / 10;
    }
};
class testInterfacePerformance2 {
public:
    void basePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }
    int8_t basePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + 5) / 10;
    }
};
class testInterfacePerformance3 {
public:
    void basePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }
    int8_t basePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + 5) / 10;
    }
};
class testInterfacePerformance4 {
public:
    void basePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }
    int8_t basePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + 5) / 10;
    }
};
class testInterfacePerformance5 {
public:
    void basePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }
    int8_t basePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + 5) / 10;
    }
};
class testInterfacePerformance6 {
public:
    void basePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }
    int8_t basePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + 5) / 10;
    }
};
class testInterfacePerformance7 {
public:
    void basePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }
    int8_t basePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + 5) / 10;
    }
};
class testInterfacePerformance8 {
public:
    void basePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }
    int8_t basePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + 5) / 10;
    }
};
class testInterfacePerformance9 {
public:
    void basePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }
    int8_t basePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + 5) / 10;
    }
};
class testInterfacePerformance10 {
public:
    void basePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }
    int8_t basePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + 5) / 10;
    }
};
class testInterfacePerformance11 {
public:
    void basePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }
    int8_t basePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + 5) / 10;
    }
};
class testInterfacePerformance12 {
public:
    void basePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }
    int8_t basePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + 5) / 10;
    }
};
class testInterfacePerformance13 {
public:
    void basePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }
    int8_t basePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + 5) / 10;
    }
};
class testInterfacePerformance14 {
public:
    void basePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }
    int8_t basePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + 5) / 10;
    }
};
class testInterfacePerformance15 {
public:
    void basePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }
    int8_t basePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + 5) / 10;
    }
};
class testInterfacePerformance16 {
public:
    void basePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }
    int8_t basePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + 5) / 10;
    }
};
class testInterfacePerformance17 {
public:
    void basePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }
    int8_t basePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + 5) / 10;
    }
};
class testInterfacePerformance18 {
public:
    void basePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }
    int8_t basePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + 5) / 10;
    }
};
class testInterfacePerformance19 {
public:
    void basePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }
    int8_t basePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + 5) / 10;
    }
};
class testInterfacePerformance20 {
public:
    void basePerformanceFunctionTest1()
    {
        std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
    }
    int8_t basePerformanceFunctionTest2(int8_t param1)
    {
        return (param1 + 5) / 10;
    }
};
void baseFunctionTest1()
{
    std::cout << "NameSpaceImpl: " << __func__ << " is true " << std::endl;
}
void baseFunctionTest2(int8_t param1)
{
    std::cout << "NameSpaceImpl: " << __func__ << " param1 " << (int)param1 << std::endl;
}
void baseFunctionTest3(int16_t param1)
{
    std::cout << "NameSpaceImpl: " << __func__ << " param1 " << param1 << std::endl;
}
void baseFunctionTest4(int32_t param1)
{
    std::cout << "NameSpaceImpl: " << __func__ << " param1 " << param1 << std::endl;
}
void baseFunctionTest5(int64_t param1)
{
    std::cout << "NameSpaceImpl: " << __func__ << " param1 " << param1 << std::endl;
}
void baseFunctionTest6(float param1)
{
    std::cout << "NameSpaceImpl: " << __func__ << " param1 " << param1 << std::endl;
}
void baseFunctionTest7(double param1)
{
    std::cout << "NameSpaceImpl: " << __func__ << " param1 " << param1 << std::endl;
}
void baseFunctionTest8(string_view param1)
{
    std::cout << "NameSpaceImpl: " << __func__ << " param1 " << param1 << std::endl;
}
void baseFunctionTest9(bool param1)
{
    std::cout << "NameSpaceImpl: " << __func__ << " param1 " << param1 << std::endl;
}
void baseFunctionTest10(array_view<int8_t> param1)
{
    // 输出 param1 的内容
    std::cout << "NameSpaceImpl: " << __func__ << " param1 ";
    for (int8_t value : param1) {
        std::cout << (int)value << " ";
    }
    std::cout << std::endl;
}
void baseFunctionTest11(array_view<int16_t> param1)
{
    // 输出 param1 的内容
    std::cout << "NameSpaceImpl: " << __func__ << " param1 ";
    for (int16_t value : param1) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
}
void baseFunctionTest12(optional_view<int16_t> param1)
{
    if (param1) {
        std::cout << "NameSpaceImpl: " << __func__ << " param1 " << *param1 << std::endl;
    } else {
        std::cout << "NameSpaceImpl: " << __func__ << " param1 is null" << std::endl;
    }
}
void baseFunctionTest13(optional_view<int64_t> param1)
{
    if (param1) {
        std::cout << "NameSpaceImpl: " << __func__ << " param1 " << *param1 << std::endl;
    } else {
        std::cout << "NameSpaceImpl: " << __func__ << " param1 is null" << std::endl;
    }
}
void baseFunctionTest14(array_view<uint8_t> param1)
{
    // 输出 param1 的内容
    std::cout << "NameSpaceImpl: " << __func__ << " param1 ";
    for (int16_t value : param1) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
}
void baseFunctionTest15(map_view<string, int32_t> param1)
{
    // std::cout << "NameSpaceImpl: " << __func__ << " param1 contents:" <<
    // std::endl;
    // // 遍历 map_view 并输出 key 和 value
    // for (const auto& pair : param1) {
    //     std::cout << "  Key: " << pair.first << ", Value: " << pair.second <<
    //     std::endl;
    // }
    std::cout << "NameSpaceImpl: " << __func__ << " param1 is null" << std::endl;
}
int8_t baseFunctionTest16(int8_t param1)
{
    // 检查结果是否超出 int8_t 的范围
    int32_t result = param1 + 10;
    if (result > INT8_MAX || result < INT8_MIN) {
        taihe::core::set_error("baseFunctionTest16: result exceeds int8_t range");
    }
    // 返回结果
    return static_cast<int8_t>(result);
}
int16_t baseFunctionTest17(int16_t param1)
{
    // 检查结果是否超出 int16_t 的范围
    int32_t result = param1 * 10;
    if (result > INT16_MAX || result < INT16_MIN) {
        taihe::core::set_error("baseFunctionTest17: result exceeds int16_t range");
    }
    // 返回结果
    return static_cast<int16_t>(result);
}
int32_t baseFunctionTest18(int32_t param1)
{
    // 检查结果是否超出 int32_t 的范围
    int64_t result = static_cast<int64_t>(param1) * 100;
    std::cout << "NameSpaceImpl: " << __func__ << " result " << result << std::endl;
    if (result > INT32_MAX || result < INT32_MIN) {
        taihe::core::set_error("baseFunctionTest18: result exceeds int32_t range");
    }
    // 返回结果
    return static_cast<int32_t>(result);
}
int64_t baseFunctionTest19(int64_t param1)
{
    // 检查结果是否超出 int64_t 的范围
    int64_t result = param1 * 10;
    std::cout << "NameSpaceImpl: " << __func__ << " result " << result << std::endl;
    if (result > INT64_MAX || result < INT64_MIN) {
        taihe::core::set_error("baseFunctionTest19: result exceeds int64_t range");
    }
    // 返回结果
    return static_cast<int64_t>(result);
}
float baseFunctionTest20(float param1)
{
    return param1 + 100;
}
double baseFunctionTest21(double param1)
{
    return param1 + 1.01;
}
string baseFunctionTest22(string_view param1)
{
    if (param1 == "baseFunctionTest22") {
        return std::string(param1) + "hello ani";
    } else {
        return param1;
    }
}
bool baseFunctionTest23(bool param1)
{
    if (param1) {
        return false;
    } else {
        return true;
    }
}
array<int8_t> baseFunctionTest24(array_view<int8_t> param1)
{
    array<int8_t> result = array<int8_t>::make(param1.size());
    for (int i = 0; i < param1.size(); i++) {
        result[i] = param1[i] * 2;
    }
    return result;
}
array<int16_t> baseFunctionTest25(array_view<int16_t> param1)
{
    array<int16_t> result = array<int16_t>::make(param1.size());
    for (int i = 0; i < param1.size(); i++) {
        result[i] = param1[i] + 2;
    }
    return result;
}
optional<int16_t> baseFunctionTest26(optional_view<int16_t> param1)
{
    if (param1) {
        // 检查结果是否超出 int16_t 的范围
        // int32_t result = param1+10;
        std::cout << "NameSpaceImpl: " << __func__ << " param1 " << *param1 << std::endl;
        int32_t result = static_cast<int32_t>(*param1) + 10;
        std::cout << "NameSpaceImpl: " << __func__ << " result " << result << std::endl;
        if (result > INT16_MAX || result < INT16_MIN) {
            taihe::core::set_error("baseFunctionTest26: result exceeds int16_t range");
        }
        // 返回结果
        return optional<int16_t>::make(result);
    } else {
        std::cout << "NameSpaceImpl: " << __func__ << " param1 is null" << std::endl;
        return optional<int16_t>(nullptr);
    }
}
optional<int64_t> baseFunctionTest27(optional_view<int64_t> param1)
{
    if (param1) {
        // 检查结果是否超出 int64_t 的范围
        std::cout << "NameSpaceImpl: " << __func__ << " param1 " << *param1 << std::endl;
        int64_t result = *param1 + 10;
        if (result > INT64_MAX || result < INT64_MIN) {
            taihe::core::set_error("baseFunctionTest27: result exceeds int16_t range");
        }
        // 返回结果
        return optional<int64_t>::make(result);
    } else {
        std::cout << "NameSpaceImpl: " << __func__ << " param1 is null" << std::endl;
        return optional<int64_t>(nullptr);
    }
}
array<uint8_t> baseFunctionTest28(array_view<uint8_t> param1)
{
    array<uint8_t> result = array<uint8_t>::make(param1.size());
    for (int i = 0; i < param1.size(); i++) {
        result[i] = param1[i] * 10;
    }
    return result;
}
map<string, int32_t> baseFunctionTest29(map_view<string, int32_t> param1)
{
    map<string, int32_t> m;
    for (int i = 0; i < param1.size(); ++i) {
        m.emplace("test" + std::to_string(i), 10 + i);
    }
    return m;
}

::ns_alltest::functiontest::Color baseFunctionTest30(::ns_alltest::functiontest::Color param1)
{
    return (::ns_alltest::functiontest::Color::key_t)(((int)param1.get_key() + 1) % 3);
}

void baseFunctionTest31(::ns_alltest::functiontest::Color param1)
{
    std::cout << "NameSpaceImpl: " << __func__ << " param1 " << param1 << std::endl;
}

int16_t baseFunctionTest32(int8_t param1, int16_t param2, int32_t param3, int64_t param4, float param5, double param6,
                           string_view param7, bool param8, array_view<int16_t> param9, array_view<int64_t> param10)
{
    if (param8) {
        return param2;
    } else {
        return static_cast<int16_t>(param1);
    }
}
int32_t baseFunctionTest33(int8_t param1, int16_t param2, int32_t param3, int64_t param4, float param5, double param6,
                           string_view param7, bool param8, array_view<int8_t> param9, array_view<int32_t> param10)
{
    if (param8) {
        return param3;
    } else {
        return static_cast<int32_t>(param2);
    }
}
int64_t baseFunctionTest34(int8_t param1, int16_t param2, int32_t param3, int64_t param4, float param5, double param6,
                           string_view param7, bool param8, array_view<string> param9, array_view<int64_t> param10)
{
    if (param8) {
        return param4;
    } else {
        return static_cast<int64_t>(param2);
    }
}
int8_t baseFunctionTest35(int8_t param1, int16_t param2, int32_t param3, int64_t param4, float param5, double param6,
                          string_view param7, bool param8, array_view<bool> param9, array_view<int64_t> param10)
{
    if (param8) {
        return static_cast<int8_t>(param1);
    } else {
        return static_cast<int8_t>(param2);
    }
}
array<int32_t> baseFunctionTest36(int8_t param1, int16_t param2, int32_t param3, int64_t param4, float param5,
                                  double param6, string_view param7, bool param8, array_view<int8_t> param9,
                                  array_view<int64_t> param10)
{
    if (param8) {
        array<int32_t> result = array<int32_t>::make(param9.size());
        for (int i = 0; i < param9.size(); i++) {
            result[i] = param9[i] + param1 + param2 + param3;
        }
        return result;
    } else {
        array<int32_t> result = array<int32_t>::make(param10.size());
        for (int i = 0; i < param10.size(); i++) {
            result[i] = static_cast<int32_t>(param10[i]);
        }
        return result;
    }
}
array<int64_t> baseFunctionTest37(int8_t param1, int16_t param2, int32_t param3, int64_t param4, float param5,
                                  double param6, string_view param7, bool param8, array_view<int32_t> param9,
                                  array_view<int64_t> param10)
{
    if (param8) {
        array<int64_t> result = array<int64_t>::make(param10.size());
        for (int i = 0; i < param10.size(); i++) {
            result[i] = param10[i] + param1 + param2 + param3;
        }
        return result;
    } else {
        array<int64_t> result = array<int64_t>::make(param10.size());
        for (int i = 0; i < param10.size(); i++) {
            result[i] = param10[i] * 10;
        }
        return result;
    }
}
string baseFunctionTest38(int8_t param1, int16_t param2, int32_t param3, int64_t param4, float param5, double param6,
                          string_view param7, bool param8, array_view<int8_t> param9, array_view<int16_t> param10)
{
    if (param8) {
        return std::string(param7) + std::to_string(param1) + std::to_string(param2) + std::to_string(param3) +
               std::to_string(param4);
    } else {
        return std::string(param7) + std::to_string(param9[0]) + std::to_string(param10[0]);
    }
}
bool baseFunctionTest39(int8_t param1, int16_t param2, int32_t param3, int64_t param4, float param5, double param6,
                        string_view param7, bool param8, array_view<bool> param9, array_view<int32_t> param10)
{
    if (param2 == 100) {
        return param8;
    } else {
        return false;
    }
}
array<int8_t> baseFunctionTest40(int8_t param1, int16_t param2, int32_t param3, int64_t param4, float param5,
                                 double param6, string_view param7, bool param8, array_view<int8_t> param9,
                                 array_view<uint8_t> param10)
{
    if (param8) {
        array<int8_t> result = array<int8_t>::make(param9.size());
        for (int i = 0; i < param9.size(); i++) {
            result[i] = param9[i] * 2;
        }
        return result;
    } else {
        array<int8_t> result = array<int8_t>::make(param9.size());
        for (int i = 0; i < param9.size(); i++) {
            result[i] = param9[i] + 10;
        }
        return result;
    }
}
void baseFunctionTest41(int8_t param1, int16_t param2, int32_t param3, int64_t param4, float param5, double param6,
                        string_view param7, bool param8, array_view<int16_t> param9, array_view<int64_t> param10)
{
    std::cout << "NameSpaceImpl: " << __func__ << " param1 " << param1 << " param2 " << param2 << " param3 " << param3
              << " param4 " << param4 << std::endl;
}
int32_t baseFunctionTest42_int(int32_t param1, int16_t param2)
{
    std::cout << "NameSpaceImpl: " << __func__ << " param1 " << param1 << " param2 " << param2 << std::endl;
    int64_t result = static_cast<int64_t>(param1) + static_cast<int64_t>(param2);
    std::cout << "NameSpaceImpl: " << __func__ << " result " << result << std::endl;
    if (result > INT32_MAX || result < INT32_MIN) {
        taihe::core::set_error("baseFunctionTest42_int: result exceeds int32_t range");
    }
    // 返回结果
    return static_cast<int32_t>(result);
}
int32_t baseFunctionTest42_container(optional_view<int32_t> param1, map_view<string, int32_t> param2)
{
    if (param1) {
        return *param1 + 10;
    } else {
        std::cout << "NameSpaceImpl: " << __func__ << " param1 is null" << std::endl;
        return INT32_MIN;
    }
}
int32_t baseFunctionTest42_void()
{
    return INT32_MAX;
}

void baseFunctionTest42_struct(::ns_alltest::functiontest::Data const &param1)
{
    std::cout << "NameSpaceImpl: " << __func__ << "data1: " << param1.data1 << std::endl;
    std::cout << "NameSpaceImpl: " << __func__ << "data2: " << param1.data2 << std::endl;
    std::cout << "NameSpaceImpl: " << __func__ << "data3: " << param1.data3 << std::endl;
}

::ns_alltest::functiontest::testNameSpace get_interface_NameSpace()
{
    return make_holder<testNameSpace, ::ns_alltest::functiontest::testNameSpace>();
}

::ns_alltest::functiontest::testInterfacePerformance1 get_interface_performance1()
{
    return make_holder<testInterfacePerformance1, ::ns_alltest::functiontest::testInterfacePerformance1>();
}
::ns_alltest::functiontest::testInterfacePerformance2 get_interface_performance2()
{
    return make_holder<testInterfacePerformance2, ::ns_alltest::functiontest::testInterfacePerformance2>();
}
::ns_alltest::functiontest::testInterfacePerformance3 get_interface_performance3()
{
    return make_holder<testInterfacePerformance3, ::ns_alltest::functiontest::testInterfacePerformance3>();
}
::ns_alltest::functiontest::testInterfacePerformance4 get_interface_performance4()
{
    return make_holder<testInterfacePerformance4, ::ns_alltest::functiontest::testInterfacePerformance4>();
}
::ns_alltest::functiontest::testInterfacePerformance5 get_interface_performance5()
{
    return make_holder<testInterfacePerformance5, ::ns_alltest::functiontest::testInterfacePerformance5>();
}
::ns_alltest::functiontest::testInterfacePerformance6 get_interface_performance6()
{
    return make_holder<testInterfacePerformance6, ::ns_alltest::functiontest::testInterfacePerformance6>();
}
::ns_alltest::functiontest::testInterfacePerformance7 get_interface_performance7()
{
    return make_holder<testInterfacePerformance7, ::ns_alltest::functiontest::testInterfacePerformance7>();
}
::ns_alltest::functiontest::testInterfacePerformance8 get_interface_performance8()
{
    return make_holder<testInterfacePerformance8, ::ns_alltest::functiontest::testInterfacePerformance8>();
}
::ns_alltest::functiontest::testInterfacePerformance9 get_interface_performance9()
{
    return make_holder<testInterfacePerformance9, ::ns_alltest::functiontest::testInterfacePerformance9>();
}
::ns_alltest::functiontest::testInterfacePerformance10 get_interface_performance10()
{
    return make_holder<testInterfacePerformance10, ::ns_alltest::functiontest::testInterfacePerformance10>();
}
::ns_alltest::functiontest::testInterfacePerformance11 get_interface_performance11()
{
    return make_holder<testInterfacePerformance11, ::ns_alltest::functiontest::testInterfacePerformance11>();
}
::ns_alltest::functiontest::testInterfacePerformance12 get_interface_performance12()
{
    return make_holder<testInterfacePerformance12, ::ns_alltest::functiontest::testInterfacePerformance12>();
}
::ns_alltest::functiontest::testInterfacePerformance13 get_interface_performance13()
{
    return make_holder<testInterfacePerformance13, ::ns_alltest::functiontest::testInterfacePerformance13>();
}
::ns_alltest::functiontest::testInterfacePerformance14 get_interface_performance14()
{
    return make_holder<testInterfacePerformance14, ::ns_alltest::functiontest::testInterfacePerformance14>();
}
::ns_alltest::functiontest::testInterfacePerformance15 get_interface_performance15()
{
    return make_holder<testInterfacePerformance15, ::ns_alltest::functiontest::testInterfacePerformance15>();
}
::ns_alltest::functiontest::testInterfacePerformance16 get_interface_performance16()
{
    return make_holder<testInterfacePerformance16, ::ns_alltest::functiontest::testInterfacePerformance16>();
}
::ns_alltest::functiontest::testInterfacePerformance17 get_interface_performance17()
{
    return make_holder<testInterfacePerformance17, ::ns_alltest::functiontest::testInterfacePerformance17>();
}
::ns_alltest::functiontest::testInterfacePerformance18 get_interface_performance18()
{
    return make_holder<testInterfacePerformance18, ::ns_alltest::functiontest::testInterfacePerformance18>();
}
::ns_alltest::functiontest::testInterfacePerformance19 get_interface_performance19()
{
    return make_holder<testInterfacePerformance19, ::ns_alltest::functiontest::testInterfacePerformance19>();
}
::ns_alltest::functiontest::testInterfacePerformance20 get_interface_performance20()
{
    return make_holder<testInterfacePerformance20, ::ns_alltest::functiontest::testInterfacePerformance20>();
}

}  // namespace

TH_EXPORT_CPP_API_baseFunctionTest1(baseFunctionTest1);
TH_EXPORT_CPP_API_baseFunctionTest2(baseFunctionTest2);
TH_EXPORT_CPP_API_baseFunctionTest3(baseFunctionTest3);
TH_EXPORT_CPP_API_baseFunctionTest4(baseFunctionTest4);
TH_EXPORT_CPP_API_baseFunctionTest5(baseFunctionTest5);
TH_EXPORT_CPP_API_baseFunctionTest6(baseFunctionTest6);
TH_EXPORT_CPP_API_baseFunctionTest7(baseFunctionTest7);
TH_EXPORT_CPP_API_baseFunctionTest8(baseFunctionTest8);
TH_EXPORT_CPP_API_baseFunctionTest9(baseFunctionTest9);
TH_EXPORT_CPP_API_baseFunctionTest10(baseFunctionTest10);
TH_EXPORT_CPP_API_baseFunctionTest11(baseFunctionTest11);
TH_EXPORT_CPP_API_baseFunctionTest12(baseFunctionTest12);
TH_EXPORT_CPP_API_baseFunctionTest13(baseFunctionTest13);
TH_EXPORT_CPP_API_baseFunctionTest14(baseFunctionTest14);
TH_EXPORT_CPP_API_baseFunctionTest15(baseFunctionTest15);
TH_EXPORT_CPP_API_baseFunctionTest16(baseFunctionTest16);
TH_EXPORT_CPP_API_baseFunctionTest17(baseFunctionTest17);
TH_EXPORT_CPP_API_baseFunctionTest18(baseFunctionTest18);
TH_EXPORT_CPP_API_baseFunctionTest19(baseFunctionTest19);
TH_EXPORT_CPP_API_baseFunctionTest20(baseFunctionTest20);
TH_EXPORT_CPP_API_baseFunctionTest21(baseFunctionTest21);
TH_EXPORT_CPP_API_baseFunctionTest22(baseFunctionTest22);
TH_EXPORT_CPP_API_baseFunctionTest23(baseFunctionTest23);
TH_EXPORT_CPP_API_baseFunctionTest24(baseFunctionTest24);
TH_EXPORT_CPP_API_baseFunctionTest25(baseFunctionTest25);
TH_EXPORT_CPP_API_baseFunctionTest26(baseFunctionTest26);
TH_EXPORT_CPP_API_baseFunctionTest27(baseFunctionTest27);
TH_EXPORT_CPP_API_baseFunctionTest28(baseFunctionTest28);
TH_EXPORT_CPP_API_baseFunctionTest29(baseFunctionTest29);
TH_EXPORT_CPP_API_baseFunctionTest30(baseFunctionTest30);
TH_EXPORT_CPP_API_baseFunctionTest31(baseFunctionTest31);
TH_EXPORT_CPP_API_baseFunctionTest32(baseFunctionTest32);
TH_EXPORT_CPP_API_baseFunctionTest33(baseFunctionTest33);
TH_EXPORT_CPP_API_baseFunctionTest34(baseFunctionTest34);
TH_EXPORT_CPP_API_baseFunctionTest35(baseFunctionTest35);
TH_EXPORT_CPP_API_baseFunctionTest36(baseFunctionTest36);
TH_EXPORT_CPP_API_baseFunctionTest37(baseFunctionTest37);
TH_EXPORT_CPP_API_baseFunctionTest38(baseFunctionTest38);
TH_EXPORT_CPP_API_baseFunctionTest39(baseFunctionTest39);
TH_EXPORT_CPP_API_baseFunctionTest40(baseFunctionTest40);
TH_EXPORT_CPP_API_baseFunctionTest41(baseFunctionTest41);
TH_EXPORT_CPP_API_baseFunctionTest42_int(baseFunctionTest42_int);
TH_EXPORT_CPP_API_baseFunctionTest42_container(baseFunctionTest42_container);
TH_EXPORT_CPP_API_baseFunctionTest42_void(baseFunctionTest42_void);
TH_EXPORT_CPP_API_baseFunctionTest42_struct(baseFunctionTest42_struct);
TH_EXPORT_CPP_API_get_interface_NameSpace(get_interface_NameSpace);
TH_EXPORT_CPP_API_get_interface_performance1(get_interface_performance1);
TH_EXPORT_CPP_API_get_interface_performance2(get_interface_performance2);
TH_EXPORT_CPP_API_get_interface_performance3(get_interface_performance3);
TH_EXPORT_CPP_API_get_interface_performance4(get_interface_performance4);
TH_EXPORT_CPP_API_get_interface_performance5(get_interface_performance5);
TH_EXPORT_CPP_API_get_interface_performance6(get_interface_performance6);
TH_EXPORT_CPP_API_get_interface_performance7(get_interface_performance7);
TH_EXPORT_CPP_API_get_interface_performance8(get_interface_performance8);
TH_EXPORT_CPP_API_get_interface_performance9(get_interface_performance9);
TH_EXPORT_CPP_API_get_interface_performance10(get_interface_performance10);
TH_EXPORT_CPP_API_get_interface_performance11(get_interface_performance11);
TH_EXPORT_CPP_API_get_interface_performance12(get_interface_performance12);
TH_EXPORT_CPP_API_get_interface_performance13(get_interface_performance13);
TH_EXPORT_CPP_API_get_interface_performance14(get_interface_performance14);
TH_EXPORT_CPP_API_get_interface_performance15(get_interface_performance15);
TH_EXPORT_CPP_API_get_interface_performance16(get_interface_performance16);
TH_EXPORT_CPP_API_get_interface_performance17(get_interface_performance17);
TH_EXPORT_CPP_API_get_interface_performance18(get_interface_performance18);
TH_EXPORT_CPP_API_get_interface_performance19(get_interface_performance19);
TH_EXPORT_CPP_API_get_interface_performance20(get_interface_performance20);
