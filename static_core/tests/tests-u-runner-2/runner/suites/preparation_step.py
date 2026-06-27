#!/usr/bin/env python3
# -- coding: utf-8 --
#
# Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

import multiprocessing
import re
import shutil
import subprocess
from abc import ABC, abstractmethod
from collections.abc import Generator
from concurrent.futures import ThreadPoolExecutor
from functools import cached_property
from pathlib import Path

from runner.common_exceptions import InvalidConfiguration, TestGenerationException, TimeoutException, UnknownException
from runner.extensions.generators.igenerator import IGenerator
from runner.logger import Log
from runner.options.config import Config
from runner.options.options_collections import CollectionsOptions
from runner.suites.test_metadata import TestMetadata
from runner.utils import UiUpdater, copy, get_class_by_name, read_file, write_2_file

_LOGGER = Log.get_logger(__file__)
EXPECTED_EXT = ".expected"


class TestPreparationStep(ABC):
    def __init__(self, test_source_path: Path, *, test_gen_path: Path, config: Config, collection: CollectionsOptions,
                 extension: str = "ets") -> None:
        self.__test_source_path = test_source_path.resolve()
        self.__test_gen_path = test_gen_path.resolve()

        self.config = config
        self.extension = extension
        self.collection = collection

    @property
    def test_source_path(self) -> Path:
        return self.__test_source_path

    @property
    def test_gen_path(self) -> Path:
        return self.__test_gen_path

    @staticmethod
    def get_expected_files_for_test(test_path: Path) -> set[Path]:
        parent = test_path.parent
        pattern = f"{test_path.stem}*{EXPECTED_EXT}*"
        return set(parent.glob(pattern))

    @staticmethod
    def get_d_ets_files(test_path: Path) -> set[Path]:
        d_ets_files = set()
        for file_name in test_path.parent.iterdir():
            if file_name.is_file() and file_name.suffixes[-2:] == [".d", ".ets"]:
                d_ets_files.add(file_name)
        return d_ets_files

    @cached_property
    def test_gen_root(self) -> Path:
        return self._get_root_from_path(self.test_gen_path)

    @cached_property
    def test_source_root(self) -> Path:
        return self._get_root_from_path(self.test_source_path)

    def get_metadata_test_files(self, test_path: Path) -> set[Path]:
        seen: set[Path] = set()
        result: set[Path] = set()

        self._collect_deps(test_path, seen, result)
        return result

    @abstractmethod
    def transform(self, force_generated: bool) -> Generator[Path, None, None]:
        pass

    def _get_root_from_path(self, path: Path) -> Path:
        base = path
        if base.is_file():
            base = base.parent
        rel = Path(self.collection.name)
        if rel.suffix:
            rel = rel.parent

        parts = [part for part in rel.parts if part not in (".", "")]

        return base.parents[len(parts) - 1] if parts else base

    def _collect_deps(self, test_path: Path, seen: set[Path], result: set[Path]) -> None:
        """
        Recursively collect all dependent files for a given test file.
        """

        if test_path in seen:
            return

        seen.add(test_path)

        if not test_path.is_file():
            return

        if test_path.suffixes[-2:] == ['.d', '.ets']:
            return

        test_metadata = TestMetadata.get_metadata(test_path)
        for dep_file in (test_metadata.files or []):
            if self.test_source_path.is_file():
                full_dep_path = (self.test_source_path.parent / Path(dep_file)).resolve()
            else:
                full_dep_path = (self.test_source_path / Path(dep_file)).resolve()
            result.add(full_dep_path)

            if full_dep_path.is_file() and full_dep_path.suffix == f".{self.extension}":
                self._collect_deps(full_dep_path, seen, result)


class CustomGeneratorTestPreparationStep(TestPreparationStep):
    def __init__(self, test_source_path: Path, *, test_gen_path: Path, config: Config, collection: CollectionsOptions,
                 extension: str) -> None:
        super().__init__(
            test_source_path=test_source_path,
            test_gen_path=test_gen_path,
            config=config,
            collection=collection,
            extension=extension)

    def __str__(self) -> str:
        return f"Test Generator for {self.config.test_suite.suite_name}' test suite"

    @staticmethod
    def __get_generator_class(clazz: str) -> type[IGenerator]:
        class_obj = get_class_by_name(clazz)
        if not issubclass(class_obj, IGenerator):
            raise InvalidConfiguration(
                f"Generator class '{clazz}' not found. "
                f"Check value of 'generator-class' option")
        return class_obj

    def transform(self, force_generated: bool) -> Generator[Path, None, None]:
        # call of the custom generator
        if self.test_gen_path.exists() and force_generated:
            shutil.rmtree(self.test_gen_path)
        self.test_gen_path.mkdir(exist_ok=True, parents=True)
        try:
            if self.collection.generator_class is not None:
                self.__generate_by_class(self.collection.generator_class)
            elif self.collection.generator_script is not None:
                self.__generate_by_script(self.collection.generator_script)
        finally:
            yield from self.test_gen_path.rglob(f"**/*.{self.extension}")

    def __generate_by_class(self, clazz: str) -> None:
        generator_class: type[IGenerator] = self.__get_generator_class(clazz)
        ets_templates_generator = generator_class(self.test_source_path, self.test_gen_path, self.config)
        with ThreadPoolExecutor(max_workers=1) as executor:
            future = executor.submit(ets_templates_generator.generate)
            if self.config.general.show_progress:
                UiUpdater("Tests are generating").start(future)
            if exception := future.exception():
                raise TestGenerationException(exception.args) from exception

    def __generate_by_script(self, script: str) -> None:
        cmd: list[str | Path] = [
            script,
            "--source", self.test_source_path,
            "--target", self.test_gen_path
        ]
        cmd.extend(self.collection.generator_options)
        timeout = 300
        with subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                encoding='utf-8',
                errors='ignore',
        ) as process:
            try:
                process.communicate(timeout=timeout)
            except subprocess.TimeoutExpired as exc:
                process.kill()
                raise TimeoutException(f"Generation failed by timeout after {timeout} sec") from exc
            except subprocess.SubprocessError as exc:
                raise UnknownException(f"Generation failed by unknown reason: {exc}") from exc


