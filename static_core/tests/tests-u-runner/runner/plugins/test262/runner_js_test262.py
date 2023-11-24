import logging
from os import path
from pathlib import Path
from typing import List

from runner.logger import Log
from runner.options.config import Config
from runner.plugins.test262.test_js_test262 import TestJSTest262
from runner.plugins.test262.util_test262 import UtilTest262
from runner.runner_base import correct_path, get_test_id
from runner.runner_js import RunnerJS

_LOGGER = logging.getLogger("runner.plugins.test262.runner_js_test262")


class RunnerJSTest262(RunnerJS):
    def __init__(self, config: Config) -> None:
        RunnerJS.__init__(self, config, "test262")
        self.ignored_name_prefix = "test262"

        self.list_root = self.list_root if self.list_root else path.join(self.default_list_root, self.name)
        Log.summary(_LOGGER, f"LIST_ROOT set to {self.list_root}")

        self.collect_excluded_test_lists(test_name=self.ignored_name_prefix)
        self.collect_ignored_test_lists(test_name=self.ignored_name_prefix)

        self.util = UtilTest262(config=self.config, work_dir=self.work_dir)

        self.test_root = self.util.generate(
            harness_path=path.join(path.dirname(__file__), "test262harness.js"),
        )
        Log.summary(_LOGGER, f"TEST_ROOT reset to {self.test_root}")
        self.test_env.util = self.util

        if self.config.general.bco:
            self.bco_list = correct_path(self.list_root, f"{self.ignored_name_prefix}skiplist-bco.txt")
            self.bco_tests = self.load_tests_from_lists([self.bco_list])

        self.add_directory(self.test_root, "js", [])

    def create_test(self, test_file: str, flags: List[str], is_ignored: bool) -> TestJSTest262:
        with_optimizer = test_file not in self.bco_tests
        test = TestJSTest262(self.test_env, test_file, flags, with_optimizer, get_test_id(test_file, self.test_root))
        test.ignored = is_ignored
        return test

    @property
    def default_work_dir_root(self) -> Path:
        return Path("/tmp") / "test262"
