# Copyright (c) 2025 Huawei Device Co., Ltd.
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

ani_verify_add_gtest(ani_verify_test_fixed_array_new_boolean
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_fixed_array_new_boolean_test.cpp
)

ani_verify_add_gtest(ani_verify_test_fixed_array_new_char
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_fixed_array_new_char_test.cpp
)

ani_verify_add_gtest(ani_verify_test_fixed_array_new_byte
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_fixed_array_new_byte_test.cpp
)

ani_verify_add_gtest(ani_verify_test_fixed_array_new_short
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_fixed_array_new_short_test.cpp
)

ani_verify_add_gtest(ani_verify_test_fixed_array_new_int
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_fixed_array_new_int_test.cpp
)

ani_verify_add_gtest(ani_verify_test_fixed_array_new_long
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_fixed_array_new_long_test.cpp
)

ani_verify_add_gtest(ani_verify_test_fixed_array_new_float
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_fixed_array_new_float_test.cpp
)

ani_verify_add_gtest(ani_verify_test_fixed_array_new_double
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_fixed_array_new_double_test.cpp
)
