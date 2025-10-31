#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
from dataclasses import dataclass
from io import StringIO
from json import JSONDecodeError, JSONDecoder
from typing import cast, List

from runner.descriptor import Descriptor
from runner.enum_types.params import TestEnv
from runner.logger import Log
from runner.parser import DIAG_PATTERN
from runner.plugins.astchecker.util_astchecker import UtilASTChecker
from runner.test_file_based import TestFileBased
from runner.utils import unlines

_LOGGER = logging.getLogger("runner.plugins.astchecker")


@dataclass(frozen=True)
class _CategorizedLines:
    diagnostics: List[str]
    others: List[str]


def _categorize_lines(stderr: str) -> _CategorizedLines:
    errors = []
    other = []
    for line in StringIO(stderr):
        if DIAG_PATTERN.match(line):
            errors.append(line)
        else:
            other.append(line)
    return _CategorizedLines(errors, other)


def _decode_dump_json(dump: str) -> dict:
    if any(char != '\b' for char in dump):
        decoder = JSONDecoder()
        obj, _ = decoder.raw_decode(dump, 0)
        return cast(dict, obj)

    return {}


class TestASTChecker(TestFileBased):
    def __init__(self, test_env: TestEnv, test_path: str, flags: List[str], test_id: str,
                 test_cases: UtilASTChecker.TestCasesList) -> None:
        TestFileBased.__init__(self, test_env, test_path, flags, test_id)
        self.util = self.test_env.util
        self.is_a_test = True
        self.test_cases = test_cases

    def do_run(self) -> TestFileBased:
        if not self.is_a_test:
            return self

        es2panda_flags = self.flags
        desc = Descriptor(self.path).get_descriptor()

        if 'flags' in desc and 'dynamic-ast' in desc['flags']:
            es2panda_flags += ["--dump-dynamic-ast"]
        else:
            es2panda_flags += ["--dump-ast"]
        if 'flags' in desc and 'module' in desc['flags']:
            es2panda_flags += ["--module"]

        self.passed, self.report, self.fail_kind = self.run_es2panda(
            flags=list(set(es2panda_flags)),
            test_abc=self.get_tests_abc(),
            result_validator=self.es2panda_result_validator
        )
        return self

    def es2panda_result_validator(self, actual_stdout: str, actual_stderr: str, return_code: int) -> bool:
        try:
            # NOTE(pronai) merging stdout and stderr until #29808
            # this is fine because we can separate diagnostics using a regex
            categorized = _categorize_lines(actual_stdout + actual_stderr)
        except ValueError as err:
            Log.exception_and_raise(_LOGGER, f'Output from file: {self.path}.\nThrows JSON error: {err}.')

        try:
            passed = self.util.run_tests(
                test_file=self.path,
                test_cases=self.test_cases,
                ast=_decode_dump_json(''.join(categorized.others)),
                error=unlines(categorized.diagnostics))
        except JSONDecodeError as err:
            message = f"{self.test_id}: {err}"
            Log.default(_LOGGER, message)
            passed = False

        if self.test_cases.has_error_tests and not self.test_cases.skip_errors:
            return passed and return_code == 1
        return passed and return_code == 0
