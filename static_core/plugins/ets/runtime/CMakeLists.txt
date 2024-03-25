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

set(ETS_EXT_SOURCES ${PANDA_ETS_PLUGIN_SOURCE}/runtime)

panda_promote_to_definitions(USE_ETS_NAPI_CRITICAL_BY_DEFAULT)

set(ETS_RUNTIME_SOURCES
    ${ETS_EXT_SOURCES}/ets_annotation.cpp
    ${ETS_EXT_SOURCES}/ets_class_linker.cpp
    ${ETS_EXT_SOURCES}/ets_class_linker_extension.cpp
    ${ETS_EXT_SOURCES}/ets_exceptions.cpp
    ${ETS_EXT_SOURCES}/ets_errors.cpp
    ${ETS_EXT_SOURCES}/ets_language_context.cpp
    ${ETS_EXT_SOURCES}/ets_napi_env.cpp
    ${ETS_EXT_SOURCES}/ets_native_library.cpp
    ${ETS_EXT_SOURCES}/ets_native_library_provider.cpp
    ${ETS_EXT_SOURCES}/ets_runtime_interface.cpp
    ${ETS_EXT_SOURCES}/ets_vm.cpp
    ${ETS_EXT_SOURCES}/ets_stubs.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core.cpp
    ${ETS_EXT_SOURCES}/ets_itable_builder.cpp
    ${ETS_EXT_SOURCES}/ets_vtable_builder.cpp
    ${ETS_EXT_SOURCES}/intrinsics/escompat_ArrayBuffer.cpp
    ${ETS_EXT_SOURCES}/intrinsics/escompat_Date.cpp
    ${ETS_EXT_SOURCES}/intrinsics/escompat_RegExp.cpp
    ${ETS_EXT_SOURCES}/intrinsics/compiler_intrinsics.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_Runtime.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_Char.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_Console.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_Double.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_Float.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_String.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_StringBuilder.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_gc.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_finalization_registry.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_Promise.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_StackTrace.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_Type.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_TypeCreator.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_Method.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_Value.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_Arrays.cpp
    ${ETS_EXT_SOURCES}/intrinsics/escompat_Atomics.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_math.cpp
    ${ETS_EXT_SOURCES}/intrinsics/escompat_JSON.cpp
    ${ETS_EXT_SOURCES}/intrinsics/helpers/ets_intrinsics_helpers.cpp
    ${ETS_EXT_SOURCES}/mem/ets_reference_processor.cpp
    ${ETS_EXT_SOURCES}/napi/ets_napi_helpers.cpp
    ${ETS_EXT_SOURCES}/napi/ets_napi_invoke_interface.cpp
    ${ETS_EXT_SOURCES}/napi/ets_napi_native_interface.cpp
    ${ETS_EXT_SOURCES}/napi/ets_mangle.cpp
    ${ETS_EXT_SOURCES}/regexp/regexp_executor.cpp
    ${ETS_EXT_SOURCES}/ets_coroutine.cpp
    ${ETS_EXT_SOURCES}/ets_entrypoints.cpp
    ${ETS_EXT_SOURCES}/types/ets_class.cpp
    ${ETS_EXT_SOURCES}/types/ets_field.cpp
    ${ETS_EXT_SOURCES}/types/ets_method.cpp
    ${ETS_EXT_SOURCES}/types/ets_promise.cpp
    ${ETS_EXT_SOURCES}/types/ets_object.cpp
    ${ETS_EXT_SOURCES}/types/ets_string_builder.cpp
    ${ETS_EXT_SOURCES}/types/ets_typeapi_create.cpp
    ${ETS_EXT_SOURCES}/types/ets_typeapi_field.cpp
    ${ETS_EXT_SOURCES}/types/ets_typeapi_method.cpp
    ${ETS_EXT_SOURCES}/types/ets_typeapi_parameter.cpp
    ${ETS_EXT_SOURCES}/types/ets_shared_memory.cpp
    ${ETS_EXT_SOURCES}/ets_vm_api.cpp
    ${ETS_EXT_SOURCES}/ets_utils.cpp
)

get_directory_property(CURRENT_DEFS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMPILE_DEFINITIONS)

if (PANDA_TARGET_ARM32_ABI_SOFT OR PANDA_TARGET_ARM32_ABI_SOFTFP)
    list (APPEND ETS_RUNTIME_SOURCES
        ${ETS_EXT_SOURCES}/napi/arch/arm32/ets_async_entry_point_arm32.S
        ${ETS_EXT_SOURCES}/napi/arch/arm32/ets_napi_entry_point_arm32.S)
elseif(PANDA_TARGET_ARM32_ABI_HARD)
    list (APPEND ETS_RUNTIME_SOURCES
        ${ETS_EXT_SOURCES}/napi/arch/arm32/ets_async_entry_point_arm32hf.S
        ${ETS_EXT_SOURCES}/napi/arch/arm32/ets_napi_entry_point_arm32hf.S)
elseif(PANDA_TARGET_ARM64)
    list (APPEND ETS_RUNTIME_SOURCES
        ${ETS_EXT_SOURCES}/napi/arch/arm64/ets_async_entry_point_aarch64.S
        ${ETS_EXT_SOURCES}/napi/arch/arm64/ets_napi_entry_point_aarch64.S)
elseif (PANDA_TARGET_AMD64)
    list (APPEND ETS_RUNTIME_SOURCES
        ${ETS_EXT_SOURCES}/napi/arch/amd64/ets_async_entry_point_amd64.S
        ${ETS_EXT_SOURCES}/napi/arch/amd64/ets_napi_entry_point_amd64.S)
endif()

panda_target_sources(arkruntime_obj PRIVATE ${ETS_RUNTIME_SOURCES})
panda_target_include_directories(arkruntime_obj PUBLIC
    ${PANDA_BINARY_ROOT}/cross_values
    ${PANDA_ETS_PLUGIN_SOURCE}/runtime/
    ${PANDA_BINARY_ROOT}/
)

panda_target_link_libraries(arkruntime_obj arkassembler)

panda_target_compile_definitions(arkruntime_obj PRIVATE ${CURRENT_DEFS})
add_dependencies(arkruntime_obj cross_values)

if(PANDA_TARGET_OHOS)
    panda_target_link_libraries(arkruntime_obj hilog_ndk.z)
endif()

if (PANDA_ETS_INTEROP_JS)
    add_subdirectory(interop_js)

    # NOTE:
    #   Implementation of intrinsics should be built in 'arkruntime_obj' target
    panda_target_sources(arkruntime_obj PRIVATE ${ETS_EXT_SOURCES}/interop_js/intrinsics/std_js_jsruntime.cpp)
endif()

add_subdirectory(napi)
