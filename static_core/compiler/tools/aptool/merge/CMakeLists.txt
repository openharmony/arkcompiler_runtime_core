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

panda_add_library(ark_aptool_merge OBJECT
    merge_command.cpp
)

panda_target_include_directories(ark_aptool_merge
    PUBLIC ${APTOOL_COMMON_INCLUDE_DIRS}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

panda_target_link_libraries(ark_aptool_merge
    ${APTOOL_COMMON_LIBRARIES}
)

panda_gen_options(TARGET ark_aptool_merge YAML_FILE merge_options.yaml GENERATED_HEADER merge_options.h)
