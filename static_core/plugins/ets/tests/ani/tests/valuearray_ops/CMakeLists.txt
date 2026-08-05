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

ani_add_gtest(ani_test_valuearray_getlen
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/valuearray_get_length_test.cpp
)

ani_add_gtest(ani_test_valuearray_new
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/valuearray_new_test.cpp
)

ani_add_gtest(ani_test_valuearray_compatibility
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/array_compatibility_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/array_compatibility_test.ets
)

ani_add_gtest(ani_test_valuearray_region_boolean
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/valuearray_region_boolean_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/valuearray_region_boolean_test.ets
)

ani_add_gtest(ani_test_valuearray_region_byte
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/valuearray_region_byte_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/valuearray_region_byte_test.ets
)

ani_add_gtest(ani_test_valuearray_region_char
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/valuearray_region_char_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/valuearray_region_char_test.ets
)

ani_add_gtest(ani_test_valuearray_region_short
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/valuearray_region_short_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/valuearray_region_short_test.ets
)

ani_add_gtest(ani_test_valuearray_region_int
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/valuearray_region_int_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/valuearray_region_int_test.ets
)

ani_add_gtest(ani_test_valuearray_region_long
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/valuearray_region_long_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/valuearray_region_long_test.ets
)

ani_add_gtest(ani_test_valuearray_region_float
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/valuearray_region_float_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/valuearray_region_float_test.ets
)

ani_add_gtest(ani_test_valuearray_region_double
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/valuearray_region_double_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/valuearray_region_double_test.ets
)
