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
from collections.abc import Callable
from pathlib import Path
from typing import ClassVar
from unittest import TestCase
from unittest.mock import MagicMock, PropertyMock, patch

from runner.options.cli_options import get_args
from runner.options.config import Config
from runner.options.root_dir import RootDir
from runner.suites.runner_standard_flow import RunnerStandardFlow
from runner.suites.test_suite import TestSuite
from runner.test import test_utils
from runner.test.test_utils import create_runner_test_id


class CollectionsTest(TestCase):
    current_folder: ClassVar[Path] = Path(__file__).parent
    data_folder: ClassVar[Path] = Path(__file__).parent / "data_with_collections"
    get_instance_id: ClassVar[Callable[[], str]] = lambda: create_runner_test_id(__file__)
    test_environ: ClassVar[dict[str, str]] = test_utils.test_environ(
        'TESTS_LOADING_TEST_ROOT', (data_folder / "tests").as_posix())

    @staticmethod
    def get_config() -> Config:
        args = get_args()
        return Config(args)

    @patch('runner.utils.get_config_workflow_folder', lambda: CollectionsTest.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: CollectionsTest.data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch('runner.suites.test_lists.TestLists._cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists._gn_build_properties', test_utils.test_gn_build)
    @test_utils.parametrized_test_cases([
        (["runner.sh", "panda", "test_suite_col1"],),
        (["runner.sh", "panda", "test_suite_col1", "--gn-build"],),
    ])
    def test_load_collection_of_one_file(self, argv: Callable) -> None:
        with patch('sys.argv', argv):

            config = CollectionsTest.get_config()
            runner = RunnerStandardFlow(config)
            actual_tests = sorted(test.test_id for test in runner.tests)
            expected_tests = ["test.ets"]
            try:
                self.assertEqual(actual_tests, expected_tests)
            finally:
                test_utils.clear_after_test()

    @patch('runner.utils.get_config_workflow_folder', lambda: CollectionsTest.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: CollectionsTest.data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch('runner.suites.test_lists.TestLists._cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists._gn_build_properties', test_utils.test_gn_build)
    @test_utils.parametrized_test_cases([
        (["runner.sh", "panda", "test_suite_col_folder"],),
        (["runner.sh", "panda", "test_suite_col_folder", "--gn-build"],),
    ])
    def test_load_collection_folder(self, argv: Callable) -> None:
        with patch('sys.argv', argv):
            config = CollectionsTest.get_config()
            runner = RunnerStandardFlow(config)
            actual_tests = sorted(test.test_id for test in runner.tests)
            expected_tests = ["tests_folder/test1.ets", "tests_folder/test2.ets"]
            try:
                self.assertEqual(actual_tests, expected_tests)
            finally:
                test_utils.clear_after_test()

    @patch('runner.utils.get_config_workflow_folder', lambda: CollectionsTest.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: CollectionsTest.data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch('runner.suites.test_lists.TestLists._cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists._gn_build_properties', test_utils.test_gn_build)
    @test_utils.parametrized_test_cases([
        (["runner.sh", "panda", "test_suite_col_several"],),
        (["runner.sh", "panda", "test_suite_col_several", "--gn-build"],),
    ])
    def test_load_collection_both(self, argv: Callable) -> None:
        with patch('sys.argv', argv):
            config = CollectionsTest.get_config()
            runner = RunnerStandardFlow(config)
            actual_tests = sorted(test.test_id for test in runner.tests)
            expected_tests = ["test.ets", "tests_folder/test1.ets", "tests_folder/test2.ets"]
            try:
                self.assertEqual(actual_tests, expected_tests)
            finally:
                test_utils.clear_after_test()

    @patch('runner.utils.get_config_workflow_folder', lambda: CollectionsTest.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: CollectionsTest.data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch('runner.suites.test_lists.TestLists._cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists._gn_build_properties', test_utils.test_gn_build)
    @test_utils.parametrized_test_cases([
        (["runner.sh", "panda", "test_suite_col_several",
          "--chapters-file", "chapters.yaml",
          "--chapters", "ch1"],)])
    def test_load_collection_folder_with_chapter(self, argv: Callable) -> None:
        """
        test-suite test_suite_col_several contains 2 collections that are filtered by chapters
        expected result: loaded tests that are listed in folder collection and chapter ch1
        """
        with patch('sys.argv', argv):
            with patch.object(TestSuite, 'list_roots', new_callable=PropertyMock) as mock_list_roots:
                mock_list_roots.return_value = [
                    RootDir(CollectionsTest.data_folder, None, MagicMock())
                ]
                config = CollectionsTest.get_config()
                runner = RunnerStandardFlow(config)
                actual_tests = sorted(test.test_id for test in runner.tests)
                expected_tests = ["tests_folder/test1.ets", "tests_folder/test2.ets"]
                try:
                    self.assertEqual(actual_tests, expected_tests)
                finally:
                    test_utils.clear_after_test()

    @patch('runner.utils.get_config_workflow_folder', lambda: CollectionsTest.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: CollectionsTest.data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch('runner.suites.test_lists.TestLists._cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists._gn_build_properties', test_utils.test_gn_build)
    @test_utils.parametrized_test_cases([
        (["runner.sh", "panda", "test_suite_col1",
          "--chapters-file", "chapters.yaml",
          "--chapters", "ch1"],)])
    def test_load_collection_test_with_chapter(self, argv: Callable) -> None:
        """
        test-suite test_suite_col1 contains 1 collection with 1 test not listed in chapters file
        collections are filtered by chapters
        expected result: []
        """
        with patch('sys.argv', argv):
            with patch.object(TestSuite, 'list_roots', new_callable=PropertyMock) as mock_list_roots:
                mock_list_roots.return_value = [
                    RootDir(CollectionsTest.data_folder, None, MagicMock())
                ]

                config = CollectionsTest.get_config()
                runner = RunnerStandardFlow(config)
                actual_tests = sorted(test.test_id for test in runner.tests)
                expected_tests: list[str | None] = []
                try:
                    self.assertEqual(actual_tests, expected_tests)
                finally:
                    test_utils.clear_after_test()

    @patch('runner.utils.get_config_workflow_folder', lambda: CollectionsTest.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: CollectionsTest.data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('sys.argv', ["runner.sh", "panda", "test_suite_col_several",
          "--filter", "*/test*.ets"])
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch('runner.suites.test_lists.TestLists._cmake_build_properties', test_utils.test_cmake_build)
    @patch('runner.suites.test_lists.TestLists._gn_build_properties', test_utils.test_gn_build)
    def test_load_collection_with_filter(self) -> None:
        """
        test-suite test_suite_col_several contains 2 collections
        expected result: loaded test1.ets, test2.ets
        """
        with patch.object(TestSuite, 'list_roots', new_callable=PropertyMock) as mock_list_roots:
            mock_list_roots.return_value = [
                RootDir(CollectionsTest.data_folder, None, MagicMock())
            ]
            config = CollectionsTest.get_config()
            runner = RunnerStandardFlow(config)
            actual_tests = sorted(test.test_id for test in runner.tests)
            expected_tests = ["tests_folder/test1.ets", "tests_folder/test2.ets"]
            try:
                self.assertEqual(actual_tests, expected_tests)
            finally:
                test_utils.clear_after_test()
