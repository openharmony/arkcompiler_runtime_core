#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
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
#
import os
import sys
from typing import List
import logging

import stress_common
from stress_hermes import HermesStressTest
from stress_test import Test, Result
from stress_common import SCRIPT_DIR

FAIL_LIST_PATH = os.path.join(SCRIPT_DIR, 'fail_list_hermes_full.json')
OPTIONS_LIST_PATH = os.path.join(SCRIPT_DIR, 'options_list.json')

logging.basicConfig(format='%(message)s', level=logging.DEBUG)


class HermesStressTestFull(HermesStressTest):
    def __init__(self, options_list, args):
        super().__init__(options_list, args)
        self.build_dir = args.build_dir

    def run_single(self, test: Test) -> Result:
        stress_abc = test.abc + '.stress.abc'
        r1p = Test(test.source, test.abc)
        r2p = Test(test.source, stress_abc)

        test_result_one = self.run_js_test_single(r1p)  # Run test once

        cmd = [stress_common.STRESS, '--input-file', test.abc, '--output-file', stress_abc]
        result = stress_common.stress_exec(cmd, allow_error=True, print_command=False, env={"OUT_DIR": self.build_dir})
        if result.returncode != 0:
            error = stress_common.parse_stdout(result.returncode, result.stdout)
            return Result(test.source, error)
        # Stress test passed

        if test_result_one.result == -1:  # First attempt JS Test failed with timeout. This bypass next test
            return Result(test.source, "0")

        test_result_two = self.run_js_test_single(r2p, self.repeats)  # Run test with defined repeats

        if test_result_two.result == 0:
            return Result(test.source, "0")

        if test_result_one.result != test_result_two.result:
            msg = f'JS Test result changed. Was {test_result_one.result}, now {test_result_two.result}'
            return Result(test.source, msg)

        return Result(test.source, "0")


def main():
    logging.debug('ABCKit stress test')
    args = stress_common.get_args()
    test = HermesStressTestFull(stress_common.get_options_list(OPTIONS_LIST_PATH), args)
    test.prepare()

    tests: List[Test] = test.build()
    results = test.run(tests)

    fail_list = stress_common.get_fail_list(results)

    if args.update_fail_list:
        stress_common.update_fail_list(FAIL_LIST_PATH, fail_list)
        logging.debug('Failures/Total: %s/%s', len(fail_list), len(results))
        return 0

    if not stress_common.check_regression_errors(FAIL_LIST_PATH, fail_list):
        return 1

    if not stress_common.check_fail_list(FAIL_LIST_PATH, fail_list):
        logging.debug('Failures/Total: %s/%s', len(fail_list), len(results))

    logging.debug('ABCKit: no regressions')
    logging.debug('Failures/Total: %s/%s', len(fail_list), len(results))
    return 0


if __name__ == '__main__':
    sys.exit(main())
