#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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

from os import environ
from typing import NamedTuple
from unittest import TestCase
from unittest.mock import Mock, patch

from runner.common_exceptions import MacroNotExpanded, MacroSyntaxError
from runner.macro_utils import Macro, get_all_macros, has_macro, replace_macro, tokenize
from runner.options.macros import Macros
from runner.options.macros_modifiers import MacroModifier


class ReplaceMacroTestCase(NamedTuple):
    input_string: str
    macro: Macro
    replacement: str
    expected: str


class ProcessTestCase(NamedTuple):
    input_string: str
    config_values: dict[str, str | list[str]]
    env_values: dict[str, str]
    expected_result: str | list[str]
    list_is_allowed: bool = False
    should_raise: type[Exception] | None = None


class CorrectMacroTestCase(NamedTuple):
    input_string: str
    config_values: dict[str, str | list[str]]
    env_values: dict[str, str]
    expected_result: str | list[str]
    should_raise: type[Exception] | None = None


class MacroUtilsTests(TestCase):
    def test_has_macro(self) -> None:
        cases = [
            # --- Plain strings without macro syntax ---
            ("test-id", False),
            ("plain text", False),
            ("", False),
            # --- Missing delimiters ---
            ("${TEST_ROOT", False),  # missing closing brace
            ("$TEST_ROOT}", False),  # missing opening brace
            ("{TEST_ROOT}", False),  # missing dollar sign
            ("$ {TEST_ROOT}", False),  # space between $ and {
            # --- Valid macros: simple identifiers ---
            ("${test-id}", True),
            ("${TEST_ROOT}", True),
            ("${a}", True),  # single letter
            ("${A}", True),  # single uppercase letter
            ("${abc}", True),  # all letters
            ("${a1}", True),  # letter then digit
            ("${a-b}", True),  # letter-hyphen-letter
            ("${a_b}", True),  # letter-underscore-letter
            ("${a1-b2_c3}", True),  # mixed
            # --- Valid macros: dotted identifiers ---
            ("${parameters.work-dir}", True),
            ("${a.b}", True),  # two single-letter segments
            ("${a.b.c}", True),  # three segments
            ("${seg1.seg2.seg3}", True),  # three multi-char segments
            # --- Valid macros: with valid modifiers ---
            ("${test-id:parent}", True),
            ("${test-id:name}", True),
            ("${test-id:stem}", True),
            ("${parameters.work-dir:parent}", True),
            ("${parameters.work-dir:name}", True),
            ("${parameters.work-dir:stem}", True),
            # --- Invalid macro body: empty identifier ---
            ("${}", False),
            ("${:parent}", False),
            ("${:id}", False),
            # --- Invalid macro body: identifier starts with digit ---
            ("${1}", False),
            ("${1.2}", False),
            ("${1abc}", False),
            ("${0test}", False),
            # --- Invalid macro body: identifier starts with hyphen ---
            ("${-abc}", False),
            # --- Invalid macro body: identifier starts with underscore ---
            ("${_abc}", False),
            # --- Invalid macro body: consecutive dots ---
            ("${a..b}", False),
            # --- Invalid macro body: trailing dot ---
            ("${a.}", False),
            # --- Invalid macro body: leading dot ---
            ("${.a}", False),
            # --- Invalid macro body: dot-only ---
            ("${.}", False),
            # --- Invalid modifier ---
            ("${test-id:id}", False),
            ("${test-id:123}", False),
            ("${test-id:-123}", False),
            ("${test-id:NAME}", False),  # case-sensitive modifier
            # --- Invalid: multiple modifiers ---
            ("${test-id:parent:name:stem}", False),
            ("${test-id:parent:name}", False),
            # --- Macro embedded in larger string ---
            ("hello/${parameters.work-dir}/intermediate/${test-id}", True),
            ("prefix_${test-id}_suffix", True),
            ("no-macro-here but ${valid-one}", True),
            # --- Invalid macro embedded in larger string ---
            ("${1invalid} text", False),
            ("text ${-bad} more", False),
        ]

        for macro, expected in cases:
            with self.subTest(source=macro):
                actual = has_macro(macro)
                self.assertEqual(expected, actual, f"'{macro}' should be {expected} but was {actual}")

    def test_has_macro_skip_special(self) -> None:
        cases = [
            ("test-id", False),
            ("${test-id}", False),
            ("${parameters.work-dir}", True),
            ("hello/intermediate/${test-id}", False),
        ]

        for macro, expected in cases:
            with self.subTest(source=macro):
                actual = has_macro(macro, skip_special=True)
                self.assertEqual(expected, actual, f"'{macro}' should be {expected} but was {actual}")

    def test_get_all_macros(self) -> None:
        cases = [
            # --- No macros ---
            ("test-id", [], []),
            ("test-id:id", [], []),
            ("test-id:name:stem", [], []),
            (":parent", [], []),
            ("plain text", [], []),
            ("", [], []),
            # --- Invalid macro bodies: no macros extracted ---
            ("${}", [], [""]),
            ("${:id}", [], [":id"]),
            ("${1abc}", [], ["1abc"]),
            ("${-abc}", [], ["-abc"]),
            ("${_abc}", [], ["_abc"]),
            ("${a..b}", [], ["a..b"]),
            ("${a.}", [], ["a."]),
            ("${.a}", [], [".a"]),
            ("${test-id:id}", [], ["test-id:id"]),  # invalid modifier
            ("${test-id:parent:name:stem}", [], ["test-id:parent:name:stem"]),  # multiple modifiers
            # --- Single valid macro without modifier ---
            ("${test-id}", [Macro('test-id')], []),
            ("${TEST_ROOT}", [Macro('TEST_ROOT')], []),
            ("${a}", [Macro('a')], []),
            ("${parameters.work-dir}", [Macro('parameters.work-dir')], []),
            ("${a.b.c}", [Macro('a.b.c')], []),
            # --- Single valid macro with modifier ---
            ("${test-id:parent}/test-id", [Macro('test-id', MacroModifier.PARENT)], []),
            ("${test-id:name}", [Macro('test-id', MacroModifier.NAME)], []),
            ("${test-id:stem}", [Macro('test-id', MacroModifier.STEM)], []),
            ("${parameters.work-dir:parent}", [Macro('parameters.work-dir', MacroModifier.PARENT)], []),
            # --- Multiple valid macros ---
            ("${parameters.work-dir}/${test-id:stem}", [
                Macro('parameters.work-dir'),
                Macro("test-id", MacroModifier.STEM)], []),
            ("hello/${parameters.work-dir}/intermediate/${test-id}", [
                Macro('parameters.work-dir'),
                Macro('test-id')], []),
            ("hello/${test-id:parent}/intermediate/${test-id:stem}.txt", [
                Macro('test-id', MacroModifier.PARENT),
                Macro('test-id', MacroModifier.STEM)], []),
            ("hello/${test-id:name}/intermediate/${test-id:name}.txt", [
                Macro('test-id', MacroModifier.NAME),
                Macro('test-id', MacroModifier.NAME)], []),
            ("${a}/${b}/${c}", [Macro('a'), Macro('b'), Macro('c')], []),
            # --- Mix of valid and invalid macros: only valid ones extracted ---
            ("${parameters.work-dir}/${test-id:hello}", [Macro('parameters.work-dir')], ["test-id:hello"]),
            ("${valid-id} and ${1invalid}", [Macro('valid-id')], ["1invalid"]),
            ("${valid-id} and ${-bad}", [Macro('valid-id')], ["-bad"]),
        ]

        for macro, expected_correct, expected_incorrect in cases:
            with self.subTest(source=macro):
                actual, incorrect = get_all_macros(macro)
                self.assertEqual(expected_correct, actual,
                                 f"'{macro}' should be {expected_correct} but was {actual}")
                self.assertEqual(expected_incorrect, incorrect,
                                 f"'{macro}' incorrect macros should be {expected_incorrect} but was {incorrect}")

    def test_replace_macro(self) -> None:
        cases = [
            # Basic replacement without modifier
            ReplaceMacroTestCase(
                input_string="Hello ${test-id} world",
                macro=Macro("test-id"),
                replacement="my-test",
                expected="Hello my-test world"
            ),
            # Replacement with modifier
            ReplaceMacroTestCase(
                input_string="Path: ${test-id:parent}",
                macro=Macro("test-id", MacroModifier.PARENT),
                replacement="/path/to/my-test",
                expected="Path: /path/to"
            ),
            # Multiple occurrences
            ReplaceMacroTestCase(
                input_string="${test-id} and ${test-id}",
                macro=Macro("test-id"),
                replacement="my-test",
                expected="my-test and my-test"
            ),
            ReplaceMacroTestCase(
                input_string="${test-id:stem} and ${test-id:stem}",
                macro=Macro("test-id", MacroModifier.STEM),
                replacement="/path/to/my-test.py",
                expected="my-test and my-test"
            ),
            # Different macros - should only replace matching one
            ReplaceMacroTestCase(
                input_string="${test-id} and ${other-id}",
                macro=Macro("test-id"),
                replacement="my-test",
                expected="my-test and ${other-id}"
            ),
            ReplaceMacroTestCase(
                input_string="${test-id} and ${test-id:parent}",
                macro=Macro("test-id"),
                replacement="path/to/my-test",
                expected="path/to/my-test and ${test-id:parent}"
            ),
            ReplaceMacroTestCase(
                input_string="${test-id} and ${test-id:parent}",
                macro=Macro("test-id", MacroModifier.PARENT),
                replacement="path/to/my-test",
                expected="${test-id} and path/to"
            ),
            # No macro found - should return original
            ReplaceMacroTestCase(
                input_string="No macro here",
                macro=Macro("test-id"),
                replacement="my-test",
                expected="No macro here"
            ),
            ReplaceMacroTestCase(
                input_string="Another macro here ${test-root}",
                macro=Macro("test-id"),
                replacement="my-test",
                expected="Another macro here ${test-root}"
            ),
            # Case sensitive replacement - another case should not be replaced
            ReplaceMacroTestCase(
                input_string="Hello ${TEST-ID} world",
                macro=Macro("test-id"),
                replacement="my-test",
                expected="Hello ${TEST-ID} world"
            ),
            # Empty string replacement
            ReplaceMacroTestCase(
                input_string="Start ${test-id} end",
                macro=Macro("test-id"),
                replacement="",
                expected="Start  end"
            ),
            # Macro with dots in value
            ReplaceMacroTestCase(
                input_string="${parameters.work-dir}/file.txt",
                macro=Macro("parameters.work-dir"),
                replacement="/custom/path",
                expected="/custom/path/file.txt"
            ),
            ReplaceMacroTestCase(
                input_string="${parameters.work-dir:parent}/file.txt",
                macro=Macro("parameters.work-dir", MacroModifier.PARENT),
                replacement="/custom/path/nested",
                expected="/custom/path/file.txt"
            ),
        ]

        for case in cases:
            with self.subTest(input_string=f"{case.input_string}", macro=f'{case.macro}'):
                actual = replace_macro(case.input_string, case.macro, case.replacement)
                self.assertEqual(case.expected, actual,
                                 f"replace_macro('{case.input_string}', {case.macro}, "
                                 f"'{case.replacement}') should be '{case.expected}' but was '{actual}'")

    def test_process_nested_list_expanding(self) -> None:
        """
        Tests for Macro.process - nested list expansion.
        """
        cases = [
            # --- List item contains a macro that resolves to a plain string ---
            # Each item in the parent list is processed; a macro inside an item
            # should be replaced with its str value.
            ProcessTestCase(
                input_string="${parent-list}",
                config_values={
                    "parent-list": ["prefix/${child-val}/suffix", "plain"],
                    "child-val": "resolved",
                },
                env_values={},
                expected_result=["prefix/resolved/suffix", "plain"],
                list_is_allowed=True,
            ),

            # --- List item is a single macro that resolves to another list ---
            # _is_single_macro("${child-list}") is True, so list_is_allowed=True
            # for the nested call; the result is a list that gets extended into
            # the parent list (the core "nested list expanding" path).
            ProcessTestCase(
                input_string="${parent-list}",
                config_values={
                    "parent-list": ["${child-list}"],
                    "child-list": ["a", "b", "c"],
                },
                env_values={},
                expected_result=["a", "b", "c"],
                list_is_allowed=True,
            ),

            # --- Multiple items: some expand to lists, some to strings ---
            # The final result is the flattened union of all expansions.
            ProcessTestCase(
                input_string="${parent-list}",
                config_values={
                    "parent-list": ["${child-list}", "literal", "${str-macro}"],
                    "child-list": ["x", "y"],
                    "str-macro": "hello",
                },
                env_values={},
                expected_result=["x", "y", "literal", "hello"],
                list_is_allowed=True,
            ),

            # --- Nested list macro expands to an empty list ---
            # extend([]) is a no-op; the item simply disappears from the result.
            ProcessTestCase(
                input_string="${parent-list}",
                config_values={
                    "parent-list": ["before", "${empty-list}", "after"],
                    "empty-list": [],
                },
                env_values={},
                expected_result=["before", "after"],
                list_is_allowed=True,
            ),

            # --- List item with surrounding text + macro expanding to a list ---
            # _is_single_macro("text-${child-list}-more") is False, so
            # list_is_allowed=False for the nested call; expand_one_macro will
            # raise MacroNotExpanded because a list macro is not permitted here.
            ProcessTestCase(
                input_string="${parent-list}",
                config_values={
                    "parent-list": ["text-${child-list}-more"],
                    "child-list": ["a", "b"],
                },
                env_values={},
                expected_result=[],
                list_is_allowed=True,
                should_raise=MacroNotExpanded,
            ),

            # --- Two levels of nesting: list → list → str ---
            # parent-list contains "${mid-list}" which itself contains "${leaf-list}".
            # Each level is a single-macro item so list expansion is allowed at
            # every depth.
            ProcessTestCase(
                input_string="${parent-list}",
                config_values={
                    "parent-list": ["${mid-list}"],
                    "mid-list": ["${leaf-list}", "mid-literal"],
                    "leaf-list": ["leaf1", "leaf2"],
                },
                env_values={},
                expected_result=["leaf1", "leaf2", "mid-literal"],
                list_is_allowed=True,
            ),

            # --- Cyclic reference inside a nested list item ---
            # parent-list → child-list → item "${parent-list}" which is already
            # in the previous_macros chain → MacroNotExpanded (cyclic detection).
            ProcessTestCase(
                input_string="${parent-list}",
                config_values={
                    "parent-list": ["${child-list}"],
                    "child-list": ["${parent-list}"],
                },
                env_values={},
                expected_result=[],
                list_is_allowed=True,
                should_raise=MacroNotExpanded,
            ),

            # --- Syntax error in a nested list item (valid-looking macro with bad body) ---
            # has_macro("${a..b}", skip_special=True) returns False because the strict
            # Macro.pattern does not match an invalid body — so the item is NOT processed
            # and is kept literally. No error is raised.
            #
            # However, if a list item passes has_macro (i.e. looks like a valid macro
            # syntactically, e.g. "${a:bad-modifier}"), process() is called on it and
            # detect_macro_syntax_errors raises MacroSyntaxError.
            # Note: "${a:bad-modifier}" — has_macro returns False for invalid modifier too,
            # so the syntax check only fires for items that contain BOTH a valid macro AND
            # an adjacent invalid macro form in the same item string.
            # The most reliable trigger: an item that contains a valid macro alongside
            # an invalid macro token, e.g. "${valid-key} ${-bad}".
            ProcessTestCase(
                input_string="${parent-list}",
                config_values={
                    "parent-list": ["${valid-key} ${-bad}"],
                    "valid-key": "ok",
                },
                env_values={},
                expected_result=[],
                list_is_allowed=True,
                should_raise=MacroSyntaxError,
            ),

            # --- Invalid macro body in a list item should raise MacroSyntaxError ---
            ProcessTestCase(
                input_string="${parent-list}",
                config_values={
                    "parent-list": ["good-item", "${1bad}"],
                },
                env_values={},
                expected_result=["good-item", "${1bad}"],
                list_is_allowed=True,
                should_raise=MacroSyntaxError,
            ),

            # --- Nested list item macro is a macro with a modifier ---
            # Modifier (e.g. :parent) applied to a str-valued child macro inside
            # a list item with surrounding text.
            ProcessTestCase(
                input_string="${parent-list}",
                config_values={
                    "parent-list": ["${child-path:parent}", "other"],
                    "child-path": "/some/deep/path/file.txt",
                },
                env_values={},
                expected_result=["/some/deep/path", "other"],
                list_is_allowed=True,
            ),

            # --- list_is_allowed=False at the top level while parent-list is a list ---
            # expand_one_macro raises MacroNotExpanded immediately because the macro
            # would expand to a list but list_is_allowed=False.
            ProcessTestCase(
                input_string="${parent-list}",
                config_values={
                    "parent-list": ["${child-list}"],
                    "child-list": ["a", "b"],
                },
                env_values={},
                expected_result=[],
                list_is_allowed=False,
                should_raise=MacroNotExpanded,
            ),
        ]

        for case in cases:
            with self.subTest(input_string=case.input_string,
                              config_values=case.config_values):
                self.process_case_test(case)

    def test_process(self) -> None:
        cases = [
            # Basic macro replacement from config
            ProcessTestCase(
                input_string="Hello ${test-id} world",
                config_values={"test-id": "my-test"},
                env_values={},
                expected_result="Hello my-test world"
            ),
            # Macro replacement from environment (takes precedence)
            ProcessTestCase(
                input_string="Path: ${parameters.work-dir-env}",
                config_values={"parameters.work-dir-env": "/config/path"},
                env_values={"parameters.work-dir-env": "/env/path"},
                expected_result="Path: /env/path"
            ),
            # Macro with modifier
            ProcessTestCase(
                input_string="Parent: ${test-id:parent}",
                config_values={"test-id": "/path/to/test"},
                env_values={},
                expected_result="Parent: /path/to"
            ),
            # Multiple macros
            ProcessTestCase(
                input_string="${test-id} in ${parameters.work-dir}",
                config_values={"test-id": "my-test", "parameters.work-dir": "/work/dir"},
                env_values={},
                expected_result="my-test in /work/dir"
            ),
            # Recursive correct macros
            ProcessTestCase(
                input_string="Recursive correct: ${parameters.work-dir}",
                config_values={"a.b": "my-test", "parameters.work-dir": "/work/dir/${a.b}"},
                env_values={},
                expected_result="Recursive correct: /work/dir/my-test"
            ),
            # Recursive incorrect macros
            ProcessTestCase(
                input_string="Recursive incorrect: ${parameters.work-dir}",
                config_values={"a.b": "my-test", "parameters.work-dir": "/work/dir/${a.b.c}"},
                env_values={},
                expected_result="Recursive incorrect: /work/dir/my-test",
                should_raise=MacroNotExpanded
            ),
            # Cyclic simple recursive incorrect macros
            ProcessTestCase(
                input_string="Cyclic recursive simple: ${parameters.work-dir}",
                config_values={"parameters.work-dir": "/work/dir/${parameters.work-dir}"},
                env_values={},
                expected_result="Value: /work/dir/my-test",
                should_raise=MacroNotExpanded
            ),
            # Cyclic complex recursive incorrect macros
            ProcessTestCase(
                input_string="Cyclic recursive complex: ${parameters.work-dir}",
                config_values={"parameters.work-dir": "/work/dir/${aaa}", "aaa": "aaa/${parameters.work-dir}"},
                env_values={},
                expected_result="Value: /work/dir/my-test",
                should_raise=MacroNotExpanded
            ),
            # Special macro (should not raise error)
            ProcessTestCase(
                input_string="Special: ${test-id}",
                config_values={},
                env_values={},
                expected_result="Special: ${test-id}"
            ),
            # Non-special macro not found (should raise error)
            ProcessTestCase(
                input_string="Missing: ${unknown-macro}",
                config_values={},
                env_values={},
                expected_result="",
                should_raise=MacroNotExpanded
            ),
            # List value handling
            ProcessTestCase(
                input_string="${my-list}",
                config_values={"my-list": ["item1", "item2", "item3"]},
                env_values={},
                expected_result=["item1", "item2", "item3"],
                list_is_allowed=True
            ),
            ProcessTestCase(
                input_string="${my-empty-list}",
                config_values={"my-empty-list": []},
                env_values={},
                expected_result=[],
                list_is_allowed=True
            ),
            ProcessTestCase(
                input_string="Surrounding ${my-list} text",
                config_values={"my-list": ["item1", "item2", "item3"]},
                env_values={},
                expected_result="",
                should_raise=MacroNotExpanded
            ),
            ProcessTestCase(
                input_string="Surrounding ${my-empty-list} text",
                config_values={"my-empty-list": []},
                env_values={},
                expected_result="",
                should_raise=MacroNotExpanded
            ),
            # Macro with parameters (remove parameters prefix)
            ProcessTestCase(
                input_string="Param: ${parameters.test-value}",
                config_values={"test_value": "param-value"},
                env_values={},
                expected_result="Param: param-value"
            ),
            # --- Incorrect macro bodies: should raise MacroSyntaxError ---
            # Empty macro body
            ProcessTestCase(
                input_string="Value: ${}",
                config_values={},
                env_values={},
                expected_result="",
                should_raise=MacroSyntaxError
            ),
            # Identifier starting with digit
            ProcessTestCase(
                input_string="Value: ${1abc}",
                config_values={},
                env_values={},
                expected_result="",
                should_raise=MacroSyntaxError
            ),
            # Identifier starting with hyphen
            ProcessTestCase(
                input_string="Value: ${-bad}",
                config_values={},
                env_values={},
                expected_result="",
                should_raise=MacroSyntaxError
            ),
            # Identifier starting with underscore
            ProcessTestCase(
                input_string="Value: ${_bad}",
                config_values={},
                env_values={},
                expected_result="",
                should_raise=MacroSyntaxError
            ),
            # Consecutive dots in identifier
            ProcessTestCase(
                input_string="Value: ${a..b}",
                config_values={},
                env_values={},
                expected_result="",
                should_raise=MacroSyntaxError
            ),
            # Trailing dot in identifier
            ProcessTestCase(
                input_string="Value: ${a.}",
                config_values={},
                env_values={},
                expected_result="",
                should_raise=MacroSyntaxError
            ),
            # Incorrect modifier
            ProcessTestCase(
                input_string="Value: ${a:a}",
                config_values={},
                env_values={},
                expected_result="",
                should_raise=MacroSyntaxError
            ),
            # Mix of valid and incorrect macros: incorrect one triggers error
            ProcessTestCase(
                input_string="${valid-id} and ${1invalid}",
                config_values={"valid-id": "ok"},
                env_values={},
                expected_result="",
                should_raise=MacroSyntaxError
            ),
        ]

        for case in cases:
            with self.subTest(input_string=case.input_string):
                self.process_case_test(case)

    def process_case_test(self, case: ProcessTestCase) -> None:
        mock_config = Mock()
        mock_config.get_value.side_effect = case.config_values.get

        with patch.dict(environ, case.env_values, clear=True):
            if case.should_raise:
                with self.assertRaises(case.should_raise):
                    Macros.process(case.input_string, case.input_string, mock_config, case.list_is_allowed)
            else:
                result = Macros.process(case.input_string, case.input_string, mock_config, case.list_is_allowed)
                self.assertEqual(case.expected_result, result,
                                 f"Expected result '{case.expected_result}' but got '{result}'")

    def test_correct_macro(self) -> None:
        cases = [
            # --- No macros: raw value returned unchanged ---
            CorrectMacroTestCase(
                input_string="",
                config_values={},
                env_values={},
                expected_result=""
            ),
            CorrectMacroTestCase(
                input_string="/some/path/without/macros",
                config_values={},
                env_values={},
                expected_result="/some/path/without/macros"
            ),
            # --- Macro embedded in a string: returns str ---
            CorrectMacroTestCase(
                input_string="${work-dir}/tests",
                config_values={"work-dir": "/home/user/work"},
                env_values={},
                expected_result="/home/user/work/tests"
            ),
            # --- Macro expanding to multiple whitespace-separated tokens:
            #     returns str ---
            CorrectMacroTestCase(
                input_string="${my-flags}",
                config_values={"my-flags": "--flag1 --flag2 --flag3"},
                env_values={},
                expected_result="--flag1 --flag2 --flag3"
            ),
            # --- Macro expands to list value (joined with space then split):
            #     single-item list gives str, multi-item gives list ---
            CorrectMacroTestCase(
                input_string="${my-multi-value-list}",
                config_values={"my-multi-value-list": ["item1", "item2", "item3"]},
                env_values={},
                expected_result=["item1", "item2", "item3"]
            ),
            CorrectMacroTestCase(
                input_string="${my-one-value-list}",
                config_values={"my-one-value-list": ["only-item"]},
                env_values={},
                expected_result=["only-item"]
            ),
            CorrectMacroTestCase(
                input_string="${my-empty-list}",
                config_values={"my-empty-list": []},
                env_values={},
                expected_result=[]
            ),
            # --- Space inside a list item is encoded as %20 and preserved ---
            CorrectMacroTestCase(
                input_string="${my-list}",
                config_values={"my-list": ["hello world", "foo"]},
                env_values={},
                expected_result=["hello world", "foo"]
            ),
            # --- Macro produces a list has prepended and following text
            #     that is returned as list ---
            CorrectMacroTestCase(
                input_string="List ${my-flags} extra",
                config_values={"my-flags": ["--flag1", "--flag2"]},
                env_values={},
                expected_result="",
                should_raise=MacroNotExpanded
            ),
            # --- Macro followed by static text produces a multi-token string
            #     that is returned as str ---
            CorrectMacroTestCase(
                input_string="${my-flags} extra",
                config_values={"my-flags": "--flag1 --flag2"},
                env_values={},
                expected_result="--flag1 --flag2 extra"
            ),
            # --- Single macro that expands to a list where items are single macros
            #     that each expand to lists themselves ---
            CorrectMacroTestCase(
                input_string="${parent-list}",
                config_values={
                    "parent-list": ["${child-list-a}", "${empty}", "${child-list-b}"],
                    "child-list-a": ["a1", "a2"],
                    "child-list-b": ["b1", "b2"],
                    "empty": [],
                },
                env_values={},
                expected_result=["a1", "a2", "b1", "b2"],
            ),

            # --- Macro in raw_value has surrounding text: list not allowed at top
            #     level, so even if first token expands to list it raises ---
            CorrectMacroTestCase(
                input_string="extra ${outer} text",
                config_values={
                    "outer": ["${child-list}"],
                    "child-list": ["a", "b"],
                },
                env_values={},
                expected_result=[],
                should_raise=MacroNotExpanded,
            ),
        ]

        for case in cases:
            with self.subTest(input_string=case.input_string):
                self.correct_macro_case_test(case)

    def correct_macro_case_test(self, case: CorrectMacroTestCase) -> None:
        mock_config = Mock()
        mock_config.get_value.side_effect = case.config_values.get

        with patch.dict(environ, case.env_values, clear=True):
            if case.should_raise:
                with self.assertRaises(case.should_raise):
                    Macros.correct_macro(case.input_string, mock_config)
            else:
                result = Macros.correct_macro(case.input_string, mock_config)
                self.assertEqual(case.expected_result, result,
                                 f"correct_macro('{case.input_string}') "
                                 f"expected '{case.expected_result}' but got '{result}'")

    def test_tokenize(self) -> None:
        cases: list[tuple[str, list[str]]] = [
            # --- Empty and plain strings: no macros, returned as single token ---
            ("", []),
            ("plain text", ["plain text"]),
            ("/some/path/without/macros", ["/some/path/without/macros"]),

            # --- Single macro token only ---
            ("${test-id}", ["${test-id}"]),
            ("${parameters.work-dir}", ["${parameters.work-dir}"]),
            ("${test-id:parent}", ["${test-id:parent}"]),

            # --- Invalid macro bodies are still tokenized (light tokenizer) ---
            ("${}", ["${}"]),
            ("${1abc}", ["${1abc}"]),
            ("${a..b}", ["${a..b}"]),

            # --- Text surrounding macro ---
            ("prefix/${test-id}/suffix", ["prefix/", "${test-id}", "/suffix"]),
            ("--work-dir ${parameters.work-dir}/intermediate",
             ["--work-dir ", "${parameters.work-dir}", "/intermediate"]),
            ("hello ${test-id} world", ["hello ", "${test-id}", " world"]),

            # --- Multiple macros with surrounding text ---
            ("${parameters.work-dir}/${test-id:stem}",
             ["${parameters.work-dir}", "/", "${test-id:stem}"]),
            ("hello/${parameters.work-dir}/intermediate/${test-id}",
             ["hello/", "${parameters.work-dir}", "/intermediate/", "${test-id}"]),
            ("${a} and ${b} and ${c}",
             ["${a}", " and ", "${b}", " and ", "${c}"]),

            # --- Invalid macro body mixed with surrounding text ---
            ("text ${1invalid} more", ["text ", "${1invalid}", " more"]),
            ("${valid-id} and ${-bad}", ["${valid-id}", " and ", "${-bad}"]),

            # --- Missing delimiters: treated as plain text ---
            ("${TEST_ROOT", ["${TEST_ROOT"]),
            ("${TEST_ROOT${root}", ["${TEST_ROOT${root}"]),
            ("$TEST_ROOT}", ["$TEST_ROOT}"]),
            ("$TEST_ROOT}${root}", ["$TEST_ROOT}", "${root}"]),
            ("{TEST_ROOT}${root}", ["{TEST_ROOT}", "${root}"]),
        ]

        for line, expected in cases:
            with self.subTest(line=line):
                actual = tokenize(line)
                self.assertEqual(expected, actual,
                                 f"tokenize('{line}') expected {expected} but got {actual}")
