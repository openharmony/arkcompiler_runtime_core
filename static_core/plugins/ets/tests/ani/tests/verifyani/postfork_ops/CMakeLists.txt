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

ani_verify_add_gtest(ani_verify_test_postfork_initialize
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_postfork_initialize_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_postfork_initialize_test.ets
)

ani_verify_add_gtest(ani_verify_test_postfork_raw_weak_ref
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_postfork_raw_weak_ref_test.cpp
)

ani_verify_add_gtest(ani_verify_test_postfork_raw_local_ref
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/verify_postfork_raw_local_ref_test.cpp
)

# Post-fork tests late-init Verify ANI via runtime helpers. When runtime symbols are hidden,
# link the static archive instead.
if(PANDA_WITH_HIDDEN_SYMBOLS)
    foreach(_postfork_test ani_verify_test_postfork_raw_weak_ref ani_verify_test_postfork_raw_local_ref)
        if(TARGET ${_postfork_test})
            get_target_property(_postfork_test_libs ${_postfork_test} LINK_LIBRARIES)
            if(_postfork_test_libs)
                list(FILTER _postfork_test_libs EXCLUDE REGEX "^arkruntime$")
                list(APPEND _postfork_test_libs arkruntime_static)
                set_target_properties(${_postfork_test} PROPERTIES LINK_LIBRARIES "${_postfork_test_libs}")
            endif()
        endif()
    endforeach()
endif()
