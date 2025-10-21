#!/usr/bin/env python3
# coding=utf-8
#
# Copyright (c) 2025 Huawei Device Co., Ltd.
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

import argparse
import concurrent.futures
import json
import os
import re
import shlex
import shutil
import subprocess
import sys
import threading
import time
from collections import defaultdict
from pathlib import Path
from typing import Any, Dict, List, Optional

workdir = os.getcwd()
mode = "host"


class Constants:
    """Constant definitions"""

    # Field name constants
    START_LINE = "start_line"
    BRANCH_INDEX = "branch_index"
    CASE_INDEX = "case_index"
    COUNT = "count"
    BODY = "body"
    ALTERNATE = "alternate"
    STATEMENTS = "statements"
    CONSEQUENT = "consequent"
    FUNCTION_NAME = "function_name"
    LINE_TABLE = "line_table"
    NAME = "name"
    LOC = "loc"
    START = "start"
    LINE = "line"
    TYPE = "type"
    END = "end"
    BLOCK = "block"
    FINALIZER = "finalizer"
    HANDLER = "handler"
    KEY = "key"
    CASES = "cases"
    VALUE = "value"
    FUNCTION = "function"

    # File extensions
    EXT_ETS = ".ets"
    EXT_JSON = ".json"
    EXT_PA = ".pa"
    EXT_INFO = ".info"
    EXT_CSV = ".csv"

    # Branch types
    IF_STATEMENT = "IfStatement"
    SWITCH_STATEMENT = "SwitchStatement"
    SWITCH_CASE = "SwitchCase"
    CONDITIONAL_EXPRESSION = "ConditionalExpression"
    FOR_UPDATE_STATEMENT = "ForUpdateStatement"
    FOR_IN_STATEMENT = "ForInStatement"
    FOR_OF_STATEMENT = "ForOfStatement"
    WHILE_STATEMENT = "WhileStatement"
    DO_WHILE_STATEMENT = "DoWhileStatement"
    TRY_STATEMENT = "TryStatement"

    BRANCH_TYPES = {
        IF_STATEMENT,
        SWITCH_STATEMENT,
        SWITCH_CASE,
        CONDITIONAL_EXPRESSION,
        FOR_UPDATE_STATEMENT,
        FOR_IN_STATEMENT,
        FOR_OF_STATEMENT,
        WHILE_STATEMENT,
        DO_WHILE_STATEMENT,
        TRY_STATEMENT,
    }

    # AST node types
    NODE_METHOD_DEFINITION = "MethodDefinition"
    NODE_IDENTIFIER = "Identifier"
    NODE_CATCH_CLAUSE = "CatchClause"
    NODE_BLOCK_STATEMENT = "BlockStatement"

    # Regular expression patterns
    FUNCTION_PATTERN = (
        r"\.function\s+(?:[^\.]+\.)*((?:lambda\$invoke\$\d+|lambda_invoke-\d+|[^\.\s<]+)"
        r"\([^)]*\))\s+<.*?>\s+\{\s*#\s+offset:\s+(0x[0-9a-fA-F]+).*?LINE_NUMBER_TABLE:"
        r"(.*?)\}"
    )

    FUNCTION_PATTERN_HAP = (
        r"\.function\s+((?:[^\.]+\.)*)((?:lambda\$invoke\$\d+|lambda_invoke-\d+|[^\.\s<]+)"
        r"\([^)]*\))\s+<.*?>\s+\{\s*#\s+offset:\s+(0x[0-9a-fA-F]+).*?LINE_NUMBER_TABLE:"
        r"(.*?)\}"
    )

    LINE_TABLE_PATTERN = r"line\s+(\d+):\s+(\d+)"
    FUNCTION_NAME_PATTERN = r"{}\s*(?:<[^>]*>)?\s*\("  # Matching common function and template function

    # Default values
    DEFAULT_ANONYMOUS_FUNC_NAME = "anonymous"
    INIT_FUNC_NAME = "_$init$_"
    LAMBDA_INVOKE_1 = "lambda$invoke$"
    LAMBDA_INVOKE_2 = "lambda_invoke-"
    CTOR = "_ctor_"
    CONSTRUCTOR = "constructor"
    CCTOR = "_cctor_"


class Config:
    """Configuration management class"""

    # Directory configuration
    AST_DIR = "ast"
    PA_DIR = "pa"
    RUNTIME_DIR = "runtime_info"
    OUTPUT_FILE = "result.info"

    # CSV column names
    COLUMN_METHODID = "methodid"
    COLUMN_OFFSET = "offset"
    COLUMN_COUNTER = "counter"

    CSV_COLUMNS = [COLUMN_METHODID, COLUMN_OFFSET, COLUMN_COUNTER]

    @classmethod
    def validate_directories(cls):
        """Validate required directories exist"""
        required_dirs = [cls.AST_DIR, cls.PA_DIR, cls.RUNTIME_DIR]
        for directory in required_dirs:
            if not Path(workdir).joinpath(directory).exists():
                raise FileNotFoundError(f"Directory does not exist: {directory}")

    @classmethod
    def get_runtime_dir(cls) -> str:
        """Get output file path"""
        return Path(workdir).joinpath(cls.RUNTIME_DIR)

    @classmethod
    def get_output_file(cls) -> str:
        """Get output file path"""
        return str(Path(workdir).joinpath(cls.OUTPUT_FILE))


class FileUtils:
    """File operation utility class"""

    @staticmethod
    def read_file(file_path: str, encoding: str = "utf-8") -> str:
        """Safely read file content"""
        if not Path(file_path).exists():
            raise FileNotFoundError(f"File does not exist: {file_path}")

        with open(file_path, "r", encoding=encoding) as f:
            return f.read()

    @staticmethod
    def write_file(file_path: str, content: str, open_mode: str = "w", encoding: str = "utf-8"):
        """Safely write file"""
        Path(file_path).parent.mkdir(parents=True, exist_ok=True)
        with open(file_path, open_mode, encoding=encoding) as f:
            f.write(content)

    @staticmethod
    def ensure_extension(file_path: str, extension: str) -> str:
        """Ensure file has correct extension"""
        if not file_path.endswith(extension):
            return file_path + extension
        return file_path

    @staticmethod
    def find_files(directory: str, pattern: str) -> List[Path]:
        """Find files matching pattern"""
        return list(Path(directory).rglob(pattern))


