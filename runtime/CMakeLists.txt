# Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

project(arkruntime)

enable_language(ASM)

include(core/Core.cmake)
include(${PANDA_ROOT}/verification/Verification.cmake)

set(SOURCES
    assert_gc_scope.cpp
    bridge/bridge.cpp
    entrypoints/entrypoints.cpp
    exceptions.cpp
    class_linker.cpp
    class_linker_extension.cpp
    class_initializer.cpp
    tooling/debugger.cpp
    tooling/pt_thread.cpp
    tooling/debug_inf.cpp
    field.cpp
    gc_task.cpp
    dprofiler/dprofiler.cpp
    interpreter/interpreter.cpp
    interpreter/runtime_interface.cpp
    intrinsics.cpp
    coretypes/string.cpp
    coretypes/array.cpp
    class.cpp
    class_helper.cpp
    locks.cpp
    panda_vm.cpp
    language_context.cpp
    mem/gc/epsilon/epsilon.cpp
    mem/gc/epsilon/epsilon_barrier.cpp
    mem/gc/gc.cpp
    mem/gc/gc_adaptive_stack.cpp
    mem/gc/gc_settings.cpp
    mem/gc/lang/gc_lang.cpp
    mem/gc/gc_queue.cpp
    mem/gc/gc_root.cpp
    mem/gc/gc_root_type.cpp
    mem/gc/gc_stats.cpp
    mem/gc/gc_trigger.cpp
    mem/gc/gc-hung/gc_hung.cpp
    mem/gc/card_table.cpp
    mem/gc/heap-space-misc/crossing_map.cpp
    mem/gc/heap-space-misc/crossing_map_singleton.cpp
    mem/gc/bitmap.cpp
    mem/gc/gc_scope.cpp
    mem/gc/gc_scoped_phase.cpp
    mem/gc/gc_workers_thread_pool.cpp
    mem/gc/stw-gc/stw-gc.cpp
    mem/gc/gc_barrier_set.cpp
    mem/gc/gen-gc/gen-gc.cpp
    mem/gc/hybrid-gc/hybrid_object_allocator.cpp
    mem/refstorage/ref_block.cpp
    mem/refstorage/reference_storage.cpp
    mem/refstorage/global_object_storage.cpp
    mem/allocator.cpp
    mem/tlab.cpp
    mem/heap_manager.cpp
    mem/heap_verifier.cpp
    mem/rendezvous.cpp
    mem/runslots.cpp
    mem/region_space.cpp
    mem/object_helpers.cpp
    mem/mem_stats_additional_info.cpp
    mem/mem_stats_default.cpp
    mem/mem_stats.cpp
    mem/internal_allocator.cpp
    mem/panda_string.cpp
    mem/memory_manager.cpp
    mem/heap_space.cpp
    methodtrace/trace.cpp
    mark_word.cpp
    method.cpp
    monitor.cpp
    monitor_object_lock.cpp
    monitor_pool.cpp
    global_object_lock.cpp
    object_header.cpp
    runtime.cpp
    runtime_controller.cpp
    string_table.cpp
    thread.cpp
    thread_manager.cpp
    lock_order_graph.cpp
    time_utils.cpp
    timing.cpp
    compiler.cpp
    vtable_builder.cpp
    imtable_builder.cpp
    cframe.cpp
    stack_walker.cpp
    deoptimization.cpp
    object_accessor.cpp
    osr.cpp
    arch/asm_support.cpp
    cha.cpp
    runtime_helpers.cpp
    handle_scope.cpp
    file_manager.cpp
    mem/gc/generational-gc-base.cpp
    mem/gc/g1/g1-gc.cpp
    mem/gc/g1/g1-allocator.cpp
    plugins.cpp
    mem/gc/g1/update_remset_thread.cpp
    relayout_profiler.cpp
    loadable_agent.cpp
)

if(PANDA_TARGET_ARM32_ABI_SOFT OR PANDA_TARGET_ARM32_ABI_SOFTFP)
    list(APPEND SOURCES
        bridge/arch/arm/interpreter_to_compiled_code_bridge_arm.S
        bridge/arch/arm/compiled_code_to_interpreter_bridge_arm.S
        bridge/arch/arm/compiled_code_to_interpreter_bridge_dyn_arm.S
        bridge/arch/arm/interpreter_to_compiled_code_bridge_dyn_arm.S
        bridge/arch/arm/compiled_code_to_runtime_bridge_arm.S
        bridge/arch/arm/deoptimization_arm.S
        arch/arm/osr_arm.S
        arch/arm/interpreter_support.S)
elseif (PANDA_TARGET_ARM32_ABI_HARD)
    list(APPEND SOURCES
        bridge/arch/arm/interpreter_to_compiled_code_bridge_armhf.S
        bridge/arch/arm/compiled_code_to_interpreter_bridge_armhf.S
        bridge/arch/arm/compiled_code_to_interpreter_bridge_dyn_arm.S
        bridge/arch/arm/interpreter_to_compiled_code_bridge_dyn_arm.S
        bridge/arch/arm/compiled_code_to_runtime_bridge_arm.S
        bridge/arch/arm/deoptimization_arm.S
        arch/arm/osr_arm.S
        arch/arm/interpreter_support.S)
