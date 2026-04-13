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
if (PANDA_TARGET_ARM32)
    return()
endif()

file(REAL_PATH "${CMAKE_CURRENT_SOURCE_DIR}" CURRENT_SOURCE_DIR)
file(REAL_PATH "${CMAKE_CURRENT_BINARY_DIR}" CURRENT_BINARY_DIR)
file(REAL_PATH "${CMAKE_BINARY_DIR}" CMAKE_BINARY_ROOT)

set(DEFAULT_CACHE_DIR "${CMAKE_BINARY_ROOT}/plugins/ets/stdlib/decls")
file(MAKE_DIRECTORY "${DEFAULT_CACHE_DIR}")

add_custom_target(use_simultaneous_mode)

set(PACKAGE_CONFIG ${CURRENT_BINARY_DIR}/arktsconfig_package.json)
configure_file(${CURRENT_SOURCE_DIR}/lib/arktsconfig.in.json ${PACKAGE_CONFIG})

add_custom_target(use_simultaneous_mode_z COMMENT "Generating interface library declaration"
    COMMAND ${PANDA_BINARY_ROOT}/bin/es2panda ${CMAKE_CURRENT_SOURCE_DIR}/lib/z.ets
    --arktsconfig=${PACKAGE_CONFIG}
    --emit-declaration
    --opt-level=2 --extension=ets --gen-stdlib=false --output=${CMAKE_CURRENT_BINARY_DIR}/z.abc
    DEPENDS es2panda etsstdlib)

set(ENTRY_CONFIG ${CURRENT_BINARY_DIR}/arktsconfig_entry.json)
configure_file(${CURRENT_SOURCE_DIR}/arktsconfig.in.json ${ENTRY_CONFIG})

add_custom_target(use_simultaneous_mode_x1 COMMENT "Using interface literal in dir1/x.ets"
    COMMAND ${PANDA_BINARY_ROOT}/bin/es2panda --opt-level=2 --extension=ets --gen-stdlib=false
    --arktsconfig=${ENTRY_CONFIG} --emit-declaration --simultaneous=true
    --output=${CMAKE_CURRENT_BINARY_DIR}/x1.abc ${CMAKE_CURRENT_SOURCE_DIR}/dir1/x.ets
    DEPENDS es2panda etsstdlib)

add_custom_target(use_simultaneous_mode_x2 COMMENT "Using interface literal in dir1/dir2/x.ets"
    COMMAND ${PANDA_BINARY_ROOT}/bin/es2panda --opt-level=2 --extension=ets --gen-stdlib=false
    --arktsconfig=${ENTRY_CONFIG} --emit-declaration --simultaneous=true
    --output=${CMAKE_CURRENT_BINARY_DIR}/x2.abc ${CMAKE_CURRENT_SOURCE_DIR}/dir1/dir2/x.ets
    DEPENDS es2panda etsstdlib)

add_custom_target(use_simultaneous_mode_link COMMENT "Using interface literal in different sources"
    COMMAND ${PANDA_BINARY_ROOT}/bin/ark_link --output ${CMAKE_CURRENT_BINARY_DIR}/target.abc
    -- ${CMAKE_CURRENT_BINARY_DIR}/x1.abc ${CMAKE_CURRENT_BINARY_DIR}/x2.abc ${CMAKE_CURRENT_BINARY_DIR}/z.abc
    DEPENDS ark_link)

add_custom_target(use_simultaneous_mode_exec1 COMMENT "Check literal usage from the first file"
    COMMAND ${PANDA_BINARY_ROOT}/bin/ark --load-runtimes=ets
    --boot-panda-files=${CMAKE_BINARY_ROOT}/plugins/ets/etsstdlib.abc
    ${CMAKE_CURRENT_BINARY_DIR}/target.abc @panda.entry.dir1.x.ETSGLOBAL::main
    DEPENDS ark)

add_custom_target(use_simultaneous_mode_exec2 COMMENT "Check literal usage from the second file"
    COMMAND ${PANDA_BINARY_ROOT}/bin/ark --load-runtimes=ets
    --boot-panda-files=${CMAKE_BINARY_ROOT}/plugins/ets/etsstdlib.abc
    ${CMAKE_CURRENT_BINARY_DIR}/target.abc @panda.entry.dir1.dir2.x.ETSGLOBAL::main
    DEPENDS ark)

add_dependencies(use_simultaneous_mode_x1 use_simultaneous_mode_z)
add_dependencies(use_simultaneous_mode_x2 use_simultaneous_mode_z)
add_dependencies(use_simultaneous_mode_link use_simultaneous_mode_x1 use_simultaneous_mode_x2)
add_dependencies(use_simultaneous_mode_exec1 use_simultaneous_mode_link)
add_dependencies(use_simultaneous_mode_exec2 use_simultaneous_mode_link)
add_dependencies(use_simultaneous_mode use_simultaneous_mode_exec1 use_simultaneous_mode_exec2)
add_dependencies(ets_gtests use_simultaneous_mode)