class CoverageInfo:
    def __init__(self):
        self.source_file_path = ""
        self.runtime_info = None
        self.functions = defaultdict(self._default_func_info)
        self.line_stats = defaultdict(int)
        self.func_stats = defaultdict(int)
        self.function_definition_line = {}
        self.function_name_to_lines = defaultdict(set)
        self.branch_stats = {}
        self.process_file_result = False
        self.source_file_path_trans = ""

    def reset_stats(self):
        """Reset statistics"""
        self.source_file_path = ""
        self.runtime_info = None
        self.functions.clear()
        self.line_stats.clear()
        self.func_stats.clear()
        self.function_definition_line.clear()
        self.function_name_to_lines.clear()
        self.branch_stats.clear()
        self.process_file_result = False

    def export_lcov(self, output_file: str):
        """Export coverage report in LCOV format"""
        if not self.process_file_result:
            raise Exception("Coverage information not parsed")

        output_file = FileUtils.ensure_extension(output_file, Constants.EXT_INFO)
        lcov_content = self._generate_lcov_content()

        FileUtils.write_file(output_file, lcov_content, "a")

    def process_data(self, src_file_path: str, pa_file_path: str,
                     ast_file_path: str, runtime_info: List[Dict]):
        """Set file paths and parse"""
        self.reset_stats()
        self.source_file_path = src_file_path
        if mode == 'hap':
            self._trans_source_path(src_file_path)
            if self.source_file_path_trans is None:
                print("source_file_path_trans is none!")
                return

        # Parse files
        self.process_file_result = (
                self._process_ast_file(ast_file_path) and
                self._process_pa_file(pa_file_path) and
                self._process_runtime_info(runtime_info)
        )

    def _trans_source_path(self, src_file_path):
        match = re.search(r'entry/.+?\.ets$', src_file_path)
        if match:
            file_part = os.path.basename(match.group())
            dir_part = os.path.dirname(match.group()).replace('/', '.')
            self.source_file_path_trans = f"{dir_part}.{file_part.replace('.ets', '')}"
            return self.source_file_path_trans
        else:
            print("src path is not expected")
            return None

    def _default_func_info(self) -> Dict:
        """Default function information template"""
        return {
            Constants.FUNCTION_NAME: "",
            Constants.LINE_TABLE: {},
        }

    def _parse_line_table(self, line_table_text: str) -> Dict[int, int]:
        """Parse line table text"""
        line_pattern = re.compile(Constants.LINE_TABLE_PATTERN)
        return {
            int(offset): int(line_num)
            for line_num, offset in line_pattern.findall(line_table_text)
        }

    def _process_ast_file(self, ast_file_path: str) -> bool:
        """Parse AST file"""
        try:
            content = FileUtils.read_file(ast_file_path)
            data = json.loads(content)
            self._traverse_ast(data)
            return True
        except Exception as e:
            print(f"Error parsing AST file {ast_file_path}: {e}")
            return False

    def _traverse_ast(self, node: Any):
        """Traverse AST nodes"""
        if isinstance(node, dict):
            self._process_ast_node(node)

            # Recursively traverse child nodes
            for key, value in node.items():
                if key != "parent":  # Avoid recursion trap
                    self._traverse_ast(value)

        elif isinstance(node, list):
            for item in node:
                self._traverse_ast(item)

    def _process_ast_node(self, node: Dict):
        """Process single AST node"""
        node_type = node.get(Constants.TYPE)

        # Process function declarations
        if node_type == Constants.NODE_METHOD_DEFINITION:
            self._process_function_node(node)

        # Process branch nodes
        elif node_type in Constants.BRANCH_TYPES:
            self._process_branch_node(node)

    def _process_function_node(self, node: Dict):
        """Process function node"""
        # Check if it is in the interface declaration, and if so, skip it
        if self._is_in_interface_declaration(node):
            return

        node_key = node.get(Constants.KEY)
        if node_key and Constants.TYPE in node_key:
            if node_key[Constants.TYPE] != Constants.NODE_IDENTIFIER:
                return
            func_name = node_key.get(Constants.NAME)
            define_line = node.get(Constants.LOC, {}).get(Constants.START, {}).get(Constants.LINE)
            if func_name and define_line and self._validate_function_name_in_source(func_name, define_line):
                self.function_definition_line[define_line] = func_name
                self.function_name_to_lines[func_name].add(define_line)

    def _is_in_interface_declaration(self, node: Dict) -> bool:
        """check if the node is in interface declaration"""
        value = node.get(Constants.VALUE)
        if value and isinstance(value, dict):
            function = value.get(Constants.FUNCTION)
            if function and isinstance(function, dict):
                body = function.get(Constants.BODY)
                if body is None:
                    return True

                if isinstance(body, dict):
                    statements = body.get(Constants.STATEMENTS)
                    if statements is None or (isinstance(statements, list) and len(statements) == 0):
                        return True

        return False

    def _extract_function_name(self, node: Dict) -> str:
        """Extract function name"""
        id_node = node.get("id")
        if id_node and Constants.NAME in id_node:
            return id_node[Constants.NAME]
        return Constants.DEFAULT_ANONYMOUS_FUNC_NAME

    def _validate_function_name_in_source(self, func_name: str, start_line: int) -> bool:
        """Validate function name exists in source file"""
        try:
            pattern = Constants.FUNCTION_NAME_PATTERN.format(re.escape(func_name))
            content = FileUtils.read_file(self.source_file_path)
            lines = content.splitlines()
            if 0 < start_line <= len(lines):
                target_line = lines[start_line - 1]
                return re.search(pattern, target_line) is not None

        except Exception as e:
            print(f"Error validating function name: {e}")

        return False

    def _process_branch_node(self, node: Dict):
        """Process branch node"""
        node_type = node.get(Constants.TYPE)

        try:
            if node_type == Constants.SWITCH_STATEMENT:
                self._process_switch_statement(node)
            elif node_type == Constants.IF_STATEMENT:
                self._process_if_statement(node)
            elif node_type in [
                Constants.FOR_UPDATE_STATEMENT,
                Constants.FOR_IN_STATEMENT,
                Constants.FOR_OF_STATEMENT,
                Constants.WHILE_STATEMENT,
                Constants.DO_WHILE_STATEMENT
            ]:
                self._process_loop_statement(node)
            elif node_type == Constants.TRY_STATEMENT:
                self._process_try_statement(node)

        except Exception as e:
            print(f"Error processing branch node: {e}")

    def _process_switch_statement(self, node: Dict):
        """Process switch statement"""
        # Validate node parameter
        if not isinstance(node, dict):
            print("Warning: SwitchStatement node is not a dictionary")
            return

        switch_cases = node.get(Constants.CASES, [])
        if not isinstance(switch_cases, list):
            print("Warning: SwitchStatement cases is not a list")
            return

        # Validate required keys in node
        if Constants.LOC not in node or not isinstance(node[Constants.LOC], dict):
            print("Warning: SwitchStatement node missing loc information")
            return

        loc = node[Constants.LOC]
        if Constants.START not in loc or not isinstance(loc[Constants.START], dict):
            print("Warning: SwitchStatement node missing start information")
            return

        start = loc[Constants.START]
        if Constants.LINE not in start:
            print("Warning: SwitchStatement node missing line information")
            return

        start_line = start[Constants.LINE]
        for case_index, case in enumerate(switch_cases):
            # Validate case structure
            if not isinstance(case, dict):
                print(f"Warning: Case {case_index} is not a dictionary")
                continue

            # Check if case is SwitchCase type and has consequent statements
            if (
                    case.get(Constants.TYPE) == Constants.SWITCH_CASE
                    and Constants.CONSEQUENT in case
                    and isinstance(case[Constants.CONSEQUENT], list)
                    and len(case[Constants.CONSEQUENT]) > 0
            ):

                # Get the first consequent node
                consequent_list = case[Constants.CONSEQUENT]
                first_consequent = consequent_list[0]

                # Validate first consequent
                if not isinstance(first_consequent, dict):
                    print(
                        f"Warning: First consequent in case {case_index} is not a dictionary"
                    )
                    continue

                enter_node = first_consequent

                # Check if it's a block statement
                if (
                        Constants.TYPE in first_consequent
                        and first_consequent[Constants.TYPE] == Constants.NODE_BLOCK_STATEMENT
                ):
                    # For block statements, get the first statement in the block
                    statements = first_consequent.get(Constants.STATEMENTS)
                    if isinstance(statements, list) and len(statements) > 0:
                        enter_node = statements[0]

                # Create branch entry
                self._create_branch_entry(enter_node, start_line, case_index)

    def _process_if_branch(self, node: Dict, start_line: int, branch_index: int):
        """Process if statement branch (consequent or alternate)"""
        # Handle block statements
        if (
                Constants.TYPE in node and
                node[Constants.TYPE] == Constants.NODE_BLOCK_STATEMENT and
                Constants.STATEMENTS in node
        ):
            statements = node[Constants.STATEMENTS]
            if isinstance(statements, list) and len(statements) > 0:
                # Check for nested if statements in block
                first_statement = statements[0]
                if (
                        isinstance(first_statement, dict) and
                        first_statement.get(Constants.TYPE) == Constants.IF_STATEMENT
                ):
                    # For nested if statements, recursively process them
                    self._process_if_statement(first_statement)
                else:
                    self._create_branch_entry(first_statement, start_line, branch_index)
        # Handle direct statements with STATEMENTS key
        elif Constants.STATEMENTS in node:
            statements = node[Constants.STATEMENTS]
            if isinstance(statements, list) and len(statements) > 0:
                # Check for nested if statements
                first_statement = statements[0]
                if (
                        isinstance(first_statement, dict) and
                        first_statement.get(Constants.TYPE) == Constants.IF_STATEMENT
                ):
                    # For nested if statements, recursively process them
                    self._process_if_statement(first_statement)
                else:
                    self._create_branch_entry(first_statement, start_line, branch_index)
        # Handle direct node without STATEMENTS key
        else:
            # Check if it's a nested if statement
            if (
                    isinstance(node, dict) and
                    node.get(Constants.TYPE) == Constants.IF_STATEMENT
            ):
                # For nested if statements, recursively process them
                self._process_if_statement(node)
            else:
                self._create_branch_entry(node, start_line, branch_index)

    def _process_if_statement(self, node: Dict):
        """Process if statement"""
        loc = node.get(Constants.LOC, {})
        start = loc.get(Constants.START, {})
        start_line = start.get(Constants.LINE)

        if not start_line:
            print(f"Warning: IfStatement node missing line information")
            return

        # Process consequent branch
        if (
                Constants.CONSEQUENT in node
                and isinstance(node[Constants.CONSEQUENT], dict)
        ):
            self._process_if_branch(node[Constants.CONSEQUENT], start_line, 0)

        # Process alternate branch (else)
        if (
                Constants.ALTERNATE in node
                and isinstance(node[Constants.ALTERNATE], dict)
        ):
            self._process_if_branch(node[Constants.ALTERNATE], start_line, 1)

    def _process_loop_statement(self, node: Dict):
        """Process loop statement"""
        loc = node.get(Constants.LOC, {})
        start_line = loc.get(Constants.START, {}).get(Constants.LINE)

        if not start_line:
            print(f"Warning: Loop statement node missing line information")
            return

        body = node.get(Constants.BODY)
        first_statement = None

        if isinstance(body, dict):
            # Prioritize attempting to retrieve from the statements list
            statements = None
            # Simplified condition check - if STATEMENTS exists, use it regardless of type
            if Constants.STATEMENTS in body:
                statements = body[Constants.STATEMENTS]

            if isinstance(statements, list) and statements:
                first_statement = statements[0]
            elif Constants.LOC in body:
                first_statement = body

        if first_statement:
            self._create_branch_entry(first_statement, start_line, 0)
        else:
            print(f"Warning: Loop statement has unexpected body structure")

    def _process_try_statement(self, node: Dict):
        """Process try statement"""
        loc = node.get(Constants.LOC, {})
        if not isinstance(loc, dict):
            print(f"Warning: TryStatement node has invalid loc information")
            return

        start = loc.get(Constants.START, {})
        if not isinstance(start, dict):
            print(f"Warning: TryStatement node has invalid start information")
            return

        start_line = start.get(Constants.LINE)
        if not start_line:
            print(f"Warning: TryStatement node missing line information")
            return

        case_index = 0

        # try block
        if (
                Constants.BLOCK in node
                and isinstance(node[Constants.BLOCK], dict)
                and Constants.STATEMENTS in node[Constants.BLOCK]
        ):
            block_statements = node[Constants.BLOCK][Constants.STATEMENTS]
            if isinstance(block_statements, list) and len(block_statements) > 0:
                self._create_branch_entry(block_statements[0], start_line, case_index)
            case_index += 1

        # catch block
        handlers = node.get(Constants.HANDLER, [])
        if isinstance(handlers, list):
            for handler in handlers:
                if (
                        isinstance(handler, dict)
                        and handler.get(Constants.TYPE) == Constants.NODE_CATCH_CLAUSE
                ):
                    body = handler.get(Constants.BODY, {})
                    if isinstance(body, dict) and Constants.STATEMENTS in body:
                        body_statements = body[Constants.STATEMENTS]
                        if (
                                isinstance(body_statements, list)
                                and len(body_statements) > 0
                        ):
                            self._create_branch_entry(
                                body_statements[0], start_line, case_index
                            )
                        case_index += 1

        # finally block
        if (
                Constants.FINALIZER in node
                and isinstance(node[Constants.FINALIZER], dict)
                and Constants.STATEMENTS in node[Constants.FINALIZER]
        ):
            finalizer_statements = node[Constants.FINALIZER][Constants.STATEMENTS]
            if isinstance(finalizer_statements, list) and len(finalizer_statements) > 0:
                self._create_branch_entry(
                    finalizer_statements[0], start_line, case_index
                )

    def _create_branch_entry(self, node: Dict, start_line: int,
                             case_index: int, default_count: int = 0):
        """Create branch entry"""
        if node and Constants.LOC in node:
            line_num = node[Constants.LOC][Constants.START].get(Constants.LINE)
            if line_num and line_num not in self.branch_stats:
                self.branch_stats[line_num] = {
                    Constants.START_LINE: start_line,
                    Constants.BRANCH_INDEX: 0,
                    Constants.CASE_INDEX: case_index,
                    Constants.COUNT: default_count,
                }

    def _process_runtime_info(self, runtime_info: List[Dict]) -> bool:
        """Process runtime information"""
        if runtime_info is None or not runtime_info:
            print(f"Warning: No runtime information available for {self.source_file_path}")
            return False

        try:
            for row in runtime_info:
                self._process_runtime_row(row)
            return True
        except Exception as e:
            print(f"Error processing runtime information: {e}")
            return False

    def _process_runtime_row(self, row: Dict):
        """Process single row of runtime data"""
        func_id = row[Config.COLUMN_METHODID]
        bc_offset = row[Config.COLUMN_OFFSET]
        line_count = row[Config.COLUMN_COUNTER]

        func_info = self.functions.get(func_id)
        if not func_info:
            return
        # Record line number and branch execution count
        line_num = self._find_line_number(func_id, bc_offset)
        if line_num is not None:
            self.line_stats[line_num] += line_count
            if line_num in self.branch_stats:
                self.branch_stats[line_num][Constants.COUNT] += line_count

        func_name = func_info[Constants.FUNCTION_NAME]
        # Record function execution count
        if bc_offset == 0 and func_name in self.function_name_to_lines:
            # Find function definition line
            smaller_lines = [
                line
                for line in self.function_name_to_lines[func_name]
                if line <= line_num
            ]
            if smaller_lines:
                definition_line = max(smaller_lines)
                self.func_stats[f"{func_name}_{definition_line}"] += line_count

    def _find_line_number(self, func_id: int, bc_offset: int) -> Optional[int]:
        """Find line number corresponding to bytecode offset"""
        func_info = self.functions.get(func_id)
        if not func_info:
            return None
        return func_info[Constants.LINE_TABLE].get(bc_offset)

    def _generate_lcov_content(self) -> str:
        """Generate LCOV content"""
        lines = []

        # File header
        lines.extend([
            "TN:",
            f"SF:{self.source_file_path}"
        ])

        # Function information
        lines.extend(self._generate_function_lines())

        # Line coverage
        lines.extend(self._generate_line_coverage_lines())

        # Branch coverage
        lines.extend(self._generate_branch_coverage_lines())

        # File footer
        lines.append("end_of_record")

        return "\n".join(lines) + "\n"

    def _generate_function_lines(self) -> List[str]:
        """Generate function-related LCOV lines"""
        lines = []

        # Function definitions
        for start_line, name in sorted(self.function_definition_line.items()):
            lines.append(f"FN:{start_line},{name}_{start_line}")

        # Function execution counts
        for func_name, count in self.func_stats.items():
            lines.append(f"FNDA:{count},{func_name}")

        return lines

    def _generate_line_coverage_lines(self) -> List[str]:
        """Generate line coverage-related LCOV lines"""
        # Collect all line numbers
        line_coverage = {
            line_num: 0
            for func_info in self.functions.values()
            for line_num in func_info[Constants.LINE_TABLE].values()
        }

        # Update counts for executed line numbers
        line_coverage.update(self.line_stats)

        return [
            f"DA:{line_num},{count}"
            for line_num, count in sorted(line_coverage.items())
        ]

    def _generate_branch_coverage_lines(self) -> List[str]:
        """Generate branch coverage-related LCOV lines"""
        lines = []

        for branch in self.branch_stats.values():
            lines.append(
                f"BRDA:{branch[Constants.START_LINE]},"
                f"{branch[Constants.BRANCH_INDEX]},"
                f"{branch[Constants.CASE_INDEX]},"
                f"{branch[Constants.COUNT]}"
            )

        return lines


