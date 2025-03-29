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
#include "primitives_test.impl.hpp"

#include <iomanip>
#include <iostream>

#include "core/optional.hpp"
#include "core/runtime.hpp"
#include "core/string.hpp"
#include "primitives_test.primitivesVoid.proj.2.hpp"
#include "stdexcept"
// Please delete this include when you implement
using namespace taihe::core;
namespace {

class primitivesVoid {
public:
    void testBaseFunc1()
    {
        std::cout << "testBaseFunc1 is true " << std::endl;
    }
    void testBaseFunc2(int32_t option1, bool option2)
    {
        if (option2) {
            std::cout << "testBaseFunc2 is option1  " << option1 << std::endl;
            std::cout << "testBaseFunc2 is option2  " << option2 << std::endl;
        } else {
            std::cout << "testBaseFunc2 is option1  " << option1 << std::endl;
            std::cout << "testBaseFunc2 is option2  " << option2 << std::endl;
        }
    }
    void testBaseFunc3(int32_t option1, int64_t option2)
    {
        std::cout << "testBaseFunc3 is option1  " << option1 << std::endl;
        std::cout << "testBaseFunc3 is option2  " << option2 << std::endl;
    }
    void testBaseFunc4(int32_t option1, string_view option2)
    {
        std::cout << "testBaseFunc4 is option1  " << option1 << std::endl;
        std::cout << "testBaseFunc4 is option2  " << option2 << std::endl;
    }
    void testBaseFunc5(int64_t option1, bool option2)
    {
        if (option2) {
            std::cout << "testBaseFunc5 is option1  " << option1 << std::endl;
            std::cout << "testBaseFunc5 is option2  " << option2 << std::endl;
        } else {
            std::cout << "testBaseFunc5 is option1  " << option1 << std::endl;
            std::cout << "testBaseFunc5 is option2  " << option2 << std::endl;
        }
    }
    void testBaseFunc6(int64_t option1, float option2)
    {
        std::cout << "testBaseFunc6 is option1  " << option1 << std::endl;
        std::cout << "testBaseFunc6 is option2  " << option2 << std::endl;
    }
    void testBaseFunc7(int64_t option1, double option2)
    {
        std::cout << "testBaseFunc7 is option1  " << option1 << std::endl;
        std::cout << "testBaseFunc7 is option2  " << option2 << std::endl;
    }
    void testBaseFunc8(float option1, bool option2)
    {
        if (option2) {
            std::cout << "testBaseFunc8 is option1  " << option1 << std::endl;
            std::cout << "testBaseFunc8 is option2  " << option2 << std::endl;
        } else {
            std::cout << "testBaseFunc8 is option1  " << option1 << std::endl;
            std::cout << "testBaseFunc8 is option2  " << option2 << std::endl;
        }
    }
    void testBaseFunc9(float option1, string_view option2)
    {
        std::cout << "testBaseFunc9 is option1  " << option1 << std::endl;
        std::cout << "testBaseFunc9 is option2  " << option2 << std::endl;
    }
    void testBaseFunc10(double option1, string_view option2)
    {
        std::cout << "testBaseFunc10 is option1  " << option1 << std::endl;
        std::cout << "testBaseFunc10 is option2  " << option2 << std::endl;
    }
    void testBaseFunc11(double option1, bool option2)
    {
        if (option1) {
            std::cout << "testBaseFunc11 is option1  " << option1 << std::endl;
            std::cout << "testBaseFunc11 is option2  " << option2 << std::endl;
        } else {
            std::cout << "testBaseFunc11 is option1  " << option1 << std::endl;
            std::cout << "testBaseFunc11 is option2  " << option2 << std::endl;
        }
    }
    void testBaseFunc12(optional_view<int32_t> option1, optional_view<int64_t> option2)
    {
        if (option1) {
            std::cout << *option1 << std::endl;
        } else if (option2) {
            std::cout << *option2 << std::endl;
        } else {
            std::cout << "Null" << std::endl;
        }
    }
    void testBaseFunc13(optional_view<float> option1, optional_view<double> option2)
    {
        if (option1) {
            std::cout << *option1 << std::endl;
        } else if (option2) {
            std::cout << *option2 << std::endl;
        } else {
            std::cout << "Null" << std::endl;
        }
    }
    void testBaseFunc14(optional_view<string> option1, optional_view<bool> option2)
    {
        if (option1) {
            std::cout << *option1 << std::endl;
        } else if (option2) {
            std::cout << *option2 << std::endl;
        } else {
            std::cout << "Null" << std::endl;
        }
    }
    void testBaseFunc15(optional_view<int16_t> option1, optional_view<int64_t> option2)
    {
        if (option1) {
            std::cout << *option1 << std::endl;
        } else if (option2) {
            std::cout << *option2 << std::endl;
        } else {
            std::cout << "Null" << std::endl;
        }
    }
    void testBaseFunc16(int8_t option1, int16_t option2)
    {
        std::cout << "testBaseFunc16 is option1  " << (int)option1 << std::endl;
        std::cout << "testBaseFunc16 is option2  " << (int)option2 << std::endl;
    }

