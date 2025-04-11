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
#include "on_off.impl.hpp"

#include <iostream>

#include "on_off.IBase.proj.2.hpp"
#include "stdexcept"
#include "taihe/callback.hpp"
#include "taihe/string.hpp"
// Please delete <stdexcept> include when you implement
using namespace taihe;

namespace {
class IBase {
public:
    IBase(string a, string b) : str(a), new_str(b) {}

    ~IBase() {}

    void onSet(callback_view<void()> a)
    {
        a();
        std::cout << "IBase::onSet" << std::endl;
    }

    void offSet(callback_view<void()> a)
    {
        a();
        std::cout << "IBase::offSet" << std::endl;
    }

private:
    string str;
    string new_str;
};

class BaseCls {
public:
    BaseCls() {}
};

::on_off::IBase getIBase(string_view a, string_view b)
{
    return make_holder<IBase, ::on_off::IBase>(a, b);
}

void onFoo(callback_view<void()> a)
{
    a();
    std::cout << "onFoo" << std::endl;
}

void onBar(callback_view<void()> a)
{
    a();
    std::cout << "onBar" << std::endl;
}

void onBaz(int32_t a, callback_view<void()> cb)
{
    cb();
    std::cout << "a =" << a << ", onBaz" << std::endl;
}

void offFoo(callback_view<void()> a)
{
    a();
    std::cout << "offFoo" << std::endl;
}

void offBar(callback_view<void()> a)
{
    a();
    std::cout << "offBar" << std::endl;
}

void offBaz(int32_t a, callback_view<void()> cb)
{
    cb();
    std::cout << "offBaz" << std::endl;
}

void onFooStatic(callback_view<void()> a)
{
    a();
    std::cout << "onFooStatic" << std::endl;
}

void offFooStatic(callback_view<void()> a)
{
    a();
    std::cout << "offFooStatic" << std::endl;
}
}  // namespace

// The macros used below are automatically generated code
// NOLINTBEGIN
TH_EXPORT_CPP_API_getIBase(getIBase);
TH_EXPORT_CPP_API_onFoo(onFoo);
TH_EXPORT_CPP_API_onBar(onBar);
TH_EXPORT_CPP_API_onBaz(onBaz);
TH_EXPORT_CPP_API_offFoo(offFoo);
TH_EXPORT_CPP_API_offBar(offBar);
TH_EXPORT_CPP_API_offBaz(offBaz);
TH_EXPORT_CPP_API_onFooStatic(onFooStatic);
TH_EXPORT_CPP_API_offFooStatic(offFooStatic);
// NOLINTEND
