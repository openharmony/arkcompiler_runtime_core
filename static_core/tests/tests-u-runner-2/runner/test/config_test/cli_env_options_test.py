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
import unittest
from pathlib import Path
from typing import ClassVar
from unittest.mock import patch

from runner.environment import MandatoryPropDescription, RunnerEnv
from runner.options.cli_options import apply_cli_environment_overrides, get_args


class CliEnvOptionsTest(unittest.TestCase):
    data_folder: ClassVar[Path] = Path(__file__).with_name("data")
    config_path: ClassVar[Path] = data_folder / ".urunner.env"
    work_dir: ClassVar[Path] = Path.cwd()
    new_work_dir: ClassVar[Path] = work_dir / "new_work_dir"
    build_dir: ClassVar[Path] = Path.cwd()
    new_build_dir: ClassVar[Path] = build_dir / "new_build_dir"
    test_environ: ClassVar[dict[str, str]] = {
        'ARKCOMPILER_RUNTIME_CORE_PATH': Path.cwd().as_posix(),
        'ARKCOMPILER_ETS_FRONTEND_PATH': Path.cwd().as_posix(),
        'PANDA_BUILD': build_dir.as_posix(),
        'WORK_DIR': work_dir.as_posix()
    }
    env_properties: ClassVar[list[MandatoryPropDescription]] = RunnerEnv.mandatory_props

    @patch('runner.utils.get_config_workflow_folder', lambda: CliEnvOptionsTest.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: CliEnvOptionsTest.data_folder)
    @patch('sys.argv', ["runner.sh", "config-1", "test_suite1", "--work-dir-runner",
                        f"{new_work_dir.as_posix()}"])
    @patch.dict(os.environ, test_environ, clear=True)
    def test_change_work_dir_env(self) -> None:
        args = get_args(CliEnvOptionsTest.env_properties)
        self.assertEqual(os.environ["WORK_DIR"], CliEnvOptionsTest.work_dir.as_posix())
        apply_cli_environment_overrides(args, CliEnvOptionsTest.env_properties)

        self.assertEqual(os.environ["WORK_DIR"], CliEnvOptionsTest.new_work_dir.as_posix())

    @patch('runner.utils.get_config_workflow_folder', lambda: CliEnvOptionsTest.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: CliEnvOptionsTest.data_folder)
    @patch('sys.argv', ["runner.sh", "config-1", "test_suite1", "--panda-build",
                        f"{new_build_dir.as_posix()}"])
    @patch.dict(os.environ, test_environ, clear=True)
    def test_change_build_dir_env(self) -> None:
        args = get_args(CliEnvOptionsTest.env_properties)
        self.assertEqual(os.environ["PANDA_BUILD"], CliEnvOptionsTest.build_dir.as_posix())
        apply_cli_environment_overrides(args, CliEnvOptionsTest.env_properties)
        self.assertEqual(os.environ["PANDA_BUILD"], CliEnvOptionsTest.new_build_dir.as_posix())

    @patch('runner.utils.get_config_workflow_folder', lambda: CliEnvOptionsTest.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: CliEnvOptionsTest.data_folder)
    @patch('sys.argv', ["runner.sh", "config-1", "test_suite1",
                        "--panda-build", f"{new_build_dir.as_posix()}",
                        "--work-dir-runner", f"{new_work_dir.as_posix()}"])
    @patch.dict(os.environ, test_environ, clear=True)
    def test_change_several_env_vars(self) -> None:
        args = get_args(CliEnvOptionsTest.env_properties)
        self.assertEqual(os.environ["PANDA_BUILD"], CliEnvOptionsTest.build_dir.as_posix())
        self.assertEqual(os.environ["WORK_DIR"], CliEnvOptionsTest.work_dir.as_posix())
        apply_cli_environment_overrides(args, CliEnvOptionsTest.env_properties)
        self.assertEqual(os.environ["PANDA_BUILD"], CliEnvOptionsTest.new_build_dir.as_posix())
        self.assertEqual(os.environ["WORK_DIR"], CliEnvOptionsTest.new_work_dir.as_posix())
