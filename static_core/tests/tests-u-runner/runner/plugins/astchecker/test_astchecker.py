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

import logging
from json import JSONDecoder
from typing import List, Any, Tuple, Dict

from runner.descriptor import Descriptor
from runner.enum_types.params import TestEnv
from runner.logger import Log
from runner.plugins.astchecker.util_astchecker import UtilASTChecker
from runner.test_file_based import TestFileBased

_LOGGER = logging.getLogger("runner.plugins.astchecker")


class TestASTChecker(TestFileBased):
    def __init__(self, test_env: TestEnv, test_path: str, flags: List[str], test_id: str,
                 test_cases: UtilASTChecker.TestCasesList) -> None:
        TestFileBased.__init__(self, test_env, test_path, flags, test_id)
        self.util = self.test_env.util
        self.test_cases = test_cases

    @staticmethod
    def handle_error_dump(input_string: str) -> Tuple[str, dict]:
        errors_list = []
        lines = input_string.splitlines()
        for line in lines:
            if line.split()[0] in ("Warning:", "SyntaxError:", "TypeError:", "Error:"):
                errors_list.append(line)  # Add the line to the errors list

        errors = "\n".join(errors_list)

        filtered_lines = [line for line in lines if line not in errors_list]
        dump = "\n".join(filtered_lines)

        if any(char != '\b' for char in dump):
            decoder = JSONDecoder()
            obj, _ = decoder.raw_decode(dump, 0)
            return errors, obj

        return errors, {}

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

        self.passed, self.report, self.fail_kind = self.run_es2panda(
            flags=es2panda_flags,
            test_abc='',
            result_validator=self.es2panda_result_validator
        )
        return self

    def es2panda_result_validator(self, actual_output: str, _: Any, return_code: int) -> bool:
        error = str()
        dump: Dict[str, Any] = {}
        try:
            error, dump = self.handle_error_dump(actual_output)
        except ValueError as err:
            Log.exception_and_raise(_LOGGER, f'Output from file: {self.path}.\nThrows JSON error: {err}.')

        passed = self.util.run_tests(self.path, self.test_cases, dump, error=error)

        if self.test_cases.has_error_tests and not self.test_cases.skip_errors:
            return passed and return_code == 1
        return passed and return_code == 0
