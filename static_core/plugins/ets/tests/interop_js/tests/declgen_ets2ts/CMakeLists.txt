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

set(MAIN_TS_INPUT ${CMAKE_CURRENT_SOURCE_DIR}/main.ts)
set(LIB_1_ETS_INPUT ${CMAKE_CURRENT_SOURCE_DIR}/lib1.ets)
set(LIB_2_ETS_INPUT ${CMAKE_CURRENT_SOURCE_DIR}/lib2.ets)
set(LIB_3_JS_INPUT ${CMAKE_CURRENT_SOURCE_DIR}/lib3.js)
set(LIB_1_DECL_TS_EXPECTED ${CMAKE_CURRENT_SOURCE_DIR}/lib1.expected)
set(LIB_2_DECL_TS_EXPECTED ${CMAKE_CURRENT_SOURCE_DIR}/lib2.expected)

set(MAIN_TS ${CMAKE_CURRENT_BINARY_DIR}/main.ts)
set(MAIN_JS ${CMAKE_CURRENT_BINARY_DIR}/main.js)
set(LIB_1_ABC ${CMAKE_CURRENT_BINARY_DIR}/lib1.abc)
set(LIB_2_ABC ${CMAKE_CURRENT_BINARY_DIR}/lib2.abc)
set(LIB_LINKED_ABC ${CMAKE_CURRENT_BINARY_DIR}/lib_linked.abc)
set(LIB_1_DECL_TS ${CMAKE_CURRENT_BINARY_DIR}/lib1.ts)
set(LIB_2_DECL_TS ${CMAKE_CURRENT_BINARY_DIR}/lib2.ts)

set(ARKTSCONFIG ${CMAKE_CURRENT_BINARY_DIR}/arktsconfig.json)
file(WRITE ${ARKTSCONFIG}
    "{\n"
    "  \"compilerOptions\": {\n"
    "    \"baseUrl\": \"${PANDA_ROOT}\",\n"
    "    \"paths\": {\n"
    "      \"std\": [\"${PANDA_ROOT}/plugins/ets/stdlib/std\"],\n"
    "      \"escompat\": [\"${PANDA_ROOT}/plugins/ets/stdlib/escompat\"]\n"
    "    },\n"
    "    \"dynamicPaths\": {\n"
    "      \"${PANDA_ROOT}/plugins/ets/tests/interop_js/tests/declgen_ets2ts/lib3.js\": {\"language\": \"js\", \"hasDecl\": false}\n"
    "    }\n"
    "  }\n"
    "}\n"
)

# Compile ts+ets+js interop project:
#  1. main.ts + lib1.ets + lib2.ets + lib3.js
#  2. main.ts + lib1.abc + lib1.ts + lib2.abc + lib2.ts + lib3.js
#  3. main.js + lib_linked.abc + lib1.js + lib2.js + lib3.js
add_custom_command(
    OUTPUT ${MAIN_TS} ${MAIN_JS} ${LIB_1_ABC} ${LIB_2_ABC} ${LIB_LINKED_ABC} ${LIB_1_DECL_TS} ${LIB_2_DECL_TS}
    COMMENT "Compile interop project: ${MAIN_TS_INPUT} ${LIB_1_ETS_INPUT} ${LIB_2_ETS_INPUT} ${LIB_3_JS_INPUT}"
    COMMAND $<TARGET_FILE:es2panda> --arktsconfig ${ARKTSCONFIG} --output ${LIB_1_ABC} --gen-ts-decl ${LIB_1_DECL_TS} ${LIB_1_ETS_INPUT}
    COMMAND $<TARGET_FILE:es2panda> --output ${LIB_2_ABC} --gen-ts-decl ${LIB_2_DECL_TS} ${LIB_2_ETS_INPUT} --ets-module
    COMMAND $<TARGET_FILE:ark_link> --output ${LIB_LINKED_ABC} -- ${LIB_1_ABC} ${LIB_2_ABC}
    COMMAND cp ${MAIN_TS_INPUT} ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${PANDA_THIRD_PARTY_SOURCES_DIR}/typescript/bin/tsc --strict --outDir ${CMAKE_CURRENT_BINARY_DIR} ${MAIN_TS}
    DEPENDS es2panda ark_link compile_typescript ${LIB_1_ETS_INPUT} ${LIB_2_ETS_INPUT} ${MAIN_TS_INPUT} ${ARKTSCONFIG}
)
add_custom_target(compile_declgen_ets2ts_project
    DEPENDS ${MAIN_TS} ${MAIN_JS} ${LIB_1_ABC} ${LIB_2_ABC} ${LIB_LINKED_ABC} ${LIB_1_DECL_TS} ${LIB_2_DECL_TS}
)

# Test difference between produced TS declarations and expected TS declarations
add_custom_target(ets_interop_js_test_declgen_ets2ts_diff
    COMMENT "Test declgen ets2ts diff"
    COMMAND ${CMAKE_COMMAND} -E compare_files ${LIB_1_DECL_TS} ${LIB_1_DECL_TS_EXPECTED}
    COMMAND ${CMAKE_COMMAND} -E compare_files ${LIB_2_DECL_TS} ${LIB_2_DECL_TS_EXPECTED}
    DEPENDS compile_declgen_ets2ts_project ${LIB_1_DECL_TS} ${LIB_2_DECL_TS} ${LIB_1_DECL_TS_EXPECTED} ${LIB_2_DECL_TS_EXPECTED}
)
add_dependencies(ets_interop_js_tests ets_interop_js_test_declgen_ets2ts_diff)

# Execute compiled interop project
panda_ets_interop_js_test(ets_interop_js_test_declgen_ets2ts
    ABC_FILE ${LIB_LINKED_ABC}
    JS_LAUNCHER ${MAIN_JS}
)
add_dependencies(ets_interop_js_test_declgen_ets2ts compile_declgen_ets2ts_project)
