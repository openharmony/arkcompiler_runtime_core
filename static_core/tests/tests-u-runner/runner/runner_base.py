import fnmatch
import logging
import multiprocessing
import re
from abc import abstractmethod, ABC
from collections import Counter
from datetime import datetime
from glob import glob
from itertools import chain
from os import path
from typing import List, Set

from tqdm import tqdm

from runner.enum_types.configuration_kind import ConfigurationKind
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


def get_test_id(file, start_directory):
    return path.relpath(file, start_directory)


def is_line_a_comment(line):
    for comment in CONST_COMMENT:
        if line.startswith(comment):
            return True
    return False


def is_line_a_test(line):
    return len(line) and not is_line_a_comment(line)


def get_test_and_comment_from_line(line):
    line_parts = TEST_COMMENT_EXPR.search(line)
    if line_parts:
        return line_parts["test"], line_parts["comment"]
    return None, None


def get_test_from_line(line):
    test, _ = get_test_and_comment_from_line(line)
    return test


def get_comment_from_line(line):
    _, comment = get_test_and_comment_from_line(line)
    return comment


def correct_path(root, test_list):
    return path.abspath(test_list) if path.exists(test_list) else path.join(root, test_list)


_LOGGER = logging.getLogger("runner.runner_base")


class Runner(ABC):
    def __init__(self, config: Config, name):
        # TODO(vpukhov): adjust es2panda path
        default_ets_arktsconfig = path.join(
            config.general.build,
            "plugins", "es2panda", "generated", "arktsconfig.json"
        )
        if not path.exists(default_ets_arktsconfig):
            default_ets_arktsconfig = path.join(
                config.general.build,
                "gen",  # for GN build
                "plugins", "es2panda", "generated", "arktsconfig.json"
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
        self.name = name

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
    def detect_conf(config: Config):
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

    def load_tests_from_list(self, list_name):
        list_path = correct_path(self.list_root, list_name)
        Log.summary(_LOGGER, f"Loading tests from the list {list_path}")
        assert self.test_root, "TEST_ROOT not set to correct value"
        return load_list(self.test_root, list_path)

    def load_tests_from_lists(self, lists):
        return list(chain(*(
            map(self.load_tests_from_list, lists)
        )))

    # Read excluded_lists and load list of excluded tests
    def load_excluded_tests(self):
        excluded_tests_list = self.load_tests_from_lists(self.excluded_lists)
        self.excluded_tests.update(excluded_tests_list)
        self.excluded = len(self.excluded_tests)
        self._search_duplicates(excluded_tests_list, "excluded")

    # Read ignored_lists and load list of ignored tests
    def load_ignored_tests(self):
        ignored_tests_list = self.load_tests_from_lists(self.ignored_lists)
        self.ignored_tests.update(ignored_tests_list)
        self._search_duplicates(ignored_tests_list, "ignored")

    @staticmethod
    def _search_duplicates(original, kind):
        main_counter = Counter(original)
        dupes = [test for test, frequency in main_counter.items() if frequency > 1]
        if len(dupes) > 0:
            Log.summary(_LOGGER, f"There are {len(dupes)} duplicates in {kind} lists.")
            for test in dupes:
                Log.short(_LOGGER, f"\t{test}")
        elif len(original) > 0:
            Log.summary(_LOGGER, f"No duplicates found in {kind} lists.")

    # Read explicit_list and load list of executed tests
    def load_explicit_tests(self):
        if self.explicit_list is not None:
            return self.load_tests_from_list(self.explicit_list)

        return []

    # Load one explicitly specified test what should be executed
    def load_explicit_test(self):
        if self.explicit_test is not None:
            return [correct_path(self.test_root, self.explicit_test)]

        return []

    # Browse the directory, search for files with the specified extension
    # and add them as tests
    def add_directory(self, directory, extension, flags):
        Log.summary(_LOGGER, f"Loading tests from the directory {directory}")
        test_files = []
        if self.explicit_test is not None:
            test_files.extend(self.load_explicit_test())
        elif self.explicit_list is not None:
            test_files.extend(self.load_explicit_tests())
        else:
            if not self.config.test_lists.skip_test_lists:
                self.load_excluded_tests()
                self.load_ignored_tests()
            if len(test_files) == 0:
                glob_expression = path.join(directory, f"**/*.{extension}")
                test_files.extend(fnmatch.filter(
                    glob(glob_expression, recursive=True),
                    path.join(directory, self.config.test_lists.filter)
                ))

        without_excluded = [test for test in test_files
                            if self.update_excluded or test not in self.excluded_tests]

        if self.config.test_lists.groups.quantity > 1:
            groups = self.config.test_lists.groups.quantity
            n_group = self.config.test_lists.groups.number
            n_group = n_group if n_group <= groups else groups
            without_excluded = [
                test for test in without_excluded
                if get_group_number(test, groups) == n_group
            ]

        self._search_both_excluded_and_ignored_tests()
        self._search_not_used_ignored(without_excluded)

        tests = [self.create_test(test, flags, test in self.ignored_tests)
                 for test in without_excluded]
        self.tests.update(tests)
        Log.all(_LOGGER, f"Loaded {len(self.tests)} tests")

    def _search_both_excluded_and_ignored_tests(self):
        already_excluded = [test for test in self.ignored_tests if test in self.excluded_tests]
        if not already_excluded:
            return
        Log.summary(_LOGGER, f"Found {len(already_excluded)} tests present both in excluded and ignored test "
                             f"lists.")
        for test in already_excluded:
            Log.all(_LOGGER, f"\t{test}")
            self.ignored_tests.remove(test)

    def _search_not_used_ignored(self, found_tests: List[str]):
        ignored_absent = [test for test in self.ignored_tests if test not in found_tests]
        if ignored_absent:
            Log.summary(_LOGGER, f"Found {len(ignored_absent)} tests in ignored lists but absent on the file system:")
            for test in ignored_absent:
                Log.summary(_LOGGER, f"\t{test}")
        else:
            Log.short(_LOGGER, "All ignored tests are found on the file system")

    @abstractmethod
    def create_test(self, test_file, flags, is_ignored) -> Test:
        pass

    @staticmethod
    def run_test(test):
        return test.run()

    def run(self):
        Log.all(_LOGGER, "Start test running")
        with multiprocessing.Pool(processes=self.config.general.processes) as pool:
            result_iter = pool.imap_unordered(self.run_test, self.tests, chunksize=self.config.general.chunksize)
            pool.close()

            if self.config.general.show_progress:
                result_iter = tqdm(result_iter, total=len(self.tests))

            results = []
            for res in result_iter:
                results.append(res)

            self.results = results
            pool.join()

    @abstractmethod
    def summarize(self):
        pass

    @abstractmethod
    def create_coverage_html(self):
        pass
