#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
from collections import Counter
from functools import cached_property
from glob import glob
from os import path
from pathlib import Path
from typing import List, Optional, Set, Tuple, Dict, Any

from runner.chapters import Chapters
from runner.common_exceptions import TestNotExistException, InvalidConfiguration
from runner.enum_types.params import TestEnv
from runner.logger import Log
from runner.options.options import IOptions
from runner.suites.preparation_step import TestPreparationStep, CustomGeneratorTestPreparationStep, CopyStep
from runner.suites.step_utils import StepUtils
from runner.suites.test_lists import TestLists
from runner.suites.test_standard_flow import TestStandardFlow
from runner.utils import correct_path, get_group_number, get_test_id

_LOGGER = Log.get_logger(__file__)


class TestSuite:
    CONST_COMMENT: ClassVar[str] = "#"
    TEST_COMMENT_EXPR = re.compile(r"^\s*(?P<test>[^# ]+)?(\s*#\s*(?P<comment>.+))?", re.MULTILINE)

    def __init__(self, test_env: TestEnv) -> None:
        self.__test_env = test_env
        self.__suite_name = test_env.config.test_suite.suite_name
        self.__work_dir = test_env.work_dir
        self._list_root = Path(test_env.config.test_suite.list_root)

        self.config = test_env.config

        self._preparation_steps: List[TestPreparationStep] = self.set_preparation_steps()
        self.__collections_parameters = self.__get_collections_parameters()

        self._explicit_file: Optional[Path] = None
        self._explicit_list: Optional[Path] = None
        self._test_lists = TestLists(self._list_root, self.__test_env)
        self._test_lists.collect_excluded_test_lists(test_name=self.__suite_name)
        self._test_lists.collect_ignored_test_lists(test_name=self.__suite_name)
        self.excluded = 0
        self.ignored_tests: List[Path] = []
        self.excluded_tests: List[Path] = []

    @property
    def test_root(self) -> Path:
        return self.__work_dir.gen

    @staticmethod
    def __load_list(test_root: Path, test_list_path: Path, prefixes: List[str]) -> Tuple[List[Path], List[Path]]:
        result: List[Path] = []
        not_found: List[Path] = []
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
    def __load_line(line: str, test_root: Path, prefixes: List[str]) -> Tuple[bool, Optional[Path]]:
        test, _ = TestSuite.__get_test_and_comment_from_line(line.strip(" \n"))
        if test is not None:
            test_path = test_root / test
            if test_path.exists():
                return True, test_path
            else:
                is_found, prefixed_test_path = TestSuite.__load_line_with_prefix(test_root, prefixes, test)
                if prefixed_test_path is not None:
                    return is_found, prefixed_test_path
            return False, test_path
        return False, None

    @staticmethod
    def __load_line_with_prefix(test_root: Path, prefixes: List[str], test: str) -> Tuple[bool, Optional[Path]]:
        for prefix in prefixes:
            test_path = test_root / prefix / test
            if test_path.exists():
                return True, test_path
        return False, None

    @staticmethod
    def __get_test_and_comment_from_line(line: str) -> Tuple[Optional[str], Optional[str]]:
        line_parts = TestSuite.TEST_COMMENT_EXPR.search(line)
        if line_parts:
            return line_parts["test"], line_parts["comment"]
        return None, None

    @staticmethod
    def __search_duplicates(original: List[Path], kind: str) -> None:
        main_counter = Counter([str(test) for test in original])
        dupes = [test for test, frequency in main_counter.items() if frequency > 1]
        if len(dupes) > 0:
            _LOGGER.summary(f"There are {len(dupes)} duplicates in {kind} lists.")
            for test in dupes:
                Log.short(_LOGGER, f"\t{test}")
        elif len(original) > 0:
            _LOGGER.summary(f"No duplicates found in {kind} lists.")

    @cached_property
    def name(self) -> str:
        return self.__suite_name

    @cached_property
    def list_root(self) -> Path:
        return self._list_root

    @cached_property
    def explicit_list(self) -> Optional[Path]:
        return self._explicit_list

    @cached_property
    def explicit_file(self) -> Optional[Path]:
        return self._explicit_file

    def process(self, force_generate: bool) -> List[TestStandardFlow]:
        raw_set = self.__get_raw_set(force_generate)
        self._explicit_file = self.__set_explicit_file()
        self._explicit_list = self.__set_explicit_list()
        executed_set = self.__get_executed_files(raw_set)
        self.__search_both_excluded_and_ignored_tests()
        executed_set = self.__get_by_groups(executed_set)
        tests = self.__create_tests(executed_set)
        return tests

    def set_preparation_steps(self) -> List[TestPreparationStep]:
        steps: List[TestPreparationStep] = []
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
                steps.append(CopyStep(
                    test_source_path=copy_source_path,
                    test_gen_path=self.test_root / collection.name,
                    config=self.config,
                    collection=collection,
                    extension=extension
                ))
        return steps

    def __get_raw_set(self, force_generate: bool) -> List[Path]:
        util = StepUtils()
        tests: List[Path] = []
        if not force_generate and util.are_tests_generated(self.test_root):
            _LOGGER.all(f"Reused earlier generated tests from {self.test_root}")
            return self.__load_generated_test_files()
        _LOGGER.all("Generated folder : " + str(self.test_root))

        if self.test_root.exists():
            _LOGGER.all(f"INFO: {str(self.test_root.absolute())} already exist. WILL BE CLEANED")
            shutil.rmtree(self.test_root)

        for step in self._preparation_steps:
            tests.extend(step.transform(force_generate))
        tests = list(set(tests))

        if len(tests) == 0:
            raise InvalidConfiguration("No tests loaded to execution")

        util.create_report(self.test_root, tests)

        return tests

    def __load_generated_test_files(self) -> List[Path]:
        tests = []
        for collection in self.config.test_suite.collections:
            extension = self.config.test_suite.extension(collection)
            glob_expression = path.join(self.test_root, collection.name, f"**/*.{extension}")
            tests.extend(fnmatch.filter(
                glob(glob_expression, recursive=True),
                path.join(self.test_root, self.config.test_suite.filter)
            ))
        return [Path(test) for test in tests]

    def __get_explicit_test_path(self, test_id: str) -> Optional[Path]:
        for collection in self.config.test_suite.collections:
            if test_id.startswith(collection.name):
                test_path: Path = correct_path(self.test_root, test_id)
                if test_path.exists():
                    return test_path
                break
            new_test_id = str(os.path.join(collection.name, test_id))
            test_path = correct_path(self.test_root, new_test_id)
            if test_path and test_path.exists():
                return test_path
        return None

    def __get_executed_files(self, raw_test_files: List[Path]) -> List[Path]:
        """
        Browse the directory, search for files with the specified extension
        """
        _LOGGER.summary(f"Loading tests from the directory {self.test_root}")
        test_files: List[Path] = []
        if self.explicit_file is not None:
            test_files.append(self.explicit_file)
        elif self.explicit_list is not None:
            test_files.extend(self.__load_tests_from_lists([self.explicit_list]))
        else:
            if not self.config.test_suite.test_lists.skip_test_lists:
                self.excluded_tests = self.__load_excluded_tests()
                self.ignored_tests = self.__load_ignored_tests()
            test_files.extend(self.__load_test_files(raw_test_files))
        return test_files

    def __get_by_groups(self, raw_test_files: List[Path]) -> List[Path]:
        if self.config.test_suite.groups.quantity > 1:
            filtered_tests = {test for test in raw_test_files if self.__in_group_number(test)}
            return list(filtered_tests)
        return raw_test_files

    def __get_n_group(self) -> int:
        groups = self.config.test_suite.groups.quantity
        n_group = self.config.test_suite.groups.number
        return n_group if n_group <= groups else groups

    def __in_group_number(self, test: Path) -> bool:
        groups = self.config.test_suite.groups.quantity
        n_group = self.__get_n_group()
        return get_group_number(str(test.relative_to(self.test_root)), groups) == n_group

    def __create_test(self, test_file: Path, is_ignored: bool) -> TestStandardFlow:
        test_id = get_test_id(test_file, self.test_root)
        coll_names = [name for name in self.__collections_parameters if test_id.startswith(name)]
        params = self.__collections_parameters.get(coll_names[0], {}) if coll_names else {}
        test = TestStandardFlow(
            test_env=self.__test_env,
            test_path=test_file,
            params=IOptions(params),
            test_id=test_id)
        test.ignored = is_ignored
        return test

    def __create_tests(self, raw_test_files: List[Path]) -> List[TestStandardFlow]:
        all_tests = {self.__create_test(test, test in self.ignored_tests) for test in raw_test_files}
        not_tests = {t for t in all_tests if not t.is_valid_test}
        valid_tests = all_tests - not_tests

        _LOGGER.all(f"Loaded {len(valid_tests)} tests")

        return list(valid_tests)

    def __load_test_files(self, raw_test_files: List[Path]) -> List[Path]:
        if self.config.test_suite.filter != "*" and self.config.test_suite.groups.chapters:
            raise InvalidConfiguration(
                "Incorrect configuration: specify either filter or chapter options"
            )
        test_files: List[Path] = raw_test_files
        excluded: List[Path] = list(self.excluded_tests)[:]
        if self.config.test_suite.groups.chapters:
            test_files = self.__filter_by_chapters(self.test_root, test_files)
        pattern = re.compile(self.config.test_suite.filter.replace(".", r"\.").replace("*", ".*"))
        return [test for test in test_files if test not in excluded and pattern.search(str(test))]

    def __filter_by_chapters(self, base_folder: Path, files: List[Path]) -> List[Path]:
        test_files: Set[Path] = set()
        chapters: Chapters = self.__parse_chapters()
        for chapter in self.config.test_suite.groups.chapters:
            test_files.update(chapters.filter_by_chapter(chapter, base_folder, files))
        return list(test_files)

    def __parse_chapters(self) -> Chapters:
        chapters: Optional[Chapters] = None
        if path.isfile(self.config.test_suite.groups.chapters_file):
            chapters = Chapters(self.config.test_suite.groups.chapters_file)
        else:
            corrected_chapters_file = correct_path(self.list_root,
                                                   self.config.test_suite.groups.chapters_file)
            if path.isfile(corrected_chapters_file):
                chapters = Chapters(corrected_chapters_file)
            else:
                raise FileNotFoundError(
                    f"Not found either '{self.config.test_suite.groups.chapters_file}' or "
                    f"'{corrected_chapters_file}'")
        return chapters

    def __set_explicit_file(self) -> Optional[Path]:
        explicit_file = self.config.test_suite.test_lists.explicit_file
        if explicit_file is not None and self.list_root is not None:
            test_path = self.__get_explicit_test_path(explicit_file)
            if test_path and test_path.exists():
                return test_path
            raise TestNotExistException(f"Test '{explicit_file}' does not exist")
        return None

    def __set_explicit_list(self) -> Optional[Path]:
        if self.config.test_suite.test_lists.explicit_list is not None and self.list_root is not None:
            return correct_path(self.list_root, self.config.test_suite.test_lists.explicit_list)
        return None

    def __load_tests_from_lists(self, lists: List[Path]) -> List[Path]:
        tests = []
        any_not_found = False
        report = []
        for list_path in lists:
            _LOGGER.summary(f"Loading tests from the list {list_path}")
            prefixes: List[str] = []
            if len(self.config.test_suite.collections) > 1:
                prefixes = [coll.name for coll in self.config.test_suite.collections]
            loaded, not_found = self.__load_list(self.test_root, list_path, prefixes)
            tests.extend(loaded)
            if not_found:
                any_not_found = True
                report.append(f"List '{list_path}': following tests are not found on the file system:")
                for test in not_found:
                    report.append(str(test))
        if any_not_found:
            _LOGGER.summary("\n".join(report))
        return tests

    def __load_excluded_tests(self) -> List[Path]:
        """
        Read excluded_lists and load list of excluded tests
        """
        excluded_tests = self.__load_tests_from_lists(self._test_lists.excluded_lists)
        self.excluded = len(excluded_tests)
        self.__search_duplicates(excluded_tests, "excluded")
        return excluded_tests

    def __load_ignored_tests(self) -> List[Path]:
        """
        Read ignored_lists and load list of ignored tests
        """
        ignored_tests = self.__load_tests_from_lists(self._test_lists.ignored_lists)
        self.__search_duplicates(ignored_tests, "ignored")
        return ignored_tests

    def __search_both_excluded_and_ignored_tests(self) -> None:
        already_excluded = [test for test in self.ignored_tests if test in self.excluded_tests]
        if not already_excluded:
            return
        _LOGGER.summary(f"Found {len(already_excluded)} tests present both "
                        "in excluded and ignored test lists.")
        for test in already_excluded:
            _LOGGER.all(f"\t{test}")
            self.ignored_tests.remove(test)

    def __get_collections_parameters(self) -> Dict[str, Dict[str, Any]]:
        result: Dict[str, Dict[str, Any]] = {}
        for collection in self.__test_env.config.test_suite.collections:
            result[collection.name] = collection.parameters
        return result
