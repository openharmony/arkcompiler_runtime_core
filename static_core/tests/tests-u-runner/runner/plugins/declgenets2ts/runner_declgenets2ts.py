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
import logging
import multiprocessing
import os
import subprocess
from collections import namedtuple
from datetime import datetime
from pathlib import Path
from typing import Any, Callable, cast, Generator, List, Set

import pytz
import yaml
from tqdm import tqdm

from runner.enum_types.test_directory import TestDirectory
from runner.logger import Log
from runner.options.cli_args_wrapper import CliArgsWrapper
from runner.options.config import Config
from runner.plugins.declgenets2ts.declgenets2ts_suites import DeclgenEtsSuites
from runner.plugins.declgenets2ts.test_declgenets2ts import TestDeclgenETS2TS
from runner.plugins.ets.ets_test_dir import EtsTestDir
from runner.plugins.ets.ets_test_suite import (CtsEtsTestSuite, EtsTestSuite, FuncEtsTestSuite, RuntimeEtsTestSuite)
from runner.plugins.ets.preparation_step import CopyStep, JitStep
from runner.plugins.work_dir import WorkDir
from runner.runner_base import get_test_id, worker_cli_wrapper_args, init_worker
from runner.runner_file_based import RunnerFileBased
from runner.test_base import Test

_LOGGER = logging.getLogger(
    "runner.plugins.declgenets2ts.runner_declgenets2ts")

DeclgenEtsConfig = namedtuple('DeclgenEtsConfig', ['declgen_ets_name', 'ignored_ets_prefix'])


class DeclgenSdkEtsTestSuite(EtsTestSuite):
    def __init__(self, config: Config, work_dir: WorkDir, default_list_root: str):
        super().__init__(config, work_dir, DeclgenEtsSuites.DECLGENSDK.value, default_list_root)
        self._ets_test_dir = EtsTestDir(config.general.static_core_root, config.general.test_root)
        self.set_preparation_steps()

    def set_preparation_steps(self) -> None:
        self._preparation_steps.append(CopyStep(
            test_source_path=self._ets_test_dir.declgen_sdk,
            test_gen_path=self.test_root,
            config=self.config
        ))
        if self._is_jit:
            self._preparation_steps.append(JitStep(
                test_source_path=self.test_root,
                test_gen_path=self.test_root,
                config=self.config,
                num_repeats=self._jit.num_repeats
            ))


class InvalidConfiguration(Exception):
    pass


def run_declgen_test(test: TestDeclgenETS2TS) -> TestDeclgenETS2TS:
    start = datetime.now(pytz.UTC)
    Log.all(_LOGGER, f"Start to execute declgen_ets2ts: {test.test_id}")
    CliArgsWrapper.args = worker_cli_wrapper_args
    test.passed, test.report, test.fail_kind = test.run_declgen_ets2ts(test.test_dets, test.test_ets)
    finish = datetime.now(pytz.UTC)
    test.time = (finish - start).total_seconds()
    if not test.passed:
        # If a test is failed, it won't go to the next stages,
        # so we define status, output log and generate report here
        Test.output_status_and_log(test)
        Test.create_report(test)
    return test


def run_tsc_test(test: TestDeclgenETS2TS) -> TestDeclgenETS2TS:
    start = datetime.now(pytz.UTC)
    Log.all(_LOGGER, f"Start to execute tsc: {test.test_id}")
    CliArgsWrapper.args = worker_cli_wrapper_args
    test.passed, test.report, test.fail_kind = test.run_tsc(test.test_dets)
    finish = datetime.now(pytz.UTC)
    if test.time is None:
        test.time = 0
    test.time = test.time + (finish - start).total_seconds()
    # as tsc is the last stage, we define here status, output log and generate report
    Test.output_status_and_log(test)
    Test.create_report(test)
    return test


