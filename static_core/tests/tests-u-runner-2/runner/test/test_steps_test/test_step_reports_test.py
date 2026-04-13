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

import os
from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path
from typing import ClassVar, cast
from unittest import TestCase
from unittest.mock import patch

from runner.environment import MandatoryPropDescription, RunnerEnv
from runner.options.cli_options import get_args
from runner.options.config import Config
from runner.suites.runner_standard_flow import RunnerStandardFlow
from runner.suites.test_standard_flow import TestStandardFlow
from runner.test import test_utils
from runner.test.test_utils import test_cmake_build
from runner.types.step_report import StepReport


@dataclass
class TestStep:
    name: str
    type: str
    args: list[str]


COMPILER_STEP = "compiler"
RUNTIME_STEP = "runtime"
GTEST_RUNNER = "gtest-runner"
INTERMEDIATE = "intermediate"


class TestStepReportsTest(TestCase):
    data_folder: ClassVar[Callable[[], Path]] = lambda: test_utils.data_folder(__file__)
    test_environ: ClassVar[dict[str, str]] = test_utils.test_environ(
        'TEST_STEPS_TEST_ROOT', data_folder().as_posix())
    get_instance_id: ClassVar[Callable[[], str]] = lambda: test_utils.create_runner_test_id(__file__)
    env_properties: ClassVar[list[MandatoryPropDescription]] = RunnerEnv.mandatory_props

    @staticmethod
    def prepare() -> list[TestStandardFlow]:
        args = get_args(TestStepReportsTest.env_properties)
        runner = RunnerStandardFlow(Config(args))
        return [cast(TestStandardFlow, test.do_run()) for test in runner.tests]

    @staticmethod
    def get_error_message(field: str, expected: str, actual: str) -> str:
        return f"Step '{field}' is '{actual}', but expected '{expected}'"

    def assert_step_report_equal(self, expected: StepReport, actual: StepReport) -> None:
        self.assertEqual(expected.name, actual.name,
                         self.get_error_message("step", expected.name, actual.name))
        self.assertListEqual(expected.command_line.split(" "), actual.command_line.split(" "),
                             self.get_error_message("command line", expected.command_line, actual.command_line))
        self.assertEqual(expected.cmd_output, actual.cmd_output,
                         self.get_error_message("output", expected.cmd_output, actual.cmd_output))
        self.assertEqual(expected.cmd_error, actual.cmd_error,
                         self.get_error_message("error", expected.cmd_error, actual.cmd_error))
        self.assertEqual(expected.cmd_return_code, actual.cmd_return_code,
                         self.get_error_message("return code",
                                                str(expected.cmd_return_code), str(actual.cmd_return_code)))
        self.assertEqual(expected.extra, actual.extra,
                         self.get_error_message("extra", expected.extra, actual.extra))
        self.assertEqual(expected.status, actual.status,
                         self.get_error_message("status", expected.status, actual.status))

    def test_empty_step_report(self) -> None:
        """Create empty step report"""
        empty_step_report = StepReport("test")
        self.assertEqual("", str(empty_step_report))

    def test_filled_step_report1(self) -> None:
        """Tests minimal configuration to create step report"""
        report = StepReport(
            name="step",
            command_line="cmd"
        )
        expected_str = """step: cmd
step: Actual output: ''
step: Actual error: ''
step: Actual return code: 0"""
        self.assertEqual(expected_str, str(report).strip())

    def test_filled_step_report2(self) -> None:
        """Tests maximal configuration to create step report"""
        report = StepReport(
            name="step",
            command_line="cmd",
            cmd_output="output",
            cmd_error="error",
            cmd_return_code=3,
            validator_messages="validators",
            extra="extra",
            status="status"
        )
        expected_str = """step: cmd
step: Actual output: 'output'
step: Actual error: 'error'
step: Actual return code: 3
step: Validator messages: validators
step: Extra: extra
step: Step status: status"""
        self.assertEqual(expected_str, str(report).strip())

    def test_duplicated_step_report(self) -> None:
        """
        Tests that duplicated step reports are not added to the set
        Step reports are considered the same if they have the same command line
        """
        report1 = StepReport(
            name="step1",
            command_line="cmd",
            cmd_output="output",
            cmd_error="error",
            cmd_return_code=3,
            validator_messages="validators",
            extra="extra",
            status="status"
        )
        report2 = StepReport(
            name="step1",
            command_line="cmd",
            cmd_output="output",
            cmd_error="error",
            cmd_return_code=0,
        )
        self.assertEqual(report1, report2)
        reports = list({report1, report2})
        self.assertEqual(1, len(reports))
        self.assertEqual(report1, reports[0])
        self.assertEqual(report2, reports[0])

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists._cmake_build_properties', test_cmake_build)
    @patch('runner.suites.test_lists.TestLists._gn_build_properties', test_utils.test_gn_build)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @test_utils.parametrized_test_cases([
        (["runner.sh", "workflow", "test_suite", "--test-file=simple1.ets"],),
    ])
    def test_report_one_test(self, argv: Callable) -> None:
        """
        Feature: for every executed step we have a separate step report
        Expected:
        - 2 step reports are generated
        """
        with patch('sys.argv', argv):
            # preparation
            result = self.prepare()[0]
            # test
            try:
                self.assertEqual(2, len(result.step_reports))

                first = result.step_reports[0]
                work_dir = Path(os.environ["WORK_DIR"])
                first_args = f"--output={work_dir.as_posix()}/intermediate/simple1.ets.abc simple1.ets"
                expected_first_step_report = StepReport(
                    name="echo-compiler",
                    command_line=f"/usr/bin/echo {first_args}",
                    cmd_return_code=0,
                    cmd_output=first_args,
                    status="ECHO-COMPILER_PASSED"
                )
                self.assert_step_report_equal(expected_first_step_report, first)

                second = result.step_reports[1]
                second_args = (f"--output={work_dir.as_posix()}/intermediate/simple1.ets.abc "
                               "--verification-mode=ahead-of-time "
                               f"--panda-files={work_dir.as_posix()}/intermediate/simple1.ets.abc "
                               "--boot-panda-files=stdlib "
                               "simple1.ets")
                expected_second_step_report = StepReport(
                    name="echo-runtime",
                    command_line=f"/usr/bin/echo {second_args}",
                    cmd_return_code=0,
                    cmd_output=second_args,
                    status="ECHO-RUNTIME_PASSED"
                )
                self.assert_step_report_equal(expected_second_step_report, second)

            finally:
                test_utils.clear_after_test()
