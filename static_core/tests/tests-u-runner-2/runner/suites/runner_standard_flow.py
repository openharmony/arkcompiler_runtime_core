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
#

from datetime import datetime
from pathlib import Path
from typing import Optional

import pytz

from runner import utils
from runner.code_coverage.coverage import LlvmCov
from runner.common_exceptions import InvalidConfiguration
from runner.cpumask import CPUMask
from runner.enum_types.params import TestEnv
from runner.logger import Log
from runner.options.config import Config
from runner.runner_file_based import RunnerFileBased
from runner.suites.test_suite import TestSuite
from runner.suites.work_dir import WorkDir

_LOGGER = Log.get_logger(__file__)


class RunnerStandardFlow(RunnerFileBased):
    __DEFAULT_SUITE_NAME = "custom-scheme"

    def __init__(self, config: Config):
        self.suite_name = config.test_suite.suite_name if config.test_suite.suite_name else self.__DEFAULT_SUITE_NAME
        self.steps = config.workflow.steps
        RunnerFileBased.__init__(self, config, self.suite_name)

        self.__aot_check()

        self.test_env = TestEnv(
            config=config,
            cmd_prefix=self.cmd_prefix,
            cmd_env=self.cmd_env,
            timestamp=int(datetime.timestamp(datetime.now(pytz.UTC))),
            report_formats={self.config.general.report_format},
            work_dir=WorkDir(config, self.default_work_dir_root),
            coverage=LlvmCov(self.config.general.build, self.work_dir),
        )

        test_suite: TestSuite = TestSuite(self.test_env)
        self.tests = set(test_suite.process(self.config.test_suite.ets.force_generate))
        self.list_root = test_suite.list_root
        self.test_root = self.test_env.work_dir.gen

        cpu_mask = self.config.test_suite.get_parameter("mask")
        self.cpu_mask: Optional[CPUMask] = CPUMask(cpu_mask) \
            if cpu_mask is not None and isinstance(cpu_mask, str) else None

    @property
    def default_work_dir_root(self) -> Path:
        return Path("/tmp") / self.suite_name

    def create_execution_plan(self) -> None:
        _LOGGER.default(f"\n{utils.pretty_divider()}")
        _LOGGER.default(f"Execution plan for the test suite '{self.name}'")
        _LOGGER.default(self.config.workflow.pretty_str())

    def create_coverage_html(self) -> None:
        pass

    def before_suite(self) -> None:
        if self.cpu_mask:
            self.cpu_mask.apply()

    def after_suite(self) -> None:
        if self.cpu_mask:
            self.cpu_mask.restore()

    def __aot_check(self) -> None:
        if self.config.general.aot_check:
            aot_check_passed: bool = False
            for step in self.steps:
                if step.executable_path is None:
                    continue
                all_args = " ".join(step.args)
                if step.executable_path.name == 'ark_js_vm' or (
                        step.executable_path.name == 'hdc' and all_args.find('ark_js_vm') > -1
                ):
                    aot_check_passed = aot_check_passed or all_args.find('--aot-file') > -1
            if not aot_check_passed:
                raise InvalidConfiguration(
                    f"AOT check FAILED. Check command line options of steps running `ark_js_vm` in workflow"
                    f"configuration file {self.config.workflow.name}. The expected option `--aot-file` absent."
                )
            _LOGGER.default("AOT check PASSED")
