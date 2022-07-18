#!/bin/bash
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

set -e

# This script is needed beacuse GN can't get output file path for source_set.

SRC_FULL_PATH="$(realpath ${0})"
SRC_DIR="$(dirname ${SRC_FULL_PATH})"
ASM_DEF_SRC=${1}

echo "ASM_DEF_SRC: ${ASM_DEF_SRC}"

${SRC_DIR}/cross_values_getters_generator.rb ${ASM_DEF_SRC} ${@:2}
