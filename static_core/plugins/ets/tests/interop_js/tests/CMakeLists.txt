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

set(INTEROP_JS_TESTS_SOURCE_DIR ${PANDA_ETS_PLUGIN_SOURCE}/tests/interop_js/tests)

set(ETS_CONFIG "${PANDA_BINARY_ROOT}/tests/ets_interop_js/arktsconfig.json")
file(WRITE ${ETS_CONFIG}
    "{\n"
    "  \"compilerOptions\": {\n"
    "    \"baseUrl\": \"${PANDA_ROOT}\",\n"
    "    \"paths\": {\n"
    "      \"escompat\": [\"${PANDA_ROOT}/plugins/ets/stdlib/escompat\"],\n"
    "      \"std\": [\"${PANDA_ROOT}/plugins/ets/stdlib/std\"]\n"
    "    },\n"
    "    \"dynamicPaths\": {\n"
    "      \"${PANDA_ROOT}/plugins/ets/tests/interop_js/tests/test_intrins/test_intrins.js\": {\"language\": \"js\", \"hasDecl\": false},\n"
    "      \"${PANDA_ROOT}/plugins/ets/tests/interop_js/tests/test_frontend/test_frontend_package.js\": {\"language\": \"js\", \"hasDecl\": false},\n"
    "      \"${PANDA_ROOT}/plugins/ets/tests/interop_js/tests/number_subtypes/module.js\": {\"language\": \"js\", \"hasDecl\": false},\n"
    "      \"${PANDA_ROOT}/plugins/ets/tests/interop_js/tests/strict_equality/strict_equality.js\": {\"language\": \"js\", \"hasDecl\": false},\n"
    "      \"${PANDA_ROOT}/plugins/ets/tests/interop_js/tests/test_undefined/test_undefined.js\": {\"language\": \"js\", \"hasDecl\": false},\n"
    "      \"${PANDA_ROOT}/plugins/ets/tests/interop_js/tests/class_composite/js/class_composite.js\": {\"language\": \"js\", \"hasDecl\": false},\n"
    "      \"${PANDA_ROOT}/plugins/ets/tests/interop_js/tests/dynamic_instanceof/dynamic_instanceof.js\": {\"language\": \"js\", \"hasDecl\": false},\n"
    "      \"${PANDA_ROOT}/plugins/ets/tests/interop_js/tests/scenarios/js_to_ets/scenarios.js\": {\"language\": \"js\", \"hasDecl\": false},\n"
    "      \"${PANDA_ROOT}/plugins/ets/tests/interop_js/tests/es_object/to_string/test_to_string_method.js\": {\"language\": \"js\", \"hasDecl\": false},\n"
    "      \"${PANDA_ROOT}/plugins/ets/tests/interop_js/tests/checked/example/strings.js\": {\"language\": \"js\", \"hasDecl\": false},\n"
    "      \"${PANDA_ROOT}/plugins/ets/tests/interop_js/tests/checked/js_call/js_call.js\": {\"language\": \"js\", \"hasDecl\": false},\n"
    "      \"${PANDA_ROOT}/plugins/ets/tests/interop_js/tests/checked/obj_call/obj_call.js\": {\"language\": \"js\", \"hasDecl\": false},\n"
    "      \"${PANDA_ROOT}/plugins/ets/tests/interop_js/tests/checked/qualified_call/qualified_call.js\": {\"language\": \"js\", \"hasDecl\": false},\n"
    "      \"${PANDA_ROOT}/plugins/ets/tests/interop_js/tests/dynamic_with_lambda/dynamic_with_lambda.js\": {\"language\": \"js\", \"hasDecl\": false},\n"
    "      \"${PANDA_ROOT}/plugins/ets/tests/interop_js/tests/dynamic_enum/dynamic_enum.js\": {\"language\": \"js\", \"hasDecl\": false},\n"
    "      \"${PANDA_ROOT}/plugins/ets/tests/interop_js/tests/perf/code.js\": {\"language\": \"js\", \"hasDecl\": false},\n"
    "      \"${PANDA_ROOT}/plugins/ets/tests/interop_js/tests/dynamic_object/dynamic_call.js\": {\"language\": \"js\", \"hasDecl\": false},\n"
    "      \"${PANDA_ROOT}/plugins/ets/tests/interop_js/tests/bouncing_pandas/bouncing_pandas.js\": {\"language\": \"js\", \"hasDecl\": false},\n"
    "      \"convert.js\": {\"language\": \"js\", \"hasDecl\": false}\n"
    "    }\n"
    "  }\n"
    "}\n"
)

