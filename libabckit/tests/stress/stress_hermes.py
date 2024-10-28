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
from typing import List
import logging

import stress_common
from stress_test import Test
from stress import StressJSTest
from stress_common import SCRIPT_DIR, collect_from

FAIL_LIST_PATH = os.path.join(SCRIPT_DIR, 'fail_list_hermes.json')
OPTIONS_LIST_PATH = os.path.join(SCRIPT_DIR, 'options_list.json')
# CC-OFFNXT(WordsTool.66) false positive
HERMES_REVISION = "3feac7b2f9759d83879b04232479041baa805e7b"
HERMES_URL = "https://github.com/facebook/hermes.git"

logging.basicConfig(format='%(message)s', level=logging.DEBUG)


class HermesStressTest(StressJSTest):

    def __init__(self, options_list, args):
        super().__init__(options_list, args)
        self.js_dir = os.path.join(stress_common.TMP_DIR, 'abckit_hermes')

    def prepare(self) -> None:
        self.download_hermes()

    def download_hermes(self) -> None:
        if not os.path.exists(self.js_dir):
            stress_common.stress_exec(['git', 'clone', HERMES_URL, self.js_dir])
            stress_common.stress_exec(['git', '-C', self.js_dir, 'checkout', HERMES_REVISION])

    def collect(self) -> List[str]:
        tests: List[str] = []
        sp = os.path.join(self.js_dir, 'test')
        tests.extend(collect_from(sp, lambda name: name.endswith('.js') and not name.startswith('.')))
        random.shuffle(tests)

        logging.debug('Total tests: %s', len(tests))
        return tests


def main():
    logging.debug('ABCKit stress test')
    args = stress_common.get_args()
    test = HermesStressTest(stress_common.get_options_list(OPTIONS_LIST_PATH), args)
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
