#!/usr/bin/env python3
# -- coding: utf-8 --
#
# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
import copy
import os
import re
from collections.abc import Sequence
from copy import deepcopy
from pathlib import Path

from runner import utils
from runner.common_exceptions import InvalidConfiguration
from runner.enum_types.params import BinaryParams, TestEnv, TestReport
from runner.extensions.validators.base_validator import BaseValidator
from runner.extensions.validators.ivalidator import IValidator
from runner.logger import Log
from runner.options.macros import Macros, ParameterNotFound
from runner.options.options import IOptions
from runner.options.step import Step, StepKind
from runner.suites.one_test_runner import OneTestRunner
from runner.suites.test_metadata import TestMetadata
from runner.test_base import Test
from runner.utils import get_class_by_name

_LOGGER = Log.get_logger(__file__)


class TestStandardFlow(Test):
    __DEFAULT_ENTRY_POINT = "ETSGLOBAL::main"

    def __init__(self, test_env: TestEnv, test_path: Path, *,
                 params: IOptions, test_id: str, is_dependent: bool = False,
                 parent_test_id: str = "") -> None:
        Test.__init__(self, test_env, test_path, params, test_id)

        self.main_entry_point: str = self.__DEFAULT_ENTRY_POINT
        self.metadata: TestMetadata = TestMetadata.get_metadata(test_path)
        if self.metadata.package is not None:
            self.main_entry_point = f"{self.metadata.package}.{self.main_entry_point}"
        else:
            self.main_entry_point = f"{test_path.stem}.{self.main_entry_point}"

        # Defines if in dependent packages there is at least one file compile-only and negative
        self.dependent_packages: dict[str, bool] = {
            'package': self.is_negative_compile
        }

        self.parent_test_id = parent_test_id

        # If test fails it contains reason of first failed step
        # It's supposed if the first step is failed then no step is executed further
        self.fail_kind: str | None = None

        self.bytecode_path: Path = test_env.work_dir.intermediate
        self.test_abc: Path = self.bytecode_path.joinpath(f"{self.test_id}.abc")
        self.test_an: Path = self.bytecode_path.joinpath(f"{self.test_id}.an")
        self.test_abc.parent.mkdir(parents=True, exist_ok=True)

        self.validator: IValidator = self.__init_validator()
        self.__dependent_tests: list[TestStandardFlow] = []
        self.__is_dependent = is_dependent
        self.__boot_panda_files: str = ""

    @property
    def all_dependent_tests(self) -> list['TestStandardFlow']:
        if not self.metadata.files:
            return []
        id_pos: int = str(self.path).find(self.test_id)
        test_root: str = str(self.path)[:id_pos]

        dependent_tests: list[TestStandardFlow] = []

        for file in self.metadata.files:
            new_test_file_path: Path = self.path.parent.joinpath(file).resolve()
            new_test_id: Path = new_test_file_path.relative_to(test_root)
            test = self.__class__(self.test_env, new_test_file_path,
                                  params=self.test_extra_params,
                                  test_id=str(new_test_id),
                                  is_dependent=True,
                                  parent_test_id=self.test_id)
            dependent_tests.append(test)
            if (sub_tests := test.all_dependent_tests) is not None:
                dependent_tests.extend(sub_tests)

        return dependent_tests

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

        if self.__dependent_tests:
            return self.__dependent_tests

        self.__dependent_tests.extend(self.all_dependent_tests)
        current_test_id: Path = Path(self.test_id)
        for test in self.__dependent_tests:
            prefix = Path(self.parent_test_id).stem if self.parent_test_id else current_test_id.stem

            test_abc_name = f'{prefix}_{Path(test.test_abc).name}'
            test_an_name = f'{prefix}_{Path(test.test_an).name}'

            test.test_abc = Path(test.test_abc).parent / Path(test_abc_name)
            test.test_an = Path(test.test_abc).parent / Path(test_an_name)

            os.makedirs(str(test.test_abc.parent), exist_ok=True)

            package = test.metadata.get_package_name()
            self.dependent_packages[package] = self.dependent_packages.get(package, False) or test.is_negative_compile
            if test.dependent_packages:
                for dep_key, dep_item in test.dependent_packages.items():
                    self.dependent_packages[dep_key] = self.dependent_packages.get(dep_key, False) or dep_item

        return self.__dependent_tests

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
    def __add_options(options: list[str]) -> list[str]:
        for index, option in enumerate(options):
            if not option.startswith("--"):
                options[index] = f"--{option}"
        return options

    @staticmethod
    def __get_validator_class(clazz: str) -> type[IValidator]:
        class_obj = get_class_by_name(clazz)
        if not issubclass(class_obj, IValidator):
            raise InvalidConfiguration(
                f"Validator class '{clazz}' not found. "
                f"Check value of 'validator' parameter")
        return class_obj

    def continue_after_process_dependent_files(self) -> bool:
        """
        Processes dependent files
        Returns True if to continue test run
        False - break test run
        """
        for test in self.dependent_tests:
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

    def do_run(self) -> 'TestStandardFlow':
        if not self.continue_after_process_dependent_files():
            return self

        for step in self.test_env.config.workflow.steps:
            if step.executable_path is not None:
                self.passed, self.report, self.fail_kind = self.__do_run_one_step(step)
                is_break = ((not self.passed) or
                            (step.step_kind == StepKind.COMPILER and
                             (self.is_compile_only or self.metadata.tags.not_a_test)))
                if is_break:
                    return self
        return self

    def dependent_abc_files(self) -> list[str]:
        return [df.test_abc.as_posix() for df in self.dependent_tests]

    def __do_run_one_step(self, step: Step) -> tuple[bool, TestReport | None, str | None]:
        if not step.enabled:
            passed, report, fail_kind = True, None, None
        elif step.step_kind == StepKind.COMPILER:
            passed, report, fail_kind = self.__run_compiler(step)
            if self.is_negative_compile:
                passed = not passed
        elif step.step_kind == StepKind.VERIFIER:
            passed, report, fail_kind = self.__run_verifier(step)
        elif step.step_kind == StepKind.AOT:
            passed, report, fail_kind = self.__run_aot(step)
        elif step.step_kind == StepKind.RUNTIME:
            passed, report, fail_kind = self.__run_runtime(step)
            if self.is_negative_runtime:
                passed = not passed
        else:
            passed, report, fail_kind = self.__run_step(step)
        return passed, report, fail_kind

    def __fix_entry_point(self, args: list[str]) -> list[str]:
        result: list[str] = args[:]
        for index, arg in enumerate(result):
            if self.__DEFAULT_ENTRY_POINT in str(arg):
                result[index] = arg.replace(self.__DEFAULT_ENTRY_POINT, self.main_entry_point).strip()
        return [res for res in result if res]

    def __change_output_arg(self, source_args: list[str], new_args: list[str]) -> list[str]:
        for index, arg in enumerate(source_args):
            if arg.startswith("--output="):
                new_args[index] = f"--output={self.test_abc}"
                break
        return new_args

    def __change_arktsconfig_arg(self, source_args: list[str], new_args: list[str]) -> list[str]:
        for index, arg in enumerate(source_args):
            if arg.startswith("--arktsconfig=") and self.metadata.arktsconfig is not None:
                stdlib_path = self.test_env.config.general.static_core_root / 'plugins' / 'ets' / 'stdlib'
                new_args[index] = f"--arktsconfig={self.metadata.arktsconfig}"
                new_args.insert(0, f"--stdlib={stdlib_path.as_posix()}")
                break
        return new_args

    def __run_compiler(self, step: Step) -> tuple[bool, TestReport, str | None]:
        new_step = copy.copy(step)
        new_step.args = step.args[:]
        if self.__is_dependent:
            new_step.args = self.__change_output_arg(step.args, new_step.args)
            new_step.args = self.__change_arktsconfig_arg(step.args, new_step.args)
            return self.__run_step(new_step)
        new_step.args = self.__change_arktsconfig_arg(step.args, new_step.args)
        return self.__run_step(new_step)

    def __run_verifier(self, step: Step) -> tuple[bool, TestReport, str | None]:
        if self.dependent_tests:
            new_step = copy.copy(step)
            new_step.args = step.args[:]
            new_step.args = self.__add_boot_panda_files(new_step.args)
            return self.__run_step(new_step)
        return self.__run_step(step)

    def __run_aot(self, step: Step) -> tuple[bool, TestReport, str | None]:
        new_step = copy.copy(step)
        new_step.args = step.args[:]
        if self.dependent_tests:
            for abc_file in list(self.dependent_abc_files()):
                new_step.args.extend([f'--paoc-panda-files={abc_file}'])
        new_step.args.extend([f'--paoc-panda-files={self.test_abc}'])
        return self.__run_step(new_step)

    def __run_runtime(self, step: Step) -> tuple[bool, TestReport, str | None]:
        new_step = copy.copy(step)
        new_step.args = step.args[:]
        if self.dependent_tests:
            new_step.args = self.__add_boot_panda_files(new_step.args)
            return self.__run_step(new_step)

        new_step.args.insert(-2, self.__add_panda_files())
        return self.__run_step(new_step)

    def __run_step(self, step: Step) -> tuple[bool, TestReport, str | None]:
        cmd_env = self.test_env.cmd_env
        if step.env:
            cmd_env = deepcopy(self.test_env.cmd_env)
            for env_item in step.env:
                env_value: str | list[str] = step.env[env_item]
                if isinstance(env_value, list):
                    cmd_env[env_item] = "".join(env_value)
                else:
                    cmd_env[env_item] = env_value

        assert step.executable_path is not None
        params = BinaryParams(
            executor=step.executable_path,
            flags=self.__expand_last_call_macros(step),
            env=cmd_env,
            timeout=step.timeout,
        )

        test_runner = OneTestRunner(self.test_env)
        passed, report, fail_kind = test_runner.run_with_coverage(
            name=step.name,
            params=params,
            result_validator=lambda out, err, return_code: self._step_validator(step, out, err, return_code),
            return_code_interpreter=lambda out, err, return_code: self._get_return_code_from_device(out, return_code)
        )
        self.reproduce += test_runner.reproduce
        return passed, report, fail_kind

    def __expand_last_call_macros(self, step: Step) -> list[str]:
        flags: list[str] = []
        for arg in self.__fix_entry_point(step.args):
            flag = utils.replace_macro(str(arg), "test-id", self.test_id)
            if utils.has_macro(flag):
                flag_expanded: str | list[str] = ""
                try:
                    flag_expanded = Macros.correct_macro(flag, self.test_extra_params)
                except ParameterNotFound:
                    flag_expanded = Macros.correct_macro(flag, self.test_env.config.workflow)
                if isinstance(flag_expanded, list):
                    flags.extend(flag_expanded)
                elif isinstance(flag_expanded, str):
                    flags.extend(flag_expanded.split())
            else:
                flags.extend(flag.split())
        if step.step_kind == StepKind.COMPILER and self.metadata.es2panda_options:
            if 'dynamic-ast' in self.metadata.es2panda_options:
                index = flags.index("--dump-ast")
                flags[index] = "--dump-dynamic-ast"
            if 'module' in self.metadata.es2panda_options:
                flags.insert(0, "--module")
        if step.step_kind == StepKind.RUNTIME and self.metadata.ark_options:
            prepend_options = self.__add_options(self.metadata.ark_options)
            flags = utils.prepend_list(prepend_options, flags)
        return flags

    def __init_validator(self) -> IValidator:
        validator_class_name = self.test_env.config.test_suite.get_parameter("validator")
        if validator_class_name is None:
            return BaseValidator()
        validator_class = self.__get_validator_class(validator_class_name)
        return validator_class()

    def __add_boot_panda_files(self, args: list[str]) -> list[str]:
        dep_files_args: list[str] = []
        for arg in args:
            name = '--boot-panda-files'
            if name in arg:
                if not self.__boot_panda_files:
                    _, value = arg.split('=')
                    boot_panda_files = [value, *self.dependent_abc_files(), self.test_abc.as_posix()]
                    self.__boot_panda_files = f'{name}={":".join(boot_panda_files)}'
                dep_files_args.append(self.__boot_panda_files)
            else:
                dep_files_args.append(arg)
        return dep_files_args

    def __add_panda_files(self) -> str:
        opt_name = '--panda-files'
        if self.dependent_abc_files():
            return f'{opt_name}={":".join(self.dependent_abc_files())}'

        return f'{opt_name}={self.test_abc!s}'

    def _step_validator(self, step: Step, output: str, error: str, return_code: int) -> bool:
        validator = (self.validator.get_validator(step.name)
                     if step.name in self.validator.validators
                     else self.validator.get_validator(step.step_kind.value))
        if validator is not None:
            return validator(self, step.name, output, error, return_code)
        return True
