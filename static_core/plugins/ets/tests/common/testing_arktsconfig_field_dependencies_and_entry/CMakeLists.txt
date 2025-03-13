# Copyright (c) 2025 Huawei Device Co., Ltd.
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

run_int_ets_code(testing_arktsconfig_field_dependencies_and_entry ${CMAKE_CURRENT_BINARY_DIR}
                 ENTRYPOINT "HarA.a1"
                 SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/A/src/main/ets/a1.ets ${CMAKE_CURRENT_SOURCE_DIR}/A/indexA.ets ${CMAKE_CURRENT_SOURCE_DIR}/B/src/main/ets/pages/b.ets ${CMAKE_CURRENT_SOURCE_DIR}/B/indexB.ets ${CMAKE_CURRENT_SOURCE_DIR}/C/src/main/ets/pages/c.ets ${CMAKE_CURRENT_SOURCE_DIR}/C/indexC.ets
                 ARKTS_CONFIGS ${CMAKE_CURRENT_SOURCE_DIR}/A/arktsconfig.json ${CMAKE_CURRENT_SOURCE_DIR}/A/arktsconfig.json ${CMAKE_CURRENT_SOURCE_DIR}/B/arktsconfig.json ${CMAKE_CURRENT_SOURCE_DIR}/B/arktsconfig.json ${CMAKE_CURRENT_SOURCE_DIR}/C/arktsconfig.json ${CMAKE_CURRENT_SOURCE_DIR}/C/arktsconfig.json
                )