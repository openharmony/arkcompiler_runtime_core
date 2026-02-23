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
from dataclasses import dataclass
from functools import cached_property
from pathlib import Path
from typing import Any, cast

from runner.common_exceptions import InvalidConfiguration
from runner.options.macros import Macros
from runner.options.options import IOptions

OPT_LIST_ROOT = "list-root"
OPT_LIST_ROOTS = "list-roots"
OPT_TEST_ROOT = "test-root"


@dataclass
class RootDir:
    """
    Class represents the idea of "the named path" - a path and a name describing the path somehow.
    It's for typing data read following keys from yaml file: `list-root`, `list-roots`, `test-root`
    """
    _path: Path
    _name: str | None

    def __init__(self, root_dir: str | Path, name: str | None, options: IOptions):
        if isinstance(root_dir, str):
            root_expanded = Macros.expand_macros_in_path(root_dir, options)
            if root_expanded is None:
                raise InvalidConfiguration(f"Cannot expand macros in {root_dir}")
            self._path = Path(root_expanded)
        else:
            self._path = root_dir
        self._name = name

    def __str__(self) -> str:
        return self.root_dir.as_posix() if self.name is None else f"{self.name}: {self.root_dir.as_posix()}"

    def __call__(self) -> Path:
        """
        The notation
        a: RootDir = ...
        a() - shortens way to get main data item `path`
        You still can write `a.root_dir` to get the same data.
        """
        return self._path

    @staticmethod
    def from_obj(options: IOptions, **kwargs: Any) -> 'RootDir':  # type: ignore[explicit-any]
        """
        Creates object `RootDir` from raw dictionary read from yaml
        Params:
        : options : usually the object of config, required for expanding macros
        : kwargs : in the arbitrary order pairs `key=value`. Required keys: root, name
        """
        root = RootDir._get_value('root', kwargs)
        name = RootDir._get_value('name', kwargs)
        return RootDir(root_dir=root, name=name, options=options)

    @staticmethod
    def get_test_root(obj: dict, options: IOptions) -> 'RootDir':
        """
        Creates object `RootDir` for representing single test-root.
        Params:
        : obj : dictionary with raw data, read from yaml
        : options : usually the object of config, required for expanding macros
        """
        root: str | Path | None = cast(str, obj.get(OPT_TEST_ROOT, None))
        if root is None:
            raise InvalidConfiguration(f"The key 'test-root' should be set. Got {obj}")
        return RootDir(root_dir=root, name=None, options=options)

    @staticmethod
    def get_list_roots(obj: dict, options: IOptions, default_name: str,
                       default_list_roots: list['RootDir']) -> list['RootDir']:
        """
        Creates list of objects `RootDir` for representing multiple list-roots.
        Params:
        : obj : dictionary with raw data, read from yaml
        : options : usually the object of config, required for expanding macros
        : default_name : the name of root used if it's absent in the yaml
        : default_list_roots : list of values used if no `list-root` or `list-roots`
        items are found in the dictionary. Usually, this list contains only one item.

        In the yaml list roots can be specified by 2 ways:
        1) with key `list-root` - it's supposed that only one value is provided in the form:
            list-root: <path>
        2) with key `list-roots` - it's supposed that several values can be provided in any of 2 forms:
            list-roots:
                - <path>
                - root: <path>
                  name: <name>
            The following form with explicit field `root` and without `name` is prohibited:
            list-roots:
                - root: <path>
        """
        if obj and (data := obj.get(OPT_LIST_ROOTS, None)):
            if not isinstance(data, list):
                raise InvalidConfiguration("Incorrect configuration of LIST_ROOTS - the list of roots is expected."
                                           f"Got: {obj}")
            roots = [RootDir.from_obj(options, **item)
                     if isinstance(item, dict)
                     else RootDir(root_dir=item, name=default_name, options=options)
                     for item in data]
            return roots

        if obj and (data := obj.get(OPT_LIST_ROOT, None)):
            if isinstance(data, str):
                root = RootDir(root_dir=data, name=default_name, options=options)
            elif isinstance(data, dict):
                root = RootDir.from_obj(options, **data)
            else:
                raise InvalidConfiguration("Incorrect configuration of LIST_ROOT - the one root is expected. No list."
                                           f"Got: {obj}")
            return [root]

        return default_list_roots

    @staticmethod
    def _get_value(field_name: str, kwargs: dict) -> str:
        """
        Returns value from provided dict.
        Params:
        : field_name : the searched key. If it's not found InvalidConfiguration raises.
        : kwargs : dictionary with raw data
        """
        if field_name in kwargs:
            return cast(str, kwargs[field_name])
        raise InvalidConfiguration(f"Required field is absent. Expected field '{field_name}'. Got {kwargs}")

    @cached_property
    def name(self) -> str | None:
        return self._name

    @cached_property
    def root_dir(self) -> Path:
        return self()
