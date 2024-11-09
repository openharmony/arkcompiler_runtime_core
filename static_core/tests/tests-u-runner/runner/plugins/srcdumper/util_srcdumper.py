#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024 Huawei Device Co., Ltd.
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

import os
from typing import Tuple, Optional, Any

from runner.enum_types.params import TestReport
from runner.enum_types.fail_kind import FailKind


class PathConverter:
    def __init__(self, test_id: str, artefacts_root: str):
        self.test_id = test_id
        self.artefacts = os.path.join(artefacts_root, 'dumped_src')

    def init_artefact_dirs(self) -> None:
        os.makedirs(os.path.join(self.artefacts, os.path.dirname(self.test_id)), exist_ok=True)

    def dumped_src_path(self) -> str:
        return f"{os.path.splitext(os.path.join(self.artefacts, self.test_id))[0]}_dumped.sts"


class AstComparator:
    def __init__(self, original_ast: dict, dumped_ast: dict):
        self.original_ast = AstComparator.normalize_ast(original_ast)
        self.dumped_ast = AstComparator.normalize_ast(dumped_ast)
        self.output = ""


    @staticmethod
    def normalize_ast(ast: dict) -> dict:
        ast = AstComparator.remove_loc_nodes(ast)
        ast = AstComparator.remove_empty_statements(ast)
        ast = AstComparator.remove_duplicate_undefined_types(ast)
        ast = AstComparator.flatten_block_statements(ast)
        ast = AstComparator.replace_null_literals(ast)

        return ast


    @staticmethod
    def remove_loc_nodes(ast: Any) -> Any:
        """Removes all 'loc' keys from dictionaries within the AST."""

        if isinstance(ast, list):
            return [AstComparator.remove_loc_nodes(item) for item in ast]

        if isinstance(ast, dict):
            new_ast = {}
            for key, value in ast.items():
                if key != "loc":
                    new_ast[key] = AstComparator.remove_loc_nodes(value)
            return new_ast

        return ast


    @staticmethod
    def remove_empty_statements(ast: Any) -> Any:
        """Removes all EmptyStatement nodes from the AST."""

        if isinstance(ast, list):
            return [AstComparator.remove_empty_statements(item) for item in ast
                    if not (isinstance(item, dict) and item.get("type") == "EmptyStatement")]

        if isinstance(ast, dict):
            new_ast = {}
            for key, value in ast.items():
                new_ast[key] = AstComparator.remove_empty_statements(value)
            return new_ast

        return ast


    @staticmethod
    def flatten_block_statements(ast: Any) -> Any:
        """Flattens BlockStatement nodes where possible."""

        if isinstance(ast, list):
            nodes = []
            for item in ast:
                if isinstance(item, dict) and item.get("type") == "BlockStatement":
                    nodes.extend(AstComparator.flatten_block_statements(item.get("statements", [])))
                else:
                    nodes.append(AstComparator.flatten_block_statements(item))
            return nodes

        if isinstance(ast, dict):
            new_ast = {}
            for key, value in ast.items():
                if not isinstance(value, dict) or value.get("type") != "BlockStatement":
                    new_ast[key] = AstComparator.flatten_block_statements(value)
                elif (statements := value.get("statements")) and len(statements) == 1:
                    new_ast[key] = AstComparator.flatten_block_statements(statements[0])
                else:
                    new_ast[key] = AstComparator.flatten_block_statements(value)
            return new_ast

        return ast


    @staticmethod
    def remove_duplicate_undefined_types(ast: Any) -> Any:
        """Removes duplicate ETSUndefinedType nodes from lists within the AST."""

        if isinstance(ast, list):
            nodes = []
            seen_undefined = False
            for item in ast:
                if not isinstance(item, dict) or item.get("type") != "ETSUndefinedType":
                    nodes.append(AstComparator.remove_duplicate_undefined_types(item))
                elif not seen_undefined:
                    nodes.append(item)
                    seen_undefined = True
            return nodes

        if isinstance(ast, dict):
            new_ast = {}
            for key, value in ast.items():
                new_ast[key] = AstComparator.remove_duplicate_undefined_types(value)
            return new_ast

        return ast


    @staticmethod
    def replace_null_literals(ast: Any) -> Any:
        """Replaces all NullLiteral values with ETSNullType."""

        if isinstance(ast, list):
            return [AstComparator.replace_null_literals(item) for item in ast]

        if isinstance(ast, dict):
            new_ast = {}
            for key, value in ast.items():
                if value in ("ETSNullType", "NullLiteral"):
                    return "ETSNullType or NullLiteral"

                new_ast[key] = AstComparator.replace_null_literals(value)
            return new_ast

        return ast


    def run(self) -> Tuple[bool, TestReport, Optional[FailKind]]:
        passed = self.compare_asts(self.original_ast, self.dumped_ast)
        fail_kind = None if passed else FailKind.SRC_DUMPER_FAIL
        return passed, TestReport(self.output, "", 0), fail_kind

    # pylint: disable=too-many-return-statements
    def compare_asts(self, original_ast: Any, dumped_ast: Any) -> bool:
        """Compares two ASTs recursively after normalization."""

        if not isinstance(original_ast, type(dumped_ast)):
            return False

        if isinstance(dumped_ast, dict):
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
        elif isinstance(dumped_ast, list):
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
        else:
            if original_ast != dumped_ast:
                self.output += (
                    f"AST comparison failed!\n"
                    f"Values don't match!\n"
                    f"Original value: {original_ast}\n"
                    f"Dumped value: {dumped_ast}\n"
                )
                return False

        return True
