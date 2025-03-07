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
    # Blocked by #21277
    class_composite
    class_extension
    class_operations
    implement_interfaces
    instanceof
    interface
    interfaces
    setter
    scenarios
    generic
    getter

    # Blocked by #22353
    dot_operator_nodejs

    # Blocked by #22422
    indexed_type

    # Blocked by #22821
    declgen_ets2ts

    # Blocked by #22822
    perf

    # mem will be fixed by MM and concurrency, ets_function_proxy blocked by # 22648 
    proxies

    #Blocked by #22430
    test_callable

    # Blocked by #21832
    test_incorrect_conversion_types

    # Blocked by #22647
    test_intrins

    # Blocked by 22820
    checked
)

# NOTE: Remove this condition after all tests have been migrated to ArkJSVM.
list(LENGTH TESTS_EXECUTED_WITH_NODEJS INTEROP_NODEJS_TEST_NUM)
if(NOT ${INTEROP_NODEJS_TEST_NUM} EQUAL 20)
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
