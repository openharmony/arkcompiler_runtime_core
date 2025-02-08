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
    standalone_function
    static_enum
)

set(TESTS_EXECUTED_WITH_NODEJS
    checked
    class_composite
    class_extension
    class_operations
    declgen_ets2ts
    dot_operator_nodejs
    event_loop_callback_poster_tests
    exceptions
    generic
    getter
    implement_interfaces
    indexed_signature
    indexed_type
    instanceof
    interface
    interfaces
    interop_hash_state
    intersection_signature
    map_mirror_classes
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

# NOTE: Remove this condition after all tests have been migrated to ArkJSVM.
list(LENGTH TESTS_EXECUTED_WITH_NODEJS INTEROP_NODEJS_TEST_NUM)
if(NOT ${INTEROP_NODEJS_TEST_NUM} EQUAL 39)
    message(FATAL_ERROR "All new interop tests shouls be executed on ArkJSVM. 
                         INTEROP_NODEJS_TEST_NUM = ${INTEROP_NODEJS_TEST_NUM}")
endif()

if(PANDA_JS_ETS_HYBRID_MODE)
    set(NOT_ARKJSVM_TEST_PACKAGES ${TESTS_EXECUTED_WITH_NODEJS})
    list(APPEND NOT_ARKJSVM_TEST_PACKAGES ${DISABLES_TESTS})

    SUBDIRLIST(TESTS_EXECUTED_ON_ARKJSVM 
        ${CMAKE_CURRENT_LIST_DIR}
        NOT_ARKJSVM_TEST_PACKAGES
    )

    foreach(TEST_DIR ${TESTS_EXECUTED_ON_ARKJSVM})
        add_subdirectory(${TEST_DIR})
    endforeach()
else()
    foreach(TEST_DIR ${TESTS_EXECUTED_WITH_NODEJS})
        add_subdirectory(${TEST_DIR})
    endforeach()
endif()