class HostCoverageInfo(CoverageInfo):
    """Host coverage information class"""

    def __init__(self):
        super().__init__()

    def _process_pa_file(self, pa_file_path: str) -> bool:
        """Parse assembly file and build function information dictionary"""
        try:
            content = FileUtils.read_file(pa_file_path)

            function_pattern = re.compile(
                Constants.FUNCTION_PATTERN,
                re.DOTALL
            )

            for match in function_pattern.finditer(content):
                full_signature = match.group(1).strip()
                func_name = full_signature.split("(")[0]
                bc_offset = int(match.group(2), 16)
                line_table_text = match.group(3)
                if func_name == Constants.CTOR:
                    func_name = Constants.CONSTRUCTOR
                # Function is defined in source file
                if (
                        func_name in self.function_name_to_lines
                        or func_name == Constants.INIT_FUNC_NAME
                        or func_name.startswith(Constants.LAMBDA_INVOKE_1)
                        or Constants.LAMBDA_INVOKE_2 in func_name
                ):
                    # Parse line table
                    line_table = self._parse_line_table(line_table_text)
                    self.functions[bc_offset] = {
                        Constants.FUNCTION_NAME: func_name,
                        Constants.LINE_TABLE: line_table,
                    }

            return True

        except Exception as e:
            print(f"Error parsing PA file {pa_file_path}: {e}")
            return False


