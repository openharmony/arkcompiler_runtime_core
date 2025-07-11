# Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

set(DISABLES_TESTS
    callable_signature
    cast_dynamic_to_object
    checked
    class_composite
    class_extension
    class_method
    class_operations
    constructor
    constructor_as_arg
    conversion_types
    coroutines
    declgen_ets2ts
    dot_operator
    escompat_demo
    es_object
    exceptions
    for_of
    generic
    generic_as_parameter
    generic_call_params
    generic_static
    getter
    getter_arkjs
    implement_interfaces
    import
    indexed_signature
    indexed_type
    instanceof
    interface
    interfaces
    intersection_signature
    mimic_iterator
    mimic_special_function
    number_subtypes
    operator_new
    optional_reference_type
    passing_optional_type
    perf
    promise
    scenarios
    setter
    standalone_function
    test_class_methods_returning_values
    test_composite_types
    test_frontend
    test_intrins
    test_return_this
    test_undefined
    interface_arg_impl
)

SUBDIRLIST(EXECUTED_TESTS
    ${CMAKE_CURRENT_LIST_DIR}
    DISABLES_TESTS
)

foreach(TEST_DIR ${EXECUTED_TESTS})
    add_subdirectory(${TEST_DIR})
endforeach()