    void testBaseFunc17(array_view<int32_t> option1, array_view<int8_t> option2)
    {
        // 输出 option1 的内容
        std::cout << "testBaseFunc17 option1: ";
        for (int32_t value : option1) {
            std::cout << value << " ";
        }
        std::cout << std::endl;

        // 输出 option2 的内容
        std::cout << "testBaseFunc17 option2: ";
        for (int8_t value : option2) {
            std::cout << (int)value << " ";
        }
        std::cout << std::endl;
    }
    void testBaseFunc18(array_view<int16_t> option1, array_view<int64_t> option2)
    {
        // 输出 option1 的内容
        std::cout << "testBaseFunc18 option1: ";
        for (int16_t value : option1) {
            std::cout << value << " ";
        }
        std::cout << std::endl;

        // 输出 option2 的内容
        std::cout << "testBaseFunc18 option2: ";
        for (int64_t value : option2) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }
    void testBaseFunc19(array_view<float> option1, array_view<double> option2)
    {
        // 输出 option1 的内容
        std::cout << "testBaseFunc19 option1: ";
        // std::cout << std::fixed << std::setprecision(6); // float 保留 6 位小数
        for (float value : option1) {
            std::cout << value << " ";
        }
        std::cout << std::endl;

        // 输出 option2 的内容
        std::cout << "testBaseFunc19 option2: ";
        // std::cout << std::fixed << std::setprecision(15); // double 保留 15
        // 位小数
        for (double value : option2) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }
    void testBaseFunc20(array_view<bool> option1, array_view<string> option2)
    {
        // 输出 option1 的内容
        std::cout << "testBaseFunc20 option1: ";
        for (bool value : option1) {
            std::cout << value << " ";
        }
        std::cout << std::endl;

        // 输出 option2 的内容
        std::cout << "testBaseFunc20 option2: ";
        for (string value : option2) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }
};

class primitivesBoolean {
public:
    void testBaseBoolFunc1(bool option1)
    {
        if (option1) {
            std::cout << "testBaseBoolFunc1 is true " << option1 << std::endl;
        } else {
            std::cout << "testBaseBoolFunc1 is false  " << option1 << std::endl;
        }
    }
    int32_t testBaseBoolFunc2(bool option1)
    {
        if (option1) {
            return 65535;
        } else {
            return 0;
        }
    }
    bool testBaseBoolFunc3(bool option1)
    {
        if (option1) {
            return false;
        } else {
            return true;
        }
    }
    bool testBaseBoolFunc4(optional_view<bool> option1)
    {
        if (option1) {
            return false;
        } else {
            return true;
        }
    }
    bool testBaseBoolFunc5(bool option1)
    {
        if (option1) {
            return true;
        } else {
            return false;
        }
    }
};
class primitivesInteger {
public:
    int8_t testBaseIntegerFunc1(int8_t option1)
    {
        if (option1 == -1) {
            taihe::core::set_error("out of range The i8 maximum value is 127 and minnum values is -128");
            return -1;
        }
        return option1 + 1;
    }
    int8_t testBaseIntegerFunc2(int8_t option1, int16_t option2)
    {
        if (option1 == -1) {
            taihe::core::set_error("out of range The i8 maximum value is 127 and minnum values is -128");
            return -1;
        }
        if (option2 == -1) {
            taihe::core::set_error(
                "out of range The i16 maximum value is 32767 and minnum values is "
                "-32768");
            return -1;
        }
        return option1 + option2;
    }
    void testBaseIntegerFunc3(int8_t option1, int16_t option2)
    {
        std::cout << "testBaseIntegerFunc3 is option1  " << (int)option1 << std::endl;
        std::cout << "testBaseIntegerFunc3 is option2  " << option2 << std::endl;
    }
    int16_t testBaseIntegerFunc4(int8_t option1, int16_t option2)
    {
        return option1 + option2;
    }
    int8_t testBaseIntegerFunc5(int8_t option1, int32_t option2)
    {
        return option1 + option2;
    }
    int32_t testBaseIntegerFunc6(int8_t option1, int32_t option2)
    {
        return option1 + option2;
    }
    void testBaseIntegerFunc7(int8_t option1, int32_t option2)
    {
        if (option2 == -1) {
            taihe::core::set_error(
                "out of range The i32 maximum value is 2147483647 and minnum values "
                "is -2147483648");
        }
        std::cout << "testBaseIntegerFunc7 is option1  " << (int)option1 << std::endl;
        std::cout << "testBaseIntegerFunc7 is option2  " << option2 << std::endl;
    }
    int64_t testBaseIntegerFunc8(int8_t option1, int64_t option2)
    {
        if (option2 == -1) {
            taihe::core::set_error(
                "out of range The i64 maximum value is 9223372036854775807 and "
                "minnum values is -9223372036854775808");
            return -1;
        }
        return option2 - option1;
    }
    int8_t testBaseIntegerFunc9(int8_t option1, int64_t option2)
    {
        if (option1 > 127 || option1 < -128) {
            taihe::core::set_error("out of range The i8 maximum value is 127 and minnum values is -128");
            return -1;
        }
        if (option2 > INT64_MAX || option2 < INT64_MIN) {
            taihe::core::set_error(
                "out of range The i64 maximum value is 9223372036854775807 and "
                "minnum values is -9223372036854775808");
            return -1;
        }
        return option2 - option1;
    }
    float testBaseIntegerFunc10(int8_t option1, float option2)
    {
        return option1 + option2;
    }

