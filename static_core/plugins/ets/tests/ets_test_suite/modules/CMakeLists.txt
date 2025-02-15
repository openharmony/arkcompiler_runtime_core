# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

function(build_spawn_native_module TARGET OUTPUT_DIR)
    panda_add_library(${TARGET} SHARED spawn/spawn.cpp)
    set_target_properties(${TARGET} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
    panda_target_link_libraries(${TARGET} arkruntime)
    panda_target_include_directories(${TARGET} PRIVATE ${PANDA_ETS_PLUGIN_SOURCE}/runtime/napi)
endfunction()

function(build_spawn_managed_code TARGET ABC_FILES WORK_DIR)
    set(oneValueArgs OPT_LEVEL)
    set(multiValueArgs )
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(SPAWN_ETS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/spawn/spawn.sts;
        ${CMAKE_CURRENT_SOURCE_DIR}/spawn/ability.sts;
    )
    compile_arktsconfig_unit(${TARGET} RESULT ${WORK_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/spawn/arktsconfig.json
                             "${SPAWN_ETS_SOURCES}"
                             OPT_LEVEL ${ARG_OPT_LEVEL}
    )
    set(${ABC_FILES} "${RESULT}" PARENT_SCOPE)
endfunction()

function(prepare_spawn TARGET ABC_FILES WORK_DIR)
    # Build Spawn native module
    set(TARGET_LIB ${TARGET}-lib)
    set(ETSNAPI_LIB_DIR "${WORK_DIR}/lib")
    file(MAKE_DIRECTORY ${ETSNAPI_LIB_DIR})
    build_spawn_native_module(${TARGET_LIB} ${ETSNAPI_LIB_DIR})

    # Build Spawn managed code. Produced abc files will be loaded into boot context
    set(TARGET_ABC ${TARGET}-abc)
    build_spawn_managed_code(${TARGET_ABC} RESULT ${WORK_DIR})
    set(${ABC_FILES} "${RESULT}" PARENT_SCOPE)

    # Add env variables required for spawn native module
    set(PANDA_RUN_PREFIX
        LD_LIBRARY_PATH=${ETSNAPI_LIB_DIR}
        NATIVE_LIB_NAME=${TARGET_LIB}
        ${PANDA_RUN_PREFIX}
        PARENT_SCOPE)

    add_custom_target(${TARGET} DEPENDS ${TARGET_LIB} ${TARGET_ABC})
endfunction()

function(run_sts_app_mode TARGET WORK_DIR APP_MAIN_ABILITY_CLASS APP_ABC_FILES)
    set(oneValueArgs STDOUT_FILE SPAWN_ENTRYPOINT_FUNCTION)
    set(multiValueArgs RUNTIME_EXTRA_OPTIONS)
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    prepare_spawn(${TARGET}-spawn SPAWN_ABC_FILES ${WORK_DIR})
    list(GET SPAWN_ABC_FILES 0 SPAWN_ENTRY_ABC)
    list(JOIN SPAWN_ABC_FILES ":" SPAWN_ABC_FILES)

    list(JOIN APP_ABC_FILES ":" APP_ABC_FILES)
    set(PANDA_RUN_PREFIX
        APP_ABC_FILES=${APP_ABC_FILES}
        APP_MAIN_ABILITY_CLASS=${APP_MAIN_ABILITY_CLASS}
        ${PANDA_RUN_PREFIX})

    if (NOT DEFINED ARG_SPAWN_ENTRYPOINT_FUNCTION)
        set(ARG_SPAWN_ENTRYPOINT_FUNCTION "main")
    endif()

    set(RUNTIME_ARGUMENTS
        --boot-panda-files=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc:${SPAWN_ABC_FILES}
        --load-runtimes=ets
        --verification-enabled=true
        --verification-mode=on-the-fly
        --gc-type=g1-gc
        ${ARG_RUNTIME_EXTRA_OPTIONS}
        ${SPAWN_ENTRY_ABC}
        "@spawn.spawn.ETSGLOBAL::${ARG_SPAWN_ENTRYPOINT_FUNCTION}"
    )

    if (DEFINED ARG_STDOUT_FILE)
        set(RUNTIME_ARGUMENTS ${RUNTIME_ARGUMENTS} 1> ${ARG_STDOUT_FILE})
    endif()

    add_custom_target(${TARGET}
        COMMAND ${PANDA_RUN_PREFIX} $<TARGET_FILE:ark> ${RUNTIME_ARGUMENTS}
        DEPENDS ${TARGET}-spawn
    )
endfunction()

function(prepare_app TARGET ABC_FILES WORK_DIR SOURCE_DIR)
    # All application files
    set(RESULT "")

    # `app` package
    set(SOURCES_APP_DIR ${SOURCE_DIR}/app)
    set(SOURCES_APP
        ${SOURCES_APP_DIR}/module1.sts
        ${SOURCES_APP_DIR}/module2.sts
        ${SOURCES_APP_DIR}/module3.sts
        ${SOURCES_APP_DIR}/module4.sts
        ${SOURCES_APP_DIR}/module5.sts
        ${SOURCES_APP_DIR}/module6.sts
    )
    set(TARGET_APP ${TARGET}-app-abc)
    compile_arktsconfig_unit(${TARGET_APP} ABCS_MODULES ${WORK_DIR} ${SOURCES_APP_DIR}/arktsconfig.json
                             "${SOURCES_APP}"
    )
    set(RESULT ${RESULT} ${ABCS_MODULES})

    # `foo` package
    set(TARGET_FOO ${TARGET}-foo-abc)
    set(SOURCES_FOO
        ${SOURCE_DIR}/foo/base.sts
        ${SOURCE_DIR}/foo/foo.sts
    )
    compile_arktsconfig_unit(${TARGET_FOO} ABCS_FOO ${WORK_DIR} ${SOURCE_DIR}/foo/arktsconfig.json
                             "${SOURCES_FOO}"
    )
    set(RESULT ${RESULT} ${ABCS_FOO})

    # `bar` package
    set(TARGET_BAR ${TARGET}-bar-abc)
    compile_arktsconfig_unit(${TARGET_BAR} ABCS_BAR ${WORK_DIR} ${SOURCE_DIR}/bar/arktsconfig.json
                             ${SOURCE_DIR}/bar/bar.sts
    )
    set(RESULT ${RESULT} ${ABCS_BAR})

    # `mismatched` package
    set(TARGET_MIS_MATCHED ${TARGET}-mismatched-abc)
    set(SOURCES_MIS_MATCHED
        ${SOURCE_DIR}/mismatched/src/extends_final.sts
        ${SOURCE_DIR}/mismatched/src/final.sts
        ${SOURCE_DIR}/mismatched/src/override_final.sts
        ${SOURCE_DIR}/mismatched/src/final_method.sts
    )
    compile_arktsconfig_unit(${TARGET_MIS_MATCHED} ABCS_MIS_MATCHED ${WORK_DIR} ${SOURCE_DIR}/mismatched/src/arktsconfig.json
                             "${SOURCES_MIS_MATCHED}"
    )
    set(RESULT ${RESULT} ${ABCS_MIS_MATCHED})

    # `loop_inheritance` package
    set(TARGET_LOOP ${TARGET}-loop_inheritance-abc)
    set(SOURCES_LOOP
        ${SOURCE_DIR}/loop_inheritance/src/level_one.sts
        ${SOURCE_DIR}/loop_inheritance/src/level_two.sts
    )
    compile_arktsconfig_unit(${TARGET_LOOP} ABCS_LOOP ${WORK_DIR} ${SOURCE_DIR}/loop_inheritance/src/arktsconfig.json
                             "${SOURCES_LOOP}"
    )
    set(RESULT ${RESULT} ${ABCS_LOOP})

    set(${ABC_FILES} "${RESULT}" PARENT_SCOPE)
    add_custom_target(${TARGET} DEPENDS ${TARGET_APP} ${TARGET_FOO} ${TARGET_BAR} ${TARGET_MIS_MATCHED} ${TARGET_LOOP})
endfunction()

function(create_modules_test TARGET WORK_DIR SOURCE_DIR)
    prepare_app(${TARGET}-app ABC_FILES ${WORK_DIR} ${SOURCE_DIR})

    run_sts_app_mode(${TARGET} ${WORK_DIR} "@app.module1.MainAbility" "${ABC_FILES}")

    add_dependencies(${TARGET} ${TARGET}-app)
endfunction()

create_modules_test(ets_modules_app_mode_test ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

function(list_find_by_regex ELEMENTS REGEX FOUND_IDX)
    set(RESULT -1)
    set(IDX 0)
    foreach(ITER ${ELEMENTS})
        if(ITER MATCHES ${REGEX})
            set(RESULT ${IDX})
            break()
        endif()
        math(EXPR IDX "${IDX}+1")
    endforeach()
    set(${FOUND_IDX} ${RESULT} PARENT_SCOPE)
endfunction()

function(create_no_class_def_found_test TARGET WORK_DIR SOURCE_DIR)
    prepare_app(${TARGET}-app ABC_FILES ${WORK_DIR} ${SOURCE_DIR})
    list_find_by_regex("${ABC_FILES}" "(.*)-base.abc" BASE_ABC_IDX)
    # As a result, `@foo.foo.Dummy` class from `foo.sts` must be not loaded
    list(REMOVE_AT ABC_FILES ${BASE_ABC_IDX})

    set(STDOUT_FILE ${WORK_DIR}/${TARGET}-output.txt)
    run_sts_app_mode(${TARGET}-run-sts-app-mode ${WORK_DIR} "@app.module1.MainAbility" "${ABC_FILES}"
                     STDOUT_FILE ${STDOUT_FILE}
                     SPAWN_ENTRYPOINT_FUNCTION "mainCatchLinkerUnresolvedClassError"
    )
    add_dependencies(${TARGET}-run-sts-app-mode ${TARGET}-app)

    add_custom_target(${TARGET}
        COMMENT "Check class not loaded due to LinkerUnresolvedClassError"
        COMMAND ${CMAKE_COMMAND} -E compare_files ${STDOUT_FILE} ${SOURCE_DIR}/incompatible_foo/not_verified_method.txt
        DEPENDS ${TARGET}-run-sts-app-mode
    )
endfunction()

create_no_class_def_found_test(ets_modules_no_class_def_found_test ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

function(create_incompatible_class_change_test TARGET WORK_DIR SOURCE_DIR)
    prepare_app(${TARGET}-app ABC_FILES ${WORK_DIR} ${SOURCE_DIR})
    list_find_by_regex("${ABC_FILES}" "(.*)-base.abc" BASE_ABC_IDX)
    list(REMOVE_AT ABC_FILES ${BASE_ABC_IDX})

    # Take `Base` class from `incompatible-foo` package's file
    set(TARGET_INCOMPATIBLE_FOO ${TARGET}-incompatible-foo-abc)
    compile_arktsconfig_unit(${TARGET_INCOMPATIBLE_FOO} ABCS_FOO ${WORK_DIR} ${SOURCE_DIR}/incompatible_foo/arktsconfig.json
                             ${SOURCE_DIR}/incompatible_foo/base.sts
    )
    set(ABC_FILES ${ABC_FILES} ${ABCS_FOO})

    set(STDOUT_FILE ${WORK_DIR}/${TARGET}-output.txt)
    run_sts_app_mode(${TARGET}-run-sts-app-mode ${WORK_DIR} "@app.module1.MainAbility" "${ABC_FILES}"
                     STDOUT_FILE ${STDOUT_FILE}
                     SPAWN_ENTRYPOINT_FUNCTION "mainCatchLinkerBadSupertypeError")
    add_dependencies(${TARGET}-run-sts-app-mode ${TARGET}-app ${TARGET_INCOMPATIBLE_FOO})

    add_custom_target(${TARGET}
        COMMENT "Check class not loaded due to LinkerBadSupertypeError"
        COMMAND ${CMAKE_COMMAND} -E compare_files ${STDOUT_FILE} ${SOURCE_DIR}/incompatible_foo/not_loaded_class.txt
        DEPENDS ${TARGET}-run-sts-app-mode
    )
endfunction()

create_incompatible_class_change_test(ets_modules_incompatible_class_change_test ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

function(create_mismatched_extends_final_class_test TARGET WORK_DIR SOURCE_DIR)
    prepare_app(${TARGET}-app ABC_FILES ${WORK_DIR} ${SOURCE_DIR})

    # replace `Final` class with incompatible `Final` class
    list_find_by_regex("${ABC_FILES}" "(.*)-final.abc" BASE_ABC_IDX)
    list(REMOVE_AT ABC_FILES ${BASE_ABC_IDX})
    set(TARGET_MIS_MATCHED ${TARGET}-incompatible-extends-final-abc)
    compile_arktsconfig_unit(${TARGET_MIS_MATCHED} ABCS_MIS_MATCHED ${WORK_DIR} ${SOURCE_DIR}/mismatched/incompatible/arktsconfig.json
                             ${SOURCE_DIR}/mismatched/incompatible/final.sts
    )

    set(ABC_FILES ${ABC_FILES} ${ABCS_MIS_MATCHED})
    run_sts_app_mode(${TARGET}-run-sts-app-mode ${WORK_DIR} "@app.module4.MainAbility" "${ABC_FILES}"
                     SPAWN_ENTRYPOINT_FUNCTION "mainMisatchedExtendsFinal")
    add_dependencies(${TARGET}-run-sts-app-mode ${TARGET}-app ${TARGET_MIS_MATCHED})

    add_custom_target(${TARGET}
        COMMENT "Check class extends final class mainMisatchedExtendsFinal"
        DEPENDS ${TARGET}-run-sts-app-mode
    )

endfunction()

create_mismatched_extends_final_class_test(ets_modules_mismatched_extends_final_class_test ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

function(create_mismatched_override_final_class_test TARGET WORK_DIR SOURCE_DIR)
    prepare_app(${TARGET}-app ABC_FILES ${WORK_DIR} ${SOURCE_DIR})

    # replace `FinalMethod` class with incompatible `FinalMethod` class
    list_find_by_regex("${ABC_FILES}" "(.*)-final_method.abc" BASE_ABC_IDX)
    list(REMOVE_AT ABC_FILES ${BASE_ABC_IDX})
    set(TARGET_MIS_MATCHED ${TARGET}-incompatible-override-final-abc)
    compile_arktsconfig_unit(${TARGET_MIS_MATCHED} ABCS_MIS_MATCHED ${WORK_DIR} ${SOURCE_DIR}/mismatched/incompatible/arktsconfig.json
                             ${SOURCE_DIR}/mismatched/incompatible/final_method.sts
    )

    set(ABC_FILES ${ABC_FILES} ${ABCS_MIS_MATCHED})
    run_sts_app_mode(${TARGET}-run-sts-app-mode ${WORK_DIR} "@app.module5.MainAbility" "${ABC_FILES}"
                     SPAWN_ENTRYPOINT_FUNCTION "mainMisatchedOverrideFinal")
    add_dependencies(${TARGET}-run-sts-app-mode ${TARGET}-app ${TARGET_MIS_MATCHED})

    add_custom_target(${TARGET}
        COMMENT "Check class override final method mainMisatchedOverrideFinal"
        DEPENDS ${TARGET}-run-sts-app-mode
    )

endfunction()

create_mismatched_override_final_class_test(ets_modules_mismatched_override_final_class_test ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

function(create_loop_inheritance_class_test TARGET WORK_DIR SOURCE_DIR)
    prepare_app(${TARGET}-app ABC_FILES ${WORK_DIR} ${SOURCE_DIR})

    # replace `LevelOne` class with incompatible `LevelOne` class
    list_find_by_regex("${ABC_FILES}" "(.*)-level_one.abc" BASE_ABC_IDX)
    list(REMOVE_AT ABC_FILES ${BASE_ABC_IDX})
    set(TARGET_LOOP ${TARGET}-incompatible-loop-abc)
    compile_arktsconfig_unit(${TARGET_LOOP} ABCS_LOOP ${WORK_DIR} ${SOURCE_DIR}/loop_inheritance/incompatible/arktsconfig.json
                             ${SOURCE_DIR}/loop_inheritance/incompatible/level_one.sts
    )

    set(ABC_FILES ${ABC_FILES} ${ABCS_LOOP})
    run_sts_app_mode(${TARGET}-run-sts-app-mode ${WORK_DIR} "@app.module6.MainAbility" "${ABC_FILES}"
                     SPAWN_ENTRYPOINT_FUNCTION "mainCircularityExtends")
    add_dependencies(${TARGET}-run-sts-app-mode ${TARGET}-app ${TARGET_LOOP})

    add_custom_target(${TARGET}
        COMMENT "Check class loop inheritance class mainCircularityExtends"
        DEPENDS ${TARGET}-run-sts-app-mode
    )

endfunction()

create_loop_inheritance_class_test(ets_modules_loop_inheritance_class_test ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(ets_modules_tests
                  DEPENDS
                    ets_modules_app_mode_test
                    ets_modules_no_class_def_found_test
                    ets_modules_incompatible_class_change_test
                    ets_modules_mismatched_extends_final_class_test
                    ets_modules_mismatched_override_final_class_test
                    ets_modules_loop_inheritance_class_test)
add_dependencies(ets_tests ets_modules_tests)
