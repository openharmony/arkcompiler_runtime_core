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
#
import fnmatch
import os
import re
import shutil
import subprocess
from collections import Counter
from dataclasses import replace
from functools import cached_property
from glob import glob
from pathlib import Path
from typing import ClassVar, cast
from zipfile import ZipFile

from runner.chapters import Chapters
from runner.common_exceptions import (
    FileNotFoundException,
    InvalidConfiguration,
    TestNotExistException,
)
from runner.enum_types.qemu import QemuKind
from runner.extensions.flows.itest_flow import ITestFlow
from runner.extensions.flows.test_flow_registry import workflow_registry
from runner.extensions.suites.test_suite_registry import ITestSuite
from runner.logger import Log
from runner.options.options import IOptions
from runner.options.options_collections import CollectionsOptions
from runner.options.options_step import StepKind
from runner.options.root_dir import RootDir
from runner.runner_types.test_env import TestEnv
from runner.suites.gtest_file import GTestFile
from runner.suites.preparation_step import CopyStep, CustomGeneratorTestPreparationStep, JitStep, TestPreparationStep
from runner.suites.step_utils import StepUtils
from runner.suites.test_lists import TestLists
from runner.utils import (
    ani_name_rule,
    correct_path,
    get_group_number,
    get_test_id,
    is_executable_file,
    make_gtest_name,
    pretty_divider,
)

_LOGGER = Log.get_logger(__file__)
DEFAULT_EXTENSION = "ets"


