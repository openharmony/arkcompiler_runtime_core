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
import unittest
from pathlib import Path
from typing import List

from runner.common_exceptions import InvalidConfiguration
from runner.enum_types.base_enum import IncorrectEnumValue
from runner.enum_types.verbose_format import VerboseKind
from runner.options.cli_options import CliOptions


class TestSuiteConfigTest2(unittest.TestCase):
    cfg_ext = ".yaml"
    workflow_name = "config-1"
    workflow_path: Path
    test_suite_name = "test_suite2"
    test_suite_path: Path
    current_path = Path(__file__).parent
    cfg_path = current_path.parent.parent.parent.joinpath("cfg")

    @classmethod
    def setUpClass(cls) -> None:
        os.environ["PANDA_SOURCE_PATH"] = "."
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

    def test_empty_args(self) -> None:
        args: List[str] = []
        with self.assertRaises(InvalidConfiguration):
            CliOptions(args)

    def test_only_workflow(self) -> None:
        args = [self.workflow_name]
        with self.assertRaises(InvalidConfiguration):
            CliOptions(args)

    def test_option_without_value(self) -> None:
        args = [self.workflow_name, self.test_suite_name, "--verbose"]
        with self.assertRaises(BaseException):
            CliOptions(args)

    def test_option_with_incorrect_value(self) -> None:
        args = [self.workflow_name, self.test_suite_name, "--verbose", "abc"]
        with self.assertRaises(IncorrectEnumValue):
            CliOptions(args)

    def test_option_with_repeated_value(self) -> None:
        args = [
            self.workflow_name, self.test_suite_name,
            "--verbose", "all", "--verbose", "short"
        ]
        actual = CliOptions(args)
        self.assertEqual(VerboseKind.SHORT, actual.data.get("runner.verbose"))

    def test_option_with_test_suite_option1(self) -> None:
        args = [
            self.workflow_name, self.test_suite_name,
            "--with-js"
        ]
        actual = CliOptions(args)
        self.assertTrue(actual.data.get('test_suite2.parameters.with-js'))

    def test_option_with_test_suite_option2(self) -> None:
        args = [
            self.workflow_name, self.test_suite_name,
            "--validator"
        ]
        with self.assertRaises(InvalidConfiguration):
            CliOptions(args)

    def test_option_with_test_suite_option3(self) -> None:
        args = [
            self.workflow_name, self.test_suite_name,
            "--validator", "a.b.c"
        ]
        actual = CliOptions(args)
        self.assertEqual("a.b.c", actual.data.get('test_suite2.parameters.validator'))

    def test_option_with_test_suite_option_several_times1(self) -> None:
        args = [
            self.workflow_name, self.test_suite_name,
            "--validator", "a.b.c", "--validator", "d.e.f"
        ]
        actual = CliOptions(args)
        self.assertEqual("d.e.f", actual.data.get('test_suite2.parameters.validator'))

    def test_option_with_test_suite_option_several_times2(self) -> None:
        args = [
            self.workflow_name, self.test_suite_name,
            "--es2panda-full-args", "--thread=0", "--es2panda-full-args", "--parse-only"
        ]
        actual = CliOptions(args)
        expected = ['--thread=0', '--parse-only', '--dump-ast', '--extension=${parameters.extension}']
        self.assertEqual(expected, actual.data.get('test_suite2.parameters.es2panda-full-args'))
