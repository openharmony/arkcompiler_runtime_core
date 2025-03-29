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
#include "overload.impl.hpp"

#include <numeric>

#include "core/array.hpp"
#include "core/map.hpp"
#include "core/string.hpp"
#include "overload.Color.proj.0.hpp"
#include "overload.Mystruct.proj.1.hpp"
#include "overload.overloadInterface.proj.2.hpp"
#include "stdexcept"
// Please delete <stdexcept> include when you implement
using namespace taihe::core;
namespace {
class overloadInterface {
public:
    int8_t overloadFunc_i8(int8_t a, int8_t b)
    {
        std::cout << "overloadFunc_i8: a = " << (int)(a) << ", b = " << (int)(b) << std::endl;
        return a;
    }

    int16_t overloadFunc_i16(int16_t a, int16_t b)
    {
        std::cout << "overloadFunc_i16: a = " << a << ", b = " << b << std::endl;
        return a;
    }

    int32_t overloadFunc_i32(int32_t a, int32_t b)
    {
        std::cout << "overloadFunc_i32: a = " << a << ", b = " << b << std::endl;
        return a;
    }

    float overloadFunc_f32(float a, float b)
    {
        std::cout << "overloadFunc_f32: a = " << a << ", b = " << b << std::endl;
        return a;
    }

    double overloadFunc_f64(double a, double b)
    {
        std::cout << "overloadFunc_f64: a = " << a << ", b = " << b << std::endl;
        return a;
    }

    bool overloadFunc_bool(bool a, bool b)
    {
        std::cout << "overloadFunc_bool: a = " << a << ", b = " << b << std::endl;
        return a;
    }

    string overloadFunc_String(string_view a, string_view b)
    {
        std::cout << "overloadFunc_String: a = " << a << ", b = " << b << std::endl;
        return string(a);
    }

    int8_t overloadFunc_i8_i16(int8_t a, int16_t b)
    {
        std::cout << "overloadFunc_i8_i16: a = " << (int)(a) << ", b = " << b << std::endl;
        return a;
    }

    int8_t overloadFunc_i8_f32(int8_t a, float b)
    {
        std::cout << "overloadFunc_i8_f32: a = " << (int)(a) << ", b = " << b << std::endl;
        return a;
    }

    int8_t overloadFunc_i8_String(int8_t a, string_view b)
    {
        std::cout << "overloadFunc_i8_String: a = " << (int)(a) << ", b = " << b << std::endl;
        return a;
    }
    int32_t overloadFunc_enum(::overload::Color const &p0)
    {
        std::cout << "overloadFunc_enum: color = " << p0 << std::endl;
        return static_cast<int32_t>(p0);
    }
    string overloadFunc_mystruct(::overload::Mystruct const &p0)
    {
        std::cout << "overloadFunc_mystruct: testNum = " << p0.testNum << ", testStr = " << p0.testStr << std::endl;
        return p0.testStr;
    }
    void overloadFunc_5param_1(int8_t p0, int16_t p1, int32_t p2, float p3, double p4)
    {
        std::cout << "overloadFunc_5param_1: p0 = " << (int)p0 << ", p1 = " << p1 << ", p2 = " << p2 << ", p3 = " << p3
                  << ", p4 = " << p4 << std::endl;
    }

    bool overloadFunc_5param_primitive_2(bool p0, string_view p1, int8_t p2, int16_t p3, int32_t p4)
    {
        std::cout << "overloadFunc_5param_primitive_2: p0 = " << p0 << ", p1 = " << p1 << ", p2 = " << (int)p2
                  << ", p3 = " << p3 << ", p4 = " << p4 << std::endl;
        return true;
    }

    float overloadFunc_5param_primitive_3(float p0, double p1, bool p2, string_view p3, int8_t p4)
    {
        std::cout << "overloadFunc_5param_primitive_3: p0 = " << p0 << ", p1 = " << p1 << ", p2 = " << p2
                  << ", p3 = " << p3 << ", p4 = " << (int)p4 << std::endl;
        return p0;
    }

    string overloadFunc_5param_primitive_4(string_view p0, int16_t p1, int32_t p2, float p3, bool p4)
    {
        std::cout << "overloadFunc_5param_primitive_4: p0 = " << p0 << ", p1 = " << p1 << ", p2 = " << p2
                  << ", p3 = " << p3 << ", p4 = " << p4 << std::endl;
        return string(p0);
    }

