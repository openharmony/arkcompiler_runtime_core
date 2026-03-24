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

# pylint: disable=protected-access

import unittest
from unittest.mock import MagicMock

from runner.reports.spec_node import SpecNode
from runner.reports.spec_report import SpecReport


def _make_test(test_id: str, passed: bool | None = True,
               ignored: bool = False, excluded: bool = False) -> MagicMock:
    test = MagicMock()
    test.test_id = test_id
    test.passed = passed
    test.ignored = ignored
    test.excluded = excluded
    return test


def _make_report_shell() -> SpecReport:
    """Create a SpecReport instance bypassing __init__."""
    instance = object.__new__(SpecReport)
    instance.spec = SpecNode("SUMMARY", "", "", None)
    instance.has_excluded = False
    instance.tests = []
    return instance


class SpecReportCounterTest(unittest.TestCase):

    def test_acc_counter_passed(self) -> None:
        """
        Feature: accumulated counter for a passed test
        Expected:
        - total_acc and passed_acc incremented by 1, failed_acc stays 0
        """
        report = _make_report_shell()
        node = SpecNode("Ch1", "1", "", None)
        test = _make_test("05.generics/file.ets", passed=True, ignored=False, excluded=False)
        report._SpecReport__update_counters(test, node)  # type: ignore[attr-defined]
        self.assertEqual(node.total_acc, 1)
        self.assertEqual(node.passed_acc, 1)
        self.assertEqual(node.failed_acc, 0)

    def test_acc_counter_failed(self) -> None:
        """
        Feature: accumulated counter for a failed test
        Expected:
        - total_acc and failed_acc incremented by 1, passed_acc stays 0
        """
        report = _make_report_shell()
        node = SpecNode("Ch1", "1", "", None)
        test = _make_test("file.ets", passed=False, ignored=False, excluded=False)
        report._SpecReport__update_counters(test, node)  # type: ignore[attr-defined]
        self.assertEqual(node.total_acc, 1)
        self.assertEqual(node.failed_acc, 1)
        self.assertEqual(node.passed_acc, 0)

    def test_acc_counter_ignored_passed(self) -> None:
        """
        Feature: accumulated counter for an ignored test that actually passed
        Expected:
        - ignored_but_passed_acc incremented, passed_acc stays 0 (not double-counted)
        """
        report = _make_report_shell()
        node = SpecNode("Ch1", "1", "", None)
        test = _make_test("file.ets", passed=True, ignored=True)
        report._SpecReport__update_counters(test, node)  # type: ignore[attr-defined]
        self.assertEqual(node.ignored_but_passed_acc, 1)
        self.assertEqual(node.passed_acc, 0)

    def test_acc_counter_ignored_failed(self) -> None:
        """
        Feature: accumulated counter for an ignored test that failed
        Expected:
        - ignored_acc incremented, failed_acc stays 0 (classified as ignored, not failed)
        """
        report = _make_report_shell()
        node = SpecNode("Ch1", "1", "", None)
        test = _make_test("file.ets", passed=False, ignored=True)
        report._SpecReport__update_counters(test, node)  # type: ignore[attr-defined]
        self.assertEqual(node.ignored_acc, 1)
        self.assertEqual(node.failed_acc, 0)

    def test_acc_counter_excluded(self) -> None:
        """
        Feature: accumulated counter for an excluded test
        Expected:
        - excluded_after_acc incremented and has_excluded flag set to True
        """
        report = _make_report_shell()
        node = SpecNode("Ch1", "1", "", None)
        test = _make_test("file.ets", passed=False, excluded=True)
        report._SpecReport__update_counters(test, node)  # type: ignore[attr-defined]
        self.assertEqual(node.excluded_after_acc, 1)
        self.assertTrue(report.has_excluded)

    def test_acc_counter_none_passed(self) -> None:
        """
        Feature: accumulated counter when passed=None (test did not run to completion)
        Expected:
        - treated as failed: failed_acc incremented by 1
        """
        report = _make_report_shell()
        node = SpecNode("Ch1", "1", "", None)
        test = _make_test("file.ets", passed=None, ignored=False)
        report._SpecReport__update_counters(test, node)  # type: ignore[attr-defined]
        self.assertEqual(node.failed_acc, 1)

    def test_non_acc_counter_passed(self) -> None:
        """
        Feature: own (non-accumulated) counter on the deepest matched node
        Expected:
        - node.total and node.passed incremented by 1, node.failed stays 0
        """
        report = _make_report_shell()
        node = SpecNode("Ch1", "1", "", None)
        test = _make_test("file.ets", passed=True)
        report._SpecReport__update_last_node_counters(test, node)  # type: ignore[attr-defined]
        self.assertEqual(node.total, 1)
        self.assertEqual(node.passed, 1)
        self.assertEqual(node.failed, 0)

    def test_acc_counter_accumulates(self) -> None:
        """
        Feature: accumulated counters sum across multiple tests on the same node
        Expected:
        - total_acc=3, passed_acc=1, failed_acc=1, ignored_but_passed_acc=1
        """
        report = _make_report_shell()
        node = SpecNode("Ch1", "1", "", None)
        report._SpecReport__update_counters(  # type: ignore[attr-defined]
            _make_test("a.ets", passed=True), node)
        report._SpecReport__update_counters(  # type: ignore[attr-defined]
            _make_test("b.ets", passed=False), node)
        report._SpecReport__update_counters(  # type: ignore[attr-defined]
            _make_test("c.ets", passed=True, ignored=True), node)
        self.assertEqual(node.total_acc, 3)
        self.assertEqual(node.passed_acc, 1)
        self.assertEqual(node.failed_acc, 1)
        self.assertEqual(node.ignored_but_passed_acc, 1)


