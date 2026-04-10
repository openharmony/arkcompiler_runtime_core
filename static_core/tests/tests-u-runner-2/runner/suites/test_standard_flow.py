#!/usr/bin/env python3
# -- coding: utf-8 --
#
# Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

import re
from collections import OrderedDict
from collections.abc import Sequence
from copy import deepcopy
from dataclasses import replace
from fnmatch import translate
from pathlib import Path
from typing import TYPE_CHECKING, cast

from typing_extensions import Self

from runner import utils
from runner.common_exceptions import InvalidConfiguration, MacroNotExpanded
from runner.enum_types.fail_kind import FailKind
from runner.enum_types.validation_result import ValidationResult, ValidatorFailKind
from runner.extensions.flows.itest_flow import ITestFlow
from runner.extensions.flows.test_flow_registry import workflow_registry
from runner.extensions.validators.base_validator import BaseValidator
from runner.extensions.validators.ivalidator import IValidator
from runner.logger import Log
from runner.macro_utils import has_macro, list_has_macros
from runner.options.macros import Macros
from runner.options.options import IOptions
from runner.options.options_step import Step, StepKind
from runner.options.options_step_utils import StepFields
from runner.suites.comparison_utils import ComparisonUtils
from runner.suites.one_step_runner import OneStepRunner
from runner.suites.test_metadata import TestMetadata
from runner.test_base import Test
from runner.types.step_report import StepReport
from runner.types.test_env import TestEnv
from runner.utils import get_validator_class

if TYPE_CHECKING:
    pass

_LOGGER = Log.get_logger(__file__)


class StandardFlowUtils:

    def __init__(self, test_env: TestEnv):
        self.test_env = test_env

    def get_step_env(self, step: Step) -> dict[str, str]:
        cmd_env = self.test_env.cmd_env
        if step.env:
            cmd_env = deepcopy(self.test_env.cmd_env)
            for env_item in step.env:
                env_value: str | list[str] = step.env[env_item]
                if isinstance(env_value, list):
                    cmd_env[env_item] = "".join(env_value)
                else:
                    cmd_env[env_item] = env_value
        return cmd_env


@workflow_registry.register("ets")
def make_test_standard_flow(test_env: TestEnv, test_path: Path, *, params: IOptions, test_id: str) -> ITestFlow:
    return TestStandardFlow(test_env, test_path, params=params, test_id=test_id)