class HapCoverageInfo(CoverageInfo):
    """Hap coverage information class"""

    def __init__(self):
        super().__init__()
        self._pa_file_content = None
        self._function_pattern = None

    def _is_valid_path_prefix(self, prefix: str, full_path: str) -> bool:
        """Check if the full path starts with the given prefix"""
        path_part = full_path.split(' ', 1)[-1]

        prefix_parts = prefix.split('.')
        path_parts = path_part.split('.')

        if len(path_parts) < len(prefix_parts):
            return False

        for i, prefix_part in enumerate(prefix_parts):
            if prefix_part != path_parts[i]:
                return False

        if len(path_parts) == len(prefix_parts):
            return True

        last_prefix_part = prefix_parts[-1]
        next_part_after_prefix = path_parts[len(prefix_parts)]

        if next_part_after_prefix.startswith(last_prefix_part) and len(next_part_after_prefix) > len(last_prefix_part):
            return False

        return True

    def _get_pa_file_content(self, pa_file_path: str) -> str:
        if self._pa_file_content is None:
            self._pa_file_content = FileUtils.read_file(pa_file_path)
        return self._pa_file_content

    def _get_function_pattern(self):
        if self._function_pattern is None:
            self._function_pattern = re.compile(
                Constants.FUNCTION_PATTERN_HAP,
                re.DOTALL
            )
        return self._function_pattern

    def _process_pa_file(self, pa_file_path: str) -> bool:
        """Parse assembly file and build function information dictionary"""
        try:
            # Read the contents of the pa file using a caching mechanism
            content = self._get_pa_file_content(pa_file_path)

            function_pattern = self._get_function_pattern()

            for match in function_pattern.finditer(content):
                path_part = match.group(1)
                if not self._is_valid_path_prefix(self.source_file_path_trans, path_part):
                    continue
                full_signature = match.group(2).strip()
                func_name = full_signature.split("(")[0]
                bc_offset = int(match.group(3), 16)
                line_table_text = match.group(4)
                if func_name == Constants.CTOR:
                    func_name = Constants.CONSTRUCTOR

                # Function is defined in source file
                if (
                        func_name in self.function_name_to_lines
                        or func_name == Constants.CCTOR
                        or func_name.startswith(Constants.LAMBDA_INVOKE_1)
                        or Constants.LAMBDA_INVOKE_2 in func_name
                ):
                    # Parse line table
                    line_table = self._parse_line_table(line_table_text)
                    self.functions[bc_offset] = {
                        Constants.FUNCTION_NAME: func_name,
                        Constants.LINE_TABLE: line_table,
                    }

            return True

        except Exception as e:
            print(f"Error parsing PA file {pa_file_path}: {e}")
            return False


