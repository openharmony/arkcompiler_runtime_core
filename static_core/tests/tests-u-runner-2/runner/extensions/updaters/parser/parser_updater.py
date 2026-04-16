#!/usr/bin/env python3
# -- coding: utf-8 --
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
#

from pathlib import Path

from runner import utils
from runner.extensions.updaters.iupdater import IUpdater
from runner.logger import Log
from runner.options.options_step import StepKind
from runner.types.step_report import StepReport

_LOGGER = Log.get_logger(__file__)


class ParserUpdater(IUpdater):
    """Updater for parser test suite expected output files.
    
    Processes compiler step reports and updates the corresponding expected
    output files with normalized build system paths for parser tests.
    """
    def process(self, test_file: Path, data: list[StepReport], _: bool = False) -> None:
        """Process parser test results and update expected output file.
        
        Args:
            test_file: Path to the test file
            data: List of step reports from test execution
            _: Unused boolean parameter (use_metadata)
        """
        expected_name = f"{test_file.stem}-expected"
        expected_path = test_file.with_stem(expected_name).with_suffix(".txt")
        compiler_reports = [report for report in data if report.step_kind == StepKind.COMPILER]
        if not compiler_reports:
            return
        report = compiler_reports[0]
        normalized_output = utils.normalize_build_system_path(report.cmd_output)
        expected_path.write_text(normalized_output, encoding="utf-8", newline="\n")
        _LOGGER.all(f"[{test_file.name}] Updated expected output file: {expected_path}")
