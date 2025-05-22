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

add_custom_target(ets_tests_runtime_gtests)

panda_ets_add_gtest(
    NO_CORES
    NAME ets_vm_test
    SOURCES ets_vm_test.cpp
    LIBRARIES arkbase arkfile arkruntime
    INCLUDE_DIRS ${PANDA_ETS_PLUGIN_SOURCE}/runtime
    SANITIZERS ${PANDA_SANITIZERS_LIST}
    PANDA_STD_LIB $<TARGET_PROPERTY:mock_stdlib,FILE>
    DEPS_TARGETS mock_stdlib
    TEST_GROUP ets_tests_runtime_gtests
)

panda_add_gtest_with_ani(ets_vm_init_prealloc_test
    CPP_SOURCES ${PANDA_ETS_PLUGIN_SOURCE}/tests/runtime/ets_vm_init_prealloc_test.cpp
    ETS_SOURCES ${PANDA_ETS_PLUGIN_SOURCE}/tests/runtime/ets_vm_init_prealloc_test.ets
    INCLUDE_DIRS ${PANDA_ETS_PLUGIN_SOURCE}/runtime
    LIBRARIES arkbase arkfile arkruntime
    TEST_GROUP ets_tests_runtime_gtests
)

add_subdirectory(types)
add_subdirectory(tooling)
