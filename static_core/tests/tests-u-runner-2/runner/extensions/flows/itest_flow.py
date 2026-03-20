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

from abc import ABC, abstractmethod
from pathlib import Path

from typing_extensions import Self

from runner.types.step_report import StepReport


class ITestFlow(ABC):
    test_id: str
    passed: bool | None
    path: Path
    ignored: bool
    last_failure: str
    step_reports: list[StepReport]
    fail_kind: str | None
    last_failure_check_passed: bool

    @property
    @abstractmethod
    def is_valid_test(self) -> bool:
        ...

    @property
    @abstractmethod
    def is_negative_compile(self) -> bool:
        ...

    @abstractmethod
    def do_run(self) -> Self:
        ...
