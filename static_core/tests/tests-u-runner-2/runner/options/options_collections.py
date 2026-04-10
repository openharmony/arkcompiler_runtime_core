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

from functools import cached_property
from pathlib import Path
from typing import Any, cast

from runner.common_exceptions import MacroNotExpanded, MacroSyntaxError
from runner.logger import Log
from runner.options.macros import Macros
from runner.options.options import IOptions
from runner.options.root_dir import RootDir
from runner.utils import convert_minus

_LOGGER = Log.get_logger(__file__)


class CollectionsOptions(IOptions):
    __TEST_ROOT = "test-root"
    __LIST_ROOT = "list-root"
    __PARAMETERS = "parameters"
    __GENERATOR_CLASS = "generator-class"
    __GENERATOR_SCRIPT = "generator-script"
    __GENERATOR_OPTIONS = "generator-options"
    __EXCLUDE = "exclude"

    def __init__(self, name: str, args: dict[str, Any], parent: IOptions):  # type: ignore[explicit-any]
        super().__init__(None)
        self.__name = name
        self._parent: IOptions = parent
        self.__args: dict[str, Any] = args  # type: ignore[explicit-any]
        self.__test_root: RootDir = RootDir.get_test_root(obj=self.__args, options=self) \
            if not name \
            else RootDir(cast(Path, parent.get_value('test_root')), name, self)
        parent_list_roots: list[RootDir] = cast(list[RootDir], parent.get_value('list_roots'))
        self.__list_roots: list[RootDir] = RootDir.get_list_roots(
            obj=self.__args,
            default_name=cast(str, parent.get_value('suite_name')),
            default_list_roots=parent_list_roots,
            options=self)
        self.__exclude: list[str] = args[self.__EXCLUDE] if args and self.__EXCLUDE in args else []
        self.__parameters: dict[str, Any] = (  # type: ignore[explicit-any]
            args)[self.__PARAMETERS] if args and self.__PARAMETERS in args else {}
        self.__expand_macros_in_parameters()

    def __str__(self) -> str:
        return f"{self.__name}: {self._to_str(indent=3)}"

    @cached_property
    def name(self) -> str:
        return self.__name

    @cached_property
    def test_root(self) -> Path:
        return self.__test_root()

    @cached_property
    def list_roots(self) -> list[RootDir]:
        return self.__list_roots

    @cached_property
    def generator_class(self) -> str | None:
        return self.__get_from_args(self.__GENERATOR_CLASS)

    @cached_property
    def generator_script(self) -> str | None:
        return self.__get_from_args(self.__GENERATOR_SCRIPT)

    @cached_property
    def generator_options(self) -> list[str]:
        default_options: list[str] = []
        return cast(list[str], self.__get_from_args(self.__GENERATOR_CLASS, default_options))

    @cached_property
    def exclude(self) -> list[str]:
        return self.__exclude

    @cached_property
    def parameters(self) -> dict[str, Any]:  # type: ignore[explicit-any]
        return self.__parameters

    def get_parameter(self, key: str, default: Any | None = None) -> Any | None:  # type: ignore[explicit-any]
        return self.__parameters.get(key, default)

    def __get_from_args(self, key: str, default_value: Any | None = None) -> Any | None:  # type: ignore[explicit-any]
        if self.__args and key in self.__args:
            return self.__args[key]
        parent_value = getattr(self._parent, convert_minus(key), None)
        if parent_value is not None:
            return parent_value
        return default_value

    def __expand_macros_in_parameters(self) -> None:
        for param_name, param_value in self.__parameters.items():
            if isinstance(param_value, str):
                try:
                    self.__parameters[param_name] = Macros.correct_macro(param_value, self)
                except (MacroSyntaxError, MacroNotExpanded):
                    _LOGGER.all(f"Macro not expanded or has syntax error: {param_value}")