# ==================== Utility Functions ====================
def create_file_mapping(src_dir: str, ast_dir: str, pa_dir: str) -> List[Dict]:
    """Create file mapping list"""
    src_path = Path(src_dir)
    data = []

    for ets_file in src_path.rglob(f"*{Constants.EXT_ETS}"):
        relative_path = ets_file.relative_to(src_path)

        if mode == "host" or (mode == "hap" and "entry/src/main/ets" in str(relative_path).replace("\\", "/")):
            data.append({
                "src": str(src_path / relative_path),
                "ast": str(Path(ast_dir) / relative_path.with_suffix(Constants.EXT_JSON)),
                "pa": str(Path(pa_dir) / relative_path.with_suffix(Constants.EXT_PA)),
            })

    return data


def summarize_runtime_data(runtime_dir: str,
                           delimiter: str = ',',
                           skip_errors: bool = True,
                           max_errors: int = 10) -> Optional[List[Dict]]:
    """Summarize runtime data"""
    try:
        csv_files = FileUtils.find_files(runtime_dir, f"*{Constants.EXT_CSV}")
        if not csv_files:
            print(f"No CSV files {runtime_dir} found")
            return None

        # Use a dictionary to group by (methodid, offset) and sum counters
        grouped_data = defaultdict(int)
        total_lines = 0
        error_count = 0
        skipped_lines = []

        for csv_file in csv_files:
            print(f"Processing file: {csv_file.name}")

            with open(csv_file, 'r', encoding='utf-8') as f:
                for line_num, line in enumerate(f, 1):
                    try:
                        line = line.strip()

                        # Skip empty lines and comment lines
                        if not line or line.startswith('#'):
                            continue

                        # Split line data
                        parts = line.split(delimiter)
                        if len(parts) != 3:
                            raise ValueError(f"Incorrect number of columns, expected 3, got {len(parts)}")

                        # Convert data types
                        methodid = int(parts[0].strip())
                        offset = int(parts[1].strip())
                        counter = int(parts[2].strip())

                        # Validate data range
                        if methodid < 0 or offset < 0 or counter < 0:
                            raise ValueError("Values cannot be negative")

                        # Group and sum
                        grouped_data[(methodid, offset)] += counter
                        total_lines += 1

                    except Exception as e:
                        error_count += 1
                        skipped_lines.append((line_num, line, str(e)))

                        if not skip_errors:
                            raise

                        if error_count <= max_errors:
                            print(f"Warning: Error at line {line_num} - {e}")

        # Error statistics
        if error_count > 0:
            print(f"Warning: Total {error_count} error lines skipped")
            if error_count > max_errors:
                print(f"(Only showing first {max_errors} errors)")

        if not grouped_data:
            print("Error: No valid runtime data")
            return None

        # Convert to list of dictionaries
        result = [
            {
                Config.COLUMN_METHODID: methodid,
                Config.COLUMN_OFFSET: offset,
                Config.COLUMN_COUNTER: counter
            }
            for (methodid, offset), counter in grouped_data.items()
        ]

        print(f"Successfully processed: {total_lines} lines of runtime data")
        return result

    except FileNotFoundError:
        print(f"File not found: {runtime_dir}")
        return None
    except PermissionError:
        print(f"Permission error: Cannot read file")
        return None
    except Exception as e:
        print(f"Error summarizing runtime data: {e}")
        return None


def find_first_pa_file(pa_dir):
    """Returns the path of the first .pa file found, or None if no file is found."""
    for dirpath, _, filenames in os.walk(pa_dir):
        for filename in filenames:
            if filename.lower().endswith('.pa'):
                return os.path.join(dirpath, filename)
    return None


def export_lcov(args):
    """Export coverage information in LCOV format"""
    try:
        # Validate directories
        Config.validate_directories()

        # Create file mapping
        file_mappings = create_file_mapping(
            args.src,
            str(Path(workdir) / Config.AST_DIR),
            str(Path(workdir) / Config.PA_DIR)
        )
        print(f"Found {len(file_mappings)} file mappings")

        # get pa file
        first_pa_file = None
        if mode == 'hap':
            first_pa_file = find_first_pa_file(Config.PA_DIR)
            if first_pa_file is None:
                print("No .pa file found in PA_DIR")
                return
            print(f"First .pa file found: {first_pa_file}")

        # Summarize runtime data
        runtime_df = summarize_runtime_data(Config.get_runtime_dir())
        if runtime_df is None:
            print("No runtime data found")
            return

        # Delete old result file
        if Path(Config.get_output_file()).exists():
            Path(Config.get_output_file()).unlink()

        # Process each file
        success_count = 0
        if mode == 'host':
            coverage_info = HostCoverageInfo()
        else:
            coverage_info = HapCoverageInfo()

        for row in file_mappings:
            try:
                pa_file_path = first_pa_file if mode == 'hap' else row["pa"]
                coverage_info.process_data(
                    row["src"],
                    pa_file_path,
                    row["ast"],
                    runtime_df
                )
                if coverage_info.process_file_result:
                    coverage_info.export_lcov(Config.get_output_file())
                    success_count += 1

            except FileNotFoundError as e:
                print(f"File not found: {e}")
            except Exception as e:
                print(f"Error processing file {row['src']}: {e}")

        print(f"Successfully processed {success_count}/{len(file_mappings)} files")

    except Exception as e:
        print(f"Program execution error: {e}")


