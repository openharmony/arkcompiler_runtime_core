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

project(aptool)

panda_add_executable(ark_aptool
    main.cpp
    common/utils.cpp
    common/profile_loader.cpp
    common/abc_metadata.cpp
    dump/filter.cpp
    dump/yaml_writer.cpp
    dump/dump_command.cpp
)

panda_target_link_libraries(ark_aptool
    arkbase
    arkruntime
    arkfile
    arkassembler
)

panda_target_include_directories(ark_aptool
    PUBLIC ${PANDA_ROOT}
    PUBLIC ${PANDA_ROOT}/runtime
    PUBLIC ${PANDA_ROOT}/libarkfile
    PUBLIC ${PANDA_ROOT}/assembler
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

panda_gen_options(TARGET ark_aptool YAML_FILE dump/dump_options.yaml GENERATED_HEADER dump_options.h)

panda_add_sanitizers(TARGET ark_aptool SANITIZERS ${PANDA_SANITIZERS_LIST})
