# Copyright (c) 2026 Huawei Device Co., Ltd.
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

set(HEAP_DUMP_TEST_LIBRARIES
    arkruntime
    arkassembler
    arkdisassembler
)

set(HEAP_DUMP_TEST_DEPENDENCIES
    arkstdlib
    ark_aot
)

panda_ets_add_gtest(
    NO_CORES
    NAME arkruntime_heap_dump_session_test
    SOURCES
        session/abstract_writer_test.cpp
        session/common_writer_test.cpp
        session/heap_dump_coordinator_test.cpp
        session/object_id_map_test.cpp
        session/output_stream_test.cpp
        session/string_id_pool_test.cpp
        session/test_common.cpp
    LIBRARIES ${HEAP_DUMP_TEST_LIBRARIES}
    INCLUDE_DIRS ${PANDA_ROOT} ${GEN_INCLUDE_DIR}
    SANITIZERS ${PANDA_SANITIZERS_LIST}
    PANDA_STD_LIB $<TARGET_PROPERTY:arkstdlib,FILE>
    DEPS_TARGETS ${HEAP_DUMP_TEST_DEPENDENCIES}
    TEST_GROUP ets_tests_runtime_gtests
)

panda_ets_add_gtest(
    NO_CORES
    NAME arkruntime_static_dump_test
    SOURCES static_dump_test.cpp
    LIBRARIES ${HEAP_DUMP_TEST_LIBRARIES}
    INCLUDE_DIRS ${PANDA_ROOT} ${GEN_INCLUDE_DIR}
    SANITIZERS ${PANDA_SANITIZERS_LIST}
    PANDA_STD_LIB $<TARGET_PROPERTY:arkstdlib,FILE>
    DEPS_TARGETS ${HEAP_DUMP_TEST_DEPENDENCIES}
    TEST_GROUP ets_tests_runtime_gtests
)
