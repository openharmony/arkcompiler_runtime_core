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
#

import argparse
from functools import cached_property
from pathlib import Path
from typing import Any, cast

from runner.options.options import IOptions
from runner.reports.report_format import ReportFormat

DETAILED_REPORT = "detailed-report"
DETAILED_REPORT_FILE = "detailed-report-file"
REPORT_DIR = "report-dir"
SPEC_REPORT = "spec-report"
SPEC_REPORT_FILE = "spec-report-file"
SPEC_REPORT_YAML = "spec-report-yaml"
SPEC_FILE = "spec-file"

DEFAULT_DETAILED_REPORT = False
DEFAULT_DETAILED_REPORT_FILE = "detailed-report-file"
DEFAULT_REPORT_DIR = "report"
DEFAULT_SPEC_REPORT = False
DEFAULT_REPORT_FORMAT = ReportFormat.LOG


class ReportOptions(IOptions):
    """
    Report-related options for the test runner: detailed report, spec coverage report,
    report directory, and output file paths. Parsed from CLI arguments and YAML configuration.
    """

    def __init__(self, args: dict[str, Any]):  # type: ignore[explicit-any]
        super().__init__(args)
        self._parameters = args

    def __str__(self) -> str:
        return self._to_str(indent=2)

    @staticmethod
    def add_cli_args(parser: argparse.ArgumentParser, dest: str | None = None) -> None:
        report_config = parser.add_argument_group(title="URunner report options")
        report_config.add_argument(
            f'--{DETAILED_REPORT}', action='store_true',
            default=DEFAULT_DETAILED_REPORT,
            dest=f"{dest}{DETAILED_REPORT}",
            help='Create additional detailed report with counting tests for each folder.')
        report_config.add_argument(
            f'--{DETAILED_REPORT_FILE}', action='store',
            default=DEFAULT_DETAILED_REPORT_FILE,
            dest=f"{dest}{DETAILED_REPORT_FILE}",
            help='Name of additional detailed report. By default, the report is created at '
                 '$WorkDir/<suite-name>/report/<suite-name>_detailed-report-file.md , '
                 'where $WorkDir is the folder specified by the environment variable WORK_DIR')
        report_config.add_argument(
            f'--{REPORT_DIR}', action='store',
            default=DEFAULT_REPORT_DIR,
            dest=f"{dest}{REPORT_DIR}",
            help='Name of report folder under $WorkDir. By default, the name is "report".'
                 'The location is "$WorkDir/<suite-name>/<report-dir>", '
                 'where $WorkDir is the folder specified by the environment variable WORK_DIR')
        report_config.add_argument(
            f'--{SPEC_REPORT}', action='store_true',
            default=DEFAULT_SPEC_REPORT,
            dest=f"{dest}{SPEC_REPORT}",
            help='Creates specification coverage report in two formats: markdown and YAML. '
                 'By default the report files are written into $WorkDir/report/ directory.')
        report_config.add_argument(
            f'--{SPEC_REPORT_FILE}', action='store',
            default=None,
            dest=f"{dest}{SPEC_REPORT_FILE}",
            help='Custom file name for the specification coverage report in markdown format. '
                 'By default the report is $WorkDir/report/<suite name>_spec-report.md')
        report_config.add_argument(
            f'--{SPEC_REPORT_YAML}', action='store',
            default=None,
            dest=f"{dest}{SPEC_REPORT_YAML}",
            help='Custom file name for the specification coverage report in YAML format. '
                 'By default the report is $WorkDir/report/<suite name>_spec-report.yaml')
        report_config.add_argument(
            f'--{SPEC_FILE}', action='store',
            default=None,
            dest=f"{dest}{SPEC_FILE}",
            help='Path to specification PDF file.')

    @cached_property
    def report_format(self) -> ReportFormat:
        return DEFAULT_REPORT_FORMAT

    @cached_property
    def detailed_report(self) -> bool:
        return cast(bool, self._parameters.get(DETAILED_REPORT, DEFAULT_DETAILED_REPORT))

    @cached_property
    def detailed_report_file(self) -> Path | None:
        path_str = self._parameters.get(DETAILED_REPORT_FILE, DEFAULT_DETAILED_REPORT_FILE)
        if path_str:
            return Path(cast(str, path_str))
        return None

    @cached_property
    def report_dir_name(self) -> str:
        return cast(str, self._parameters.get(REPORT_DIR, DEFAULT_REPORT_DIR))

    @cached_property
    def spec_report(self) -> bool:
        return cast(bool, self._parameters.get(SPEC_REPORT, DEFAULT_SPEC_REPORT))

    @cached_property
    def spec_report_file(self) -> Path | None:
        path_str = self._parameters.get(SPEC_REPORT_FILE, None)
        if path_str:
            return Path(cast(str, path_str))
        return None

    @cached_property
    def spec_report_yaml(self) -> Path | None:
        path_str = self._parameters.get(SPEC_REPORT_YAML, None)
        if path_str:
            return Path(cast(str, path_str))
        return None

    @cached_property
    def spec_file(self) -> Path | None:
        path_str = self._parameters.get(SPEC_FILE, None)
        if path_str:
            return Path(cast(str, path_str))
        return None
