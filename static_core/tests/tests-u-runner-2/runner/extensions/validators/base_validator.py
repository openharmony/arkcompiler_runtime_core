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
from runner.extensions.validators.ivalidator import IValidator
from runner.logger import Log
from runner.options.step import StepKind

_LOGGER = Log.get_logger(__file__)


class BaseValidator(IValidator):
    def __init__(self) -> None:
        for value in StepKind.values():
            self.add(value, BaseValidator.check_return_code_0)

    @staticmethod
    def check_return_code_0(_: object, step_name: str, output: str, error: str, return_code: int) -> bool:
        if error.strip():
            _LOGGER.all(f"Step validator {step_name}: get error output '{error}'")
        if output.find('[Fail]Device not founded or connected') > -1:
            return False
        return return_code == 0
