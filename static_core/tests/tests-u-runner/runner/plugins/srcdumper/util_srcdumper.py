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
        AstComparator.remove_loc_nodes(ast)
        AstComparator.remove_empty_statements(ast)
        AstComparator.replace_null_literals(ast)
        AstComparator.flatten_block_statements(ast)
        AstComparator.remove_duplicate_undefined_types(ast)

        return ast


    @staticmethod
    def remove_loc_nodes(ast: Any) -> None:
        """Removes all 'loc' keys from dictionaries within the AST."""

        if isinstance(ast, list):
            for item in ast:
                AstComparator.remove_loc_nodes(item)

        elif isinstance(ast, dict):
            if "loc" in ast.keys():
                ast.pop("loc")
            for value in ast.values():
                AstComparator.remove_loc_nodes(value)


    @staticmethod
    def remove_empty_statements(ast: Any) -> None:
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
    def flatten_block_statements(ast: Any) -> None:
        """Flattens BlockStatement nodes where possible."""

        if isinstance(ast, list):
            for i in range(len(ast) - 1, -1, -1):
                if isinstance(ast[i], dict) and ast[i].get("type") == "BlockStatement":
                    AstComparator.flatten_block_statements(ast[i].get("statements"))
                    ast.extend(ast[i].get("statements", []))
                    ast.pop(i)
                else:
                    AstComparator.flatten_block_statements(ast[i])

        elif isinstance(ast, dict):
            for key, value in ast.items():
                if not isinstance(value, dict) or value.get("type") != "BlockStatement":
                    AstComparator.flatten_block_statements(value)
                elif (statements := value.get("statements", [])) and len(statements) == 1:
                    AstComparator.flatten_block_statements(statements[0])
                    ast[key] = statements[0]
                else:
                    AstComparator.flatten_block_statements(value)


    @staticmethod
    def remove_duplicate_undefined_types(ast: Any) -> None:
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
    def replace_null_literals(ast: Any) -> None:
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
