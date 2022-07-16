# Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

project(aot_manager)

add_library(arkaotmanager SHARED aot_file.cpp aot_manager.cpp)

target_link_libraries(arkaotmanager arkbase arkfile arkcompiler)

add_dependencies(arkaotmanager intrinsics_gen_compiler isa_gen_compiler entrypoints_gen arkbase)
target_include_directories(arkaotmanager
    PUBLIC ${GENERATED_DIR}
    PUBLIC ${PANDA_ROOT}/runtime
    )

if(PANDA_WITH_TESTS AND TARGET arkruntime_test_interpreter_impl)
    target_include_directories(arkaotmanager
            PUBLIC "$<TARGET_PROPERTY:arkruntime,INTERFACE_INCLUDE_DIRECTORIES>"
            PUBLIC "$<TARGET_PROPERTY:arkruntime_test_interpreter_impl,INTERFACE_INCLUDE_DIRECTORIES>"
            )
else()
    target_include_directories(arkaotmanager
        PUBLIC "$<TARGET_PROPERTY:arkruntime,INTERFACE_INCLUDE_DIRECTORIES>"
        )
endif()

panda_add_sanitizers(TARGET arkaotmanager SANITIZERS ${PANDA_SANITIZERS_LIST})
panda_add_to_clang_tidy(TARGET arkaotmanager)

add_check_style(".")

