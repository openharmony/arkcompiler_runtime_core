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

set(work_dir "${CMAKE_CURRENT_BINARY_DIR}")
set(gen_lib_dir "${work_dir}/lib")

panda_add_library(unsafe_lib SHARED null_safety_test.cpp)
set_target_properties(unsafe_lib PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${gen_lib_dir})
panda_target_link_libraries(unsafe_lib)
panda_target_include_directories(unsafe_lib
   PRIVATE ${PANDA_ETS_PLUGIN_SOURCE}/runtime/ani
)

compile_ets_sources(${work_dir}
    additional_abc_target
    additional_abc_file
    "${CMAKE_CURRENT_SOURCE_DIR}/AdditionalFile.ets")
add_custom_target(additional_dependencies DEPENDS unsafe_lib additional_abc_target)

set(PANDA_RUN_PREFIX
    LD_LIBRARY_PATH=${gen_lib_dir}
    ADDITIONAL_ABC_FILE=${additional_abc_file}
    ${PANDA_RUN_PREFIX})

run_int_jit_aot_ets_code("${CMAKE_CURRENT_SOURCE_DIR}/NullSafetyTest.ets"
    "${work_dir}"
    ets_api_null_safety_test)

add_dependencies(ets_api_null_safety_test additional_dependencies)
add_dependencies(ets_api_null_safety_test-ets-int-cpp additional_dependencies)
add_dependencies(ets_api_null_safety_test-ets-int-irtoc additional_dependencies)
if (TARGET ets_api_null_safety_test-ets-llvmaot)
    add_dependencies(ets_api_null_safety_test-ets-llvmaot additional_dependencies)
endif()
if (TARGET ets_api_null_safety_test-ets-int-llvmirtoc)
    add_dependencies(ets_api_null_safety_test-ets-int-llvmirtoc additional_dependencies)
endif()
if (TARGET ets_api_null_safety_test-ets-aot)
    add_dependencies(ets_api_null_safety_test-ets-aot additional_dependencies)
endif()
if (TARGET ets_api_null_safety_test-ets-jit)
    add_dependencies(ets_api_null_safety_test-ets-jit additional_dependencies)
endif()
