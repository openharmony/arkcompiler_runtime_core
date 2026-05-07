#!/usr/bin/env python3
# -- coding: utf-8 --
#
# Copyright (c) 2026 Huawei Device Co., Ltd.
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

from runner.enum_types.validation_result import ValidationResult
from runner.extensions.validators.ivalidator import IValidator
from runner.logger import Log
from runner.options.options_step import StepKind
from runner.options.options_step_utils import StepFields

if TYPE_CHECKING:
    from runner.suites.test_standard_flow import TestStandardFlow

_LOGGER = Log.get_logger(__file__)


class BypassValidator(IValidator):
    """
    Always passed validator

    Sometimes we need just pass result of a step to the next one
    """
    def __init__(self) -> None:
        super().__init__()
        for value in StepKind.values():
            self.add(value, BypassValidator.validate)

    @staticmethod
    def validate(_: "TestStandardFlow", _2: StepFields, _3: str,
                 _4: str, _5: int) -> ValidationResult:
        return ValidationResult(passed=True)
