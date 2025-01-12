# Copyright (c) 2024 Huawei Device Co., Ltd.
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

function(compile_arktsconfig_unit TARGET ABC_FILES WORK_DIR ARKTS_CONFIG ETS_SOURCES)
    set(oneValueArgs OPT_LEVEL)
    set(multiValueArgs )
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    compile_ets_sources(${WORK_DIR} ${TARGET} RESULT "${ETS_SOURCES}"
                        ARKTS_CONFIG ${ARKTS_CONFIG}
                        OPT_LEVEL ${ARG_OPT_LEVEL}
    )
    set(${ABC_FILES} ${RESULT} PARENT_SCOPE)
endfunction()

function(run_sts_app_mode TARGET WORK_DIR ABC_FILES)
    set(oneValueArgs ENTRYPOINT)
    set(multiValueArgs RUNTIME_EXTRA_OPTIONS)
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    list(GET ABC_FILES 0 ENTRY_ABC)
    string(REPLACE ";" ":" ABC_FILES "${ABC_FILES}")

    set(RUNTIME_ARGUMENTS
        --boot-panda-files=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc
        --panda-files=${ABC_FILES}
        --load-runtimes=ets
        --gc-type=g1-gc
        ${ARG_RUNTIME_EXTRA_OPTIONS}
        ${ENTRY_ABC}
        "${ARG_ENTRYPOINT}.ETSGLOBAL::main"
    )

    add_custom_target(${TARGET}
        COMMAND ${PANDA_RUN_PREFIX} $<TARGET_FILE:ark> ${RUNTIME_ARGUMENTS}
    )
endfunction()

function(create_modules_config_test TARGET WORK_DIR SOURCE_DIR)
    set(ABC_FILES "")

    set(SOURCES_APP ${SOURCE_DIR}/app/action.sts)
    compile_arktsconfig_unit(${TARGET}-app-abc ABCS_APP ${WORK_DIR} ${SOURCE_DIR}/app/arktsconfig.json
                             "${SOURCES_APP}"
    )
    set(ABC_FILES ${ABC_FILES} ${ABCS_APP})
    
    set(SOURCES_FOO ${SOURCE_DIR}/foo/some.sts;${SOURCE_DIR}/foo/thing.sts)
    compile_arktsconfig_unit(${TARGET}-foo-abc ABCS_FOO ${WORK_DIR} ${SOURCE_DIR}/app/arktsconfig.json
                             "${SOURCES_FOO}"
    )
    set(ABC_FILES ${ABC_FILES} ${ABCS_FOO})
    
    run_sts_app_mode(${TARGET} ${WORK_DIR} "${ABC_FILES}" ENTRYPOINT "@app.action")

    add_dependencies(${TARGET} ark etsstdlib ${TARGET}-app-abc ${TARGET}-foo-abc)
endfunction()

create_modules_config_test(modules-config ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
