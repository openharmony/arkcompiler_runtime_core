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
#include <cstdint>
#include <iostream>

#include "staticTest.impl.hpp"
#include "taihe/runtime.hpp"

namespace {
int32_t add_impl(int32_t a, int32_t b)
{
    if (a == 0) {
        taihe::set_error("some error happen in add impl");
        return b;
    } else {
        std::cout << "add impl " << a + b << std::endl;
        return a + b;
    }
}

int32_t sum_impl(int32_t a, int32_t b)
{
    return a * b;
}

struct AuthorIBase {
    taihe::string name;

    explicit AuthorIBase(taihe::string_view name) : name(name) {}

    AuthorIBase(taihe::string_view name, taihe::string_view t) : name(t) {}

    ~AuthorIBase() {}

    taihe::string get()
    {
        return name;
    }

    void set(taihe::string_view a)
    {
        this->name = a;
        return;
    }
};

::staticTest::IBase getIBase_impl(taihe::string_view name)
{
    return taihe::make_holder<AuthorIBase, ::staticTest::IBase>(name);
}

::staticTest::IBase getIBase_test_impl(taihe::string_view name, taihe::string_view t)
{
    return taihe::make_holder<AuthorIBase, ::staticTest::IBase>(name, t);
}

class ITest {
public:
    ITest() : name("hello ITest") {}

    ~ITest() {}

    taihe::string name;

    taihe::string get()
    {
        return name;
    }

    void set(taihe::string_view a)
    {
        this->name = a;
        return;
    }
};

int32_t static_func(int32_t a, int32_t b)
{
    return a + b;
}

::staticTest::ITest ctor_func()
{
    return taihe::make_holder<ITest, ::staticTest::ITest>();
}

}

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_addSync(add_impl);
TH_EXPORT_CPP_API_sumSync(sum_impl);
TH_EXPORT_CPP_API_getIBase(getIBase_impl);
TH_EXPORT_CPP_API_getIBase_test(getIBase_test_impl);
TH_EXPORT_CPP_API_static_func(static_func);
TH_EXPORT_CPP_API_ctor_func(ctor_func);
// NOLINTEND