class TestSuite(ITestSuite):
    CONST_COMMENT: ClassVar[str] = "#"
    TEST_COMMENT_EXPR = re.compile(r"^\s*(?P<test>[^# ]+)?(\s*#\s*(?P<comment>.+))?", re.MULTILINE)

    def __init__(self, test_env: TestEnv) -> None:
        self._test_env = test_env
        self.__suite_name = test_env.config.test_suite.suite_name
        self.__work_dir = test_env.work_dir
        self._list_roots: list[RootDir] = test_env.config.test_suite.list_roots

        self.config = test_env.config

        self.jit_repeats: int = int(repeats) \
            if (repeats := self.config.workflow.get_parameter("num-repeats")) is not None else 0
        self._preparation_steps: list[TestPreparationStep] = self.set_preparation_steps()
        self._collections_parameters = self.__get_collections_parameters()

        self._test_lists = TestLists(self._list_roots, self._test_env, self.jit_repeats)
        self._explicit_file: Path | None = None
        self._explicit_list: Path | None = self._test_lists.resolve_explicit_list()
        self._test_lists.collect_excluded_test_lists()
        self._test_lists.collect_ignored_test_lists()
        self.excluded = 0
        self.ignored_tests: list[Path] = []
        self.ignored_tests_with_failures: dict[Path, str] = {}
        self.excluded_tests: list[Path] = []
        self.workflow_kind = self.config.workflow.workflow_type
        self.supported_extensions: list[str] = list({
            self.config.test_suite.extension(collection)
            for collection in self.config.test_suite.collections
        })

    @property
    def test_root(self) -> Path:
        return self.__work_dir.gen

    @property
    def extension(self) -> str:
        return cast(str, self.config.test_suite.get_parameter("extension", DEFAULT_EXTENSION))

    @property
    def explicit_list(self) -> Path | None:
        return self._explicit_list

    @staticmethod
    def __load_list(test_root: Path, test_list_path: Path, prefixes: list[str]) -> tuple[list[Path], list[Path]]:
        result: list[Path] = []
        not_found: list[Path] = []
        if not test_list_path.exists():
            return result, not_found

        with os.fdopen(os.open(test_list_path, os.O_RDONLY, 0o755), 'r', encoding="utf-8") as file_handler:
            for line in file_handler:
                is_found, test_path = TestSuite.__load_line(line, test_root, prefixes)
                if is_found and test_path is not None:
                    result.append(test_path)
                elif not is_found and test_path is not None:
                    not_found.append(test_path)
        return result, not_found

    @staticmethod
    def __load_line(line: str, test_root: Path, prefixes: list[str]) -> tuple[bool, Path | None]:
        test, _ = TestSuite._get_test_and_comment_from_line(line.strip(" \n"))
        if test is None:
            return False, None
        test_path = test_root / test
        if test_path.exists():
            return True, test_path
        is_found, prefixed_test_path = TestSuite.__load_line_with_prefix(test_root, prefixes, test)
        if prefixed_test_path is not None:
            return is_found, prefixed_test_path
        return False, test_path

    @staticmethod
    def __load_line_with_prefix(test_root: Path, prefixes: list[str], test: str) -> tuple[bool, Path | None]:
        for prefix in prefixes:
            test_path = test_root / prefix
            if test_path.is_file() and test_path.name == test:
                return True, test_path
            if test_path.is_dir():
                test_path = test_path / test
                if test_path.exists():
                    return True, test_path
        return False, None

    @staticmethod
    def _get_test_and_comment_from_line(line: str) -> tuple[str | None, str | None]:
        line_parts = TestSuite.TEST_COMMENT_EXPR.search(line)
        if line_parts:
            return line_parts["test"], line_parts["comment"]
        return None, None

    @staticmethod
    def _search_duplicates(original: list[Path], kind: str) -> None:
        main_counter = Counter([str(test) for test in original])
        dupes = [test for test, frequency in main_counter.items() if frequency > 1]
        if len(dupes) > 0:
            _LOGGER.short(f"There are {len(dupes)} duplicates in {kind} lists. "
                          "To see the full list run with the option `--verbose=all`")
            for test in dupes:
                _LOGGER.all(f"\t{test}")
        elif len(original) > 0:
            _LOGGER.short(f"No duplicates found in {kind} lists.")

    @staticmethod
    def __is_path_excluded(collection: CollectionsOptions, tested_path: Path) -> bool:
        excluded = [excl for excl in collection.exclude if tested_path.as_posix().endswith(excl)]
        return len(excluded) > 0

    @staticmethod
    def __matches_filter(rel: Path, matched_by_filter: set[str] | None) -> bool:
        """
        Applies glob filter result to a relative path.
        If matched_by_filter is None, filter is treated as default.
        """
        if matched_by_filter is None:
            return True
        return rel.as_posix() in matched_by_filter

    @staticmethod
    def _comment_has_failure(comment: str | None) -> bool:
        return bool(TestSuite._get_failure_text(comment))

    @staticmethod
    def _get_failure_text(comment: str | None) -> str:
        pattern = r'^#\d+\s+.*?@@Failure:\s*(.+?)@@.*$'
        if comment is not None:
            match = re.match(pattern, comment)
            return match.group(1) if match else ""
        return ""

    @cached_property
    def explicit_file(self) -> Path | None:
        return self._explicit_file

    @cached_property
    def list_roots(self) -> list[RootDir]:
        return self._list_roots

    @cached_property
    def name(self) -> str:
        return self.__suite_name

    def process(self, force_generate: bool) -> list[ITestFlow]:
        """
        Main processing method that orchestrates the entire test suite workflow.

        This method follows a systematic pipeline to prepare, filter, and create test objects:
        1. Prepares test files (generates new ones or reuses existing)
        2. Resolves explicit test file if specified
        3. Filters test files based on execution criteria (excluded/ignored lists, filters, chapters) and
        by group number if group filtering is enabled
        4. Creates test objects from the filtered test files

        :param force_generate: if True, forces regeneration of test files even if they exist.
                              If False, reuses existing test files when available.
        :return: list of ITestFlow objects ready for execution
        :raises InvalidConfiguration: if no tests are loaded for execution
        """
        prepared_test_files = self.__prepare_test_files(force_generate)
        self._explicit_file = self._get_explicit_file(self._test_env.config.test_suite.test_lists.explicit_file)
        filtered_test_files = self._resolve_test_files(prepared_test_files)
        tests = self._create_tests(filtered_test_files)
        _LOGGER.short(f"Loaded {len(tests)} test files from the folder '{self.test_root}'. "
                      f"Excluded {self.excluded} tests are not loaded.")
        return tests

    def set_preparation_steps(self) -> list[TestPreparationStep]:
        steps: list[TestPreparationStep] = []
        for collection in self.config.test_suite.collections:
            if collection.generator_script is not None or collection.generator_class is not None:
                steps.append(CustomGeneratorTestPreparationStep(
                    test_source_path=collection.test_root / collection.name,
                    test_gen_path=self.test_root / collection.name,
                    config=self.config,
                    collection=collection,
                    extension=self.config.test_suite.extension(collection)
                ))
                copy_source_path = self.test_root / collection.name
                real_source_path = collection.test_root / collection.name
            else:
                copy_source_path = collection.test_root / collection.name
                real_source_path = copy_source_path
            if not real_source_path.exists():
                error = (f"Source path '{real_source_path}' does not exist! "
                         f"Cannot process the collection '{collection.name}'")
                _LOGGER.default(error)
                continue
            extension = self.config.test_suite.extension(collection)
            with_js = self.config.test_suite.with_js(collection)
            if (extension == "js" and with_js) or extension != "js":
                steps.extend(self.__add_copy_steps(collection, copy_source_path, extension))
            if self.jit_repeats > 0:
                steps.extend(self.__add_jit_steps(collection, copy_source_path, extension))
        return steps

    def _locate_chapters_file(self) -> Path:
        """
        Searches for chapters file (by default it is chapters.yaml) in all existing list roots
        First found file path is returned
        If neither list root contains the chapters file the FileNotFoundException raises
        """
        for root in self.list_roots:
            file_path = correct_path(root.root_dir, self.config.test_suite.groups.chapters_file)
            if Path(file_path).is_file():
                return file_path
        raise FileNotFoundException(
            f"Not found '{self.config.test_suite.groups.chapters_file}' in any list root "
            f"'{self.list_roots!s}'")

    def __add_copy_steps(self, collection: CollectionsOptions, copy_source_path: Path, extension: str) \
            -> list[TestPreparationStep]:
        steps: list[TestPreparationStep] = []
        if collection.exclude:
            for file_path in copy_source_path.iterdir():
                if self.__is_path_excluded(collection, file_path):
                    continue
                steps.append(CopyStep(
                    test_source_path=file_path,
                    test_gen_path=self.test_root / collection.name / file_path.name,
                    config=self.config,
                    collection=collection,
                    extension=extension
                ))
        elif copy_source_path != self.test_root / collection.name:
            steps.append(CopyStep(
                test_source_path=copy_source_path,
                test_gen_path=self.test_root / collection.name,
                config=self.config,
                collection=collection,
                extension=extension
            ))
        return steps

    def __add_jit_steps(self, collection: CollectionsOptions, copy_source_path: Path, extension: str) \
            -> list[TestPreparationStep]:
        steps: list[TestPreparationStep] = []
        if collection.exclude:
            for file_path in copy_source_path.iterdir():
                if self.__is_path_excluded(collection, file_path):
                    continue
                steps.append(JitStep(
                    test_source_path=file_path,
                    test_gen_path=self.test_root / collection.name / file_path.name,
                    config=self.config,
                    collection=collection,
                    extension=extension,
                    num_repeats=self.jit_repeats
                ))
        else:
            steps.append(JitStep(
                test_source_path=copy_source_path,
                test_gen_path=self.test_root / collection.name,
                config=self.config,
                collection=collection,
                extension=extension,
                num_repeats=self.jit_repeats
            ))
        return steps

    def __prepare_test_files(self, force_generate: bool) -> list[Path]:
        """
        Prepares test files by either generating them according to preparation_steps or loading existing ones
        :param force_generate: if True and the test files exist, it forces the test files' regeneration.
            If False then the existing test files are reused.
            If no test files exist, the generation occurs anyway.
        :return: list of prepared test file paths (unique, no duplicates)
        Note: Handles both generation of new test files and reuse of existing generated files
        """
        util = StepUtils()
        tests: list[Path] = []
        if not force_generate and util.are_tests_generated(self.test_root):
            _LOGGER.all(f"Reused earlier generated tests from {self.test_root}")
            return self.__load_generated_test_files()

        if self.test_root.exists():
            _LOGGER.all(f"INFO: {self.test_root.absolute()!s} already exist. WILL BE CLEANED")
            shutil.rmtree(self.test_root)

        for step in self._preparation_steps:
            tests.extend(step.transform(force_generate))
        tests = list(set(tests))

        if len(tests) == 0:
            message = "No tests loaded for execution"
            _LOGGER.default(message)
            raise InvalidConfiguration(message)

        util.create_report(self.test_root, tests)

        return tests

    def __load_generated_test_files(self) -> list[Path]:
        """
        Searches starting from `test_root` and collects all existing files with specified extension.
        Collections are taken into account.
        :return: list of unique found files
        """
        tests: list[Path] = []
        for collection in self.config.test_suite.collections:
            extension = self.config.test_suite.extension(collection)
            glob_expression = (self.test_root / collection.name).rglob(f"**/*.{extension}")
            tests.extend(glob_expression)
        return [Path(test) for test in set(tests)]

    def __get_explicit_test_path(self, test_id: str) -> Path | None:
        for collection in self.config.test_suite.collections:
            if test_id.startswith(collection.name):
                test_path: Path = correct_path(self.test_root, test_id)
                if test_path.exists():
                    return test_path
                break
            new_test_id = f"{collection.name}/{test_id}"
            test_path = correct_path(self.test_root, new_test_id)
            if test_path and test_path.exists():
                return test_path
        return None

    def __check_test_in_coll(self, rel: Path, collections: list[Path]) -> bool:
        """
        Checks whether test path belongs to at least one collection.
        If no collections specified, returns True.
        """
        if not collections:
            return True

        for col in collections:
            # collection points to a specific test file
            if (self.config.test_suite.test_root / col.name).is_file():
                if rel == col:
                    return True
                continue

            # collection points to a directory
            if rel == col or rel.is_relative_to(col):
                return True

        return False

    def _resolve_test_files(self, original_test_files: list[Path]) -> list[Path]:
        """
        Resolves the final list of test files by processing explicit selections or applying comprehensive filtering.

        :param original_test_files: full list of files with the specified extension that exist under the test_root
        :return: resolved list of test files ready for test object creation
        """
        _LOGGER.short(f"Loading tests from the directory {self.test_root}")

        test_files = self._get_explicit_test_files() or self._get_discovered_test_files(original_test_files)
        return self._filter_by_group(test_files)

    def _get_explicit_test_files(self) -> list[Path] | None:
        """Returns test files from explicit selection (file or list), or None if no explicit selection."""
        if self.explicit_file is not None:
            return [self.explicit_file]
        if self.explicit_list is not None:
            return self._load_tests_from_lists([self.explicit_list])
        return None

    def _get_discovered_test_files(self, original_test_files: list[Path]) -> list[Path]:
        """Performs automatic test discovery with comprehensive filtering."""
        self._load_test_lists_if_needed()
        test_files = self.__load_test_files(original_test_files)
        return test_files

    def _load_test_lists_if_needed(self) -> None:
        """Loads excluded and ignored test lists if not skipped."""
        if not self.config.test_suite.test_lists.skip_test_lists:
            self.excluded_tests = self._load_excluded_tests()
            self.ignored_tests = self._load_ignored_tests()
            self.ignored_tests_with_failures = self._get_tests_from_ignore_with_failures(
                self._test_lists.ignored_lists)
            self._search_both_excluded_and_ignored_tests()

    def _filter_by_group(self, original_test_files: list[Path]) -> list[Path]:
        if self.config.test_suite.groups.quantity > 1:
            filtered_tests = {test for test in original_test_files if self.__in_group_number(test)}
            return list(filtered_tests)
        return original_test_files

    def __get_n_group(self) -> int:
        groups = self.config.test_suite.groups.quantity
        n_group = self.config.test_suite.groups.number
        return n_group if n_group <= groups else groups

    def __in_group_number(self, test: Path) -> bool:
        groups = self.config.test_suite.groups.quantity
        n_group = self.__get_n_group()
        return get_group_number(str(test.relative_to(self.test_root)), groups) == n_group

    def _create_test(self, test_file: Path, is_ignored: bool) -> ITestFlow:
        test_id = get_test_id(test_file, self.test_root)
        coll_name = self._get_coll_name(test_id)
        params = self._collections_parameters.get(coll_name, {}) if coll_name is not None else {}
        test = workflow_registry.create(self.workflow_kind, test_env=self._test_env, test_path=test_file,
                                        params=IOptions(params), test_id=test_id)
        test.ignored = is_ignored
        if is_ignored and test.path in self.ignored_tests_with_failures:
            test.last_failure = self.ignored_tests_with_failures[test.path]

        return test

    def _get_coll_name(self, test_id: str) -> str | None:
        coll_names = [name for name in self._collections_parameters if test_id.startswith(name)]
        if len(coll_names) == 1:
            return coll_names[0]
        weights = {len(name): name for name in coll_names}
        if weights:
            name = max(weights.keys())
            return weights[name]
        return None

    def _create_tests(self, test_files: list[Path]) -> list[ITestFlow]:
        self._test_env.test_flow_registry.reset()
        for test_file in test_files:
            test_obj = self._create_test(test_file, test_file in self.ignored_tests)
            self._test_env.test_flow_registry.register(test_obj)

        if self.config.test_suite.skip_compile_only_neg:
            loaded = list(self._test_env.test_flow_registry.runtime_tests)
        else:
            loaded = list(self._test_env.test_flow_registry.valid_tests)
        _LOGGER.all(f"Loaded {len(loaded)} tests")
        return loaded

    def _get_test_root_pattern(self, mask: str) -> re.Pattern:
        return re.compile(f"{self.test_root / mask}")

    def __load_test_files(self, original_test_files: list[Path]) -> list[Path]:
        """
        Applies excluded list, filters, chapters and collections to input original_test_files list
        :param original_test_files: unprocessed list of test files
        :return: filtered list.
        """
        if self.config.test_suite.filter != "*" and self.config.test_suite.groups.chapters:
            raise InvalidConfiguration(
                "Incorrect configuration: specify either filter or chapter options"
            )
        test_files: list[Path] = original_test_files
        excluded: list[Path] = list(self.excluded_tests)[:]
        if self.config.test_suite.groups.chapters:
            test_files = self.__filter_by_chapters(self.test_root, test_files)

        filter_mask = self.config.test_suite.filter
        matched_by_filter = self.__build_filter_match_set(filter_mask)
        collections = self.__get_collections_paths()

        result: list[Path] = []
        for test in test_files:
            if test in excluded:
                continue

            rel = self.__to_rel_posix_path(test)

            if self.__check_test_in_coll(rel, collections) and TestSuite.__matches_filter(rel, matched_by_filter):
                result.append(test)

        return result

    def __build_filter_match_set(self, filter_mask: str) -> set[str] | None:
        """
        Returns a set of relative paths matched by glob filter,
        if filter matches dirs - the tests from the dirs are included recursively.
        Returns None when filter is '*' (default val - no filtering).
        """
        if filter_mask == "*":
            return None

        matched: set[str] = set()

        test_glob = f"*.{self.extension}"

        for p in self.test_root.glob(filter_mask):
            if p.is_file():
                matched.add(p.relative_to(self.test_root).as_posix())
            elif p.is_dir():
                for f in p.rglob(test_glob):
                    if f.is_file():
                        matched.add(f.relative_to(self.test_root).as_posix())

        return matched

    def __get_collections_paths(self) -> list[Path]:
        """
        Collection entries are stored as relative POSIX paths.
        They can be either dirs or to a specific .ets file.
        """
        return [Path(col_name) for col_name in self._collections_parameters]

    def __to_rel_posix_path(self, test: Path) -> Path:
        """
        Converts absolute test path to relative path from test_root in POSIX form.
        """
        return test.relative_to(self.test_root)

    def __filter_by_chapters(self, base_folder: Path, files: list[Path]) -> list[Path]:
        test_files: set[Path] = set()
        chapters: Chapters = self.__get_chapters()
        for chapter in self.config.test_suite.groups.chapters:
            test_files.update(chapters.filter_by_chapter(chapter, base_folder, files))
        return list(test_files)

    def __get_chapters(self) -> Chapters:
        chapters_file = self.config.test_suite.groups.chapters_file \
            if Path(self.config.test_suite.groups.chapters_file).is_file() \
            else self._locate_chapters_file()
        return Chapters(chapters_file)

    def _get_explicit_file(self, explicit_file: str | None) -> Path | None:
        """
        Calculates a full file path to be used as a test file (specified in the option `--test-list`) if it exists.

        If both 'explicit_file' and 'test_root' are provided, this method attempts to locate the specified test file
        within the test root directory structure.

        :param explicit_file: name or related path from LIST_ROOT, taken from --test-list option
        :return: the full path if the test list specified or None if `explicit_file` was not given
        :raise TestNotExistException: if the test list is specified, but no real file is found
        """
        if explicit_file is not None and self.test_root is not None:
            test_path = self.__get_explicit_test_path(explicit_file)
            if test_path and test_path.exists():
                return test_path
            raise TestNotExistException(f"Test '{explicit_file}' does not exist")
        return None

    def _load_tests_from_lists(self, lists: list[Path]) -> list[Path]:
        tests: list[Path] = []
        any_not_found = False
        report: list[str] = []
        for list_path in lists:
            _LOGGER.default(f"Loading tests from the list {list_path}")
            prefixes: list[str] = []
            if len(self.config.test_suite.collections) > 1:
                prefixes = [coll.name for coll in self.config.test_suite.collections]
            loaded, not_found = self.__load_list(self.test_root, list_path, prefixes)
            tests.extend(loaded)
            if not_found:
                any_not_found = True
                report.append(f"List '{list_path}': {len(report)} tests are not found on the file system. "
                              "To see full list run with --verbose=all option")
                for test in not_found:
                    report.append(str(test))
        if any_not_found:
            _LOGGER.all("\n".join(report))
        return tests

    def _load_excluded_tests(self) -> list[Path]:
        """
        Read excluded_lists and load list of excluded tests
        """
        excluded_tests = self._filter_by_group(self._load_tests_from_lists(self._test_lists.excluded_lists))
        self._search_duplicates(excluded_tests, "excluded")
        excluded_tests = list(set(excluded_tests))
        self.excluded = len(excluded_tests)
        return excluded_tests

    def _load_ignored_tests(self) -> list[Path]:
        """
        Read ignored_lists and load list of ignored tests
        """
        ignored_tests = self._load_tests_from_lists(self._test_lists.ignored_lists)
        self._search_duplicates(ignored_tests, "ignored")
        return ignored_tests

    def _search_both_excluded_and_ignored_tests(self) -> None:
        already_excluded = [test for test in self.ignored_tests if test in self.excluded_tests]
        if not already_excluded:
            return
        _LOGGER.short(f"Found {len(already_excluded)} tests present both "
                      "in excluded and ignored test lists. "
                      "To see the full list run with the option `--verbose=all`")
        for test in already_excluded:
            _LOGGER.all(f"\t{test}")
            self.ignored_tests.remove(test)

    def __get_collections_parameters(self) -> dict[str, dict[str, list | str]]:
        result: dict[str, dict[str, list | str]] = {}
        for collection in self._test_env.config.test_suite.collections:
            result[collection.name] = collection.parameters
        return result

    def _get_tests_from_ignore_with_failures(self, lists: list[Path]) -> dict[Path, str]:
        """
        Detects whether the test from ignored test list has an expected failure
        :param lists: list of loaded ignored test lists
        :return: a map where a test file is paired with the expected error message
        """
        tests: dict[Path, str] = {}
        for list_path in lists:
            tests_from_list = self._get_tests_with_comment(list_path)
            tests = {**tests, **tests_from_list}

        tests = {key: val for key, val in tests.items() if self._comment_has_failure(val)}
        tests = {key: self._get_failure_text(val) for key, val in tests.items()}
        return tests

    def _get_tests_with_comment(self, list_path: Path) -> dict[Path, str]:
        """
        Searches the test with comments in the excluded/ignored test list
        :param list_path: the investigated test list
        :return: a map where a test file is paired with the comment
        Note: only tests with comments are returned
        """
        tests: dict[Path, str] = {}
        comment_lines: list[str] = []
        prev_line_was_comment = False

        with open(list_path, encoding="utf-8") as file_handler:
            for line in file_handler:
                line = line.rstrip('\n').strip()
                if not line:
                    continue

                if line.startswith("#"):
                    if not prev_line_was_comment:
                        comment_lines = []
                    comment_lines.append(line)
                    prev_line_was_comment = True
                    continue

                prev_line_was_comment = False

                if all(not line.endswith(f".{ext}") for ext in self.supported_extensions):
                    continue

                test_path = self.test_root / line
                if test_path.exists():
                    tests[test_path] = "\n".join(comment_lines)

        return tests


