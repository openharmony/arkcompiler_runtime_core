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
#include "inner.impl.hpp"

#include "core/array.hpp"
#include "core/map.hpp"
#include "core/string.hpp"
#include "inner.Color.proj.0.hpp"
#include "inner.ErrorResponse.proj.1.hpp"
#include "inner.Mystruct.proj.1.hpp"
#include "inner.TestInterface.proj.2.hpp"
#include "inner.test1.proj.2.hpp"
#include "inner.test20.proj.2.hpp"
#include "inner.testA.proj.2.hpp"
#include "inner.union_primitive.proj.1.hpp"
#include "stdexcept"

// Please delete <stdexcept> include when you implement
using namespace taihe::core;
namespace {
class TestInterface {
public:
    void noparam_noreturn() {}
    void primitives_noreturn(int8_t a) {}
    void primitives_noreturn1(int16_t a) {}
    void primitives_noreturn2(int32_t a) {}
    void primitives_noreturn3(float a) {}
    void primitives_noreturn4(double a) {}
    void primitives_noreturn5(bool a) {}
    void primitives_noreturn6(string_view a) {}
    void primitives_noreturn7(int64_t a) {}
    void primitives_noreturn8(int8_t a) {}
    void primitives_noreturn9(int32_t a) {}
    int32_t primitives_return(int32_t a)
    {
        return 1;
    }
    void containers_noreturn1(array_view<int8_t> a) {}
    void containers_noreturn3(array_view<uint8_t> a) {}
    void containers_noreturn2(::inner::union_primitive const &a) {}
    void containers_noreturn4(::inner::Color a) {}
    void containers_noreturn5(map_view<string, int32_t> a) {}
    string containers_return(::inner::union_primitive const &a)
    {
        return "containers_return";
    }
    ::inner::ErrorResponse func_ErrorResponse()
    {
        return {true, 10000, "test58"};
    }
    void overloadFunc_i8(int8_t a, int8_t b) {}
    string overloadFunc_i16(array_view<int8_t> a, array_view<uint8_t> b)
    {
        return "overload array";
    }
    void overloadFunc_i32() {}
    ::inner::Mystruct overloadFunc_f32(::inner::Mystruct const &a)
    {
        return a;
    }

    int8_t i8 = -128;           // 范围: -128 到 127
    int16_t i16 = -32768;       // 16位有符号整数，范围: -32,768 到 32,767
    int32_t i32 = -2147483648;  // 32位有符号整数，范围: -2,147,483,648 到 2,147,483,647
    int64_t i64 = 1000;         // 64位有符号整数，范围: -9,223,372,036,854,775,808 到
                                // 9,223,372,036,854,775,807

    // 浮点类型
    float f32 = 3.14159265f;         // 32位单精度浮点，约7位有效数字
    double f64 = 3.141592653589793;  // 64位双精度浮点，约15位有效数字

    // 其他类型
    string name_ {"String"};
    bool flag = true;  // 布尔类型，值: true 或 false

    string getName()
    {
        std::cout << __func__ << " " << name_ << std::endl;
        return name_;
    }
    int8_t geti8()
    {
        std::cout << __func__ << " " << i8 << std::endl;
        return i8;
    }
    int16_t geti16()
    {
        std::cout << __func__ << " " << i16 << std::endl;
        return i16;
    }
    int32_t geti32()
    {
        std::cout << __func__ << " " << i32 << std::endl;
        return i32;
    }
    int64_t geti64()
    {
        std::cout << __func__ << " " << i64 << std::endl;
        return i64;
    }
    float getf32()
    {
        std::cout << __func__ << " " << f32 << std::endl;
        return f32;
    }
    double getf64()
    {
        std::cout << __func__ << " " << f64 << std::endl;
        return f64;
    }
    bool getbool()
    {
        std::cout << __func__ << " " << flag << std::endl;
        return flag;
    }