elseif(PANDA_TARGET_ARM64)
    list(APPEND SOURCES
        bridge/arch/aarch64/interpreter_to_compiled_code_bridge_aarch64.S
        bridge/arch/aarch64/compiled_code_to_interpreter_bridge_aarch64.S
        bridge/arch/aarch64/compiled_code_to_interpreter_bridge_dyn_aarch64.S
        bridge/arch/aarch64/interpreter_to_compiled_code_bridge_dyn_aarch64.S
        bridge/arch/aarch64/compiled_code_to_runtime_bridge_aarch64.S
        bridge/arch/aarch64/deoptimization_aarch64.S
        arch/aarch64/osr_aarch64.S
        arch/aarch64/interpreter_support.S
        tests/arch/aarch64/stack_walker_test_bridge_aarch64.S)
elseif(PANDA_TARGET_X86)
    list(APPEND SOURCES
        bridge/arch/x86/interpreter_to_compiled_code_bridge_x86.S
        bridge/arch/x86/compiled_code_to_interpreter_bridge_x86.S
        bridge/arch/x86/deoptimization_x86.S
        arch/x86/osr_x86.S
        arch/x86/interpreter_support.S)
elseif(PANDA_TARGET_AMD64)
    list(APPEND SOURCES
        bridge/arch/amd64/compiled_code_to_interpreter_bridge_amd64.S
        bridge/arch/amd64/compiled_code_to_interpreter_bridge_dyn_amd64.S
        bridge/arch/amd64/interpreter_to_compiled_code_bridge_amd64.S
        bridge/arch/amd64/interpreter_to_compiled_code_bridge_dyn_amd64.S
        bridge/arch/amd64/compiled_code_to_runtime_bridge_amd64.S
        bridge/arch/amd64/deoptimization_amd64.S
        arch/amd64/common_amd64.S
        arch/amd64/osr_amd64.S
        arch/amd64/interpreter_support.S)
endif()

if (NOT PANDA_TARGET_MOBILE_WITH_NATIVE_LIBS)
    list(APPEND SOURCES ${PANDA_ROOT}/platforms/common/runtime/thread.cpp)
endif()

if (NOT PANDA_TARGET_MOBILE)
    list(APPEND SOURCES ${PANDA_ROOT}/platforms/common/runtime/verify_app_install.cpp)
endif()

set(INTERPRETER_IMPL_SOURCES
    interpreter/interpreter_impl.cpp
)

add_library(arkruntime_interpreter_impl OBJECT ${INTERPRETER_IMPL_SOURCES})
panda_set_lib_32bit_property(arkruntime_interpreter_impl)

if (PANDA_WITH_ECMASCRIPT)
    set_source_files_properties(${PANDA_ECMASCRIPT_PLUGIN_SOURCE}/runtime/js_locale.cpp PROPERTIES COMPILE_FLAGS -Wno-shadow)
    set_source_files_properties(${PANDA_ECMASCRIPT_PLUGIN_SOURCE}/runtime/js_date_time_format.cpp PROPERTIES COMPILE_FLAGS -Wno-shadow)
    set_source_files_properties(${PANDA_ECMASCRIPT_PLUGIN_SOURCE}/runtime/js_relative_time_format.cpp PROPERTIES COMPILE_FLAGS -Wno-shadow)
    set_source_files_properties(${PANDA_ECMASCRIPT_PLUGIN_SOURCE}/runtime/js_number_format.cpp PROPERTIES COMPILE_FLAGS -Wno-shadow)
    set_source_files_properties(${PANDA_ECMASCRIPT_PLUGIN_SOURCE}/runtime/global_env.cpp PROPERTIES COMPILE_FLAGS -Wno-shadow)
    set_source_files_properties(${PANDA_ECMASCRIPT_PLUGIN_SOURCE}/runtime/ecma_vm.cpp PROPERTIES COMPILE_FLAGS -Wno-shadow)

    set_source_files_properties(${PANDA_ECMASCRIPT_PLUGIN_SOURCE}/runtime/mem/verification.cpp PROPERTIES COMPILE_FLAGS -Wno-shadow)
    set_source_files_properties(${PANDA_ECMASCRIPT_PLUGIN_SOURCE}/runtime/mem/semi_space_collector.cpp PROPERTIES COMPILE_FLAGS -Wno-shadow)

    set_source_files_properties(${PANDA_ECMASCRIPT_PLUGIN_SOURCE}/runtime/snapshot/mem/snapshot_serialize.cpp PROPERTIES COMPILE_FLAGS -Wno-shadow)

    set_source_files_properties(${PANDA_ECMASCRIPT_PLUGIN_SOURCE}/runtime/builtins.cpp PROPERTIES COMPILE_FLAGS -Wno-shadow)
    set_source_files_properties(${PANDA_ECMASCRIPT_PLUGIN_SOURCE}/runtime/builtins/builtins_string.cpp PROPERTIES COMPILE_FLAGS -Wno-shadow)
    set_source_files_properties(${PANDA_ECMASCRIPT_PLUGIN_SOURCE}/runtime/builtins/builtins_locale.cpp PROPERTIES COMPILE_FLAGS -Wno-shadow)
    set_source_files_properties(${PANDA_ECMASCRIPT_PLUGIN_SOURCE}/runtime/builtins/builtins_intl.cpp PROPERTIES COMPILE_FLAGS -Wno-shadow)
    set_source_files_properties(${PANDA_ECMASCRIPT_PLUGIN_SOURCE}/runtime/builtins/builtins_number_format.cpp PROPERTIES COMPILE_FLAGS -Wno-shadow)
    set_source_files_properties(${PANDA_ECMASCRIPT_PLUGIN_SOURCE}/runtime/builtins/builtins_number.cpp PROPERTIES COMPILE_FLAGS -Wno-shadow)
    set_source_files_properties(${PANDA_ECMASCRIPT_PLUGIN_SOURCE}/runtime/builtins/builtins_relative_time_format.cpp PROPERTIES COMPILE_FLAGS -Wno-shadow)
    set_source_files_properties(${PANDA_ECMASCRIPT_PLUGIN_SOURCE}/runtime/builtins/builtins_date.cpp PROPERTIES COMPILE_FLAGS -Wno-shadow)
    set_source_files_properties(${PANDA_ECMASCRIPT_PLUGIN_SOURCE}/runtime/builtins/builtins_date_time_format.cpp PROPERTIES COMPILE_FLAGS -Wno-shadow)
