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
from stress import StressJSTest
from stress_common import SCRIPT_DIR, TMP_DIR
from stress_test import StressTest, Test, Result

FAIL_LIST_PATH = os.path.join(SCRIPT_DIR, 'fail_list_node.json')
OPTIONS_LIST_PATH = os.path.join(SCRIPT_DIR, 'options_list.json')

NODE_GIT_URL = "https://github.com/nodejs/node.git"
NODE_GIT_HASH = "0095726bf3d0a2c01062d98e087526299e709515"

logging.basicConfig(format='%(message)s', level=logging.DEBUG)


class NodeJSTest(StressJSTest):
    def __init__(self, options_list, args):
        super().__init__(options_list, args)
        self.js_dir = os.path.join(TMP_DIR, 'abckit_nodejs')

    def prepare(self) -> None:
        if not os.path.exists(self.js_dir):
            stress_common.stress_exec(['git', 'clone', NODE_GIT_URL, self.js_dir])
            stress_common.stress_exec(['git', '-C', self.js_dir, 'checkout', NODE_GIT_HASH])


def main():
    logging.debug('ABCKit stress test')
    args = stress_common.get_args()
    test = NodeJSTest(stress_common.get_options_list(OPTIONS_LIST_PATH), args)
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
