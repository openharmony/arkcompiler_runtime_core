/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef ABCKIT_CPP_HELPERS_MOCK
#define ABCKIT_CPP_HELPERS_MOCK

#include <string>
#include "../../include/cpp/abckit_cpp.h"
#include "check_mock.h"

#include <gtest/gtest.h>

namespace abckit::mock::helpers {

inline abckit::core::Module getMockCoreModule(const abckit::File &file)
{
    std::vector<abckit::core::Module> modules;

    file.EnumerateModules([&modules](abckit::core::Module md) -> bool {
        modules.push_back(md);
        return true;
    });

    EXPECT_TRUE(CheckMockedApi("fileEnumerateModules"));

    return modules.front();
}

inline abckit::core::Class getMockCoreClass(const abckit::File &file)
{
    abckit::core::Module cmd = getMockCoreModule(file);

    std::vector<abckit::core::Class> classes;

    cmd.EnumerateClasses([&classes](abckit::core::Class cls) -> bool {
        classes.push_back(cls);
        return true;
    });

    EXPECT_TRUE(CheckMockedApi("moduleEnumerateClasses"));

    return classes.front();
}

inline abckit::core::Function getMockCoreFunction(const abckit::File &file)
{
    abckit::core::Class cls = getMockCoreClass(file);
    std::vector<abckit::core::Function> funcs;

    cls.EnumerateMethods([&](abckit::core::Function md) -> bool {
        funcs.push_back(md);
        return true;
    });

    EXPECT_TRUE(CheckMockedApi("classEnumerateMethods"));

    return funcs.front();
}

}  // namespace abckit::mock::helpers

#endif  // ABCKIT_CPP_HELPERS_MOCK
