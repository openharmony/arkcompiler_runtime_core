#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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

import json
from unittest import TestCase

from runner.extensions.validators.srcdumper.ast_comparator import AstComparator


class TestCompareAsts(TestCase):
    """Tests for AstComparator.compare_asts() and AstComparator.run()."""

    @staticmethod
    def _make_comparator(ast1: dict, ast2: dict) -> AstComparator:
        """Build an AstComparator from two in-memory dicts (bypassing file I/O)."""
        comparator = object.__new__(AstComparator)
        comparator.original_ast = ast1
        comparator.dumped_ast = ast2
        comparator.output = ""
        return comparator

    def test_compare_asts_identical(self) -> None:
        """Two identical ASTs → run() returns True."""
        ast = {"type": "Program", "statements": [{"type": "ReturnStatement", "value": 42}]}
        comparator = self._make_comparator(ast, json.loads(json.dumps(ast)))
        self.assertTrue(comparator.run())
        self.assertEqual(comparator.output, "")

    def test_compare_asts_different_values(self) -> None:
        """Scalar value mismatch → False, error message in output."""
        ast1 = {"type": "Program", "value": 1}
        ast2 = {"type": "Program", "value": 2}
        comparator = self._make_comparator(ast1, ast2)
        self.assertFalse(comparator.run())
        self.assertIn("AST comparison failed", comparator.output)

    def test_compare_asts_different_keys(self) -> None:
        """Dict key mismatch → False, error message in output."""
        ast1 = {"type": "Program", "foo": 1}
        ast2 = {"type": "Program", "bar": 1}
        comparator = self._make_comparator(ast1, ast2)
        self.assertFalse(comparator.run())
        self.assertIn("Keys don't match", comparator.output)

    def test_compare_asts_different_list_lengths(self) -> None:
        """List length mismatch → False, error message in output."""
        ast1 = {"statements": [{"type": "A"}, {"type": "B"}]}
        ast2 = {"statements": [{"type": "A"}]}
        comparator = self._make_comparator(ast1, ast2)
        self.assertFalse(comparator.run())
        self.assertIn("length of the arrays does not match", comparator.output)

    def test_compare_asts_type_mismatch(self) -> None:
        """Dict vs list type mismatch → False."""
        ast1: dict = {"statements": []}
        ast2: dict = {"statements": {}}
        comparator = self._make_comparator(ast1, ast2)
        self.assertFalse(comparator.run())

    def test_compare_asts_empty_dicts(self) -> None:
        """Two empty dicts are equal."""
        comparator = self._make_comparator({}, {})
        self.assertTrue(comparator.run())

    def test_compare_asts_empty_lists(self) -> None:
        """Two empty lists are equal."""
        comparator = object.__new__(AstComparator)
        comparator.original_ast = {}
        comparator.dumped_ast = {}
        comparator.output = ""
        self.assertTrue(comparator.compare_asts({}, {}))
