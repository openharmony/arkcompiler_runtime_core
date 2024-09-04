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

set(ETS_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/arktsconfig.json)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.in.json ${ETS_CONFIG})

panda_ets_interop_js_checked_test(FILE ${CMAKE_CURRENT_LIST_DIR}/example/strings.sts)
panda_ets_interop_js_checked_test(FILE ${CMAKE_CURRENT_LIST_DIR}/js_call/js_call.sts)
panda_ets_interop_js_checked_test(FILE ${CMAKE_CURRENT_LIST_DIR}/obj_call/obj_call.sts)
panda_ets_interop_js_checked_test(FILE ${CMAKE_CURRENT_LIST_DIR}/../class_operations/class_operations_frontend.sts)
panda_ets_interop_js_checked_test(FILE ${CMAKE_CURRENT_LIST_DIR}/merge_local_scopes/merge_local_scopes.sts)
panda_ets_interop_js_checked_test(FILE ${CMAKE_CURRENT_LIST_DIR}/peepholes/peepholes.sts)
panda_ets_interop_js_checked_test(FILE ${CMAKE_CURRENT_LIST_DIR}/qualified_call/qualified_call.sts)

set(INTEROP_TESTS_GENERATED_DIR "${PANDA_BINARY_ROOT}/plugins/ets/tests/ets_interop_js/generated")
panda_ets_interop_js_checked_test(FILE ${INTEROP_TESTS_GENERATED_DIR}/number_subtypes_frontend.sts)

gen_file(TARGET test_convert_gen_ets
    TEMPLATE ${CMAKE_CURRENT_LIST_DIR}/convert/convert.sts.erb
    OUTPUT ${INTEROP_TESTS_GENERATED_DIR}/convert.sts
    REQUIRES ${CMAKE_CURRENT_LIST_DIR}/convert/convert.rb
)
gen_file(TARGET test_convert_gen_js
    TEMPLATE ${CMAKE_CURRENT_LIST_DIR}/convert/convert.js.erb
    OUTPUT ${INTEROP_TESTS_GENERATED_DIR}/convert.js
    REQUIRES ${CMAKE_CURRENT_LIST_DIR}/convert/convert.rb
)
panda_ets_interop_js_checked_test(FILE ${INTEROP_TESTS_GENERATED_DIR}/convert.sts)
add_dependencies(interop_convert.checked test_convert_gen_ets)
add_dependencies(interop_convert.checked test_convert_gen_js)