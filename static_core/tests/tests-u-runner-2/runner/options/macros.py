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
from os import environ

from runner.common_exceptions import MacroNotExpanded, MacroSyntaxError
from runner.logger import Log
from runner.macro_utils import Macro, get_all_macros, has_macro, replace_macro, tokenize
from runner.options.local_env import LocalEnv
from runner.options.macros_modifiers import MacroModifier
from runner.options.options import IOptions
from runner.utils import expand_file_name

_LOGGER = Log.get_logger(__file__)

ExpandedValueType = str | list[str]


def _is_single_macro(value: str) -> bool:
    """Return True if value consists of exactly one macro token and nothing else."""
    tokens = tokenize(value)
    return len(tokens) == 1 and value.startswith('${') and value.endswith('}')


class Macros:
    @staticmethod
    def add_steps_2_macro(macro: str) -> str:
        steps = 'steps'
        if macro.startswith(f'{steps}.') and not macro.startswith(f'{steps}.{steps}.'):
            return f"{steps}.{macro}"
        return macro

    @staticmethod
    def remove_parameters_and_minuses(macro: str) -> str:
        parameters = "parameters."
        if macro.find(parameters) >= 0:
            macro = macro.replace(parameters, "")
        return macro.replace("-", "_")

    @staticmethod
    def process_special_macros(line: str, test_id: str) -> str:
        result = line
        for spec in Macro.special_macros:
            result = replace_macro(result, Macro(spec), test_id)
            for mod in MacroModifier:
                result = replace_macro(result, Macro(spec, mod), test_id)
        return result

    @classmethod
    def expand_macros_in_path(cls, value: str, config: IOptions) -> str | None:
        corrected = cls.correct_macro(value, config)
        return expand_file_name(corrected) if isinstance(corrected, str) else None

    @classmethod
    def correct_macro(cls, raw_value: str, config: IOptions) -> ExpandedValueType:
        """
        Expand all macros in raw_value.

        :param raw_value: the source line with one or several macros
        :param config: object with options where macros are searched
        :return: the line with expanded macros, or a list of strings

        Rules for list expansion:
        - A macro can expand to a list only if raw_value consists of EXACTLY one macro
          (no surrounding text, no other macros). Otherwise MacroNotExpanded is raised.
        - Any macro expanding into a list can contain nested macros, but those nested
          macros must expand to str (no nested list).
        - If a macro expands to a str that itself contains macros, those are expanded
          recursively (with cycle detection).
        """
        if not has_macro(raw_value, skip_special=True):
            return raw_value

        list_is_allowed = _is_single_macro(raw_value)

        tokens = tokenize(raw_value)

        # Process each token, expanding macros in it.
        # For list context (single token): the token may return a list.
        # For non-list context (multiple tokens): if a macro expands to a list, raise MacroNotExpanded.
        processed_tokens: list[str | list[str]] = []
        for token in tokens:
            expanded = cls.process(token, raw_value, config, list_is_allowed, previous_macros=[])
            processed_tokens.append(expanded)

        # If we have a single token that expanded to a list, return the list as-is
        if list_is_allowed and len(processed_tokens) == 1 and isinstance(processed_tokens[0], list):
            return processed_tokens[0]

        # Concatenate all token strings into a single string
        if any(isinstance(token, list) for token in processed_tokens):
            raise MacroNotExpanded("Macro expanded to a list, but it was not allowed")
        result_str = "".join([token for token in processed_tokens if isinstance(token, str)])

        if result_str != raw_value:
            _LOGGER.all(f"Corrected macro: '{raw_value}' => '{result_str}'")

        return result_str

    @classmethod
    def process(cls, input_string: str, raw_value: str, config: IOptions,
                list_is_allowed: bool = False,
                previous_macros: list[str] | None = None) -> ExpandedValueType:
        """
        Internal method: expand all macros in `input_string`.

        :param input_string: the string to expand
        :param raw_value: original raw value for error messages
        :param config: options object
        :param list_is_allowed: whether this token may expand to a list
        :param previous_macros: chain of macro names already being expanded (cycle detection)
        :return: expanded string or list[str]
        """
        if previous_macros is None:
            previous_macros = []

        result: ExpandedValueType = input_string
        correct_macros, incorrect_macros = get_all_macros(input_string)
        cls.detect_macro_syntax_errors(raw_value, incorrect_macros)
        cls.detect_cyclic_reference(correct_macros, previous_macros)

        for macro in correct_macros:
            prop_value = cls.expand_one_macro(macro, config, list_is_allowed)
            if prop_value is None:
                cls.detect_not_expanded_macro(raw_value, macro)
                continue

            if isinstance(prop_value, list):
                expanded_items: list[str] = []
                for item in prop_value:
                    _, incorrect_macros = get_all_macros(item)
                    cls.detect_macro_syntax_errors(raw_value, incorrect_macros)
                    if has_macro(item, skip_special=True):
                        expanded_item = cls.process(
                            item, raw_value, config,
                            list_is_allowed=_is_single_macro(item),
                            previous_macros=[*previous_macros, macro.value]
                        )
                        if isinstance(expanded_item, list):
                            expanded_items.extend(expanded_item)
                        else:
                            expanded_items.append(expanded_item)
                    else:
                        expanded_items.append(item)
                return expanded_items

            # prop_value is a str
            result = replace_macro(str(result), macro, prop_value)

            # Recursively expand any macros that appeared in the expanded value
            if has_macro(prop_value, skip_special=True):
                result = cls.process(
                    result, raw_value, config,
                    list_is_allowed=list_is_allowed,
                    previous_macros=[*previous_macros, macro.value]
                )

        return result

    @classmethod
    def detect_macro_syntax_errors(cls, raw_value: str, incorrect_macros: list[str]) -> None:
        incorrect_macros = [macro for macro in incorrect_macros if macro not in Macro.special_macros]
        if incorrect_macros:
            raise MacroSyntaxError(
                f"Macro syntax errors: '{incorrect_macros}' at value '{raw_value}'.")

    @classmethod
    def detect_cyclic_reference(cls, correct_macros: list[Macro], previous_macros: list[str]) -> None:
        if seen_macros := {macro.value for macro in correct_macros if macro.value in previous_macros}:
            raise MacroNotExpanded(
                f"Cyclic macro reference detected: {seen_macros}"
            )

    @classmethod
    def detect_not_expanded_macro(cls, raw_value: str, macro: Macro) -> None:
        if macro.value not in Macro.special_macros:
            raise MacroNotExpanded(
                f"Cannot expand macro '{macro!s}' at value '{raw_value}'. "
                "Check yaml path or provide the value as environment variable")

    @classmethod
    def expand_one_macro(cls, macro: Macro, config: IOptions,
                         list_is_allowed: bool) -> ExpandedValueType | None:
        # Search macro in the Environment
        local_env = LocalEnv.get()
        if local_env.is_present(macro.value):
            prop_value: ExpandedValueType | None = local_env.get_value(macro.value)
        else:
            prop_value = environ.get(macro.value)
            if prop_value is not None:
                local_env.add(macro.value, str(prop_value))
        # Search macro in the Config
        prop_value = config.get_value(macro.value) if prop_value is None else prop_value
        if prop_value is None:
            underscored_macro = cls.remove_parameters_and_minuses(macro.value)
            prop_value = config.get_value(underscored_macro)

        # Handle list expansion
        if prop_value is not None and isinstance(prop_value, list):
            if not list_is_allowed:
                raise MacroNotExpanded(
                    f"{macro!s} is expanded into list but there is additional text. "
                    "Only one macro without additional text can be expanded into list."
                )
            # Return the list as-is (even single-item or empty lists)
        elif prop_value is not None and not isinstance(prop_value, str):
            # Convert any non-str, non-list value (e.g. Path, bool, int) to str
            prop_value = str(prop_value)

        return prop_value
