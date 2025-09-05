# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

add_custom_target(ani_verify_tests COMMENT "Common target to run VerifyANI tests")

# Add gtest-based tests to ani_verify_tests target.
#
# Example usage:
#   ani_verify_add_gtest(test_name
#     ETS_CONFIG path/to/arktsconfig.json
#     CPP_SOURCES
#       tests/unit1_test.cpp
#       tests/unit2_test.cpp
#     ETS_SOURCES
#       tests/unit1_test.ets
#     LIBRARIES
#       lib_target1
#       lib_target2
#   )
function(ani_verify_add_gtest TARGET)
    # Parse arguments
    cmake_parse_arguments(
        ARG
        ""
        "ETS_CONFIG"
        "CPP_SOURCES;ETS_SOURCES;LIBRARIES;TSAN_EXTRA_OPTIONS"
        ${ARGN}
    )

    # Check TARGET prefix
    set(TARGET_PREFIX "ani_verify_test_")
    string(FIND "${TARGET}" "${TARGET_PREFIX}" PREFIX_INDEX)
    if(NOT ${PREFIX_INDEX} EQUAL 0)
        message(FATAL_ERROR "TARGET (${TARGET}) should have '${TARGET_PREFIX}' prefix")
    endif()

    ani_add_gtest(${TARGET}
        TARGET_PREFIX ${TARGET_PREFIX}
        ETS_CONFIG ${ARG_ETS_CONFIG}
        ETS_SOURCES ${ARG_ETS_SOURCES}
        CPP_SOURCES ${ARG_CPP_SOURCES}
        LIBRARIES ${ARG_LIBRARIES} ani_gtest
        TSAN_EXTRA_OPTIONS ${ARG_TSAN_EXTRA_OPTIONS}
        INCLUDE_DIRS ${PANDA_ETS_PLUGIN_SOURCE}/runtime/ani
    )
    add_dependencies(ani_verify_tests ${TARGET}_gtests)
endfunction(ani_verify_add_gtest)
