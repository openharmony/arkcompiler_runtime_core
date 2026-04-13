#!/usr/bin/env python3
# -- coding: utf-8 --
#
# Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
from collections.abc import Callable
from typing import TYPE_CHECKING

from runner import utils
from runner.enum_types.validation_result import ValidationResult, ValidatorFailKind
from runner.extensions.validators.base_validator import BaseValidator
from runner.logger import Log
from runner.options.options_step import StepKind
from runner.options.options_step_utils import StepFields

if TYPE_CHECKING:
    from runner.suites.test_standard_flow import TestStandardFlow

_LOGGER = Log.get_logger(__file__)

ValidatorFunction = Callable[["TestStandardFlow", str, str, str, int], ValidationResult]


class ParserValidator(BaseValidator):

    def __init__(self) -> None:

        super().__init__(add_base_validators=False)

        for value in StepKind.values():
            self.add(value, ParserValidator.es2panda_result_validator)

    @staticmethod
    def es2panda_result_validator(test: "TestStandardFlow", _: StepFields, actual_output: str, _2: str,
                                  return_code: int) -> ValidationResult:
        fail_kind = ValidatorFailKind.NONE
        actual_output = actual_output.strip()
        validator_message = ""
        expected_name = f"{test.path.stem}-expected"
        expected_path = test.path.with_stem(expected_name).with_suffix(".txt")
        if not expected_path.exists():
            return ValidationResult(False,
                                    ValidatorFailKind.OTHER,
                                    f"Expected file {expected_path.as_posix()} not found")
        try:
            with open(expected_path, encoding="utf-8") as file_pointer:
                expected = file_pointer.read().strip()
            normalized_output = utils.normalize_build_system_path(actual_output)
            passed_outputs = expected == normalized_output
            passed_returns_codes = return_code in [0, 1]
            if not passed_outputs:
                fail_kind = ValidatorFailKind.COMPARE_OUTPUT
                validator_message = f"Actual output '{normalized_output}' does not match to expected one '{expected}'"
            elif not passed_returns_codes:
                fail_kind = ValidatorFailKind.COMPARE_OUTPUT
                validator_message = f"Actual return code {return_code} does not match to expected values [0, 1]"
            passed = passed_outputs and passed_returns_codes
        except OSError:
            passed = False
            fail_kind = ValidatorFailKind.OTHER
            validator_message = f"Cannot read {expected_path.as_posix()}"

        return ValidationResult(passed, fail_kind, validator_message)
