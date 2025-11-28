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
from pathlib import Path
from typing import ClassVar, cast
from unittest import TestCase
from unittest.mock import MagicMock, patch

from runner.options.cli_options import get_args
from runner.options.config import Config
from runner.suites.runner_standard_flow import RunnerStandardFlow
from runner.suites.test_standard_flow import TestStandardFlow
from runner.test import test_utils


class FailKindTest(TestCase):
    COMPILER_STEP_NAME = "ECHO-COMPILER"
    get_instance_id: ClassVar[Callable[[], str]] = lambda: test_utils.create_runner_test_id(__file__)
    data_folder: ClassVar[Callable[[], Path]] = lambda: test_utils.data_folder(__file__, "test_data")
    test_environ: ClassVar[dict[str, str]] = test_utils.test_environ(
        'TESTS_LOADING_TEST_ROOT', data_folder().as_posix())

    @staticmethod
    def get_config() -> Config:
        args = get_args()
        return Config(args)

    @staticmethod
    def set_process_mock(mock_popen: MagicMock, return_code: int, output: str = "", error_out: str = "") -> None:
        proc = MagicMock()
        proc.__enter__.return_value = proc
        proc.communicate.return_value = (output, error_out)
        proc.returncode = return_code
        mock_popen.return_value = proc

    @staticmethod
    def run_test() -> TestStandardFlow:
        config = FailKindTest.get_config()
        runner = RunnerStandardFlow(config)
        actual_test = next(iter(runner.tests))
        test_result = actual_test.do_run()
        return cast(TestStandardFlow, test_result)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('sys.argv', ["runner.sh", "panda", "test_suite1", "--test-file", "test4.ets"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_cache', test_utils.test_cmake_cache)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch("runner.suites.one_test_runner.subprocess.Popen")
    def test_fail_kind_passed(self, mock_popen: MagicMock) -> None:
        self.set_process_mock(mock_popen, return_code=0)

        test_result = self.run_test()

        self.assertTrue(test_result.passed)
        self.assertEqual(test_result.fail_kind, f"{FailKindTest.COMPILER_STEP_NAME}_PASSED")

        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('sys.argv', ["runner.sh", "panda", "test_suite1", "--test-file", "test4.ets"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_cache', test_utils.test_cmake_cache)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch("runner.suites.one_test_runner.subprocess.Popen")
    def test_fail_kind_fail(self, mock_popen: MagicMock) -> None:
        self.set_process_mock(mock_popen, return_code=1)

        test_result = self.run_test()

        self.assertFalse(test_result.passed)
        self.assertEqual(test_result.fail_kind, f"{FailKindTest.COMPILER_STEP_NAME}_FAIL")

        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('sys.argv', ["runner.sh", "panda", "test_suite1", "--test-file", "test5_compile_only_neg.ets"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_cache', test_utils.test_cmake_cache)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch("runner.suites.one_test_runner.subprocess.Popen")
    def test_fail_kind_neg_fail(self, mock_popen: MagicMock) -> None:
        self.set_process_mock(mock_popen, return_code=0)

        test_result = self.run_test()

        self.assertFalse(test_result.passed)
        self.assertEqual(test_result.fail_kind, f"{FailKindTest.COMPILER_STEP_NAME}_NEG_FAIL")

        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('sys.argv', ["runner.sh", "panda", "test_suite1", "--test-file", "test4.ets"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_cache', test_utils.test_cmake_cache)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch("runner.suites.one_test_runner.subprocess.Popen")
    def test_fail_kind_segfault(self, mock_popen: MagicMock) -> None:
        self.set_process_mock(mock_popen, return_code=139)

        test_result = self.run_test()

        self.assertFalse(test_result.passed)
        self.assertEqual(test_result.fail_kind, f"{FailKindTest.COMPILER_STEP_NAME}_SEGFAULT_FAIL")

        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('sys.argv', ["runner.sh", "panda", "test_suite1", "--test-file", "test4.ets"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_cache', test_utils.test_cmake_cache)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch("runner.suites.one_test_runner.subprocess.Popen")
    def test_fail_kind_abort_fail(self, mock_popen: MagicMock) -> None:
        self.set_process_mock(mock_popen, return_code=-6)

        test_result = self.run_test()

        self.assertFalse(test_result.passed)
        self.assertEqual(test_result.fail_kind, f"{FailKindTest.COMPILER_STEP_NAME}_ABORT_FAIL")

        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('sys.argv', ["runner.sh", "panda", "test_suite1", "--test-file", "test4.ets"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_cache', test_utils.test_cmake_cache)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch("runner.suites.one_test_runner.subprocess.Popen")
    def test_fail_kind_irtoc_fail(self, mock_popen: MagicMock) -> None:
        self.set_process_mock(mock_popen, return_code=133)

        test_result = self.run_test()

        self.assertFalse(test_result.passed)
        self.assertEqual(test_result.fail_kind, f"{FailKindTest.COMPILER_STEP_NAME}_IRTOC_ASSERT_FAIL")

        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('sys.argv', ["runner.sh", "panda", "test_suite1", "--test-file", "test1.console.ets"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_cache', test_utils.test_cmake_cache)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch("runner.suites.one_test_runner.subprocess.Popen")
    def test_fail_kind_expected_fail(self, mock_popen: MagicMock) -> None:
        self.set_process_mock(mock_popen, return_code=0, output="default: 5")
        test_result = self.run_test()

        self.assertFalse(test_result.passed)
        self.assertEqual(test_result.fail_kind, f"{FailKindTest.COMPILER_STEP_NAME}_COMPARE_OUTPUT")

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('sys.argv', ["runner.sh", "panda", "test_suite1", "--test-file", "test1.console.ets"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_cache', test_utils.test_cmake_cache)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch("runner.suites.one_test_runner.subprocess.Popen")
    def test_fail_kind_expected_output_ok(self, mock_popen: MagicMock) -> None:
        self.set_process_mock(mock_popen, return_code=0, output="default: 1\ndefault: 2\ndefault: 3\n"
                                                                "userClick: 1\nuserClick: 2\nuserClick: 3")
        test_result = self.run_test()

        self.assertTrue(test_result.passed)
        self.assertEqual(test_result.fail_kind, f"{FailKindTest.COMPILER_STEP_NAME}_PASSED")

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('sys.argv', ["runner.sh", "panda", "test_suite1", "--test-file", "test1.ets"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_cache', test_utils.test_cmake_cache)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch("runner.suites.one_test_runner.subprocess.Popen")
    def test_fail_kind_expected_stderr_not_empty_fail(self, mock_popen: MagicMock) -> None:
        self.set_process_mock(mock_popen, return_code=0, error_out="default: 1")

        test_result = self.run_test()
        self.assertFalse(test_result.passed)
        self.assertEqual(test_result.fail_kind, f"{FailKindTest.COMPILER_STEP_NAME}_STDERR_NOT_EMPTY")

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('sys.argv', ["runner.sh", "panda", "test_suite1", "--test-file", "test1.console.ets"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_cache', test_utils.test_cmake_cache)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch("runner.suites.one_test_runner.subprocess.Popen")
    def test_fail_kind_expected_err_fail(self, mock_popen: MagicMock) -> None:
        self.set_process_mock(mock_popen, return_code=0, output="default: 1\ndefault: 2\ndefault: 3\n"
                                                                "userClick: 1\nuserClick: 2\nuserClick: 3",
                              error_out="error!")

        test_result = self.run_test()
        self.assertFalse(test_result.passed)
        self.assertEqual(test_result.fail_kind, f"{FailKindTest.COMPILER_STEP_NAME}_COMPARE_OUTPUT")

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('sys.argv', ["runner.sh", "panda", "test_suite1", "--test-file", "test1.console.ets"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.suites.test_lists.TestLists.cmake_cache', test_utils.test_cmake_cache)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch("runner.suites.one_test_runner.subprocess.Popen")
    def test_fail_kind_expected_err_fail_no_file(self, mock_popen: MagicMock) -> None:
        self.set_process_mock(mock_popen, return_code=0, error_out="default: 1\ndefault: 2\ndefault: 3\n"
                                                                "userClick: 1\nuserClick: 2\nuserClick: 3")

        test_result = self.run_test()
        self.assertFalse(test_result.passed)
        self.assertEqual(test_result.fail_kind, f"{FailKindTest.COMPILER_STEP_NAME}_COMPARE_OUTPUT")
