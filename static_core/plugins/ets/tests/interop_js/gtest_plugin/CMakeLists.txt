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


# Create ets_interop_js_gtest plugin
panda_add_library(ets_interop_js_gtest STATIC ets_interop_js_gtest.cpp)
panda_target_link_libraries(ets_interop_js_gtest gtest)
panda_target_link_libraries(ets_interop_js_gtest arkruntime)
set_target_properties(ets_interop_js_gtest PROPERTIES POSITION_INDEPENDENT_CODE ON)
panda_target_include_directories(ets_interop_js_gtest
    PUBLIC ${PANDA_ETS_PLUGIN_SOURCE}/tests/interop_js/gtest_plugin
    PUBLIC ${PANDA_ROOT}
)
if(PANDA_TARGET_OHOS)
    panda_target_link_libraries(ets_interop_js_gtest ace_napi.z)
else()
    panda_target_include_directories(ets_interop_js_gtest SYSTEM PRIVATE ${NAPI_HEADERS_PATH})
endif()


# Link gtest_launcher.js to <INTEROP_TESTS_DIR>/tests/ets_interop_js/
set(GTEST_LAUNCHER_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/gtest_launcher.js")
set(GTEST_LAUNCHER "${PANDA_BINARY_ROOT}/tests/ets_interop_js/gtest_launcher.js")
add_custom_command(
    OUTPUT ${GTEST_LAUNCHER}
    COMMENT "Link ${GTEST_LAUNCHER_SOURCE} to ${GTEST_LAUNCHER}"
    COMMAND ln -sf ${GTEST_LAUNCHER_SOURCE} ${GTEST_LAUNCHER}
    DEPENDS ${GTEST_LAUNCHER_SOURCE}
)
add_custom_target(ets_interop_js_gtest_launcher
    DEPENDS ${GTEST_LAUNCHER} ets_interop_js_napi
)
