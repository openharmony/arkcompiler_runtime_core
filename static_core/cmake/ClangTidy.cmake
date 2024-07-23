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

# There seems to be a bug in either clang-tidy or CMake:
# When clang/gcc is used for cross-compilation, it is ran on host and use defines and options for host
# For example for arm32 cross-compilation Clang-Tidy:
#   - don't know about -march=armv7-a
#   - believes that size of pointer is 64 instead of 32 for aarch32
# TODO: Retry once we upgrade the checker.


add_custom_target(clang-tidy-check
  COMMAND ${PANDA_ROOT}/scripts/clang-tidy/clang_tidy_check.py ${PANDA_ROOT} ${PANDA_BINARY_ROOT}
  USES_TERMINAL
  DEPENDS panda_gen_files
)

add_custom_target(clang-tidy-check-full
  COMMAND ${PANDA_ROOT}/scripts/clang-tidy/clang_tidy_check.py --full ${PANDA_ROOT} ${PANDA_BINARY_ROOT}
  USES_TERMINAL
  DEPENDS panda_gen_files
)

add_custom_target(cmake-checker
  COMMAND ${PANDA_ROOT}/scripts/cmake-checker/cmake_checker.py ${PANDA_ROOT}
  USES_TERMINAL
)

add_custom_target(test-cmake-checker
  COMMAND ${PANDA_ROOT}/scripts/cmake-checker/cmake_checker.py ${PANDA_ROOT} TEST
  USES_TERMINAL
)
