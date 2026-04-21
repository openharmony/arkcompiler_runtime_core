#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#

import os
import shutil
from pathlib import Path
from typing import ClassVar, cast
from unittest import TestCase
from unittest.mock import patch

from runner.common_exceptions import InvalidInitialization
from runner.environment import MandatoryPropDescription, RunnerEnv
from runner.init_runner import InitRunner, MandatoryProp
from runner.test.test_utils import assert_not_raise, random_suffix
from runner.utils import write_2_file

non_existing_path_01: str = str(Path(__file__).parent / "non-exist-01")
existing_path_01: str = str(Path(__file__).parent / "env-exist-01")
existing_path_02: str = str(Path(__file__).parent / "env-exist-02")


class EnvironmentTest(TestCase):
    runner_help_mode_off: ClassVar[bool] = False
    runner_help_mode_on: ClassVar[bool] = True

    @staticmethod
    def clear_dir(dir_path: Path) -> None:
        if dir_path.exists():
            dir_path.rmdir()

    def setUp(self) -> None:
        urunner_test: str = ".urunner.test." + random_suffix()
        self.urunner_path = Path(__file__).with_name(urunner_test)

    def tearDown(self) -> None:
        self.urunner_path.unlink(missing_ok=True)

    @patch.dict(os.environ, {}, clear=True)
    def test_get_mandatory_props_empty(self) -> None:
        expected = {
            'ARKCOMPILER_RUNTIME_CORE_PATH': MandatoryProp(None, is_path=True, require_exist=True, mandatory=True),
            'ARKCOMPILER_ETS_FRONTEND_PATH': MandatoryProp(None, is_path=True, require_exist=True, mandatory=True),
            'PANDA_BUILD': MandatoryProp(None, is_path=True, require_exist=True, mandatory=True),
            'WORK_DIR': MandatoryProp(None, is_path=True, require_exist=False, mandatory=True),
            'CFG_PATH': MandatoryProp(None, is_path=True, require_exist=True, mandatory=False)
        }
        props = RunnerEnv.get_mandatory_props()
        self.assertIsInstance(props, dict)
        self.assertDictEqual(props, expected)

    @patch.dict(os.environ, {
        'ARKCOMPILER_RUNTIME_CORE_PATH': "aaa",
        'ARKCOMPILER_ETS_FRONTEND_PATH': "bbb",
        'PANDA_BUILD': "ccc",
        'WORK_DIR': "ddd",
        'CFG_PATH': "eee"
    }, clear=True)
    def test_get_mandatory_props_full(self) -> None:
        expected = {
            'ARKCOMPILER_RUNTIME_CORE_PATH': MandatoryProp("aaa", is_path=True, require_exist=True, mandatory=True),
            'ARKCOMPILER_ETS_FRONTEND_PATH': MandatoryProp("bbb", is_path=True, require_exist=True, mandatory=True),
            'PANDA_BUILD': MandatoryProp("ccc", is_path=True, require_exist=True, mandatory=True),
            'WORK_DIR': MandatoryProp("ddd", is_path=True, require_exist=False, mandatory=True),
            'CFG_PATH': MandatoryProp("eee", is_path=True, require_exist=True, mandatory=False)
        }
        props = RunnerEnv.get_mandatory_props()
        self.assertIsInstance(props, dict)
        self.assertDictEqual(props, expected)

    @patch.dict(os.environ, {}, clear=True)
    def test_check_mandatory_prop_absent(self) -> None:
        # test
        self.assertRaises(
            InvalidInitialization,
            RunnerEnv.expand_mandatory_prop,
            MandatoryPropDescription('ARKCOMPILER_RUNTIME_CORE_PATH', is_path=True, require_exist=True),
            EnvironmentTest.runner_help_mode_off)

    @patch.dict(os.environ, {}, clear=True)
    def test_check_mandatory_prop_absent_help_mode(self) -> None:
        # test
        assert_not_raise(
            test_case=self,
            cls=InvalidInitialization,
            function=RunnerEnv.expand_mandatory_prop,
            params=[MandatoryPropDescription('ARKCOMPILER_RUNTIME_CORE_PATH', is_path=True, require_exist=True),
                    EnvironmentTest.runner_help_mode_on])

    @patch.dict(os.environ, {
        'ARKCOMPILER_RUNTIME_CORE_PATH': non_existing_path_01,
    }, clear=True)
    def test_check_mandatory_prop_not_exist_but_must(self) -> None:
        # test
        self.assertRaises(
            InvalidInitialization,
            RunnerEnv.expand_mandatory_prop,
            MandatoryPropDescription('ARKCOMPILER_RUNTIME_CORE_PATH', is_path=True, require_exist=True),
            EnvironmentTest.runner_help_mode_off)

    @patch.dict(os.environ, {
        'WORK_DIR': non_existing_path_01,
    }, clear=True)
    def test_check_mandatory_prop_not_exist_and_should_not(self) -> None:
        # test
        assert_not_raise(
            test_case=self,
            cls=InvalidInitialization,
            function=RunnerEnv.expand_mandatory_prop,
            params=[MandatoryPropDescription('WORK_DIR', is_path=True, require_exist=False),
                    EnvironmentTest.runner_help_mode_off])

    @patch.dict(os.environ, {
        'ARKCOMPILER_RUNTIME_CORE_PATH': existing_path_01,
    }, clear=True)
    def test_check_mandatory_prop_exist_and_must(self) -> None:
        # preparation
        Path(existing_path_01).mkdir(parents=True, exist_ok=True)
        # test
        assert_not_raise(
            test_case=self,
            cls=InvalidInitialization,
            function=RunnerEnv.expand_mandatory_prop,
            params=[MandatoryPropDescription('ARKCOMPILER_RUNTIME_CORE_PATH', is_path=True, require_exist=True),
                    EnvironmentTest.runner_help_mode_off])
        # clear up
        self.clear_dir(Path(existing_path_01))

    @patch.dict(os.environ, {
        'WORK_DIR': existing_path_02,
    }, clear=True)
    def test_check_mandatory_prop_exist_and_should_not(self) -> None:
        # preparation
        Path(existing_path_02).mkdir(parents=True, exist_ok=True)
        # test
        assert_not_raise(
            test_case=self,
            cls=InvalidInitialization,
            function=RunnerEnv.expand_mandatory_prop,
            params=[MandatoryPropDescription('WORK_DIR', is_path=True, require_exist=False),
                    EnvironmentTest.runner_help_mode_off])
        # clear up
        self.clear_dir(Path(existing_path_02))

    @patch.dict(os.environ, {}, clear=True)
    def test_load_local_env_not_set(self) -> None:
        init_runner = InitRunner()
        # test
        assert_not_raise(
            test_case=self,
            cls=Exception,
            function=RunnerEnv(global_env=init_runner.urunner_env_path).load_local_env,
            params=[])

    @patch.dict(os.environ, {}, clear=True)
    def test_load_local_env_not_exist(self) -> None:
        # preparation
        local_env = Path(__file__).with_name(f".env.{random_suffix()}")
        # test
        init_runner = InitRunner()
        assert_not_raise(
            test_case=self,
            cls=Exception,
            function=RunnerEnv(global_env=init_runner.urunner_env_path, local_env=local_env).load_local_env,
            params=[])

    @patch.dict(os.environ, {}, clear=True)
    def test_load_local_env_exist(self) -> None:
        # preparation
        local_env_path = Path(__file__).with_name(f".env.{random_suffix()}")
        write_2_file(local_env_path, "aaa=bbb")
        init_runner = InitRunner()
        # test
        runner_env = RunnerEnv(local_env=local_env_path, global_env=init_runner.urunner_env_path)
        self.assertIsNotNone(runner_env.local_env_path)
        self.assertEqual(cast(Path, runner_env.local_env_path).name, local_env_path.name)
        runner_env.load_local_env()
        self.assertEqual(os.environ["aaa"], "bbb",
                         "Property `aaa` has not read from local env file")
        # clear up
        local_env_path.unlink(missing_ok=True)

    @patch.dict(os.environ, {}, clear=True)
    def test_load_home_env_not_exist(self) -> None:
        # preparation
        runner_env = RunnerEnv(global_env=self.urunner_path)
        self.urunner_path.unlink(missing_ok=True)
        # test
        assert_not_raise(
            test_case=self,
            cls=Exception,
            function=runner_env.load_home_env,
            params=[])

    @patch.dict(os.environ, {}, clear=True)
    def test_load_home_env_exist(self) -> None:
        # preparation
        self.urunner_path.unlink(missing_ok=True)
        write_2_file(self.urunner_path, "aaa=bbb")
        # test
        runner_env = RunnerEnv(global_env=self.urunner_path)
        runner_env.load_home_env()
        self.assertEqual(os.environ["aaa"], "bbb",
                         "Property `aaa` has not read from global env file")
        # clear up
        self.urunner_path.unlink(missing_ok=True)

    @patch.dict(os.environ, {}, clear=True)
    def test_load_environment_from_file(self) -> None:
        # preparation
        self.urunner_path.unlink(missing_ok=True)
        current_path = Path(__file__).parent
        runtime_path = current_path / f"aaa-{random_suffix()}"
        frontend_path = current_path / f"bbb-{random_suffix()}"
        build_path = current_path / f"ccc-{random_suffix()}"
        work_path = current_path / f"ddd-{random_suffix()}"
        cfg_path = current_path / f"eee-{random_suffix()}"

        runtime_path.mkdir(parents=True, exist_ok=True)
        frontend_path.mkdir(parents=True, exist_ok=True)
        build_path.mkdir(parents=True, exist_ok=True)
        cfg_path.mkdir(parents=True, exist_ok=True)

        os.environ[RunnerEnv.mandatory_props[0][0]] = str(runtime_path)
        os.environ[RunnerEnv.mandatory_props[1][0]] = str(frontend_path)
        os.environ[RunnerEnv.mandatory_props[2][0]] = str(build_path)
        os.environ[RunnerEnv.mandatory_props[3][0]] = str(work_path)
        os.environ[RunnerEnv.mandatory_props[4][0]] = str(cfg_path)
        # test
        runner_env = RunnerEnv(global_env=self.urunner_path)
        runner_env.load_environment()
        runtime_env = os.getenv(RunnerEnv.mandatory_props[0][0])
        frontend_env = os.getenv(RunnerEnv.mandatory_props[1][0])
        build_env = os.getenv(RunnerEnv.mandatory_props[2][0])
        work_env = os.getenv(RunnerEnv.mandatory_props[3][0])
        cfg_env = os.getenv(RunnerEnv.mandatory_props[4][0])

        self.assertFalse(self.urunner_path.exists())
        self.assertEqual(str(runtime_path), runtime_env, f"for {RunnerEnv.mandatory_props[0][0]}")
        self.assertEqual(str(frontend_path), frontend_env, f"for {RunnerEnv.mandatory_props[1][0]}")
        self.assertEqual(str(build_path), build_env, f"for {RunnerEnv.mandatory_props[2][0]}")
        self.assertEqual(str(work_path), work_env, f"for {RunnerEnv.mandatory_props[3][0]}")
        self.assertEqual(str(cfg_path), cfg_env, f"for {RunnerEnv.mandatory_props[4][0]}")
        # clear up
        cfg_path.rmdir()
        runtime_path.rmdir()
        frontend_path.rmdir()
        build_path.rmdir()
        self.urunner_path.unlink(missing_ok=True)

    @patch.dict(os.environ, {}, clear=True)
    def test_load_env_from_file_cfg_path_default(self) -> None:
        # preparation
        self.urunner_path.unlink(missing_ok=True)
        current_path = Path(__file__).parent
        runtime_path = current_path / f"aaa-{random_suffix()}"
        frontend_path = current_path / f"bbb-{random_suffix()}"
        build_path = current_path / f"ccc-{random_suffix()}"
        work_path = current_path / f"ddd-{random_suffix()}"
        default_cfg_pth = runtime_path / "static_core" / "tests" / "tests-u-runner-2" / "cfg"

        props = {
            'ARKCOMPILER_RUNTIME_CORE_PATH': str(runtime_path),
            'ARKCOMPILER_ETS_FRONTEND_PATH': str(frontend_path),
            'PANDA_BUILD': str(build_path),
            'WORK_DIR': str(work_path)
        }
        result = [f"{prop}={value}" for prop, value in props.items()]
        write_2_file(self.urunner_path, "\n".join(result))

        runtime_path.mkdir(parents=True, exist_ok=True)
        frontend_path.mkdir(parents=True, exist_ok=True)
        build_path.mkdir(parents=True, exist_ok=True)
        default_cfg_pth.mkdir(parents=True, exist_ok=True)

        try:
            runner_env = RunnerEnv(global_env=self.urunner_path)
            runner_env.load_environment()

            self.assertEqual(len(os.environ), 5)
            for key, val in props.items():
                self.assertEqual(os.environ.get(key), val)
            self.assertEqual(os.environ.get("CFG_PATH"), str(default_cfg_pth))
        finally:
            shutil.rmtree(runtime_path)
            shutil.rmtree(frontend_path)
            shutil.rmtree(build_path)

    @patch.dict(os.environ, {}, clear=True)
    def test_load_env_from_file_cfg_path_set(self) -> None:
        # preparation
        self.urunner_path.unlink(missing_ok=True)
        current_path = Path(__file__).parent
        runtime_path = current_path / f"aaa-{random_suffix()}"
        frontend_path = current_path / f"bbb-{random_suffix()}"
        build_path = current_path / f"ccc-{random_suffix()}"
        work_path = current_path / f"ddd-{random_suffix()}"
        cfg_pth = runtime_path / "cfg"

        props = {
            'ARKCOMPILER_RUNTIME_CORE_PATH': str(runtime_path),
            'ARKCOMPILER_ETS_FRONTEND_PATH': str(frontend_path),
            'PANDA_BUILD': str(build_path),
            'WORK_DIR': str(work_path),
            'CFG_PATH': str(cfg_pth)
        }
        result = [f"{prop}={value}" for prop, value in props.items()]
        write_2_file(self.urunner_path, "\n".join(result))

        runtime_path.mkdir(parents=True, exist_ok=True)
        frontend_path.mkdir(parents=True, exist_ok=True)
        build_path.mkdir(parents=True, exist_ok=True)
        cfg_pth.mkdir(parents=True, exist_ok=True)

        try:
            runner_env = RunnerEnv(global_env=self.urunner_path)
            runner_env.load_environment()

            self.assertEqual(len(os.environ), 5)
            for key, val in props.items():
                self.assertEqual(os.environ.get(key), val)
            self.assertEqual(os.environ.get("CFG_PATH"), str(cfg_pth))
        finally:
            shutil.rmtree(runtime_path)
            shutil.rmtree(frontend_path)
            shutil.rmtree(build_path)
