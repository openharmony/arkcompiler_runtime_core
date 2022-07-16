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

cmake_minimum_required(VERSION 3.5.2 FATAL_ERROR)

# NB! This order of adding plugins is important, it should be the same
# as in ark_config.gni's enabled_plugins list

if (PANDA_WITH_ECMASCRIPT AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ecmascript")
    add_subdirectory(ecmascript)
elseif(PANDA_WITH_ECMASCRIPT)
    message(FATAL_ERROR "ecmascript plugin is missing")
endif()

if (PANDA_WITH_ACCORD AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/accord")
    add_subdirectory(accord)
elseif(PANDA_WITH_ACCORD)
    message(FATAL_ERROR "accord plugin is missing")
endif()

if (PANDA_WITH_CANGJIE AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cangjie")
    add_subdirectory(cangjie)
elseif(PANDA_WITH_CANGJIE)
    message(FATAL_ERROR "cangjie plugin is missing")
endif()

if (PANDA_WITH_JAVA AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/java")
    add_subdirectory(java)
elseif(PANDA_WITH_JAVA)
    message(FATAL_ERROR "java plugin is missing")
endif()

if (PANDA_WITH_TESTS AND PANDA_WITH_RUNTIME AND PANDA_WITH_JAVA AND PANDA_WITH_ECMASCRIPT AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests_java_ecmascript")
    add_subdirectory(tests_java_ecmascript)
endif()