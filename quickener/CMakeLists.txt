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

cmake_minimum_required(VERSION 3.10)

project(quickener)

set(QUICK_SOURCES
    quick.cpp
)

panda_add_executable(arkquick ${QUICK_SOURCES})

target_link_libraries(arkquick arkbase arkfile)

add_dependencies(arkquick arkbase arkfile)

panda_add_to_clang_tidy(TARGET arkquick CHECKS
    # "Inherited" from assembler:
    "-cert-dcl21-cpp"
    "-cppcoreguidelines-macro-usage"
    "-google-runtime-references"
    "-misc-non-private-member-variables-in-classes"
    # Component-specific:
    "-fuchsia-statically-constructed-objects"
    "-readability-identifier-naming"
)

panda_add_sanitizers(TARGET arkquick SANITIZERS ${PANDA_SANITIZERS_LIST})

add_check_style(".")
