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

# NOTE: Enable gtest for AARCH64 when #22303 is solved.
if (PANDA_COMPILER_TARGET_AARCH64)
    return()
endif()

ani_add_gtest(ani_test_constructor_native_library
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/constructor_native_library_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/constructor_native_library_test.ets
)

# Build rules for ctor_test native shared library
panda_add_library(ctor_test SHARED ctor_test.cpp)
panda_target_include_directories(ctor_test PRIVATE ${PANDA_ETS_PLUGIN_SOURCE}/runtime/ani)
add_dependencies(ani_test_constructor_native_library ctor_test)
