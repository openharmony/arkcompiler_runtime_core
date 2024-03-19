#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
from os import path
from pathlib import Path
from typing import List

from runner.logger import Log
from runner.options.config import Config
from runner.plugins.parser.test_js_parser import TestJSParser
from runner.runner_base import get_test_id
from runner.runner_js import RunnerJS

_LOGGER = logging.getLogger("runner.plugins.parser.runner_js_parser")


class RunnerJSParser(RunnerJS):
    def __init__(self, config: Config) -> None:
        super().__init__(config, "parser")

        symlink_es2panda_test = Path(config.general.static_core_root) / "tools" / "es2panda" / "test"
        if symlink_es2panda_test.exists():
            es2panda_test = symlink_es2panda_test.resolve()
        else:
            es2panda_test = Path(config.general.static_core_root).parent.parent / 'ets_frontend' / 'ets2panda' / 'test'
            if not es2panda_test.exists():
                raise Exception(f'There is no path {es2panda_test}')
        self.default_list_root = es2panda_test / 'test-lists'

        if self.list_root:
            self.list_root = self.list_root
        else:
            self.list_root = path.normpath(path.join(self.default_list_root, self.name))
        Log.summary(_LOGGER, f"LIST_ROOT set to {self.list_root}")

        self.test_root = es2panda_test if self.test_root is None else self.test_root
        Log.summary(_LOGGER, f"TEST_ROOT set to {self.test_root}")

        self.collect_excluded_test_lists()
        self.collect_ignored_test_lists()

        if self.config.general.with_js:
            self.add_directory("ark_tests/parser/js", "js", flags=["--parse-only"])

        if self.config.general.with_js:
            self.add_directory("compiler/js", "js", flags=["--extension=js", "--output=/dev/null"])
        self.add_directory("compiler/ts", "ts", flags=["--extension=ts", ])
        self.add_directory("compiler/ets", "ets", flags=[
            "--extension=ets",
            "--output=/dev/null",
            f"--arktsconfig={self.arktsconfig}"
        ])

        if self.config.general.with_js:
            self.add_directory("parser/js", "js", flags=["--parse-only"])
        self.add_directory("parser/ts", "ts", flags=["--parse-only", '--extension=ts'])
        self.add_directory("parser/as", "ts", flags=["--parse-only", "--extension=as"])
        self.add_directory("parser/ets", "ets", flags=[
            "--extension=ets",
            "--output=/dev/null",
            f"--arktsconfig={self.arktsconfig}"
        ])

    def add_directory(self, directory: str, extension: str, flags: List[str]) -> None:
        new_dir = path.normpath(path.join(self.test_root, directory))
        super().add_directory(new_dir, extension, flags)

    def create_test(self, test_file: str, flags: List[str], is_ignored: bool) -> TestJSParser:
        test = TestJSParser(self.test_env, test_file, flags, get_test_id(test_file, self.test_root))
        test.ignored = is_ignored
        return test

    @property
    def default_work_dir_root(self) -> Path:
        return Path("/tmp") / "parser"
