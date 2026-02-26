#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

from __future__ import annotations

import logging
from datetime import datetime
from enum import Enum
from typing import Dict, List, Optional

import pytz

from runner.enum_types.params import TestEnv, TestReport
from runner.enum_types.verbose_format import VerboseFilter, VerboseKind
from runner.logger import Log
from runner.reports.report import ReportGenerator
from runner.reports.report_format import ReportFormat
from runner.utils import unlines

_LOGGER = logging.getLogger("runner.test_base")


class TestStatus(Enum):
    PASSED = "PASSED"
    PASSED_IGNORED = "PASSED IGNORED"
    FAILURE_IGNORED = "FAILURE IGNORED"
    EXCLUDED = "EXCLUDED"
    NEW_FAILURE = "NEW FAILURE"


class Test:
    """
    Test knows how to run test and defines whether the test is failed or passed
    """

    def __init__(self, test_env: TestEnv, test_path: str, flags: List[str], test_id: str):
        self.test_env = test_env
        # full path to the test file
        self.path = test_path
        self.flags = flags
        self.test_id = test_id
        self.expected = ""
        self.expected_err = ""
        # Contains fields output, error, and return_code of the last executed step
        self.report: Optional[TestReport] = None
        # Test result: True if all steps passed, False is any step fails
        self.passed: Optional[bool] = None
        # If the test is mentioned in any ignored_list
        self.ignored = False
        # Test can detect itself as excluded additionally to excluded_tests
        # In such case the test will be counted as `excluded by other reasons`
        self.excluded = False
        # Collect all executable commands
        self.reproduce: List[str] = []
        # Time to execute in seconds
        self.time: Optional[float] = None
        # Reports if generated. Key is ReportFormat.XXX. Value is a path to the generated report
        self.reports: Dict[ReportFormat, str] = {}
        self.coverage = self.test_env.coverage

    @property
    def should_update_expected(self) -> bool:
        b = self.test_env.config.test_lists.update_expected
        # NOTE(pronai) mypy erroneously thinks `b` is Any
        if not isinstance(b, bool):
            raise TypeError("expected a bool, got something else:", b)
        return b

    @staticmethod
    def output_status_and_log(result: Test) -> None:
        if result.is_output_status():
            Test.output_status(result)
        if result.is_output_log():
            Test.output_log(result)

    @staticmethod
    def output_status(result: Test) -> None:
        if result.time is None:
            Log.default(_LOGGER, f"Finished {result.test_id} - not executed - {result.status().value}")
        else:
            message = f"Finished {result.test_id} - {round(result.time, 2)} sec - {result.status().value}"
            Log.default(_LOGGER, message)

    @staticmethod
    def output_log(result: Test) -> None:
        result.reproduce.append(f"\nTo reproduce with URunner run:\n{result.get_command_line()}")
        Log.default(_LOGGER, f"{result.test_id}: steps: {result.steps_to_reproduce()}")
        if result.report:
            Log.default(_LOGGER, f"{result.test_id}: expected output: {result.expected}")
            Log.default(_LOGGER, f"{result.test_id}: actual output: {result.report.output}")
            Log.default(_LOGGER, f"{result.test_id}: actual error: {result.report.error}")
            Log.default(_LOGGER, f"{result.test_id}: actual return code: {result.report.return_code}")
        else:
            Log.default(_LOGGER, f"{result.test_id}: no information about test running neither output nor error")

    @staticmethod
    def create_report(result: Test) -> None:
        report_generator = ReportGenerator(result.test_id, result.test_env)
        result.reports = report_generator.generate_fail_reports(result)

    def steps_to_reproduce(self) -> str:
        return unlines(iter(self.reproduce))

    # this is used as a general text log and it also doesn't escape whitespace
    # NOTE(pronai) use List[str] and escape for pasting into bash
    def log_cmd(self, cmd: str) -> None:
        self.reproduce.append(cmd)

    def run(self) -> Test:
        start = datetime.now(pytz.UTC)
        Log.all(_LOGGER, f"Start to execute: {self.test_id}")

        result = self.do_run()

        finish = datetime.now(pytz.UTC)
        self.time = (finish - start).total_seconds()

        if result.passed is None:
            return result

        self.output_status_and_log(result)
        self.create_report(result)

        return self

    def do_run(self) -> Test:
        return self

    def status(self) -> TestStatus:
        if self.excluded:
            return TestStatus.EXCLUDED
        if self.passed:
            return TestStatus.PASSED_IGNORED if self.ignored else TestStatus.PASSED
        return TestStatus.FAILURE_IGNORED if self.ignored else TestStatus.NEW_FAILURE

    def get_command_line(self) -> str:
        config_cmd = self.test_env.config.get_command_line()
        reproduce_message = [
            f'{self.test_env.config.general.static_core_root}/tests/tests-u-runner/runner.sh',
            config_cmd
        ]
        if config_cmd.find('--test-file') < 0:
            reproduce_message.append(f" --test-file {self.test_id}")
        return ' '.join(reproduce_message)

    def is_output_log(self) -> bool:
        """
        Returns True if logs should be logged
        verbose in [ALL, SHORT] and filter == ALL -> True
        verbose in [ALL, SHORT] and filter == KNOWN and status = New or Known or Passed Ignored -> True
        verbose in [ALL, SHORT] and filter == NEW and status = New -> True

        verbose == NONE and status == New -> True
        """
        verbose_filter = self.test_env.config.general.verbose_filter
        verbose = self.test_env.config.general.verbose
        status = self.status()
        if verbose in [VerboseKind.ALL, VerboseKind.SHORT]:
            is_all = verbose_filter == VerboseFilter.ALL
            is_ignored = (verbose_filter == VerboseFilter.IGNORED and
                          status in [TestStatus.NEW_FAILURE, TestStatus.FAILURE_IGNORED, TestStatus.PASSED_IGNORED])
            is_new = (verbose_filter == VerboseFilter.NEW_FAILURES and
                      status == TestStatus.NEW_FAILURE)
            return is_all or is_ignored or is_new

        return verbose == VerboseKind.NONE and status == TestStatus.NEW_FAILURE

    def is_output_status(self) -> bool:
        """
        Returns True if status should be logged
        verbose in [ALL, SHORT] -> True

        verbose == NONE and filter == ALL -> True
        verbose == NONE and filter == KNOWN and status = New or Known or Passed Ignored -> True
        verbose == NONE and filter == NEW and status = New -> True
        """
        verbose = self.test_env.config.general.verbose
        verbose_filter = self.test_env.config.general.verbose_filter
        status = self.status()
        if verbose == VerboseKind.NONE:
            is_all = verbose_filter == VerboseFilter.ALL
            is_ignored = (verbose_filter == VerboseFilter.IGNORED and
                          status in [TestStatus.NEW_FAILURE, TestStatus.FAILURE_IGNORED, TestStatus.PASSED_IGNORED])
            is_new = (verbose_filter == VerboseFilter.NEW_FAILURES and
                      status == TestStatus.NEW_FAILURE)
            return is_all or is_ignored or is_new

        return True
