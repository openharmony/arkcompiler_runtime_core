#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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

import os
import shutil
from collections.abc import Callable
from pathlib import Path
from typing import ClassVar, cast
from unittest import TestCase
from unittest.mock import MagicMock, patch

import yaml

from runner.common_exceptions import MalformedStepConfigurationException
from runner.options.cli_options import get_args
from runner.options.config import Config
from runner.options.options_step import RawStepData, Step
from runner.options.step_reqs import ReqKind, StepRequirements
from runner.suites.runner_standard_flow import RunnerStandardFlow
from runner.suites.test_standard_flow import TestStandardFlow
from runner.test import test_utils
from runner.test.test_utils import compare_dicts, compare_lists, test_cmake_build


class TestStepsTest2(TestCase):
    data_folder: ClassVar[Callable[[], Path]] = lambda: test_utils.data_folder(__file__)
    test_environ: ClassVar[dict[str, str]] = test_utils.test_environ(
        'TEST_STEPS_TEST_ROOT', data_folder().as_posix())
    get_instance_id: ClassVar[Callable[[], str]] = lambda: test_utils.create_runner_test_id(__file__)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch.object(StepRequirements, 'check')
    @patch('sys.argv', ["runner.sh", "workflow_step_pre_reqs", "test_suite", "--test-file=inner/simple.ets"])
    def test_pre_reqs(self, check_mock: MagicMock) -> None:
        """
        Feature: the step contain 3 requirements
        Expected:
        - all 3 requirements should be completed
        """
        # preparation
        check_mock.return_value = True, ""
        args = get_args()
        runner = RunnerStandardFlow(Config(args))
        no_req_step = runner.steps[0]
        try:
            self.assertEqual(len(no_req_step.pre_requirements), 0,
                             "Step 'echo-compiler' should not contain any requirements")
            req_step = runner.steps[1]
            self.assertEqual(len(req_step.pre_requirements), 3,
                             "Step 'echo-runtime' should contain 3 requirements")
            actual_reqs = {req.req.value for req in req_step.pre_requirements}
            exp_reqs = set(ReqKind.values())
            self.assertSetEqual(actual_reqs, exp_reqs)

            # test
            with patch('runner.options.step_reqs.StepRequirements.check', wraps=StepRequirements.check) as mock_check:
                cast(TestStandardFlow, next(iter(runner.tests))).do_run()
                self.assertEqual(mock_check.call_count, 3)
        finally:
            # clear up
            work_dir = Path(os.environ["WORK_DIR"])
            shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch('sys.argv', ["runner.sh", "workflow_step_pre_reqs_not_exist", "test_suite"])
    def test_pre_reqs_not_exist(self) -> None:
        """
        Feature: workflow file uses not supported step pre-requirement
        Expected:
        - exception InvalidConfiguration
        """
        try:
            # preparation
            args = get_args()
            with self.assertRaises(MalformedStepConfigurationException):
                RunnerStandardFlow(Config(args))
        finally:
            # clear up
            work_dir = Path(os.environ["WORK_DIR"])
            shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch('sys.argv', ["runner.sh", "workflow_step_post_reqs_not_exist", "test_suite"])
    def test_post_reqs_not_exist(self) -> None:
        """
        Feature: workflow file uses not supported step post-requirement
        Expected:
        - exception InvalidConfiguration
        """
        try:
            # preparation
            args = get_args()
            with self.assertRaises(MalformedStepConfigurationException):
                RunnerStandardFlow(Config(args))
        finally:
            # clear up
            work_dir = Path(os.environ["WORK_DIR"])
            shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch('sys.argv', ["runner.sh", "workflow_step_reqs_empty", "test_suite"])
    def test_reqs_empty(self) -> None:
        """
        Feature: workflow file specifies empty key `pre-reqs` and `post-reqs`
        Expected:
        - empty requirements lists
        """
        try:
            # preparation
            args = get_args()
            runner = RunnerStandardFlow(Config(args))
            no_req_step = runner.steps[0]
            self.assertEqual(len(no_req_step.pre_requirements), 0,
                             "Step 'echo-compiler' should not contain any pre-requirements")
            self.assertEqual(len(no_req_step.post_requirements), 0,
                             "Step 'echo-compiler' should not contain any post-requirements")
        finally:
            # clear up
            work_dir = Path(os.environ["WORK_DIR"])
            shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @test_utils.parametrized_test_cases([
        (["runner.sh", "workflow_step_pre_reqs_failed_file", "test_suite", "--test-file=simple1.ets"],),
        (["runner.sh", "workflow_step_post_reqs_failed_file", "test_suite", "--test-file=simple1.ets"],),
    ])
    def test_reqs_failed_file(self, argv: Callable) -> None:
        """
        Feature: requirement FileExist refers to not-existing file
        Expected:
        - test failed and report.error contains message about failed requirement
        """
        with patch('sys.argv', argv):
            # preparation
            args = get_args()
            runner = RunnerStandardFlow(Config(args))
            result = cast(TestStandardFlow, next(iter(runner.tests))).do_run()
            report_error = result.report.error if result.report is not None else ""
            try:
                self.assertFalse(result.passed, "Test with failed pre/post requirement check 'FileExist' should fail")
                self.assertIn("FILE_EXIST", report_error)
            finally:
                # clear up
                work_dir = Path(os.environ["WORK_DIR"])
                shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @test_utils.parametrized_test_cases([
        (["runner.sh", "workflow_step_pre_reqs_failed_folder", "test_suite", "--test-file=simple1.ets"],),
        (["runner.sh", "workflow_step_post_reqs_failed_folder", "test_suite", "--test-file=simple1.ets"],),
    ])
    def test_reqs_failed_folder(self, argv: Callable) -> None:
        """
        Feature: requirement FolderExist refers to not-existing folder
        Expected:
        - test failed and report.error contains message about failed requirement
        """
        with patch('sys.argv', argv):
            # preparation
            args = get_args()
            runner = RunnerStandardFlow(Config(args))
            result = cast(TestStandardFlow, next(iter(runner.tests))).do_run()
            report_error = result.report.error if result.report is not None else ""
            try:
                self.assertFalse(result.passed, "Test with failed requirement check 'FolderExist' should fail")
                self.assertIn("FOLDER_EXIST", report_error)
            finally:
                # clear up
                work_dir = Path(os.environ["WORK_DIR"])
                shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @test_utils.parametrized_test_cases([
        (["runner.sh", "workflow_step_out", "test_suite", "--test-file=simple1.ets"],),
        (["runner.sh", "workflow_step_err", "test_suite", "--test-file=simple1.ets"],),
    ])
    def test_stdout_stderr(self, argv: Callable) -> None:
        """
        Feature: step with stdout and stderr fields. As well pre-req ParentExistCreate and
        post-reqs FileExist and FolderExist are checked here.
        Expected:
        - test passed and a file with stdout content should be created at the specified path
        """
        with patch('sys.argv', argv):
            # preparation
            args = get_args()
            runner = RunnerStandardFlow(Config(args))
            result = cast(TestStandardFlow, next(iter(runner.tests))).do_run()
            expected_rep_path = result.test_env.work_dir.intermediate / "report"
            expected_out_path = expected_rep_path / "simple1.ets-stdout.txt"
            expected_err_path = expected_rep_path / "simple1.ets-stderr.txt"
            try:
                self.assertTrue(expected_rep_path.exists(), f"Folder '{expected_rep_path}' not found")
                self.assertTrue(expected_out_path.exists(), f"File with stdout '{expected_out_path}' not found")
                self.assertTrue(expected_err_path.exists(), f"File with stderr '{expected_err_path}' not found")
                report_output = result.report.output if result.report is not None else ""
                expected_output = expected_out_path.read_text(encoding="utf-8")
                self.assertEqual(expected_output, report_output)
                report_error = result.report.error if result.report is not None else ""
                expected_error = expected_err_path.read_text(encoding="utf-8")
                self.assertEqual(expected_error, report_error)
            finally:
                # clear up
                work_dir = Path(os.environ["WORK_DIR"])
                shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch('sys.argv', ["runner.sh", "workflow_step_validator", "test_suite", "--test-file=simple1.ets"])
    def test_validator(self) -> None:
        """
        Feature: step with custom validator
        Expected:
        - test passed
        """
        try:
            # preparation
            args = get_args()
            runner = RunnerStandardFlow(Config(args))
            result = cast(TestStandardFlow, next(iter(runner.tests))).do_run()
            self.assertTrue(result.passed, "Test is expected to pass")

        finally:
            # clear up
            work_dir = Path(os.environ["WORK_DIR"])
            shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch('sys.argv', ["runner.sh", "workflow_step_post_reqs_not_allowed", "test_suite", "--test-file=simple1.ets"])
    def test_post_not_allowed(self) -> None:
        """
        Feature: step with not allowed requirement
        Expected:
        - test failed
        """
        # preparation
        args = get_args()
        try:
            with self.assertRaises(MalformedStepConfigurationException):
                RunnerStandardFlow(Config(args))
        finally:
            # clear up
            work_dir = Path(os.environ["WORK_DIR"])
            shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('runner.suites.test_lists.TestLists.cmake_build_properties', test_cmake_build)
    @patch('runner.suites.test_lists.TestLists.gn_build_properties', test_utils.test_gn_build)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch('sys.argv', ["runner.sh", "workflow_step_full", "test_suite", "--test-file=simple1.ets"])
    def test_str_full(self) -> None:
        """
        Feature: step with all values explicitly specified
        Expected:
        - test passed
        """
        try:
            # preparation
            args = get_args()
            runner = RunnerStandardFlow(Config(args))
            full_step = runner.steps[0]
            step_str = str(full_step)
            step_dict = cast(dict, yaml.safe_load(step_str))
            step_name = next(iter(step_dict))
            step_body = step_dict.get(step_name)
            self.assertIsNotNone(step_body)
            restored_step = Step(step_name, cast(RawStepData, step_body))
            self.assertEqual(restored_step.executable_path, full_step.executable_path)
            self.assertEqual(restored_step.timeout, full_step.timeout)
            self.assertEqual(restored_step.can_be_instrumented, full_step.can_be_instrumented)
            self.assertEqual(restored_step.enabled, full_step.enabled)
            self.assertEqual(restored_step.step_kind, full_step.step_kind)
            self.assertEqual(restored_step.skip_qemu, full_step.skip_qemu)
            self.assertEqual(restored_step.step_filter, full_step.step_filter)
            self.assertEqual(restored_step.validator_class, full_step.validator_class)
            self.assertEqual(restored_step.stdout, full_step.stdout)
            self.assertEqual(restored_step.stderr, full_step.stderr)
            # check args
            compare_lists(self, restored_step.args, full_step.args)
            # check env
            compare_dicts(self, restored_step.env, full_step.env)
            # check reqs
            actual_pre_reqs = [str(req) for req in restored_step.pre_requirements]
            expected_pre_reqs = [str(req) for req in full_step.pre_requirements]
            compare_lists(self, actual_pre_reqs, expected_pre_reqs)
            actual_post_reqs = [str(req) for req in restored_step.post_requirements]
            expected_post_reqs = [str(req) for req in full_step.post_requirements]
            compare_lists(self, actual_post_reqs, expected_post_reqs)

        finally:
            # clear up
            work_dir = Path(os.environ["WORK_DIR"])
            shutil.rmtree(work_dir, ignore_errors=True)