class GTestSuite(TestSuite):
    def __init__(self, test_env: TestEnv) -> None:
        super().__init__(test_env)
        self.__test_root = Path(self.config.test_suite.test_root)
        self.__gtest_bin_root = self.config.test_suite.get_parameter("gtest-bin-root")
        self.precompiled_test_files: bool = cast(bool, self.config.test_suite.get_parameter("precompiled-test-files"))
        self.workflow_kind = self.config.workflow.workflow_type
        self.__gtest_zip_prefix: str | None = cast(str, test_env.config.test_suite.get_parameter("gtest-zip-prefix"))

    @property
    def gtest_root(self) -> Path:
        if self.__gtest_bin_root is not None:
            return Path(self.__gtest_bin_root)

        raise InvalidConfiguration(f"Path to gtest binaries is not specified"
                                   f" in the test suite {self.config.test_suite.suite_name}")

    @property
    def gtest_source_root(self) -> Path:
        return self.__test_root

    @property
    def intermediate(self) -> Path:
        return self.test_root.parent / "intermediate"

    @staticmethod
    def get_tests_from_gtest_cmd(
        file_path: Path,
        qemu_prefix: list[str],
        gtest_filter: str | None = None
    ) -> list[Path]:
        cmd = [file_path.as_posix(), "--gtest_list_tests"]
        if qemu_prefix:
            cmd = [*qemu_prefix, *cmd]
        if gtest_filter is not None:
            cmd.append(f"--gtest_filter={gtest_filter}")
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            check=False,
        )
        return GTestSuite.parse_gtest_list_tests_command(file_path.as_posix(), result.stdout)

    @staticmethod
    def parse_gtest_list_tests_command(binary_name: str, gtest_list_tests: str) -> list[Path]:
        tests: list[Path] = []
        current_suite = None
        for line in gtest_list_tests.splitlines():
            line = line.rstrip()
            if not line:
                continue
            if not line.startswith((' ', '\t')):
                current_suite = line.rstrip('.')
                continue
            if current_suite and line.strip():
                test_name = line.strip().split('#')[0].strip()
                if not test_name:
                    continue
                suite_name = f"{binary_name}/{current_suite}"
                tests.append(Path(f'{suite_name}.{test_name}'))
        return tests

    @staticmethod
    def load_gtest_names_from_file(test_lists: list[Path]) -> list[Path]:
        tests = []
        for test_list in test_lists:
            with open(test_list, encoding="utf-8") as t_file:
                lines = t_file.readlines()
                tests.extend([Path(test.rstrip()) for test in lines if test.rstrip('\n') and not test.startswith("#")])
        return tests

    def load_tests_from_test_path(self, test_path: Path, root_path: Path) -> list[Path]:
        qemu_prefix = self._test_env.cmd_prefix if self.config.general.qemu != QemuKind.NONE else []
        gtest = GTestFile.parse_from_path(test_path.as_posix())
        if gtest is None:
            _LOGGER.error(f"\n{pretty_divider()}\n"
                          f"ERROR: Invalid gtest binary at path '{test_path}'\n"
                          f"Failed to parse gtest file structure.\n"
                          f"Please verify that the file exists and is a valid gtest executable.\n"
                          f"{pretty_divider()}\n")
            return []
        gtest_binary_full_path = correct_path(root_path, gtest.test_file_name)

        if gtest.test_name is not None or gtest.test_param is not None:
            return [correct_path(root_path, test_path)]

        if gtest.case_name and gtest.suite_name:
            gtest_filter = f'{gtest.suite_name}/{gtest.case_name}.*'
        elif gtest.suite_name:
            gtest_filter = f'{gtest.suite_name}/*'
        elif gtest.case_name:
            gtest_filter = f'{gtest.case_name}.*'
        else:
            return self.get_tests_from_gtest_cmd(gtest_binary_full_path, qemu_prefix)

        return self.get_tests_from_gtest_cmd(gtest_binary_full_path, qemu_prefix, gtest_filter)

    def load_tests_from_directory(self, pattern: Path, filter_path: Path) -> list[Path]:
        tests_paths = fnmatch.filter(glob(pattern.as_posix(), recursive=True), filter_path.as_posix())
        result = []
        for file_path in tests_paths:
            path = Path(file_path)
            if path.is_file() and is_executable_file(path):
                test_path = path.relative_to(self.gtest_root)
                result.extend(self.load_tests_from_test_path(test_path, self.gtest_root))
        return result

    def process(self, force_generate: bool) -> list[ITestFlow]:
        """
        Overrides parent process method to handle gtest-specific workflow.

        This method adapts the standard test suite processing for gtest binaries:
        1. Prepares test files through preparation steps
        2. Archives ETS test files into zip packages
        3. Loads tests from gtest binaries using --gtest_list_tests
        4. Applies filtering based on excluded/ignored lists

        :param force_generate: if True, forces regeneration of test files
        :return: list of ITestFlow objects ready for execution
        """
        test_paths: list[Path] = []
        ets_raw_set: set[Path] = set()

        self._explicit_file = self._set_explicit_file()
        self._explicit_list = self._test_lists.resolve_explicit_list()

        for step in self._preparation_steps:
            ets_raw_set.update(step.transform(force_generate))

        compiled_ets_set: set[Path] = set(self.intermediate.rglob("**/*"))

        self._disable_es2panda_step()

        self._archive_ets_files(compiled_ets_set)

        if self._explicit_file is not None:
            # check exist _explicit_file
            test_paths = self.load_tests_from_test_path(self._explicit_file, self.gtest_root)
        elif self._explicit_list is not None:
            test_paths = self._load_tests_from_lists([self._explicit_list])
        else:
            pattern = self.gtest_root / '**' / '*'
            filter_path = self.gtest_root / self.config.test_suite.filter
            test_paths = self.load_tests_from_directory(pattern, filter_path)

        if not self.config.test_suite.test_lists.skip_test_lists:
            test_paths = self._resolve_test_files(test_paths)

        self._search_both_excluded_and_ignored_tests()

        # to-do
        # test_paths = self._filter_by_group(test_paths)

        tests = self._create_tests(test_paths)
        _LOGGER.default(f"Loaded {len(tests)} valid tests from the folder '{self.gtest_root}'. "
                        f"Excluded {self.excluded} tests are not loaded.")
        return tests

    def _archive_ets_files(self, ets_test_parts: set[Path]) -> None:
        """
        Ets test parts need to be renamed and zip-ed
        """
        file_name = "classes"
        file_ext = "abc"

        for test_file in ets_test_parts:
            naming_rule = ani_name_rule if self.__gtest_zip_prefix == "ani_test" else None
            zip_name = make_gtest_name(test_file, self.__gtest_zip_prefix, rule=naming_rule)
            zip_path = test_file.parent / f"{zip_name}_gtest_package.zip"
            arcname = f"{file_name}.{file_ext}"

            with ZipFile(zip_path, "w") as z:
                z.write(test_file, arcname=arcname)

            test_file.unlink()

    def _disable_es2panda_step(self) -> None:
        steps = self.config.workflow.steps
        for i, step in enumerate(steps):
            if step.step_kind == StepKind.COMPILER:
                steps[i] = replace(step, enabled=False)
        _LOGGER.short(f"es2panda step for workflow {self.config.workflow.name} is disabled")

    def _create_test(self, test_file: Path, is_ignored: bool) -> ITestFlow:
        """
        Overrides parent _create_test to use gtest_root instead of test_root for test ID resolution.

        This adaptation is necessary because gtest tests are located in the gtest binaries directory,
        not in the generated test directory.

        :param test_file: path to the gtest binary file
        :param is_ignored: flag indicating if the test is in the ignored list
        :return: ITestFlow object configured for gtest execution
        """
        test_id = get_test_id(test_file, self.gtest_root)
        coll_name = self._get_coll_name(test_id)
        params = self._collections_parameters.get(coll_name, {}) if coll_name is not None else {}

        test = workflow_registry.create(self.workflow_kind, test_env=self._test_env, test_path=self.gtest_root,
                                        params=IOptions(params), test_id=test_id)
        test.ignored = is_ignored
        if is_ignored and test.path in self.ignored_tests_with_failures:
            test.last_failure = self.ignored_tests_with_failures[test.path]

        return test

    def _set_explicit_file(self) -> Path | None:
        test_id = self.config.test_suite.test_lists.explicit_file
        if test_id is not None and self.list_roots is not None:
            return Path(test_id)
        return None

    def _load_tests_from_lists(self, lists: list[Path]) -> list[Path]:
        """
        Overrides parent method to handle gtest-specific test name loading.

        Instead of loading file paths directly, this method:
        1. Reads gtest test names from list files
        2. Resolves each gtest binary to its individual test cases
           using --gtest_list_tests command

        :param lists: list of paths to gtest test list files
        :return: expanded list of gtest test paths (including individual test cases)
        """
        tests_from_lists = self.load_gtest_names_from_file(lists)
        result = []
        for test_path in tests_from_lists:
            result.extend(self.load_tests_from_test_path(test_path, self.gtest_root))
        return result

    def _resolve_test_files(self, original_test_files: list[Path]) -> list[Path]:
        """
        Overrides parent method to resolve gtest test files with exclusion filtering.

        This method:
        1. Loads excluded and ignored test lists
        2. Filters out excluded tests from the original test files
        3. Handles ignored tests with expected failures

        :param original_test_files: list of gtest test paths to filter
        :return: filtered list of test files excluding excluded tests
        """
        _LOGGER.short(f"Loading tests from the directory {self.test_root}")
        _test_paths = []
        self.excluded_tests = self._load_excluded_tests()
        self.ignored_tests = self._load_ignored_tests()
        self.ignored_tests_with_failures = self._get_tests_from_ignore_with_failures(
            self._test_lists.ignored_lists)
        _test_paths.extend([test_file for test_file in original_test_files if test_file not in self.excluded_tests])
        return _test_paths
