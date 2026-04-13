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
import subprocess
from collections.abc import Callable
from pathlib import Path
from typing import ClassVar
from unittest import TestCase
from unittest.mock import MagicMock, patch

from runner.common_exceptions import CompileEtsTestPartException, CompileEtsTestPartTimeoutException
from runner.environment import MandatoryPropDescription, RunnerEnv
from runner.extensions.generators.gtests_ets_compiler.gtests_ets_compiler import GTestEtsCompiler
from runner.options.cli_options import get_args
from runner.options.config import Config
from runner.options.options_step import Step, StepKind
from runner.test import test_utils
from runner.utils import ani_name_rule, make_gtest_name


class EtsGeneratorTest(TestCase):
    data_folder: ClassVar[Callable[[], Path]] = lambda: test_utils.data_folder(__file__,
                                                                               test_data_folder="config_folder")
    test_environ: ClassVar[dict[str, str]] = test_utils.test_environ(
        'TESTS_LOADING_TEST_ROOT', data_folder().as_posix())
    get_instance_id: ClassVar[Callable[[], str]] = lambda: test_utils.create_runner_test_id(__file__)
    env_properties: ClassVar[list[MandatoryPropDescription]] = RunnerEnv.mandatory_props
    failed_test = Path("fail.ets")
    failed_timeout_test = Path("fail_timeout.ets")
    cmake_fake = "CMake.fake"
    cmake_for_test = "CMakeLists.txt"

    def config_test(self) -> Config:
        args = get_args(EtsGeneratorTest.env_properties)
        config = Config(args)
        return config

    def setUp(self) -> None:
        self._compiled: list[Path] = []
        self._compiler: GTestEtsCompiler | None = None

    def clear_after_test(self, gen_folder: Path, int_folder: Path) -> None:
        shutil.rmtree(gen_folder, ignore_errors=True)
        shutil.rmtree(int_folder, ignore_errors=True)

    def fake_compile_file(self, _step: Step, test_id: Path) -> None:
        if test_id.name == self.failed_test.name:
            raise CompileEtsTestPartException("compile failed")

        if test_id.name == self.failed_timeout_test.name:
            raise CompileEtsTestPartTimeoutException("compile timeout")

        self._compiled.append(test_id)
        assert self._compiler is not None

        output_dir = self._compiler.intermediate
        output_dir.mkdir(parents=True, exist_ok=True)
        abc_name = make_gtest_name(test_id, "ani_test", ani_name_rule)
        fake_abc = (output_dir / abc_name).with_suffix(".abc")
        fake_abc.write_text("", encoding="utf-8")

    def get_compiled_tests(self) -> list[Path]:
        if self._compiler is not None:
            return list(self._compiler.intermediate.glob("*.abc"))
        return []

    def get_generated_tests_from_j2(self) -> list[Path]:
        if self._compiler is not None:
            return list(self._compiler.gen.glob("*.ets"))
        return []

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('sys.argv', ["runner.sh", "ani_tests_wf", "ani_suite", "--test-file", "ani_test_any_call/AnyCallTest"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    def test_gtest_ets_compile(self) -> None:
        test_source_path: Path = Path(__file__).with_name("ets")
        work_dir = Path(os.environ["WORK_DIR"])
        test_gen_path: Path = work_dir.with_name("gen")
        test_intermediate_path = work_dir.with_name("intermediate")
        test_config = self.config_test()

        shutil.rmtree(test_gen_path, ignore_errors=True)
        shutil.rmtree(test_intermediate_path, ignore_errors=True)

        ets_test_generator = GTestEtsCompiler(test_source_path, test_gen_path, test_config)
        self._compiler = ets_test_generator

        with patch.object(ets_test_generator, "compile_file", side_effect=self.fake_compile_file):
            _ = ets_test_generator.generate()
        try:
            self.assertEqual(len(self._compiled), 1)

            gen_sources = self.get_generated_tests_from_j2()
            abc_files = self.get_compiled_tests()

            # there were no j2 ets templates, all ets were found in test sources
            self.assertEqual(len(gen_sources), 0)

            self.assertEqual(len(abc_files), 1)

            self.assertTrue(all(abc_file.name.startswith("ani_test_") for abc_file in abc_files))
        finally:
            # clear up
            shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('sys.argv', ["runner.sh", "ani_tests_wf", "ani_suite", "--test-file", "ani_test_any_call/AnyCallTest"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    def test_gtest_ets_j2_compile(self) -> None:
        test_source_path: Path = Path(__file__).with_name("ets_j2")
        cmake_src = test_source_path / EtsGeneratorTest.cmake_fake
        cmake_dst = test_source_path / EtsGeneratorTest.cmake_for_test

        work_dir = Path(os.environ["WORK_DIR"])
        test_gen_path: Path = work_dir.with_name("gen")
        test_intermediate_path = work_dir.with_name("intermediate")
        test_config = self.config_test()

        shutil.rmtree(test_gen_path, ignore_errors=True)
        shutil.rmtree(test_intermediate_path, ignore_errors=True)

        ets_test_generator = GTestEtsCompiler(test_source_path, test_gen_path, test_config)
        self._compiler = ets_test_generator

        with patch.object(ets_test_generator, "compile_file", side_effect=self.fake_compile_file):
            try:
                # rename cmake file for test to be discoverable
                cmake_dst.unlink(missing_ok=True)
                cmake_src.rename(cmake_dst)

                _ = ets_test_generator.generate()
            finally:
                if cmake_dst.exists():
                    cmake_dst.rename(cmake_src)

        gen_sources = self.get_generated_tests_from_j2()
        abc_files = self.get_compiled_tests()

        try:
            # the source file was .ets.j2, there were generated 3 tests in {WORK_DIR}/gen
            self.assertEqual(len(gen_sources), 3)

            self.assertEqual(len(self._compiled), 3)
            self.assertEqual(len(abc_files), 3)

            self.assertTrue(all(abc_file.name.startswith("ani_verify_test") for abc_file in abc_files))
        finally:
            # clear up
            shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('sys.argv', ["runner.sh", "ani_tests_wf", "ani_suite", "--test-file", "ani_test_any_call/AnyCallTest"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    def test_gtest_ets_j2_with_config_compile(self) -> None:
        test_source_path: Path = Path(__file__).with_name("ets_j2_conf")
        cmake_src = test_source_path / EtsGeneratorTest.cmake_fake
        cmake_dst = test_source_path / EtsGeneratorTest.cmake_for_test

        work_dir = Path(os.environ["WORK_DIR"])
        test_gen_path: Path = work_dir / "ani-tests" / "gen"
        test_intermediate_path = work_dir / "ani-tests" / "intermediate"
        test_config = self.config_test()

        shutil.rmtree(test_gen_path, ignore_errors=True)
        shutil.rmtree(test_intermediate_path, ignore_errors=True)

        ets_test_generator = GTestEtsCompiler(test_source_path, test_gen_path, test_config)
        self._compiler = ets_test_generator

        with patch.object(ets_test_generator, "compile_file", side_effect=self.fake_compile_file):
            try:
                # rename cmake file for test to be discoverable
                cmake_dst.unlink(missing_ok=True)
                cmake_src.rename(cmake_dst)

                _ = ets_test_generator.generate()
            finally:
                if cmake_dst.exists():
                    cmake_dst.rename(cmake_src)

        gen_sources = self.get_generated_tests_from_j2()
        abc_files = self.get_compiled_tests()

        try:
            # the source file was .ets.j2, there were generated 3 tests in {WORK_DIR}/gen
            self.assertEqual(len(gen_sources), 9)

            self.assertEqual(len(self._compiled), 9)
            self.assertEqual(len(abc_files), 9)

            # tests has custom config, it was copied with tests
            custom_config = list(self._compiler.gen.glob("*.json"))
            self.assertEqual(len(custom_config), 1)

            self.assertTrue(all(abc_file.name.startswith("ani_verify_test") for abc_file in abc_files))
        finally:
            # clear up
            shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('sys.argv', ["runner.sh", "ani_tests_wf", "ani_suite", "--test-file", "ani_test_any_call/AnyCallTest"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    def test_gtest_ets_partial_compile_failure(self) -> None:
        test_source_path: Path = Path(__file__).with_name("ets_with_failures")
        work_dir = Path(os.environ["WORK_DIR"])
        test_gen_path: Path = work_dir.with_name("gen")
        test_intermediate_path = work_dir.with_name("intermediate")
        test_config = self.config_test()

        shutil.rmtree(test_gen_path, ignore_errors=True)
        shutil.rmtree(test_intermediate_path, ignore_errors=True)

        ets_test_generator = GTestEtsCompiler(test_source_path, test_gen_path, test_config)
        self._compiler = ets_test_generator

        try:
            with patch.object(ets_test_generator, "compile_file", side_effect=self.fake_compile_file):
                compiled_tests = ets_test_generator.generate()

            # 2 tests from 4 - failed, the other 2 were compiled
            self.assertEqual(len(compiled_tests), 2)
        finally:
            shutil.rmtree(work_dir, ignore_errors=True)

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('sys.argv', ["runner.sh", "ani_tests_wf", "ani_suite", "--test-file", "ani_test_any_call/AnyCallTest"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch("runner.extensions.generators.gtests_ets_compiler.gtests_ets_compiler.subprocess.run")
    def test_gtest_ets_compilation_failed(self, mock_run: MagicMock) -> None:
        mock_run.return_value = subprocess.CompletedProcess(
            args=["/usr/bin/echo"],
            returncode=1,
            stdout="stdout",
            stderr="stderr"
        )

        test_source_path: Path = Path(__file__).with_name("ets")
        work_dir = Path(os.environ["WORK_DIR"])
        test_gen_path: Path = work_dir.with_name("gen")
        test_intermediate_path = work_dir.with_name("intermediate")
        step = Step(
            name="test_step",
            timeout=0,
            args=[],
            env={},
            step_kind=StepKind.COMPILER
        )

        test_config = self.config_test()

        shutil.rmtree(test_gen_path, ignore_errors=True)
        shutil.rmtree(test_intermediate_path, ignore_errors=True)

        ets_test_generator = GTestEtsCompiler(test_source_path, test_gen_path, test_config)

        self.assertRaises(CompileEtsTestPartException, ets_test_generator.compile_file, step,
                          test_source_path / "test_func_call.ets")

    @patch('runner.utils.get_config_workflow_folder', data_folder)
    @patch('runner.utils.get_config_test_suite_folder', data_folder)
    @patch('sys.argv', ["runner.sh", "ani_tests_wf", "ani_suite", "--test-file", "ani_test_any_call/AnyCallTest"])
    @patch.dict(os.environ, test_environ, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    @patch("runner.extensions.generators.gtests_ets_compiler.gtests_ets_compiler.subprocess.run")
    def test_gtest_ets_compilation_timeout(self, mock_run: MagicMock) -> None:
        mock_run.side_effect = subprocess.TimeoutExpired(cmd="cmd", timeout=30)

        test_source_path: Path = Path(__file__).with_name("ets")
        work_dir = Path(os.environ["WORK_DIR"])
        test_gen_path: Path = work_dir.with_name("gen")
        test_intermediate_path = work_dir.with_name("intermediate")
        step = Step(
            name="test_step",
            timeout=0,
            args=[],
            env={},
            step_kind=StepKind.COMPILER
        )
        test_config = self.config_test()

        shutil.rmtree(test_gen_path, ignore_errors=True)
        shutil.rmtree(test_intermediate_path, ignore_errors=True)

        ets_test_generator = GTestEtsCompiler(test_source_path, test_gen_path, test_config)

        self.assertRaises(CompileEtsTestPartTimeoutException, ets_test_generator.compile_file, step,
                          test_source_path / "test_func_call.ets")
