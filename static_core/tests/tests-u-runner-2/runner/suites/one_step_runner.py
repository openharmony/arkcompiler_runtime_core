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

import inspect
import subprocess
import tempfile
from collections.abc import Callable
from copy import deepcopy
from dataclasses import replace
from pathlib import Path
from subprocess import CompletedProcess

from runner.common_exceptions import InvalidConfiguration
from runner.enum_types.fail_kind import FailKind, FailureReturnCode
from runner.enum_types.validation_result import ValidationResult, ValidatorFailKind
from runner.logger import Log
from runner.options.options_step import Step, StepKind
from runner.types.step_report import StepReport
from runner.types.test_env import TestEnv

_LOGGER = Log.get_logger(__file__)

GTestResultValidator = Callable[[str, CompletedProcess], ValidationResult]
ResultValidator = Callable[[str, str, int], ValidationResult]
CommonResultValidator = Callable[..., ValidationResult]  # type: ignore[explicit-any]
ReturnCodeInterpreter = Callable[[str, str, int], int]


def is_gtest_result_validator(result_validator: CommonResultValidator) -> bool:
    try:
        sig = inspect.signature(result_validator)
        params = list(sig.parameters.values())

        if len(params) != 2:
            return False

        if (params[0].annotation not in (str, inspect.Parameter.empty) or
            params[1].annotation not in (CompletedProcess, inspect.Parameter.empty)):
            return False

    except (ValueError, TypeError):
        return False

    return True


def is_result_validator(result_validator: CommonResultValidator) -> bool:
    try:
        sig = inspect.signature(result_validator)
        params = list(sig.parameters.values())

        if len(params) != 3:
            return False

        if (params[0].annotation not in (str, inspect.Parameter.empty) or
            params[1].annotation not in (str, inspect.Parameter.empty) or
            params[2].annotation not in (int, inspect.Parameter.empty)):
            return False

    except (ValueError, TypeError):
        return False

    return True


