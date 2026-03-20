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
from collections.abc import Callable
from pathlib import Path, PurePath, PureWindowsPath
from typing import cast

from runner.common_exceptions import InvalidConfiguration
from runner.enum_types.base_enum import BaseEnum

ExpandedValueType = str | int | bool | list | dict


class MacroModifier(BaseEnum):
    NAME = "name"
    STEM = "stem"
    PARENT = "parent"

    @staticmethod
    def take_name(value: str) -> str:
        return MacroModifier._get_path(value).name

    @staticmethod
    def take_stem(value: str) -> str:
        return MacroModifier._get_path(value).stem

    @staticmethod
    def take_parent(value: str) -> str:
        return MacroModifier._get_path(value).parent.as_posix()

    @staticmethod
    def is_path(value: ExpandedValueType) -> bool:
        return isinstance(value, str) and '\n' not in value

    @staticmethod
    def _process_path(check: Callable[[ExpandedValueType], bool], fn: Callable[[str], ExpandedValueType],
                      value: ExpandedValueType) -> ExpandedValueType:
        if not check(value):
            return value
        value = cast(str, value)
        return fn(value)

    @staticmethod
    def _get_path(value: str | PurePath) -> PurePath:
        if isinstance(value, PurePath):
            return value
        return PureWindowsPath(value) if '\\' in value else Path(value)

    def modify(self, value: ExpandedValueType) -> ExpandedValueType:
        if self == MacroModifier.NAME:
            return self._process_path(self.is_path, self.take_name, value)
        if self == MacroModifier.STEM:
            return self._process_path(self.is_path, self.take_stem, value)
        if self == MacroModifier.PARENT:
            return self._process_path(self.is_path, self.take_parent, value)
        raise InvalidConfiguration(f"Cannot process unknown modifier '{self}'")
