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

add_custom_target(ets_test_suite_core)

function(core_copy_sources SOURCE_FILES DESTINATION_DIR OUT_FILES)
    set(copied_files)
    foreach(source_file ${SOURCE_FILES})
        get_filename_component(source_name "${source_file}" NAME)
        configure_file("${source_file}" "${DESTINATION_DIR}/${source_name}" COPYONLY)
        list(APPEND copied_files "${DESTINATION_DIR}/${source_name}")
    endforeach()
    set(${OUT_FILES} ${copied_files} PARENT_SCOPE)
endfunction()

function(core_prepare_evolution_test TEST_NAME TEST_SOURCE_DIR TEST_OUT_DIR OUT_MAIN_CFG OUT_OLD_ABCS OUT_NEW_ABCS OUT_PREP_TARGET)
    file(GLOB old_sources LIST_DIRECTORIES false "${TEST_SOURCE_DIR}/old/*.ets")
    file(GLOB new_sources LIST_DIRECTORIES false "${TEST_SOURCE_DIR}/new/*.ets")

    if (NOT old_sources OR NOT new_sources)
        set(${OUT_MAIN_CFG} "" PARENT_SCOPE)
        set(${OUT_OLD_ABCS} "" PARENT_SCOPE)
        set(${OUT_NEW_ABCS} "" PARENT_SCOPE)
        set(${OUT_PREP_TARGET} "" PARENT_SCOPE)
        return()
    endif()

    set(old_dir "${TEST_OUT_DIR}/old")
    set(new_dir "${TEST_OUT_DIR}/new")
    set(entry_dir "${TEST_OUT_DIR}/entry")
    file(MAKE_DIRECTORY "${old_dir}" "${new_dir}" "${entry_dir}")

    core_copy_sources("${old_sources}" "${old_dir}" copied_old_sources)
    core_copy_sources("${new_sources}" "${new_dir}" copied_new_sources)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig_dep.json" "${old_dir}/arktsconfig.json" COPYONLY)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig_dep.json" "${new_dir}/arktsconfig.json" COPYONLY)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig_entry.json" "${entry_dir}/arktsconfig.json" COPYONLY)

    set(old_target "${TEST_NAME}-old-abc")
    set(new_target "${TEST_NAME}-new-abc")
    compile_arktsconfig_unit(${old_target} old_abcs "${TEST_OUT_DIR}" "${old_dir}/arktsconfig.json" "${copied_old_sources}")
    compile_arktsconfig_unit(${new_target} new_abcs "${TEST_OUT_DIR}" "${new_dir}/arktsconfig.json" "${copied_new_sources}")

    add_custom_target(${TEST_NAME}-prepare DEPENDS ${old_target} ${new_target})

    set(${OUT_MAIN_CFG} "${entry_dir}/arktsconfig.json" PARENT_SCOPE)
    set(${OUT_OLD_ABCS} "${old_abcs}" PARENT_SCOPE)
    set(${OUT_NEW_ABCS} "${new_abcs}" PARENT_SCOPE)
    set(${OUT_PREP_TARGET} "${TEST_NAME}-prepare" PARENT_SCOPE)
endfunction()