set(ETS_CONFIG_DECL "${PANDA_BINARY_ROOT}/tests/ets_interop_js/arktsconfigdecl.json")
file(WRITE ${ETS_CONFIG_DECL}
    "{\n"
    "  \"compilerOptions\": {\n"
    "    \"baseUrl\": \"${PANDA_ROOT}\",\n"
    "    \"paths\": {\n"
    "      \"escompat\": [\"${PANDA_ROOT}/plugins/ets/stdlib/escompat\"],\n"
    "      \"std\": [\"${PANDA_ROOT}/plugins/ets/stdlib/std\"]\n"
    "    },\n"
    "    \"dynamicPaths\": {\n"
    "      \"${PANDA_ROOT}/plugins/ets/tests/interop_js/tests/number_subtypes/module\": {\"language\": \"js\", \"hasDecl\": true},\n"
    "      \"${PANDA_ROOT}/plugins/ets/tests/interop_js/tests/class_composite/js/class_composite\": {\"language\": \"js\", \"hasDecl\": true}\n"
    "    }\n"
    "  }\n"
    "}\n"
)

panda_ets_interop_js_gtest(ets_interop_js_test__sample
    CPP_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/sample/test_sample.cpp
    ETS_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/sample/test_sample.ets
)

panda_ets_interop_js_gtest(ets_interop_js_test__map_mirror_classes
    CPP_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/map_mirror_classes/test_map_mirror_classes.cpp
    ETS_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/map_mirror_classes/test_map_mirror_classes.ets
    LIBRARIES arkruntime
)

panda_ets_interop_js_gtest(ets_interop_js_test_bouncing_pandas
    CPP_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/bouncing_pandas/bouncing_pandas.cpp
    ETS_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/bouncing_pandas/bouncing_pandas_test.ets
                ${PANDA_ETS_PLUGIN_SOURCE}/tests/common/bouncing_pandas/bouncing_pandas.ets
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js_test_intrins
    CPP_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/test_intrins/test_intrins.cpp
    ETS_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/test_intrins/test_intrins.ets
)

panda_ets_interop_js_gtest(ets_interop_js_frontend_test_intrins
    CPP_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/test_intrins/test_intrins.cpp
    ETS_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/test_intrins/frontend_test_intrins.ets
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js_frontend_test_frontend_package
    CPP_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/test_frontend/test_frontend_package.cpp
    ETS_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/test_frontend/test_frontend_package.ets
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js_test__dynamic_object
    CPP_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/dynamic_object/dynamic_object.cpp
    ETS_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/dynamic_object/dynamic_object.ets
)

panda_ets_interop_js_gtest(ets_interop_js_class_operations
    CPP_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/class_operations/class_operations.cpp
    ETS_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/class_operations/class_operations.ets
)
panda_ets_interop_js_gtest(ets_interop_js_frontend_class_operations
    CPP_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/class_operations/class_operations.cpp
    ETS_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/class_operations/class_operations_frontend.ets
    ETS_CONFIG ${ETS_CONFIG}
)
panda_ets_interop_js_gtest(ets_interop_js_class_operations_decl
    CPP_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/class_operations/class_operations.cpp
    ETS_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/class_operations/class_operations_decl.ets
    ETS_CONFIG ${ETS_CONFIG_DECL}
)

panda_ets_interop_js_gtest(ets_interop_js_strict_equality
    CPP_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/strict_equality/strict_equality.cpp
    ETS_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/strict_equality/strict_equality.ets
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js_class_composite
    CPP_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/class_composite/class_composite.cpp
    ETS_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/class_composite/class_composite.ets
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js_class_composite_decl
    CPP_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/class_composite/class_composite.cpp
    ETS_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/class_composite/class_composite_decl.ets
    ETS_CONFIG ${ETS_CONFIG_DECL}
)

#panda_ets_interop_js_gtest(ets_interop_js_dynamic_instanceof
#    CPP_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/dynamic_instanceof/dynamic_instanceof.cpp
#    ETS_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/dynamic_instanceof/dynamic_instanceof.ets
#    ETS_CONFIG ${ETS_CONFIG}
#)

panda_ets_interop_js_gtest(ets_interop_dynamic_with_lambda
    CPP_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/dynamic_with_lambda/dynamic_with_lambda.cpp
    ETS_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/dynamic_with_lambda/dynamic_with_lambda.ets
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js_test_dynamic_call
        CPP_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/dynamic_object/dynamic_call.cpp
        ETS_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/dynamic_object/dynamic_call.ets
        ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js_dynamic_enum
    CPP_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/dynamic_enum/dynamic_enum.cpp
    ETS_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/dynamic_enum/dynamic_enum.ets
    ETS_CONFIG ${ETS_CONFIG}
)

panda_ets_interop_js_gtest(ets_interop_js_test_undefined
    CPP_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/test_undefined/test_undefined.cpp
    ETS_SOURCES ${INTEROP_JS_TESTS_SOURCE_DIR}/test_undefined/test_undefined.ets
    ETS_CONFIG ${ETS_CONFIG}
)

add_subdirectory(declgen_ets2ts)
add_subdirectory(number_subtypes)
add_subdirectory(promise)
add_subdirectory(escompat_demo)
add_subdirectory(escompat)
add_subdirectory(es_object)
add_subdirectory(generic_types)
add_subdirectory(proxies)
add_subdirectory(checked)
add_subdirectory(perf)
add_subdirectory(compiler)
add_subdirectory(scenarios)
