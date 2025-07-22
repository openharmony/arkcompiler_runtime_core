#!/usr/bin/env python3
# -- coding: utf-8 --
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
import unittest
from pathlib import Path

from runner.common_exceptions import InvalidConfiguration
from runner.options import cli_options_utils as cli_utils
from runner.options.cli_options import CliOptionsParser, CliParserBuilder, ConfigsLoader
from runner.test.config_test import data_1, data_2
from runner.test.test_utils import compare_dicts


class TestSuiteConfigTest1(unittest.TestCase):
    cfg_ext = ".yaml"
    workflow_name = "config-1"
    workflow_path: Path
    test_suite_name = "test_suite1"
    test_suite_path: Path
    current_path = Path(__file__).parent
    cfg_path = current_path.parent.parent.parent.joinpath("cfg")

    @classmethod
    def setUpClass(cls) -> None:
        os.environ["ARKCOMPILER_RUNTIME_CORE_PATH"] = "."
        os.environ["ARKCOMPILER_ETS_FRONTEND_PATH"] = "."
        os.environ["WORK_DIR"] = "."
        os.environ["PANDA_BUILD"] = "."

        shutil.copy(cls.current_path.joinpath(cls.workflow_name + cls.cfg_ext), cls.cfg_path.joinpath("workflows"))
        cls.workflow_path = cls.cfg_path.joinpath("workflows").joinpath(cls.workflow_name + cls.cfg_ext)

        shutil.copy(cls.current_path.joinpath(cls.test_suite_name + cls.cfg_ext), cls.cfg_path.joinpath("test-suites"))
        cls.test_suite_path = cls.cfg_path.joinpath("test-suites").joinpath(cls.test_suite_name + cls.cfg_ext)

    @classmethod
    def tearDownClass(cls) -> None:
        cls.workflow_path.unlink(missing_ok=True)
        cls.test_suite_path.unlink(missing_ok=True)

    def test_min_args(self) -> None:

        configs = ConfigsLoader(self.workflow_name, self.test_suite_name)

        parser_builder = CliParserBuilder(configs)
        test_suite_parser, key_lists_ts = parser_builder.create_parser_for_test_suite()
        workflow_parser, key_lists_wf = parser_builder.create_parser_for_workflow()

        cli = CliOptionsParser(configs, parser_builder.create_parser_for_runner(),
                               test_suite_parser,
                               parser_builder.create_parser_for_default_test_suite(),
                               workflow_parser, *[])
        cli.parse_args()

        actual = cli.full_options
        actual = cli_utils.restore_duplicated_options(configs, actual)
        actual = cli_utils.add_config_info(configs, actual)
        actual = cli_utils.restore_default_list(actual, key_lists_ts | key_lists_wf)
        expected = data_1.args
        compare_dicts(self, actual, expected)

    def test_args_urunner(self) -> None:
        args = [
            "--show-progress",
            "--verbose", "short",
            "--processes", "12",
            "--detailed-report",
            "--detailed-report-file", "my-report",
            "--report-dir", "my-report-dir",
            "--verbose-filter", "ignored",
            "--enable-time-report",
            "--qemu", "arm64",
            "--report-dir", "my-report-dir",
            "--use-llvm-cov",
            "--use-lcov",
            "--llvm-cov-exclude", "/tmp",
            "--llvm-cov-exclude", "*.h",
            "--lcov-exclude", "/tmp",
            "--lcov-exclude", "*.h",
            "--profdata-files-dir", ".", 
            "--coverage-html-report-dir", ".",
            "--coverage-per-binary",
            "--clean-gcda-before-run",
            "--time-edges", "1,10,100,500",
            "--gn-build",
            "--gcov-tool", "/usr/bin/ls"]

        configs = ConfigsLoader(self.workflow_name, self.test_suite_name)

        parser_builder = CliParserBuilder(configs)
        test_suite_parser, key_lists_ts = parser_builder.create_parser_for_test_suite()
        workflow_parser, key_lists_wf = parser_builder.create_parser_for_workflow()

        cli = CliOptionsParser(configs, parser_builder.create_parser_for_runner(),
                               test_suite_parser,
                               parser_builder.create_parser_for_default_test_suite(),
                               workflow_parser, *args)
        cli.parse_args()

        actual = cli.full_options
        actual = cli_utils.restore_duplicated_options(configs, actual)
        actual = cli_utils.add_config_info(configs, actual)
        actual = cli_utils.restore_default_list(actual, key_lists_ts | key_lists_wf)
        expected = data_2.args
        compare_dicts(self, actual, expected)


    def test_wrong_config_names(self) -> None:
        args = [("panda-int1", "ets-runtime"), ("panda-int", "ets-runtime1"),
                ("panda-int1", "ets-runtime1")]
        for arg in args:
            workflow_name, test_suite_name = arg
            with self.assertRaises(InvalidConfiguration):
                cli_utils.check_valid_workflow_name(cli_utils.WorkflowName(workflow_name))
                cli_utils.check_valid_test_suite_name(cli_utils.TestSuiteName(test_suite_name))
