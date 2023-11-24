import logging
from os import path
from typing import List, Any

from runner.descriptor import Descriptor
from runner.utils import write_2_file
from runner.test_file_based import TestFileBased
from runner.enum_types.params import TestEnv

_LOGGER = logging.getLogger("runner.plugins.js_parser")


class TestJSParser(TestFileBased):
    def __init__(self, test_env: TestEnv, test_path: str, flags: List[str], test_id: str) -> None:
        TestFileBased.__init__(self, test_env, test_path, flags, test_id)
        self.expected_path = f"{path.splitext(self.path)[0]}-expected.txt"

    def do_run(self) -> TestFileBased:
        desc = Descriptor(self.path).get_descriptor()

        if 'flags' in desc and 'dynamic-ast' in desc['flags']:
            es2panda_flags = ["--dump-dynamic-ast"]
        else:
            es2panda_flags = ["--dump-ast"]
        es2panda_flags.extend(self.flags)
        if 'flags' in desc and 'module' in desc['flags']:
            es2panda_flags.append("--module")

        self.passed, self.report, self.fail_kind = self.run_es2panda(
            flags=es2panda_flags,
            test_abc="",
            result_validator=self.es2panda_result_validator
        )

        if self.update_expected and self.report:
            self.update_expected_files(self.report.output)

        return self

    def update_expected_files(self, output: str) -> None:
        write_2_file(self.expected_path, output)

    def es2panda_result_validator(self, actual_output: str, _: Any, actual_return_code: int) -> bool:
        try:
            with open(self.expected_path, 'r', encoding="utf-8") as file_pointer:
                self.expected = file_pointer.read()
            passed = self.expected == actual_output and actual_return_code in [0, 1]
        except OSError:
            passed = False

        return passed
