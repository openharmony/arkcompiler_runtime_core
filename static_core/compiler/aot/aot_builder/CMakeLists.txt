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

project(aot_builder)

set(AOT_BUILDER_SOURCES aot_builder.cpp)

if (PANDA_LLVM_AOT)
    list(APPEND AOT_BUILDER_SOURCES llvm_aot_builder.cpp)
endif()

panda_add_library(aot_builder STATIC ${AOT_BUILDER_SOURCES})

panda_target_link_libraries(aot_builder arkbase arkfile arkcompiler)
set_target_properties(aot_builder PROPERTIES POSITION_INDEPENDENT_CODE ON)

panda_set_lib_32bit_property(aot_builder)
