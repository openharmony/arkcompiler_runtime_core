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
#include "test_union.impl.hpp"

#include <numeric>

#include "stdexcept"
#include "taihe/runtime.hpp"
#include "taihe/string.hpp"
#include "test_union.MyInterface.proj.2.hpp"
#include "test_union.UnionArrayMap1.proj.1.hpp"
#include "test_union.UnionArrayMap2.proj.1.hpp"
#include "test_union.UnionArrayMap3.proj.1.hpp"
#include "test_union.UnionArrayMap4.proj.1.hpp"
#include "test_union.UnionArrayMap5.proj.1.hpp"
#include "test_union.UnionArrayMap6.proj.1.hpp"
#include "test_union.UnionArrayMap7.proj.1.hpp"
#include "test_union.UnionArrayMap8.proj.1.hpp"
#include "test_union.UnionArrayMap9.proj.1.hpp"
#include "test_union.color_map_value1.proj.1.hpp"
#include "test_union.color_map_value2.proj.1.hpp"
#include "test_union.color_map_value3.proj.1.hpp"
#include "test_union.color_map_value4.proj.1.hpp"
#include "test_union.color_map_value5.proj.1.hpp"
#include "test_union.color_map_value6.proj.1.hpp"
#include "test_union.color_map_value7.proj.1.hpp"
#include "test_union.color_map_value8.proj.1.hpp"
#include "test_union.color_map_value9.proj.1.hpp"
#include "test_union.union_mix.proj.1.hpp"
#include "test_union.union_mix_5.proj.1.hpp"
#include "test_union.union_primitive.proj.1.hpp"
#include "test_union.union_primitive_2.proj.1.hpp"
#include "test_union.union_primitive_2_1.proj.1.hpp"
#include "test_union.union_primitive_2_2.proj.1.hpp"
#include "test_union.union_primitive_2_3.proj.1.hpp"
#include "test_union.union_primitive_2_4.proj.1.hpp"
#include "test_union.union_primitive_2_5.proj.1.hpp"
#include "test_union.union_primitive_2_6.proj.1.hpp"
#include "test_union.union_primitive_2_7.proj.1.hpp"
#include "test_union.union_primitive_2_8.proj.1.hpp"
#include "test_union.union_primitive_2_9.proj.1.hpp"
// Please delete <stdexcept> include when you implement
using namespace taihe;

namespace {
class MyInterface {
public:
    string func_union_primitive(::test_union::union_primitive const &data, ::test_union::union_primitive const &data2)
    {
        switch (data.get_tag()) {
            case ::test_union::union_primitive::tag_t::sValue:
                std::cout << "s: " << data.get_sValue_ref() << std::endl;
                return "s";
            case ::test_union::union_primitive::tag_t::i8Value:
                std::cout << "i8: " << (int)data.get_i8Value_ref() << std::endl;
                return "i8";
            case ::test_union::union_primitive::tag_t::i16Value:
                std::cout << "i16: " << data.get_i16Value_ref() << std::endl;
                return "i16";
            case ::test_union::union_primitive::tag_t::i32Value:
                std::cout << "i32: " << data.get_i32Value_ref() << std::endl;
                return "i32";
            case ::test_union::union_primitive::tag_t::f32Value:
                std::cout << "i32: " << data.get_f32Value_ref() << std::endl;
                return "f32";
            case ::test_union::union_primitive::tag_t::f64Value:
                std::cout << "i32: " << data.get_f64Value_ref() << std::endl;
                return "f64";
            case ::test_union::union_primitive::tag_t::bValue:
                std::cout << "bool: " << data.get_bValue_ref() << std::endl;
                return "bool";
        }
    }

    string func_union_primitive_5param(::test_union::union_primitive const &data1,
                                       ::test_union::union_primitive const &data2,
                                       ::test_union::union_primitive const &data3,
                                       ::test_union::union_primitive const &data4,
                                       ::test_union::union_primitive const &data5)
    {
        return "func_union_primitive_5param";
    }

    string func_color_map_value1(::test_union::color_map_value1 const &data1)
    {
        return "func_color_map_value1";
    }

    string func_color_map_value2(::test_union::color_map_value2 const &data1)
    {
        return "func_color_map_value2";
    }

    string func_color_map_value3(::test_union::color_map_value3 const &data1)
    {
        return "func_color_map_value3";
    }

    string func_color_map_value4(::test_union::color_map_value4 const &data1)
    {
        return "func_color_map_value4";
    }

    string func_color_map_value5(::test_union::color_map_value5 const &data1)
    {
        return "func_color_map_value5";
    }

    string func_color_map_value6(::test_union::color_map_value6 const &data1)
    {
        return "func_color_map_value6";
    }

    string func_color_map_value7(::test_union::color_map_value7 const &data1)
    {
        return "func_color_map_value7";
    }

    string func_color_map_value8(::test_union::color_map_value8 const &data1)
    {
        return "func_color_map_value8";
    }

    string func_color_map_value9(::test_union::color_map_value9 const &data1)
    {
        return "func_color_map_value9";
    }

