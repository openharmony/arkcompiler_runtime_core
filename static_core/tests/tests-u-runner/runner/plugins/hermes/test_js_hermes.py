from os import path, makedirs
from typing import List

from runner.enum_types.configuration_kind import ConfigurationKind
from runner.test_file_based import TestFileBased


class TestJSHermes(TestFileBased):
    def __init__(self, test_env, test_path, flags, test_id):
        TestFileBased.__init__(self, test_env, test_path, flags, test_id)
        self.work_dir = test_env.work_dir
        self.util = self.test_env.util

    def do_run(self):
        test_abc = str(self.work_dir.intermediate / f"{self.test_id}.abc")
        test_an = str(self.work_dir.intermediate / f"{self.test_id}.an")
        makedirs(path.dirname(test_abc), exist_ok=True)

        # Run es2panda
        self.passed, self.report, self.fail_kind = self.run_es2panda(
            [],
            test_abc,
            lambda o, e, rc: self._validate_compiler(rc, test_abc)
        )

        if not self.passed:
            return self

        # Run quick if required
        if self.test_env.config.quick.enable:
            ark_flags: List[str] = []
            self.passed, self.report, self.fail_kind, test_abc = self.run_ark_quick(
                ark_flags,
                test_abc,
                lambda o, e, rc: rc == 0
            )

            if not self.passed:
                return self

        # Run aot if required
        if self.test_env.conf_kind in [ConfigurationKind.AOT, ConfigurationKind.AOT_FULL]:
            self.passed, self.report, self.fail_kind = self.run_aot(
                test_an,
                test_abc,
                lambda o, e, rc: rc == 0
            )

            if not self.passed:
                return self

        # Run ark
        self.passed, self.report, self.fail_kind = self.run_runtime(
            test_an,
            test_abc,
            self.ark_validate_result
        )

        return self

    @staticmethod
    def _validate_compiler(return_code: int, output_path: str) -> bool:
        return return_code == 0 and path.exists(output_path) and path.getsize(output_path) > 0

    def ark_validate_result(self, actual_output, _1, return_code):
        return self.util.run_filecheck(self.path, actual_output) and return_code == 0
