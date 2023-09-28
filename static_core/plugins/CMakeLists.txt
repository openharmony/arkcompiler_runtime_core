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

function(init_plugin plugin_name)
    string(TOUPPER ${plugin_name} plugin_name_upper)

    string(CONCAT PANDA_WITH_PLUGIN "PANDA_WITH_" ${plugin_name_upper})
    if (NOT ${PANDA_WITH_PLUGIN})
        return()
    endif()

    string(CONCAT PLUGIN_SOURCE "PANDA_" ${plugin_name_upper} "_PLUGIN_SOURCE")
    if(EXISTS ${${PLUGIN_SOURCE}}/CMakeLists.txt)
        add_subdirectory(${plugin_name})
    endif()
endfunction()

foreach(plugin ${PLUGINS})
    init_plugin(${plugin})
endforeach()
