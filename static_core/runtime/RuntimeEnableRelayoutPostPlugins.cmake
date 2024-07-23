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

enable_language(ASM)

if(PANDA_WITH_TESTS)
    get_target_property(ALL_ARK_SOURCES_VAR_STATIC arkruntime_interpreter_static SOURCES)
    get_target_property(ALL_ARK_SOURCES_VAR_OBJ arkruntime_obj SOURCES)
    set(ALL_ARK_SOURCES_VAR ${ALL_ARK_SOURCES_VAR_STATIC} ${ALL_ARK_SOURCES_VAR_OBJ})
    get_target_property(ALL_ARK_INCLUDE_DIRECTORIES_VAR arkruntime_obj INCLUDE_DIRECTORIES)
    get_target_property(ALL_ARK_LINK_LIBRARIES_VAR arkruntime_obj LINK_LIBRARIES)

    set(ARK_ABSOLUTE_PATH_SOURCE)
    foreach(SRC ${ALL_ARK_SOURCES_VAR})
        string(FIND ${SRC} "TARGET_OBJECTS" RESULT)
        if("${RESULT}" STREQUAL "-1" AND NOT IS_ABSOLUTE ${SRC})
            set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/runtime/${SRC})
        endif()
        string(FIND ${SRC} "irtoc" RESULT)
        if(NOT "${RESULT}" STREQUAL "-1")
            SET_SOURCE_FILES_PROPERTIES(${SRC} PROPERTIES
                EXTERNAL_OBJECT true
                GENERATED true
            )
        endif()
        list(APPEND ARK_ABSOLUTE_PATH_SOURCE ${SRC})
    endforeach()

    panda_add_library(arkruntime_for_relayout_static STATIC ${ARK_ABSOLUTE_PATH_SOURCE})
    panda_target_include_directories(arkruntime_for_relayout_static PUBLIC ${ALL_ARK_INCLUDE_DIRECTORIES_VAR})
    panda_target_compile_definitions(arkruntime_for_relayout_static PUBLIC PANDA_ENABLE_RELAYOUT_PROFILE)
    set_source_files_properties(${ARK_ABSOLUTE_PATH_SOURCE} PROPERTIES COMPILE_FLAGS "-Wno-shadow -Wno-unused-parameter")

    add_dependencies(arkruntime_for_relayout_static
        intrinsics_gen_arkruntime
        runtime_intrinsics_inl
        isa_gen_pandaverification
        messages_data_pandaverification
        messages_gen_pandaverification
        verifier_plugin_gen
        arkstdlib
        asm_defines_generator
        entrypoints_gen
        irtoc_fastpath
        irtoc_interpreter
        ${irtoc_target}
    )

    foreach(plugin ${PLUGINS})
        string(TOUPPER ${plugin} plugin_name_upper)
        string(CONCAT PANDA_WITH_PLUGIN "PANDA_WITH_" ${plugin_name_upper})

        if(${${PANDA_WITH_PLUGIN}})
            string(CONCAT PLUGIN_SOURCE "PANDA_" ${plugin_name_upper} "_PLUGIN_SOURCE")
            if(EXISTS ${${PLUGIN_SOURCE}}/RuntimeEnableRelayoutPostPlugins.cmake)
                include(${${PLUGIN_SOURCE}}/RuntimeEnableRelayoutPostPlugins.cmake)
            endif()
        endif()
    endforeach()

    if(PANDA_TARGET_ARM32_ABI_SOFT OR PANDA_TARGET_ARM32_ABI_SOFTFP)
        add_dependencies(arkruntime_for_relayout_static bridge_dispatch_arm bridge_dispatch_dyn_arm)
    elseif (PANDA_TARGET_ARM32_ABI_HARD)
        add_dependencies(arkruntime_for_relayout_static bridge_dispatch_armhf bridge_dispatch_dyn_arm)
    elseif(PANDA_TARGET_ARM64)
        add_dependencies(arkruntime_for_relayout_static bridge_dispatch_aarch64 bridge_dispatch_dyn_aarch64)
        panda_target_compile_options(arkruntime_for_relayout_static PUBLIC "-march=armv8-a+crc")
    elseif(PANDA_TARGET_X86)
        add_dependencies(arkruntime_for_relayout_static bridge_dispatch_x86)
    elseif(PANDA_TARGET_AMD64)
        add_dependencies(arkruntime_for_relayout_static bridge_dispatch_amd64 bridge_dispatch_dyn_amd64)
    endif()

    panda_target_link_libraries(arkruntime_for_relayout_static ${ALL_ARK_LINK_LIBRARIES_VAR})
    set_property(TARGET arkruntime_for_relayout_static PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()
