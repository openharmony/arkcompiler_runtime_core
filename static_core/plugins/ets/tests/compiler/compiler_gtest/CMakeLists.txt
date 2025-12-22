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

panda_add_library(compiler_gtest INTERFACE)
panda_target_link_libraries(compiler_gtest INTERFACE gtest)
add_dependencies(compiler_gtest ark_aot)

panda_target_include_directories(compiler_gtest
    INTERFACE ${PANDA_ETS_PLUGIN_SOURCE}/tests/compiler/compiler_gtest/
    INTERFACE ${PANDA_ROOT}
)
