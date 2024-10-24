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
from typing import List, Tuple, Final, Dict
import logging

import stress_common

from stress_test import StressTest, Test, Result
from stress_common import SCRIPT_DIR, TMP_DIR, STRESS, collect_from

FAIL_LIST_PATH = os.path.join(SCRIPT_DIR, 'fail_list.json')
OPTIONS_LIST_PATH = os.path.join(SCRIPT_DIR, 'options_list.json')

TEST262_GIT_URL = "https://gitee.com/hufeng20/test262.git"
TEST262_GIT_HASH = "6f4601d095a3899d6102f2c320b671495cbe8757"


EXCLUDED_TESTS = [
    'regress-4595.js',
    'module-jit-reachability.js',
]


logging.basicConfig(format='%(message)s', level=logging.DEBUG)


class StressJSTest(StressTest):
    def __init__(self, options_list, args):
        rc_lib_path: Final[str] = f'{args.build_dir}/arkcompiler/runtime_core'
        ets_rc_lib_path: Final[str] = f'{args.build_dir}/arkcompiler/ets_runtime'
        icu_lib_path: Final[str] = f'{args.build_dir}/thirdparty/icu/'
        zlib_lib_path: Final[str] = f'{args.build_dir}/thirdparty/zlib/'
        self.runtime_env: Dict[str, str] = {
            'LD_LIBRARY_PATH': f'{rc_lib_path}:{ets_rc_lib_path}:{icu_lib_path}:{zlib_lib_path}'
        }

        self.js_dir = os.path.join(TMP_DIR, 'abckit_test262')
        self.cp = self.get_compiler_path(args.build_dir)
        self.jvm = os.path.join(args.build_dir, 'arkcompiler/ets_runtime/ark_js_vm')
        self.timeout = 10
        self.repeats = 1
        self.options_list = options_list or {}
        self.build_dir = args.build_dir

    def run_single(self, test: Test) -> Result:
        cmd = [STRESS, '--input-file', test.abc, '--output-file', '/tmp/tmp.abc']
        if test.source in self.options_list:
            cmd += [self.options_list[test.source][0], self.options_list[test.source][1]]
        result = stress_common.stress_exec(cmd, allow_error=True, print_command=False, env={"OUT_DIR": self.build_dir})
        if result.returncode != 0:
            error = stress_common.parse_stdout(str(result.returncode), result.stdout)
            return Result(test.source, error)
        return Result(test.source, "0")

    def compile_single(self, src: str) -> Tuple[str, str, int]:
        abc_path: src = src + '.abc'
        abc_path = abc_path.replace("[", "_").replace("]", "_")
        cmd = [
            self.cp,
            '--module',
            '--merge-abc',
            '--target-api-version',
            '12',
            '--target-api-sub-version',
            'beta3',
            '--output',
            abc_path, src
        ]
        result = stress_common.stress_exec(cmd, allow_error=True, print_command=False)
        if result.returncode == 0 and not os.path.exists(abc_path):
            logging.debug('WARNING: for %s es2abc has %s return code, but has no output', src, result.returncode)
        return src, abc_path, result.returncode

    def prepare(self) -> None:
        self.download_262()

    def get_compiler_path(self, build_dir) -> str:
        return os.path.join(build_dir, 'arkcompiler/ets_frontend/es2abc')

    def download_262(self) -> None:
        if not os.path.exists(self.js_dir):
            stress_common.stress_exec(['git', 'clone', TEST262_GIT_URL, self.js_dir])
            stress_common.stress_exec(['git', '-C', self.js_dir, 'checkout', TEST262_GIT_HASH])

    def collect(self) -> List[str]:
        tests: List[str] = []
        tests.extend(collect_from(self.js_dir, lambda name: name.endswith('.js') and not name.startswith('.')))
        random.shuffle(tests)

        logging.debug('Total tests: %s', len(tests))
        for excluded in EXCLUDED_TESTS:
            tests = list(filter(lambda name: excluded not in name, tests))
        logging.debug('Tests after exclude: %s', len(tests))

        return tests

    def run_js_test_single(self, test: Test, repeats: int = 1) -> Result:
        entry: str = f'--entry={os.path.basename(test.source)}'
        cmd = [self.jvm, entry, test.abc]
        result = stress_common.stress_exec(cmd, allow_error=True, print_command=False, env=self.runtime_env,
                                           timeout=self.timeout, print_output=False, repeats=repeats)
        if result.returncode == 0:
            return Result(test.source, 0)

        return Result(test.source, result.returncode, result.stdout, result.stderr)


def main():
    logging.debug('ABCKit stress test')
    args = stress_common.get_args()
    test = StressJSTest(stress_common.get_options_list(OPTIONS_LIST_PATH), args)
    test.prepare()

    tests: List[Test] = test.build()
    results = test.run(tests)

    fail_list = stress_common.get_fail_list(results)

    if not stress_common.check_regression_errors(FAIL_LIST_PATH, fail_list):
        return 1

    if args.update_fail_list:
        stress_common.update_fail_list(FAIL_LIST_PATH, fail_list)
        logging.debug('Failures/Total: %s/%s', len(fail_list), len(results))
        return 0

    if not stress_common.check_fail_list(FAIL_LIST_PATH, fail_list):
        logging.debug('Failures/Total: %s/%s', len(fail_list), len(results))

    logging.debug('ABCKit: no regressions')
    logging.debug('Failures/Total: %s/%s', len(fail_list), len(results))
    return 0


if __name__ == '__main__':
    sys.exit(main())
