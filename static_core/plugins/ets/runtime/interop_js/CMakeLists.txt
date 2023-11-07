# Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
    list(APPEND ETS_INTEROP_ARCH_SOURCES arch/arm32/ts2ets_jscall_bridge_arm32.S)
elseif(PANDA_TARGET_ARM32_ABI_HARD)
    list(APPEND ETS_INTEROP_ARCH_SOURCES arch/arm32/ts2ets_jscall_bridge_arm32hf.S)
elseif(PANDA_TARGET_ARM64)
    list(APPEND ETS_INTEROP_ARCH_SOURCES arch/arm64/ts2ets_jscall_bridge_aarch64.S)
elseif(PANDA_TARGET_AMD64)
    list(APPEND ETS_INTEROP_ARCH_SOURCES arch/amd64/ts2ets_jscall_bridge_amd64.S)
endif()

panda_ets_interop_js_plugin(ets_interop_js_napi
    SOURCES
        interop_context.cpp
        ets_vm_plugin.cpp
        intrinsics_api_impl.cpp
        js_proxy/js_proxy.cpp
        ets_proxy/ets_class_wrapper.cpp
        ets_proxy/ets_field_wrapper.cpp
        ets_proxy/ets_method_wrapper.cpp
        ets_proxy/shared_reference.cpp
        ets_proxy/shared_reference_storage.cpp
        ets_proxy/ets_proxy.cpp
        js_value.cpp
        js_refconvert.cpp
        js_refconvert_builtin.cpp
        js_refconvert_function.cpp
        js_value_call.cpp
        interop_common.cpp
        ts2ets_copy.cpp
        ts2ets_tstype.cpp
        js_job_queue.cpp
        pending_promise_listener.cpp
        ${ETS_INTEROP_ARCH_SOURCES}
    LIBRARIES
        arkruntime
)