class TestStandardFlow(ITestFlow, Test):
    __DEFAULT_ENTRY_POINT = "ETSGLOBAL::main"
    CTE_RETURN_CODE = 1
    metadata: TestMetadata
    flow_utils: StandardFlowUtils
    comparison_utils: ComparisonUtils = ComparisonUtils()

    def __init__(self, test_env: TestEnv, test_path: Path, *,
                 params: IOptions, test_id: str, is_dependent: bool = False,
                 parent_test_id: str = "") -> None:

        Test.__init__(self, test_env, test_path, params, test_id)
        self.metadata = TestMetadata.get_metadata(test_path) if test_env.config.test_suite.use_metadata \
            else TestMetadata.create_empty_metadata(test_path)
        self._init_bytecode_paths(test_env)

        self.test_cli: list[str] = self.metadata.test_cli or []
        self.main_entry_point: str = (
            f"ETSGLOBAL::{self.metadata.entry_point}"
            if self.metadata.entry_point else self.__DEFAULT_ENTRY_POINT
        )
        if self.metadata.package is not None:
            self.main_entry_point = f"{self.metadata.package}.{self.main_entry_point}"
        else:
            self.main_entry_point = f"{test_path.stem}.{self.main_entry_point}"

        # Defines if in dependent packages there is at least one file compile-only and negative
        self.dependent_packages: dict[str, bool] = {
            'package': self.is_negative_compile
        }

        self.parent_test_id = parent_test_id

        self.is_panda = test_env.config.workflow.is_panda

        # If test fails it contains reason of first failed step
        # It's supposed if the first step is failed then no step is executed further
        self.fail_kind: str | None = None

        self.expected_err_log = ""
        self.flow_utils = StandardFlowUtils(test_env)
        self.validator_utils = ValidatorUtils(self, test_env)
        self.runtime_steps = [step for step in self.test_env.config.workflow.steps
                              if step.step_kind not in (StepKind.COMPILER, StepKind.VERIFIER)]
        self._dependent_tests: list[TestStandardFlow] = []
        self.__is_dependent = is_dependent
        self.__boot_panda_files: str = ""
        self._is_declgen = any(step.step_kind == StepKind.DECLGEN for step in self.test_env.config.workflow.steps)

    @property
    def is_negative_runtime(self) -> bool:
        """ True if a test is expected to fail on ark """
        negative_runtime_metadata = self.metadata.tags.negative and not self.metadata.tags.compile_only
        return negative_runtime_metadata or self.path.stem.startswith("n.")

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
    def dependent_tests(self) -> Sequence['TestStandardFlow']:
        if not self.metadata.files:
            return []

        if self._dependent_tests:
            return self._dependent_tests

        self._dependent_tests.extend(self.search_dependent_tests())
        for test in self._dependent_tests:
            package = test.metadata.get_package_name()
            self.dependent_packages[package] = self.dependent_packages.get(package, False) or test.is_negative_compile
            if test.dependent_packages:
                for dep_key, dep_item in test.dependent_packages.items():
                    self.dependent_packages[dep_key] = self.dependent_packages.get(dep_key, False) or dep_item
        return self._dependent_tests

    @property
    def dependent_abc_files(self) -> list[str]:
        return list(OrderedDict.fromkeys(
            abc_file
            for df in self.dependent_tests
            for abc_file in [*df.dependent_abc_files, df.test_abc.as_posix()]
        ))

    @property
    def invalid_tags(self) -> list:
        return self.metadata.tags.invalid_tags

    @property
    def negative_steps(self) -> list[str]:
        return self.metadata.negative_steps

    @property
    def has_expected(self) -> bool:
        """ True if test.expected file exists or expected_out is in metadata """
        return super().has_expected or bool(self.metadata.expected_out)

    @property
    def has_expected_err(self) -> bool:
        """ True if test.expected.err file exists or expected_error is in metadata """
        return super().has_expected_err or bool(self.metadata.expected_error)

    @property
    def continue_if_failed(self) -> bool:
        return self.test_env.config.general.continue_if_failed

    @staticmethod
    def __add_options(options: list[str]) -> list[str]:
        return [opt if opt.startswith("--") else f"--{opt}" for opt in options]

    @staticmethod
    def _get_return_code_from_device(output: str, actual_return_code: int) -> int:
        if output.find('TypeError:') > -1 or output.find('FatalOutOfMemoryError:') > -1:
            return actual_return_code if actual_return_code else -1
        match = re.search(r'Exit code:\s*(-?\d+)', output)
        if match:
            return_code_from_device = int(match.group(1))
            return return_code_from_device
        return actual_return_code

    @staticmethod
    def _remove_main_decl(lines: str) -> str:
        expected = lines.split("\n")
        index_to_delete = len(expected)
        for i, item in enumerate(expected):
            if '.main' in item or ':main' in item:
                index_to_delete = i
                break

        return '\n'.join(expected[:index_to_delete])

    @staticmethod
    def _clarify_fail_kind(steps: list[Step]) -> str | None:
        failed = [step for step in steps if step.passed is False]
        if len(failed) > 1:
            return 'MULTIPLE_STEPS_FAILED'
        if len(failed) == 1:
            return f'{failed[0].name.upper()}_FAIL'
        return None

    def do_run(self) -> Self:
        if self.is_completed:
            return self
        if not self._continue_after_process_dependent_files():
            self.is_completed = True
            return self

        self._log_invalid_tags_if_any()
        compile_only_test = self._is_compile_only_test()
        allowed_steps = [StepKind.COMPILER, StepKind.DECLGEN]  # steps to run for compile only or not-a-test tests
        steps = self._collect_steps(compile_only_test, allowed_steps)
        for i, step in enumerate(steps):
            pattern = re.compile(translate(step.step_filter))
            if not pattern.search(str(self.path)):
                continue
            self.passed = self._do_run_one_step(step)
            self.fail_kind = self.step_reports[-1].status
            if step.step_kind in allowed_steps:
                allowed_steps.remove(step.step_kind)

            steps[i] = replace(step, passed=self.passed)

            if ((not self.passed and not self.continue_if_failed)
                    or (compile_only_test and not allowed_steps)):
                self.is_completed = True
                return self

        if not steps:
            self._mark_test_exclude()

        if self.continue_if_failed:
            self._finalize_test_status(steps)

        self.is_completed = True
        return self

    def search_dependent_tests(self) -> list['TestStandardFlow']:
        if not self.metadata.files:
            return []

        if files := [file for file in self.metadata.files if file.endswith(".d.ets")]:
            raise InvalidConfiguration(f"Test {self.test_id} should not refer to a dependent .d.ets file(s): {files}")

        id_pos: int = str(self.path).find(self.test_id)
        test_root: str = str(self.path)[:id_pos]
        dependent_tests: list[TestStandardFlow] = []

        for file in self.metadata.files:
            new_test_file_path: Path = self.path.parent.joinpath(file).resolve()
            new_test_id: str = new_test_file_path.relative_to(test_root).as_posix()

            if (test := self.test_env.test_flow_registry.find(new_test_id)) is None:
                test = self.__class__(test_env=self.test_env,
                                      test_path=new_test_file_path,
                                      params=self.test_extra_params,
                                      test_id=new_test_id,
                                      is_dependent=True,
                                      parent_test_id=self.test_id)
                test = self.test_env.test_flow_registry.register(test)

            dependent_tests.append(cast(TestStandardFlow, test))

        return dependent_tests

    def prepare_compiler_step(self, step: Step) -> Step:
        if self.__is_dependent:
            args = self.__change_output_arg(step.args)
            args = self.__change_arktsconfig_arg(args)
        else:
            args = self.__change_arktsconfig_arg(step.args)
        return replace(step, args=args)

    def prepare_verifier_step(self, step: Step) -> Step:
        if self.dependent_tests and self.is_panda:
            args = self.__add_boot_panda_files(step.args)
            return replace(step, args=args)
        return step

    def prepare_aot_step(self, step: Step) -> Step:
        if not self.is_panda:
            return step
        args = step.args[:]
        if self.dependent_tests:
            for abc_file in list(self.dependent_abc_files):
                args.extend([f'--paoc-panda-files={abc_file}'])
        args.extend([f'--paoc-panda-files={self.test_abc}'])
        return replace(step, args=args)

    def prepare_runtime_step(self, step: Step) -> Step:
        if not self.is_panda:
            return step
        args = step.args[:]
        args.insert(-2, "--verification-mode=ahead-of-time")

        args.insert(-2, self.__add_panda_files())
        if self.metadata.test_cli:
            args.append("--")
            args.extend(self.metadata.test_cli)

        return replace(step, args=args)

    def compare_with_stdout(self, output: str, step_fields: StepFields) -> tuple[bool, str]:
        """
        Compare test stdout with expected output
        """
        try:
            self._get_expected_data()

            step_name = step_fields.step_name
            step_kind = step_fields.step_kind

            expected_output = self.expected.get(step_name) or self.expected.get(step_kind)
            if not expected_output:
                return True, ""
            passed, message = self._determine_test_status(output, expected_output)
        except OSError:
            message = "Failed to read expected data"
            passed = False
            _LOGGER.all(message)

        return passed, message

    def compare_with_stderr(self, error: str, step_fields: StepFields) -> tuple[bool, str]:
        """
        Compare test stderr with expected output
        """
        try:
            self._get_expected_data()
            step_name = step_fields.step_name
            step_kind = step_fields.step_kind

            expected_err = self.expected_err.get(step_name) or self.expected_err.get(step_kind)
            if not expected_err:
                return False, "There is no expected stderr to compare with"

            passed, message = self._determine_test_status(error, expected_err)
        except OSError:
            message = "Failed to read expected data"
            passed = False
            _LOGGER.all(message)

        return passed, message

    def configure_step_last_call(self, step: Step) -> Step:
        """
        Configures and prepares a test step for execution by expanding macros and applying step-specific modifications.

        This method performs the final configuration of a test step before execution:
        - Expands all macros in step arguments, stdout/stderr paths, and requirements
        - Applies compiler-specific options from test metadata (es2panda_options)
        - Applies runtime-specific options from test metadata (ark_options)
        - Handles dynamic AST dumping and module flags for compiler steps
        - Prepares environment variables for the step

        Args:
            step (Step): The original step to configure. Contains arguments, paths, requirements,
                        environment variables, and step kind information.

        Returns:
            Step: A new Step instance with all macros expanded and step-specific modifications applied.
                 The original step remains unchanged.

        Note:
            - For compiler steps: applies es2panda_options like --dump-dynamic-ast and --module
            - For runtime steps: prepends ark_options to the argument list
            - Macros are expanded recursively until no macros remain in the arguments
            - Environment variables are resolved from the step configuration and test environment
        """
        cmd_env = self.flow_utils.get_step_env(step)
        flags = step.args[:]
        while list_has_macros(flags):
            flags = self.__expand_last_call_in_args(flags)
        stdout = self.__expand_last_call_in_path(step.stdout) if step.stdout else step.stdout
        stderr = self.__expand_last_call_in_path(step.stderr) if step.stderr else step.stderr
        pre_requirements = [replace(req, value=self.__expand_last_call_in_args([req.value])[0])
                            for req in step.pre_requirements]
        post_requirements = [replace(req, value=self.__expand_last_call_in_args([req.value])[0])
                             for req in step.post_requirements]
        if step.step_kind == StepKind.COMPILER and self.metadata.es2panda_options:
            if 'dynamic-ast' in self.metadata.es2panda_options:
                index = flags.index("--dump-ast")
                flags[index] = "--dump-dynamic-ast"
            if 'module' in self.metadata.es2panda_options:
                flags.insert(0, "--module")
        if step.step_kind == StepKind.RUNTIME and self.metadata.ark_options:
            prepend_options = self.__add_options(self.metadata.ark_options)
            flags = prepend_options + flags
        return replace(step, args=flags, stdout=stdout, stderr=stderr, pre_requirements=pre_requirements,
                       post_requirements=post_requirements, env=cmd_env)

    def _continue_after_process_dependent_files(self) -> bool:
        """
        Processes dependent files
        Returns True if to continue test run
        False - break test run
        """
        for test in self.dependent_tests:
            with test.run_lock:
                dependent_result = test.do_run()
            for dependent_report in dependent_result.step_reports:
                self.add_step_report(dependent_report)
            simple_failed = not dependent_result.passed
            negative_compile = dependent_result.passed and dependent_result.is_negative_compile
            dep_package = dependent_result.metadata.get_package_name()
            package_neg_compile = self.dependent_packages.get(dep_package, False)
            if simple_failed or negative_compile or package_neg_compile:
                self.passed = dependent_result.passed if not package_neg_compile else True
                self.excluded = dependent_result.excluded
                self.fail_kind = dependent_result.fail_kind
                self.last_failure_check_passed = dependent_result.last_failure_check_passed
                return False
        return True

    def _log_invalid_tags_if_any(self) -> None:
        if len(self.invalid_tags) > 0:
            _LOGGER.default(
                f"\n{utils.FontColor.RED_BOLD.value}Invalid tags:{utils.FontColor.RESET.value} `"
                f"{', '.join(self.invalid_tags)}` in test file: {self.test_id}")

    def _is_compile_only_test(self) -> bool:
        return self.is_compile_only or self.metadata.tags.not_a_test or self.parent_test_id != ""

    def _read_expected_or_metadata(self, path: Path,
                                   metadata_value: dict[str, list[str]] | None) -> dict[str, list[str]]:

        expected_data = utils.read_expected_file(path)

        if expected_data is None:
            return metadata_value if metadata_value is not None else {}

        key = StepKind.COMPILER.value if self.is_compile_only else StepKind.RUNTIME.value
        return {key: expected_data}

    def _get_expected_data(self) -> None:
        if self.has_expected:
            self.expected = self._read_expected_or_metadata(
                self.path_to_expected,
                self.metadata.expected_out,
            )

        if self.has_expected_err:
            self.expected_err = self._read_expected_or_metadata(
                self.path_to_expected_err,
                self.metadata.expected_error,
            )

    def _determine_test_status(self, actual_output: str, expected_output: list[str]) -> tuple[bool, str]:
        actual = ComparisonUtils.normalize_output_lines(actual_output.splitlines())
        expected = ComparisonUtils.normalize_output_lines(expected_output)
        passed, msg = ComparisonUtils.compare_line_sets(expected, actual, self.path_to_expected_err)

        if not passed:
            differences = ComparisonUtils.log_comparison_difference(actual, expected)
            if differences:
                msg += "\n".join(differences)

        return passed, msg

    def _check_expected_err(self, error: str | None) -> bool:
        # to cover case: there is no expected_err file, but stderr is not empty
        # or there is expected err file, but it's empty
        passed = True
        if (not self.has_expected_err and error) or (not self.expected_err and error):
            passed = False

        return passed

    def _do_run_one_step(self, step: Step) -> bool:
        if not step.enabled:
            passed = True
        elif step.step_kind == StepKind.COMPILER:
            passed = self.__run_step(self.prepare_compiler_step(step))
        elif step.step_kind == StepKind.VERIFIER:
            passed = self.__run_step(self.prepare_verifier_step(step))
        elif step.step_kind == StepKind.AOT:
            passed = self.__run_step(self.prepare_aot_step(step))
        elif step.step_kind == StepKind.RUNTIME:
            passed = self.__run_step(self.prepare_runtime_step(step))
        else:
            passed = self.__run_step(step)
        return passed

    def _init_bytecode_paths(self, test_env: TestEnv) -> None:
        self.bytecode_path: Path = test_env.work_dir.intermediate
        self.test_abc: Path = self.bytecode_path / f"{self.test_id}.abc"
        self.test_an: Path = self.bytecode_path / f"{self.test_id}.an"
        self.test_abc.parent.mkdir(parents=True, exist_ok=True)

    def _mark_test_exclude(self) -> None:
        # no step runs, so nothing bad occurs, and we consider the test is passed
        # but set the flag `excluded` to track correct statistics
        # The test without steps is considered as self-excluded or excluded-after
        self.passed = True
        self.fail_kind = FailKind.NOT_RUN.make_fail_kind("")
        self.excluded = True

    def _collect_steps(self, compile_only_test: bool, allowed_steps: list[StepKind]) -> list[Step]:
        return [step for step in self.test_env.config.workflow.steps
                if step.executable_path is not None and
                ((compile_only_test and step.step_kind in allowed_steps) or not compile_only_test)]

    def _finalize_test_status(self, steps: list[Step]) -> None:
        ran_steps = [step for step in steps if step.passed is not None]
        self.passed = True if not ran_steps else all(s.passed for s in ran_steps)
        if not self.passed:
            clarified_failed_kind = TestStandardFlow._clarify_fail_kind(steps)
            if clarified_failed_kind is not None:
                self.fail_kind = clarified_failed_kind

    def __fix_entry_point(self, args: list[str]) -> list[str]:
        result: list[str] = args[:]
        for index, arg in enumerate(result):
            if self.__DEFAULT_ENTRY_POINT in str(arg):
                result[index] = arg.replace(self.__DEFAULT_ENTRY_POINT, self.main_entry_point).strip()
        return [res for res in result if res]

    def __change_output_arg(self, args: list[str]) -> list[str]:
        new_args = args[:]
        for index, arg in enumerate(args):
            if arg.startswith("--output="):
                new_args[index] = f"--output={self.test_abc}"
                break
        return new_args

    def __change_arktsconfig_arg(self, args: list[str]) -> list[str]:
        new_args = args[:]
        for index, arg in enumerate(args):
            if arg.startswith("--arktsconfig=") and self.metadata.arktsconfig is not None:
                stdlib_path = self.test_env.config.general.static_core_root / 'plugins' / 'ets' / 'stdlib'
                new_args[index] = f"--arktsconfig={self.metadata.arktsconfig}"
                new_args.insert(0, f"--stdlib={stdlib_path.as_posix()}")
                break
        return new_args

    def __run_step(self, orig_step: Step) -> bool:
        step = self.configure_step_last_call(orig_step)

        pre_reqs_succeed, pre_reqs_result = step.check_pre_requirements()
        if not pre_reqs_succeed:
            errors = "\n".join(pre_reqs_result)
            test_report = StepReport(name=orig_step.name, status=f"{step.name.upper()}_PRE_REQS_FAILED",
                                     validator_messages=errors)
            self.add_step_report(test_report)
            return False

        step_runner = OneStepRunner(step, self.test_env)
        passed = step_runner.run_with_coverage(
            result_validator=lambda out, err, return_code:
                self.validator_utils.step_validator(step, out, err, return_code),
            return_code_interpreter=lambda out, err, return_code: self._get_return_code_from_device(out, return_code)
        )
        self.add_step_report(step_runner.report)
        return passed

    def __expand_last_call_in_path(self, arg: Path) -> Path:
        line = arg.as_posix()
        line = self.__expand_last_call_in_line(line)[0]
        return Path(line)

    def __expand_last_call_in_line(self, arg: str) -> list[str]:
        line: str = arg
        while has_macro(line):
            line = Macros.process_special_macros(line, self.test_id)
            try:
                expanded_line: str | list[str] = Macros.correct_macro(line, self.test_extra_params)
            except MacroNotExpanded:
                expanded_line = Macros.correct_macro(line, self.test_env.config.workflow)
            line = " ".join(expanded_line) if isinstance(expanded_line, list) else expanded_line
        return line.split()

    def __expand_last_call_in_args(self, args: list[str]) -> list[str]:
        flags: list[str] = []
        for arg in self.__fix_entry_point(args):
            flags.extend(self.__expand_last_call_in_line(str(arg)))
        return flags

    def __add_panda_files(self) -> str:
        opt_name = '--panda-files'
        if self.dependent_abc_files:
            return f'{opt_name}={":".join(self.dependent_abc_files)}'

        return f'{opt_name}={self.test_abc!s}'

    def __add_boot_panda_files(self, args: list[str]) -> list[str]:
        dep_files_args: list[str] = []
        for arg in args:
            name = '--boot-panda-files'
            if name in arg:
                if not self.__boot_panda_files:
                    _, value = arg.split('=')
                    boot_panda_files = [value, *self.dependent_abc_files, self.test_abc.as_posix()]
                    self.__boot_panda_files = f'{name}={":".join(boot_panda_files)}'
                dep_files_args.append(self.__boot_panda_files)
            else:
                dep_files_args.append(arg)
        return dep_files_args


