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

# NOTE: Enable gtest for AARCH64 when #22303 is solved.
if (PANDA_COMPILER_TARGET_AARCH64)
    return()
endif()

ani_add_gtest(ani_test_example_gc_guard_native
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/gc_guard_native_demo_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/gc_guard_native_demo.ets
)

# This is the native library loaded by gc_guard_native_demo.ets.  Linking it
# to ani_helpers models how an internal ArkUI/Web native library consumes
# arkts::GCGuard from the SDK runtime helper library.
panda_add_library(gc_guard_native_demo SHARED gc_guard_native_demo_library.cpp)
panda_target_include_directories(gc_guard_native_demo
    PRIVATE ${PANDA_ETS_PLUGIN_SOURCE}/runtime/ani
    PRIVATE ${PANDA_ROOT}
)
panda_target_link_libraries(gc_guard_native_demo ani_helpers arkruntime)

add_dependencies(ani_test_example_gc_guard_native gc_guard_native_demo)
