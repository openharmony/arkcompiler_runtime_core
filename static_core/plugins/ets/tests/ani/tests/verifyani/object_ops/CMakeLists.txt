# Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

ani_verify_add_gtest(ani_verify_test_object_call_method_boolean
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_object_call_method_boolean_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_object_call_method_boolean_test.ets
)

ani_verify_add_gtest(ani_verify_test_object_get_field_boolean
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_object_get_field_boolean_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_object_get_field_boolean_test.ets
)

ani_verify_add_gtest(ani_verify_test_object_new
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_object_new_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_object_new_test.ets
)

ani_verify_add_gtest(ani_verify_test_object_new_a
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_object_new_a_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_object_new_a_test.ets
)

ani_verify_add_gtest(ani_verify_test_object_set_field_boolean
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_object_set_field_boolean_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_object_set_field_boolean_test.ets
)
