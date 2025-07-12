#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2025 Huawei Device Co., Ltd.
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
import shutil
from collections.abc import Generator
from datetime import datetime
from pathlib import Path
from typing import ClassVar, NamedTuple
from unittest import TestCase
from unittest.mock import patch

import pytz

from runner.code_coverage.coverage_manager import CoverageManager
from runner.enum_types.params import TestEnv
from runner.options.cli_options import get_args
from runner.options.config import Config
from runner.options.options import IOptions
from runner.options.step import StepKind
from runner.suites.test_standard_flow import TestStandardFlow
from runner.suites.work_dir import WorkDir
from runner.test.test_utils import random_suffix


class KeyValueType(NamedTuple):
    key: str
    value: str | None


class OptionsTest(TestCase):
    current_folder: ClassVar[Path] = Path(__file__).parent
    data_folder: ClassVar[Path] = Path(__file__).parent / "data"

    @staticmethod
    def divide_param(param: str) -> KeyValueType:
        parts = param.split("=")
        if len(parts) == 1:
            return KeyValueType(parts[0], None)
        return KeyValueType(parts[0], parts[1])

    @staticmethod
    def compare_list(list1: list[str], list2: list[str]) -> Generator[bool, None, None]:
        for first, second in zip(list1, list2, strict=True):
            yield first.endswith(second)

    def prepare(self) -> tuple[TestEnv, Path, WorkDir]:
        args = get_args()
        config = Config(args)
        timestamp = int(datetime.timestamp(datetime.now(pytz.UTC)))
        work_root = self.current_folder / Path(os.environ["WORK_DIR"])
        work_dir = WorkDir(config, work_root)
        test_env = TestEnv(
            config=config,
            cmd_prefix=[],
            cmd_env={},
            timestamp=timestamp,
            report_formats={config.general.report_format},
            work_dir=work_dir,
            coverage=CoverageManager(Path().cwd(), work_dir.coverage_dir)
        )
        test_root = self.data_folder
        return test_env, test_root, work_dir

    def check_parameter(self, param: str, args: list[str], *, is_present: bool = True) -> None:
        checked = self.divide_param(param)
        is_found = False
        is_equal = False
        for arg in args:
            key, value = self.divide_param(arg)
            if checked.key == key and is_present:
                is_found = True
                if ((value and checked.value and value.endswith(checked.value)) or
                        (value is None and checked.value is None)):
                    is_equal = True
        presence = "present" if is_found else "absent"
        expectancy = "expected" if is_present else "not expected"
        self.assertEqual(is_found, is_present,
                         f"The property {param} is {presence} in args list {args}. "
                         f"But it's {expectancy}")
        if is_present:
            self.assertTrue(is_equal,
                            f"The property {checked.key} is found, "
                            f"but the value {checked.value} is absent in args list {args}")

    def check_boot_panda_files(self, checked: list[str], arg: str) -> None:
        parts = arg.split(":")
        self.assertEqual(len(parts), 1 + len(checked))
        self.assertTrue(all(self.compare_list(parts[1:], checked)))

    @patch('runner.utils.get_config_workflow_folder', lambda: OptionsTest.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: OptionsTest.data_folder)
    @patch('sys.argv', ["runner.sh", "panda", "test_suite1"])
    @patch.dict(os.environ, {
        'ARKCOMPILER_RUNTIME_CORE_PATH': ".",
        'ARKCOMPILER_ETS_FRONTEND_PATH': ".",
        'PANDA_BUILD': ".",
        'WORK_DIR': f"work-{random_suffix()}"
    }, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', lambda: "111111111")
    def test_workflow_panda(self) -> None:
        # preparation
        test_env, test_root, work_dir = self.prepare()
        # test
        test = TestStandardFlow(
            test_env=test_env,
            test_path=test_root / "test1.ets",
            params=IOptions({}),
            test_id="test1.ets"
        )
        self.assertTrue(test.is_panda)
        for step in test_env.config.workflow.steps:
            match step.step_kind:
                case StepKind.COMPILER:
                    new_step = test.prepare_compiler_step(step)
                    self.assertListEqual(step.args, new_step.args)
                case StepKind.VERIFIER:
                    new_step = test.prepare_verifier_step(step)
                    self.assertListEqual(step.args, new_step.args)
                case StepKind.AOT:
                    new_step = test.prepare_aot_step(step)
                    self.check_parameter("--paoc-panda-files=intermediate/test1.ets.abc", new_step.args)
                case StepKind.RUNTIME:
                    new_step = test.prepare_runtime_step(step)
                    self.check_parameter("--panda-files=intermediate/test1.ets.abc", new_step.args)

        # clear up
        shutil.rmtree(work_dir.root, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', lambda: OptionsTest.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: OptionsTest.data_folder)
    @patch('sys.argv', ["runner.sh", "panda", "test_suite1", "--is-panda", "False"])
    @patch.dict(os.environ, {
        'ARKCOMPILER_RUNTIME_CORE_PATH': ".",
        'ARKCOMPILER_ETS_FRONTEND_PATH': ".",
        'PANDA_BUILD': ".",
        'WORK_DIR': f"work-{random_suffix()}"
    }, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', lambda: "999999999")
    def test_workflow_not_panda(self) -> None:
        # preparation
        test_env, test_root, work_dir = self.prepare()
        # test
        test = TestStandardFlow(
            test_env=test_env,
            test_path=test_root / "test1.ets",
            params=IOptions({}),
            test_id="test1.ets"
        )
        self.assertFalse(test.is_panda)
        for step in test_env.config.workflow.steps:
            match step.step_kind:
                case StepKind.COMPILER:
                    new_step = test.prepare_compiler_step(step)
                    self.assertListEqual(step.args, new_step.args)
                case StepKind.VERIFIER:
                    new_step = test.prepare_verifier_step(step)
                    self.assertListEqual(step.args, new_step.args)
                case StepKind.AOT:
                    new_step = test.prepare_aot_step(step)
                    self.check_parameter("--paoc-panda-files=intermediate/test1.ets.abc", new_step.args,
                                         is_present=False)
                case StepKind.RUNTIME:
                    new_step = test.prepare_runtime_step(step)
                    self.check_parameter("--panda-files=intermediate/test1.ets.abc", new_step.args,
                                         is_present=False)

        # clear up
        shutil.rmtree(work_dir.root, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', lambda: OptionsTest.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: OptionsTest.data_folder)
    @patch('sys.argv', ["runner.sh", "panda", "test_suite1"])
    @patch.dict(os.environ, {
        'ARKCOMPILER_RUNTIME_CORE_PATH': ".",
        'ARKCOMPILER_ETS_FRONTEND_PATH': ".",
        'PANDA_BUILD': ".",
        'WORK_DIR': f"work-{random_suffix()}"
    }, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', lambda: "222222222")
    def test_workflow_panda_multiple(self) -> None:
        # preparation
        test_env, test_root, work_dir = self.prepare()
        # test
        test = TestStandardFlow(
            test_env=test_env,
            test_path=test_root / "test2.ets",
            params=IOptions({}),
            test_id="test2.ets"
        )

        dependent_tests = test.dependent_tests
        self.assertEqual(len(dependent_tests), 1)
        dependent_test = dependent_tests[0]
        self.assertEqual(dependent_test.test_id, "dependent_test2.ets")
        self.assertFalse(dependent_test.is_valid_test)
        self.assertEqual(dependent_test.parent_test_id, "test2.ets")
        self.assertEqual(dependent_test.test_abc.name, "test2_dependent_test2.ets.abc")
        self.assertEqual(dependent_test.test_an.name, "test2_dependent_test2.ets.an")

        self.assertTrue(test.is_panda)
        for step in test_env.config.workflow.steps:
            match step.step_kind:
                case StepKind.COMPILER:
                    new_step = test.prepare_compiler_step(step)
                    self.assertListEqual(step.args, new_step.args)
                case StepKind.VERIFIER:
                    new_step = test.prepare_verifier_step(step)
                    boot_panda_files = arg[0] \
                        if len(arg := [arg for arg in new_step.args if arg.startswith("--boot-panda-files=")]) > 0 \
                        else ""
                    self.check_boot_panda_files(
                        ['intermediate/test2_dependent_test2.ets.abc', 'intermediate/test2.ets.abc'],
                        boot_panda_files)
                case StepKind.AOT:
                    new_step = test.prepare_aot_step(step)
                    boot_panda_files = arg[0] \
                        if len(arg := [arg for arg in new_step.args if arg.startswith("--boot-panda-files=")]) > 0 \
                        else ""
                    self.check_boot_panda_files([], boot_panda_files)
                    self.check_parameter("--paoc-panda-files=intermediate/test2_dependent_test2.ets.abc", new_step.args)
                    self.check_parameter("--paoc-panda-files=intermediate/test2.ets.abc", new_step.args)
                case StepKind.RUNTIME:
                    new_step = test.prepare_runtime_step(step)
                    boot_panda_files = arg[0] \
                        if len(arg := [arg for arg in new_step.args if arg.startswith("--boot-panda-files=")]) > 0 \
                        else ""
                    self.check_boot_panda_files(
                        ['intermediate/test2_dependent_test2.ets.abc', 'intermediate/test2.ets.abc'],
                        boot_panda_files)

        # clear up
        shutil.rmtree(work_dir.root, ignore_errors=True)
