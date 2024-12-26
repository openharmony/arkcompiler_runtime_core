#!/bin/bash
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

set -e

declare -a TEST_LIST=(
    "basic_interop_object_collect_weak_ref_test.ts"
    "cyclic_reference_between_vm_test.ts"
    "cyclic_reference_between_vm_string_test.ts"
    "cyclic_reference_between_vm_array_test.ts"
    "cyclic_reference_vm_weak_ref_test.ts"
    "cyclic_reference_vm_weak_ref_sts_test.ts"
    "extend_class_imported_and_weak_ref_test.ts"
    "extend_class_imported_and_weak_ref_sts_test.ts"
    "extend_class_imported_from_panda_vm_test.ts"
    "js_2_sts_obj_pass_check_mem_test.ts"
)
declare TEST_PATH=""
declare BUILD_ROOT=""
declare TS_COMMON_PATH="gc_test_common.ts"
declare STS_TEST_PATH="sts/gc_test_sts_common.sts"
declare TEST_RUN_RESULT=0
while [[ -n "$1" ]]; do
    case "$1" in
        -b|--build-dir)
            shift
            BUILD_ROOT="$1"
            ;;
        --tests)
            shift
            TEST_PATH="$1"
            ;;
        *)
            echo "Unknow option '$1'"
            ;;
    esac
    shift
done
for TEST in "${TEST_LIST[@]}"; do
    test_exit_code=0
    ./run_hybrid_test.sh -b "${BUILD_ROOT}" --ts-test "${TEST_PATH}${TEST}" --ts-common "${TEST_PATH}${TS_COMMON_PATH}" --sts-test "${TEST_PATH}${STS_TEST_PATH}" || test_exit_code=$?
    if [ $test_exit_code -ne 0 ]; then
        echo "=============================================="
        echo "Test $TEST failed with code: $test_exit_code!"
        echo "=============================================="
        TEST_RUN_RESULT=255
    else
        echo "Test $TEST passed!"
    fi
done
echo "Summary: "
if [ $TEST_RUN_RESULT -ne 0 ]; then
    echo "Some test failed!"
    exit $TEST_RUN_RESULT
else
    echo "All tests passed!"
    exit 0
fi