class RunnerDeclgenETS2TS(RunnerFileBased):
    def __init__(self, config: Config):
        config.verifier.enable = False
        self.ets_config = self.get_declgen_ets_name(config.test_suites)
        super().__init__(config, self.ets_config.declgen_ets_name)

        self.declgen_ets2ts_executor = os.path.join(
            self.build_dir, "bin", "declgen_ets2ts")

        test_suite_class = self.get_declgen_ets_class(self.ets_config.declgen_ets_name)
        if self.ets_config.declgen_ets_name == DeclgenEtsSuites.RUNTIME.value:
            self.default_list_root = self._get_frontend_test_lists()
        self.declgen_list_root = os.path.join(self.default_list_root, "declgenets2ts")
        test_suite = test_suite_class(
            self.config, self.work_dir, self.declgen_list_root)
        test_suite.process(
            self.config.general.generate_only or self.config.ets.force_generate)
        self.test_root, self.list_root = test_suite.test_root, test_suite.list_root

        Log.short(_LOGGER, f"TEST_ROOT set to {self.test_root}")
        Log.short(_LOGGER, f"LIST_ROOT set to {self.list_root}")

        self.collect_ignored_excluded_lists()
        self.explicit_list = self.recalculate_explicit_list(config.test_lists.explicit_list)
        self.add_directories([TestDirectory(self.test_root, "ets", [])])

    @staticmethod
    def get_declgen_ets_class(ets_suite_name: str) -> Any:
        name_to_class = {
            DeclgenEtsSuites.CTS.value: CtsEtsTestSuite,
            DeclgenEtsSuites.FUNC.value: FuncEtsTestSuite,
            DeclgenEtsSuites.RUNTIME.value: RuntimeEtsTestSuite,
            DeclgenEtsSuites.DECLGENSDK.value: DeclgenSdkEtsTestSuite,
        }
        return name_to_class.get(ets_suite_name)

    @staticmethod
    def run_command(command: list, cwd: str, description: str, timeout: int = 60 * 10) -> None:
        Log.short(_LOGGER, f"Running command: {' '.join(command)}")
        Log.short(_LOGGER, description)
        with subprocess.Popen(
                command,
                cwd=cwd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                stdin=subprocess.PIPE) as process:
            try:
                stdout, stderr = process.communicate(timeout=timeout)
                stdout_utf8 = stdout.decode("utf-8", errors="ignore")
                stderr_utf8 = stderr.decode("utf-8", errors="ignore")
                if process.returncode != 0:
                    Log.exception_and_raise(
                        _LOGGER, f"invalid return code {process.returncode}\n {stdout_utf8}\n {stderr_utf8}\n")
            except subprocess.TimeoutExpired as ex:
                Log.exception_and_raise(
                    _LOGGER, f"Timeout: {' '.join(command)} failed with {ex}")
            except Exception as ex:  # pylint: disable=broad-except
                Log.exception_and_raise(
                    _LOGGER, f"Exception{' '.join(command)} failed with {ex}")

    @staticmethod
    def read_package_info(package_json_path: str) -> str:
        try:
            with open(package_json_path, "r", encoding="utf-8") as file:
                package_data = yaml.safe_load(file)
            name = package_data.get("name")
            version = package_data.get("version")

            if not name or not version:
                Log.exception_and_raise(
                    _LOGGER, "The 'name' or 'version' field is missing in the YAML file.")

            package_name = f"{name}-{version}.tgz"
            return package_name
        except Exception as ex:  # pylint: disable=broad-except
            Log.exception_and_raise(_LOGGER, f"Error reading YAML file: {ex}")

    @staticmethod
    def filter_not_run(tests: List[TestDeclgenETS2TS]) -> Generator[TestDeclgenETS2TS, Any, None]:
        Log.short(_LOGGER, "Exclude compile-only-negative tests")
        for test in tests:
            if test.is_compile_only_negative:
                test.passed = True
                test.fail_kind = None
                test.report = None
                test.excluded = True
                Test.output_status_and_log(test)
                yield test
        yield from []

    @property
    def default_work_dir_root(self) -> Path:
        return Path("/tmp/declgenets2ts") / str(self.ets_config.declgen_ets_name)

    def collect_ignored_excluded_lists(self) -> None:
        Log.all(_LOGGER, f"Collecting test lists and tests for names"
                         f" {self.ets_config.declgen_ets_name} and {self.ets_config.ignored_ets_prefix}")
        self.collect_excluded_test_lists(test_name=self.ets_config.declgen_ets_name)
        self.collect_ignored_test_lists(test_name=self.ets_config.declgen_ets_name)

        self.list_root = os.path.join(self.default_list_root, self.ets_config.ignored_ets_prefix)
        self.collect_excluded_test_lists(test_name=self.ets_config.ignored_ets_prefix)
        self.collect_ignored_test_lists(test_name=self.ets_config.ignored_ets_prefix)

    def create_test(self, test_file: str, flags: List[str], is_ignored: bool) -> TestDeclgenETS2TS:
        test = TestDeclgenETS2TS(self.test_env, test_file, flags, get_test_id(
            test_file, self.test_root), self.build_dir, self.ets_config.declgen_ets_name)
        test.ignored = is_ignored
        return test

    def get_declgen_ets_name(self, test_suites: Set[str]) -> DeclgenEtsConfig:
        if "declgen_ets2ts_cts" in test_suites:
            return DeclgenEtsConfig(DeclgenEtsSuites.CTS.value, "ets-cts")
        if "declgen_ets2ts_func_tests" in test_suites:
            return DeclgenEtsConfig(DeclgenEtsSuites.FUNC.value, "ets-func-tests")
        if "declgen_ets2ts_runtime" in test_suites:
            return DeclgenEtsConfig(DeclgenEtsSuites.RUNTIME.value, "ets-runtime")
        if "declgen_ets2ts_sdk" in test_suites:
            return DeclgenEtsConfig(DeclgenEtsSuites.DECLGENSDK.value, "ets-sdk")
        message = f"Unsupported test suite: {self.config.test_suites}"
        Log.exception_and_raise(_LOGGER, message, InvalidConfiguration)

    def run_stage(self, stage_name: str, tests: List[TestDeclgenETS2TS],
                  run_test: Callable[[TestDeclgenETS2TS], TestDeclgenETS2TS]) -> List[
        TestDeclgenETS2TS]:
        Log.default(_LOGGER, f"Start stage '{stage_name}' running for every test")
        with multiprocessing.Pool(processes=self.config.general.processes,
                                  initializer=init_worker, initargs=(CliArgsWrapper.args,)) as pool:
            results = pool.imap_unordered(run_test, tests, chunksize=self.config.general.chunksize)
            if self.config.general.show_progress:
                results = tqdm(results, total=len(tests))
            stage_results: List[TestDeclgenETS2TS] = list(results)
            pool.close()
            pool.join()
            return stage_results

    def run(self) -> None:
        """
        Fully override `run` from base class `Runner` -
        here, first, all tests goes through the first stage, declgen_ets2ts,
        then all tests goes through the second stage, tsc.
        """
        all_tests = [cast(TestDeclgenETS2TS, test) for test in self.tests]
        not_run = list(self.filter_not_run(all_tests))
        declgen_to_run = [test for test in all_tests if not test.is_compile_only_negative]
        declgen_result = self.run_stage("declgen_ets2ts", declgen_to_run, run_declgen_test)
        declgen_failed = [result for result in declgen_result if not result.passed]
        tsc_to_run = [result for result in declgen_result if result.passed and result.is_valid_test]
        tsc_result = self.run_stage("tsc", tsc_to_run, run_tsc_test)
        self.results = tsc_result + declgen_failed + not_run
