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
#include "iface_test.impl.hpp"

#include "iface_test.Foo.proj.2.hpp"
#include "stdexcept"
#include "taihe/string.hpp"
// Please delete <stdexcept> include when you implement
using namespace taihe;

namespace {

class Foo {
    string name_ {"foo"};

public:
    void bar()
    {
        std::cout << "Fooimpl: " << __func__ << std::endl;
    }

    string getName()
    {
        std::cout << "Fooimpl: " << __func__ << " " << name_ << std::endl;
        return name_;
    }

    void setName(string_view name)
    {
        std::cout << "Fooimpl: " << __func__ << " " << name << std::endl;
        name_ = name;
    }
};

::iface_test::Foo getFooIface()
{
    std::cout << __func__ << std::endl;
    return make_holder<Foo, ::iface_test::Foo>();
}

string printFooName(::iface_test::weak::Foo foo)
{
    auto name = foo->getName();
    std::cout << __func__ << ": " << name << std::endl;
    return name;
}

}  // namespace
// The macros used below are automatically generated code.
// NOLINTBEGIN
TH_EXPORT_CPP_API_getFooIface(getFooIface);
TH_EXPORT_CPP_API_printFooName(printFooName);
// NOLINTEND
