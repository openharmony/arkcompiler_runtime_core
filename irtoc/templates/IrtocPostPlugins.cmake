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

set(GENERATED_DIRECTORY ${PANDA_BINARY_ROOT}/irtoc/generated)
file(MAKE_DIRECTORY "${GENERATED_DIRECTORY}")
set(PLUGINS_TXT ${GENERATED_DIRECTORY}/plugins.txt)
panda_gen_file(
    DATAFILE ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/irtoc/templates/plugins.txt.erb
    REQUIRES ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${PLUGINS_TXT}
)

add_custom_target(irtoc_plugins_txt
    DEPENDS plugin_options_gen ${PLUGINS_TXT}
)
