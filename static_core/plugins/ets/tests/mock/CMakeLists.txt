# Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

add_custom_target(ets_test_napi_mock_run)

panda_ets_napi_add_gtest(
    NAME calling_methods_test_1
    CPP_SOURCE calling_methods_test_1.cpp
    ETS_SOURCE CallingMethodsTest.sts
)

panda_ets_napi_add_gtest(
    NAME calling_methods_test_2
    CPP_SOURCE calling_methods_test_2.cpp
    ETS_SOURCE CallingMethodsTest.sts
)

panda_ets_napi_add_gtest(
    NAME calling_methods_test_3
    CPP_SOURCE calling_methods_test_3.cpp
    ETS_SOURCE CallingMethodsTest.sts
)

panda_ets_napi_add_gtest(
    NAME calling_methods_test_4
    CPP_SOURCE calling_methods_test_4.cpp
    ETS_SOURCE CallingMethodsTest.sts
)

panda_ets_napi_add_gtest(
    NAME calling_methods_test_5
    CPP_SOURCE calling_methods_test_5.cpp
    ETS_SOURCE CallingMethodsTest.sts
)

panda_ets_napi_add_gtest(
    NAME calling_methods_test_6
    CPP_SOURCE calling_methods_test_6.cpp
    ETS_SOURCE CallingMethodsTest.sts
)

panda_ets_napi_add_gtest(
    NAME calling_methods_test_7
    CPP_SOURCE calling_methods_test_7.cpp
    ETS_SOURCE CallingMethodsTest.sts
)

panda_ets_napi_add_gtest(
    NAME calling_methods_test_8
    CPP_SOURCE calling_methods_test_8.cpp
    ETS_SOURCE CallingMethodsTest.sts
)

panda_ets_napi_add_gtest(
    NAME accessing_objects_fields_test_1
    CPP_SOURCE accessing_objects_fields_test_1.cpp
    ETS_SOURCE AccessingObjectsFieldsTest.sts
)

panda_ets_napi_add_gtest(
    NAME accessing_objects_fields_test_2
    CPP_SOURCE accessing_objects_fields_test_2.cpp
    ETS_SOURCE AccessingObjectsFieldsTest.sts
)

panda_ets_napi_add_gtest(
    NAME accessing_objects_fields_test_3
    CPP_SOURCE accessing_objects_fields_test_3.cpp
    ETS_SOURCE AccessingObjectsFieldsTest.sts
)

panda_ets_napi_add_gtest(
    NAME clinit_native_test
    CPP_SOURCE clinit_native_test.cpp
    ETS_SOURCE ClinitNativeTest.sts
)

panda_ets_napi_add_gtest(
    NAME effective_types_test
    CPP_SOURCE effective_types_test.cpp
    ETS_SOURCE EffectiveTypesTest.sts
)

panda_ets_napi_add_gtest(
    NAME final_field_native_test
    CPP_SOURCE final_field_native_test.cpp
    ETS_SOURCE FinalFieldNativeTest.sts
)

panda_ets_napi_add_gtest(
    NAME registering_native_methods_test
    CPP_SOURCE registering_native_methods_test.cpp
    ETS_SOURCE RegisteringNativeMethodsTest.sts
)

panda_ets_napi_add_gtest(
    NAME register_native_allocation_test
    CPP_SOURCE register_native_allocation_test.cpp
    ETS_SOURCE RegisterNativeAllocationTest.sts
)

panda_ets_napi_add_gtest(
    NAME stack_reference_check_test
    CPP_SOURCE stack_reference_check_test.cpp
    ETS_SOURCE StackReferenceCheckTest.sts
)

panda_ets_napi_add_gtest(
    NAME stack_trace_test
    CPP_SOURCE stack_trace_test.cpp
    ETS_SOURCE StackTraceTest.sts
)

panda_ets_napi_add_gtest(
    NAME array_native_test
    CPP_SOURCE array_native_test.cpp
)

panda_ets_napi_add_gtest(
    NAME array_native_test_2
    CPP_SOURCE array_native_test_2.cpp
)

panda_ets_napi_add_gtest(
    NAME array_native_test_3
    CPP_SOURCE array_native_test_3.cpp
)

panda_ets_napi_add_gtest(
    NAME js_mode_launch
    CPP_SOURCE js_mode_launch.cpp
    ETS_SOURCE js_mode_launch.sts
)

panda_ets_napi_add_gtest(
    NAME coroswitch_in_cctor
    CPP_SOURCE coroswitch_in_cctor.cpp
    ETS_SOURCE coroswitch_in_cctor.sts
)