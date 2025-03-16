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

if (PANDA_TARGET_ARM32)
    return()
endif()

add_custom_target(recursive_import)

run_int_ets_code(recursive_import_0 ${CMAKE_CURRENT_BINARY_DIR}/case_0
                 ENTRYPOINT "source0" SOURCES
                 ${CMAKE_CURRENT_SOURCE_DIR}/case_0/source0.ets;
                 ${CMAKE_CURRENT_SOURCE_DIR}/case_0/source1.ets;
                 ${CMAKE_CURRENT_SOURCE_DIR}/case_0/source2.ets)
add_dependencies(recursive_import recursive_import_0)

run_int_ets_code(recursive_import_1 ${CMAKE_CURRENT_BINARY_DIR}/case_1_fail_not_called_w_module
                 ENTRYPOINT "foo0" SOURCES
                 ${CMAKE_CURRENT_SOURCE_DIR}/case_1_fail_not_called_w_module/foo0.ets;
                 ${CMAKE_CURRENT_SOURCE_DIR}/case_1_fail_not_called_w_module/foo1.ets;
                 ${CMAKE_CURRENT_SOURCE_DIR}/case_1_fail_not_called_w_module/foo2.ets)
add_dependencies(recursive_import recursive_import_1)

run_int_ets_code(recursive_import_2 ${CMAKE_CURRENT_BINARY_DIR}/case_2_3-sources-in-circle
                 ENTRYPOINT "foo0" SOURCES
                 ${CMAKE_CURRENT_SOURCE_DIR}/case_2_3-sources-in-circle/foo0.ets;
                 ${CMAKE_CURRENT_SOURCE_DIR}/case_2_3-sources-in-circle/foo1.ets;
                 ${CMAKE_CURRENT_SOURCE_DIR}/case_2_3-sources-in-circle/foo2.ets;
                 ${CMAKE_CURRENT_SOURCE_DIR}/case_2_3-sources-in-circle/foo3.ets)
add_dependencies(recursive_import recursive_import_2)

run_int_ets_code(recursive_import_3 ${CMAKE_CURRENT_BINARY_DIR}/case_3_3-sources-in-circle-fail-no-ets-module-compile
                 ENTRYPOINT "foo0" SOURCES
                 ${CMAKE_CURRENT_SOURCE_DIR}/case_3_3-sources-in-circle-fail-no-ets-module-compile/foo0.ets;
                 ${CMAKE_CURRENT_SOURCE_DIR}/case_3_3-sources-in-circle-fail-no-ets-module-compile/foo1.ets;
                 ${CMAKE_CURRENT_SOURCE_DIR}/case_3_3-sources-in-circle-fail-no-ets-module-compile/foo2.ets;
                 ${CMAKE_CURRENT_SOURCE_DIR}/case_3_3-sources-in-circle-fail-no-ets-module-compile/foo3.ets)
add_dependencies(recursive_import recursive_import_3)

run_int_ets_code(recursive_import_4 ${CMAKE_CURRENT_BINARY_DIR}/case_4_3-sources-in-circle_no-ets-module-compile_2
                 ENTRYPOINT "foo0" SOURCES
                 ${CMAKE_CURRENT_SOURCE_DIR}/case_4_3-sources-in-circle_no-ets-module-compile_2/foo0.ets;
                 ${CMAKE_CURRENT_SOURCE_DIR}/case_4_3-sources-in-circle_no-ets-module-compile_2/foo1.ets;
                 ${CMAKE_CURRENT_SOURCE_DIR}/case_4_3-sources-in-circle_no-ets-module-compile_2/foo2.ets;
                 ${CMAKE_CURRENT_SOURCE_DIR}/case_4_3-sources-in-circle_no-ets-module-compile_2/foo3.ets)
add_dependencies(recursive_import recursive_import_4)

run_int_ets_code(recursive_import_5 ${CMAKE_CURRENT_BINARY_DIR}/case_5_main-is-not-part-of-circle
                 ENTRYPOINT "foo0" SOURCES
                 ${CMAKE_CURRENT_SOURCE_DIR}/case_5_main-is-not-part-of-circle/foo0.ets;
                 ${CMAKE_CURRENT_SOURCE_DIR}/case_5_main-is-not-part-of-circle/foo1.ets;
                 ${CMAKE_CURRENT_SOURCE_DIR}/case_5_main-is-not-part-of-circle/foo2.ets;
                 ${CMAKE_CURRENT_SOURCE_DIR}/case_5_main-is-not-part-of-circle/foo3.ets)
add_dependencies(recursive_import recursive_import_5)

run_int_ets_code(recursive_import_binds_all ${CMAKE_CURRENT_BINARY_DIR}
                 ENTRYPOINT "recursive_import_binds_all" SOURCES
                 ${CMAKE_CURRENT_SOURCE_DIR}/recursive_import_binds_all.ets;
                 ${CMAKE_CURRENT_SOURCE_DIR}/packages/recursive/package_module_1.ets;
                 ${CMAKE_CURRENT_SOURCE_DIR}/packages/recursive/subpackage/subpackage_module_1.ets)
add_dependencies(recursive_import recursive_import_binds_all)
