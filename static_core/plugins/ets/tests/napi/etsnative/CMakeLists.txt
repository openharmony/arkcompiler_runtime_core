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

set(PANDA_STD_LIB "${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc")

panda_ets_add_gtest(
    NO_CORES
    NAME etsnative_test
    SOURCES etsnative_test.cpp
    LIBRARIES etsnative
    SANITIZERS ${PANDA_SANITIZERS_LIST}
    CUSTOM_PRERUN_ENVIRONMENT
            "ARK_ETS_STDLIB_PATH=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc"
    DEPS_TARGETS etsstdlib
)

if (PANDA_TARGET_ARM32)
    return()
endif()

panda_ets_add_gtest(
    NO_CORES
    NAME etsnative_library_path_test
    SOURCES etsnative_library_path_test.cpp
    LIBRARIES etsnative
    SANITIZERS ${PANDA_SANITIZERS_LIST}
    CUSTOM_PRERUN_ENVIRONMENT
            "ARK_ETS_STDLIB_PATH=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc"
            "ARK_ETS_LIBRARY_PATH=${PANDA_BINARY_ROOT}/plugins/ets/tests/napi/lib"
    DEPS_TARGETS etsstdlib etsnapi_lib
)