class ValidatorUtils:

    def __init__(self, flow_type: TestStandardFlow, test_env: TestEnv):
        self.test_env = test_env
        self.validator: IValidator = self._init_validator()
        self._flow_type: TestStandardFlow = flow_type

    def step_validator(self, step: Step, output: str, error: str, return_code: int) -> ValidationResult:
        validator_name = step.name if step.name in self.validator.validators else step.step_kind.value
        validator = self.validator.get_validator(validator_name) \
            if step.validator is None \
            else step.validator.get_validator(validator_name)
        step_fields = StepFields(step_kind=step.step_kind.value, step_name=step.name)
        if validator is not None:
            for validator_func in validator:
                step_validator_result = validator_func(self._flow_type, step_fields,
                                                       output, error, return_code)
                if not step_validator_result.passed:
                    return ValidationResult(step_validator_result.passed,
                                            step_validator_result.kind,
                                            step_validator_result.description)

        post_reqs_succeed, post_reqs_result = step.check_post_requirements()
        if not post_reqs_succeed:
            post_failures_str = "\n".join(post_reqs_result)
            return ValidationResult(passed=False,
                                    kind=ValidatorFailKind.POST_REQ_FAILED,
                                    description=f"{step.name.upper()}_POST_REQS_FAILED: {post_failures_str}")

        return ValidationResult(True, ValidatorFailKind.NONE, "")

    def _init_validator(self) -> IValidator:
        validator_class_name = self.test_env.config.test_suite.validator_class
        if validator_class_name is None:
            return BaseValidator()
        validator_class = get_validator_class(validator_class_name)
        return validator_class()
