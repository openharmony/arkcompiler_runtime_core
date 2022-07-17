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

project(aot_builder)

add_library(aot_builder STATIC
    aot_builder.cpp
)

target_link_libraries(aot_builder arkbase arkfile arkcompiler)

panda_set_lib_32bit_property(aot_builder)

add_check_style(".")

panda_add_to_clang_tidy(TARGET aot_builder)
