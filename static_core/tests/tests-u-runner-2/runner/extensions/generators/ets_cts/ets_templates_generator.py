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
import glob
import os
import shutil
from functools import cached_property
from pathlib import Path

from runner.extensions.generators.ets_cts.benchmark import Benchmark
from runner.extensions.generators.igenerator import IGenerator
from runner.logger import Log
from runner.options.config import Config
from runner.suites.test_metadata import TestMetadata

_LOGGER = Log.get_logger(__file__)

EXPECTED_FILE_EXTENSION = ".expected"
DEFAULT_FILTER = "*"


class EtsTemplatesGenerator(IGenerator):
    def __init__(self, source: Path, target: Path, config: Config) -> None:
        super().__init__(source, target, config)
        self.extension = f".{self._config.test_suite.extension()}"
        self.filter: str = str(self._config.test_suite.get_parameter("filter"))
        self.test_file = self._config.test_suite.get_parameter("test-file")
        self._generated_deps: set[Path] = set()

    @staticmethod
    def get_expected_files_for_test(test_path: Path) -> list[Path]:
        test_parent_dir = test_path.parent
        test_expected = []
        for file_name in test_parent_dir.iterdir():
            if test_path.stem in str(file_name) and EXPECTED_FILE_EXTENSION in str(file_name):
                test_expected.append(file_name)
        return test_expected

    @staticmethod
    def _get_test_dep_files(parent_test: Path, generated_tests: list[str]) -> set[Path]:
        dep_files: set[Path] = set()
        for test in generated_tests:
            test_metadata = TestMetadata.get_metadata(Path(test))
            if test_metadata.files:
                dep_files |= {(parent_test.parent / Path(dep)).resolve() for dep in test_metadata.files}

        return dep_files

    @cached_property
    def _matched(self) -> set[Path]:
        return self._get_matched_paths()

    def generate(self) -> list[str]:
        _LOGGER.all("Starting generate test")
        if self._target.exists():
            shutil.rmtree(self._target)
        test_source = self._source
        if self.test_file and fnmatch.fnmatchcase(self.test_file, self.filter):
            test_source = self._source / self.test_file
        self.__dfs(test_source, set())

        _LOGGER.default(f"Generation finished! Generated {len(self.generated_tests)} test files")
        return self.generated_tests

    def _get_matched_paths(self) -> set[Path]:
        matched: set[Path] = set()
        test_root = Path(self._source)
        flt = self.filter

        patterns = [flt, f"{flt}/**/*"]

        for pattern in patterns:
            for rel in glob.iglob(pattern, root_dir=str(test_root), recursive=True):
                p = test_root / rel
                if p.is_file() and str(p).endswith(self.extension):
                    matched.add(p)
        return matched

    def _copy_expected_file(self, path: Path) -> None:
        test_expected_files = EtsTemplatesGenerator.get_expected_files_for_test(path)
        if test_expected_files:
            for expected_file in test_expected_files:
                test_full_name = expected_file.relative_to(self._source)
                output = self._target / test_full_name
                shutil.copy(expected_file, output.parent)

    def _strip_test_template_suffix(self, path: Path) -> Path | None:
        if path.exists():
            return path

        # test_file might have postfix from template, should find parent file
        base, _, _ = path.name.rpartition("_")
        parent_test = path.parent / f"{base}{self.extension}"
        return parent_test if parent_test.exists() else None

    def _collect_d_ets(self, test_full_name: str, seen_d_ets: set[Path]) -> tuple[set[Path], set[Path]]:
        new_d_ets_files = set()
        test_parent_dir = self._source / Path(test_full_name).parent
        for file_name in test_parent_dir.iterdir():
            if self._is_d_ets_file(file_name) and file_name not in seen_d_ets:
                new_d_ets_files.add(Path(file_name.name))
                seen_d_ets.add(file_name)

        return new_d_ets_files, seen_d_ets

    def _is_d_ets_file(self, file_path: Path) -> bool:
        return file_path.is_file() and file_path.suffixes[-2:] == [".d", ".ets"]

    def _should_generate_dependency(self, dep_test: Path) -> bool:
        if dep_test in self._generated_deps or dep_test in self.generated_tests:
            return False

        # Check if test was already generated
        dep_relative = dep_test.relative_to(self._source)
        generated_relative = [
            Path(gen_test).relative_to(self._target)
            for gen_test in self.generated_tests
        ]

        return dep_relative not in generated_relative

    def _collect_dependency_tests(self, path: Path, generated_tests: list,
                                  d_ets_files: set[Path]) -> set[Path]:
        dep_tests = set(d_ets_files)

        dep_files = EtsTemplatesGenerator._get_test_dep_files(path, generated_tests)
        for test_path in dep_files:
            strip_test_path = self._strip_test_template_suffix(test_path)
            if strip_test_path is not None:
                dep_tests.add(Path(strip_test_path))

        return {(path.parent / dep_test).resolve() for dep_test in dep_tests}

    def __generate_test(self, path: Path, seen_d_ets: set) -> None:
        if path in self._generated_deps:
            return
        test_full_name = os.path.relpath(path, self._source)
        output = self._target / test_full_name

        bench = Benchmark(path, output, test_full_name, self._source, self.extension)
        generated_tests = bench.generate()
        if len(generated_tests) > 1:
            self._generated_deps.add(path)
        self.generated_tests.extend(generated_tests)

        self._copy_expected_file(path)

        if self.test_file is not None or self.filter != DEFAULT_FILTER:
            d_ets_files, seen_d_ets = self._collect_d_ets(test_full_name, seen_d_ets)

            dep_tests = self._collect_dependency_tests(path, generated_tests, d_ets_files)

            for dep_test in dep_tests:
                if self._should_generate_dependency(dep_test):
                    self.__generate_test(dep_test, seen_d_ets)
                    self._generated_deps.add(dep_test)

    def __dfs(self, test_path: Path, seen: set[Path]) -> None:
        path = self._strip_test_template_suffix(test_path)
        if path is None or path in seen:
            return

        seen.add(path)

        if path.is_dir():
            for child in sorted(path.iterdir()):
                self.__dfs(child, seen)
            return

        if path not in self._matched:
            return

        if str(path).endswith(self.extension):
            self.__generate_test(path, set())

            return
