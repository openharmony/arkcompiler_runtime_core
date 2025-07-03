# Copyright (c) 2025 Huawei Device Co., Ltd.
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

cmake_minimum_required(VERSION 3.14.1 FATAL_ERROR)

project(plugin_ets_sdk_native)

set(ETS_SDK_NATIVE_SRC
    main.cpp
    api/Util.cpp
    api/ani_textencoder.cpp
    api/ani_textencoder_helper.cpp
    api/ani_textdecoder.cpp
    api/ani_stringdecoder.cpp
)

set(ETS_SDK_SO_LOC "${PANDA_BINARY_ROOT}/lib")

panda_add_library(ets_sdk_native SHARED ${ETS_SDK_NATIVE_SRC})
panda_target_sources(ets_sdk_native PRIVATE ${ETS_SDK_NATIVE_SRC})
set_target_properties(ets_sdk_native PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${ETS_SDK_SO_LOC})
panda_target_link_libraries(ets_sdk_native arkruntime)
panda_target_include_directories(ets_sdk_native PRIVATE ${PANDA_ROOT}/plugins/ets/sdk/native/)
panda_target_include_directories(ets_sdk_native PRIVATE ${PANDA_ROOT}/plugins/ets/runtime/ani/)

add_dependencies(ets_sdk_prereqs ets_sdk_native)
