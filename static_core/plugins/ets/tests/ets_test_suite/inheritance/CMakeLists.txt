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

set(inheritance_tests
# TODO() Enable when inheritance will be merged to es2panda
#    extend_class
#    impl_interface
#    lambda_interface
)

set(inheritance_tests_in_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(inheritance_tests_out_dir "${CMAKE_CURRENT_BINARY_DIR}")

foreach(test ${inheritance_tests})
    set(test_out_dir "${inheritance_tests_out_dir}/${test}")

    set(test_in "${inheritance_tests_in_dir}/${test}.ets")
    set(target ets_test_suite_inheritance_${test})

    run_int_jit_aot_ets_code(${test_in} ${test_out_dir} ${target})
endforeach()