class CopyStep(TestPreparationStep):
    def __str__(self) -> str:
        return (f"Test preparation step: copying *.{self.extension} files "
                f"from {self.test_source_path} to {self.test_gen_path}")

    def transform(self, force_generated: bool) -> Generator[Path, None, None]:
        try:
            if self.test_source_path != self.test_gen_path:
                copy(self.test_source_path, self.test_gen_path, remove_if_exist=False)
                if self.test_source_path.is_file():
                    dep_files = self.get_metadata_test_files(self.test_source_path)
                    expected_files = self.get_expected_files_for_test(self.test_source_path)
                    d_ets = self.get_d_ets_files(self.test_source_path)
                    for dep_file in dep_files | expected_files | d_ets:
                        rel_path = dep_file.parent.relative_to(self.test_source_root)
                        dep_file_dest = self.test_gen_root / rel_path / dep_file.name
                        copy(dep_file, dep_file_dest, remove_if_exist=False)
            if self.test_gen_path.is_file():
                yield from self.test_gen_path.parent.rglob(f"**/*.{self.extension}")
            else:
                yield from self.test_gen_path.rglob(f"**/*.{self.extension}")
        except FileNotFoundError:
            yield from []


class JitStep(TestPreparationStep):
    __main_pattern = r"\bfunction\s+(?P<main>main)\b"
    __param_pattern = r"\s*\(\s*(?P<param>(?P<param_name>\w+)(\s*:\s*\w+))?\s*\)"
    __return_pattern = r"\s*(:\s*(?P<return_type>\w+)\b)?"
    __throws_pattern = r"\s*(?P<throws>throws)?"
    __indent = "    "

    def __init__(self, test_source_path: Path, *, test_gen_path: Path, config: Config,
                 collection: CollectionsOptions,
                 extension: str = "ets", num_repeats: int = 0):
        super().__init__(
            test_source_path=test_source_path,
            test_gen_path=test_gen_path,
            config=config,
            collection=collection,
            extension=extension)
        self.num_repeats = num_repeats
        self.main_regexp = re.compile(
            f"{self.__main_pattern}{self.__param_pattern}{self.__return_pattern}{self.__throws_pattern}",
            re.MULTILINE
        )

    def __str__(self) -> str:
        return "Test preparation step for any ets test suite: transforming for JIT testing"

    def transform(self, force_generated: bool) -> Generator[Path, None, None]:
        tests = self.test_gen_path.rglob(f"**/*.{self.extension}")
        with multiprocessing.Pool(processes=self.config.general.processes) as pool:
            run_tests = pool.imap_unordered(self.jit_transform_one_test, tests, chunksize=self.config.general.chunksize)
            pool.close()
            pool.join()

        yield from run_tests

    def jit_transform_one_test(self, test_path: Path) -> Path:
        metadata = TestMetadata.get_metadata(test_path)
        is_convert = not metadata.tags.not_a_test and \
                     not metadata.tags.compile_only and \
                     not metadata.tags.no_warmup
        if not is_convert:
            return test_path

        original = read_file(test_path)
        match = self.main_regexp.search(original)
        if match is None:
            return test_path

        is_int_main = match.group("return_type") == "int"
        return_type = ": int" if is_int_main else ""
        throws = "throws " if match.group("throws") else ""
        param_line = param if (param := match.group("param")) is not None else ""
        param_name = param if (param := match.group("param_name")) is not None else ""

        tail = [f"\nfunction main({param_line}){return_type} {throws}{{"]
        if is_int_main:
            tail.append(f"{self.__indent}let result = 0")
        tail.append(f"{self.__indent}for(let i = 0; i < {self.num_repeats}; i++) {{")
        if is_int_main:
            tail.append(f"{self.__indent * 2}result += main_run({param_name})")
        else:
            tail.append(f"{self.__indent * 2}main_run({param_name})")
        tail.append(f"{self.__indent}}}")
        if is_int_main:
            tail.append(f"{self.__indent}return result;")
        tail.append("}")

        result = self.main_regexp.sub(lambda arg: arg.group(0).replace("main", "main_run"), original)
        result += "\n".join(tail)
        write_2_file(test_path, result)
        return test_path
