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
import unittest
from pathlib import Path

from runner.suites.test_metadata import TestMetadata
from runner.suites.test_standard_flow import TestStandardFlow
from runner.types.step_report import StepReport


def get_test_instance(actual_stdout: str, actual_stderr: str,
                      expected_out: str | None, expected_error: str | None,
                      fail_kind: str = "") -> TestStandardFlow:
    test_instance = TestStandardFlow.__new__(TestStandardFlow)
    test_instance.metadata = TestMetadata()
    test_instance.metadata.expected_out = expected_out
    test_instance.metadata.expected_error = expected_error
    test_instance.expected = ""
    test_instance.expected_err = ""
    test_instance.passed = True
    test_step_report = StepReport(name="test", command_line="", cmd_output=actual_stdout,
                                  cmd_error=actual_stderr, status=fail_kind)
    test_instance.step_reports = [test_step_report]
    test_instance.path_to_expected = Path("metadata.expected_out")
    test_instance.path_to_expected_err = Path("metadata.expected_error")

    return test_instance


class ExpectedMetadataTest(unittest.TestCase):

    def test_expected_stdout_ok(self) -> None:
        expected_out = "default: 1\ndefault: 2\ndefault: 3\nuserClick: 1\n        userClick: 2\n        userClick: 3"
        expected_error = None
        expected_out_val_msg = "Comparison with expected output/error has passed"
        report_stdout = "default: 1\n       default: 2\n       default: 3\nuserClick: 1\n userClick: 2\nuserClick: 3"
        test_result = get_test_instance(report_stdout, "", expected_out, expected_error)
        passed, output_val_msg = test_result.compare_with_stdout(report_stdout)

        self.assertTrue(passed)
        self.assertEqual(expected_out_val_msg, output_val_msg)

    def test_expected_some_lines_stdout_ok(self) -> None:
        expected_out = "default: 3\nuserClick: 1\n        userClick: 3"
        expected_error = None
        expected_out_val_msg = "Comparison with expected output/error has passed"
        report_stdout = "default: 1\n       default: 2\n       default: 3\nuserClick: 1\n userClick: 2\nuserClick: 3"
        test_result = get_test_instance(report_stdout, "", expected_out, expected_error)
        passed, output_val_msg = test_result.compare_with_stdout(report_stdout)

        self.assertTrue(passed)
        self.assertEqual(expected_out_val_msg, output_val_msg)

    def test_expected_stdout_fail(self) -> None:
        expected_out = "default: 1\ndefault: 2\ndefault: 3\nuserClick: 1\n        userClick: 2\n        userClick: 3"
        expected_error = None
        expected_out_val_msg = ("Comparison with expected output/error has failed\n"
                                "Actual output doesn't contain expected lines:\n"
                                "['default: 1']")
        report_stdout = "default: 2\n   default: 2\n default: 3\nuserClick: 1\n    userClick: 2\nuserClick: 3"
        test_result = get_test_instance(report_stdout, "",
                                        expected_out, expected_error, fail_kind="Other")
        passed, output_val_msg = test_result.compare_with_stdout(report_stdout)

        self.assertFalse(passed)
        self.assertEqual(expected_out_val_msg, output_val_msg)

    def test_expected_stdout_some_lines_fail(self) -> None:
        expected_out = "default: 3\nuserClick: 1\n        userClick: 3"
        expected_error = None
        expected_out_val_msg = ("Comparison with expected output/error has failed\n"
                                "Actual output doesn't contain expected lines:\n"
                                "['default: 3']")
        report_stdout = "default: 2\n   default: 2\nuserClick: 1\n    userClick: 2\nuserClick: 3"
        test_result = get_test_instance(report_stdout, "",
                                        expected_out, expected_error, fail_kind="Other")
        passed, output_val_msg = test_result.compare_with_stdout(report_stdout)

        self.assertFalse(passed)
        self.assertEqual(expected_out_val_msg, output_val_msg)

    def test_expected_stderr_ok(self) -> None:
        expected_out = None
        expected_error = "default: 1\n    default: 2\n    default: 3\nuserClick: 1\nuserClick: 2\n    userClick: 3"
        report_stderr = "default: 1\n  default: 2\ndefault: 3\n     userClick: 1\nuserClick: 2\nuserClick: 3"
        expected_err_val_msg = "Comparison with expected output/error has passed"
        test_result = get_test_instance("report_stdout", report_stderr, expected_out, expected_error)
        passed, error_val_msg = test_result.compare_with_stderr(report_stderr)

        self.assertTrue(passed)
        self.assertEqual(expected_err_val_msg, error_val_msg)

    def test_expected_stderr_some_lines_ok(self) -> None:
        expected_out = None
        expected_error = "default: 1\n    default: 2\n    default: 3\nuserClick: 1\nuserClick: 2\n    userClick: 3"
        report_stderr = "default: 1\n  default: 2\ndefault: 3\n     userClick: 1\nuserClick: 2\nuserClick: 3"
        expected_err_val_msg = "Comparison with expected output/error has passed"
        test_result = get_test_instance("report_stdout", report_stderr, expected_out, expected_error)
        passed, error_val_msg = test_result.compare_with_stderr(report_stderr)

        self.assertTrue(passed)
        self.assertEqual(expected_err_val_msg, error_val_msg)

    def test_expected_stderr_fail(self) -> None:
        expected_out = None
        expected_error = "default: 1\n    default: 2\n    default: 3\nuserClick: 1\nuserClick: 2\n    userClick: 3"
        report_stderr = "default: 2\ndefault: 2\n   default: 3\nuserClick: 1\nuserClick: 2\n  userClick: 3"
        expected_err_val_msg = ("Comparison with expected output/error has failed\n"
                                "Actual output doesn't contain expected lines:\n"
                                "['default: 1']")
        test_result = get_test_instance("", report_stderr,
                                        expected_out, expected_error, fail_kind="Other")
        passed, error_val_msg = test_result.compare_with_stderr(report_stderr)

        self.assertFalse(passed)
        self.assertEqual(expected_err_val_msg, error_val_msg)

    def test_expected_stderr_some_lines_fail(self) -> None:
        expected_out = None
        expected_error = "        userClick: 3\n    default: 2\n     default: 3\nuserClick: 1\nuserClick: 2"
        report_stderr = "default: 2\ndefault: 2\n   default: 3\nuserClick: 1\n  userClick: 3"
        expected_err_val_msg = ("Comparison with expected output/error has failed\n"
                                "Actual output doesn't contain expected lines:\n"
                                "['userClick: 2']")
        test_result = get_test_instance("", report_stderr,
                                        expected_out, expected_error, fail_kind="Other")
        passed, error_val_msg = test_result.compare_with_stderr(report_stderr)

        self.assertFalse(passed)
        self.assertEqual(expected_err_val_msg, error_val_msg)

    def test_compare_both_metadata(self) -> None:
        expected_out = "default: 1\ndefault: 2\n default: 3\n    userClick: 1\n    userClick: 2\n  userClick: 3"
        expected_error = "default: 4\ndefault: 2\ndefault: 3\n    userClick: 1\n    userClick: 2\n    userClick: 3"
        report_stdout = "default: 1\ndefault: 2\n default: 3\nuserClick: 1\n     userClick: 2\n      userClick: 3"
        report_stderr = "default: 4\ndefault: 2\ndefault: 3\n userClick: 1\nuserClick: 2\nuserClick: 3"
        expected_out_val_msg = "Comparison with expected output/error has passed"
        expected_err_val_msg = "Comparison with expected output/error has passed"
        test_result = get_test_instance(report_stdout, report_stderr, expected_out, expected_error)
        passed_output, output_val_msg = test_result.compare_with_stdout(report_stdout)
        passed_error, error_val_msg = test_result.compare_with_stderr(report_stderr)

        self.assertTrue(passed_output)
        self.assertTrue(passed_error)
        self.assertEqual(expected_out_val_msg, output_val_msg)
        self.assertEqual(expected_err_val_msg, error_val_msg)

    def test_compare_both_some_lines_metadata(self) -> None:
        expected_out = "[TID 005030] default: 1\n    userClick: 2\n  userClick: 3"
        expected_error = ("    userClick: 3\n[TID 005030] default: 4\n[TID 005030] default: 2\n"
                          "    userClick: 2")
        report_stdout = ("[TID 005030] default: 1\ndefault: 2\n default: 3\nuserClick: 1\n"
                         "     userClick: 2\n      userClick: 3")
        report_stderr = ("    userClick: 3\n[TID 005030] default: 4\n[TID 005030] default: 2\n"
                         "    userClick: 2\ndefault: 2\ndefault: 3\n userClick: 1")
        expected_out_val_msg = "Comparison with expected output/error has passed"
        expected_err_val_msg = "Comparison with expected output/error has passed"
        test_result = get_test_instance(report_stdout, report_stderr, expected_out, expected_error)
        passed_output, output_val_msg = test_result.compare_with_stdout(report_stdout)
        passed_error, error_val_msg = test_result.compare_with_stderr(report_stderr)

        self.assertTrue(passed_output)
        self.assertTrue(passed_error)
        self.assertEqual(expected_out_val_msg, output_val_msg)
        self.assertEqual(expected_err_val_msg, error_val_msg)

    def test_expected_empty_string_vs_none(self) -> None:
        expected_out = ""
        expected_error = None
        expected_out_val_msg = "Failed to read expected data"
        report_stdout = "default: 1\ndefault: 2"
        test_result = get_test_instance(report_stdout, "", expected_out, expected_error)
        passed, output_val_msg = test_result.compare_with_stdout(report_stdout)

        self.assertFalse(passed)
        self.assertEqual(expected_out_val_msg, output_val_msg)

    def test_expected_empty_string_fail(self) -> None:
        expected_out = ""
        expected_error = None
        expected_out_val_msg = "Failed to read expected data"
        report_stdout = ""
        test_result = get_test_instance(report_stdout, "", expected_out, expected_error)
        passed, output_val_msg = test_result.compare_with_stdout(report_stdout)

        self.assertFalse(passed)
        self.assertEqual(expected_out_val_msg, output_val_msg)

    def test_expected_none_with_non_empty_error(self) -> None:
        expected_out = None
        expected_error = None
        expected_err_val_msg = "Comparison with expected output/error has failed"
        report_stderr = "error: something went wrong"
        test_result = get_test_instance("", report_stderr, expected_out, expected_error)
        passed, error_val_msg = test_result.compare_with_stderr(report_stderr)

        self.assertFalse(passed)
        self.assertEqual(expected_err_val_msg, error_val_msg)

    def test_both_none_empty_outputs(self) -> None:
        expected_out = None
        expected_error = None
        expected_out_val_msg = "Comparison with expected output/error has passed"
        expected_err_val_msg = "Comparison with expected output/error has passed"
        test_result = get_test_instance("", "", expected_out, expected_error)
        passed_output, output_val_msg = test_result.compare_with_stdout("")
        passed_error, error_val_msg = test_result.compare_with_stderr("")

        self.assertTrue(passed_output)
        self.assertTrue(passed_error)
        self.assertEqual(expected_out_val_msg, output_val_msg)
        self.assertEqual(expected_err_val_msg, error_val_msg)

    def test_compare_both_fail(self) -> None:
        expected_out = "default: 1\ndefault: 2"
        expected_error = "error: expected"
        report_stdout = "default: 3\ndefault: 4"
        report_stderr = "error: actual"
        expected_out_val_msg = ("Comparison with expected output/error has failed\n"
                                "Actual output doesn't contain expected lines:\n['default: 1', 'default: 2']")
        expected_err_val_msg = ("Comparison with expected output/error has failed\n"
                                "Actual output doesn't contain expected lines:\n['error: expected']")
        test_result = get_test_instance(report_stdout, report_stderr, expected_out, expected_error,
                                        fail_kind="Other")
        passed_output, output_val_msg = test_result.compare_with_stdout(report_stdout)
        passed_error, error_val_msg = test_result.compare_with_stderr(report_stderr)

        self.assertFalse(passed_output)
        self.assertFalse(passed_error)
        self.assertEqual(expected_out_val_msg, output_val_msg)
        self.assertEqual(expected_err_val_msg, error_val_msg)
