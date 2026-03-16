#!/usr/bin/env python3
# -- coding: utf-8 --
#
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

from abc import abstractmethod
from dataclasses import dataclass, field
from datetime import datetime
from functools import cached_property
from os import environ
from pathlib import Path

import pytz

from runner import common_exceptions
from runner.enum_types.qemu import QemuKind
from runner.logger import Log
from runner.options.config import Config
from runner.reports.detailed_report import DetailedReport
from runner.reports.html_view import HtmlView
from runner.reports.report_format import ReportFormat
from runner.reports.standard_view import StandardView
from runner.reports.summary import Summary
from runner.reports.xml_view import XmlView
from runner.runner_base import Runner
from runner.suites.gtest_flow import GTestFlow
from runner.suites.test_standard_flow import TestStandardFlow
from runner.suites.work_dir import WorkDir
from runner.test_base import Test
from runner.types.test_env import TestEnv

_LOGGER = Log.get_logger(__file__)


@dataclass
class ResultsAnalysis:
    ignored_but_passed: list[Test] = field(default_factory=list)
    ignored_still_failed: list[Test] = field(default_factory=list)
    excluded_but_passed: list[Test] = field(default_factory=list)
    excluded_still_failed: list[Test] = field(default_factory=list)
    excluded_after: list[Test] = field(default_factory=list)
    fail_lists: dict[str, list[Test]] = field(default_factory=dict)


class RunnerUtils:
    @staticmethod
    def set_cmd_prefix(config: Config) -> list[str]:
        match config.general.qemu:
            case QemuKind.ARM64:
                return ["qemu-aarch64", "-L", "/usr/aarch64-linux-gnu/"]
            case QemuKind.ARM32:
                return ["qemu-arm", "-L", "/usr/arm-linux-gnueabihf"]
            case _:
                return []


class RunnerFileBased(Runner):

    def __init__(self, config: Config, name: str) -> None:
        Runner.__init__(self, config, name)
        self.cmd_env = environ.copy()
        self.cmd_prefix = RunnerUtils.set_cmd_prefix(config)

        self.__set_test_list_options()

        self.test_env: TestEnv | None = None

    @property
    @abstractmethod
    def default_work_dir_root(self) -> Path:
        pass

    @cached_property
    def work_dir(self) -> WorkDir:
        return WorkDir(self.config, self.default_work_dir_root)

    @abstractmethod
    def create_coverage_html(self) -> None:
        pass

    @abstractmethod
    def create_execution_plan(self) -> None:
        pass

    def summarize(self) -> int:
        _LOGGER.all("Processing run statistics")

        results = [result for result in self.results if result.passed is not None]
        analysis_results = self.__results_analysis(results)

        total_tests = len(results) + self.excluded
        summary = Summary(
            self.name, total_tests, self.passed, self.failed,
            self.ignored, len(analysis_results.ignored_but_passed), self.excluded, self.excluded_after
        )

        if results:
            xml_view = XmlView(self.work_dir.report, summary)
            execution_time = round((datetime.now(pytz.UTC) - self.start_time).total_seconds(), 3)
            xml_view.create_xml_report(results, execution_time)
            xml_view.create_ignore_list(set(results))
            self.__generate_detailed_report(results)

        if self.test_env is None:
            raise common_exceptions.UnknownException("It seems no one test has been launched.")
        if ReportFormat.HTML in self.test_env.report_formats:
            html_view = HtmlView(self.work_dir.report, self.config, summary)
            html_view.create_html_index(analysis_results.fail_lists, self.test_env.timestamp)

        summary.passed = summary.passed - summary.ignored_but_passed

        standard_view = StandardView(self.work_dir.report, self.update_excluded, self.excluded_lists, summary)
        standard_view.summarize_failures(analysis_results.fail_lists)
        standard_view.create_failures_report(analysis_results.fail_lists)
        standard_view.create_update_excluded(analysis_results.excluded_but_passed,
                                             analysis_results.excluded_still_failed)
        standard_view.summarize_passed_ignored(analysis_results.ignored_but_passed)
        self.create_execution_plan()
        standard_view.display_summary(analysis_results.fail_lists)
        standard_view.create_time_report(results, self.config.general.time_report)

        return self.failed

    def _process_failed(self, test_result: TestStandardFlow | GTestFlow, analysis: ResultsAnalysis) -> None:
        if test_result.ignored and test_result.last_failure_check_passed:
            self.ignored += 1
            analysis.ignored_still_failed.append(test_result)
        elif test_result.path in self.excluded_tests:
            analysis.excluded_still_failed.append(test_result)
            self.excluded += 1
        else:
            self.failed += 1
            if test_result.fail_kind is None:
                test_result.fail_kind = "<not set>"
            if test_result.fail_kind not in analysis.fail_lists:
                analysis.fail_lists[test_result.fail_kind] = []
            analysis.fail_lists[test_result.fail_kind].append(test_result)

    def __results_analysis(self, results: list[Test]) -> ResultsAnalysis:
        self.failed = 0
        self.ignored = 0
        self.passed = 0
        self.excluded_after = 0
        self.excluded = 0 if self.update_excluded else self.excluded

        analysis = ResultsAnalysis()

        for test_result in results:
            if not isinstance(test_result, TestStandardFlow) and not isinstance(test_result, GTestFlow):
                raise ValueError(f"Incorrect type of test {type(test_result)}. "
                                 "Expected: TestStandardFlow or GTestFlow")

            if test_result.excluded:
                self.excluded_after += 1
                analysis.excluded_after.append(test_result)
                continue

            if not test_result.passed:
                self._process_failed(test_result, analysis)
            else:
                self.passed += 1
                if test_result.ignored:
                    analysis.ignored_but_passed.append(test_result)
                if test_result.path in self.excluded_tests:
                    analysis.excluded_but_passed.append(test_result)
        return analysis

    def __set_test_list_options(self) -> None:
        for san in ["ASAN_OPTIONS", "TSAN_OPTIONS", "MSAN_OPTIONS", "LSAN_OPTIONS"]:
            # we don't want to interpret asan failures as SyntaxErrors
            self.cmd_env[san] = ":exitcode=255"

    def __generate_detailed_report(self, results: list[Test]) -> None:
        if self.config.general.detailed_report:
            detailed_report = DetailedReport(
                results,
                self.name,
                self.work_dir.report,
                self.config.general.detailed_report_file)
            detailed_report.populate_report()
