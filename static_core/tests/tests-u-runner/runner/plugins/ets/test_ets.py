#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

from __future__ import annotations

from unittest import TestCase
from os import path, makedirs
from pathlib import Path
from typing import Tuple, Optional, Sequence, List
from runner.utils import compare_files

from runner.plugins.ets.ets_templates.test_metadata import get_metadata, TestMetadata
from runner.enum_types.configuration_kind import ConfigurationKind
from runner.enum_types.fail_kind import FailKind
from runner.enum_types.params import TestEnv, TestReport, Params
from runner.test_file_based import TestFileBased


class TestETS(TestFileBased):
    def __init__(self, test_env: TestEnv, test_path: str, flags: List[str], test_id: str) -> None:
        TestFileBased.__init__(self, test_env, test_path, flags, test_id)

        self.main_entry_point = "ETSGLOBAL::main"
        self.metadata: TestMetadata = get_metadata(Path(test_path))
        if self.metadata.package is not None:
            self.main_entry_point = f"{self.metadata.package}.{self.main_entry_point}"

        # If test fails it contains reason (of FailKind enum) of first failed step
        # It's supposed if the first step is failed then no step is executed further
        self.fail_kind = None

        self.bytecode_path = test_env.work_dir.intermediate
        makedirs(self.bytecode_path, exist_ok=True)
        self.test_abc = path.join(self.bytecode_path, f"{self.test_id}.abc")
        self.test_an = path.join(self.bytecode_path, f"{self.test_id}.an")
        self.test_expected = path.join(test_env.work_dir.gen, f"{self.test_id}.expected")
        makedirs(path.dirname(self.test_abc), exist_ok=True)

    @property
    def is_negative_runtime(self) -> bool:
        """ True if a test is expected to fail on ark """
        negative_runtime_metadata = self.metadata.tags.negative and not self.metadata.tags.compile_only
        return negative_runtime_metadata or path.basename(self.path).startswith("n.")

    @property
    def is_negative_compile(self) -> bool:
        """ True if a test is expected to fail on es2panda """
        return self.metadata.tags.negative and self.metadata.tags.compile_only

    @property
    def is_compile_only(self) -> bool:
        """ True if a test should be run only on es2panda """
        return self.metadata.tags.compile_only

    @property
    def is_valid_test(self) -> bool:
        """ True if a test is valid """
        return not self.metadata.tags.not_a_test

    @property
    def has_expected(self) -> bool:
        """True if test.expected file exists"""
        return path.isfile(self.test_expected)

    @property
    def ark_extra_options(self) -> List[str]:
        return self.metadata.ark_options

    @property
    def ark_timeout(self) -> int:
        return self.metadata.timeout if self.metadata.timeout else super().ark_timeout

    @property
    def dependent_files(self) -> Sequence[TestETS]:
        if not self.metadata.files:
            return []

        tests = []
        for file in self.metadata.files:
            test_path = Path(self.path).parent / Path(file)
            current_test_id = Path(self.test_id)
            test = self.__class__(self.test_env, str(test_path), self.flags, str(current_test_id.parent / Path(file)))

            test_abc_name = f'{current_test_id.stem}_{Path(test.test_abc).name}'
            test_an_name = f'{current_test_id.stem}_{Path(test.test_an).name}'

            test.test_abc = str(Path(test.test_abc).parent / Path(test_abc_name))
            test.test_an = str(Path(test.test_abc).parent / Path(test_an_name))

            tests.append(test)
        return tests

    @property
    def runtime_args(self) -> List[str]:
        if not self.dependent_files:
            return super().runtime_args
        return self.add_boot_panda_files(super().runtime_args)

    @property
    def verifier_args(self) -> List[str]:
        if not self.dependent_files:
            return super().verifier_args
        return self.add_boot_panda_files(super().verifier_args)

    def add_boot_panda_files(self, args: List[str]) -> List[str]:
        dep_files_args = []
        for arg in args:
            name = '--boot-panda-files'
            if name in arg:
                _, value = arg.split('=')
                dep_files_args.append(f'{name}={":".join([value] + [dt.test_abc for dt in self.dependent_files])}')
            else:
                dep_files_args.append(arg)
        return dep_files_args

    # pylint: disable=too-many-return-statements
    def do_run(self) -> TestETS:
        for test in self.dependent_files:
            test.do_run()

        if not self.is_valid_test and not self.is_compile_only:
            return self

        if not self.is_valid_test and self.is_compile_only:
            self._run_compiler(self.test_abc)
            return self

        if self.test_env.config.ets.compare_files:
            return self._run_compare_mode()
        self.passed, self.report, self.fail_kind = self._run_compiler(self.test_abc)

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
                [test.test_abc for test in list(self.dependent_files) + [self]],
                lambda o, e, rc: rc == 0 and path.exists(self.test_an) and path.getsize(self.test_an) > 0
            )

            if not self.passed:
                return self

        self.passed, self.report, self.fail_kind = self.run_runtime(
            self.test_an,
            self.test_abc,
            lambda _, _2, rc: self._runtime_result_validator(rc))

        if self.has_expected and self.passed:
            self.passed, self.report, self.fail_kind = self._compare_output_with_expected()
        return self

    def _read_expected_file(self) -> None:
        with open(self.test_expected, 'r', encoding="utf-8") as file_pointer:
            expected = ""
            for line in file_pointer.readlines():
                if line.startswith("#"):
                    continue
                expected += line

            self.expected = expected


    def _compare_output_with_expected(self) -> Tuple[bool, TestReport, Optional[FailKind]]:
        "Compares test output with expected"
        fail_kind = None
        if not self.report:
            return False, TestReport(output="", error="No test report to compare output", return_code=1), fail_kind
        report = self.report
        try:
            self._read_expected_file()
            if not self.test_env.config.ark.jit.enable:
                passed = self.expected == self.report.output + '\n' and self.report.return_code in [0, 1]
            else:
                passed = self.expected in self.report.output + '\n' and self.report.return_code in [0, 1]
        except OSError as error:
            passed = False
            report = TestReport(error=str(error), output=self.report.output, return_code=self.report.return_code)

        if not passed:
            fail_kind = FailKind.COMPARE_OUTPUT_FAIL
        return passed, report, fail_kind

    def _run_compare_mode(self) -> TestETS:
        files = []
        iterations = self.test_env.config.ets.compare_files_iterations
        for i in range(iterations):
            if i != 0:
                test_ets = self.bytecode_path / self.test_id
                stem, suffix_ets = test_ets.stem, test_ets.suffix
                suffix_abc = ".abc"
                test_abc = test_ets.parent / Path(f'{stem}_{str(i + 1)}{suffix_ets}{suffix_abc}')
            else:
                test_abc = Path(self.test_abc)

            files.append(test_abc)
            self.passed, self.report, self.fail_kind = self._run_compiler(str(test_abc))
            if not self.passed:
                return self

        self.passed = compare_files(files)
        if not self.passed:
            self.fail_kind = FailKind.COMPARE_FAIL
        return self

    def _runtime_result_validator(self, return_code: int) -> bool:
        """
        :return: True if test is successful, False if failed
        """
        if self.is_negative_runtime:
            return return_code != 0

        return return_code == 0

    def _run_compiler(self, test_abc: str) -> Tuple[bool, TestReport, Optional[FailKind]]:
        es2panda_flags = []
        if not self.is_valid_test:
            es2panda_flags.append('--ets-module')
        es2panda_flags.extend(self.test_env.es2panda_args)
        es2panda_flags.append(f"--output={test_abc}")
        es2panda_flags.append(self.path)

        params = Params(
            executor=self.test_env.es2panda,
            flags=es2panda_flags,
            env=self.test_env.cmd_env,
            timeout=self.test_env.config.es2panda.timeout,
            gdb_timeout=self.test_env.config.general.gdb_timeout,
            fail_kind_fail=FailKind.ES2PANDA_FAIL,
            fail_kind_timeout=FailKind.ES2PANDA_TIMEOUT,
            fail_kind_other=FailKind.ES2PANDA_OTHER,
        )

        passed, report, fail_kind = self.run_one_step(
            name="es2panda",
            params=params,
            result_validator=lambda _, _2, rc: self._validate_compiler(rc, test_abc)
        )
        if fail_kind == FailKind.ES2PANDA_FAIL and report and report.return_code == 0:
            fail_kind = FailKind.ES2PANDA_NEG_FAIL
        return passed, report, fail_kind

    def _validate_compiler(self, return_code: int, output_path: str) -> bool:
        if self.is_negative_compile:
            return return_code == self.CTE_RETURN_CODE
        return return_code == 0 and path.exists(output_path) and path.getsize(output_path) > 0

    def _run_verifier(self, test_abc: str) -> Tuple[bool, TestReport, Optional[FailKind]]:
        TestCase().assertTrue(
            path.exists(self.test_env.verifier),
            f"Verifier binary '{self.test_env.verifier}' is absent or not set"
        )
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
            gdb_timeout=self.test_env.config.general.gdb_timeout,
            fail_kind_fail=FailKind.VERIFIER_FAIL,
            fail_kind_timeout=FailKind.VERIFIER_TIMEOUT,
            fail_kind_other=FailKind.VERIFIER_OTHER,
        )
        return self.run_one_step(
            name="verifier",
            params=params,
            result_validator=lambda _, _2, rc: rc == 0,
        )
