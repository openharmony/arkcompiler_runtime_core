/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "abc2program_test_utils.h"
#include <algorithm>
#include <iostream>

namespace panda::abc2program {

std::set<std::string> Abc2ProgramTestUtils::helloworld_expected_program_strings_ = {"", ".HelloWorld", ".foo", ".goo",
                                                                                    ".instance_initializer",
                                                                                    "HelloWorld", "a_var", "error",
                                                                                    "inner catch",
                                                                                    "masg", "max", "min", "msg",
                                                                                    "null", "outter catch",
                                                                                    "print", "prototype", "str",
                                                                                    "string", "toString", "x"};
std::vector<std::string> Abc2ProgramTestUtils::helloworld_expected_record_names_ = {"_ESModuleRecord",
                                                                                    "_ESSlotNumberAnnotation",
                                                                                    "_ESTypeInfoRecord",
                                                                                    "_GLOBAL"};

template <typename T>
bool Abc2ProgramTestUtils::ValidateStrings(const T &strings, const T &expected_strings)
{
    if (strings.size() != expected_strings.size()) {
        return false;
    }
    for (const std::string &expected_string : expected_strings) {
        const auto string_iter = std::find(strings.begin(), strings.end(), expected_string);
        if (string_iter == strings.end()) {
            return false;
        }
    }
    return true;
}

bool Abc2ProgramTestUtils::ValidateProgramStrings(const std::set<std::string> &program_strings)
{
    return ValidateStrings(program_strings, helloworld_expected_program_strings_);
}

bool Abc2ProgramTestUtils::ValidateRecordNames(const std::vector<std::string> &record_names)
{
    return ValidateStrings(record_names, helloworld_expected_record_names_);
}

}  // namespace panda::abc2program