#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

from runner.enum_types.params import TestReport
from runner.suites.test_standard_flow import TestStandardFlow


def get_test_instance(report_stdout: str, report_stderr: str,
                      expected_file: str, expected_err_file: str, fail_kind: str | None = None) -> TestStandardFlow:
    test_instance = TestStandardFlow.__new__(TestStandardFlow)
    test_instance.expected = ""
    test_instance.expected_err = ""
    test_instance.passed = True
    test_instance.report = TestReport(report_stdout, report_stderr, 0)
    test_instance.fail_kind = fail_kind
    test_instance.path_to_expected = Path(__file__).parent / expected_file
    test_instance.path_to_expected_err = Path(__file__).parent / expected_err_file

    return test_instance


class ExpectedFilesTest(unittest.TestCase):

    def test_expected_stdout_ok(self) -> None:
        expected_file = "test_data/test1.console.ets.expected"
        expected_err_file = "test_data/test1.console.ets.expected.err"
        report_stdout = "default: 1\n       default: 2\n       default: 3\nuserClick: 1\n userClick: 2\nuserClick: 3"
        test_result = get_test_instance(report_stdout, "", expected_file, expected_err_file)
        passed = test_result.compare_output_with_expected(report_stdout, "")

        self.assertTrue(passed)

    def test_expected_some_lines_stdout_ok(self) -> None:
        expected_file = "test_data/test1b.console.ets.expected"
        expected_err_file = "test_data/test1b.console.ets.expected.err"
        report_stdout = "default: 1\n       default: 2\n       default: 3\nuserClick: 1\n userClick: 2\nuserClick: 3"
        test_result = get_test_instance(report_stdout, "", expected_file, expected_err_file)
        passed = test_result.compare_output_with_expected(report_stdout, "")

        self.assertTrue(passed)

    def test_expected_stdout_fail(self) -> None:
        expected_file = "test_data/test1.console.ets.expected"
        expected_err_file = "test_data/test1.console.ets.expected.err"
        report_stdout = "default: 2\n   default: 2\n default: 3\nuserClick: 1\n    userClick: 2\nuserClick: 3"
        test_result = get_test_instance(report_stdout, "",
                                        expected_file, expected_err_file, fail_kind="Other")
        passed = test_result.compare_output_with_expected(report_stdout, "")

        self.assertFalse(passed)

    def test_expected_stdout_some_lines_fail(self) -> None:
        expected_file = "test_data/test1b.console.ets.expected"
        expected_err_file = "test_data/test1b.console.ets.expected.err"
        report_stdout = "default: 2\n   default: 2\nuserClick: 1\n    userClick: 2\nuserClick: 3"
        test_result = get_test_instance(report_stdout, "",
                                        expected_file, expected_err_file, fail_kind="Other")
        passed = test_result.compare_output_with_expected(report_stdout, "")

        self.assertFalse(passed)

    def test_expected_stderr_ok(self) -> None:
        expected_file = "test_data/test2.console.ets.expected"
        expected_err_file = "test_data/test2.console.ets.expected.err"
        report_stderr = "default: 1\n  default: 2\ndefault: 3\n     userClick: 1\nuserClick: 2\nuserClick: 3"
        test_result = get_test_instance("report_stdout", report_stderr, expected_file, expected_err_file)
        passed = test_result.compare_output_with_expected("", report_stderr)

        self.assertTrue(passed)

    def test_expected_stderr_some_lines_ok(self) -> None:
        expected_file = "test_data/test2.console.ets.expected"
        expected_err_file = "test_data/test2.console.ets.expected.err"
        report_stderr = "default: 1\n  default: 2\ndefault: 3\n     userClick: 1\nuserClick: 2\nuserClick: 3"
        test_result = get_test_instance("report_stdout", report_stderr, expected_file, expected_err_file)
        passed = test_result.compare_output_with_expected("", report_stderr)

        self.assertTrue(passed)

    def test_expected_stderr_fail(self) -> None:
        expected_file = "test_data/test2.console.ets.expected"
        expected_err_file = "test_data/test2.console.ets.expected.err"
        report_stderr = "default: 2\ndefault: 2\n   default: 3\nuserClick: 1\nuserClick: 2\n  userClick: 3"
        test_result = get_test_instance("", report_stderr,
                                        expected_file, expected_err_file, fail_kind="Other")
        passed = test_result.compare_output_with_expected("", report_stderr)

        self.assertFalse(passed)

    def test_expected_stderr_some_lines_fail(self) -> None:
        expected_file = "test_data/test2b.console.ets.expected"
        expected_err_file = "test_data/test2b.console.ets.expected.err"
        report_stderr = "default: 2\ndefault: 2\n   default: 3\nuserClick: 1\n  userClick: 3"
        test_result = get_test_instance("", report_stderr,
                                        expected_file, expected_err_file, fail_kind="Other")
        passed = test_result.compare_output_with_expected("", report_stderr)

        self.assertFalse(passed)

    def test_compare_both_files(self) -> None:
        expected_file = "test_data/test3.console.ets.expected"
        expected_err_file = "test_data/test3.console.ets.expected.err"
        report_stdout = "default: 1\ndefault: 2\n default: 3\nuserClick: 1\n     userClick: 2\n      userClick: 3"
        report_stderr = "default: 4\ndefault: 2\ndefault: 3\n userClick: 1\nuserClick: 2\nuserClick: 3"
        test_result = get_test_instance(report_stdout, report_stderr, expected_file, expected_err_file)
        passed = test_result.compare_output_with_expected(report_stdout, report_stderr)

        self.assertTrue(passed)

    def test_compare_both_some_lines_files(self) -> None:
        expected_file = "test_data/test3b.console.ets.expected"
        expected_err_file = "test_data/test3b.console.ets.expected.err"
        report_stdout = "default: 1\ndefault: 2\n default: 3\nuserClick: 1\n     userClick: 2\n      userClick: 3"
        report_stderr = "default: 4\ndefault: 2\ndefault: 3\n userClick: 1\nuserClick: 2\nuserClick: 3"
        test_result = get_test_instance(report_stdout, report_stderr, expected_file, expected_err_file)
        passed = test_result.compare_output_with_expected(report_stdout, report_stderr)

        self.assertTrue(passed)
