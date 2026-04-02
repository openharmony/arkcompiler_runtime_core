#!/usr/bin/env python3
# -- coding: utf-8 --
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
import unittest
from collections.abc import Callable
from pathlib import Path
from typing import ClassVar
from unittest.mock import patch

from runner.environment import MandatoryPropDescription, RunnerEnv
from runner.options.cli_options import get_args
from runner.options.config import Config
from runner.test import test_utils


class TestSuiteConfigTest4(unittest.TestCase):
    data_folder: ClassVar[Callable[[], Path]] = lambda: test_utils.data_folder(__file__)
    test_environ: ClassVar[dict[str, str]] = {
        'ARKCOMPILER_RUNTIME_CORE_PATH': Path.cwd().as_posix(),
        'ARKCOMPILER_ETS_FRONTEND_PATH': Path.cwd().as_posix(),
        'PANDA_BUILD': Path.cwd().as_posix(),
        'WORK_DIR': (Path.cwd() / f"work-{test_utils.random_suffix()}").as_posix()
    }
    get_instance_id: ClassVar[Callable[[], str]] = lambda: test_utils.create_runner_test_id(__file__)
    env_properties: ClassVar[list[MandatoryPropDescription]] = RunnerEnv.mandatory_props
    test_suite_gen = "test-suite-generator"

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch('sys.argv', ["runner.sh", "config-1", "test_suite4-1"])
    def test_col_gen_from_test_suite_both(self) -> None:
        """
        Test suite has 2 collections
        Gen class is specified for the test-suite overall - it will be copied for collections
        """
        expected_gen = {"collection1": TestSuiteConfigTest4.test_suite_gen,
                        "collection2": TestSuiteConfigTest4.test_suite_gen}
        args = get_args(TestSuiteConfigTest4.env_properties)
        config = Config(args)
        collections_gens = {col.name: col.generator_class for col in config.test_suite.collections}
        try:
            self.assertEqual(expected_gen, collections_gens)
        finally:
            work_dir = Path(os.environ["WORK_DIR"])
            shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch('sys.argv', ["runner.sh", "config-1", "test_suite4-2"])
    def test_col_gen_from_test_suite_one(self) -> None:
        """
        Test suite has 2 collections
        Gen class is specified for collection1, gen class for collection 2 will be copied from test suite params
        """
        col1_gen = "col1-generator"
        args = get_args(TestSuiteConfigTest4.env_properties)
        config = Config(args)
        collections_gens = {col.name: col.generator_class for col in config.test_suite.collections}

        expected_gen = {"collection1": col1_gen,
                        "collection2": TestSuiteConfigTest4.test_suite_gen}
        try:
            self.assertEqual(expected_gen, collections_gens)
        finally:
            work_dir = Path(os.environ["WORK_DIR"])
            shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch('sys.argv', ["runner.sh", "config-1", "test_suite4-3"])
    def test_col_gen(self) -> None:
        """
        Test suite has 2 collections
        Gen class is specified for each, generator class from the test suite is not copied
        """
        col1_gen = "col1-generator"
        col2_gen = "col2-generator"
        args = get_args(TestSuiteConfigTest4.env_properties)

        config = Config(args)
        collections_gens = {col.name: col.generator_class for col in config.test_suite.collections}

        expected_gen = {"collection1": col1_gen,
                        "collection2": col2_gen}
        try:
            self.assertEqual(expected_gen, collections_gens)
        finally:
            work_dir = Path(os.environ["WORK_DIR"])
            shutil.rmtree(work_dir, ignore_errors=True)
