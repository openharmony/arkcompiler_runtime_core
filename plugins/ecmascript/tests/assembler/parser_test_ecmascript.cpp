/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "operand_types_print.h"

#include <gtest/gtest.h>
#include <string>

namespace panda::pandasm {
TEST(parsertests, calli_dyn_3args)
{
    {
        Parser p;
        std::string source = R"(
            .language ECMAScript

            # a0 - function, a1 - this
            .function any main(any a0, any a1) {
                calli.dyn.short 1, a0, a1
                return.dyn
            }
        )";

        auto res = p.Parse(source);

        Error e = p.ShowError();

        ASSERT_EQ(e.err, Error::ErrorType::ERR_NONE);
    }
}
}  // namespace panda::pandasm
