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

cmake_minimum_required(VERSION 3.10)

project(ani_hotreload_app)

panda_add_executable(ani_hotreload_app main.cpp)

panda_target_link_libraries(ani_hotreload_app arkruntime)

panda_add_sanitizers(TARGET ani_hotreload_app SANITIZERS ${PANDA_SANITIZERS_LIST})

# Target and patch must produce identical module records, so both sources are `t.ets`
# in separate directories.
compile_ets_code(${CMAKE_CURRENT_SOURCE_DIR}/target/t.ets ${CMAKE_CURRENT_BINARY_DIR}/target/t.abc
                 hotreload_app_target)
compile_ets_code(${CMAKE_CURRENT_SOURCE_DIR}/patch/t.ets ${CMAKE_CURRENT_BINARY_DIR}/patch/t.abc
                 hotreload_app_patch)

# NOTE: enable with asan after fix, same as ani_app
if (NOT PANDA_ENABLE_ADDRESS_SANITIZER)
    # Create a custom target to run ani_hotreload_app after build
    add_custom_target(run_ani_hotreload_app
        COMMAND ARK_ETS_STDLIB_PATH=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc
                ANI_HOTRELOAD_TARGET_ABC=${CMAKE_CURRENT_BINARY_DIR}/target/t.abc
                ANI_HOTRELOAD_PATCH_ABC=${CMAKE_CURRENT_BINARY_DIR}/patch/t.abc
                LD_LIBRARY_PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
            ${PANDA_RUN_PREFIX} $<TARGET_FILE:ani_hotreload_app>
        DEPENDS ani_hotreload_app hotreload_app_target hotreload_app_patch
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running ani_hotreload_app"
    )

    # Add the run target as a dependency of the main test target
    add_dependencies(ani_tests run_ani_hotreload_app)
endif()