def _make_report_with_chapters(ch_count: int = 7) -> SpecReport:
    report = _make_report_shell()
    for i in range(1, ch_count + 1):
        SpecNode(f"Chapter {i}", str(i), "", report.spec)
    return report


class SpecReportCalculateTest(unittest.TestCase):

    def test_single_level_path(self) -> None:
        """
        Feature: map a test with a single-level directory path (e.g. "05.generics/file.ets")
        Expected:
        - root and chapter 5 both get total_acc=1; chapter 5 gets total=1 and dir set
        """
        report = _make_report_with_chapters()
        test = _make_test("05.generics/file.ets", passed=True)
        report._SpecReport__calculate_one_test(test)  # type: ignore[attr-defined]
        self.assertEqual(report.spec.total_acc, 1)
        ch5 = report.spec.children[4]
        self.assertEqual(ch5.total_acc, 1)
        self.assertEqual(ch5.total, 1)
        self.assertEqual(ch5.dir, "05.generics")

    def test_two_level_path(self) -> None:
        """
        Feature: map a test with a two-level directory path (e.g. "05.generics/03.utility_types/file.ets")
        Expected:
        - counters propagated to root, chapter 5, and subsection 5.3; dir set on deepest node
        """
        report = _make_report_with_chapters()
        SpecNode("Utility Types", "5.1", "", report.spec.children[4])
        SpecNode("Generic Inst", "5.2", "", report.spec.children[4])
        SpecNode("Utility Types", "5.3", "", report.spec.children[4])
        test = _make_test("05.generics/03.utility_types/file.ets", passed=True)
        report._SpecReport__calculate_one_test(test)  # type: ignore[attr-defined]
        self.assertEqual(report.spec.total_acc, 1)
        ch5 = report.spec.children[4]
        self.assertEqual(ch5.total_acc, 1)
        sec3 = ch5.children[2]
        self.assertEqual(sec3.total_acc, 1)
        self.assertEqual(sec3.total, 1)
        self.assertEqual(sec3.dir, "05.generics/03.utility_types")

    def test_gap_filling(self) -> None:
        """
        Feature: create placeholder nodes for chapters that precede the target
        Expected:
        - test for "03.types" on empty tree creates 3 children; first two have title "?"
        - third node gets the test counters and dir
        """
        report = _make_report_shell()
        test = _make_test("03.types/file.ets", passed=True)
        report._SpecReport__calculate_one_test(test)  # type: ignore[attr-defined]
        self.assertEqual(len(report.spec.children), 3)
        self.assertEqual(report.spec.children[0].title, "?")
        self.assertEqual(report.spec.children[1].title, "?")
        self.assertEqual(report.spec.children[2].dir, "03.types")
        self.assertEqual(report.spec.children[2].total, 1)

    def test_multiple_tests_same_chapter(self) -> None:
        """
        Feature: multiple tests in the same chapter accumulate counters correctly
        Expected:
        - root total_acc=2; chapter 5 gets total_acc=2, passed_acc=1, failed_acc=1
        """
        report = _make_report_with_chapters()
        report._SpecReport__calculate_one_test(  # type: ignore[attr-defined]
            _make_test("05.generics/file1.ets", passed=True))
        report._SpecReport__calculate_one_test(  # type: ignore[attr-defined]
            _make_test("05.generics/file2.ets", passed=False))
        self.assertEqual(report.spec.total_acc, 2)
        ch5 = report.spec.children[4]
        self.assertEqual(ch5.total_acc, 2)
        self.assertEqual(ch5.passed_acc, 1)
        self.assertEqual(ch5.failed_acc, 1)

    def test_multiple_tests_different_chapters(self) -> None:
        """
        Feature: tests in different chapters update their respective nodes independently
        Expected:
        - root total_acc=2; chapter 5 and chapter 7 each get total_acc=1
        """
        report = _make_report_with_chapters()
        report._SpecReport__calculate_one_test(  # type: ignore[attr-defined]
            _make_test("05.generics/file.ets", passed=True))
        report._SpecReport__calculate_one_test(  # type: ignore[attr-defined]
            _make_test("07.expressions/file.ets", passed=True))
        self.assertEqual(report.spec.total_acc, 2)
        self.assertEqual(report.spec.children[4].total_acc, 1)
        self.assertEqual(report.spec.children[6].total_acc, 1)


