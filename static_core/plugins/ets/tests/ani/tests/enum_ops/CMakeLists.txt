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

ani_add_gtest(ani_test_get_enum_item
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/get_enum_item_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/get_enum_item_test.ets
)

ani_add_gtest(ani_test_enum_item_get_enum
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/enum_item_get_enum_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/enum_item_get_enum_test.ets
)

ani_add_gtest(ani_test_enum_item_get_value
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/enum_item_get_value_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/enum_item_get_value_test.ets
)

ani_add_gtest(ani_test_enum_item_get_name
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/enum_item_get_name_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/enum_item_get_name_test.ets
)

ani_add_gtest(ani_test_enum_item_get_index
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/enum_item_get_index_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/enum_item_get_index_test.ets
)
