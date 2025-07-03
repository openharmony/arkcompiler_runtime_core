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

set(DECLGEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/declgen_ts2sts")
set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/plugins/ets/tools/declgen_ts2sts")

add_custom_target(build_declgen_ts2sts
    COMMAND ${CMAKE_COMMAND} -E echo "Building declgen_ts2sts..."
    COMMAND ${CMAKE_COMMAND} -E chdir ${DECLGEN_DIR} npm install
    COMMAND ${CMAKE_COMMAND} -E chdir ${DECLGEN_DIR} npm run build
    COMMAND ${CMAKE_COMMAND} -E chdir ${DECLGEN_DIR} npm run webpack
    COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}/build
    COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}/dist
    COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}/node_modules
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${DECLGEN_DIR}/build ${OUTPUT_DIR}/build
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${DECLGEN_DIR}/build
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${DECLGEN_DIR}/dist ${OUTPUT_DIR}/dist
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${DECLGEN_DIR}/dist
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${DECLGEN_DIR}/node_modules ${OUTPUT_DIR}/node_modules
    COMMAND ${CMAKE_COMMAND} -E copy ${DECLGEN_DIR}/package.json ${OUTPUT_DIR}/package.json
    COMMENT "Building declgen_ts2sts with npm"
)

add_custom_target(default_target ALL
    DEPENDS build_declgen_ts2sts
)