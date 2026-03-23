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
import sys
from pathlib import Path
from typing import Any

AstNode = Any  # type: ignore[explicit-any]


class AstComparatorException(Exception):
    pass


class AstComparator:
    def __init__(self, dump1: Path, dump2: Path):
        self.original_ast: dict = AstComparator.load_ast(dump1)
        self.dumped_ast: dict = AstComparator.load_ast(dump2)
        self.output = ""

    @staticmethod
    def load_ast(dump: Path) -> dict:
        """
        Load ast from a dump file.
        1. The first line is always skipped as it's an es2panda phase name where the dump is created
        2. The file can contain error messages at the file end. They should be skipped as well.
        """
        content = dump.read_text(encoding="utf-8").splitlines()[1:]
        last_index = 0
        for i, line in enumerate(content[::-1]):
            if str(line).strip() == "}":
                last_index = i
                break
        if last_index > 0:
            content = content[:-1 * last_index]
        content_str = "\n".join(content)
        try:
            ast = json.loads(content_str)
            return AstComparator.normalize_ast(ast)
        except json.JSONDecodeError as ex:
            raise AstComparatorException(f"File {dump.as_posix()} does not contain correct json object") from ex

    @staticmethod
    def normalize_ast(ast: dict) -> dict:
        AstComparator.remove_loc_nodes(ast)
        AstComparator.remove_empty_statements(ast)
        AstComparator.replace_null_literals(ast)
        AstComparator.flatten_similar_nested_nodes(ast, "BlockStatement", "statements")
        AstComparator.flatten_similar_nested_nodes(ast, "ETSUnionType", "types")
        AstComparator.remove_duplicate_undefined_types(ast)

        return ast

    @staticmethod
    def remove_loc_nodes(ast: AstNode) -> None:
        """Removes all 'loc' keys from dictionaries within the AST."""

        if isinstance(ast, list):
            for item in ast:
                AstComparator.remove_loc_nodes(item)

        elif isinstance(ast, dict):
            ast.pop("loc", None)
            for value in ast.values():
                AstComparator.remove_loc_nodes(value)

    @staticmethod
    def remove_empty_statements(ast: AstNode) -> None:
        """Removes all EmptyStatement nodes from the AST."""

        if isinstance(ast, list):
            for i in range(len(ast) - 1, -1, -1):
                if isinstance(ast[i], dict) and ast[i].get("type") == "EmptyStatement":
                    ast.pop(i)
                else:
                    AstComparator.remove_empty_statements(ast[i])

        elif isinstance(ast, dict):
            for value in ast.values():
                AstComparator.remove_empty_statements(value)

    @staticmethod
    def list_extend_in_pos(nodes: list, extend_nodes: list, pos: int) -> None:
        """Removes the element in 'pos' position and inserts extend_nodes instead."""
        nodes.pop(pos)
        for i, extend_node in enumerate(extend_nodes):
            nodes.insert(pos + i, extend_node)

    @staticmethod
    def flatten_similar_nested_nodes(ast: AstNode, node_type: str, nested_key: str) -> None:
        """Flattens nested nodes with similar type e.g. BlockStatement in BlockStatement."""

        if isinstance(ast, list):
            for i in range(len(ast) - 1, -1, -1):
                if isinstance(ast[i], dict) and ast[i].get("type") == node_type:
                    AstComparator.flatten_similar_nested_nodes(ast[i].get(nested_key), node_type, nested_key)
                    AstComparator.list_extend_in_pos(ast, ast[i].get(nested_key, []), i)
                else:
                    AstComparator.flatten_similar_nested_nodes(ast[i], node_type, nested_key)

        elif isinstance(ast, dict):
            for key, value in ast.items():
                if not isinstance(value, dict) or value.get("type") != node_type:
                    AstComparator.flatten_similar_nested_nodes(value, node_type, nested_key)
                elif (nested_nodes := value.get(nested_key, [])) and len(nested_nodes) == 1:
                    AstComparator.flatten_similar_nested_nodes(nested_nodes[0], node_type, nested_key)
                    ast[key] = nested_nodes[0]
                else:
                    AstComparator.flatten_similar_nested_nodes(value, node_type, nested_key)

    @staticmethod
    def remove_duplicate_undefined_types(ast: AstNode) -> None:
        """Removes duplicate ETSUndefinedType nodes from lists within the AST."""

        if isinstance(ast, list):
            seen_undefined = False
            for i in range(len(ast) - 1, -1, -1):
                if not isinstance(ast[i], dict) or ast[i].get("type") != "ETSUndefinedType":
                    AstComparator.remove_duplicate_undefined_types(ast[i])
                elif not seen_undefined:
                    seen_undefined = True
                else:
                    ast.pop(i)

        elif isinstance(ast, dict):
            for value in ast.values():
                AstComparator.remove_duplicate_undefined_types(value)

    @staticmethod
    def replace_null_literals(ast: AstNode) -> None:
        """Replaces all NullLiteral values with ETSNullType."""

        if isinstance(ast, list):
            for item in ast:
                AstComparator.replace_null_literals(item)

        elif isinstance(ast, dict):
            if ast.get("type") in ("ETSNullType", "NullLiteral"):
                ast.clear()
                ast["type"] = "ETSNullType or NullLiteral"

            for value in ast.values():
                AstComparator.replace_null_literals(value)

    # NOTE(zhelyapovaleksey): need to remove this patch-method (Issue: #22553)
    @staticmethod
    def remove_default_constructor_properties(ast: dict) -> None:
        ast.pop('accessibility', None)
        ast.pop('declare', None)

    def run(self) -> bool:
        return self.compare_asts(self.original_ast, self.dumped_ast)

    def compare_asts(self, original_ast: AstNode, dumped_ast: AstNode) -> bool:
        """Compares two ASTs recursively after normalization."""

        if not isinstance(original_ast, type(dumped_ast)):
            return False

        if isinstance(dumped_ast, dict):
            return self.__compare_dicts(original_ast, dumped_ast)

        if isinstance(dumped_ast, list):
            return self.__compare_lists(original_ast, dumped_ast)

        if original_ast != dumped_ast:
            self.output += (
                f"AST comparison failed!\n"
                f"Values don't match!\n"
                f"Original value: {original_ast}\n"
                f"Dumped value: {dumped_ast}\n"
            )
            return False

        return True

    def __compare_dicts(self, original_ast: dict, dumped_ast: dict) -> bool:
        AstComparator.remove_default_constructor_properties(original_ast)
        AstComparator.remove_default_constructor_properties(dumped_ast)

        if original_ast.keys() != dumped_ast.keys():
            self.output += (
                f"AST comparison failed!\n"
                f"Keys don't match!\n"
                f"Original keys: {original_ast.keys()}\n"
                f"Dumped keys: {dumped_ast.keys()}\n"
            )
            return False
        for key, value in dumped_ast.items():
            if not self.compare_asts(original_ast.get(key), value):
                return False
        return True

    def __compare_lists(self, original_ast: list, dumped_ast: list) -> bool:
        if len(original_ast) != len(dumped_ast):
            self.output += (
                f"AST comparison failed!\n"
                f"The length of the arrays does not match!\n"
                f"Original array len: {len(original_ast)}\n"
                f"Dumped array len: {len(dumped_ast)}\n"
            )
            return False
        for i, dumped_value in enumerate(dumped_ast):
            if not self.compare_asts(original_ast[i], dumped_value):
                return False
        return True


def main() -> int:
    args = sys.argv[1:]
    dump1 = Path(args[0])
    dump2 = Path(args[1])
    comparator = AstComparator(dump1, dump2)
    passed = comparator.run()
    if not passed:
        sys.stderr.write(comparator.output)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
