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

function(ani_helpers_add_gtest TARGET)
    cmake_parse_arguments(
        ARG # give prefix `ARG` to each argument
        ""
        "ETS_CONFIG"
        "CPP_SOURCES;ETS_SOURCES;LIBRARIES"
        ${ARGN}
    )
    ani_add_gtest(${TARGET}
        ETS_CONFIG ${ARG_ETS_CONFIG}
        CPP_SOURCES ${ARG_CPP_SOURCES}
        ETS_SOURCES ${ARG_ETS_SOURCES}
        LIBRARIES ${ARG_LIBRARIES} ani_helpers
    )
    panda_target_include_directories(${TARGET} PRIVATE
        ${PANDA_ROOT}/plugins/ets/runtime/ani
    )
endfunction(ani_helpers_add_gtest)

set(EXCLUDED_TEST_PACKAGES)

SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_LIST_DIR} EXCLUDED_TEST_PACKAGES)

foreach(SUBDIR ${SUBDIRS})
    get_filename_component(TEST_PACKAGE_NAME ${SUBDIR} NAME)
    add_subdirectory(${SUBDIR})
endforeach()