class OneStepRunner:
    def __init__(self, step: Step, test_env: TestEnv) -> None:
        self.test_env = test_env
        self.step = step
        self.report: StepReport = StepReport(step.name, step.step_kind)
        self.coverage_config = self.test_env.config.general.coverage
        self.coverage_manager = test_env.coverage

    @staticmethod
    def _detect_fail_kind(name: str, return_code: int) -> str:
        if return_code in FailureReturnCode.SEGFAULT_RETURN_CODE.value:
            return FailKind.SEGFAULT.make_fail_kind(name)
        if return_code in FailureReturnCode.ABORT_RETURN_CODE.value:
            return FailKind.ABORT.make_fail_kind(name)
        if return_code in FailureReturnCode.IRTOC_ASSERT_RETURN_CODE.value:
            return FailKind.IRTOC_ASSERT.make_fail_kind(name)
        if return_code == 0:
            return FailKind.NEG_FAIL.make_fail_kind(name)
        return FailKind.FAIL.make_fail_kind(name)

    def run_with_coverage(self, result_validator: CommonResultValidator,
                          return_code_interpreter: ReturnCodeInterpreter = lambda _, _2, rtc: rtc) -> bool:
        coverage_per_binary = self.coverage_config.coverage_per_binary
        profraw_file, profdata_file, step = self.__get_prof_files(self.step.name, self.step)

        if self.coverage_config.use_lcov and coverage_per_binary:
            component_name = step.executable_path.stem if step.executable_path else "no_component"
            gcov_prefix, gcov_prefix_strip = self.coverage_manager.lcov_tool.get_gcov_prefix(component_name)
            env = deepcopy(step.env)
            env['GCOV_PREFIX'] = gcov_prefix
            env['GCOV_PREFIX_STRIP'] = gcov_prefix_strip
            step = replace(step, env=env)

        passed = self.run_one_step(step, result_validator, return_code_interpreter)

        if self.coverage_config.use_llvm_cov and profraw_file and profdata_file:
            self.coverage_manager.llvm_cov_tool.merge_and_delete_profraw_files(profraw_file, profdata_file)

        return passed

    def run_one_step(self, step: Step,
                     result_validator: CommonResultValidator,
                     return_code_interpreter: ReturnCodeInterpreter = lambda _, _2, rtc: rtc) -> bool:
        name = step.name
        passed = False

        try:
            if step.step_kind == StepKind.GTEST_RUNNER:
                passed = self.__run_gtest(step, result_validator)
            else:
                passed = self.__run(step, result_validator, return_code_interpreter)
        except subprocess.SubprocessError as ex:
            self.report.status = FailKind.SUBPROCESS.make_fail_kind(name)
            failed_msg = f"{name}: Failed with {str(ex).strip()}"
            error = self.report.cmd_error
            self.report.cmd_error = f"{error}\n{failed_msg}" if error else failed_msg
            self.report.cmd_return_code = -1 if self.report.cmd_return_code == 0 else self.report.cmd_return_code

        if not self.report.status:
            self.report.status = FailKind.PASSED.make_fail_kind(name) if passed else FailKind.OTHER.make_fail_kind(name)

        return passed

    def _terminate_timed_out_process(self, process: subprocess.Popen) -> tuple[str, str]:
        timeout = self.test_env.config.general.retrieve_log_timeout
        error = ""
        try:
            process.terminate()
            output, error = process.communicate(timeout=timeout)
            output = str(output).strip()
            error = str(error).strip()
            output = f"Actual partial output: {output}"
        except subprocess.TimeoutExpired:
            process.kill()
            output = ""
            error += f"Failed by second timeout on retrieving partial output after {timeout} sec"
        return output, error

    def __run(self, step: Step, result_validator: ResultValidator,
              return_code_interpreter: ReturnCodeInterpreter) -> bool:
        if not is_result_validator(result_validator):
            raise InvalidConfiguration(f"{step.name}: Invalid result validator for the step. "
                                       "Expected ResultValidator")

        if step.executable_path is None:
            raise InvalidConfiguration(f"Cannot run step without executable path: {step.name}")

        cmd = [step.executable_path.as_posix()]
        if self.test_env.cmd_prefix and not step.skip_qemu:
            cmd = [*self.test_env.cmd_prefix, *cmd]
        cmd.extend(step.args)
        passed = False
        name = step.name
        validation_result: ValidationResult | None = None

        self.report.command_line = ' '.join(cmd)
        _LOGGER.all(f"Run {name}: {self.report.command_line}")

        with (subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                env=step.env,
                encoding='utf-8',
                errors='ignore',
        ) as process):
            try:
                output, error = process.communicate(timeout=step.timeout)
                output = str(output).strip()
                error = str(error).strip()
                if step.stdout:
                    step.stdout.write_text(output, encoding="utf-8")
                if step.stderr:
                    step.stderr.write_text(error, encoding="utf-8")
                return_code = return_code_interpreter(output, error, process.returncode)
                validation_result = result_validator(output, error, return_code)
                passed = validation_result.passed
                if not passed:
                    if validation_result.kind in [ValidatorFailKind.COMPARE_OUTPUT,
                                                  ValidatorFailKind.STDERR_NOT_EMPTY,
                                                  ValidatorFailKind.POST_REQ_FAILED]:
                        step_status = validation_result.kind.make_fail_kind(name)
                    else:
                        step_status = self._detect_fail_kind(name, return_code)
                else:
                    step_status = FailKind.PASSED.make_fail_kind(name)
            except subprocess.TimeoutExpired:
                step_status = FailKind.TIMEOUT.make_fail_kind(name)
                return_code = process.returncode
                output, error = self._terminate_timed_out_process(process)
                error = f"Failed by timeout after {step.timeout} sec\n{error}"

        self.report.cmd_output = output
        self.report.cmd_error = error
        self.report.cmd_return_code = return_code
        self.report.validator_messages = validation_result.description if validation_result is not None else ""
        self.report.status = step_status
        return passed

    def __run_gtest(self, step: Step,
                    result_validator: GTestResultValidator) -> bool:
        if not is_gtest_result_validator(result_validator):
            raise InvalidConfiguration(f"{step.name}: Invalid result validator for the step. "
                                       "Expected GTestResultValidator")

        if step.executable_path is None:
            raise InvalidConfiguration(f"Cannot run step without executable path: {step.name}")

        cmd = [step.executable_path.as_posix()]
        if self.test_env.cmd_prefix and not step.skip_qemu:
            cmd = [*self.test_env.cmd_prefix, *cmd]
        cmd.extend(step.args)
        passed = False
        name = step.name
        validator_result: ValidationResult | None = None

        self.report.command_line = ' '.join(cmd)
        _LOGGER.all(f"Run {name}: {self.report.command_line}")

        with tempfile.NamedTemporaryFile(mode='w', suffix='.json') as f:
            json_file = f.name

            try:
                result = subprocess.run(
                    [*cmd, f"--gtest_output=json:{json_file}"],
                    capture_output=True,
                    text=True,
                    timeout=step.timeout,
                    env=step.env,
                    check=False
                )
                output = result.stdout.strip()
                error = result.stderr.strip()
                return_code = result.returncode
                validator_result = result_validator(json_file, result)
                passed = validator_result.passed
                step_status: str = (FailKind.PASSED.make_fail_kind(name)
                                    if validator_result.kind == ValidatorFailKind.NONE
                                    else validator_result.kind.value)
            except subprocess.TimeoutExpired as pt:
                step_status = FailKind.TIMEOUT.make_fail_kind(name)
                error = pt.stderr.decode().strip() if pt.stderr else ""
                error = f"Failed by timeout after {step.timeout} sec\n{error}".strip()
                output = pt.stdout.decode().strip() if pt.stdout else ""
                return_code = -1

        self.report.cmd_output = output
        self.report.cmd_error = error
        self.report.cmd_return_code = return_code
        self.report.validator_messages = validator_result.description if validator_result is not None else ""
        self.report.status = step_status
        return passed

    def __get_prof_files(
            self,
            name: str,
            step: Step
    ) -> tuple[Path | None, Path | None, Step]:
        profraw_file, profdata_file = None, None
        llvm_cov_tool = self.coverage_manager.llvm_cov_tool

        if not self.coverage_config.use_llvm_cov:
            return profraw_file, profdata_file, step

        if self.coverage_config.coverage_per_binary:
            profraw_file, profdata_file = llvm_cov_tool.get_uniq_profraw_profdata_file_paths(name)
        else:
            profraw_file, profdata_file = llvm_cov_tool.get_uniq_profraw_profdata_file_paths()

        env = deepcopy(step.env)
        env['LLVM_PROFILE_FILE'] = str(profraw_file)

        return profraw_file, profdata_file, replace(step, env=env)
