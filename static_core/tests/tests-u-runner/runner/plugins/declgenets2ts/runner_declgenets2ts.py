#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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
#

import subprocess
import os
import shutil
import logging
from pathlib import Path
from typing import Any, List, Set

import yaml

from runner.logger import Log
from runner.options.config import Config
from runner.plugins.declgenets2ts.declgenets2ts_test_suite import DeclgenCtsEtsTestSuite
from runner.plugins.declgenets2ts.declgenets2ts_suites import DeclgenEtsSuites, DeclgenEtsSuitesDir
from runner.plugins.declgenets2ts.test_declgenets2ts import TestDeclgenETS2TS
from runner.runner_base import get_test_id
from runner.runner_file_based import RunnerFileBased
from runner.enum_types.test_directory import TestDirectory

TSC_REPO_URL = "TSC_REPO_URL"

_LOGGER = logging.getLogger(
    "runner.plugins.declgenets2ts.runner_declgenets2ts")


class RunnerDeclgenETS2TS(RunnerFileBased):
    def __init__(self, config: Config):
        config.verifier.enable = False
        self.__ets_suite_name = self.get_ets_suite_name(config.test_suites)
        self.__ets_suite_dir = self.get_ets_suite_dir(config.test_suites)

        super().__init__(config, self.__ets_suite_name)

        self.declgen_ets2ts_executor = os.path.join(
            self.build_dir, "bin", "declgen_ets2ts")
        self.tsc_executor = os.path.join(
            self.build_dir, "typescript", "package", "bin", "tsc")
        self.tsc_dir = os.path.join(self.build_dir, "typescript")
        self.tsc_repo_url = os.getenv(TSC_REPO_URL)
        if self.tsc_repo_url is None:
            Log.exception_and_raise(
                _LOGGER, "The TSC_REPO_URL environment variable does not exist.")
        self.process_typescript()

        test_suite_class = self.get_declgen_ets_class(self.__ets_suite_name)

        test_suite = test_suite_class(
            self.config, self.work_dir, self.default_list_root)
        test_suite.process(
            self.config.general.generate_only or self.config.ets.force_generate)
        self.test_root, self.list_root = test_suite.test_root, test_suite.list_root

        Log.summary(_LOGGER, f"TEST_ROOT set to {self.test_root}")
        Log.summary(_LOGGER, f"LIST_ROOT set to {self.list_root}")

        self.collect_excluded_test_lists(test_name=self.__ets_suite_name)
        self.collect_ignored_test_lists(test_name=self.__ets_suite_name)

        self.add_directories([TestDirectory(self.test_root, "sts", [])])

    @property
    def default_work_dir_root(self) -> Path:
        return Path("/tmp") / self.__ets_suite_dir / self.__ets_suite_name

    def create_test(self, test_file: str, flags: List[str], is_ignored: bool) -> TestDeclgenETS2TS:
        test = TestDeclgenETS2TS(self.test_env, test_file, flags, get_test_id(
            test_file, self.test_root), self.build_dir)
        test.ignored = is_ignored
        return test

    def run_command(self, command: list, cwd: str, description: str, timeout: int = 60 * 10) -> None:
        Log.summary(_LOGGER, f"Running command: {' '.join(command)}")
        Log.summary(_LOGGER, description)
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

    def read_package_info(self, package_json_path: str) -> str:
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

    def process_typescript(self) -> None:
        try:
            if os.path.exists(self.tsc_executor):
                return

            if os.path.exists(self.tsc_dir):
                shutil.rmtree(self.tsc_dir)

            self.run_command(["git", "clone", "--depth", "1", self.tsc_repo_url,
                             self.tsc_dir], self.build_dir, "Cloning repository...")

            self.run_command(["npm", "install"], self.tsc_dir,
                             "Running npm install...")

            self.run_command(["npm", "run", "build"],
                             self.tsc_dir, "Running npm run build...")

            self.run_command(["npm", "pack"], self.tsc_dir,
                             "Running npm pack...")

            package_name = self.read_package_info(
                os.path.join(self.tsc_dir, "package.json"))
            if package_name:
                Log.summary(_LOGGER, f"Extracting {package_name}...")
                shutil.unpack_archive(os.path.join(
                    self.tsc_dir, package_name), extract_dir=self.tsc_dir)

        except Exception as ex:  # pylint: disable=broad-except
            Log.exception_and_raise(_LOGGER, f"Error during the process: {ex}")

    def get_ets_suite_name(self, test_suites: Set[str]) -> str:
        name = ""
        if "declgenets2ts_ets_cts" in test_suites:
            name = DeclgenEtsSuites.CTS.value
        else:
            Log.exception_and_raise(
                _LOGGER, f"Unsupported test suite: {self.config.test_suites}")
        return name

    def get_ets_suite_dir(self, test_suites: Set[str]) -> str:
        suite_dir = ""
        if "declgenets2ts_ets_cts" in test_suites:
            suite_dir = DeclgenEtsSuitesDir.CTS.value
        else:
            Log.exception_and_raise(
                _LOGGER, f"Unsupported test suite: {self.config.test_suites}")
        return suite_dir

    def get_declgen_ets_class(self, declgen_ets_suite_name: str) -> Any:
        name_to_class = {
            DeclgenEtsSuites.CTS.value: DeclgenCtsEtsTestSuite
        }
        return name_to_class.get(declgen_ets_suite_name)
