# Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

panda_ets_add_gtest(
    NO_CORES
    NAME verifier_ets_instructions_test
    SOURCES 
        async_dispatch_test.cpp
        async_suspend_test.cpp
    LIBRARIES arkruntime arkassembler
    INCLUDE_DIRS ${PANDA_ETS_PLUGIN_SOURCE}/runtime
    SANITIZERS ${PANDA_SANITIZERS_LIST}
    PANDA_STD_LIB "${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc"
    DEPS_TARGETS etsstdlib
)