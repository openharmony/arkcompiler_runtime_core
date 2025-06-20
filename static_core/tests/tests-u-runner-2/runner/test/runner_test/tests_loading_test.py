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
from pathlib import Path
from typing import ClassVar
from unittest import TestCase
from unittest.mock import patch

from runner.options.cli_options import get_args
from runner.options.config import Config
from runner.suites.runner_standard_flow import RunnerStandardFlow
from runner.test.test_utils import random_suffix


class TestsLoadingTest(TestCase):
    current_folder: ClassVar[Path] = Path(__file__).parent
    data_folder: ClassVar[Path] = Path(__file__).parent / "data"
    test_environ: ClassVar[dict[str, str]] = {
        'ARKCOMPILER_RUNTIME_CORE_PATH': Path.cwd().as_posix(),
        'ARKCOMPILER_ETS_FRONTEND_PATH': Path.cwd().as_posix(),
        'PANDA_BUILD': Path.cwd().as_posix(),
        'WORK_DIR': (Path.cwd() / f"work-{random_suffix()}").as_posix(),
        'TEST_ROOT': data_folder.as_posix()
    }

    @staticmethod
    def cmake_cache() -> list[str]:
        return [
            'PANDA_ENABLE_UNDEFINED_BEHAVIOR_SANITIZER=false',
            'PANDA_ENABLE_ADDRESS_SANITIZER=true',
            'PANDA_ENABLE_THREAD_SANITIZER=false',
            'CMAKE_BUILD_TYPE=fastverify'
        ]

    @staticmethod
    def get_config() -> Config:
        args = get_args()
        return Config(args)

    @patch('runner.utils.get_config_workflow_folder', lambda: TestsLoadingTest.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: TestsLoadingTest.data_folder)
    @patch('sys.argv', ["runner.sh", "workflow", "test_suite"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_cache', cmake_cache)
    def test_default_run(self) -> None:
        """
        Expected: loaded all tests but test4 which excluded one
        Tests from ignored test list (test1 and test5) are loaded
        """
        # preparation
        config = self.get_config()
        # test
        runner = RunnerStandardFlow(config)
        actual_tests = sorted(test.test_id for test in runner.tests)
        expected_tests = [
            "test1.ets",
            "test2.ets",
            "test3.ets",
            "test5.ets",
        ]
        self.assertListEqual(actual_tests, expected_tests)
        # clear up
        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', lambda: TestsLoadingTest.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: TestsLoadingTest.data_folder)
    @patch('sys.argv', ["runner.sh", "workflow", "test_suite", "--skip-compile-only"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_cache', cmake_cache)
    def test_skip_compile_only(self) -> None:
        """
        Expected: loaded all tests but test4 which excluded one and test3 which is compile-only
        Tests from ignored test list (test1 and test5) are loaded
        """
        # preparation
        config = self.get_config()
        # test
        runner = RunnerStandardFlow(config)
        actual_tests = sorted(test.test_id for test in runner.tests)
        expected_tests = [
            "test1.ets",
            "test2.ets",
            "test5.ets",
        ]
        self.assertListEqual(actual_tests, expected_tests)
        # clear up
        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', lambda: TestsLoadingTest.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: TestsLoadingTest.data_folder)
    @patch('sys.argv', ["runner.sh", "workflow", "test_suite", "--exclude-ignored-tests"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_cache', cmake_cache)
    def test_exclude_ignored(self) -> None:
        """
        Expected: loaded only test2 (normal test) and test3 (compile-only test)
        Tests from ignored test list (test1 and test5) are excluded like test3 (from excluded list)
        """
        # preparation
        config = self.get_config()
        # test
        runner = RunnerStandardFlow(config)
        actual_tests = sorted(test.test_id for test in runner.tests)
        expected_tests = [
            "test2.ets",
            "test3.ets",
        ]
        self.assertListEqual(actual_tests, expected_tests)
        # clear up
        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', lambda: TestsLoadingTest.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: TestsLoadingTest.data_folder)
    @patch('sys.argv', ["runner.sh", "workflow", "test_suite", "--exclude-ignored-tests", "--skip-compile-only"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_cache', cmake_cache)
    def test_both_exclude_ignored_skip_compile_only(self) -> None:
        """
        Expected: loaded only test2
        Other tests are excluded:
        - test1 and test5 because they are ignored and the option --exclude-ignored-tests is set
        - test4 because it's excluded
        - test3 because it's compile-only
        """
        # preparation
        config = self.get_config()
        # test
        runner = RunnerStandardFlow(config)
        actual_tests = sorted(test.test_id for test in runner.tests)
        expected_tests = [
            "test2.ets",
        ]
        self.assertListEqual(actual_tests, expected_tests)
        # clear up
        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', lambda: TestsLoadingTest.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: TestsLoadingTest.data_folder)
    @patch('sys.argv', ["runner.sh", "workflow", "test_suite", "--skip-test-lists"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_cache', cmake_cache)
    def test_skip_test_lists(self) -> None:
        """
        Expected: loaded all tests, including test4 from excluded test list.
        """
        # preparation
        config = self.get_config()
        # test
        runner = RunnerStandardFlow(config)
        actual_tests = sorted(test.test_id for test in runner.tests)
        expected_tests = [
            "test1.ets",
            "test2.ets",
            "test3.ets",
            "test4.ets",
            "test5.ets",
        ]
        self.assertListEqual(actual_tests, expected_tests)
        # clear up
        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)