function(add_corert_test test_name test_file)
    get_filename_component(test_dir_name "${test_file}" DIRECTORY)
    get_filename_component(entry_name "${test_file}" NAME_WE)
    set(test_source_dir "${CMAKE_CURRENT_SOURCE_DIR}/${test_dir_name}")
    set(test_source "${CMAKE_CURRENT_SOURCE_DIR}/${test_file}")
    set(test_out_dir "${CMAKE_CURRENT_BINARY_DIR}/${test_name}")
    set(test_target ets_test_suite_core_${test_name})
    set(compile_target ${test_target}-ets-es2panda)
    set(entry_dir "${test_out_dir}/entry")
    file(MAKE_DIRECTORY "${test_out_dir}" "${entry_dir}")

    file(GLOB scenario_sources LIST_DIRECTORIES false "${test_source_dir}/*.ets")
    list(REMOVE_ITEM scenario_sources "${test_source}")
    list(PREPEND scenario_sources "${test_source}")
    core_copy_sources("${scenario_sources}" "${entry_dir}" copied_scenario_sources)

    set(main_cfg)
    set(old_panda_files)
    set(new_panda_files)
    set(prepare_target)
    core_prepare_evolution_test(${test_name} "${test_source_dir}" "${test_out_dir}"
                                     main_cfg old_panda_files new_panda_files prepare_target)
    if (NOT main_cfg)
        set(main_cfg "${entry_dir}/arktsconfig.json")
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig_default.json" "${main_cfg}" COPYONLY)
    endif()

    compile_arktsconfig_unit(${compile_target} test_abcs "${test_out_dir}" "${main_cfg}" "${copied_scenario_sources}")
    if (prepare_target)
        add_dependencies(${compile_target} ${prepare_target})
    endif()

    list(GET test_abcs 0 test_abc)
    set(panda_file_list)
    if (new_panda_files)
        list(APPEND panda_file_list ${new_panda_files})
    endif()
    if (old_panda_files)
        list(APPEND panda_file_list ${old_panda_files})
    endif()
    list(APPEND panda_file_list ${test_abcs})
    string(REPLACE ";" ":" panda_files "${panda_file_list}")

    set(runtime_entrypoint "@entry.${entry_name}.ETSGLOBAL::main")

    add_custom_target(${test_target}
        COMMAND ${PANDA_RUN_PREFIX} $<TARGET_FILE:ark>
            --boot-panda-files=${BOOT_PANDA_FILES}
            --panda-files=${panda_files}
            --load-runtimes=ets
            --compiler-enable-jit=false
            --verification-mode=ahead-of-time
            ${test_abc}
            "${runtime_entrypoint}"
        DEPENDS ark etsstdlib ${compile_target} ${prepare_target}
    )

    add_dependencies(ets_test_suite_core ${test_target})
endfunction()

# NOTE(vpukhov): commented tests need structural rewriting because of frontend CTEs
# add_corert_test(mr_01 methodres01/entry.ets)
# add_corert_test(mr_02 methodres02/entry.ets)
# add_corert_test(mr_03 methodres03/entry.ets)
add_corert_test(mr_04 methodres04/entry.ets)
# add_corert_test(mr_05 methodres05/entry.ets)
# add_corert_test(mr_06 methodres06/entry.ets)
# add_corert_test(mr_07 methodres07/entry.ets)
# add_corert_test(mr_08 methodres08/entry.ets)
# add_corert_test(mr_09 methodres09/entry.ets)
add_corert_test(mr_10 methodres10/entry.ets)
# add_corert_test(mr_11 methodres11/entry.ets)
# add_corert_test(mr_12 methodres12/entry.ets)
# add_corert_test(mr_13 methodres13/entry.ets)
# add_corert_test(mr_14 methodres14/entry.ets)
# add_corert_test(mr_15 methodres15/entry.ets)
# add_corert_test(mr_16 methodres16/entry.ets)
# add_corert_test(mr_17 methodres17/entry.ets)
# add_corert_test(mr_18 methodres18/entry.ets)
# add_corert_test(mr_19 methodres19/entry.ets)
add_corert_test(mr_20 methodres20/entry.ets)
# add_corert_test(mr_21 methodres21/entry.ets)
# add_corert_test(mr_22 methodres22/entry.ets)
# add_corert_test(mr_23 methodres23/entry.ets)
# add_corert_test(mr_24 methodres24/entry.ets)
# add_corert_test(mr_25 methodres25/entry.ets)
add_corert_test(mr_26 methodres26/entry.ets)
# add_corert_test(mr_27 methodres27/entry.ets)
# add_corert_test(mr_28 methodres28/entry.ets)
# add_corert_test(mr_29 methodres29/entry.ets)
# add_corert_test(mr_30 methodres30/entry.ets)
# add_corert_test(mr_31 methodres31/entry.ets)
# add_corert_test(mr_32 methodres32/entry.ets)
# add_corert_test(mr_33 methodres33/entry.ets)
add_corert_test(mr_34 methodres34/entry.ets)
add_corert_test(mr_35 methodres35/entry.ets)
# add_corert_test(mr_36 methodres36/entry.ets)
# add_corert_test(mr_37 methodres37/entry.ets)
add_corert_test(mr_38 methodres38/entry.ets)
# add_corert_test(mr_39 methodres39/entry.ets)
# add_corert_test(mr_40 methodres40/entry.ets)
add_corert_test(mr_41 methodres41/entry.ets)
# add_corert_test(mr_42 methodres42/entry.ets)
add_corert_test(mr_43 methodres43/entry.ets)
add_corert_test(mr_44 methodres44/entry.ets)
add_corert_test(mr_45 methodres45/entry.ets)
# add_corert_test(mr_46 methodres46/entry.ets)
