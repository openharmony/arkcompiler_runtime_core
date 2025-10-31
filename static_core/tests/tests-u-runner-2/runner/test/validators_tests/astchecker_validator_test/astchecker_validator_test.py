#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2026 Huawei Device Co., Ltd.
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

from pathlib import Path
from unittest import TestCase
from unittest.mock import MagicMock, patch

from runner.extensions.validators.astchecker.astchecker_validator import AstCheckerValidator
from runner.suites.test_standard_flow import TestStandardFlow


class AstcheckerValidatorTest(TestCase):
    data_folder = Path(__file__).parent / "data"

    @patch('runner.suites.test_standard_flow.TestStandardFlow', spec=TestStandardFlow)
    def test_passed_error_check(self, mock_test: MagicMock) -> None:
        """
        Test expects error message at specified coordinates
        Expected: test passed
        """
        mock_test.path = self.data_folder / "test1.ts"
        step_name = "step"
        actual_output = """{
  "type": "Program",
  "statements": [
    {}
    ]
    }
[test1.ts:19:15] Semantic error ESE0000: Type '{ }' is not assignable to type 'string'."""
        actual_error = ""
        actual_return_code = 1
        actual = AstCheckerValidator.es2panda_result_validator(
            test=mock_test,
            _=step_name,
            actual_stdout=actual_output,
            _2=actual_error,
            return_code=actual_return_code)
        self.assertTrue(actual.passed)

    @patch('runner.suites.test_standard_flow.TestStandardFlow', spec=TestStandardFlow)
    def test_passed_node_check(self, mock_test: MagicMock) -> None:
        """
        Test expects node
        Expected: test passed
        """
        mock_test.path = self.data_folder / "test2.ets"
        step_name = "step"
        actual_output = (self.data_folder / "test2-expected-stdout.txt").read_text(encoding="utf-8")
        actual_error = ""
        actual_return_code = 0
        actual = AstCheckerValidator.es2panda_result_validator(
            test=mock_test,
            _=step_name,
            actual_stdout=actual_output,
            _2=actual_error,
            return_code=actual_return_code)
        self.assertTrue(actual.passed)

    @patch('runner.suites.test_standard_flow.TestStandardFlow', spec=TestStandardFlow)
    def test_failed_error_check_message(self, mock_test: MagicMock) -> None:
        """
        Test gets error message with unexpected text
        Expected: test failed
        """
        mock_test.path = self.data_folder / "test1.ts"
        step_name = "step"
        actual_output = """{
  "type": "Program",
  "statements": [
    {}
    ]
    }
[test1.ts:19:15] Semantic error: Type '{ }' is not assignable to type 'string'."""
        actual_error = ""
        actual_return_code = 1
        actual = AstCheckerValidator.es2panda_result_validator(
            test=mock_test,
            _=step_name,
            actual_stdout=actual_output,
            _2=actual_error,
            return_code=actual_return_code)
        self.assertFalse(actual.passed)

    @patch('runner.suites.test_standard_flow.TestStandardFlow', spec=TestStandardFlow)
    def test_failed_error_check_coord(self, mock_test: MagicMock) -> None:
        """
        Test gets expected error message but with other coordinates
        Expected: test failed
        """
        mock_test.path = self.data_folder / "test1.ts"
        step_name = "step"
        actual_output = """{
  "type": "Program",
  "statements": [
    {}
    ]
    }
[test1.ts:0:0] Semantic error ESE0000: Type '{ }' is not assignable to type 'string'."""
        actual_error = ""
        actual_return_code = 1
        actual = AstCheckerValidator.es2panda_result_validator(
            test=mock_test,
            _=step_name,
            actual_stdout=actual_output,
            _2=actual_error,
            return_code=actual_return_code)
        self.assertFalse(actual.passed)

    @patch('runner.suites.test_standard_flow.TestStandardFlow', spec=TestStandardFlow)
    def test_passed_no_markup_return1(self, mock_test: MagicMock) -> None:
        """
        Test doesn't specify either expected message or expected node but returns 1
        Expected: test failed because in such case we expect 0 return code, but get 1
        """
        mock_test.path = self.data_folder / "test3.ets"
        step_name = "step"
        actual_output = (self.data_folder / "test3-expected-stdout.txt").read_text(encoding="utf-8")
        actual_error = ""
        actual_return_code = 1
        actual = AstCheckerValidator.es2panda_result_validator(
            test=mock_test,
            _=step_name,
            actual_stdout=actual_output,
            _2=actual_error,
            return_code=actual_return_code)
        self.assertFalse(actual.passed)

    @patch('runner.suites.test_standard_flow.TestStandardFlow', spec=TestStandardFlow)
    def test_passed_no_markup_return0(self, mock_test: MagicMock) -> None:
        """
        Test doesn't specify either expected message or expected node and returns 0
        Expected: test passed
        """
        mock_test.path = self.data_folder / "test3.ets"
        step_name = "step"
        actual_output = (self.data_folder / "test3-expected-stdout.txt").read_text(encoding="utf-8")
        actual_error = ""
        actual_return_code = 0
        actual = AstCheckerValidator.es2panda_result_validator(
            test=mock_test,
            _=step_name,
            actual_stdout=actual_output,
            _2=actual_error,
            return_code=actual_return_code)
        self.assertTrue(actual.passed)

    @patch('runner.suites.test_standard_flow.TestStandardFlow', spec=TestStandardFlow)
    def test_failed_node_check(self, mock_test: MagicMock) -> None:
        """
        Test expects node, but does not find it
        Expected: test failed
        """
        mock_test.path = self.data_folder / "test2.ets"
        step_name = "step"
        actual_output = """{
          "type": "Program",
          "statements": [
            {}
            ]
            }
        """
        actual_error = ""
        actual_return_code = 0
        actual = AstCheckerValidator.es2panda_result_validator(
            test=mock_test,
            _=step_name,
            actual_stdout=actual_output,
            _2=actual_error,
            return_code=actual_return_code)
        self.assertFalse(actual.passed)

    @patch('runner.suites.test_standard_flow.TestStandardFlow', spec=TestStandardFlow)
    def test_failed_return_abort(self, mock_test: MagicMock) -> None:
        """
        Test gets expected error message with expected coordinates,
        but has output in the stderr and not expected return code
        Expected: test failed
        """
        mock_test.path = self.data_folder / "test1.ts"
        step_name = "step"
        actual_output = "[test1.ts:0:0] Semantic error ESE0000: Type '{ }' is not assignable to type 'string'."
        actual_error = """
        es2panda unexpectedly terminated.
PLEASE submit a bug report to https://gitcode.com/openharmony/arkcompiler_ets_frontend/issues and 
include the crash backtrace, source code and associated run script. 
This line should be unreachable
ASSERTION FAILED: false"""
        actual_return_code = -6
        actual = AstCheckerValidator.es2panda_result_validator(
            test=mock_test,
            _=step_name,
            actual_stdout=actual_output,
            _2=actual_error,
            return_code=actual_return_code)
        self.assertFalse(actual.passed)
