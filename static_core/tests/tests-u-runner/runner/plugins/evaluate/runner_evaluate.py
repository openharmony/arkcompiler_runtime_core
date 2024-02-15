#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024 Huawei Device Co., Ltd.
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
from pathlib import Path
from typing import List

from runner.logger import Log
from runner.options.config import Config
from runner.plugins.evaluate.test_evaluate import TestEvaluate
from runner.runner_base import get_test_id
from runner.runner_js import RunnerJS

_LOGGER = logging.getLogger("runner.plugins.evaluate.runner_evaluate")


class RunnerEvaluate(RunnerJS):
    def __init__(self, config: Config):
        self.__ets_suite_name = "evaluate"
        if self.__ets_suite_name not in config.test_suites:
            Log.exception_and_raise(_LOGGER, f"Unsupported test suite: {config.test_suites}")

        super().__init__(config, self.__ets_suite_name)

        self.test_env.es2panda_args.extend([
            f"--arktsconfig={self.arktsconfig}",
            "--gen-stdlib=false",
            "--extension=ets",
            "--debug-info",
            f"--opt-level={self.config.es2panda.opt_level}"
        ])

        es2panda_test = Path(config.general.static_core_root) / "plugins" / "ets" / "tests"
        if not es2panda_test.exists():
            raise Exception(f"There is no path {es2panda_test}")
        self.default_list_root = es2panda_test / "test-lists"

        self.list_root = self.list_root if self.list_root else Path(self.default_list_root).joinpath(self.name)
        Log.summary(_LOGGER, f"LIST_ROOT set to {self.list_root}")

        self.test_root = es2panda_test if self.test_root is None else self.test_root
        self.test_root = self.test_root / "evaluate/ets"
        Log.summary(_LOGGER, f"TEST_ROOT set to {self.test_root}")

        self.collect_excluded_test_lists()
        self.collect_ignored_test_lists()

        tests_root_dir = Path(self.test_root)
        for child in tests_root_dir.iterdir():
            if child.is_dir():
                self.add_directory(str(child), "base.ets", flags=[])

    def create_test(self, test_file: str, flags: List[str], is_ignored: bool) -> TestEvaluate:
        test = TestEvaluate(self.test_env, test_file, flags, get_test_id(test_file, self.test_root))
        test.ignored = is_ignored
        return test

    @property
    def default_work_dir_root(self) -> Path:
        return Path("/tmp") / "evaluate"
