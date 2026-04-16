#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
from typing import ClassVar
from unittest import TestCase
from unittest.mock import MagicMock, patch

from runner import utils
from runner.enum_types.validation_result import ValidatorFailKind
from runner.extensions.validators.parser.parser_validator import ParserValidator
from runner.options.options_step import StepKind
from runner.options.options_step_utils import StepFields
from runner.suites.test_standard_flow import TestStandardFlow


class ParserValidatorTest(TestCase):
    data_folder = Path(__file__).parent / "data"
    step_fields: ClassVar[StepFields]

    @staticmethod
    def fill_step_fields(step_kind: StepKind) -> StepFields:
        return StepFields(
            step_kind=step_kind.value,
            step_name="step name"
        )

    @classmethod
    def setUpClass(cls) -> None:
        cls.step_fields = cls.fill_step_fields(StepKind.COMPILER)

    @patch('runner.suites.test_standard_flow.TestStandardFlow', spec=TestStandardFlow)
    def test_passed_no_output(self, mock_test: MagicMock) -> None:
        """
        Test checks simple passed way: no output, expected file is empty, return code = 0
        Expected: test passed
        """
        mock_test.path = self.data_folder / "test1.ets"
        actual_output = ""
        actual_error = ""
        actual_return_code = 0
        actual = ParserValidator.es2panda_result_validator(
            test=mock_test,
            _=ParserValidatorTest.step_fields,
            actual_output=actual_output,
            _2=actual_error,
            return_code=actual_return_code)
        self.assertTrue(actual.passed)
        self.assertEqual("", actual.description)

    @patch('runner.suites.test_standard_flow.TestStandardFlow', spec=TestStandardFlow)
    def test_absent_expected(self, mock_test: MagicMock) -> None:
        """
        Input: no output, expected file is absent
        Expected: test failed
        """
        mock_test.path = self.data_folder / "test2.ets"
        actual_output = ""
        actual_error = ""
        actual_return_code = 0
        expected_description = f"Expected file {mock_test.path.parent}/test2-expected.txt not found"
        actual = ParserValidator.es2panda_result_validator(
            test=mock_test,
            _=ParserValidatorTest.step_fields,
            actual_output=actual_output,
            _2=actual_error,
            return_code=actual_return_code)
        self.assertFalse(actual.passed)
        self.assertEqual(expected_description, actual.description)
        self.assertEqual(ValidatorFailKind.OTHER, actual.kind)

    @patch('runner.suites.test_standard_flow.TestStandardFlow', spec=TestStandardFlow)
    def test_passed_with_output(self, mock_test: MagicMock) -> None:
        """
        Input: there is an output, expected file contains matching content, return code = 0
        Expected: test passed
        """
        mock_test.path = self.data_folder / "test3.ets"
        actual_output = "hello\nworld"
        actual_error = ""
        actual_return_code = 0
        actual = ParserValidator.es2panda_result_validator(
            test=mock_test,
            _=ParserValidatorTest.step_fields,
            actual_output=actual_output,
            _2=actual_error,
            return_code=actual_return_code)
        self.assertTrue(actual.passed)
        self.assertEqual("", actual.description)
        self.assertEqual(actual.kind, ValidatorFailKind.NONE)

    @patch('runner.suites.test_standard_flow.TestStandardFlow', spec=TestStandardFlow)
    def test_passed_with_output_extra_lines(self, mock_test: MagicMock) -> None:
        """
        Input: there is an output, expected file contains matching content, return code = 0
        Actual content has extra leading and finishing empty lines -
        they should be stripped and do not affect the result
        Expected: test passed
        """
        mock_test.path = self.data_folder / "test3.ets"
        actual_output = "\nhello\nworld\n"
        actual_error = ""
        actual_return_code = 0
        actual = ParserValidator.es2panda_result_validator(
            test=mock_test,
            _=ParserValidatorTest.step_fields,
            actual_output=actual_output,
            _2=actual_error,
            return_code=actual_return_code)
        self.assertTrue(actual.passed)
        self.assertEqual("", actual.description)
        self.assertEqual(ValidatorFailKind.NONE, actual.kind)

    @patch('runner.suites.test_standard_flow.TestStandardFlow', spec=TestStandardFlow)
    def test_passed_with_extra_lines_in_expected(self, mock_test: MagicMock) -> None:
        """
        Input: there is an output, expected file contains matching content, return code = 0
        Expected content has extra leading and finishing empty lines -
        they should be stripped and do not affect the result
        Expected: test passed
        """
        mock_test.path = self.data_folder / "test4.ets"
        actual_output = "hello\nworld"
        actual_error = ""
        actual_return_code = 0
        actual = ParserValidator.es2panda_result_validator(
            test=mock_test,
            _=ParserValidatorTest.step_fields,
            actual_output=actual_output,
            _2=actual_error,
            return_code=actual_return_code)
        self.assertTrue(actual.passed)
        self.assertEqual("", actual.description)
        self.assertEqual(ValidatorFailKind.NONE, actual.kind)

    @patch('runner.suites.test_standard_flow.TestStandardFlow', spec=TestStandardFlow)
    def test_failed_not_matched_output(self, mock_test: MagicMock) -> None:
        """
        Input: there is an output, expected file contains non-matching content.
        Expected: test failed
        """
        mock_test.path = self.data_folder / "test3.ets"
        actual_output = "hello\nword"
        actual_error = ""
        actual_return_code = 0
        expected_description = f"Actual output '{actual_output}' does not match to expected one 'hello\nworld'"
        actual = ParserValidator.es2panda_result_validator(
            test=mock_test,
            _=ParserValidatorTest.step_fields,
            actual_output=actual_output,
            _2=actual_error,
            return_code=actual_return_code)
        self.assertFalse(actual.passed)
        self.assertEqual(expected_description, actual.description)
        self.assertEqual(ValidatorFailKind.COMPARE_OUTPUT, actual.kind)

    @patch('runner.suites.test_standard_flow.TestStandardFlow', spec=TestStandardFlow)
    def test_passed_with_output_rt1(self, mock_test: MagicMock) -> None:
        """
        Input: there is an output, expected file contains matching content, but return code = 1
        Expected: test passed
        """
        mock_test.path = self.data_folder / "test3.ets"
        actual_output = "hello\nworld"
        actual_error = ""
        actual_return_code = 1
        actual = ParserValidator.es2panda_result_validator(
            test=mock_test,
            _=ParserValidatorTest.step_fields,
            actual_output=actual_output,
            _2=actual_error,
            return_code=actual_return_code)
        self.assertTrue(actual.passed)
        self.assertEqual("", actual.description)

    @patch('runner.suites.test_standard_flow.TestStandardFlow', spec=TestStandardFlow)
    def test_passed_with_output_rt2(self, mock_test: MagicMock) -> None:
        """
        Input: there is an output, expected file contains matching content, but return code = 2
        Expected: test failed, as return code is neither 0 nor 1
        """
        mock_test.path = self.data_folder / "test3.ets"
        actual_output = "hello\nworld"
        actual_error = ""
        actual_return_code = 2
        expected_description = f"Actual return code {actual_return_code} does not match to expected values [0, 1]"
        actual = ParserValidator.es2panda_result_validator(
            test=mock_test,
            _=ParserValidatorTest.step_fields,
            actual_output=actual_output,
            _2=actual_error,
            return_code=actual_return_code)
        self.assertFalse(actual.passed)
        self.assertEqual(expected_description, actual.description)
        self.assertEqual(ValidatorFailKind.COMPARE_OUTPUT, actual.kind)

    def test_normalize_build_system_path_unix(self) -> None:
        """
        Test normalize_build_system_path with Unix-style paths
        """
        input_str = (
            "Error Message: Type '\"string\"' cannot be assigned to type 'Double' "
            "At File: /home/user/project/ets-warnings-tests/gen/"
            "diagnostic_format_custom_tests/diagnostic_format_custom_1.ets:21:21"
        )
        expected = (
            "Error Message: Type '\"string\"' cannot be assigned to type 'Double' "
            "At File: diagnostic_format_custom_1.ets:21:21"
        )
        result = utils.normalize_build_system_path(input_str)
        self.assertEqual(result, expected)

    def test_normalize_build_system_path_windows(self) -> None:
        """
        Test normalize_build_system_path with Windows-style paths
        """
        input_str = (
            "Error Message: Type '\"string\"' cannot be assigned to type 'Double' "
            "At File: C:\\Users\\user\\project\\ets-warnings-tests\\gen\\"
            "diagnostic_format_custom_tests\\diagnostic_format_custom_1.ets:21:21"
        )
        expected = (
            "Error Message: Type '\"string\"' cannot be assigned to type 'Double' "
            "At File: diagnostic_format_custom_1.ets:21:21"
        )
        result = utils.normalize_build_system_path(input_str)
        self.assertEqual(result, expected)

    def test_normalize_build_system_path_windows_forward_slash(self) -> None:
        """
        Test normalize_build_system_path with Windows-style paths using forward slashes
        """
        input_str = (
            "Error Message: Type '\"string\"' cannot be assigned to type 'Double' "
            "At File: D:/workspace/project/ets-warnings-tests/gen/"
            "diagnostic_format_custom_tests/diagnostic_format_custom_1.ets:21:21"
        )
        expected = (
            "Error Message: Type '\"string\"' cannot be assigned to type 'Double' "
            "At File: diagnostic_format_custom_1.ets:21:21"
        )
        result = utils.normalize_build_system_path(input_str)
        self.assertEqual(result, expected)

    def test_normalize_build_system_path_short_path(self) -> None:
        """
        Test normalize_build_system_path with short path
        """
        input_str = "Error Message: Type '\"string\"' cannot be assigned to type 'Double' At File: /tmp/test.ets:10:5"
        expected = "Error Message: Type '\"string\"' cannot be assigned to type 'Double' At File: test.ets:10:5"
        result = utils.normalize_build_system_path(input_str)
        self.assertEqual(result, expected)

    def test_normalize_build_system_path_no_match(self) -> None:
        """
        Test normalize_build_system_path with string that doesn't contain At File pattern
        """
        input_str = "Error Message: Type '\"string\"' cannot be assigned to type 'Double'"
        expected = "Error Message: Type '\"string\"' cannot be assigned to type 'Double'"
        result = utils.normalize_build_system_path(input_str)
        self.assertEqual(result, expected)

    def test_normalize_build_system_path_multiline(self) -> None:
        """
        Test normalize_build_system_path with multiline output
        """
        input_str = (
            "1 ERROR: Semantic error ESE0318\n"
            "Error Message: Type '\"string\"' cannot be assigned to type 'Double' "
            "At File: /home/user/project/ets-warnings-tests/gen/"
            "diagnostic_format_custom_tests/diagnostic_format_custom_1.ets:21:21\n"
            "2 ERROR: Semantic error ESE0344\n"
            "Error Message: Invalid destination type for floating-point constant value "
            "At File: /home/user/project/ets-warnings-tests/gen/"
            "diagnostic_format_custom_tests/diagnostic_format_custom_1.ets:25:12"
        )
        expected = (
            "1 ERROR: Semantic error ESE0318\n"
            "Error Message: Type '\"string\"' cannot be assigned to type 'Double' "
            "At File: diagnostic_format_custom_1.ets:21:21\n"
            "2 ERROR: Semantic error ESE0344\n"
            "Error Message: Invalid destination type for floating-point constant value "
            "At File: diagnostic_format_custom_1.ets:25:12"
        )
        result = utils.normalize_build_system_path(input_str)
        self.assertEqual(result, expected)
