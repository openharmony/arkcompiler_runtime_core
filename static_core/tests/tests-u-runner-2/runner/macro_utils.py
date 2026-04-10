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

import re
from dataclasses import dataclass
from typing import Any, ClassVar, cast

from runner.logger import Log
from runner.options.macros_modifiers import MacroModifier

_LOGGER = Log.get_logger(__file__)


@dataclass
class Macro:
    """
    Syntax:
        macro          ::= "${" macro-body "}"
        macro-body     ::= identifier [ ":" macro-modifier ]
        identifier     ::= segment { "." segment }
        segment        ::= letter { letter | digit | "-" | "_" }
        letter         ::= "a" | ... | "z" | "A" | ... | "Z"
        digit          ::= "0" | ... | "9"
        macro-modifier ::= "name" | "stem" | "parent"  # see MacroModifier items
    """
    value: str
    modifier: MacroModifier | None = None
    segment_pattern: ClassVar[str] = r"[a-zA-Z][a-zA-Z0-9\-_]*"
    identifier_pattern: ClassVar[str] = segment_pattern + r"(?:\." + segment_pattern + r")*"
    modifier_pattern: ClassVar[str] = "|".join(MacroModifier.values())
    body_pattern: ClassVar[str] = r"(?P<value>" + identifier_pattern + r")(?::(?P<modifier>" + modifier_pattern + r"))?"
    light_body_pattern: ClassVar[str] = r"\$\{(?P<body>[^}]*)\}"
    pattern: ClassVar[str] = r"\$\{" + body_pattern + r"\}"

    special_macros: ClassVar[tuple[str, ...]] = ('test-id',)

    def __str__(self) -> str:
        if self.modifier is None:
            return self.value
        return f"{self.value}:{self.modifier.value}"

    def __repr__(self) -> str:
        return f"{self.__class__.__name__}<{self!s}>"


def has_macro(prop_value: Any, *, skip_special: bool = False) -> bool:  # type: ignore[explicit-any]
    """
    Detects whether the given property value contains a macro.
    Usually prop_value is a str, but it can be any type

    Args:
        prop_value: value to check
        skip_special: do not take special macros into account

    Returns:
        True if the property value contains a macro, False otherwise
    """
    if not isinstance(prop_value, str):
        return False
    if skip_special:
        for spec in Macro.special_macros:
            prop_value = prop_value.replace(f"${{{spec}}}", "")
    result = re.search(Macro.pattern, str(prop_value))
    return result is not None


def list_has_macros(args: list[str]) -> bool:
    return any(has_macro(arg) for arg in args)


def get_all_macros(prop_value: str) -> tuple[list[Macro], list[str]]:
    correct_result: list[Macro] = get_correct_macros(prop_value)
    incorrect_result: list[str] = get_incorrect_macros(prop_value)
    return correct_result, incorrect_result


def get_correct_macros(prop_value: str) -> list[Macro]:
    correct_result: list[Macro] = []
    for item in re.finditer(Macro.pattern, prop_value):
        macro_value = item.group('value')
        modifier_str = item.group('modifier')
        modifier = MacroModifier.from_str(modifier_str) if modifier_str else None
        correct_result.append(Macro(value=macro_value, modifier=modifier))
    return correct_result


def get_incorrect_macros(prop_value: str) -> list[str]:
    result: list[str] = []
    for item in re.finditer(Macro.light_body_pattern, prop_value):
        macro_body = item.group('body')
        if not re.fullmatch(Macro.body_pattern, macro_body):
            result.append(macro_body)
    return result


def replace_macro(prop_value: str, macro: Macro, replacing_value: str) -> str:
    """
    Replace a macro in the given property value with the specified replacement value.

    Args:
        prop_value: The string containing the macro to be replaced
        macro: The Macro object representing the macro to replace
        replacing_value: The string value to substitute for the macro

    Returns:
        The string with the macro replaced, or the original string if the macro is not found

    Notes:
        - If the macro has a modifier, the replacement value is processed by the modifier
        - The replacement is case-insensitive
        - All occurrences of the macro are replaced (count=0)
    """
    pattern = r"\$\{" + str(macro) + r"\}"
    if macro.value in prop_value:
        if macro.modifier is not None:
            replacing_value = cast(str, macro.modifier.modify(replacing_value))
        match = re.sub(pattern, replacing_value, prop_value, count=0, flags=re.IGNORECASE)
        return match
    return prop_value


def tokenize(line: str) -> list[str]:
    return [token for token in re.split(r"(\$\{[^}]*})", line) if token]
