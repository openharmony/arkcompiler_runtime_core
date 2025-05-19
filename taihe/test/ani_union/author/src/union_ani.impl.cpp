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
#include "union_ani.impl.hpp"

#include "stdexcept"
#include "taihe/runtime.hpp"
#include "taihe/string.hpp"
#include "union_ani.MyUnion.proj.1.hpp"
using namespace taihe;

namespace {
string printInnerUnion(::union_ani::InnerUnion const &data)
{
    switch (data.get_tag()) {
        case ::union_ani::InnerUnion::tag_t::stringValue:
            std::cout << "s: " << data.get_stringValue_ref() << std::endl;
            return "s";
        case ::union_ani::InnerUnion::tag_t::pairValue:
            std::cout << "p: " << data.get_pairValue_ref().a << ", " << data.get_pairValue_ref().b << std::endl;
            return "p";
        case ::union_ani::InnerUnion::tag_t::undefinedValue:
            std::cout << "u" << std::endl;
            return "u";
    }
}

string printMyUnion(::union_ani::MyUnion const &data)
{
    switch (data.get_tag()) {
        case ::union_ani::MyUnion::tag_t::innerValue:
            return printInnerUnion(data.get_innerValue_ref());
        case ::union_ani::MyUnion::tag_t::floatValue:
            std::cout << "f: " << data.get_floatValue_ref() << std::endl;
            return "f";
    }
}

::union_ani::MyUnion makeMyUnion(string_view kind)
{
    const float testFloat = 123.0f;
    if (kind == "s") {
        return ::union_ani::MyUnion::make_innerValue(::union_ani::InnerUnion::make_stringValue("string"));
    }
    if (kind == "p") {
        ::union_ani::Pair pair = {"a", "b"};
        return ::union_ani::MyUnion::make_innerValue(::union_ani::InnerUnion::make_pairValue(pair));
    }
    if (kind == "f") {
        return ::union_ani::MyUnion::make_floatValue(testFloat);
    }
    return ::union_ani::MyUnion::make_innerValue(::union_ani::InnerUnion::make_undefinedValue());
}
}  // namespace

// because these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_printMyUnion(printMyUnion);
TH_EXPORT_CPP_API_makeMyUnion(makeMyUnion);
// NOLINTEND
