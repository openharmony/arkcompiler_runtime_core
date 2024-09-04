# Copyright (c) 2024 Huawei Device Co., Ltd.
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

if (PANDA_TARGET_ARM32)
    return()
endif()

add_custom_target(recursive_import)

run_int_ets_code(${CMAKE_CURRENT_SOURCE_DIR}/case_0/source0.sts ${CMAKE_CURRENT_BINARY_DIR}/case_0 recursive_import_0 EXPORT_FILES ${CMAKE_CURRENT_SOURCE_DIR}/case_0/source1.sts;${CMAKE_CURRENT_SOURCE_DIR}/case_0/source2.sts)
add_dependencies(recursive_import recursive_import_0)

run_int_ets_code(${CMAKE_CURRENT_SOURCE_DIR}/case_1_fail_not_called_w_module/foo0.sts ${CMAKE_CURRENT_BINARY_DIR}/case_1_fail_not_called_w_module recursive_import_1 EXPORT_FILES ${CMAKE_CURRENT_SOURCE_DIR}/case_1_fail_not_called_w_module/foo1.sts;${CMAKE_CURRENT_SOURCE_DIR}/case_1_fail_not_called_w_module/foo2.sts)
add_dependencies(recursive_import recursive_import_1)

run_int_ets_code(${CMAKE_CURRENT_SOURCE_DIR}/case_2_3-sources-in-circle/foo0.sts ${CMAKE_CURRENT_BINARY_DIR}/case_2_3-sources-in-circle recursive_import_2 EXPORT_FILES ${CMAKE_CURRENT_SOURCE_DIR}/case_2_3-sources-in-circle/foo1.sts;${CMAKE_CURRENT_SOURCE_DIR}/case_2_3-sources-in-circle/foo2.sts;${CMAKE_CURRENT_SOURCE_DIR}/case_2_3-sources-in-circle/foo3.sts)
add_dependencies(recursive_import recursive_import_2)

run_int_ets_code(${CMAKE_CURRENT_SOURCE_DIR}/case_3_3-sources-in-circle-fail-no-ets-module-compile/foo0.sts ${CMAKE_CURRENT_BINARY_DIR}/case_3_3-sources-in-circle-fail-no-ets-module-compile recursive_import_3 EXPORT_FILES ${CMAKE_CURRENT_SOURCE_DIR}/case_3_3-sources-in-circle-fail-no-ets-module-compile/foo1.sts;${CMAKE_CURRENT_SOURCE_DIR}/case_3_3-sources-in-circle-fail-no-ets-module-compile/foo2.sts;${CMAKE_CURRENT_SOURCE_DIR}/case_3_3-sources-in-circle-fail-no-ets-module-compile/foo3.sts)
add_dependencies(recursive_import recursive_import_3)

run_int_ets_code(${CMAKE_CURRENT_SOURCE_DIR}/case_4_3-sources-in-circle_no-ets-module-compile_2/foo0.sts ${CMAKE_CURRENT_BINARY_DIR}/case_4_3-sources-in-circle_no-ets-module-compile_2 recursive_import_4 EXPORT_FILES ${CMAKE_CURRENT_SOURCE_DIR}/case_4_3-sources-in-circle_no-ets-module-compile_2/foo1.sts;${CMAKE_CURRENT_SOURCE_DIR}/case_4_3-sources-in-circle_no-ets-module-compile_2/foo2.sts;${CMAKE_CURRENT_SOURCE_DIR}/case_4_3-sources-in-circle_no-ets-module-compile_2/foo3.sts)
add_dependencies(recursive_import recursive_import_4)

run_int_ets_code(${CMAKE_CURRENT_SOURCE_DIR}/case_5_main-is-not-part-of-circle/foo0.sts ${CMAKE_CURRENT_BINARY_DIR}/case_5_main-is-not-part-of-circle recursive_import_5 EXPORT_FILES ${CMAKE_CURRENT_SOURCE_DIR}/case_5_main-is-not-part-of-circle/foo1.sts;${CMAKE_CURRENT_SOURCE_DIR}/case_5_main-is-not-part-of-circle/foo2.sts;${CMAKE_CURRENT_SOURCE_DIR}/case_5_main-is-not-part-of-circle/foo3.sts)
add_dependencies(recursive_import recursive_import_5)

# uncomment when internal issue: #18780 is resolved
# run_int_ets_code(${CMAKE_CURRENT_SOURCE_DIR}/recursive_import_binds_all.sts ${CMAKE_CURRENT_BINARY_DIR} recursive_import_binds_all
#                  EXPORT_FILES ${CMAKE_CURRENT_SOURCE_DIR}/packages/recursive/package_module_1.sts;${CMAKE_CURRENT_SOURCE_DIR}/packages/recursive/subpackage/subpackage_module_1.sts
# )