    int8_t testBaseIntegerFunc11(int8_t option1, float option2)
    {
        return option1 + option2;
    }
    double testBaseIntegerFunc12(int8_t option1, double option2)
    {
        return option1 + option2;
    }
    int8_t testBaseIntegerFunc13(int8_t option1, int64_t option2)
    {
        return option1 + option2;
    }
    string testBaseIntegerFunc14(int8_t option1, string_view option2)
    {
        if (option2 == "testBaseIntegerFunc14") {
            return std::string(option2) + std::to_string(option1);
        } else {
            return std::string(option2);
        }
    }
    int8_t testBaseIntegerFunc15(int8_t option1, string_view option2)
    {
        if (option2 == "testBaseIntegerFunc15") {
            return option1 + 10;
        } else {
            return option1;
        }
    }
    bool testBaseIntegerFunc16(int8_t option1, bool option2)
    {
        if (option2) {
            return true;
        } else {
            return false;
        }
    }
    int8_t testBaseIntegerFunc17(int8_t option1, bool option2)
    {
        if (option2) {
            return option1 + 1;
        } else {
            return option1;
        }
    }

    int16_t testBaseIntegerFunc18(int16_t option1)
    {
        // 检查结果是否超出 int16_t 的范围
        int32_t result = static_cast<int32_t>(option1) * 10;  // 使用 int32_t 避免溢出
        if (result > INT16_MAX || result < INT16_MIN) {
            taihe::core::set_error("testBaseIntegerFunc18: result exceeds int16_t range");
        }
        // 返回结果
        return static_cast<int16_t>(result);
    }

