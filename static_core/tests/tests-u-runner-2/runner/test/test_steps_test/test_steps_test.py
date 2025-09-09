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
from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path
from typing import ClassVar, cast
from unittest import TestCase
from unittest.mock import patch

from runner.options.cli_options import get_args
from runner.options.config import Config
from runner.suites.runner_standard_flow import RunnerStandardFlow
from runner.suites.test_standard_flow import TestStandardFlow
from runner.test import test_utils
from runner.test.test_utils import test_cmake_cache


@dataclass
class TestStep:
    name: str
    type: str
    args: list[str]


COMPILER_STEP = "compiler"
RUNTIME_STEP = "runtime"
INTERMEDIATE = "intermediate"


class TestStepsTest(TestCase):
    data_folder: ClassVar[Callable[[], Path]] = lambda: test_utils.data_folder(__file__)
    test_environ: ClassVar[dict[str, str]] = test_utils.test_environ(
        'TEST_STEPS_TEST_ROOT', data_folder().as_posix())
    get_instance_id: ClassVar[Callable[[], str]] = lambda: test_utils.create_runner_test_id(__file__)

    @staticmethod
    def prepare() -> list[TestStandardFlow]:
        args = get_args()
        runner = RunnerStandardFlow(Config(args))
        return [cast(TestStandardFlow, test.do_run()) for test in runner.tests]

    @staticmethod
    def get_steps(reproduce: str) -> list[TestStep]:
        lines_per_step = 4
        result: list[TestStep] = []
        lines = [line for line in reproduce.split('\n') if line]
        for i in range(len(lines) // lines_per_step):
            line = lines[i * lines_per_step]
            pos = line.find(":")
            step_name = line[0:pos]
            type_name = step_name.split('-')[-1]
            cli = line[pos + 1:].strip().split(' ')
            test_name = cli[-1]
            args = cli[1:]
            result.append(TestStep(name=test_name, args=args, type=type_name))
        return result

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('sys.argv', ["runner.sh", "workflow", "test_suite", "--test-file=imports_nat.ets"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_cache', test_cmake_cache)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    def test_imports_nat(self) -> None:
        """
        Feature: the main file 'imports_nat.ets' imports a file 'depends_nat.ets' which
        is marked as only 'not-a-test'
        Expected:
        - test 'imports_nat' should contain 3 steps - 2 compilation and 1 runtime
        - abc file of dependent test should contain main file name
        - runtime option boot-panda-file should contain both ets file in the order <dependent_file>:<main_file>
        """
        # preparation
        result = self.prepare()[0]
        # test
        steps = self.get_steps(result.reproduce)
        expected_types = [COMPILER_STEP, COMPILER_STEP, RUNTIME_STEP]
        expected_dependent_ets = "dependent_nat.ets"
        expected_main_ets = "imports_nat.ets"
        expected_dependent_ets_abc = f"{INTERMEDIATE}/imports_nat_dependent_nat.ets.abc"
        expected_main_ets_abc = f"{INTERMEDIATE}/imports_nat.ets.abc"

        self.assertEqual(len(steps), len(expected_types))
        self.check_steps(steps, expected_types)
        self.check_names(steps, [expected_dependent_ets, expected_main_ets, expected_main_ets])

        self.check_args(steps[0], "--output=", [expected_dependent_ets_abc])
        self.check_args(steps[1], "--output=", [expected_main_ets_abc])
        self.check_args(steps[2], "--output=", [expected_main_ets_abc])
        self.check_args(
            step=steps[2],
            option="--boot-panda-files=",
            expected_args=[expected_dependent_ets_abc, expected_main_ets_abc])

        # clear up
        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('sys.argv', ["runner.sh", "workflow", "test_suite", "--test-file=imports_nat_co1.ets"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_cache', test_cmake_cache)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    def test_imports_nat_co1(self) -> None:
        """
        Feature: the main file 'imports_nat_co1.ets' imports a file 'depends_nat_co.ets' which
        is marked as 'compile-only' and 'not-a-test'
        Expected:
        - test 'imports_nat_co' should contain 3 steps - 2 compilation and 1 runtime
        - abc file of dependent test should contain main file name
        - runtime option boot-panda-file should contain both ets file in the order <dependent_file>:<main_file>
        """
        # preparation
        result = self.prepare()[0]
        # test
        steps = self.get_steps(result.reproduce)

        expected_types = [COMPILER_STEP, COMPILER_STEP, RUNTIME_STEP]
        expected_dependent_ets = "dependent_nat_co.ets"
        expected_main_ets = "imports_nat_co1.ets"
        expected_dependent_ets_abc = f"{INTERMEDIATE}/imports_nat_co1_dependent_nat_co.ets.abc"
        expected_main_ets_abc = f"{INTERMEDIATE}/imports_nat_co1.ets.abc"

        self.assertEqual(len(steps), len(expected_types))
        self.check_steps(steps, expected_types)
        self.check_names(steps, [expected_dependent_ets, expected_main_ets, expected_main_ets])

        self.check_args(steps[0], "--output=", [expected_dependent_ets_abc])
        self.check_args(steps[1], "--output=", [expected_main_ets_abc])
        self.check_args(steps[2], "--output=", [expected_main_ets_abc])
        self.check_args(
            step=steps[2],
            option="--boot-panda-files=",
            expected_args=[expected_dependent_ets_abc, expected_main_ets_abc])

        # clear up
        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('sys.argv', ["runner.sh", "workflow", "test_suite", "--test-file=imports_nat_co2.ets"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_cache', test_cmake_cache)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    def test_imports_nat_co2(self) -> None:
        """
        Feature: the main file 'imports_nat_co2.ets' imports a file 'imports_nat_co1.ets' which
        is not marked as 'not-a-test' and which imports another file
        Expected:
        - test 'imports_nat_co' should contain 4 steps - 3 compilation and 1 runtime
        - abc file of dependent tests should contain main file name
        - runtime option boot-panda-file should contain all ets.abc file
          in the order <dependent_file2>:<dependent_file1>:<main_file>
        """
        # preparation
        result = self.prepare()[0]
        # test
        steps = self.get_steps(result.reproduce)

        expected_types = [COMPILER_STEP, COMPILER_STEP, COMPILER_STEP, RUNTIME_STEP]
        expected_dependent2_ets = "dependent_nat_co.ets"
        expected_dependent1_ets = "imports_nat_co1.ets"
        expected_main_ets = "imports_nat_co2.ets"
        expected_dependent2_ets_abc = f"{INTERMEDIATE}/imports_nat_co2_dependent_nat_co.ets.abc"
        expected_dependent1_ets_abc = f"{INTERMEDIATE}/imports_nat_co2_imports_nat_co1.ets.abc"
        expected_main_ets_abc = f"{INTERMEDIATE}/imports_nat_co2.ets.abc"

        self.assertEqual(len(steps), len(expected_types))
        self.check_steps(steps, expected_types)
        self.check_names(steps, [expected_dependent2_ets, expected_dependent1_ets,
                                 expected_main_ets, expected_main_ets])

        self.check_args(steps[0], "--output=", [expected_dependent2_ets_abc])
        self.check_args(steps[1], "--output=", [expected_dependent1_ets_abc])
        self.check_args(steps[2], "--output=", [expected_main_ets_abc])
        self.check_args(steps[3], "--output=", [expected_main_ets_abc])
        self.check_args(
            step=steps[3],
            option="--boot-panda-files=",
            expected_args=[expected_dependent2_ets_abc, expected_dependent1_ets_abc, expected_main_ets_abc])

        # clear up
        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('sys.argv', ["runner.sh", "workflow", "test_suite", "--test-file=simple1.ets"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_cache', test_cmake_cache)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    def test_simple1(self) -> None:
        """
        Feature: the main file 'simple1.ets' has return type 'void, doesn't import anything,
        doesn't have a markup
        Expected:
        - test 'simple1.ets' should contain 2 steps - 1 compilation and 1 runtime
        - runtime option boot-panda-file should contain only main ets.abc file
        """
        # preparation
        result = self.prepare()[0]
        # test
        steps = self.get_steps(result.reproduce)
        expected_types = [COMPILER_STEP, RUNTIME_STEP]
        expected_main_ets = "simple1.ets"
        expected_main_ets_abc = f"{INTERMEDIATE}/{expected_main_ets}.abc"

        self.assertEqual(len(steps), len(expected_types))
        self.check_steps(steps, expected_types)
        self.check_names(steps, [expected_main_ets, expected_main_ets])

        self.check_args(steps[0], "--output=", [expected_main_ets_abc])
        self.check_args(steps[1], "--output=", [expected_main_ets_abc])
        self.check_args(
            step=steps[1],
            option="--panda-files=",
            expected_args=[expected_main_ets_abc])

        # clear up
        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('sys.argv', ["runner.sh", "workflow", "test_suite", "--test-file=simple_co.ets"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_cache', test_cmake_cache)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    def test_simple_co(self) -> None:
        """
        Feature: the main file 'simple_co.ets' doesn't import anything, has a markup 'compile-only'
        Expected:
        - test 'simple1.ets' should contain 1 step - compilation
        """
        # preparation
        result = self.prepare()[0]
        # test
        steps = self.get_steps(result.reproduce)
        expected_types = [COMPILER_STEP]
        expected_main_ets = "simple_co.ets"
        expected_main_ets_abc = f"{INTERMEDIATE}/{expected_main_ets}.abc"

        self.assertEqual(len(steps), len(expected_types))
        self.check_steps(steps, expected_types)
        self.check_names(steps, [expected_main_ets])

        self.check_args(steps[0], "--output=", [expected_main_ets_abc])

        # clear up
        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('sys.argv', ["runner.sh", "workflow", "test_suite", "--test-file=simple_co_neg.ets"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_cache', test_cmake_cache)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    def test_simple_co_neg(self) -> None:
        """
        Feature: the main file 'simple_co_neg.ets' doesn't import anything,
        has a markup 'compile-only' and 'negative'
        Expected:
        - test should contain 1 step - compilation
        """
        # preparation
        result = self.prepare()[0]
        # test
        steps = self.get_steps(result.reproduce)
        expected_types = [COMPILER_STEP]
        expected_main_ets = "simple_co_neg.ets"
        expected_main_ets_abc = f"{INTERMEDIATE}/{expected_main_ets}.abc"

        self.assertEqual(len(steps), len(expected_types))
        self.check_steps(steps, expected_types)
        self.check_names(steps, [expected_main_ets])

        self.check_args(steps[0], "--output=", [expected_main_ets_abc])

        # clear up
        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('sys.argv', ["runner.sh", "workflow", "test_suite", "--test-file=simple_nat.ets"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_cache', test_cmake_cache)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    def test_simple_nat(self) -> None:
        """
        Feature: the main file 'simple_nat.ets' doesn't import anything,
        has a markup 'not-a-test'
        Expected:
        - test should not be executed at all
        """
        # preparation
        result = self.prepare()
        # test
        expected_types: list[str] = []

        self.assertEqual(len(result), len(expected_types))

        # clear up
        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    def check_steps(self, steps: list[TestStep], expected_types: list[str]) -> None:
        actual_types = [step.type for step in steps]
        self.assertListEqual(actual_types, expected_types)

    def check_names(self, steps: list[TestStep], expected_names: list[str]) -> None:
        actual_names = [step.name for step in steps]
        self.assertListEqual(actual_names, expected_names)

    def check_args(self, step: TestStep, option: str, expected_args: list[str]) -> None:
        for arg in step.args:
            if arg.startswith(option):
                pos = arg.find("=")
                arg_value = arg[pos + 1:].strip()
                values = [value[value.find(INTERMEDIATE):] for value in arg_value.split(":") if INTERMEDIATE in value]
                self.assertListEqual(values, expected_args)
