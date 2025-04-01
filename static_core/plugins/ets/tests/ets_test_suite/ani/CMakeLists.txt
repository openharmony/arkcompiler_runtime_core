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

# Build rules for example_lib native
panda_add_library(example_lib SHARED example_lib.cpp)
panda_target_link_libraries(example_lib arkruntime gtest)
panda_target_include_directories(example_lib
    PRIVATE ${PANDA_ETS_PLUGIN_SOURCE}/runtime
    PRIVATE ${PANDA_ROOT}
)

run_int_jit_aot_ets_code("${CMAKE_CURRENT_SOURCE_DIR}/example_lib.ets"
    "${CMAKE_CURRENT_BINARY_DIR}"
    ets_test_ani_example_lib)

add_dependencies(ets_test_ani_example_lib example_lib)
add_dependencies(ets_test_ani_example_lib-ets-int-cpp example_lib)
add_dependencies(ets_test_ani_example_lib-ets-int-irtoc example_lib)
if (PANDA_LLVM_AOT)
    add_dependencies(ets_test_ani_example_lib-ets-llvmaot example_lib)
endif()
if (ets_test_ani_example_lib-ets-aot)
    add_dependencies(ets_test_ani_example_lib-ets-aot example_lib)
endif()
if (ets_test_ani_example_lib-ets-jit )
    add_dependencies(ets_test_ani_example_lib-ets-jit example_lib)
endif()
