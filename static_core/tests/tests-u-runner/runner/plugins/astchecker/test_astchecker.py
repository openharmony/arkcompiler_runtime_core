#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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

import json
import logging
from typing import List, Any
import re

from runner.descriptor import Descriptor
from runner.test_file_based import TestFileBased
from runner.enum_types.params import TestEnv
from runner.plugins.astchecker.util_astchecker import UtilASTChecker
from runner.enum_types.fail_kind import FailKind

_LOGGER = logging.getLogger("runner.plugins.astchecker")


class TestASTChecker(TestFileBased):
    def __init__(self, test_env: TestEnv, test_path: str, flags: List[str], test_id: str,
                 test_cases: UtilASTChecker._TestCasesList) -> None:
        TestFileBased.__init__(self, test_env, test_path, flags, test_id)
        self.util = self.test_env.util
        self.test_cases = test_cases

    def do_run(self) -> TestFileBased:
        es2panda_flags = self.flags
        es2panda_flags.extend(['--output=/dev/null', '--dump-ast'])
        desc = Descriptor(self.path).get_descriptor()

        if 'flags' in desc and 'dynamic-ast' in desc['flags']:
            es2panda_flags = ["--dump-dynamic-ast"]
        else:
            es2panda_flags = ["--dump-ast"]
        es2panda_flags.extend(self.flags)
        if 'flags' in desc and 'module' in desc['flags']:
            es2panda_flags.append("--module")

        passed, self.report, self.fail_kind = self.run_es2panda(
            flags=es2panda_flags,
            test_abc='',
            result_validator=self.es2panda_result_validator
        )

        self.passed = (self.test_cases.has_error_tests and self.fail_kind == FailKind.ES2PANDA_OTHER) ^ passed
        return self

    def es2panda_result_validator(self, actual_output: str, _: Any, return_code: int) -> bool:
        ast_end = actual_output.rfind('}')
        ast = {}
        error = ''
        if ast_end != -1:
            ast_str = re.sub(r'\bundefined\b', 'null', actual_output[:ast_end + 1])
            ast = json.loads(ast_str)
        error = actual_output[actual_output.rfind('}') + 1:].strip()

        passed = self.util.run_tests(self.path, self.test_cases.tests_list, ast, error=error)

        if self.test_cases.has_error_tests:
            return passed and return_code == 1

        return passed and return_code == 0
