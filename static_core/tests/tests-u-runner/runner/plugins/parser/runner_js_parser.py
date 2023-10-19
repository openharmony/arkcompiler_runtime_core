import logging
from os import path
from pathlib import Path

from runner.logger import Log
from runner.options.config import Config
from runner.plugins.parser.test_js_parser import TestJSParser
from runner.runner_base import get_test_id
from runner.runner_js import RunnerJS

_LOGGER = logging.getLogger("runner.plugins.parser.runner_js_parser")


class RunnerJSParser(RunnerJS):
    def __init__(self, config: Config):
        super().__init__(config, "parser")

        # TODO(vpukhov): adjust es2panda path
        es2panda_test = path.join(config.general.panda_source_root, "plugins", "ecmascript", "es2panda", "test")

        self.list_root = es2panda_test if self.list_root is None else self.list_root
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

    def add_directory(self, directory, extension, flags):
        new_dir = path.join(self.test_root, directory)
        super().add_directory(new_dir, extension, flags)

    def create_test(self, test_file, flags, is_ignored) -> TestJSParser:
        test = TestJSParser(self.test_env, test_file, flags, get_test_id(test_file, self.test_root))
        test.ignored = is_ignored
        return test

    @property
    def default_work_dir_root(self) -> Path:
        return Path("/tmp") / "parser"
