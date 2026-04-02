# Copyright (c) 2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set(TEST_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/test_data")

set(TEST_SOURCES
    test_helper.cpp
    abc_file_test.cpp
    verifier_test.cpp
    verify_api_test.cpp
)

if(USE_ARKFILE)
    list(APPEND TEST_SOURCES verifier_deep_test.cpp)
endif()

add_executable(static_abc_verifier_tests ${TEST_SOURCES})

target_link_libraries(static_abc_verifier_tests PRIVATE
    static_abc_verifier_static
    GTest::gtest_main
)

target_compile_definitions(static_abc_verifier_tests PRIVATE
    TEST_DATA_DIR="${TEST_DATA_DIR}"
)

if(USE_ARKFILE)
    apply_arkfile_settings(static_abc_verifier_tests)
    if(ARKFILE_ROOT)
        target_compile_definitions(static_abc_verifier_tests PRIVATE
            DEEP_TEST_ABC_DIR="${ARKFILE_ROOT}/build"
        )
    endif()
endif()

include(GoogleTest)
gtest_discover_tests(static_abc_verifier_tests)
