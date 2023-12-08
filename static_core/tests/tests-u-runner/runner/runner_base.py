import fnmatch
import logging
import multiprocessing
import re
from abc import abstractmethod, ABC
from collections import Counter
from datetime import datetime
from glob import glob
from os import path
from pathlib import Path
from typing import List, Set, Tuple, Optional

from tqdm import tqdm

from runner.chapters import parse_chapters
from runner.enum_types.configuration_kind import ConfigurationKind
from runner.plugins.ets.test_ets import TestETS
from runner.logger import Log
from runner.options.config import Config
from runner.test_base import Test
from runner.utils import get_group_number

CONST_COMMENT = ["#"]
TEST_COMMENT_EXPR = re.compile(r"^\s*(?P<test>[^# ]+)?(\s*#\s*(?P<comment>.+))?", re.MULTILINE)


def load_list(test_root: str, test_list_path: str) -> List[str]:
    result: List[str] = []
    if not path.exists(test_list_path):
        return result

    with open(test_list_path, 'r', encoding="utf-8") as file:
        for line in file:
            test, _ = get_test_and_comment_from_line(line.strip(" \n"))
            if test is not None:
                result.append(path.join(test_root, test))

    return result


def get_test_id(file: str, start_directory: Path) -> str:
    relpath = Path(file).relative_to(start_directory)
    return str(relpath)


def get_test_and_comment_from_line(line: str) -> Tuple[Optional[str], Optional[str]]:
    line_parts = TEST_COMMENT_EXPR.search(line)
    if line_parts:
        return line_parts["test"], line_parts["comment"]
    return None, None


def correct_path(root: Path, test_list: str) -> str:
    return path.abspath(test_list) if path.exists(test_list) else path.join(root, test_list)


_LOGGER = logging.getLogger("runner.runner_base")


