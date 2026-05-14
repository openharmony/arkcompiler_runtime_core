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

ani_verify_add_gtest(ani_verify_test_any_instance_of
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_any_instance_of_test.cpp
)

ani_verify_add_gtest(ani_verify_test_any_get_property
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_any_get_property_test.cpp
)

ani_verify_add_gtest(ani_verify_test_any_set_property
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_any_set_property_test.cpp
)

ani_verify_add_gtest(ani_verify_test_any_get_by_index
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_any_get_by_index_test.cpp
)

ani_verify_add_gtest(ani_verify_test_any_set_by_index
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_any_set_by_index_test.cpp
)

ani_verify_add_gtest(ani_verify_test_any_get_by_value
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_any_get_by_value_test.cpp
)

ani_verify_add_gtest(ani_verify_test_any_set_by_value
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_any_set_by_value_test.cpp
)

ani_verify_add_gtest(ani_verify_test_any_call
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_any_call_test.cpp
)

ani_verify_add_gtest(ani_verify_test_any_call_method
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_any_call_method_test.cpp
)

ani_verify_add_gtest(ani_verify_test_any_new
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_any_new_test.cpp
)