    void testBaseIntegerFunc19(int16_t option1)
    {
        std::cout << "testBaseIntegerFunc19 is option1  " << option1 << std::endl;
    }
    int16_t testBaseIntegerFunc20(int16_t option1, int32_t option2)
    {
        // 检查结果是否超出 int16_t 的范围
        int32_t result = static_cast<int32_t>(option1) + option2;  // 使用 int32_t 避免溢出
        if (result > INT16_MAX || result < INT16_MIN) {
            taihe::core::set_error("testBaseIntegerFunc20: result exceeds int16_t range");
        }
        // 返回结果
        return static_cast<int16_t>(result);
    }
    int16_t testBaseIntegerFunc21(int16_t option1, int64_t option2)
    {
        // 检查结果是否超出 int16_t 的范围
        int64_t result = static_cast<int64_t>(option1) + option2;
        if (result > INT16_MAX || result < INT16_MIN) {
            taihe::core::set_error("testBaseIntegerFunc21: result exceeds int16_t range");
        }
        // 返回结果
        return static_cast<int16_t>(result);
    }
    int32_t testBaseIntegerFunc22(int32_t option1)
    {
        // 检查结果是否超出 int32_t 的范围
        int64_t result = static_cast<int32_t>(option1) * 100;
        if (result > INT32_MAX || result < INT32_MIN) {
            taihe::core::set_error("testBaseIntegerFunc22: result exceeds int32_t range");
        }
        // 返回结果
        return static_cast<int32_t>(result);
    }
    void testBaseIntegerFunc23(int32_t option1)
    {
        if (option1 > INT32_MAX || option1 < INT32_MIN) {
            taihe::core::set_error("testBaseIntegerFunc23: result exceeds int32_t range");
        }
        std::cout << "testBaseIntegerFunc23 is option1  " << option1 << std::endl;
    }
    int32_t testBaseIntegerFunc24(int32_t option1, int64_t option2)
    {
        // 检查结果是否超出 int32_t 的范围
        int64_t result = static_cast<int64_t>(option1) + option2;
        if (result > INT32_MAX || result < INT32_MIN) {
            taihe::core::set_error("testBaseIntegerFunc24: result exceeds int32_t range");
        }
        // 返回结果
        return static_cast<int32_t>(result);
    }
    int32_t testBaseIntegerFunc25(int32_t option1, int8_t option2)
    {
        // 检查结果是否超出 int32_t 的范围
        int32_t result = static_cast<int32_t>(option2) + option1;
        if (result > INT32_MAX || result < INT32_MIN) {
            taihe::core::set_error("testBaseIntegerFunc25: result exceeds int32_t range");
        }
        // 返回结果
        return static_cast<int32_t>(result);
    }
    int64_t testBaseIntegerFunc26(int64_t option1)
    {
        // 检查结果是否超出 int32_t 的范围
        int64_t result = option1 * 100;
        if (result > INT64_MAX || result < INT64_MIN) {
            taihe::core::set_error("testBaseIntegerFunc25: result exceeds int64_t range");
        }
        // 返回结果
        return static_cast<int64_t>(result);
    }
    void testBaseIntegerFunc27(int64_t option1)
    {
        std::cout << "testBaseIntegerFunc27 is option1  " << option1 << std::endl;
    }
    string testBaseIntegerFunc28(int64_t option1, string_view option2)
    {
        if (option2 == "testBaseIntegerFunc28") {
            return std::string(option2) + std::to_string(option1);
        } else {
            return std::string(option2);
        }
    }
    int64_t testBaseIntegerFunc29(int64_t option1, string_view option2)
    {
        if (option2 == "testBaseIntegerFunc29") {
            int64_t result = option1 * 10;
            if (result > INT64_MAX || result < INT64_MIN) {
                taihe::core::set_error("testBaseIntegerFunc29: result exceeds int32_t range");
            }
            // 返回结果
            return static_cast<int64_t>(result);
        } else {
            return option1;
        }
    }
    float testBaseIntegerFunc30(float option1)
    {
        return option1 + 1.0;
    }
    void testBaseIntegerFunc31(float option1)
    {
        std::cout << std::fixed << std::setprecision(6);  // float 保留 6 位小数
        std::cout << "testBaseIntegerFunc31 is option1  " << option1 << std::endl;
    }
    float testBaseIntegerFunc32(float option1, double option2)
    {
        return option1 + option2;
    }
    double testBaseIntegerFunc33(float option1, double option2)
    {
        double result = static_cast<double>(option1) + option2;
        // 打印调试信息，保留 6 位小数
        std::cout << std::fixed << std::setprecision(6);
        std::cout << "Debug: option1 = " << option1 << ", option2 = " << option2 << ", result = " << result
                  << std::endl;
        return result;
    }
    double testBaseIntegerFunc34(double option1)
    {
        return option1 + 1;
    }
    void testBaseIntegerFunc35(double option1)
    {
        std::cout << std::fixed << std::setprecision(15);  // float 保留 6 位小数
        std::cout << "testBaseIntegerFunc35 is option1  " << option1 << std::endl;
    }
};

static void parseOption(::primitives_test::Foo const &opt)
{
    std::cout << "num: " << opt.testNum << std::endl;
    std::cout << "str: " << opt.testStr << std::endl;
}

int32_t multiply(int32_t a, int32_t b)
{
    return a * b;
}

bool baseCFunc(int32_t testBoolean)
{
    if (testBoolean == 10) {
        return true;
    } else {
        return false;
    }
}

void baseAFunc(bool testBoolean)
{
    if (testBoolean) {
        std::cout << "testBoolean is true " << testBoolean << std::endl;
    } else {
        std::cout << "testBoolean is false  " << testBoolean << std::endl;
    }
}
bool baseBFunc(bool testBoolean)
{
    if (testBoolean) {
        return false;
    } else {
        return true;
    }
}
bool baseDFunc(string_view testBoolean)
{
    if (testBoolean == "test123") {
        return true;
    } else {
        return false;
    }
}

string baseEFunc(::primitives_test::Foo const &b)
{
    parseOption(b);
    return "success";
}
string baseHFunc(int32_t a, int64_t b)
{
    int64_t sum = a + b;
    return std::to_string(sum);
}
string baseIFunc(double a, float b)
{
    double result = static_cast<double>(b) + a;
    std::cout << "baseIFunc is true " << result << std::endl;
    return std::to_string(result);
}

float baseFunc1(float b)
{
    return b + 1.0;
}

void baseFunc2(float b)
{
    if (b == 1.2) {
        std::cout << "baseFunc2 is true " << b << std::endl;
    } else {
        std::cout << "baseFunc2 is false  " << b << std::endl;
    }
}

double baseFunc3(float a, double b)
{
    return static_cast<double>(a) + b;
}

double baseFunc4(double b)
{
    return b + 1.0;
}

void baseFunc5(double b)
{
    if (b == 3.14159) {
        std::cout << "baseFunc5 is true " << b << std::endl;
    } else {
        std::cout << "baseFunc5 is false  " << b << std::endl;
    }
}

void baseFunc6(string_view a)
{
    if (a == "testbaseFunc6") {
        std::cout << "baseFunc6 is true " << a << std::endl;
    } else {
        std::cout << "baseFunc6 is false  " << a << std::endl;
    }
}
string baseFunc7(string_view a)
{
    if (a == "testbaseFunc7") {
        return std::string(a);  // 返回 a
    } else {
        return std::string(a) + "false";  // 返回 a + "false";
    }
}
string baseFunc8(string_view a, int32_t b)
{
    if (a == "testbaseFunc8") {
        return std::string(a) + std::to_string(b);  // 返回 a + b（b 转换为字符串）
    } else {
        return std::string(a);  // 返回 a
    }
}
void baseFunc9(string_view a, int32_t b, int64_t c, bool d, float e)
{
    if (a == "testbaseFunc9") {
        std::cout << "str: " << a << std::endl;
    } else if (b == 32) {
        std::cout << "int32: " << b << std::endl;
    } else if (c == 9223372036854775807) {
        std::cout << "int64: " << c << std::endl;
    } else if (d) {
        std::cout << "boolean: " << d << std::endl;
    } else if (e == 3.1) {
        std::cout << "testFloat: " << e << std::endl;
    } else {
        std::cout << "testError: " << std::endl;
    }
}

void baseFunc10()
{
    std::cout << "baseFunc10 is true " << std::endl;
}

void baseFunc11(int32_t a, bool b)
{
    if (b) {
        std::cout << "baseFunc11 is a  " << a << std::endl;
        std::cout << "baseFunc11 is b  " << b << std::endl;
    } else {
        std::cout << "baseFunc11 is a  " << a << std::endl;
        std::cout << "baseFunc11 is b  " << b << std::endl;
    }
}
void baseFunc12(int32_t a, int64_t b)
{
    std::cout << "baseFunc12 is a  " << a << std::endl;
    std::cout << "baseFunc12 is b  " << b << std::endl;
}
void baseFunc13(int32_t a, string_view b)
{
    std::cout << "baseFunc13 is a  " << a << std::endl;
    std::cout << "baseFunc13 is b  " << b << std::endl;
}
void baseFunc14(int64_t a, bool b)
{
    if (b) {
        std::cout << "baseFunc14 is a  " << a << std::endl;
        std::cout << "baseFunc14 is b  " << b << std::endl;
    } else {
        std::cout << "baseFunc14 is a  " << a << std::endl;
        std::cout << "baseFunc14 is b  " << b << std::endl;
    }
}
void baseFunc15(int64_t a, float b)
{
    std::cout << "baseFunc15 is a  " << a << std::endl;
    std::cout << "baseFunc15 is b  " << b << std::endl;
}
void baseFunc16(int64_t a, double b)
{
    std::cout << "baseFunc16 is a  " << a << std::endl;
    std::cout << "baseFunc16 is b  " << b << std::endl;
}
void baseFunc17(float a, bool b)
{
    if (b) {
        std::cout << "baseFunc17 is a  " << a << std::endl;
        std::cout << "baseFunc17 is b  " << b << std::endl;
    } else {
        std::cout << "baseFunc17 is a  " << a << std::endl;
        std::cout << "baseFunc17 is b  " << b << std::endl;
    }
}
void baseFunc18(float a, string_view b)
{
    std::cout << "baseFunc18 is a  " << a << std::endl;
    std::cout << "baseFunc18 is b  " << b << std::endl;
}
void baseFunc19(double a, string_view b)
{
    std::cout << "baseFunc19 is a  " << a << std::endl;
    std::cout << "baseFunc19 is b  " << b << std::endl;
}

void baseFunc20(double a, bool b)
{
    if (b) {
        std::cout << "baseFunc20 is a  " << a << std::endl;
        std::cout << "baseFunc20 is b  " << b << std::endl;
    } else {
        std::cout << "baseFunc20 is a  " << a << std::endl;
        std::cout << "baseFunc20 is b  " << b << std::endl;
    }
}

void baseFunc21(optional_view<int32_t> option1, optional_view<int64_t> option2)
{
    if (option1) {
        std::cout << *option1 << std::endl;
    } else if (option2) {
        std::cout << *option2 << std::endl;
    } else {
        std::cout << "Null" << std::endl;
    }
}
void baseFunc22(optional_view<float> option1, optional_view<double> option2)
{
    if (option1) {
        std::cout << *option1 << std::endl;
    } else if (option2) {
        std::cout << *option2 << std::endl;
    } else {
        std::cout << "Null" << std::endl;
    }
}
void baseFunc23(optional_view<string> option1, optional_view<bool> option2)
{
    if (option1) {
        std::cout << *option1 << std::endl;
    } else if (option2) {
        std::cout << *option2 << std::endl;
    } else {
        std::cout << "Null" << std::endl;
    }
}
void baseFunc24(optional_view<int16_t> option1, optional_view<int64_t> option2)
{
    if (option1) {
        std::cout << *option1 << std::endl;
    } else if (option2) {
        std::cout << *option2 << std::endl;
    } else {
        std::cout << "Null" << std::endl;
    }
}

::primitives_test::primitivesVoid get_interface()
{
    return make_holder<primitivesVoid, ::primitives_test::primitivesVoid>();
}

::primitives_test::primitivesBoolean get_interface_bool()
{
    return make_holder<primitivesBoolean, ::primitives_test::primitivesBoolean>();
}
::primitives_test::primitivesInteger get_interface_interger()
{
    return make_holder<primitivesInteger, ::primitives_test::primitivesInteger>();
}

}  // namespace

