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
#include "optional.impl.hpp"
#include "optional.ExampleInterface.proj.2.hpp"
#include "optional.Union.proj.1.hpp"
#include "taihe/map.hpp"
#include "taihe/optional.hpp"
// Please delete <stdexcept> include when you implement
using namespace taihe;

namespace {
class ExampleInterface {
public:
    void func_primitive_i8(optional_view<int8_t> param1)
    {
        std::cout << "opt1 has value: " << *param1 << std::endl;
    }

    void func_primitive_i16(optional_view<int16_t> param1)
    {
        std::cout << "opt1 has value: " << *param1 << std::endl;
    }

    void func_primitive_i32(optional_view<int32_t> param1)
    {
        std::cout << "opt1 has value: " << *param1 << std::endl;
    }

    void func_primitive_i64(optional_view<int64_t> param1)
    {
        std::cout << "opt1 has value: " << *param1 << std::endl;
    }

    void func_primitive_f32(optional_view<float> param1)
    {
        std::cout << "opt1 has value: " << *param1 << std::endl;
    }

    void func_primitive_f64(optional_view<double> param1)
    {
        std::cout << "opt1 has value: " << *param1 << std::endl;
    }

    void func_primitive_bool(optional_view<bool> param1)
    {
        std::cout << "opt1 has value: " << *param1 << std::endl;
    }

    void func_primitive_String(optional_view<string> param1)
    {
        std::cout << "opt1 has value: " << *param1 << std::endl;
    }

    void func_Array(optional_view<array<int32_t>> param1)
    {
        std::cout << "func_Array: [";
        for (auto const &elem : *param1) {
            std::cout << elem << " ";
        }
        std::cout << "]" << std::endl;
    }

    void func_Buffer(optional_view<array<uint8_t>> param1)
    {
        std::cout << "func_Array: [";
        for (auto const &elem : *param1) {
            std::cout << (int)elem << " ";
        }
        std::cout << "]" << std::endl;
    }

    void func_Union(optional_view<::optional::Union> param1)
    {
        std::cout << (*param1).get_sValue_ref() << std::endl;
        std::cout << (*param1).get_iValue_ref() << std::endl;
    }

    void func_Map(optional_view<map<string, int32_t>> param1)
    {
        for (auto it = (*param1).begin(); it != (*param1).end(); ++it) {
            auto const &[key, value] = *it;
            std::cout << "Key: " << key << ", Value: " << value << std::endl;
        }
    }

    taihe::optional<string> getName()
    {
        return taihe::optional<string>::make("hello");
    }

    taihe::optional<int8_t> geti8()
    {
        return taihe::optional<int8_t>::make(1);
    }

    taihe::optional<int16_t> geti16()
    {
        const int16_t geti16_value = 100;
        return taihe::optional<int16_t>::make(geti16_value);
    }

    taihe::optional<int32_t> geti32()
    {
        const int32_t geti32_value = 1024;
        return taihe::optional<int32_t>::make(geti32_value);  // 默认返回0
    }

    taihe::optional<int64_t> geti64()
    {
        const int64_t geti64_value = 999999;
        return taihe::optional<int64_t>::make(geti64_value);  // 默认返回0
    }

    taihe::optional<float> getf32()
    {
        return taihe::optional<float>::make(0.0f);
    }

    taihe::optional<double> getf64()
    {
        return taihe::optional<double>::make(0.0);
    }

    taihe::optional<bool> getbool()
    {
        return taihe::optional<bool>::make(false);
    }

    taihe::optional<array<uint8_t>> getArraybuffer()
    {
        int arr_size = 10;
        int arr_num = 6;
        return taihe::optional<array<uint8_t>>::make(array<uint8_t>::make(arr_size, arr_num));
    }
};

::optional::ExampleInterface get_interface()
{
    return make_holder<ExampleInterface, ::optional::ExampleInterface>();
}

void printTestInterfaceName(::optional::weak::ExampleInterface testiface)
{
    auto res = testiface->getName();
    std::cout << __func__ << ": " << *res << std::endl;
}

void printTestInterfaceNumberi8(::optional::weak::ExampleInterface testiface)
{
    auto res = testiface->geti8();
    std::cout << __func__ << ": " << *res << std::endl;
}

void printTestInterfaceNumberi16(::optional::weak::ExampleInterface testiface)
{
    auto res = testiface->geti16();
    std::cout << __func__ << ": " << *res << std::endl;
}

void printTestInterfaceNumberi32(::optional::weak::ExampleInterface testiface)
{
    auto res = testiface->geti16();
    std::cout << __func__ << ": " << *res << std::endl;
}

void printTestInterfaceNumberi64(::optional::weak::ExampleInterface testiface)
{
    auto res = testiface->geti64();
    std::cout << __func__ << ": " << *res << std::endl;
}

void printTestInterfaceNumberf32(::optional::weak::ExampleInterface testiface)
{
    auto res = testiface->getf32();
    std::cout << __func__ << ": " << *res << std::endl;
}

void printTestInterfaceNumberf64(::optional::weak::ExampleInterface testiface)
{
    auto res = testiface->getf64();
    std::cout << __func__ << ": " << *res << std::endl;
}

void printTestInterfacebool(::optional::weak::ExampleInterface testiface)
{
    auto res = testiface->getbool();
    std::cout << __func__ << ": " << *res << std::endl;
}

void printTestInterfaceArraybuffer(::optional::weak::ExampleInterface testiface)
{
    auto arr = testiface->getArraybuffer();

    for (size_t i = 0; i < (*arr).size(); ++i) {
        std::cout << static_cast<int>((*arr).data()[i]);
        if (i < (*arr).size() - 1) {
            std::cout << ", ";
        }
    }
}
}  // namespace

// The macros used below are automatically generated code
// NOLINTBEGIN
TH_EXPORT_CPP_API_get_interface(get_interface) TH_EXPORT_CPP_API_printTestInterfaceName(printTestInterfaceName);
TH_EXPORT_CPP_API_printTestInterfaceNumberi8(printTestInterfaceNumberi8);
TH_EXPORT_CPP_API_printTestInterfaceNumberi16(printTestInterfaceNumberi16);
TH_EXPORT_CPP_API_printTestInterfaceNumberi32(printTestInterfaceNumberi32);
TH_EXPORT_CPP_API_printTestInterfaceNumberi64(printTestInterfaceNumberi64);
TH_EXPORT_CPP_API_printTestInterfaceNumberf32(printTestInterfaceNumberf32);
TH_EXPORT_CPP_API_printTestInterfaceNumberf64(printTestInterfaceNumberf64);
TH_EXPORT_CPP_API_printTestInterfacebool(printTestInterfacebool);
TH_EXPORT_CPP_API_printTestInterfaceArraybuffer(printTestInterfaceArraybuffer);
// NOLINTEND
