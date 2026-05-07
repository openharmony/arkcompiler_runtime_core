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

from unittest import TestCase

from runner.extensions.validators.srcdumper.ast_comparator import AstComparator


class TestNormalizationMethods(TestCase):
    """Tests for individual static normalization methods."""

    # ------------------------------------------------------------------
    # remove_loc_nodes
    # ------------------------------------------------------------------

    def test_remove_loc_nodes_from_dict(self) -> None:
        """All 'loc' keys are removed from a dict."""
        ast: dict = {"type": "Program", "loc": {"start": 0, "end": 10}, "statements": []}
        AstComparator.remove_loc_nodes(ast)
        self.assertNotIn("loc", ast)
        self.assertEqual(ast["type"], "Program")

    def test_remove_loc_nodes_nested(self) -> None:
        """'loc' keys are removed recursively from nested dicts and lists."""
        ast: dict = {
            "type": "Program",
            "loc": {"start": 0},
            "statements": [
                {"type": "ExpressionStatement", "loc": {"start": 1}},
            ],
        }
        AstComparator.remove_loc_nodes(ast)
        self.assertNotIn("loc", ast)
        self.assertNotIn("loc", ast["statements"][0])

    # ------------------------------------------------------------------
    # remove_empty_statements
    # ------------------------------------------------------------------

    def test_remove_empty_statements(self) -> None:
        """EmptyStatement nodes are removed from lists."""
        ast: dict = {
            "type": "Program",
            "statements": [
                {"type": "EmptyStatement"},
                {"type": "ExpressionStatement"},
                {"type": "EmptyStatement"},
            ],
        }
        AstComparator.remove_empty_statements(ast)
        types = [s["type"] for s in ast["statements"]]
        self.assertNotIn("EmptyStatement", types)
        self.assertIn("ExpressionStatement", types)

    def test_remove_empty_statements_nested(self) -> None:
        """EmptyStatement nodes are removed recursively."""
        ast: dict = {
            "type": "Program",
            "body": {
                "type": "BlockStatement",
                "statements": [{"type": "EmptyStatement"}, {"type": "ReturnStatement"}],
            },
        }
        AstComparator.remove_empty_statements(ast)
        types = [s["type"] for s in ast["body"]["statements"]]
        self.assertNotIn("EmptyStatement", types)

    # ------------------------------------------------------------------
    # replace_null_literals
    # ------------------------------------------------------------------

    def test_replace_null_literals_etsnulltype(self) -> None:
        """ETSNullType node is replaced with the unified sentinel value."""
        ast: dict = {"type": "ETSNullType", "extra": "data"}
        AstComparator.replace_null_literals(ast)
        self.assertEqual(ast["type"], "ETSNullType or NullLiteral")
        # extra keys should be gone (dict was cleared then re-set)
        self.assertNotIn("extra", ast)

    def test_replace_null_literals_nullliteral(self) -> None:
        """NullLiteral node is replaced with the unified sentinel value."""
        ast: dict = {"type": "NullLiteral"}
        AstComparator.replace_null_literals(ast)
        self.assertEqual(ast["type"], "ETSNullType or NullLiteral")

    def test_replace_null_literals_nested(self) -> None:
        """replace_null_literals works recursively inside lists and dicts."""
        ast: dict = {
            "type": "Program",
            "statements": [{"type": "NullLiteral"}],
        }
        AstComparator.replace_null_literals(ast)
        self.assertEqual(ast["statements"][0]["type"], "ETSNullType or NullLiteral")

    # ------------------------------------------------------------------
    # flatten_similar_nested_nodes
    # ------------------------------------------------------------------

    def test_flatten_block_statement_nested(self) -> None:
        """Nested BlockStatement inside BlockStatement is flattened."""
        inner_stmt = {"type": "ReturnStatement"}
        inner_block = {"type": "BlockStatement", "statements": [inner_stmt]}
        outer_block = {"type": "BlockStatement", "statements": [inner_block]}
        ast: dict = {"type": "Program", "body": outer_block}

        AstComparator.flatten_similar_nested_nodes(ast, "BlockStatement", "statements")

        # The outer block's statements should now directly contain ReturnStatement
        body = ast["body"]
        self.assertEqual(body["type"], "BlockStatement")
        stmt_types = [s["type"] for s in body["statements"]]
        self.assertIn("ReturnStatement", stmt_types)
        self.assertNotIn("BlockStatement", stmt_types)

    def test_flatten_ets_union_type_nested(self) -> None:
        """Nested ETSUnionType inside ETSUnionType is flattened."""
        type_a = {"type": "ETSStringType"}
        type_b = {"type": "ETSIntType"}
        inner_union = {"type": "ETSUnionType", "types": [type_a]}
        outer_union = {"type": "ETSUnionType", "types": [inner_union, type_b]}
        ast: dict = {"type": "Program", "typeAnnotation": outer_union}

        AstComparator.flatten_similar_nested_nodes(ast, "ETSUnionType", "types")

        union = ast["typeAnnotation"]
        child_types = [t["type"] for t in union["types"]]
        self.assertNotIn("ETSUnionType", child_types)
        self.assertIn("ETSStringType", child_types)
        self.assertIn("ETSIntType", child_types)

    # ------------------------------------------------------------------
    # remove_duplicate_undefined_types
    # ------------------------------------------------------------------

    def test_remove_duplicate_undefined_types(self) -> None:
        """Only one ETSUndefinedType is kept in a list; duplicates are removed."""
        ast: dict = {
            "type": "ETSUnionType",
            "types": [
                {"type": "ETSUndefinedType"},
                {"type": "ETSStringType"},
                {"type": "ETSUndefinedType"},
                {"type": "ETSUndefinedType"},
            ],
        }
        AstComparator.remove_duplicate_undefined_types(ast)
        undefined_count = sum(
            1 for t in ast["types"] if t.get("type") == "ETSUndefinedType"
        )
        self.assertEqual(undefined_count, 1)

    def test_remove_duplicate_undefined_types_single_kept(self) -> None:
        """A single ETSUndefinedType is not removed."""
        ast: dict = {
            "types": [{"type": "ETSUndefinedType"}, {"type": "ETSStringType"}]
        }
        AstComparator.remove_duplicate_undefined_types(ast)
        self.assertEqual(len(ast["types"]), 2)

    # ------------------------------------------------------------------
    # remove_default_constructor_properties
    # ------------------------------------------------------------------

    def test_remove_default_constructor_properties(self) -> None:
        """'accessibility' and 'declare' keys are stripped from a dict."""
        ast: dict = {
            "type": "MethodDefinition",
            "accessibility": "public",
            "declare": False,
            "key": "constructor",
        }
        AstComparator.remove_default_constructor_properties(ast)
        self.assertNotIn("accessibility", ast)
        self.assertNotIn("declare", ast)
        self.assertIn("key", ast)

    def test_remove_default_constructor_properties_missing_keys(self) -> None:
        """Method is safe when 'accessibility'/'declare' are absent."""
        ast: dict = {"type": "MethodDefinition", "key": "constructor"}
        # Should not raise
        AstComparator.remove_default_constructor_properties(ast)
        self.assertIn("key", ast)
