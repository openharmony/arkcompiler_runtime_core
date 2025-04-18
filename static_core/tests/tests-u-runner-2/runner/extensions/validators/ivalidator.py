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

from abc import ABC
from collections.abc import Callable

# Signature:
# test: TestStandardFlow - what test is running
# step_name: str - what step is running
# output: str - what stdout contains
# error: str - what stderr contains
# return_code: int - return code of the binary run on the step for the test
ValidatorFunction = Callable[[object, str, str, str, int], bool]


class IValidator(ABC):  # noqa: B024
    __validators: dict[str, ValidatorFunction] = {}  # noqa: RUF012

    @property
    def validators(self) -> dict[str, ValidatorFunction]:
        return self.__validators

    def add(self, step_name: str, function: ValidatorFunction) -> None:
        self.__validators[step_name] = function

    def has_step(self, step_name: str) -> bool:
        return step_name in self.__validators

    def get_validator(self, step_name: str) -> ValidatorFunction | None:
        return self.__validators.get(step_name)
