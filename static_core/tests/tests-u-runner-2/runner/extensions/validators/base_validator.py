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
from typing import TYPE_CHECKING

from runner.extensions.validators.ivalidator import IValidator
from runner.logger import Log
from runner.options.step import StepKind

if TYPE_CHECKING:
    from runner.suites.test_standard_flow import TestStandardFlow

_LOGGER = Log.get_logger(__file__)


class BaseValidator(IValidator):
    def __init__(self) -> None:
        for value in StepKind.values():
            self.add(value, BaseValidator.check_return_code_0)

    @staticmethod
    def check_return_code_0(test: "TestStandardFlow", step_value: str, output: str,
                            error_output: str, return_code: int) -> bool:
        if error_output.strip():
            _LOGGER.all(f"Step validator {step_value}: get error output '{error_output}'")
        if output.find('[Fail]Device not founded or connected') > -1:
            return False

        if BaseValidator._step_passed(test, return_code, step_value):
            # the step has passed

            if (test.has_expected or test.has_expected_err) and (output or error_output):
                comparison_res = test.compare_output_with_expected(output, error_output)

                if comparison_res:
                    return True

                test.reproduce += "Comparison with .expected or .expected.err file failed\n"
                return False

            if not error_output:
                return True

            test.reproduce += ("\nThe test has passed, but stderr is not empty. "
                               "There is no .expected.err file to compare with\n")
            return False

        return False

    @staticmethod
    def _step_passed(test: "TestStandardFlow", return_code: int, step: str) -> bool:
        if step == StepKind.COMPILER.value:
            if test.is_negative_compile:
                return return_code == 1
            return return_code == 0

        if step == StepKind.RUNTIME.value:
            if test.is_negative_runtime:
                return return_code in [1, 255]
            return return_code == 0

        return return_code == 0
