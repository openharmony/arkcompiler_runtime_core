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

panda_add_library(etsnative SHARED etsnative.cpp)

panda_target_include_directories(etsnative PUBLIC ${PANDA_ROOT})
panda_target_link_libraries(etsnative arkruntime arkbase)

add_custom_target(etsnative_symbols_test
                  COMMENT "Test that all ETS NAPI symbols exists"
                  COMMAND nm $<TARGET_FILE:etsnative> | grep -w 'T[[:space:]]ETS_CreateVM' > /dev/null
                  COMMAND nm $<TARGET_FILE:etsnative> | grep -w 'T[[:space:]]ETS_GetCreatedVMs' > /dev/null
                  COMMAND nm $<TARGET_FILE:etsnative> | grep -w 'T[[:space:]]ETS_GetDefaultVMInitArgs' > /dev/null
                  DEPENDS etsnative)
add_dependencies(ets_tests etsnative_symbols_test)
