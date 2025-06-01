#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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
from typing import List, Tuple, Any, Union, Optional, Dict

from runner.logger import Log
from runner.utils import is_type_of, convert_underscore, indent as utils_indent, convert_minus

_LOGGER = Log.get_logger(__file__)


class IOptions(ABC):
    _parent: Optional['IOptions'] = None

    def __init__(self, params: Optional[Dict[str, Any]] = None):
        if params:
            for key, value in params.items():
                setattr(self, convert_minus(key), value)

    @staticmethod
    def __get_value_from_steps(value: List[Any], parts: List[str]) -> Optional[Any]:
        for item in value:
            if hasattr(item, 'name') and getattr(item, 'name') == parts[1]:
                new_value = item.get_value(parts[2:])
                if new_value is not None:
                    return new_value
        return None

    def properties(self) -> List[str]:
        attrs = dir(self)
        attrs = [n for n in attrs if self.__is_property(n)]
        return attrs

    def values(self) -> List[Tuple[str, Any]]:
        return [(attr, getattr(self, attr)) for attr in self.properties()]

    def parent(self) -> Optional['IOptions']:
        return self._parent

    def get_value(self, option: Union[str, List[str]]) -> Optional[Any]:
        if isinstance(option, str):
            parts = option.split(".")
        else:
            parts = option
        if (new_value := self.__get_value_from_properties(parts)) is not None:
            return new_value
        if (new_value := self.__get_value_from_parameters(parts)) is not None:
            return new_value
        if self._parent is not None:
            return self._parent.get_value(option)
        return None

    def get_command_line(self) -> str:
        return ""

    def to_dict(self) -> Dict[str, Any]:
        result: Dict[str, Any] = {}
        for attr, value in self.values():
            if isinstance(value, IOptions):
                result[attr] = value.to_dict()
            else:
                result[attr] = str(value)
        return result

    def _to_str(self, indent: int = 0) -> str:
        result = [f"{self.__class__.__name__}:"]
        indent_str = utils_indent(indent + 1)
        for prop_name, prop_value in self.values():
            if isinstance(prop_value, list):
                result += [f"{indent_str}{convert_underscore(prop_name)}:"]
                for sub_name in prop_value:
                    result += [f"{utils_indent(indent + 2)}{sub_name}"]
            else:
                result += [f"{indent_str}{convert_underscore(prop_name)}: {str(prop_value)}"]
        return "\n".join(result)

    def __get_value_from_parameters(self, parts: List[str]) -> Optional[Any]:
        parameters = "parameters"
        if parameters in self.properties():
            params = getattr(self, parameters).items()
            if parts and parts[0] in params:
                value = params[parts[0]]
                return value
        return None

    def __get_value_from_properties(self, parts: List[str]) -> Optional[Any]:
        if parts and parts[0] in self.properties():
            return self.__get_value_from_property(parts)
        return None

    def __get_value_from_property(self, parts: List[str]) -> Optional[Any]:
        value = getattr(self, parts[0])
        if len(parts) == 1:
            return value
        if isinstance(value, IOptions):
            return value.get_value(parts[1:])
        if isinstance(value, dict):
            if len(parts) == 2 and parts[1] in value:
                return value[parts[1]]
        elif isinstance(value, list) and len(value) > 0 and parts[0] == "steps":
            return self.__get_value_from_steps(value, parts)
        return None

    def __is_property(self, n: str) -> bool:
        return not n.startswith("_") and \
            not is_type_of(getattr(self, n), "method") and \
            not is_type_of(getattr(self, n), "function")