    string func_color_map_value10(::test_union::color_map_value9 const &data1,
                                  ::test_union::color_map_value9 const &data2)
    {
        return "func_color_map_value10";
    }

    string func_union_array_map1(::test_union::UnionArrayMap1 const &data1, ::test_union::UnionArrayMap1 const &data2)
    {
        return "func_union_array_map1";
    }

    string func_union_array_map2(::test_union::UnionArrayMap2 const &data1, ::test_union::UnionArrayMap2 const &data2)
    {
        return "func_union_array_map2";
    }

    string func_union_array_map3(::test_union::UnionArrayMap3 const &data1, ::test_union::UnionArrayMap3 const &data2)
    {
        return "func_union_array_map3";
    }

    string func_union_array_map4(::test_union::UnionArrayMap4 const &data1, ::test_union::UnionArrayMap4 const &data2)
    {
        return "func_union_array_map4";
    }

    string func_union_array_map5(::test_union::UnionArrayMap5 const &data1, ::test_union::UnionArrayMap5 const &data2)
    {
        return "func_union_array_map5";
    }

    string func_union_array_map6(::test_union::UnionArrayMap6 const &data1, ::test_union::UnionArrayMap6 const &data2)
    {
        return "func_union_array_map6";
    }

    string func_union_array_map7(::test_union::UnionArrayMap7 const &data1, ::test_union::UnionArrayMap7 const &data2)
    {
        return "func_union_array_map7";
    }

    string func_union_array_map8(::test_union::UnionArrayMap8 const &data1, ::test_union::UnionArrayMap8 const &data2)
    {
        return "func_union_array_map8";
    }

    string func_union_array_map9(::test_union::UnionArrayMap9 const &data1, ::test_union::UnionArrayMap9 const &data2)
    {
        return "func_union_array_map9";
    }

    string func_union_array_map10(::test_union::UnionArrayMap1 const &data1, ::test_union::UnionArrayMap2 const &data2)
    {
        return "func_union_array_map10";
    }

    string func_union_mix(::test_union::union_mix const &data1, ::test_union::union_mix const &data2,
                          ::test_union::union_mix const &data3, ::test_union::union_mix const &data4,
                          ::test_union::union_mix const &data5)
    {
        return "func_union_mix";
    }

    // This is corn case for function parameters, so skip lint error
    // NOLINTNEXTLINE(readability-function-size)
    string func_union_mix_10param(::test_union::union_mix const &data1, ::test_union::union_mix const &data2,
                                  ::test_union::union_mix const &data3, ::test_union::union_mix const &data4,
                                  ::test_union::union_mix const &data5, ::test_union::union_mix const &data6,
                                  ::test_union::union_mix const &data7, ::test_union::union_mix const &data8,
                                  ::test_union::union_mix const &data9, ::test_union::union_mix const &data10)
    {
        return "func_union_mix_10param";
    }

    ::test_union::union_primitive_2 func_union_primitive_return(string_view kind)
    {
        std::string const s_value = "string";
        int8_t const i8_value = 1;
        if (kind == "s") {
            return ::test_union::union_primitive_2::make_sValue(s_value);
        } else {
            return ::test_union::union_primitive_2::make_i8Value(i8_value);
        }
    }

    ::test_union::union_primitive_2_1 func_union_primitive_return1(string_view kind)
    {
        int8_t const i8_value = 1;
        int16_t const i16_value = 12;
        if (kind == "i8") {
            return ::test_union::union_primitive_2_1::make_i8Value(i8_value);
        } else {
            return ::test_union::union_primitive_2_1::make_i16Value(i16_value);
        }
    }

    ::test_union::union_primitive_2_2 func_union_primitive_return2(string_view kind)
    {
        int8_t const i8_value = 1;
        int const i8_value_2 = 2;
        if (kind == "i8_1") {
            return ::test_union::union_primitive_2_2::make_i8Value(i8_value);
        } else {
            return ::test_union::union_primitive_2_2::make_i8Value2(i8_value_2);
        }
    }

    ::test_union::union_primitive_2_3 func_union_primitive_return3(string_view kind)
    {
        int16_t const i16_value = 12;
        int const i32_value = 123;
        if (kind == "i16") {
            return ::test_union::union_primitive_2_3::make_i16Value(i16_value);
        } else { // if kind == "i32"
            return ::test_union::union_primitive_2_3::make_i32Value(i32_value);
        }
    }

    ::test_union::union_primitive_2_4 func_union_primitive_return4(string_view kind)
    {
        int const i32_value = 123;
        float const f32_value = 1.1f;
        if (kind == "i32") {
            return ::test_union::union_primitive_2_4::make_i32Value(i32_value);
        } else { // if (kind == "f32")
            return ::test_union::union_primitive_2_4::make_f32Value(f32_value);
        }
    }

    ::test_union::union_primitive_2_5 func_union_primitive_return5(string_view kind)
    {
        float const f32_value = 1.1f;
        double const f64_value = 1.234;
        if (kind == "f32") {
            return ::test_union::union_primitive_2_5::make_f32Value(f32_value);
        } else { // if (kind == "f64")
            return ::test_union::union_primitive_2_5::make_f64Value(f64_value);
        }
    }

