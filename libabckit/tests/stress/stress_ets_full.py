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
import random
import sys
from pathlib import Path
from typing import List
import logging

import stress_common
from stress_ets import EtsStressTest
from stress_test import Test, Result
from stress_common import SCRIPT_DIR, collect_from

FAIL_LIST_PATH = os.path.join(SCRIPT_DIR, 'fail_list_ets_full.json')

logging.basicConfig(format='%(message)s', level=logging.DEBUG)


class StressEtsFull(EtsStressTest):
    def __init__(self, ark_path: str, args) -> None:
        super().__init__(args)
        self.jvm = ark_path

    def prepare(self) -> None:
        self.download_ets()

    def collect(self) -> List[str]:
        tests: List[str] = []
        tests.extend(collect_from(self.ets_dir, lambda name: name.endswith('.sts') and not name.startswith('.')))

        random.shuffle(tests)
        logging.debug('Total tests: %s', len(tests))
        return tests

    def run_single(self, test: Test) -> Result:
        result: Result = super().run_single(test)
        if result.result != "0":
            return result
        test_result_one = self.run_ark(test)
        if test_result_one.result != 0:
            # no error as test failed before stress execution
            return Result(test.source, "0", result.stdout, result.stderr)

        test2 = Test(test.source, test.abc + ".stress.abc")
        test_result_two = self.run_ark(test2)
        if test_result_two.result == 0:
            return Result(test.source, "0")

        if test_result_one.result != test_result_two.result:
            msg = f'ETS Test result changed. Was {test_result_one.result}, now {test_result_two.result}'
            return Result(test.source, msg)
        return result

    def run_ark(self, test: Test) -> Result:
        ep = f"{Path(test.source).stem}.ETSGLOBAL::main"
        boot_panda_files = f'--boot-panda-files={self.sp}'
        cmd = [self.jvm, boot_panda_files, "--load-runtimes=ets", test.abc, ep]
        result = stress_common.stress_exec(cmd, allow_error=True, print_command=False, print_output=False)
        return Result(test.source, result.returncode)


def main():
    logging.debug('ABCKit stress test')
    args = stress_common.get_args()
    test = StressEtsFull(args.ark_path, args)
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
        return 1

    logging.debug('ABCKit: no regressions')
    logging.debug('Failures/Total: %s/%s', len(fail_list), len(results))
    return 0


if __name__ == '__main__':
    sys.exit(main())
