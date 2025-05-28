#!/usr/bin/env python3
# -- coding: utf-8 --
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
#

from __future__ import annotations

from datetime import datetime
from pathlib import Path

import pytz

from runner.enum_types.base_enum import BaseEnum
from runner.enum_types.params import TestEnv, TestReport
from runner.enum_types.verbose_format import VerboseFilter, VerboseKind
from runner.logger import Log
from runner.options.local_env import LocalEnv
from runner.options.options import IOptions
from runner.reports.report import ReportGenerator
from runner.reports.report_format import ReportFormat

_LOGGER = Log.get_logger(__file__)


class TestStatus(BaseEnum):
    PASSED = "PASSED"
    PASSED_IGNORED = "PASSED IGNORED"
    FAILURE_IGNORED = "FAILURE IGNORED"
    EXCLUDED = "EXCLUDED"
    NEW_FAILURE = "NEW FAILURE"


class Test:
    """
    Test knows how to run test and defines whether the test is failed or passed
    """

    def __init__(self, test_env: TestEnv, test_path: Path, params: IOptions, test_id: str):
        self.test_env = test_env
        # full path to the test file
        self.path = test_path
        self.test_extra_params: IOptions = params
        self.test_id = test_id
        self.update_expected = self.test_env.config.test_suite.test_lists.update_expected
        # Expected output. Used only in the Parser test suite
        self.expected = ""
        # Contains fields output, error, and return_code of the last executed step
        self.report: TestReport | None = None
        # Test result: True if all steps passed, False is any step fails
        self.passed: bool | None = None
        # If the test is mentioned in any ignored_list
        self.ignored = False
        # Test can detect itself as excluded additionally to excluded_tests
        # In such case the test will be counted as `excluded by other reasons`
        self.excluded = False
        # Collect all executable commands
        self.reproduce = ""
        # Time to execute in seconds
        self.time: float | None = None
        # Reports if generated. Key is ReportFormat.XXX. Value is a path to the generated report
        self.reports: dict[ReportFormat, str] = {}
        self.repeat = 1

    def run(self, repeat: int) -> Test:
        start = datetime.now(pytz.UTC)
        _LOGGER.all(f"\033[1mStarted:\033[0m \033[1;33m{self.test_id}\033[0m. Launch #{repeat}")
        self.repeat = repeat
        self.reproduce = ''

        result = self.do_run()

        finish = datetime.now(pytz.UTC)
        self.time = (finish - start).total_seconds()

        if result.passed is None:
            return result

        if self.is_output_status():
            Log.default(
                _LOGGER,
                f"\033[1mFinished:\033[0m \033[1;33m{self.test_id}\033[0m. Launch #{repeat}"
                f"- {round(self.time, 2)} sec - {self.status_as_cstring()}")
        self.reproduce += f"\nTo reproduce with URunner run:\n{self.get_command_line()}"
        self.reproduce += f"\nProvide following environment variables: {LocalEnv.get().list()}"
        if self.is_output_log():
            _LOGGER.default(f"{self.test_id}: steps: {self.reproduce}")
            if not self.report:
                Log.default(
                    _LOGGER,
                    f"{self.test_id}: no information about test running neither output nor error")

        report_generator = ReportGenerator(self.test_id, self.test_env, repeat)
        self.reports = report_generator.generate_reports(result)

        return result

    def do_run(self) -> Test:
        return self

    def status(self) -> TestStatus:
        if self.excluded:
            return TestStatus.EXCLUDED
        if self.passed:
            return TestStatus.PASSED_IGNORED if self.ignored else TestStatus.PASSED
        return TestStatus.FAILURE_IGNORED if self.ignored else TestStatus.NEW_FAILURE

    def status_as_cstring(self) -> str:
        if self.excluded:
            return TestStatus.EXCLUDED.value
        if self.passed:
            return f"\033[3;92m{TestStatus.PASSED_IGNORED.value}\033[0m" \
                if self.ignored else f"\033[1;32m{TestStatus.PASSED.value}\033[0m"
        return f"\033[3;91m{TestStatus.FAILURE_IGNORED.value}\033[0m" \
            if self.ignored else f"\033[1;31m{TestStatus.NEW_FAILURE.value}\033[0m"

    def get_command_line(self) -> str:
        config_cmd = self.test_env.config.get_command_line()
        reproduce_message = [
            f'{self.test_env.config.general.static_core_root}/tests/tests-u-runner-2/runner.sh',
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
            is_ignored = verbose_filter == VerboseFilter.IGNORED and \
                         status in [TestStatus.NEW_FAILURE, TestStatus.FAILURE_IGNORED, TestStatus.PASSED_IGNORED]
            is_new = verbose_filter == VerboseFilter.NEW_FAILURES and \
                     status == TestStatus.NEW_FAILURE
            return is_all or is_ignored or is_new

        return verbose == VerboseKind.SILENT and status == TestStatus.NEW_FAILURE

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
        if verbose == VerboseKind.SILENT:
            is_all = verbose_filter == VerboseFilter.ALL
            is_ignored = verbose_filter == VerboseFilter.IGNORED and \
                         status in [TestStatus.NEW_FAILURE, TestStatus.FAILURE_IGNORED, TestStatus.PASSED_IGNORED]
            is_new = verbose_filter == VerboseFilter.NEW_FAILURES and \
                     status == TestStatus.NEW_FAILURE
            return is_all or is_ignored or is_new

        return True
