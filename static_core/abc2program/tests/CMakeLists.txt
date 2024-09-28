# Copyright (c) 2024 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/sts)

set(abc2progTestFiles
    "HelloWorld"
    "Functions"
)

add_custom_target(abc2program_gen)

foreach(NAME ${abc2progTestFiles})
    add_custom_target(abc2program_gen_${NAME} COMMENT "Generating ${NAME} for abc2program"
        COMMAND ${PANDA_BINARY_ROOT}/bin/es2panda ${CMAKE_CURRENT_SOURCE_DIR}/sts/${NAME}.sts
        --output=${CMAKE_CURRENT_BINARY_DIR}/sts/${NAME}.abc
        DEPENDS es2panda)
    add_dependencies(abc2program_gen abc2program_gen_${NAME})
endforeach()

panda_add_gtest(
    NAME abc2program_tests
    SOURCES
        abc2program_test.cpp
    LIBRARIES
        abc2program
    SANITIZERS
        ${PANDA_SANITIZERS_LIST}
    DEPS_TARGETS
        abc2program_gen
)
