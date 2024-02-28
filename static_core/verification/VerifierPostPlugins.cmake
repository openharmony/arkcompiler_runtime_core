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

set(VERIFIER_INCLUDE_DIR ${PANDA_BINARY_ROOT}/verification/gen/include)
set(PLUGINS_GEN_INC ${VERIFIER_INCLUDE_DIR}/plugins_gen.inc)

panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/verification/gen/templates/plugins_gen.inc.erb
    OUTPUTFILE ${PLUGINS_GEN_INC}
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
)

add_custom_target(verifier_plugin_gen DEPENDS ${PLUGINS_GEN_INC})
add_dependencies(verifier verifier_plugin_gen)
add_dependencies(arkruntime_obj verifier_plugin_gen)
add_dependencies(panda_gen_files verifier_plugin_gen)
