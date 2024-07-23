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

add_custom_target(code-style-check
    COMMAND ${PANDA_ROOT}/scripts/code_style/code_style_check.py ${PANDA_ROOT}
    USES_TERMINAL
    )

add_custom_target(clang-force-format
    COMMAND ${PANDA_ROOT}/scripts/code_style/code_style_check.py ${PANDA_ROOT} --reformat
    USES_TERMINAL
    )

add_custom_target(doxygen-style-check
    COMMAND ${PANDA_ROOT}/scripts/code_style/doxygen_style_check.py ${PANDA_ROOT}
    USES_TERMINAL
    )
