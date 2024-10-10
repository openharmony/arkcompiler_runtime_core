# Copyright (c) 2024 Huawei Device Co., Ltd.
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

set(union_tests
    union_class_01
)

set(union_tests_in_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(union_tests_out_dir "${CMAKE_CURRENT_BINARY_DIR}")

foreach(test ${union_tests})
    set(test_out_dir "${union_tests_out_dir}/${test}")

    set(test_src "${union_tests_in_dir}/${test}.sts")
    set(target ets_test_suite_union_${test})

    run_ets_code_verifier(${test_src} ${test_out_dir} ${target}-ets-verifier)
endforeach()
