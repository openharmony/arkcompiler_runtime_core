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

ani_add_gtest(ani_test_get_null
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/get_null_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/get_null_test.ets
)

ani_add_gtest(ani_test_get_undefined
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/get_undefined_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/get_undefined_test.ets
)

ani_add_gtest(ani_test_reference_is_null
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/reference_is_null_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/reference_is_null_test.ets
)

ani_add_gtest(ani_test_reference_is_undefined
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/reference_is_undefined_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/reference_is_undefined_test.ets
)

ani_add_gtest(ani_test_reference_is_nullish_value
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/reference_is_nullish_value_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/reference_is_nullish_value_test.ets
)

ani_add_gtest(ani_test_reference_delete
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/reference_delete_test.cpp
)

ani_add_gtest(ani_test_reference_equals
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/reference_equals_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/reference_equals_test.ets
)

ani_add_gtest(ani_test_reference_strict_equals
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/reference_strict_equals_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/reference_strict_equals_test.ets
)
