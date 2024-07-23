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

get_target_property(INST_TEMPLATES_YAML_FILES inst_templates_gen INST_TEMPLATES_YAML_FILES)
get_target_property(INST_TEMPLATES_GEN_YAML inst_templates_gen INST_TEMPLATES_GEN_YAML)

add_custom_command(OUTPUT ${INST_TEMPLATES_GEN_YAML}
    COMMENT "Merge yaml files: ${INST_TEMPLATES_YAML_FILES}"
    COMMAND ${PANDA_ROOT}/templates/concat_yamls.sh "${INST_TEMPLATES_GEN_YAML}" ${INST_TEMPLATES_YAML_FILES}
    DEPENDS ${INST_TEMPLATES_YAML_FILES}
)
add_custom_target(inst_templates_merge DEPENDS ${INST_TEMPLATES_GEN_YAML})
