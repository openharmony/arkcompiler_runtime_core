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

run_int_ets_code(import_name_conflicts ${CMAKE_CURRENT_BINARY_DIR} SOURCES
                 ${CMAKE_CURRENT_SOURCE_DIR}/main.ets;
                 ${CMAKE_CURRENT_SOURCE_DIR}/imported_module_1.ets;
                 ${CMAKE_CURRENT_SOURCE_DIR}/imported_module_2.ets)
