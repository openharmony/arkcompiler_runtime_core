#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
import re
from unittest import TestCase
from os import path, makedirs
from pathlib import Path
from typing import Tuple, Optional, Sequence, List, Dict

from runner.utils import compare_files
from runner.plugins.ets.ets_templates.test_metadata import get_metadata, TestMetadata
from runner.enum_types.configuration_kind import ConfigurationKind
from runner.enum_types.fail_kind import FailKind
from runner.enum_types.params import TestEnv, TestReport, Params
from runner.test_file_based import TestFileBased


class TestETS(TestFileBased):
    def __init__(self, test_env: TestEnv, test_path: str, flags: List[str], test_id: str,
                 parent_test_id: str = "") -> None:
        TestFileBased.__init__(self, test_env, test_path, flags, test_id)

        self.metadata: TestMetadata = get_metadata(Path(test_path))
        self.main_entry_point = f"{self.metadata.module}.ETSGLOBAL::main"
        package = self.metadata.get_package_name()
        # Defines if in dependent packages there is at least one file compile-only and negative
        self.dependent_packages: Dict[str, bool] = {
            package: self.is_negative_compile
        }

        # If test fails it contains reason (of FailKind enum) of first failed step
        # It's supposed if the first step is failed then no step is executed further
        self.fail_kind = None

        # parent test id
        self.parent_test_id = parent_test_id

        self.bytecode_path = test_env.work_dir.intermediate
        makedirs(self.bytecode_path, exist_ok=True)
        self.test_abc = path.join(self.bytecode_path, f"{self.test_id}.abc")
        self.test_an = path.join(self.bytecode_path, f"{self.test_id}.an")
        self.test_expected = Path(test_env.work_dir.gen) / f"{self.test_id}.expected"
        self.test_expected_err = Path(test_env.work_dir.gen) / f"{self.test_id}.expected.err"
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
        """ True if test.expected file exists """
        return path.isfile(self.test_expected)

    @property
    def has_expected_err(self) -> bool:
        """ True if test.expected.err file exists """
        return path.isfile(self.test_expected_err)

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
            test = self.__class__(
                test_env=self.test_env,
                test_path=str(test_path),
                flags=self.flags,
                test_id=str(current_test_id.parent / Path(file)),
                parent_test_id=self.test_id)

            prefix = Path(self.parent_test_id).stem if self.parent_test_id else current_test_id.stem

            test_abc_name = f'{prefix}_{Path(test.test_abc).name}'
            test_an_name = f'{prefix}_{Path(test.test_an).name}'

            test.test_abc = str(Path(test.test_abc).parent / Path(test_abc_name))
            test.test_an = str(Path(test.test_abc).parent / Path(test_an_name))

            package = test.metadata.get_package_name()
            self.dependent_packages[package] = self.dependent_packages.get(package, False) or test.is_negative_compile
            if test.dependent_packages:
                for dep_key, dep_item in test.dependent_packages.items():
                    self.dependent_packages[dep_key] = self.dependent_packages.get(dep_key, False) or dep_item

            tests.append(test)
        return tests

    @property
    def runtime_args(self) -> List[str]:
        if not self.dependent_files:
            return super().runtime_args
        return self.add_panda_files(super().runtime_args)

    @property
    def verifier_args(self) -> List[str]:
        if not self.dependent_files:
            return super().verifier_args
        return self.add_panda_files(super().verifier_args)

    @staticmethod
    def _read_file(path_to_file: Path) -> str:
        with open(path_to_file, 'r', encoding='utf-8') as f:
            return ''.join(line for line in f if not line.startswith('#')).strip()

    @staticmethod
    def _normalize_error_report(report: str) -> str:
        pattern = ".(TID .{6}])."
        return re.sub(pattern, "", report)

    def get_all_abc_dependent_files(self) -> List[str]:
        result: List[str] = []
        for dep_file in self.dependent_files:
            result.extend(dep_file.get_all_abc_dependent_files())
            result.append(dep_file.test_abc)

        return result

    def add_panda_files(self, args: List[str]) -> List[str]:
        opt_name = '--panda-files'
        met_panda_files_opt = False
        dep_files_args = []
        for arg in args:
            if opt_name in arg:
                met_panda_files_opt = True
                _, value = arg.split('=')
                opt_value = ":".join([value] + self.get_all_abc_dependent_files())
                arg = f'{opt_name}={opt_value}'
            dep_files_args.append(arg)
        # Add the option only in case of non-empty dependency files list
        if not met_panda_files_opt and len(self.dependent_files) > 0:
            opt_value = ":".join(self.get_all_abc_dependent_files())
            dep_files_args.append(f'{opt_name}={opt_value}')
        return dep_files_args

    def continue_after_process_dependent_files(self) -> bool:
        """
        Processes dependent files
        Returns True if to continue test run
        False - break test run
        """
        for test in self.dependent_files:
            dependent_result = test.do_run()
            self.reproduce += dependent_result.reproduce
            simple_failed = not dependent_result.passed
            negative_compile = dependent_result.passed and dependent_result.is_negative_compile
            dep_package = dependent_result.metadata.get_package_name()
            package_neg_compile = self.dependent_packages.get(dep_package, False)
            if simple_failed or negative_compile or package_neg_compile:
                self.passed = dependent_result.passed if not package_neg_compile else True
                self.report = dependent_result.report
                self.fail_kind = dependent_result.fail_kind
                return False
        return True

    # pylint: disable=too-many-return-statements
    def do_run(self) -> TestETS:
        if not self.continue_after_process_dependent_files():
            return self

        if not self.is_valid_test and not self.is_compile_only:
            return self

        if not self.is_valid_test and self.is_compile_only:
            self.passed, self.report, self.fail_kind = self._run_compiler(self.test_abc)
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
                self.get_all_abc_dependent_files() + [self.test_abc],
                lambda o, e, rc: rc == 0 and path.exists(self.test_an) and path.getsize(self.test_an) > 0
            )

            if not self.passed:
                return self

        self.passed, self.report, self.fail_kind = self.run_runtime(
            self.test_an,
            self.test_abc,
            self._runtime_result_validator)

        return self


    def compare_output_with_expected(self, output: str, err_output: str) -> Tuple[bool, TestReport, Optional[FailKind]]:
        """Compares test output with .expected or .expected.err files"""
        fail_kind = None
        if not self.report:
            return False, TestReport(output="", error="No test report to compare output", return_code=1), fail_kind
        report = self.report
        try:
            self._read_expected_file()
            passed = self.passed
            if not self.test_env.config.ark.jit.enable:
                if self.expected and not self.expected_err:
                    # Compare with output from std.OUT
                    passed = self.expected == output.strip() and not err_output
                elif self.expected_err and not self.expected:
                    # Compare with output from std.ERR
                    report_error = self._normalize_error_report(err_output)
                    passed = self.expected_err == report_error.strip()
                elif self.expected and self.expected_err:
                    # Compare .expected file with std.OUT and .expected.err with std.ERR
                    passed_stdout = self.expected == output.strip()
                    report_error = self._normalize_error_report(err_output)
                    passed_stderr = self.expected_err == report_error.strip()
                    passed = passed_stdout and passed_stderr
            else:
                self._refactor_expected_str_for_jit()
                if self.expected and not self.expected_err:
                    # Compare with output from std.OUT
                    passed = self.expected in output.strip() and not err_output
                elif self.expected_err and not self.expected:
                    # Compare with output from std.ERR
                    report_error = self._normalize_error_report(err_output)
                    passed = self.expected_err in report_error.strip()
                elif self.expected and self.expected_err:
                    # Compare .expected file with std.OUT and .expected.err with std.ERR
                    passed_stdout = self.expected.strip() in output
                    report_error = self._normalize_error_report(err_output)
                    passed_stderr = self.expected_err.strip() in report_error.strip()
                    passed = passed_stdout and passed_stderr

        except OSError as error:
            passed = False
            report = TestReport(error=str(error), output=self.report.output, return_code=self.report.return_code)

        if not passed:
            fail_kind = FailKind.COMPARE_OUTPUT_FAIL
        return bool(passed), report, fail_kind

    def _read_expected_file(self) -> None:
        if self.has_expected:
            self.expected = self._read_file(self.test_expected)
        if self.has_expected_err:
            self.expected_err = self._read_file(self.test_expected_err)

    def _refactor_expected_str_for_jit(self) -> None:
        if self.expected:
            index_to_delete = len(self.expected.split("\n"))
            expected = self.expected.split("\n")
            for i, item in enumerate(expected):
                if '.main' in item:
                    index_to_delete = i
                    break
            self.expected = '\n'.join(expected[:index_to_delete])

        if self.expected_err:
            index_to_delete = len(self.expected_err.split("\n"))
            expected_err = self.expected_err.split("\n")
            for i, item in enumerate(expected_err):
                if '.main' in item:
                    index_to_delete = i
                    break
            self.expected_err = '\n'.join(expected_err[:index_to_delete])

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

    def _runtime_result_validator(self, output: str, err: str, return_code: int) -> bool:
        """
        :return: True if test is successful, False if failed
        """

        if (not self.is_negative_runtime and return_code == 0) or (self.is_negative_runtime and return_code != 0):
            if self.has_expected or self.has_expected_err:
                comparison_res, self.report, self.fail_kind = self.compare_output_with_expected(output, err)

                if not comparison_res:
                    self.log_cmd("The comparison with .expected or .expected.err file failed.")
                    return False

                return True

            if not err:
                # for passed tests stderr should be empty or there should be .expected.err file to compare with
                return True

            self.log_cmd("The test has passed but stdeError is not empty. "
                         "There is no .expected.err file to compare with stdError output.")
            return False

        return False

    def _ts_node_result_validator(self, return_code: int) -> bool:
        """
        :return: True if test is successful, False if failed
        """
        if self.is_negative_runtime:
            return return_code != 0

        return return_code == 0


    def _run_compiler(self, test_abc: str) -> Tuple[bool, TestReport, Optional[FailKind]]:
        es2panda_flags = []
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
        dep_package = self.metadata.get_package_name()
        package_compile = self.dependent_packages.get(dep_package, False)
        if self.is_negative_compile or package_compile:
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