def generate_change_info(diff_file_path, output_info_path):
    """
    Read change information from diff.txt file and generate info file containing SF, DA and end_of_record

    Parameters:
        diff_file_path: diff.txt file path
        output_info_path: output info file path
    """
    # Dictionary to store change information, format: {file_path: {line_number_set}}
    change_info = {}

    # Regular expressions
    file_pattern = re.compile(r'^\+\+\+ b/(.*)$')  # Match file path
    hunk_pattern = re.compile(r'^@@ -(\d+),(\d+) \+(\d+),(\d+) @@')  # Match code block
    add_line_pattern = re.compile(r'^\+(?!\+\+).*$')  # Match added lines

    current_file = None
    current_hunk_start = None
    added_lines_in_hunk = 0

    # Step 1: Parse diff.txt to get change information
    try:
        with open(diff_file_path, 'r', encoding='utf-8') as f:
            for line in f:
                # Match file path
                file_match = file_pattern.match(line)
                if file_match:
                    current_file = file_match.group(1)
                    # Initialize line number set for this file
                    change_info[current_file] = set()
                    current_hunk_start = None
                    added_lines_in_hunk = 0
                    continue

                # Match code block, get starting line number in new file
                hunk_match = hunk_pattern.match(line)
                if hunk_match and current_file:
                    current_hunk_start = int(hunk_match.group(3))  # Starting line number in new file
                    end_hunk = int(hunk_match.group(4))
                    added_lines_in_hunk = 0
                    continue

                # Match added lines and calculate actual line number
                if (current_file and current_hunk_start is not None and
                        added_lines_in_hunk < end_hunk and
                        add_line_pattern.match(line) and line.strip() != '+'):
                    # Calculate actual line number in new file
                    actual_line_num = current_hunk_start + added_lines_in_hunk
                    change_info[current_file].add(actual_line_num)
                    added_lines_in_hunk += 1
                else:
                    added_lines_in_hunk += 1

        if not change_info:
            print("Warning: No change information extracted from diff file")
            return

    except FileNotFoundError:
        print(f"Error: Cannot find diff file '{diff_file_path}'")
        return
    except Exception as e:
        print(f"Error parsing diff file: {str(e)}")
        return

    # Step 2: Generate info file
    try:
        with open(output_info_path, 'w', encoding='utf-8') as f_out:
            # Iterate through each changed file
            for file_path, line_numbers in change_info.items():
                # Write file path (SF)
                f_out.write(f"SF:{file_path}\n")

                # Write changed line information (DA), sorted by line number
                for line_num in sorted(line_numbers):
                    f_out.write(f"DA:{line_num},0\n")

                # Write end marker
                f_out.write("end_of_record\n")

        print(f"Successfully generated change information file: {os.path.abspath(output_info_path)}")

    except Exception as e:
        print(f"Error generating info file: {str(e)}")
        return


def parse_info_file(file_path):
    """
    Parse info file and return structured data containing DA, FN, FNDA and BRDA information
    """
    info_data = {}
    current_file = None
    file_data = {
        'DA': {},  # Line coverage {line_number: coverage}
        'FN': {},  # Functions {line_number: function_name}
        'FNDA': {},  # Function coverage {function_name: coverage}
        'BRDA': []  # Branch coverage [line_number, block_number, branch_number, coverage]
    }

    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue

                # Match file path line
                if line.startswith('SF:'):
                    # If there's a file being processed, save it first
                    if current_file:
                        info_data[current_file] = file_data

                    # Extract file path and normalize
                    file_path = line[3:]
                    normalized_path = os.path.normpath(file_path)
                    current_file = normalized_path
                    file_data = {
                        'DA': {},
                        'FN': {},
                        'FNDA': {},
                        'BRDA': []
                    }

                # Match line coverage data line
                elif line.startswith('DA:') and current_file:
                    parts = line[3:].split(',')
                    if len(parts) >= 2:
                        try:
                            line_num = int(parts[0].strip())
                            coverage = parts[1].strip()
                            file_data['DA'][line_num] = coverage
                        except ValueError:
                            continue

                # Match function line
                elif line.startswith('FN:') and current_file:
                    parts = line[3:].split(',')
                    if len(parts) >= 2:
                        try:
                            line_num = int(parts[0].strip())
                            func_name = parts[1].strip()
                            file_data['FN'][line_num] = func_name
                        except ValueError:
                            continue

                # Match function coverage line
                elif line.startswith('FNDA:') and current_file:
                    parts = line[5:].split(',')
                    if len(parts) >= 2:
                        try:
                            coverage = parts[0].strip()
                            func_name = parts[1].strip()
                            file_data['FNDA'][func_name] = coverage
                        except ValueError:
                            continue

                # Match branch coverage line
                elif line.startswith('BRDA:') and current_file:
                    parts = line[5:].split(',')
                    if len(parts) >= 4:
                        try:
                            line_num = int(parts[0].strip())
                            block_num = int(parts[1].strip())
                            branch_num = int(parts[2].strip())
                            coverage = parts[3].strip()
                            file_data['BRDA'].append([line_num, block_num, branch_num, coverage])
                        except ValueError:
                            continue

                # Match record end line
                elif line == 'end_of_record' and current_file:
                    info_data[current_file] = file_data
                    current_file = None
                    file_data = {
                        'DA': {},
                        'FN': {},
                        'FNDA': {},
                        'BRDA': []
                    }

        # Process last file without end_of_record
        if current_file:
            info_data[current_file] = file_data

    except FileNotFoundError:
        print(f"Error: Cannot find file '{file_path}'")
        return None
    except Exception as e:
        print(f"Error parsing file '{file_path}': {str(e)}")
        return None

    return info_data


