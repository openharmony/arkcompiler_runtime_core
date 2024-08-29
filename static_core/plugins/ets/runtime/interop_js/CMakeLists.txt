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

set(ETS_INTEROP_ARCH_SOURCES)

if(PANDA_TARGET_ARM32_ABI_SOFT OR PANDA_TARGET_ARM32_ABI_SOFTFP)
    list(APPEND ETS_INTEROP_ARCH_SOURCES ${ETS_EXT_SOURCES}/interop_js/arch/arm32/call_bridge_arm32.S)
elseif(PANDA_TARGET_ARM32_ABI_HARD)
    list(APPEND ETS_INTEROP_ARCH_SOURCES ${ETS_EXT_SOURCES}/interop_js/arch/arm32/call_bridge_arm32hf.S)
elseif(PANDA_TARGET_ARM64)
    list(APPEND ETS_INTEROP_ARCH_SOURCES ${ETS_EXT_SOURCES}/interop_js/arch/arm64/call_bridge_aarch64.S)
elseif(PANDA_TARGET_AMD64)
    list(APPEND ETS_INTEROP_ARCH_SOURCES ${ETS_EXT_SOURCES}/interop_js/arch/amd64/call_bridge_amd64.S)
endif()

panda_ets_interop_js_plugin(ets_interop_js_napi
    SOURCES
        ets_vm_plugin.cpp
        event_loop_module.cpp
        timer_module.cpp
    LIBRARIES
        arkruntime
)

panda_target_sources(arkruntime_obj PRIVATE
        ${ETS_EXT_SOURCES}/interop_js/intrinsics/std_js_jsruntime.cpp
        ${ETS_EXT_SOURCES}/interop_js/interop_context.cpp
        ${ETS_EXT_SOURCES}/interop_js/intrinsics_api_impl.cpp
        ${ETS_EXT_SOURCES}/interop_js/call/call_js.cpp
        ${ETS_EXT_SOURCES}/interop_js/call/call_ets.cpp
        ${ETS_EXT_SOURCES}/interop_js/js_proxy/js_proxy.cpp
        ${ETS_EXT_SOURCES}/interop_js/ets_proxy/ets_class_wrapper.cpp
        ${ETS_EXT_SOURCES}/interop_js/ets_proxy/ets_field_wrapper.cpp
        ${ETS_EXT_SOURCES}/interop_js/ets_proxy/ets_method_wrapper.cpp
        ${ETS_EXT_SOURCES}/interop_js/ets_proxy/ets_method_set.cpp
        ${ETS_EXT_SOURCES}/interop_js/ets_proxy/shared_reference.cpp
        ${ETS_EXT_SOURCES}/interop_js/ets_proxy/shared_reference_storage.cpp
        ${ETS_EXT_SOURCES}/interop_js/ets_proxy/ets_proxy.cpp
        ${ETS_EXT_SOURCES}/interop_js/js_value.cpp
        ${ETS_EXT_SOURCES}/interop_js/js_refconvert.cpp
        ${ETS_EXT_SOURCES}/interop_js/js_refconvert_builtin.cpp
        ${ETS_EXT_SOURCES}/interop_js/js_refconvert_function.cpp
        ${ETS_EXT_SOURCES}/interop_js/napi_impl/napi_impl.cpp
        ${ETS_EXT_SOURCES}/interop_js/interop_common.cpp
        ${ETS_EXT_SOURCES}/interop_js/ts2ets_copy.cpp
        ${ETS_EXT_SOURCES}/interop_js/js_job_queue.cpp
        ${ETS_EXT_SOURCES}/interop_js/js_remote_promise_resolver.cpp
        ${ETS_INTEROP_ARCH_SOURCES}
)
if(PANDA_TARGET_OHOS)
    panda_target_link_libraries(arkruntime_obj ace_napi.z uv -Wl,--no-undefined)
else()
    panda_target_include_directories(arkruntime_obj SYSTEM INTERFACE ${NAPI_HEADERS_PATH})
endif()