    array<uint8_t> getArraybuffer()
    {
        array<uint8_t> result = array<uint8_t>::make(5);
        std::fill(result.begin(), result.end(), 3);
        return result;
    }
    array<int8_t> getArray()
    {
        array<int8_t> result = array<int8_t>::make(5);
        std::fill(result.begin(), result.end(), 3);
        return result;
    }
    ::inner::union_primitive getunion()
    {
        return ::inner::union_primitive::make_sValue("union string");
    }
    map<string, int8_t> getrecord()
    {
        map<string, int8_t> m;
        m.emplace("key1", static_cast<int8_t>(1));
        m.emplace("key2", static_cast<int8_t>(2));
        m.emplace("key3", static_cast<int8_t>(3));
        return m;
    }
    ::inner::Color getColorEnum()
    {
        return (::inner::Color::key_t)((int)1);
    }
};
class test1 {
public:
    void fun1() {}
    void fun2() {}
};
class test2 {
public:
    void fun1() {}
    void fun2() {}
};
class test3 {
public:
    void fun1() {}
    void fun2() {}
};
class test4 {
public:
    void fun1() {}
    void fun2() {}
};
class test5 {
public:
    void fun1() {}
    void fun2() {}
};
class test6 {
public:
    void fun1() {}
    void fun2() {}
};
class test7 {
public:
    void fun1() {}
    void fun2() {}
};
class test8 {
public:
    void fun1() {}
    void fun2() {}
};
class test9 {
public:
    void fun1() {}
    void fun2() {}
};
class test10 {
public:
    void fun1() {}
    void fun2() {}
};
class test11 {
public:
    void fun1() {}
    void fun2() {}
};
class test12 {
public:
    void fun1() {}
    void fun2() {}
};
class test13 {
public:
    void fun1() {}
    void fun2() {}
};
class test14 {
public:
    void fun1() {}
    void fun2() {}
};
class test15 {
public:
    void fun1() {}
    void fun2() {}
};
class test16 {
public:
    void fun1() {}
    void fun2() {}
};
class test17 {
public:
    void fun1() {}
    void fun2() {}
};
class test18 {
public:
    void fun1() {}
    void fun2() {}
};
class test19 {
public:
    void fun1() {}
    void fun2() {}
};
class test20 {
public:
    void fun1() {}
    void fun2() {}
};
class testA {
public:
    string fun1()
    {
        std::cout << "fun1" << std::endl;
        return "fun1";
    }
};
class testB {
public:
    string fun2()
    {
        std::cout << "IfaceB func_b" << std::endl;
        return "fun2";
    }
    string fun1()
    {
        std::cout << "IfaceB func_a" << std::endl;
        return "fun1";
    }
};
class testC {
public:
    string fun3()
    {
        std::cout << "IfaceC func_c" << std::endl;
        return "fun3";
    }
    string fun1()
    {
        std::cout << "IfaceC func_a" << std::endl;
        return "fun1";
    }
};
void primitives_noreturn(int32_t a, double b, bool c, string_view d, int8_t e) {}
string primitives_return(int32_t a, double b, bool c, string_view d, int8_t e)
{
    return "primitives_return";
}
void containers_noreturn(array_view<int8_t> a, array_view<int16_t> b, array_view<float> c, array_view<double> d,
                         ::inner::union_primitive const &e)
{
}
string containers_return(array_view<int8_t> a, array_view<int16_t> b, array_view<float> c, array_view<double> d,
                         ::inner::union_primitive const &e)
{
    return "containers_return";
}
void enum_noreturn(::inner::Color a, ::inner::Color b, ::inner::Color c, ::inner::Color d, ::inner::Color e) {}
string enum_return(::inner::Color a, ::inner::Color b, ::inner::Color c, ::inner::Color d, ::inner::Color e)
{
    return "enum_return";
}
::inner::TestInterface get_interface()
{
    return make_holder<TestInterface, ::inner::TestInterface>();
}
string printTestInterfaceName(::inner::weak::TestInterface testiface)
{
    auto name = testiface->getName();
    std::cout << __func__ << ": " << name << std::endl;
    return name;
}
int8_t printTestInterfaceNumberi8(::inner::weak::TestInterface testiface)
{
    auto name = testiface->geti8();
    std::cout << __func__ << ": " << (int)name << std::endl;
    return name;
}
int16_t printTestInterfaceNumberi16(::inner::weak::TestInterface testiface)
{
    auto name = testiface->geti16();
    std::cout << __func__ << ": " << name << std::endl;
    return name;
}
int32_t printTestInterfaceNumberi32(::inner::weak::TestInterface testiface)
{
    auto name = testiface->geti32();
    std::cout << __func__ << ": " << name << std::endl;
    return name;
}
int64_t printTestInterfaceNumberi64(::inner::weak::TestInterface testiface)
{
    auto name = testiface->geti64();
    std::cout << __func__ << ": " << name << std::endl;
    return name;
}
float printTestInterfaceNumberf32(::inner::weak::TestInterface testiface)
{
    auto name = testiface->getf32();
    std::cout << __func__ << ": " << name << std::endl;
    return name;
}
double printTestInterfaceNumberf64(::inner::weak::TestInterface testiface)
{
    auto name = testiface->getf64();
    std::cout << __func__ << ": " << name << std::endl;
    return name;
}
bool printTestInterfacebool(::inner::weak::TestInterface testiface)
{
    auto name = testiface->getbool();
    std::cout << __func__ << ": " << name << std::endl;
    return name;
}
array<uint8_t> printTestInterfaceArraybuffer(::inner::weak::TestInterface testiface)
{
    // array<int32_t> result = array<int32_t>::make(5);
    // std::fill(result.begin(), result.end(), 3);
    array<uint8_t> arr = testiface->getArraybuffer();
    for (size_t i = 0; i < arr.size(); ++i) {
        std::cout << static_cast<int>(arr.data()[i]);
        if (i < arr.size() - 1) {
            std::cout << ", ";
        }
    }
    return arr;
}
array<int8_t> printTestInterfaceArray(::inner::weak::TestInterface testiface)
{
    array<int8_t> arr = testiface->getArray();
    for (size_t i = 0; i < arr.size(); ++i) {
        std::cout << static_cast<int>(arr.data()[i]);
        if (i < arr.size() - 1) {
            std::cout << ", ";
        }
    }
    return arr;
}
::inner::union_primitive printTestInterfaceUnion(::inner::weak::TestInterface testiface)
{
    ::inner::union_primitive up = testiface->getunion();
    std::cout << "s: " << up.get_sValue_ref() << std::endl;
    return up;
}
map<string, int8_t> printTestInterfaceRecord(::inner::weak::TestInterface testiface)
{
    map<string, int8_t> m = testiface->getrecord();
    for (const auto &[key, value] : m) {
        std::cout << "Key: " << key << ", Value: " << static_cast<int>(value) << std::endl;
        // 注意：int8_t需要转为int打印，否则会输出ASCII字符
    }
    return m;
}
::inner::Color printTestInterfaceEnum(::inner::weak::TestInterface testiface)
{
    ::inner::Color c = testiface->getColorEnum();
    std::cout << "enum get_key " << (int)c.get_key() << std::endl;
    return c;
}
::inner::test1 get_interface_1()
{
    return make_holder<test1, ::inner::test1>();
}
::inner::test20 get_interface_20()
{
    return make_holder<test20, ::inner::test20>();
}
::inner::testA get_interface_A()
{
    return make_holder<testA, ::inner::testA>();
}
::inner::testB get_interface_B()
{
    return make_holder<testB, ::inner::testB>();
}
::inner::testC get_interface_C()
{
    return make_holder<testC, ::inner::testC>();
}

}  // namespace
TH_EXPORT_CPP_API_primitives_noreturn(primitives_noreturn);
TH_EXPORT_CPP_API_primitives_return(primitives_return);
TH_EXPORT_CPP_API_containers_noreturn(containers_noreturn);
TH_EXPORT_CPP_API_containers_return(containers_return);
TH_EXPORT_CPP_API_enum_noreturn(enum_noreturn);
TH_EXPORT_CPP_API_enum_return(enum_return);
TH_EXPORT_CPP_API_get_interface(get_interface);
TH_EXPORT_CPP_API_printTestInterfaceName(printTestInterfaceName);
TH_EXPORT_CPP_API_printTestInterfaceNumberi8(printTestInterfaceNumberi8);
TH_EXPORT_CPP_API_printTestInterfaceNumberi16(printTestInterfaceNumberi16);
TH_EXPORT_CPP_API_printTestInterfaceNumberi32(printTestInterfaceNumberi32);
TH_EXPORT_CPP_API_printTestInterfaceNumberi64(printTestInterfaceNumberi64);
TH_EXPORT_CPP_API_printTestInterfaceNumberf32(printTestInterfaceNumberf32);
TH_EXPORT_CPP_API_printTestInterfaceNumberf64(printTestInterfaceNumberf64);
TH_EXPORT_CPP_API_printTestInterfacebool(printTestInterfacebool);
TH_EXPORT_CPP_API_printTestInterfaceArraybuffer(printTestInterfaceArraybuffer);
TH_EXPORT_CPP_API_printTestInterfaceArray(printTestInterfaceArray);
TH_EXPORT_CPP_API_printTestInterfaceUnion(printTestInterfaceUnion);
TH_EXPORT_CPP_API_printTestInterfaceRecord(printTestInterfaceRecord);
TH_EXPORT_CPP_API_printTestInterfaceEnum(printTestInterfaceEnum);
TH_EXPORT_CPP_API_get_interface_1(get_interface_1);
TH_EXPORT_CPP_API_get_interface_20(get_interface_20);
TH_EXPORT_CPP_API_get_interface_A(get_interface_A);
TH_EXPORT_CPP_API_get_interface_B(get_interface_B);
TH_EXPORT_CPP_API_get_interface_C(get_interface_C);