def get_enhanced_coverage_info(change_info_path, full_info_path, output_info_path):
    """
    Extract common line information from two info files, including related FN, FNDA and BRDA information
    """
    # Parse both info files
    change_data = parse_info_file(change_info_path)
    full_data = parse_info_file(full_info_path)

    if not change_data or not full_data:
        print("Cannot parse input files, operation terminated")
        return

    try:
        with open(output_info_path, 'w', encoding='utf-8') as f_out:
            # Iterate through each file in change information
            for file_path, change_details in change_data.items():
                # Get changed line numbers
                change_lines = set(change_details['DA'].keys())

                # Normalize path for comparison, ensure it starts with /
                normalized_path = os.path.normpath(file_path)
                if not normalized_path.startswith('/'):
                    normalized_path = '/' + normalized_path

                # Check if file exists in full data
                found = False
                for full_file_path, full_details in full_data.items():
                    normalized_full_path = os.path.normpath(full_file_path)
                    if not normalized_full_path.startswith('/'):
                        normalized_full_path = '/' + normalized_full_path

                    if normalized_full_path == normalized_path:
                        found = True
                        f_out.write(f"SF:{file_path}\n")

                        # Write related function information (FN)
                        for line_num, func_name in full_details['FN'].items():
                            if line_num in change_lines:
                                f_out.write(f"FN:{line_num},{func_name}\n")

                        # Write related function coverage information (FNDA)
                        # First collect function names related to changed lines
                        related_funcs = set()
                        for line_num, func_name in full_details['FN'].items():
                            if line_num in change_lines:
                                related_funcs.add(func_name)
                        # Write coverage for these functions
                        for func_name, coverage in full_details['FNDA'].items():
                            if func_name in related_funcs:
                                f_out.write(f"FNDA:{coverage},{func_name}\n")

                        # Write related line coverage information (DA)
                        for line_num in sorted(change_lines):
                            if line_num in full_details['DA']:
                                f_out.write(f"DA:{line_num},{full_details['DA'][line_num]}\n")

                        # Write related branch coverage information (BRDA)
                        for branch in full_details['BRDA']:
                            line_num, block_num, branch_num, coverage = branch
                            if line_num in change_lines:
                                f_out.write(f"BRDA:{line_num},{block_num},{branch_num},{coverage}\n")

                        # Write end marker
                        f_out.write("end_of_record\n")
                        break

                if not found:
                    print(f"Warning: File '{file_path}' not found in full information file")

        print(f"Successfully generated enhanced coverage information file: {os.path.abspath(output_info_path)}")

    except Exception as e:
        print(f"Error generating enhanced coverage information file: {str(e)}")
        return


def gen_pa(args):
    print(f"Generating PA from {args.src} and outputting to {args.output}")
    src_path = Path(args.src)
    output_path = Path(args.output)

    # Create output directory if it doesn't exist
    output_path.mkdir(parents=True, exist_ok=True)
    # Find all .abc files in the source directory and its subdirectories
    abc_files = list(src_path.rglob("*.abc"))

    if not abc_files:
        print(f"No .abc files found in {args.src}")
        sys.exit(1)

    print(f"Found {len(abc_files)} .abc files to process")

    # Process each .abc file
    for abc_file in abc_files:
        # Calculate relative path from source directory to maintain directory structure
        relative_path = abc_file.relative_to(src_path)
        # Create corresponding output file path with .pa extension
        pa_file = output_path / relative_path.with_suffix('.pa')
        # Create parent directory for the output file
        pa_file.parent.mkdir(parents=True, exist_ok=True)

        cmd = [
            shlex.quote(args.ark_disasm_path),
            "--verbose",
            shlex.quote(str(abc_file)),
            shlex.quote(str(pa_file))
        ]
        print(f"Execute command: {' '.join(cmd)}")
        try:
            result = subprocess.run(cmd, check=True, capture_output=True, text=True)
        except subprocess.CalledProcessError as e:
            print(f"Error occurred while processing {abc_file}:")
            print(f"Command: {' '.join(e.cmd)}")
            print(f"Return code: {e.returncode}")
            if e.stdout:
                print(f"Stdout: {e.stdout}")
            if e.stderr:
                print(f"Stderr: {e.stderr}")
            sys.exit(1)

    print(f"Successfully processed all {len(abc_files)} .abc files")


class GenAst:
    def __init__(self, init_mode='host'):
        self.mode = init_mode
        self.compile_count = 0
        self.ets_files = 0
        self._lock = threading.Lock()
        self.src_path = None
        self.es2panda_path = None
        self.ast_dir = None
        self.abc_dir = None

    def gen_ast(self, src_path, es2panda_path, output_path):
        start_time = time.time()
        error_occurred = False

        self.src_path = Path(src_path).resolve()
        if not self.src_path.exists():
            print(f"Error: Source directory '{self.src_path}' does not exist")
            return False

        self.es2panda_path = Path(es2panda_path).resolve()
        if not self.es2panda_path.exists():
            print(f"Error: es2panda executable '{self.es2panda_path}' does not exist")
            return False

        output_path = Path(output_path).resolve()
        output_path.mkdir(parents=True, exist_ok=True)
        self.ast_dir = output_path / "ast"
        self.abc_dir = output_path / "abc"
        self.ast_dir.mkdir(parents=True, exist_ok=True)
        self.abc_dir.mkdir(parents=True, exist_ok=True)

        self.ets_files = list(self.src_path.rglob(f"*{Constants.EXT_ETS}"))
        if not self.ets_files:
            print(f"Warning: No .ets files found in '{self.src_path}'")
            return True

        print(f"Found {len(self.ets_files)} .ets files, Mode: {self.mode}")

        # Always use multithreaded processing
        max_workers = min(os.cpu_count() or 4, len(self.ets_files))
        print(f"Using {max_workers} threads for parallel processing")

        with concurrent.futures.ThreadPoolExecutor(max_workers=max_workers) as executor:
            future_to_file = {
                executor.submit(self._process_ets_file, ets_file): ets_file
                for ets_file in self.ets_files
            }

            for future in concurrent.futures.as_completed(future_to_file):
                success, rel_path, error_info = future.result()

                if self._handle_compilation_error(rel_path, error_info):
                    print("Terminating due to compilation error in host mode")
                    for f in future_to_file:
                        if not f.done():
                            f.cancel()
                    error_occurred = True
                    sys.exit(1)

        if not error_occurred:
            elapsed_time = time.time() - start_time
            print(f"Processing complete: {self.compile_count}/{len(self.ets_files)} files succeeded")
            print(f"AST/ABC files saved to: {output_path}")
            print(f"Compilation time: {elapsed_time:.2f} seconds")
        return not error_occurred

    def _validate_ast_file(self, ast_file_path):
        """Validate if AST file is generated correctly and return result with error message."""
        try:
            with open(ast_file_path, 'r', encoding='utf-8') as f:
                data = json.load(f)
            return ast_file_path.exists() and ast_file_path.stat().st_size > 0 and data is not None
        except Exception:
            return False

    def _process_ets_file(self, ets_file):
        """Work function for processing a single ETS file"""
        with self._lock:
            print(f"[{self.compile_count}/{len(self.ets_files)}] compiling file: {ets_file}")
            self.compile_count += 1

        try:
            rel_path = ets_file.relative_to(self.src_path)
            ast_output = self.ast_dir / rel_path.with_suffix(Constants.EXT_JSON)
            abc_output = self.abc_dir / rel_path.with_suffix(".abc")
            ast_output.parent.mkdir(parents=True, exist_ok=True)
            abc_output.parent.mkdir(parents=True, exist_ok=True)

            cmd = [str(self.es2panda_path), str(ets_file), f"--dump-ast:output={ast_output}", "--opt-level=0",
                   "--output", str(abc_output)]
            result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

            if result.returncode != 0 and self.mode == 'host':
                error_msg = (
                        result.stderr.strip()
                        or result.stdout.strip()
                        or f"Command failed with return code {result.returncode}"
                )
                return False, rel_path, (error_msg, result.returncode)

            if self.mode == 'hap' and abc_output.exists():
                try:
                    abc_output.unlink()
                except Exception as e:
                    print(f"Warning: Failed to delete ABC file {abc_output}: {e}")

            if not self._validate_ast_file(ast_output):
                base_msg = f"AST file validation failed: {ast_output}"
                error_details = result.stderr.strip() or result.stdout.strip() or ""
                error_msg = f"{base_msg}\n{error_details}" if error_details else base_msg
                return False, rel_path, (error_msg, None)

            return True, rel_path, None
        except Exception as e:
            return False, rel_path, (str(e), None)

    def _handle_compilation_error(self, rel_path, error_info):
        """Handle compilation error information"""
        if self.mode != 'host' or not error_info:
            return False

        error_msg, return_code = error_info
        print(f"Error processing {rel_path}:\n{error_msg}")
        if return_code is not None:
            print(f"Return code: {return_code}")
        return return_code is not None


