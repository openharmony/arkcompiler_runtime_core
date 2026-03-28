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
import shutil
from functools import cached_property
from pathlib import Path

from runner.code_coverage.coverage_dir import CoverageDir
from runner.options.config import Config


class WorkDir:
    _FAILURES_REPORT_FILE_NAME = "failures.txt"
    _RERUN_FAILURES_FILE_NAME = ".rerun-failures.txt"

    def __init__(self, config: Config, default_work_dir: Path):
        self.__config = config
        self.__default_work_dir = default_work_dir
        self.__rerun_failures = None
        if config.test_suite.test_lists.rerun_failed:
            self.__rerun_failures = self.__preserve_previous_failures()
        shutil.rmtree(self.report, ignore_errors=True)

    @property
    def report(self) -> Path:
        return self.root / self.__config.general.report.report_dir_name

    @property
    def gen(self) -> Path:
        return self.root / "gen"

    @property
    def intermediate(self) -> Path:
        return self.root / "intermediate"

    @property
    def rerun_failures(self) -> Path | None:
        if self.__rerun_failures is None or not self.__rerun_failures.exists():
            return None
        return self.__rerun_failures

    @cached_property
    def root(self) -> Path:
        root = Path(self.__config.test_suite.work_dir) if self.__config.test_suite.work_dir else self.__default_work_dir
        root.mkdir(parents=True, exist_ok=True)
        return root

    @cached_property
    def coverage_dir(self) -> CoverageDir:
        return CoverageDir(self.__config.general, self.root)

    def __preserve_previous_failures(self) -> Path | None:
        rerun_failures = self.root / self._RERUN_FAILURES_FILE_NAME
        rerun_failures.unlink(missing_ok=True)

        previous_failures = self.report / self._FAILURES_REPORT_FILE_NAME
        if not previous_failures.exists():
            return None

        shutil.copy2(previous_failures, rerun_failures)
        return rerun_failures