    string overloadFunc_5param_primitive_5(string_view p0, string_view p1, string_view p2, bool p3, bool p4)
    {
        std::cout << "overloadFunc_5param_primitive_5: p0 = " << p0 << ", p1 = " << p1 << ", p2 = " << p2
                  << ", p3 = " << p3 << ", p4 = " << p4 << std::endl;
        return string(p0);
    }

    int16_t overloadFunc_5param_primitive_6(int16_t p0, int32_t p1, float p2, double p3, bool p4)
    {
        std::cout << "overloadFunc_5param_primitive_6: p0 = " << p0 << ", p1 = " << p1 << ", p2 = " << p2
                  << ", p3 = " << p3 << ", p4 = " << p4 << std::endl;
        return p0;
    }

    string overloadFunc_5param_primitive_7(string_view p0, int8_t p1, int16_t p2, float p3, bool p4)
    {
        std::cout << "overloadFunc_5param_primitive_7: p0 = " << p0 << ", p1 = " << (int)p1 << ", p2 = " << p2
                  << ", p3 = " << p3 << ", p4 = " << p4 << std::endl;
        return string(p0);
    }

    bool overloadFunc_5param_primitive_8(bool p0, int8_t p1, int32_t p2, double p3, string_view p4)
    {
        std::cout << "overloadFunc_5param_primitive_8: p0 = " << p0 << ", p1 = " << (int)p1 << ", p2 = " << p2
                  << ", p3 = " << p3 << ", p4 = " << p4 << std::endl;
        return true;
    }

    double overloadFunc_5param_primitive_9(double p0, bool p1, string_view p2, int16_t p3, int32_t p4)
    {
        std::cout << "overloadFunc_5param_primitive_9: p0 = " << p0 << ", p1 = " << p1 << ", p2 = " << p2
                  << ", p3 = " << p3 << ", p4 = " << p4 << std::endl;
        return p0;
    }

