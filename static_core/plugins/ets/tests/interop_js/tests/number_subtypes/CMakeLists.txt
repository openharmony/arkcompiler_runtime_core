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

set(INTEROP_TESTS_GENERATED_DIR "${PANDA_BINARY_ROOT}/plugins/ets/tests/ets_interop_js/generated")


gen_file(TARGET test_number_subtypes_gen_cpp
    TEMPLATE ${INTEROP_JS_TESTS_SOURCE_DIR}/number_subtypes/number_subtypes.cpp.erb
    OUTPUT ${INTEROP_TESTS_GENERATED_DIR}/number_subtypes.cpp
    REQUIRES ${INTEROP_JS_TESTS_SOURCE_DIR}/number_subtypes/number_subtypes.rb
)

gen_file(TARGET test_number_subtypes_gen_ets
    TEMPLATE ${INTEROP_JS_TESTS_SOURCE_DIR}/number_subtypes/number_subtypes.ets.erb
    OUTPUT ${INTEROP_TESTS_GENERATED_DIR}/number_subtypes.ets
    REQUIRES ${INTEROP_JS_TESTS_SOURCE_DIR}/number_subtypes/number_subtypes.rb
)

gen_file(TARGET frontend_test_number_subtypes_gen_ets
    TEMPLATE ${INTEROP_JS_TESTS_SOURCE_DIR}/number_subtypes/number_subtypes_frontend.ets.erb
    OUTPUT ${INTEROP_TESTS_GENERATED_DIR}/number_subtypes_frontend.ets
    REQUIRES ${INTEROP_JS_TESTS_SOURCE_DIR}/number_subtypes/number_subtypes.rb
)

panda_ets_interop_js_gtest(ets_interop_js_test_number_subtypes
    CPP_SOURCES ${INTEROP_TESTS_GENERATED_DIR}/number_subtypes.cpp
    ETS_SOURCES ${INTEROP_TESTS_GENERATED_DIR}/number_subtypes.ets
)

panda_ets_interop_js_gtest(ets_interop_js_frontend_test_number_subtypes
    CPP_SOURCES ${INTEROP_TESTS_GENERATED_DIR}/number_subtypes.cpp
    ETS_SOURCES ${INTEROP_TESTS_GENERATED_DIR}/number_subtypes_frontend.ets
    ETS_CONFIG ${ETS_CONFIG}
)

add_dependencies(ets_interop_js_test_number_subtypes test_number_subtypes_gen_cpp)
add_dependencies(ets_interop_js_test_number_subtypes test_number_subtypes_gen_ets)

add_dependencies(ets_interop_js_frontend_test_number_subtypes test_number_subtypes_gen_cpp)
add_dependencies(ets_interop_js_frontend_test_number_subtypes frontend_test_number_subtypes_gen_ets)