class Runner(ABC):
    def __init__(self, config: Config, name: str) -> None:
        # This file is expected to be located at path:
        # $PANDA_SOURCE/tests/tests-u-runner/runner/runner_base.py
        current_folder_parent = path.dirname(path.dirname(path.abspath(__file__)))
        self.static_core_root = path.dirname(path.dirname(current_folder_parent))

        # TODO(vpukhov): adjust es2panda path
        default_ets_arktsconfig = path.join(
            config.general.build,
            "tools", "es2panda", "generated", "arktsconfig.json"
        )
        if not path.exists(default_ets_arktsconfig):
            default_ets_arktsconfig = path.join(
                config.general.build,
                "gen",  # for GN build
                "tools", "es2panda", "generated", "arktsconfig.json"
            )

        # Roots:
        # directory where test files are located - it's either set explicitly to the absolute value
        # or the current folder (where this python file is located!) parent
        self.test_root = config.general.test_root
        if self.test_root is not None:
            Log.summary(_LOGGER, f"TEST_ROOT set to {self.test_root}")
        # directory where list files (files with list of ignored, excluded, and other tests) are located
        # it's either set explicitly to the absolute value or
        # the current folder (where this python file is located!) parent
        self.default_list_root = path.join(self.static_core_root, "tests", "tests-u-runner", "test-lists")
        self.list_root = config.general.list_root if config.general.list_root is not None else None
        if self.list_root is not None:
            Log.summary(_LOGGER, f"LIST_ROOT set to {self.list_root}")

        # runner init time
        self.start_time = datetime.now()
        # root directory containing bin folder with binary files
        self.build_dir = config.general.build
        self.arktsconfig = config.es2panda.arktsconfig \
            if config.es2panda.arktsconfig is not None \
            else default_ets_arktsconfig

        self.config = config
        self.name: str = name

        # Lists:
        # excluded test is a test what should not be loaded and should be tried to run
        # excluded_list: either absolute path or path relative from list_root to the file with the list of such tests
        self.excluded_lists: List[str] = []
        self.excluded_tests: Set[str] = set([])
        # ignored test is a test what should be loaded and executed, but its failure should be ignored
        # ignored_list: either absolute path or path relative from list_root to the file with the list of such tests
        # aka: kfl = known failures list
        self.ignored_lists: List[str] = []
        self.ignored_tests: Set[str] = set([])
        # list of file names, each is a name of a test. Every test should be executed
        # So, it can contain ignored tests, but cannot contain excluded tests
        self.tests: Set[Test] = set([])
        # list of results of every executed test
        self.results: List[Test] = []
        # name of file with a list of only tests what should be executed
        # if it's specified other tests are not executed
        self.explicit_list = correct_path(self.list_root, config.test_lists.explicit_list) \
            if config.test_lists.explicit_list is not None and self.list_root is not None \
            else None
        # name of the single test file in form of a relative path from test_root what should be executed
        # if it's specified other tests are not executed even if test_list is set
        self.explicit_test = config.test_lists.explicit_file

        # Counters:
        # failed + ignored + passed + excluded_after = len of all executed tests
        # failed + ignored + passed + excluded_after + excluded = len of full set of tests
        self.failed = 0
        self.ignored = 0
        self.passed = 0
        self.excluded = 0
        # Test chosen to execute can detect itself as excluded one
        self.excluded_after = 0
        self.update_excluded = config.test_lists.update_excluded

        self.conf_kind = Runner.detect_conf(config)

    @staticmethod
    # pylint: disable=too-many-return-statements
    def detect_conf(config: Config) -> ConfigurationKind:
        if config.ark_aot.enable:
            is_aot_full = len([
                arg for arg in config.ark_aot.aot_args
                if "--compiler-inline-full-intrinsics=true" in arg
            ]) > 0
            if is_aot_full:
                return ConfigurationKind.AOT_FULL
            return ConfigurationKind.AOT

        if config.ark.jit.enable:
            return ConfigurationKind.JIT

        if config.ark.interpreter_type:
            return ConfigurationKind.OTHER_INT

        if config.quick.enable:
            return ConfigurationKind.QUICK

        return ConfigurationKind.INT

    def load_tests_from_lists(self, lists: List[str]) -> List[str]:
        tests = []
        for list_name in lists:
            list_path = correct_path(self.list_root, list_name)
            Log.summary(_LOGGER, f"Loading tests from the list {list_path}")
            assert self.test_root, "TEST_ROOT not set to correct value"
            tests.extend(load_list(self.test_root, list_path))
        return tests

    # Read excluded_lists and load list of excluded tests
    def load_excluded_tests(self) -> None:
        excluded_tests = self.load_tests_from_lists(self.excluded_lists)
        self.excluded_tests.update(excluded_tests)
        self.excluded = len(self.excluded_tests)
        self._search_duplicates(excluded_tests, "excluded")

    # Read ignored_lists and load list of ignored tests
    def load_ignored_tests(self) -> None:
        ignored_tests = self.load_tests_from_lists(self.ignored_lists)
        self.ignored_tests.update(ignored_tests)
        self._search_duplicates(ignored_tests, "ignored")

    @staticmethod
    def _search_duplicates(original: List[str], kind: str) -> None:
        main_counter = Counter(original)
        dupes = [test for test, frequency in main_counter.items() if frequency > 1]
        if len(dupes) > 0:
            Log.summary(_LOGGER, f"There are {len(dupes)} duplicates in {kind} lists.")
            for test in dupes:
                Log.short(_LOGGER, f"\t{test}")
        elif len(original) > 0:
            Log.summary(_LOGGER, f"No duplicates found in {kind} lists.")

    # Browse the directory, search for files with the specified extension
    # and add them as tests
    def add_directory(self, directory: str, extension: str, flags: List[str]) -> None:
        Log.summary(_LOGGER, f"Loading tests from the directory {directory}")
        test_files = []
        if self.explicit_test is not None:
            test_files.extend([correct_path(self.test_root, self.explicit_test)])
        elif self.explicit_list is not None:
            test_files.extend(self.load_tests_from_lists([self.explicit_list]))
        else:
            if not self.config.test_lists.skip_test_lists:
                self.load_excluded_tests()
                self.load_ignored_tests()
            test_files.extend(self.__load_tests_from_lists(directory, extension))

        self._search_both_excluded_and_ignored_tests()
        self._search_not_used_ignored(test_files)

        all_tests = {self.create_test(test, flags, test in self.ignored_tests) for test in test_files}
        not_tests = {t for t in all_tests if isinstance(t, TestETS) and not t.is_valid_test}
        valid_tests = all_tests - not_tests

        if self.config.test_lists.groups.quantity > 1:
            groups = self.config.test_lists.groups.quantity
            n_group = self.config.test_lists.groups.number
            n_group = n_group if n_group <= groups else groups
            valid_tests = {
                test for test in valid_tests
                if get_group_number(test.path, groups) == n_group
            }

        self.tests.update(valid_tests)
        Log.all(_LOGGER, f"Loaded {len(self.tests)} tests")

    def __load_tests_from_lists(self, directory: str, extension: str) -> List[str]:
        test_files: List[str] = []
        excluded: List[str] = list(self.excluded_tests)[:]
        glob_expression = path.join(directory, f"**/*.{extension}")
        includes, excludes = self.__parse_chapters()
        for inc in includes:
            mask = path.join(directory, inc)
            test_files.extend(self.__load_tests_by_chapter(mask, glob_expression))
        for exc in excludes:
            mask = path.join(directory, exc)
            excluded.extend(self.__load_tests_by_chapter(mask, glob_expression))
        return [
            test for test in test_files
            if self.update_excluded or test not in excluded
        ]

    @staticmethod
    def __load_tests_by_chapter(mask: str, glob_expression: str) -> List[str]:
        if "*" not in mask and path.isfile(mask):
            return [mask]
        if "*" not in mask:
            mask += '/*'
        return list(fnmatch.filter(
            glob(glob_expression, recursive=True),
            mask
        ))

    def __parse_chapters(self) -> Tuple[List[str], List[str]]:
        if not self.config.test_lists.groups.chapters:
            return [self.config.test_lists.filter], []
        if path.isfile(self.config.test_lists.groups.chapters_file):
            chapters = parse_chapters(self.config.test_lists.groups.chapters_file)
        else:
            corrected_chapters_file = correct_path(self.list_root, self.config.test_lists.groups.chapters_file)
            if path.isfile(corrected_chapters_file):
                chapters = parse_chapters(corrected_chapters_file)
            else:
                Log.exception_and_raise(
                    _LOGGER,
                    f"Not found either '{self.config.test_lists.groups.chapters_file}' or "
                    f"'{corrected_chapters_file}'", FileNotFoundError)
        __includes: List[str] = []
        __excludes: List[str] = []
        for chapter in self.config.test_lists.groups.chapters:
            if chapter in chapters:
                __includes.extend(chapters[chapter].includes)
                __excludes.extend(chapters[chapter].excludes)
        return __includes, __excludes

    def _search_both_excluded_and_ignored_tests(self) -> None:
        already_excluded = [test for test in self.ignored_tests if test in self.excluded_tests]
        if not already_excluded:
            return
        Log.summary(_LOGGER, f"Found {len(already_excluded)} tests present both in excluded and ignored test "
                             f"lists.")
        for test in already_excluded:
            Log.all(_LOGGER, f"\t{test}")
            self.ignored_tests.remove(test)

    def _search_not_used_ignored(self, found_tests: List[str]) -> None:
        ignored_absent = [test for test in self.ignored_tests if test not in found_tests]
        if ignored_absent:
            Log.summary(_LOGGER, f"Found {len(ignored_absent)} tests in ignored lists but absent on the file system:")
            for test in ignored_absent:
                Log.summary(_LOGGER, f"\t{test}")
        else:
            Log.short(_LOGGER, "All ignored tests are found on the file system")

    @abstractmethod
    def create_test(self, test_file: str, flags: List[str], is_ignored: bool) -> Test:
        pass

    @staticmethod
    def run_test(test: Test) -> Test:
        return test.run()

    def run(self) -> None:
        Log.all(_LOGGER, "Start test running")
        with multiprocessing.Pool(processes=self.config.general.processes) as pool:
            results = pool.imap_unordered(self.run_test, self.tests, chunksize=self.config.general.chunksize)
            if self.config.general.show_progress:
                results = tqdm(results, total=len(self.tests))
            self.results = list(results)
            pool.close()
            pool.join()

    @abstractmethod
    def summarize(self) -> int:
        pass

    @abstractmethod
    def create_coverage_html(self) -> None:
        pass
