# Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

panda_ets_add_gtest(
    NO_CORES
    NAME ets_internal_objects_mock_tests
    SOURCES
        get_test_class.cpp
        ets_method_signature_test.cpp
        ets_string_test.cpp
        ets_string_builder_test.cpp
        ets_field_test.cpp
        ets_object_test.cpp
        ets_array_test.cpp
        ets_method_test.cpp
        ets_class_file_test.cpp
    LIBRARIES
        arkruntime
        arkassembler
    INCLUDE_DIRS
        ${PANDA_ETS_PLUGIN_SOURCE}/runtime
    SANITIZERS
        ${PANDA_SANITIZERS_LIST}
    PANDA_STD_LIB
        $<TARGET_PROPERTY:mock_stdlib,FILE>
    DEPS_TARGETS
        mock_stdlib
    TEST_GROUP ets_tests_runtime_gtests
)

panda_ets_add_gtest(
    NO_CORES
    NAME ets_internal_objects_tests
    SOURCES
        ets_promise_test.cpp
        ets_job_test.cpp
        ets_class_test.cpp
        ets_string_from_char_code_test.cpp
        ets_sync_primitives_test.cpp
        ets_arraybuf_test.cpp
        ets_arrayobj_test.cpp
        ets_string_builder_members_test.cpp
        ets_to_string_cache_test.cpp
        ets_finalizable_weak_ref_test.cpp
        ets_runtime_linker_test.cpp
        ets_abc_runtime_linker_test.cpp
        ets_typeapi_test.cpp
        typed_arrays_test.cpp
        ets_base_enum_test.cpp
        ets_error_test.cpp
    LIBRARIES
        arkassembler
        arkruntime
    INCLUDE_DIRS
        ${PANDA_ETS_PLUGIN_SOURCE}/runtime
    SANITIZERS
        ${PANDA_SANITIZERS_LIST}
    PANDA_STD_LIB
        $<TARGET_PROPERTY:etsstdlib,FILE>
    DEPS_TARGETS
        etsstdlib
    TEST_GROUP ets_tests_runtime_gtests
)
