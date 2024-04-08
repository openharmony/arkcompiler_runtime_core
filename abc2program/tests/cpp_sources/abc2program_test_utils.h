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

#ifndef ABC2PROGRAM_TEST_CPP_SOURCES_ABC2PROGRAM_TEST_UTILS_H
#define ABC2PROGRAM_TEST_CPP_SOURCES_ABC2PROGRAM_TEST_UTILS_H

#include <cstdlib>
#include <string>
#include <vector>
#include <set>
#include <cstdint>
#include <string_view>

namespace panda::abc2program {

constexpr uint32_t NUM_OF_CODE_TEST_UT_FOO_METHOD_INS = 77;

constexpr uint32_t NUM_OF_CODE_TEST_UT_FUNC_MAIN_0_METHOD_INS = 18;

constexpr uint32_t NUM_OF_CODE_TEST_UT_GOO_METHOD_INS = 2;

constexpr uint32_t NUM_OF_CODE_TEST_UT_FOO_METHOD_CATCH_BLOCKS = 3;

constexpr uint32_t NUM_OF_HELLOE_WORLD_TEST_UT_HELLO_WORLD_METHOD_INS = 4;

constexpr uint32_t NUM_OF_HELLOE_WORLD_TEST_UT_FUNC_MAIN_0_METHOD_INS = 23;

constexpr uint32_t NUM_OF_HELLOE_WORLD_TEST_UT_INSTANCE_INITIALIZER_METHOD_INS = 4;

constexpr size_t NUM_OF_ARGS_FOR_FOO_METHOD = 3;

constexpr std::string_view FUNC_NAME_HELLO_WORLD = ".HelloWorld";

constexpr std::string_view FUNC_NAME_MAIN_0 = ".func_main_0";

constexpr std::string_view FUNC_NAME_INSTANCE_INITIALIZER = ".instance_initializer";

constexpr std::string_view FUNC_NAME_FOO = ".foo";

constexpr std::string_view FUNC_NAME_GOO = ".goo";

class Abc2ProgramTestUtils {
public:
    template <typename T>
    static bool ValidateStrings(const T &strings, const T &expected_strings);
    static std::set<std::string> helloworld_expected_program_strings_;
    static std::vector<std::string> helloworld_expected_record_names_;
    static bool ValidateProgramStrings(const std::set<std::string> &program_strings);
    static bool ValidateRecordNames(const std::vector<std::string> &record_names);
};  // class Abc2ProgramTestUtils

}  // namespace panda::abc2program

#endif  // ABC2PROGRAM_TEST_CPP_SOURCES_ABC2PROGRAM_TEST_UTILS_H