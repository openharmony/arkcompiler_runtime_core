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

file(REAL_PATH "${ES2PANDA_PATH}" URUNNER_ES2PANDA_REALPATH)
get_filename_component(URUNNER_ETS_FRONTEND_PATH "${URUNNER_ES2PANDA_REALPATH}" DIRECTORY)
get_filename_component(URUNNER_RUNTIME_CORE_PATH "${PANDA_ROOT}" DIRECTORY)

set(URUNNER_ENV_FILE "${PANDA_BINARY_ROOT}/.urunner.env")
set(URUNNER_WORK_DIR "${PANDA_BINARY_ROOT}/urunner-work")
set(URUNNER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/runner.sh")
set(URUNNER_BUILD_WRAPPER "${PANDA_BINARY_ROOT}/runner.sh")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/.urunner.env.in"
    "${URUNNER_ENV_FILE}"
    @ONLY
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/runner.sh.in"
    "${URUNNER_BUILD_WRAPPER}"
    @ONLY
    NEWLINE_STYLE UNIX
)

file(CHMOD "${URUNNER_BUILD_WRAPPER}"
    PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
)

# NOTE: URunner targets are intended for local developer workflows only.
# Do not add them under umbrella `tests` or CI-oriented target chains.
function(add_urunner_target TARGET_NAME WORKFLOW_NAME SUITE_NAME)
    add_custom_target(${TARGET_NAME}
        COMMAND ${CMAKE_COMMAND} -E chdir "${PANDA_BINARY_ROOT}"
                "${URUNNER_BUILD_WRAPPER}" "${WORKFLOW_NAME}" "${SUITE_NAME}"
        COMMENT "Running URunner suite ${SUITE_NAME} with workflow ${WORKFLOW_NAME}"
        USES_TERMINAL
    )

    if(WORKFLOW_NAME STREQUAL "panda-int")
        add_dependencies(${TARGET_NAME} es2panda ark verifier)
    elseif(WORKFLOW_NAME STREQUAL "es2panda-verifier")
        add_dependencies(${TARGET_NAME} es2panda verifier)
    else()
        # Add new workflows into if-else on demand
        message(FATAL_ERROR "Unsupported URunner workflow '${WORKFLOW_NAME}' for target ${TARGET_NAME}")
    endif()
endfunction()