def main():
    parser = argparse.ArgumentParser(description='Coverage report generation tool')
    subparsers = parser.add_subparsers(dest='command', help='Available commands')

    # Add gen_report subcommand
    report_parser = subparsers.add_parser('gen_report', help='Generate coverage report')
    report_parser.add_argument('--src', required=True, help='Source code directory path')
    report_parser.add_argument('--workdir', '-w', type=str, default='.', help='Work directory')
    report_parser.add_argument('--mode', '-m', type=str, choices=['host', 'hap'], default='host', help='Mode')
    report_group = report_parser.add_mutually_exclusive_group()
    report_group.add_argument('--all', '-a', action='store_true', help='Full coverage report')
    report_group.add_argument('--diff', '-d', action='store_true', help='Incremental coverage report')

    # Add gen_ast subcommand
    gen_ast_parser = subparsers.add_parser('gen_ast', help='Generate AST and ABC files from .ets source files')
    gen_ast_parser.add_argument('--src', required=True, help='Source code directory path')
    gen_ast_parser.add_argument('--es2panda', required=True, help='Path to es2panda executable')
    gen_ast_parser.add_argument('--output', required=True, help='Output directory path')
    gen_ast_parser.add_argument('--mode', '-m', type=str, choices=['host', 'hap'], default='host', help='Mode')

    # Add gen_pa subcommand
    pa_parser = subparsers.add_parser('gen_pa', help='Generate PA data')
    pa_parser.add_argument('--src', required=True, help='Source directory')
    pa_parser.add_argument('--output', default='.', help='Output directory (default: .)')
    pa_parser.add_argument('--ark_disasm-path', required=True, help='Path to ark_disasm executable')

    args = parser.parse_args()
    if args.command in ['gen_report', 'gen_ast']:
        global mode
        mode = args.mode
        print(f"mode: {mode}")

    if args.command == 'gen_report':
        global workdir
        workdir = args.workdir
        print(f"workdir: {workdir}, mode: {mode}")

        genhtml_path = shutil.which('genhtml')
        if not genhtml_path:
            print("Error: genhtml command not found. Please install lcov package.")
            sys.exit(1)
        if (args.all):
            export_lcov(args)
            result_file = os.path.join(workdir, 'result.info')
            if not os.path.exists(result_file):
                print(f"Error: {workdir}/result.info file does not exist")
                sys.exit(1)

            try:
                # Execute genhtml command to generate full coverage report
                cmd = [genhtml_path, str(result_file), '-o', str(os.path.join(workdir, 'report')), '-s',
                       '--branch-coverage']
                subprocess.run(
                    cmd,
                    check=True,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    text=True
                )
                print("Full coverage report generated in coverage_report directory")
            except subprocess.CalledProcessError as e:
                print(f"Failed to generate full coverage report: {e.stderr}")
                print(f"Command: {' '.join(e.cmd)}")
                print(f"Return code: {e.returncode}")
                print(f"Stdout: {e.stdout}")
                sys.exit(1)

        elif (args.diff):
            export_lcov(args)
            if not os.path.exists('result.info'):
                print("Error: result.info file does not exist")
                sys.exit(1)
            if not os.path.exists('diff.txt'):
                print("Error: diff.txt file does not exist")
                sys.exit(1)
            # Execute genhtml command to generate incremental coverage report
            generate_change_info("diff.txt", "change_info.info")
            # Extract complete coverage information including FN, FNDA and BRDA
            get_enhanced_coverage_info("change_info.info", "result.info", "enhanced_coverage.info")
            print(f"Generating incremental coverage report...")

            try:
                # Execute genhtml command to generate incremental coverage report
                subprocess.run(
                    [
                        genhtml_path,
                        'enhanced_coverage.info',
                        '-o',
                        'incremental_coverage_report',
                        '-s',
                        '--branch-coverage'
                    ],
                    check=True,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    text=True
                )
                print("Incremental coverage report generated in incremental_coverage_report directory")
            except subprocess.CalledProcessError as e:
                print(f"Failed to generate incremental coverage report: {e.stderr}")
                print(f"Command: {' '.join(e.cmd)}")
                print(f"Return code: {e.returncode}")
                print(f"Stdout: {e.stdout}")
                sys.exit(1)
        else:
            print("Please specify a parameter")
    elif args.command == 'gen_ast':
        ast_generator = GenAst(init_mode=mode)
        success = ast_generator.gen_ast(args.src, args.es2panda, args.output)
        if not success:
            sys.exit(1)
    elif args.command == 'gen_pa':
        gen_pa(args)
    else:
        print("Please specify a command")


if __name__ == "__main__":
    main()