    ::test_union::union_primitive_2_6 func_union_primitive_return6(string_view kind)
    {
        if (kind == "s") {
            return ::test_union::union_primitive_2_6::make_sValue("hello");
        } else { // if (kind == "b")
            return ::test_union::union_primitive_2_6::make_bValue(true);
        }
    }

    ::test_union::union_primitive_2_7 func_union_primitive_return7(string_view kind)
    {
        if (kind == "b") {
            return ::test_union::union_primitive_2_7::make_bValue(true);
        } else { // if (kind == "i8")
            int8_t const i8_value = 1;
            return ::test_union::union_primitive_2_7::make_i8Value(i8_value);
        }
    }

    ::test_union::union_primitive_2_8 func_union_primitive_return8(string_view kind)
    {
        if (kind == "b") {
            return ::test_union::union_primitive_2_8::make_bValue(true);
        } else { // if (kind == "b1")
            return ::test_union::union_primitive_2_8::make_b1Value(false);
        }
    }

    ::test_union::union_primitive_2_9 func_union_primitive_return9(string_view kind)
    {
        if (kind == "s") {
            return ::test_union::union_primitive_2_9::make_sValue("hello");
        } else { // if (kind == "s1")
            return ::test_union::union_primitive_2_9::make_s1Value("world");
        }
    }

    ::test_union::union_mix_5 func_union_mix5_return(string_view kind)
    {
        if (kind == "s") {
            return ::test_union::union_mix_5::make_sValue("hello");
        }
        if (kind == "i8") {
            int8_t const i8_value = 1;
            return ::test_union::union_mix_5::make_i8Value(i8_value);
        }
        if (kind == "b") {
            return ::test_union::union_mix_5::make_bValue(true);
        }
        if (kind == "c") {
            return ::test_union::union_mix_5::make_enumValue((::test_union::Color::key_t)((int)(1)));
        } else { // if (kind == "arr")
            int const arr_size = 5;
            int const arr_num = 3;
            array<int32_t> result = array<int32_t>::make(arr_size);
            std::fill(result.begin(), result.end(), arr_num);
            return ::test_union::union_mix_5::make_arr32(result);
        }
    }

    ::test_union::union_mix func_union_mix10_return(string_view kind)
    {
        return ::test_union::union_mix::make_bValue(true);
    }
};

string printUnion(::test_union::union_primitive const &data)
{
    switch (data.get_tag()) {
        case ::test_union::union_primitive::tag_t::sValue:
            std::cout << "s: " << data.get_sValue_ref() << std::endl;
            return "s";
        case ::test_union::union_primitive::tag_t::i8Value:
            std::cout << "i8: " << (int)data.get_i8Value_ref() << std::endl;
            return "i8";
        case ::test_union::union_primitive::tag_t::i16Value:
            std::cout << "i16: " << data.get_i16Value_ref() << std::endl;
            return "i16";
        case ::test_union::union_primitive::tag_t::i32Value:
            std::cout << "i32: " << data.get_i32Value_ref() << std::endl;
            return "i32";
        case ::test_union::union_primitive::tag_t::f32Value:
            std::cout << "i32: " << data.get_f32Value_ref() << std::endl;
            return "f32";
        case ::test_union::union_primitive::tag_t::f64Value:
            std::cout << "i32: " << data.get_f64Value_ref() << std::endl;
            return "f64";
        case ::test_union::union_primitive::tag_t::bValue:
            std::cout << "bool: " << data.get_bValue_ref() << std::endl;
            return "bool";
    }
}

::test_union::union_primitive makeUnion(std::string_view kind)
{
    constexpr std::string_view s_value = "string";
    constexpr int8_t i8_value = 1;
    constexpr int16_t i16_value = 123;
    constexpr int32_t i32_value = 1234;
    constexpr float f32_value = 1.12f;
    constexpr double f64_value = 1.12345;
    constexpr bool bool_value = true;

    if (kind == "s") {
        return ::test_union::union_primitive::make_sValue(s_value);
    }
    if (kind == "i8") {
        return ::test_union::union_primitive::make_i8Value(i8_value);
    }
    if (kind == "i16") {
        return ::test_union::union_primitive::make_i16Value(i16_value);
    }
    if (kind == "i32") {
        return ::test_union::union_primitive::make_i32Value(i32_value);
    }
    if (kind == "f32") {
        return ::test_union::union_primitive::make_f32Value(f32_value);
    }
    if (kind == "f64") {
        return ::test_union::union_primitive::make_f64Value(f64_value);
    }
    if (kind == "bool") {
        return ::test_union::union_primitive::make_bValue(bool_value);
    }

    // 处理未知的kind值
    // ...
}

::test_union::MyInterface get_interface()
{
    return make_holder<MyInterface, ::test_union::MyInterface>();
}
}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_printUnion(printUnion);
TH_EXPORT_CPP_API_makeUnion(makeUnion);
TH_EXPORT_CPP_API_get_interface(get_interface);
// NOLINTEND