    int8_t overloadFunc_5param_primitive_10(int8_t p0, float p1, bool p2, string_view p3, int32_t p4)
    {
        std::cout << "overloadFunc_5param_primitive_10: p0 = " << (int)p0 << ", p1 = " << p1 << ", p2 = " << p2
                  << ", p3 = " << p3 << ", p4 = " << p4 << std::endl;
        return p0;
    }
    void overloadFunc_10param(int8_t p0, int16_t p1, int32_t p2, float p3, double p4, bool p5, string_view p6,
                              array_view<int8_t> p7, array_view<int16_t> p8, array_view<int32_t> p9)
    {
        std::cout << "overloadFunc_10param: p0 = " << static_cast<int>(p0) << ", p1 = " << p1 << ", p2 = " << p2
                  << ", p3 = " << p3 << ", p4 = " << p4 << ", p5 = " << p5 << ", p6 = " << p6 << ", p7 = [";

        for (size_t i = 0; i < p7.size(); ++i) {
            std::cout << static_cast<int>(p7.data()[i]);
            if (i < p7.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << "], p8 = [";
        for (size_t i = 0; i < p8.size(); ++i) {
            std::cout << p8.data()[i];
            if (i < p8.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << "], p9 = [";
        for (size_t i = 0; i < p9.size(); ++i) {
            std::cout << p9.data()[i];
            if (i < p9.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << "]" << std::endl;
    }

    void overloadFunc_10param1(int8_t p0, int16_t p1, int32_t p2, float p3, double p4, bool p5, string_view p6,
                               array_view<int8_t> p7, ::overload::Mystruct const &p8, ::overload::Color const &p9)
    {
        std::cout << "overloadFunc_10param1: p0 = " << static_cast<int>(p0) << ", p1 = " << p1 << ", p2 = " << p2
                  << ", p3 = " << p3 << ", p4 = " << p4 << ", p5 = " << p5 << ", p6 = " << p6 << ", p7 = [";

        for (size_t i = 0; i < p7.size(); ++i) {
            std::cout << static_cast<int>(p7.data()[i]);
            if (i < p7.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << "], p8 = {testNum = " << p8.testNum << ", testStr = " << p8.testStr << "}"
                  << ", p9 = " << p9 << std::endl;
    }

    void overloadFunc_10param2(int8_t p0, ::overload::Mystruct const &p1, ::overload::Color const &p2,
                               array_view<float> p3, array_view<double> p4, array_view<bool> p5, array_view<string> p6,
                               array_view<int8_t> p7, array_view<int16_t> p8, array_view<int32_t> p9)
    {
        std::cout << "overloadFunc_10param2: p0 = " << static_cast<int>(p0) << ", p1 = {testNum = " << p1.testNum
                  << ", testStr = " << p1.testStr << "}"
                  << ", p2 = " << p2 << ", p3 = [";

        for (size_t i = 0; i < p3.size(); ++i) {
            std::cout << p3.data()[i];
            if (i < p3.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << "], p4 = [";
        for (size_t i = 0; i < p4.size(); ++i) {
            std::cout << p4.data()[i];
            if (i < p4.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << "], p5 = [";
        for (size_t i = 0; i < p5.size(); ++i) {
            std::cout << p5.data()[i];
            if (i < p5.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << "], p6 = [";
        for (size_t i = 0; i < p6.size(); ++i) {
            std::cout << p6.data()[i];
            if (i < p6.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << "], p7 = [";
        for (size_t i = 0; i < p7.size(); ++i) {
            std::cout << static_cast<int>(p7.data()[i]);
            if (i < p7.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << "], p8 = [";
        for (size_t i = 0; i < p8.size(); ++i) {
            std::cout << p8.data()[i];
            if (i < p8.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << "], p9 = [";
        for (size_t i = 0; i < p9.size(); ++i) {
            std::cout << p9.data()[i];
            if (i < p9.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << "]" << std::endl;
    }
    void overloadFunc_10param3(int8_t p0, int16_t p1, int32_t p2, float p3, double p4, bool p5, string_view p6,
                               array_view<uint8_t> p7, ::overload::Mystruct const &p8, ::overload::Color const &p9)
    {
        std::cout << "overloadFunc_10param3: p0 = " << static_cast<int>(p0) << ", p1 = " << p1 << ", p2 = " << p2
                  << ", p3 = " << p3 << ", p4 = " << p4 << ", p5 = " << p5 << ", p6 = " << p6 << ", p7 = [";

        for (size_t i = 0; i < p7.size(); ++i) {
            std::cout << static_cast<int>(p7.data()[i]);
            if (i < p7.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << "], p8 = {testNum = " << p8.testNum << ", testStr = " << p8.testStr << "}"
                  << ", p9 = " << p9 << std::endl;
    }
    void overloadFunc_10param4(int8_t p0, int16_t p1, int32_t p2, float p3, double p4, bool p5, string_view p6,
                               array_view<int8_t> p7, array_view<uint8_t> p8, ::overload::Color const &p9)
    {
        std::cout << "overloadFunc_10param4: p0 = " << static_cast<int>(p0) << ", p1 = " << p1 << ", p2 = " << p2
                  << ", p3 = " << p3 << ", p4 = " << p4 << ", p5 = " << p5 << ", p6 = " << p6 << ", p7 = [";

        for (size_t i = 0; i < p7.size(); ++i) {
            std::cout << static_cast<int>(p7.data()[i]);
            if (i < p7.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << "], p8 = [";
        for (size_t i = 0; i < p8.size(); ++i) {
            std::cout << static_cast<int>(p8.data()[i]);
            if (i < p8.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << "], p9 = " << p9 << std::endl;
    }
    int32_t overloadFunc_point(array_view<int32_t> a)
    {
        std::cout << "overloadFunc_point: a = [";
        for (size_t i = 0; i < a.size(); ++i) {
            std::cout << a.data()[i];
            if (i < a.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]" << std::endl;
        return a.data()[0];
    }

    float overloadFunc_point1(array_view<float> a)
    {
        std::cout << "overloadFunc_point1: a = [";
        for (size_t i = 0; i < a.size(); ++i) {
            std::cout << a.data()[i];
            if (i < a.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]" << std::endl;
        return a.data()[0];
    }

    uint8_t overloadFunc_ArrayBuffer(array_view<uint8_t> a)
    {
        std::cout << "overloadFunc_ArrayBuffer: a = [";
        for (size_t i = 0; i < a.size(); ++i) {
            std::cout << static_cast<int>(a.data()[i]);
            if (i < a.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]" << std::endl;
        return std::accumulate(a.begin(), a.end(), 0);
    }

    void overloadFunc_enum_record(::overload::Color const &p1, map_view<string, int16_t> p2) {}
    void overloadFunc_array_record(array_view<int32_t> p1, map_view<string, int16_t> p2) {}
};
::overload::overloadInterface get_interface()
{
    return make_holder<overloadInterface, ::overload::overloadInterface>();
}

}  // namespace
TH_EXPORT_CPP_API_get_interface(get_interface);