TH_EXPORT_CPP_API_multiply(multiply);
TH_EXPORT_CPP_API_baseCFunc(baseCFunc);
TH_EXPORT_CPP_API_baseAFunc(baseAFunc);
TH_EXPORT_CPP_API_baseBFunc(baseBFunc);
TH_EXPORT_CPP_API_baseDFunc(baseDFunc);
TH_EXPORT_CPP_API_baseEFunc(baseEFunc);
TH_EXPORT_CPP_API_baseHFunc(baseHFunc);
TH_EXPORT_CPP_API_baseIFunc(baseIFunc);
TH_EXPORT_CPP_API_baseFunc1(baseFunc1);
TH_EXPORT_CPP_API_baseFunc2(baseFunc2);
TH_EXPORT_CPP_API_baseFunc3(baseFunc3);
TH_EXPORT_CPP_API_baseFunc4(baseFunc4);
TH_EXPORT_CPP_API_baseFunc5(baseFunc5);
TH_EXPORT_CPP_API_baseFunc6(baseFunc6);
TH_EXPORT_CPP_API_baseFunc7(baseFunc7);
TH_EXPORT_CPP_API_baseFunc8(baseFunc8);
TH_EXPORT_CPP_API_baseFunc9(baseFunc9);
TH_EXPORT_CPP_API_baseFunc10(baseFunc10);
TH_EXPORT_CPP_API_baseFunc11(baseFunc11);
TH_EXPORT_CPP_API_baseFunc12(baseFunc12);
TH_EXPORT_CPP_API_baseFunc13(baseFunc13);
TH_EXPORT_CPP_API_baseFunc14(baseFunc14);
TH_EXPORT_CPP_API_baseFunc15(baseFunc15);
TH_EXPORT_CPP_API_baseFunc16(baseFunc16);
TH_EXPORT_CPP_API_baseFunc17(baseFunc17);
TH_EXPORT_CPP_API_baseFunc18(baseFunc18);
TH_EXPORT_CPP_API_baseFunc19(baseFunc19);
TH_EXPORT_CPP_API_baseFunc20(baseFunc20);
TH_EXPORT_CPP_API_baseFunc21(baseFunc21);
TH_EXPORT_CPP_API_baseFunc22(baseFunc22);
TH_EXPORT_CPP_API_baseFunc23(baseFunc23);
TH_EXPORT_CPP_API_baseFunc24(baseFunc24);
TH_EXPORT_CPP_API_get_interface(get_interface);
TH_EXPORT_CPP_API_get_interface_bool(get_interface_bool);
TH_EXPORT_CPP_API_get_interface_interger(get_interface_interger);
