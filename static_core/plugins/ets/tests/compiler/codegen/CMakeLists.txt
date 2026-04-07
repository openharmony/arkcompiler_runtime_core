# Copyright (c) 2026 Huawei Device Co., Ltd.
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

if(PANDA_COMPILER_ENABLE AND NOT PANDA_MINIMAL_VIXL AND PANDA_TARGET_AMD64)
    panda_ets_add_gtest(
        CONTAINS_MAIN
        NAME ets_tests_compiler_codegen
        SOURCES
            ${PANDA_ROOT}/compiler/tests/unit_test.cpp
            savestate_suspend_code_info_test.cpp
        LIBRARIES
            arkcompiler arkruntime
        SANITIZERS
            ${PANDA_SANITIZERS_LIST}
    )
endif()
