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

set(TESTS_EXECUTED_ON_ARKJSVM
    bouncing_pandas
    callable_signature
    compiler
    constructor
    constructor_as_arg
    conversion_types
    dynamic_enum
    dynamic_object
    dynamic_with_lambda
    import
    class_method
    test_cast
)

set(TESTS_EXECUTED_ON_NODEVM
    checked
    class_composite
    class_extension
    class_operations
    declgen_ets2ts
    dot_operator
    escompat
    escompat_demo
    es_object
    event_loop_callback_poster_tests
    exceptions
    generic
    generic_as_parameter
    generic_call_params
    generic_static
    generic_types
    getter
    implement_interfaces
    indexed_signature
    indexed_type
    instanceof
    interface
    interfaces
    intersection_signature
    map_mirror_classes
    number_subtypes
    objects_passing
    operator_new
    optional_reference_type
    passing_optional_type
    perf
    promise
    proxies
    sample
    scenarios
    setter
    # standalone_function
    # static_enum
    strict_equality
    test_callable
    test_class_methods_returning_values
    test_composite_types
    test_frontend
    test_incorrect_conversion_types
    test_intrins
    test_return_this
    test_undefined
    timer
)

if(PANDA_JS_ETS_HYBRID_MODE)
    foreach(TEST_DIR ${TESTS_EXECUTED_ON_ARKJSVM})
        add_subdirectory(${TEST_DIR})
    endforeach()
endif()

foreach(TEST_DIR ${TESTS_EXECUTED_ON_NODEVM})
    add_subdirectory(${TEST_DIR})
endforeach()
