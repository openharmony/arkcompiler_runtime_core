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
    arkts_esvalue
    array
    backtrace
    bouncing_pandas
    builtin_not_implement
    callable_signature
    cast_dynamic_to_object
    checked
    class_composite
    class_extension
    class_method
    class_operations
    compiler
    constructor
    constructor_as_arg
    conversion_types
    coroutines
    date
    declgen_ets2ts
    dot_operator
    dynamic_enum
    dynamic_object
    dynamic_with_lambda
    enum
    error
    escompat
    escompat_demo
    escompat_interface
    es_object
    esvalue_with_string_key
    event_loop_callback_poster_tests
    eworker
    exceptions
    for_of
    function
    function_overload
    function_with_this
    generic
    generic_as_parameter
    generic_call_params
    generic_static
    generic_types
    getter
    getter_arkjs
    hybridgref
    implement_interfaces
    import
    indexed_signature
    indexed_type
    instanceof
    interface
    interface_arg_impl
    interfaces
    interop_hash_state
    intersection_signature
    iteration
    map
    map_mirror_classes
    mimic_iterator
    mimic_special_function
    number_subtypes
    object_literal
    objects_passing
    operator_new
    optional_reference_type
    passing_optional_type
    perf
    primitive
    promise
    prototype_object
    proxies
    rest_spread_parameter
    runtimelinker
    sample
    scenarios
    set
    setter
    setter_arkjs
    shared_reference
    standalone_function
    static_enum
    strict_equality
    sts_vm_interface_impl_tests
    taskpool
    test_callable
    test_cast
    test_class_methods_returning_values
    test_composite_types
    test_frontend
    test_incorrect_conversion_types
    test_inheritance
    test_intrins
    test_optional_params
    test_record
    test_return_this
    test_sealed
    test_transfer
    test_undefined
    test_union
    test_wrap_jsvalue
    timer
)

SUBDIRLIST(EXECUTED_TESTS
    ${CMAKE_CURRENT_LIST_DIR}
    DISABLES_TESTS
)

foreach(TEST_DIR ${EXECUTED_TESTS})
    add_subdirectory(${TEST_DIR})
endforeach()
