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

set(ETS_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/arktsconfig.json)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.in.json ${ETS_CONFIG})

panda_ets_interop_js_gtest(ets_interop_js_objects_passing
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/objects_passing.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/objects_passing.ets
    JS_SOURCES 
        ${CMAKE_CURRENT_SOURCE_DIR}/objects_passing.js
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js_objects_passing_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/objects_passing_sts.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/objects_passing_ets_test/objects_passing_sts.ets
    JS_SOURCES 
        ${CMAKE_CURRENT_SOURCE_DIR}/objects_passing_sts.test.js
    ETS_CONFIG ${ETS_CONFIG}
    COMPILATION_JS_WITH_CJS_ON
)
