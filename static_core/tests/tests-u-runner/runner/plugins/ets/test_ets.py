from __future__ import annotations

from os import path, makedirs
from typing import Tuple, Optional, Sequence, List

from runner.enum_types.configuration_kind import ConfigurationKind
from runner.enum_types.fail_kind import FailKind
from runner.enum_types.params import TestEnv, TestReport, Params
from runner.test_file_based import TestFileBased


class TestETS(TestFileBased):
    def __init__(self, test_env: TestEnv, test_path: str, flags: List[str], test_id: str) -> None:
        TestFileBased.__init__(self, test_env, test_path, flags, test_id)
        # If test fails it contains reason (of FailKind enum) of first failed step
        # It's supposed if the first step is failed then no step is executed further
        self.fail_kind = None
        self.main_entry_point = "ETSGLOBAL::main"
        self.bytecode_path = test_env.work_dir.intermediate
        makedirs(self.bytecode_path, exist_ok=True)
        self.test_abc = path.join(self.bytecode_path, f"{self.test_id}.abc")
        self.test_an = path.join(self.bytecode_path, f"{self.test_id}.an")
        makedirs(path.dirname(self.test_abc), exist_ok=True)

    @property
    def is_negative_runtime(self) -> bool:
        """
        :return: True if a test is expected to fail on ark
        """
        test_basename = path.basename(self.path)
        return test_basename.startswith("n.")

    @property
    def is_negative_compile(self) -> bool:
        """
        :return: True if a test is expected to fail on es2panda
        """
        return False

    @property
    def is_compile_only(self) -> bool:
        """
        :return: True if a test should be run only on es2panda
        """
        return False

    @property
    def is_valid_test(self) -> bool:
        """
        :return: True if a test should be run only
        """
        return True

    @property
    def dependent_files(self) -> Sequence[TestETS]:
        return []

    def do_run(self) -> TestETS:
        for test in self.dependent_files:
            test.do_run()

        if not self.is_valid_test and not self.is_compile_only:
            return self

        if not self.is_valid_test and self.is_compile_only:
            self._run_compiler()
            return self

        self.passed, self.report, self.fail_kind = self._run_compiler()
        if not self.passed or (self.passed and self.is_compile_only):
            return self

        # Run verifier if required
        if self.test_env.config.verifier.enable:
            self.passed, self.report, self.fail_kind = self._run_verifier(self.test_abc)
            if not self.passed:
                return self

        # Run aot if required
        if self.test_env.conf_kind in [ConfigurationKind.AOT, ConfigurationKind.AOT_FULL]:
            self.passed, self.report, self.fail_kind = self.run_aot(
                self.test_an,
                self.test_abc,
                lambda o, e, rc: rc == 0 and path.exists(self.test_an) and path.getsize(self.test_an) > 0
            )

            if not self.passed:
                return self

        self.passed, self.report, self.fail_kind = self.run_runtime(
            self.test_an,
            self.test_abc,
            lambda _, _2, rc: self._runtime_result_validator(rc))
        return self

    def _runtime_result_validator(self, return_code: int) -> bool:
        """
        :return: True if test is successful, False if failed
        """
        if self.is_negative_runtime:
            return return_code != 0

        return return_code == 0

    def _run_compiler(self) -> Tuple[bool, TestReport, Optional[FailKind]]:
        es2panda_flags = []
        if not self.is_valid_test:
            es2panda_flags.append('--ets-module')
        es2panda_flags.extend(self.test_env.es2panda_args)
        es2panda_flags.append(f"--output={self.test_abc}")
        es2panda_flags.append(self.path)

        params = Params(
            executor=self.test_env.es2panda,
            flags=es2panda_flags,
            env=self.test_env.cmd_env,
            timeout=self.test_env.config.es2panda.timeout,
            fail_kind_fail=FailKind.ES2PANDA_FAIL,
            fail_kind_timeout=FailKind.ES2PANDA_TIMEOUT,
            fail_kind_other=FailKind.ES2PANDA_OTHER,
        )

        passed, report, fail_kind = self.run_one_step(
            name="es2panda",
            params=params,
            result_validator=lambda _, _2, rc: self._validate_compiler(rc, self.test_abc)
        )
        return passed, report, fail_kind

    def _validate_compiler(self, return_code: int, output_path: str) -> bool:
        if self.is_negative_compile:
            return return_code != 0
        return return_code == 0 and path.exists(output_path) and path.getsize(output_path) > 0

    def _run_verifier(self, test_abc: str) -> Tuple[bool, TestReport, Optional[FailKind]]:
        assert path.exists(self.test_env.verifier), f"Verifier binary '{self.test_env.verifier}' is absent or not set"
        config_path = self.test_env.config.verifier.config
        if config_path is None:
            config_path = path.join(path.dirname(__file__), 'ets-verifier.config')

        verifier_flags = list(self.verifier_args)
        verifier_flags.append(f"--config-file={config_path}")
        verifier_flags.append(test_abc)

        params = Params(
            executor=self.test_env.verifier,
            flags=verifier_flags,
            env=self.test_env.cmd_env,
            timeout=self.test_env.config.verifier.timeout,
            fail_kind_fail=FailKind.VERIFIER_FAIL,
            fail_kind_timeout=FailKind.VERIFIER_TIMEOUT,
            fail_kind_other=FailKind.VERIFIER_OTHER,
        )
        return self.run_one_step(
            name="verifier",
            params=params,
            result_validator=lambda _, _2, rc: rc == 0,
        )
