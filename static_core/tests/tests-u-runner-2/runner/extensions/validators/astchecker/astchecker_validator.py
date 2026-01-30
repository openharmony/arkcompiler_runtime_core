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

from dataclasses import dataclass
from io import StringIO
from json import JSONDecodeError, JSONDecoder
from typing import cast

from runner.enum_types.validation_result import ValidationResult, ValidatorFailKind
from runner.extensions.validators.astchecker.diag_type import DiagType
from runner.extensions.validators.astchecker.util_astchecker import UtilASTChecker
from runner.extensions.validators.base_validator import BaseValidator
from runner.logger import Log
from runner.options.options_step import StepKind
from runner.suites.test_standard_flow import TestStandardFlow
from runner.utils import unlines

_LOGGER = Log.get_logger(__file__)

EXPECTED_POSITIVE_RETURN_CODE = 0
EXPECTED_NEGATIVE_RETURN_CODE = 1


@dataclass(frozen=True)
class _CategorizedLines:
    diagnostics: list[str]
    others: list[str]


class AstCheckerValidator(BaseValidator):
    __util = UtilASTChecker()

    def __init__(self) -> None:
        super().__init__(add_base_validators=False)
        self.add(StepKind.COMPILER.value, AstCheckerValidator.es2panda_result_validator)
        self.add(StepKind.COMPILER.value, BaseValidator.check_stderr)

    @staticmethod
    def passed() -> ValidationResult:
        return ValidationResult(passed=True)

    @staticmethod
    def failed(description: str = "") -> ValidationResult:
        return ValidationResult(passed=False, kind=ValidatorFailKind.STEP_FAILED, description=description)

    @staticmethod
    def _categorize_lines(stderr: str) -> _CategorizedLines:
        errors = []
        other = []
        pattern = DiagType.pattern()
        for line in StringIO(stderr):
            if pattern.match(line):
                errors.append(line)
            else:
                other.append(line)
        return _CategorizedLines(errors, other)

    @staticmethod
    def _decode_dump_json(dump: str) -> dict:
        if any(char != '\b' for char in dump):
            decoder = JSONDecoder()
            obj, _ = decoder.raw_decode(dump, 0)
            return cast(dict, obj)
        return {}

    @classmethod
    def es2panda_result_validator(cls, test: "TestStandardFlow", _: str,
                                  actual_stdout: str, _2: str,
                                  return_code: int) -> ValidationResult:
        categorized = cls._categorize_lines(actual_stdout)
        test_cases = cls.__util.parse_tests(test.path.read_text(encoding='utf-8'))
        try:
            ast = cls._decode_dump_json(''.join(categorized.others))
        except JSONDecodeError:
            return cls.failed("Cannot parse es2panda output - invalid JSON")

        passed = cls.__util.run_tests(
            test_file=test.path,
            test_cases=test_cases,
            ast=ast,
            error=unlines(categorized.diagnostics))

        status_positive = passed and return_code == EXPECTED_POSITIVE_RETURN_CODE
        status_negative = (passed and (test_cases.has_error_tests and not test_cases.skip_errors) and
                           return_code == EXPECTED_NEGATIVE_RETURN_CODE)
        return cls.passed() if status_positive or status_negative else cls.failed()
