#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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

import subprocess
from copy import deepcopy
from typing import Callable, Tuple, Optional

from runner.enum_types.params import TestEnv, BinaryParams, TestReport
from runner.logger import Log

_LOGGER = Log.get_logger(__file__)

ResultValidator = Callable[[str, str, int], bool]
ReturnCodeInterpreter = Callable[[str, str, int], int]


class OneTestRunner:
    def __init__(self, test_env: TestEnv) -> None:
        self.test_env = test_env
        self.reproduce = ""

    @staticmethod
    def __fail_kind_fail(name: str) -> str:
        return f"{name.upper()}_FAIL"

    @staticmethod
    def __fail_kind_other(name: str) -> str:
        return f"{name.upper()}_OTHER"

    @staticmethod
    def __fail_kind_timeout(name: str) -> str:
        return f"{name.upper()}_TIMEOUT"

    def run_with_coverage(self, name: str, params: BinaryParams, result_validator: ResultValidator,
                          return_code_interpreter: ReturnCodeInterpreter = lambda _, _2, rtc: rtc) \
            -> Tuple[bool, TestReport, Optional[str]]:
        profraw_file, profdata_file = "", ""
        if self.test_env.config.general.coverage.use_llvm_cov and self.test_env.coverage is not None:
            params = deepcopy(params)
            profraw_file, profdata_file = self.test_env.coverage.get_uniq_profraw_profdata_file_paths()
            params.env['LLVM_PROFILE_FILE'] = profraw_file

        passed, report, fail_kind = self.run_one_step(name, params, result_validator, return_code_interpreter)

        if self.test_env.config.general.coverage.use_llvm_cov and self.test_env.coverage is not None:
            self.test_env.coverage.merge_and_delete_prowraw_files(profraw_file, profdata_file)

        return passed, report, fail_kind

    def run_one_step(self, name: str, params: BinaryParams, result_validator: ResultValidator,
                     return_code_interpreter: ReturnCodeInterpreter = lambda _, _2, rtc: rtc) \
            -> Tuple[bool, TestReport, Optional[str]]:

        cmd = self.test_env.cmd_prefix + [str(params.executor)]
        cmd.extend(params.flags)

        passed = False
        output = ""
        error = ""

        try:
            passed, fail_kind, output, error, return_code = self.__run(
                name, params, result_validator, return_code_interpreter)
        except subprocess.SubprocessError as ex:
            fail_kind = self.__fail_kind_other(name)
            fail_kind_msg = f"{name}: Failed with {str(ex).strip()}"
            error = f"{error}\n{fail_kind_msg}" if error else fail_kind_msg
            return_code = -1
        self.__log_cmd(f"{name}: Actual error: {error.strip()}")
        self.__log_cmd(f"{name}: Actual return code: {return_code}\n")

        report = TestReport(
            output=output,
            error=error,
            return_code=return_code
        )

        return passed, report, fail_kind.upper() if fail_kind else fail_kind

    def __log_cmd(self, cmd: str) -> None:
        self.reproduce += f"\n{cmd}"

    def __run(self, name: str, params: BinaryParams, result_validator: ResultValidator,
              return_code_interpreter: ReturnCodeInterpreter) -> Tuple[bool, str, str, str, int]:
        cmd = self.test_env.cmd_prefix + [str(params.executor)]
        cmd.extend(params.flags)
        passed = False
        output = ""

        self.__log_cmd(f"{name}: {' '.join(cmd)}")
        _LOGGER.all(f"Run {name}: {' '.join(cmd)}")

        with subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                env=params.env,
                encoding='utf-8',
                errors='ignore',
        ) as process:
            fail_kind: Optional[str] = None
            try:
                output, error = process.communicate(timeout=params.timeout)
                return_code = return_code_interpreter(output, error, process.returncode)
                passed = result_validator(output, error, return_code)
                self.__log_cmd(f"{name}: Actual output: {output.strip()}")
                if not passed:
                    fail_kind = self.__fail_kind_fail(name)
            except subprocess.TimeoutExpired:
                self.__log_cmd(f"{name}: Failed by timeout after {params.timeout} sec")
                fail_kind = self.__fail_kind_timeout(name)
                error = fail_kind
                return_code = process.returncode
                process.kill()
        return passed, fail_kind, output, error, return_code
