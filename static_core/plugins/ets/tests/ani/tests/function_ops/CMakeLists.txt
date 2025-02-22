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

ani_add_gtest(ani_test_function_call_short
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/function_call_short_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/function_call_short_test.sts
)

ani_add_gtest(ani_test_function_call_int
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/function_call_int_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/function_call_int_test.sts
)

ani_add_gtest(ani_test_function_call_long
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/function_call_long_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/function_call_long_test.sts
)

ani_add_gtest(ani_test_function_call_ref
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/function_call_ref_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/function_call_ref_test.sts
)

ani_add_gtest(ani_test_function_call_boolean
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/function_call_boolean_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/function_call_boolean_test.sts
)

ani_add_gtest(ani_test_function_call_byte
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/function_call_byte_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/function_call_byte_test.sts
)

ani_add_gtest(ani_test_function_call_double
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/function_call_double_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/function_call_double_test.sts
)

ani_add_gtest(ani_test_function_call_float
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/function_call_float_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/function_call_float_test.sts
)

ani_add_gtest(ani_test_function_call_void
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/function_call_void_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/function_call_void_test.sts
)