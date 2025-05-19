#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
from typing import Set, List

from runner.extensions.generators.ets_cts.benchmark import Benchmark
from runner.extensions.generators.igenerator import IGenerator
from runner.logger import Log
from runner.options.config import Config

_LOGGER = Log.get_logger(__file__)


class EtsTemplatesGenerator(IGenerator):
    def __init__(self, source: Path, target: Path, config: Config) -> None:
        super().__init__(source, target, config)
        self.extension = f".{self._config.test_suite.extension()}"

    def generate(self) -> List[str]:
        _LOGGER.all("Starting generate test")
        if self._target.exists():
            shutil.rmtree(self._target)
        self.__dfs(self._source, set())
        _LOGGER.all("Generation finished!")
        return self.generated_tests

    def __generate_test(self, path: Path) -> None:
        test_full_name = os.path.relpath(path, self._source)
        output = self._target / test_full_name
        bench = Benchmark(path, output, test_full_name, self.extension, self.extension)
        self.generated_tests.extend(bench.generate())

    def __dfs(self, path: Path, seen: Set) -> None:
        if not path.exists() or path in seen:
            return
        seen.add(path)

        if path.is_dir():
            for i in sorted(path.iterdir()):
                self.__dfs(i, seen)
        elif path.suffix == self.extension:
            self.__generate_test(path)
