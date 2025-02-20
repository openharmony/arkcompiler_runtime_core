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

ani_add_gtest(ani_test_class_ops_class_get_field
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/class_get_field_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/class_get_field_test.sts
)

ani_add_gtest(ani_test_class_ops_class_get_static_field
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/class_get_static_field_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/class_get_static_field_test.sts
)

ani_add_gtest(ani_test_class_ops_get_static_method
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/get_static_method_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/get_static_method_test.sts
)

ani_add_gtest(ani_test_class_ops_get_method
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/get_method_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/get_method_test.sts
)

ani_add_gtest(ani_test_class_ops_call_static_method_bool
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_bool_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_bool_test.sts
)

ani_add_gtest(ani_test_class_ops_call_static_method_byte
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_byte_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_byte_test.sts
)

ani_add_gtest(ani_test_class_ops_call_static_method_short
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_short_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_short_test.sts
)

ani_add_gtest(ani_test_class_ops_call_static_method_int
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_int_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_int_test.sts
)

ani_add_gtest(ani_test_class_ops_call_static_method_long
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_long_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_long_test.sts
)

ani_add_gtest(ani_test_class_ops_call_static_method_float
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_float_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_float_test.sts
)

ani_add_gtest(ani_test_class_ops_call_static_method_double
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_double_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_double_test.sts
)

ani_add_gtest(ani_test_class_ops_call_static_method_ref
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_ref_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_ref_test.sts
)

ani_add_gtest(ani_test_class_ops_call_static_method_void
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_void_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_void_test.sts
)

ani_add_gtest(ani_test_class_ops_call_static_method_by_name_bool
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_by_name_bool_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_by_name_bool_test.sts
)

ani_add_gtest(ani_test_class_ops_call_static_method_by_name_byte
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_by_name_byte_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_by_name_byte_test.sts
)

ani_add_gtest(ani_test_class_ops_call_static_method_by_name_short
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_by_name_short_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_by_name_short_test.sts
)

ani_add_gtest(ani_test_class_ops_call_static_method_by_name_int
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_by_name_int_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_by_name_int_test.sts
)

ani_add_gtest(ani_test_class_ops_call_static_method_by_name_long
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_by_name_long_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_by_name_long_test.sts
)

ani_add_gtest(ani_test_class_ops_call_static_method_by_name_float
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_by_name_float_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_by_name_float_test.sts
)

ani_add_gtest(ani_test_class_ops_call_static_method_by_name_double
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_by_name_double_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_by_name_double_test.sts
)

ani_add_gtest(ani_test_class_ops_call_static_method_by_name_ref
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_by_name_ref_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_by_name_ref_test.sts
)

ani_add_gtest(ani_test_class_ops_call_static_method_by_name_void
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_by_name_void_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/call_static_method_by_name_void_test.sts
)