class SpecReportOutputTest(unittest.TestCase):

    def test_fmt_str_escapes(self) -> None:
        """
        Feature: __fmt_str escapes a mix of markdown-special characters in one string
        Expected:
        - pipe, underscore, angle brackets all escaped with backslash
        """
        result = SpecReport._SpecReport__fmt_str("a|b_c<d>e")  # type: ignore[attr-defined]
        self.assertEqual(result, "a\\|b\\_c\\<d\\>e")

    def test_fmt_str_escapes_all_special(self) -> None:
        """
        Feature: __fmt_str correctly escapes every individual markdown-special character
        Expected:
        - each of the 18 special chars (backslash, pipe, underscore, angle brackets,
          asterisk, backtick, braces, brackets, parens, hash, plus, minus, dot, bang)
          is prefixed with a backslash
        """
        for char, escaped in [("\\", "\\\\"), ("|", "\\|"), ("_", "\\_"),
                              ("<", "\\<"), (">", "\\>"), ("*", "\\*"),
                              ("`", "\\`"), ("{", "\\{"), ("}", "\\}"),
                              ("[", "\\["), ("]", "\\]"), ("(", "\\("),
                              (")", "\\)"), ("#", "\\#"), ("+", "\\+"),
                              ("-", "\\-"), (".", "\\."), ("!", "\\!")]:
            with self.subTest(char=char):
                self.assertEqual(
                    SpecReport._SpecReport__fmt_str(char),  # type: ignore[attr-defined]
                    escaped)

    def test_fmt_num_zero(self) -> None:
        """
        Feature: __fmt_num formats zero as an empty string for clean table output
        Expected:
        - input 0 returns ""
        """
        result = SpecReport._SpecReport__fmt_num(0)  # type: ignore[attr-defined]
        self.assertEqual(result, "")

    def test_fmt_num_positive(self) -> None:
        """
        Feature: __fmt_num formats a positive number as its string representation
        Expected:
        - input 42 returns "42"
        """
        result = SpecReport._SpecReport__fmt_num(42)  # type: ignore[attr-defined]
        self.assertEqual(result, "42")

    def test_report_node_format(self) -> None:
        """
        Feature: __report_node formats a SpecNode as a markdown table row
        Expected:
        - output contains escaped "5 Generics" title and "05\\.generics" dir
        - row starts with "| " and ends with "|"
        """
        report = _make_report_shell()
        node = SpecNode("Generics", "5", "", None)
        node.dir = "05.generics"
        node.total_acc = 10
        node.passed_acc = 8
        node.failed_acc = 1
        node.ignored_acc = 1
        node.ignored_but_passed_acc = 0
        result = report._SpecReport__report_node(node)  # type: ignore[attr-defined]
        self.assertIn("5 Generics", result)
        self.assertIn("05\\.generics", result)
        self.assertTrue(result.startswith("| "))
        self.assertTrue(result.endswith("|"))

    def test_report_node_with_excluded(self) -> None:
        """
        Feature: __report_node includes the excluded column when has_excluded is True
        Expected:
        - output contains " 2 |" for the excluded_after_acc value in an extra column
        """
        report = _make_report_shell()
        report.has_excluded = True
        node = SpecNode("Ch1", "1", "", None)
        node.total_acc = 5
        node.excluded_after_acc = 2
        result = report._SpecReport__report_node(node)  # type: ignore[attr-defined]
        self.assertIn(" 2 |", result)
