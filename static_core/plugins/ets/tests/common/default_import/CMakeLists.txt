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

run_int_ets_code(default_import_1 ${CMAKE_CURRENT_BINARY_DIR}/default_import_1 SOURCES
                 ${CMAKE_CURRENT_SOURCE_DIR}/default1.ets;
                 ${CMAKE_CURRENT_SOURCE_DIR}/export.ets)

run_int_ets_code(default_import_2 ${CMAKE_CURRENT_BINARY_DIR}/default_import_2 SOURCES
                 ${CMAKE_CURRENT_SOURCE_DIR}/default2.ets;
                 ${CMAKE_CURRENT_SOURCE_DIR}/export.ets)

run_int_ets_code(default_import_3 ${CMAKE_CURRENT_BINARY_DIR}/default_import_3 SOURCES
                 ${CMAKE_CURRENT_SOURCE_DIR}/default3.ets;
                 ${CMAKE_CURRENT_SOURCE_DIR}/export.ets)

run_int_ets_code(default_import_4 ${CMAKE_CURRENT_BINARY_DIR}/default_import_4 SOURCES
                 ${CMAKE_CURRENT_SOURCE_DIR}/default4.ets;
                 ${CMAKE_CURRENT_SOURCE_DIR}/export.ets)

run_int_ets_code(default_import_5 ${CMAKE_CURRENT_BINARY_DIR}/default_import_5 SOURCES
                 ${CMAKE_CURRENT_SOURCE_DIR}/default5.ets;
                 ${CMAKE_CURRENT_SOURCE_DIR}/export.ets)

run_int_ets_code(multiple_default_import ${CMAKE_CURRENT_BINARY_DIR}/multiple_default_import SOURCES
                 ${CMAKE_CURRENT_SOURCE_DIR}/multiple_default_import.ets; 
                 ${CMAKE_CURRENT_SOURCE_DIR}/default_export/default_export_class.ets;
                 ${CMAKE_CURRENT_SOURCE_DIR}/default_export/default_export_interface.ets;
                 ${CMAKE_CURRENT_SOURCE_DIR}/default_export/default_export_function.ets;
                 ${CMAKE_CURRENT_SOURCE_DIR}/default_export/default_export_variable.ets)
