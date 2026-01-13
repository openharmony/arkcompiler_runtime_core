#!/usr/bin/env python3
# -- coding: utf-8 --
#
# Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
import fnmatch
import os
import shutil
from pathlib import Path

from runner.extensions.generators.ets_cts.benchmark import Benchmark
from runner.extensions.generators.igenerator import IGenerator
from runner.logger import Log
from runner.options.config import Config
from runner.suites.test_metadata import TestMetadata

_LOGGER = Log.get_logger(__file__)

EXPECTED_FILE_EXTENSION = ".expected"


class EtsTemplatesGenerator(IGenerator):
    def __init__(self, source: Path, target: Path, config: Config) -> None:
        super().__init__(source, target, config)
        self.extension = f".{self._config.test_suite.extension()}"
        self.filter: str = str(self._config.test_suite.get_parameter("filter"))
        self.test_file = self._config.test_suite.get_parameter("test-file")
        self._already_generated: set[Path] = set()

    def generate(self) -> list[str]:
        _LOGGER.all("Starting generate test")
        if self._target.exists():
            shutil.rmtree(self._target)
        test_source = self._source
        if self.test_file and fnmatch.fnmatchcase(self.test_file, self.filter):
            test_source = self._source / self.test_file
        self.__dfs(test_source, self._already_generated)

        _LOGGER.all("Generation finished!")
        return self.generated_tests

    def __generate_test(self, path: Path) -> None:
        test_full_name = os.path.relpath(path, self._source)
        output = self._target / test_full_name
        bench = Benchmark(path, output, test_full_name, self._source, self.extension)
        generated_tests = bench.generate()
        dep_tests_to_generate: set[Path] = set()
        for test in generated_tests:
            test_metadata = TestMetadata.get_metadata(Path(test))
            if test_metadata.files:
                dep_tests_to_generate |= {Path(test) for test in test_metadata.files}

        self.generated_tests.extend(generated_tests)
        dep_tests_to_generate = {(path.parent / Path(dep_test)).resolve() for dep_test in dep_tests_to_generate}
        if dep_tests_to_generate:
            for dep_test in dep_tests_to_generate:
                dep_test_output = self._target / dep_test.relative_to(self._source)
                if not dep_test_output.exists():
                    self.__generate_test(dep_test)
                    self._already_generated.add(dep_test)

    def __dfs(self, path: Path, seen: set) -> None:
        if (not path.exists() and not self.test_file) or path in seen:
            return
        if not path.exists() and self.test_file:
            # test_file might have postfix from template, should check path without it
            test_file_name, _, _ = path.name.rpartition("_")
            path = path.parent / f"{test_file_name}{self.extension}"
            if not path.exists():
                return

        seen.add(path)

        if path.is_dir():
            for i in sorted(path.iterdir()):
                self.__dfs(i, seen)
        elif path.suffix == self.extension and fnmatch.fnmatchcase(str(path), self.filter):
            self.__generate_test(path)

        elif EXPECTED_FILE_EXTENSION in path.suffixes and fnmatch.fnmatchcase(str(path), self.filter):
            self._copy_expected_file(path)

    def _copy_expected_file(self, path: Path) -> None:
        test_full_name = path.relative_to(self._source)
        output = self._target / test_full_name
        shutil.copy(path, output.parent)