endif()

set(ISA_TEMPLATES
    interpreter-inl_gen.h.erb
    isa_constants_gen.h.erb
    unimplemented_handlers-inl.h.erb
    irtoc_interpreter_utils.h.erb
    debug_test_interpreter-inl_gen.cpp.erb
)

set(GEN_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/include")
file(MAKE_DIRECTORY ${GEN_INCLUDE_DIR})
panda_isa_gen(
    TEMPLATES ${ISA_TEMPLATES}
    SOURCE ${CMAKE_CURRENT_LIST_DIR}/interpreter/templates
    DESTINATION ${GEN_INCLUDE_DIR}
)

set(ISA "${CMAKE_BINARY_DIR}/isa/isa.yaml")
set(ISA_API "${PANDA_ROOT}/isa/isapi.rb")
set(BRIDGE_DISPATCH_TEMPLATE "${CMAKE_CURRENT_LIST_DIR}/templates/bridge_dispatch.S.erb")
set(BRIDGE_DISPATCH_DYN_TEMPLATE "${CMAKE_CURRENT_LIST_DIR}/templates/bridge_dispatch_dyn.S.erb")
set(BRIDGE_ARCHS aarch64 arm armhf amd64 x86)
foreach(arch ${BRIDGE_ARCHS})
    panda_gen_file(
        DATAFILE ${ISA}
        TEMPLATE ${BRIDGE_DISPATCH_TEMPLATE}
        OUTPUTFILE ${GEN_INCLUDE_DIR}/bridge_dispatch_${arch}.S
        REQUIRES ${ISA_API} ${CMAKE_CURRENT_LIST_DIR}/templates/bridge_helpers_${arch}.rb
                            ${CMAKE_CURRENT_LIST_DIR}/templates/bridge_helpers_common.rb
                            ${CMAKE_CURRENT_LIST_DIR}/templates/bridge_helpers_static.rb
    )
    add_custom_target(bridge_dispatch_${arch} DEPENDS ${GEN_INCLUDE_DIR}/bridge_dispatch_${arch}.S ${ISA})
    add_dependencies(bridge_dispatch_${arch} isa_assert)

    panda_gen_file(
        DATAFILE ${ISA}
        TEMPLATE ${BRIDGE_DISPATCH_DYN_TEMPLATE}
        OUTPUTFILE ${GEN_INCLUDE_DIR}/bridge_dispatch_dyn_${arch}.S
        REQUIRES ${ISA_API} ${CMAKE_CURRENT_LIST_DIR}/templates/bridge_helpers_${arch}.rb
                            ${CMAKE_CURRENT_LIST_DIR}/templates/bridge_helpers_common.rb
                            ${CMAKE_CURRENT_LIST_DIR}/templates/bridge_helpers_dynamic.rb
    )
    add_custom_target(bridge_dispatch_dyn_${arch} DEPENDS ${GEN_INCLUDE_DIR}/bridge_dispatch_dyn_${arch}.S ${ISA})
    add_dependencies(bridge_dispatch_${arch} isa_assert)

endforeach()

set(GENERATOR ${PANDA_ROOT}/libpandafile/types.rb)
set(TEMPLATE ${CMAKE_CURRENT_LIST_DIR}/templates/shorty_values.h.erb)
set(DATAFILE ${PANDA_ROOT}/libpandafile/types.yaml)
set(DEPENDENCIES ${GENERATOR} ${TEMPLATE} ${DATAFILE})
set(OUTFILE "${CMAKE_CURRENT_BINARY_DIR}/include/shorty_values.h")
add_custom_command(OUTPUT "${OUTFILE}"
                   COMMENT "Generate shorty_values.h"
                   COMMAND ${PANDA_ROOT}/isa/gen.rb -d ${DATAFILE} -t ${TEMPLATE} -o "${OUTFILE}" -r ${GENERATOR}
                   DEPENDS ${DEPENDENCIES})
add_custom_target(shorty_values_gen_${PROJECT_NAME} ALL DEPENDS "${OUTFILE}")

target_compile_options(arkruntime_interpreter_impl PUBLIC -Wno-invalid-offsetof)

set_property(TARGET arkruntime_interpreter_impl PROPERTY POSITION_INDEPENDENT_CODE ON)

if (PANDA_ENABLE_GLOBAL_REGISTER_VARIABLES)
    if (PANDA_TARGET_ARM64)
        target_compile_options(arkruntime_interpreter_impl PUBLIC -ffixed-x20 -ffixed-x21 -ffixed-x22 -ffixed-x23 -ffixed-x24 -ffixed-x25 -ffixed-x28)
        target_compile_definitions(arkruntime_interpreter_impl PUBLIC FFIXED_REGISTERS)
    endif()

    # Temporary disable clang tidy as it cannot recognize -ffixed-<reg> params that we used. Should be fixed in clang tidy 9
    set_target_properties(arkruntime_interpreter_impl PROPERTIES CXX_CLANG_TIDY "")
endif()

target_include_directories(arkruntime_interpreter_impl
    PRIVATE "$<TARGET_PROPERTY:c_secshared,INCLUDE_DIRECTORIES>"
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${PANDA_ROOT}
    PUBLIC ${GEN_INCLUDE_DIR}
    PUBLIC ${PANDA_ROOT}/compiler
    PUBLIC ${PANDA_ROOT}/libpandabase
    PUBLIC ${PANDA_ROOT}/libpandafile
    PUBLIC ${PANDA_BINARY_ROOT}/panda_gen_options
    PUBLIC ${PANDA_BINARY_ROOT}
    PUBLIC ${PANDA_BINARY_ROOT}/libpandafile/include
    PUBLIC ${PANDA_BINARY_ROOT}/libpandabase/generated
    PUBLIC ${VERIFIER_INCLUDE_DIR}
    PUBLIC ${PANDA_BINARY_ROOT}
)

add_dependencies(arkruntime_interpreter_impl
    isa_gen_${PROJECT_NAME}
    ${PROJECT_NAME}_static_options
    intrinsics_gen_${PROJECT_NAME}
    shorty_values_gen_${PROJECT_NAME}
    arkbase
    arkfile
    entrypoints_gen
)

add_dependencies(arkcompiler entrypoints_gen entrypoints_compiler_checksum_gen)

if(NOT (CMAKE_CROSSCOMPILING OR PANDA_TARGET_OHOS))
    irtoc_compile(TARGET_NAME irtoc_fastpath
                  INPUT_FILES ${IRTOC_SOURCE_DIR}/scripts/resolvers.irt
                              ${IRTOC_SOURCE_DIR}/scripts/allocation.irt
                              ${IRTOC_SOURCE_DIR}/scripts/monitors.irt
                              ${IRTOC_SOURCE_DIR}/scripts/check_cast.irt
                  TARGET_VARIABLE IRTOC_FASTPATH_OBJ)

    irtoc_compile(TARGET_NAME irtoc_interpreter
                  INPUT_FILES ${IRTOC_SOURCE_DIR}/scripts/interpreter.irt
                  TARGET_VARIABLE IRTOC_INTERPRETER_OBJ)
    set(irtoc_fastpath_target irtoc_fastpath)
    set(irtoc_interpreter_target irtoc_interpreter)
else()
    ExternalProject_Get_Property(panda_host_tools binary_dir)
    set(IRTOC_FASTPATH_OBJ "${binary_dir}/irtoc/irtoc_fastpath/irtoc_fastpath.o")
    set(IRTOC_INTERPRETER_OBJ "${binary_dir}/irtoc/irtoc_interpreter/irtoc_interpreter.o")

    SET_SOURCE_FILES_PROPERTIES(${IRTOC_FASTPATH_OBJ} PROPERTIES
        EXTERNAL_OBJECT true
        GENERATED true
    )
    SET_SOURCE_FILES_PROPERTIES(${IRTOC_INTERPRETER_OBJ} PROPERTIES
        EXTERNAL_OBJECT true
        GENERATED true
    )
    # Due to cmake complain about absence of the irtoc file, we just fake it until it will be generated
    execute_process(COMMAND mkdir -p ${binary_dir}/irtoc/irtoc_fastpath)
    execute_process(COMMAND touch ${IRTOC_FASTPATH_OBJ})
    execute_process(COMMAND mkdir -p ${binary_dir}/irtoc/irtoc_interpreter)
    execute_process(COMMAND touch ${IRTOC_INTERPRETER_OBJ})
    set(irtoc_fastpath_target build_host_tools)
    set(irtoc_interpreter_target build_host_tools)
endif()

set(ARKRUNTIME_SOURCES
    ${IRTOC_FASTPATH_OBJ}
    ${IRTOC_INTERPRETER_OBJ}
    ${SOURCES}
    ${CORE_VM_SOURCES}
    ${VERIFIER_SOURCES}
)

add_library(arkruntime_static STATIC
    ${ARKRUNTIME_SOURCES}
    $<TARGET_OBJECTS:arkruntime_interpreter_impl>
)
set_property(TARGET arkruntime_static PROPERTY POSITION_INDEPENDENT_CODE ON)

add_library(arkruntime SHARED $<TARGET_OBJECTS:arkruntime_interpreter_impl> runtime_helpers.cpp)
target_link_libraries(arkruntime arkruntime_static)

add_subdirectory(scheduler)


panda_add_to_clang_tidy(TARGET arkruntime_static)
panda_add_to_clang_tidy(TARGET arkruntime)

add_subdirectory(asm_defines)

add_dependencies(arkruntime_static
    intrinsics_gen_${PROJECT_NAME}
    isa_gen_pandaverification
    messages_gen_pandaverification
    arkstdlib
    asm_defines_generator
    entrypoints_gen
    ${irtoc_fastpath_target}
    ${irtoc_interpreter_target}
)

if(PANDA_TARGET_ARM32_ABI_SOFT OR PANDA_TARGET_ARM32_ABI_SOFTFP)
    add_dependencies(arkruntime_static bridge_dispatch_arm bridge_dispatch_dyn_arm)
elseif (PANDA_TARGET_ARM32_ABI_HARD)
    add_dependencies(arkruntime_static bridge_dispatch_armhf bridge_dispatch_dyn_arm)
elseif(PANDA_TARGET_ARM64)
    add_dependencies(arkruntime_static bridge_dispatch_aarch64 bridge_dispatch_dyn_aarch64)
elseif(PANDA_TARGET_X86)
    add_dependencies(arkruntime_static bridge_dispatch_x86)
elseif(PANDA_TARGET_AMD64)
    add_dependencies(arkruntime_static bridge_dispatch_amd64 bridge_dispatch_dyn_amd64)
endif()

target_include_directories(arkruntime_static
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${PANDA_ROOT}
    PUBLIC ${PANDA_BINARY_ROOT}
    PUBLIC ${PANDA_BINARY_ROOT}/runtime/asm_defines
    PUBLIC ${GEN_INCLUDE_DIR}
    PUBLIC ${VERIFIER_INCLUDE_DIR}
)

target_link_libraries(arkruntime_static arkbase arkfile arkcompiler dprof arkaotmanager)
if (NOT PANDA_TARGET_OHOS)
  target_link_libraries(arkruntime_static atomic)
endif()

set(CSA_TESTS_ARKRUNTIME_PATH ${GEN_INCLUDE_DIR}/debug_test_interpreter-inl_gen.cpp)
add_library(csa_tests_arkruntime_interpreter_impl OBJECT EXCLUDE_FROM_ALL ${CSA_TESTS_ARKRUNTIME_PATH})
target_include_directories(csa_tests_arkruntime_interpreter_impl
    PRIVATE "$<TARGET_PROPERTY:c_secshared,INCLUDE_DIRECTORIES>"
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${PANDA_ROOT}
    PUBLIC ${GEN_INCLUDE_DIR}
    PUBLIC ${PANDA_ROOT}/compiler
    PUBLIC ${PANDA_ROOT}/libpandabase
    PUBLIC ${PANDA_ROOT}/libpandafile
    PUBLIC ${PANDA_BINARY_ROOT}/panda_gen_options
    PUBLIC ${PANDA_BINARY_ROOT}
    PUBLIC ${PANDA_BINARY_ROOT}/compiler/generated
    PUBLIC ${PANDA_BINARY_ROOT}/libpandafile/include
    PUBLIC ${PANDA_BINARY_ROOT}/libpandabase/generated
    PUBLIC ${VERIFIER_INCLUDE_DIR}
    PUBLIC ${PANDA_BINARY_ROOT}
)
add_dependencies(csa_tests_arkruntime_interpreter_impl arkruntime_static)

# Runtime uses CodeInfo from the compiler. Maybe in the future CodeInfo
# will be moved to another place, then this line should be removed.
target_include_directories(arkruntime_static PUBLIC ${PANDA_ROOT}/compiler)

# Disable warning about offsetof usage for non-standard layout types.
# In C++17 offsetof is conditionally implemented for such types, so
# compiler should issue error if it doesn't implemented offsetof for
# them. Also we use static asserts to ensure that offsetof works correcly
# for non-standard layout types.
target_compile_options(arkruntime_static PUBLIC -Wno-invalid-offsetof)

#Build PANDA_QEMU_AARCH64_GCC_8 hangs for target arkruntime_test_interpreter_impl. It is compiler issue.
if(PANDA_WITH_TESTS AND NOT PANDA_QEMU_AARCH64_GCC_8)
  set(TEST_INTERPRETER_IMPL_SOURCES
    tests/interpreter/test_interpreter_impl.cpp
    )

  add_library(arkruntime_test_interpreter_impl OBJECT ${TEST_INTERPRETER_IMPL_SOURCES})

  target_compile_options(arkruntime_test_interpreter_impl PUBLIC -Wno-invalid-offsetof)
  panda_set_lib_32bit_property(arkruntime_test_interpreter_impl)

  if (PANDA_ENABLE_GLOBAL_REGISTER_VARIABLES)
    if (PANDA_TARGET_ARM64)
      target_compile_options(arkruntime_test_interpreter_impl PUBLIC -ffixed-x20 -ffixed-x21 -ffixed-x22 -ffixed-x23 -ffixed-x24 -ffixed-x25 -ffixed-x28)
      target_compile_definitions(arkruntime_test_interpreter_impl PUBLIC FFIXED_REGISTERS)
    endif()

    # Temporary disable clang tidy as it cannot recognize -ffixed-<reg> params that we used. Should be fixed in clang tidy 9
    set_target_properties(arkruntime_test_interpreter_impl PROPERTIES CXX_CLANG_TIDY "")
  endif()

  target_include_directories(arkruntime_test_interpreter_impl
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${PANDA_ROOT}
    PUBLIC ${GEN_INCLUDE_DIR}
    PUBLIC ${PANDA_ROOT}/compiler
    PUBLIC ${PANDA_ROOT}/libpandabase
    PUBLIC ${PANDA_ROOT}/libpandafile
    PUBLIC ${PANDA_BINARY_ROOT}/panda_gen_options
    PUBLIC ${PANDA_BINARY_ROOT}/libpandafile/include
    PUBLIC ${PANDA_BINARY_ROOT}
    PUBLIC "$<TARGET_PROPERTY:c_secshared,INCLUDE_DIRECTORIES>"
    PUBLIC ${PANDA_BINARY_ROOT}/libpandabase/generated
    PUBLIC ${VERIFIER_INCLUDE_DIR}
    )
  if (PANDA_WITH_TESTS)
      target_include_directories(arkruntime_test_interpreter_impl
          PUBLIC "$<TARGET_PROPERTY:gtest,INCLUDE_DIRECTORIES>")
  endif()


  add_dependencies(arkruntime_test_interpreter_impl
    isa_gen_${PROJECT_NAME}
    intrinsics_gen_${PROJECT_NAME}
    ${PROJECT_NAME}_static_options
    arkbase
    arkfile
    entrypoints_gen
    )

  if(PANDA_TARGET_ARM32_ABI_SOFT OR PANDA_TARGET_ARM32_ABI_SOFTFP)
    SET(INVOKE_HELPER tests/arch/arm/invokation_helper.S)
  elseif (PANDA_TARGET_ARM32_ABI_HARD)
    SET(INVOKE_HELPER tests/arch/arm/invokation_helper_hf.S)
  elseif(PANDA_TARGET_ARM64)
    SET(INVOKE_HELPER tests/arch/aarch64/invokation_helper.S)
  elseif(PANDA_TARGET_X86)
    #SET(INVOKE_HELPER tests/arch/x86/invokation_helper.S)
  elseif(PANDA_TARGET_AMD64)
    SET(INVOKE_HELPER tests/arch/amd64/invokation_helper.S)
  endif()
endif()

function(add_gtests test_name)
    panda_add_gtest(
        NO_CORES
        NAME ${test_name}
        SOURCES
            ${ARGN}
        LIBRARIES
            arkruntime
            arkassembler
        SANITIZERS
            ${PANDA_SANITIZERS_LIST}
    )

    if(TARGET ${test_name})
        target_include_directories(${test_name}
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
            PUBLIC ${PANDA_ROOT}
            PRIVATE ${GEN_INCLUDE_DIR}
        )
        target_compile_options(${test_name} PUBLIC "-Wno-ignored-attributes")
    endif()

endfunction()

function(add_multithread_gtests test_name)
    panda_add_gtest(
        NAME ${test_name}
        SOURCES
            ${ARGN}
        LIBRARIES
            arkruntime
            arkassembler
        SANITIZERS
            ${PANDA_SANITIZERS_LIST}
    )

    if(TARGET ${test_name})
        target_include_directories(${test_name}
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
            PUBLIC ${PANDA_ROOT}
            PRIVATE ${GEN_INCLUDE_DIR}
        )
        target_compile_options(${test_name} PUBLIC "-Wno-ignored-attributes")
    endif()

endfunction()

add_multithread_gtests (
    arkruntime_multithreaded_test
    tests/multithreaded_intern_string_table_test.cpp
)

add_gtests(
    arkruntime_bitmap_ClearRange_test
    tests/bitmap_clear_range_test.cpp
)

add_gtests(
    arkruntime_bitmap_OrderObjectAlignment_test
    tests/bitmap_order_object_alignment_test.cpp
)

add_gtests(
    arkruntime_bitmap_VisitorObjectAlignment_test
    tests/bitmap_visitor_object_alignment_test.cpp
)

add_gtests(
    arkruntime_bitmap_PageAlignment_test
    tests/bitmap_page_alignment_test.cpp
)

add_gtests(
    arkruntime_core_layout_test
    tests/array_test.cpp
    tests/mark_word_test.cpp
    tests/method_test.cpp
    tests/compiler_thread_pool.cpp
    tests/mock_queue_thread_pool.cpp
    tests/string_table_base_test.h
    tests/string_table_gc_test.cpp
    tests/string_test.cpp
    tests/compilation_status_test.cpp
    tests/osr_code_test.cpp
)

add_gtests(
    arkruntime_get_method_test
    tests/getmethod_test.cpp
)

add_gtests(
    arkruntime_memory_mem_leak_test
    tests/mem_leak_test.cpp
)

add_gtests(
    arkruntime_memory_statistic_test
    tests/histogram_test.cpp
    tests/mem_stats_additional_info_test.cpp
    tests/mem_stats_gc_test.cpp
    tests/mem_stats_gen_gc_test.cpp
    tests/mem_stats_test.cpp
)

add_gtests(
    arkruntime_memory_management_bump_allocator_test
    tests/bump_allocator_test.cpp
)

add_gtests(
    arkruntime_memory_management_frame_allocator_test
    tests/frame_allocator_test.cpp
)

add_gtests(
    arkruntime_memory_management_freelist_allocator_test
    tests/freelist_allocator_test.cpp
)

add_gtests(
    arkruntime_memory_management_humongous_obj_allocator_test
    tests/humongous_obj_allocator_test.cpp
)

add_gtests(
    arkruntime_memory_management_internal_allocator_test
    tests/internal_allocator_test.cpp
)

add_gtests(
    arkruntime_memory_management_malloc-proxy-allocator-test
    tests/malloc-proxy-allocator-test.cpp
)

add_gtests(
    arkruntime_memory_management_pygote_space_allocator_gen_test
    tests/pygote_space_allocator_gen_test.cpp
)

add_gtests(
    arkruntime_memory_management_pygote_space_allocator_stw_test
    tests/pygote_space_allocator_stw_test.cpp
)

# TODO(agrebenkin) Enable the tests as soon as full gc doesn't require to maintain free young-space-size
#add_gtests(
#    arkruntime_memory_management_region_allocator_test
#    tests/region_allocator_test.cpp
#)

add_gtests(
    arkruntime_memory_management_rem_set_test
    tests/rem_set_test.cpp
)

add_gtests(
    arkruntime_memory_management_runslots_allocator_test
    tests/runslots_allocator_test.cpp
)

add_gtests(
    arkruntime_memory_management_test_2
    tests/card_table_test.cpp
    tests/crossing_map_test.cpp
    tests/panda_smart_pointers_test.cpp
    tests/panda_tl_containers_test.cpp
    tests/tlab_test.cpp
    tests/gc_task_test.cpp
    tests/g1gc_test.cpp
    tests/collection_set_test.cpp
    tests/g1gc_fullgc_test.cpp
    tests/object_helpers_test.cpp
    tests/gc_log_test.cpp
)

add_gtests(
    arkruntime_sa_object_helpers_test
    tests/static_analyzer_test.cpp
)

add_gtests(
    arkruntime_gc_trigger_test
    tests/gc_trigger_test.cpp
)

add_gtests(
    arkruntime_memory_management_heap_space_test
    tests/heap_space_test.cpp
)

add_gtests(
    arkruntime_multithreading_test
    tests/compiler_queue_test.cpp
    tests/monitor_test.cpp
    tests/thread_test.cpp
)

add_gtests(
    arkruntime_options_test
    tests/options_test.cpp
)

# We run irtoc tests only in host mode, because irtoc tests are intended for testing only Irtoc language capabilities.
if (PANDA_WITH_TESTS OR HOST_TOOLS)
    if (NOT CMAKE_CROSSCOMPILING)
        irtoc_compile(TARGET_NAME irtoc_tests
            INPUT_FILES ${IRTOC_SOURCE_DIR}/scripts/tests.irt
            TARGET_VARIABLE IRTOC_TESTS_OBJ)
        set(irtoc_tests_target irtoc_tests)
    else()
        ExternalProject_Get_Property(panda_host_tools binary_dir)
        set(IRTOC_TESTS_OBJ "${binary_dir}/irtoc/irtoc_tests/irtoc_tests.o")

        SET_SOURCE_FILES_PROPERTIES(${IRTOC_TESTS_OBJ} PROPERTIES
            EXTERNAL_OBJECT true
            GENERATED true
        )
        # Due to cmake complain about absence of the irtoc file, we just fake it until it will be generated
        execute_process(COMMAND mkdir -p ${binary_dir}/irtoc/irtoc_tests)
        execute_process(COMMAND touch ${IRTOC_TESTS_OBJ})
        set(irtoc_tests_target build_host_tools)
    endif()
    # In Host tools mode we only generate Irtoc files for irtoc_tests, we shouldn't create irtoc_tests itself.
    if (irtoc_tests_target AND NOT HOST_TOOLS)
        add_gtests(
            arkruntime_irtoc_tests
            tests/irtoc_test.cpp
            ${IRTOC_TESTS_OBJ}
        )
        add_dependencies(arkruntime_irtoc_tests ${irtoc_tests_target})
    endif()
endif()

if (NOT PANDA_PRODUCT_BUILD)
  add_gtests(
    arkruntime_intrinsics_blacklist_test
    tests/intrinsics_blacklist_test.cpp
    )
endif()

if (TARGET arkruntime_test_interpreter_impl)
    add_gtests(
        arkruntime_interpreter_test
        tests/c2i_bridge_test.cpp
        tests/class_linker_test.cpp
        tests/class_linker_test_extension.cpp
        tests/exception_test.cpp
        tests/hybrid_object_allocator_test.cpp
        tests/interpreter/test_interpreter.cpp
        tests/interpreter/test_runtime_interface.cpp
        tests/interpreter_test.cpp
        tests/invokation_helper.cpp
        $<TARGET_OBJECTS:arkruntime_test_interpreter_impl>
        ${INVOKE_HELPER}
    )
endif()

# In interpreter_test.cpp tests ResolveCtorClass and
# ResolveField are extremely slow (that lead to timeout in arm32 debug builds).
# To avoid timeout these tests were moved to special targets
# and code generation that needed in tests is made in build time
add_gtests(
    interpreter_test_resolve_field
    tests/interpreter_test_resolve_field.cpp
)

add_gtests(
    interpreter_test_resolve_ctor_class
    tests/interpreter_test_resolve_ctor_class.cpp
)

if (PANDA_WITH_TESTS)
    file(MAKE_DIRECTORY ${PANDA_BINARY_ROOT}/bin-gtests/pre-build)

    function(generate_gtests_prebuild test_name)
        set(SOURCE_TEMPLATE "${CMAKE_CURRENT_LIST_DIR}/tests/templates/${test_name}.pa.erb")
        set(GENERATED_OUTPUT "${PANDA_BINARY_ROOT}/bin-gtests/pre-build/${test_name}")

        if(CMAKE_CROSSCOMPILING)
            ExternalProject_Get_Property(panda_host_tools binary_dir)
            set(assembler_target panda_host_tools)
            set(assembler_bin    "${binary_dir}/assembler/ark_asm")
        else()
            set(assembler_target ark_asm)
            set(assembler_bin    $<TARGET_FILE:${assembler_target}>)
        endif()

        add_custom_command(OUTPUT ${GENERATED_OUTPUT}.abc
            COMMENT "Generating ${test_name}.abc"
            COMMAND erb ${SOURCE_TEMPLATE} > ${GENERATED_OUTPUT}.pa
            COMMAND ${assembler_bin} ${GENERATED_OUTPUT}.pa ${GENERATED_OUTPUT}.abc
            DEPENDS ${assembler_target}
        )

        add_custom_target(${test_name}_abc
            DEPENDS ${GENERATED_OUTPUT}.abc
        )

        add_dependencies(${test_name}_gtests ${test_name}_abc)
    endfunction()

    generate_gtests_prebuild(interpreter_test_resolve_field)
    generate_gtests_prebuild(interpreter_test_resolve_ctor_class)
endif()

add_gtests(
    arkruntime_utils_test
    tests/class_size_test.cpp
    tests/debugger_test.cpp
    tests/frame_test.cpp
    tests/i2c_bridge_test.cpp
    tests/math_helpers_test.cpp
    tests/stack_walker_test.cpp
    tests/time_utils_test.cpp
)

include(intrinsics.cmake)

add_subdirectory(tooling/inspector)

if(PANDA_WITH_TESTS)
    add_subdirectory(tests/tooling)
endif()

panda_add_sanitizers(TARGET arkruntime SANITIZERS ${PANDA_SANITIZERS_LIST})
panda_add_sanitizers(TARGET arkruntime_static SANITIZERS ${PANDA_SANITIZERS_LIST})
panda_add_sanitizers(TARGET arkruntime_interpreter_impl SANITIZERS ${PANDA_SANITIZERS_LIST})
if(PANDA_WITH_TESTS AND TARGET arkruntime_test_interpreter_impl)
    panda_add_sanitizers(TARGET arkruntime_test_interpreter_impl SANITIZERS ${PANDA_SANITIZERS_LIST})
endif()
add_check_style(".")
