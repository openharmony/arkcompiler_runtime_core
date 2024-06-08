# Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

if (NOT PANDA_BUILD_LLVM_BACKEND)
    message(FATAL_ERROR "PANDA_BUILD_LLVM_BACKEND must be true")
endif()

if (CROSS_VALUES_CONFIG)
    message(FATAL_ERROR "LLVM_BACKEND must be disabled in cross_values")
endif()

find_package(LLVM 15 REQUIRED CONFIG NO_DEFAULT_PATH CMAKE_FIND_ROOT_PATH_BOTH PATHS ${LLVM_TARGET_PATH})
message(STATUS "LLVM backend:")
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

find_library(LIB_LLVM LLVM REQUIRED NO_DEFAULT_PATH CMAKE_FIND_ROOT_PATH_BOTH PATHS ${LLVM_LIBRARY_DIR})
message(STATUS "LIB_LLVM ${LIB_LLVM}")

if (NOT CMAKE_CROSSCOMPILING AND PANDA_LLVM_INTERPRETER_INLINING)
    find_program(LLVM_LINK NAMES llvm-link REQUIRED NO_DEFAULT_PATH CMAKE_FIND_ROOT_PATH_BOTH PATHS "${LLVM_BINARY_DIR}/bin")
    message(STATUS "LLVM_LINK ${LLVM_LINK}")
endif()

set(LLVM_COPY_NAME "libLLVM-${LLVM_VERSION_MAJOR}.so")
add_custom_target(copy-libLLVM.so
    COMMENT "Copying ${LIB_LLVM} into ${PANDA_BINARY_ROOT}/lib/${LLVM_COPY_NAME}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${LIB_LLVM}" "${PANDA_BINARY_ROOT}/lib/${LLVM_COPY_NAME}")
