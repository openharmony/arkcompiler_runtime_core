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
import json
import os
import re
import shutil
import subprocess
import sys
from collections import defaultdict
from pathlib import Path
from typing import Any, Dict, List, Optional, Set
import pandas as pd


class Constants:
    """Constant definitions"""

    # Field name constants
    START_LINE = "start_line"
    BRANCH_INDEX = "branch_index"
    CASE_INDEX = "case_index"
    COUNT = "count"
    BODY = "body"
    CONSEQUENT = "consequent"
    FUNCTION_NAME = "function_name"
    LINE_TABLE = "line_table"
    NAME = "name"
    LOC = "loc"
    START = "start"
    LINE = "line"
    TYPE = "type"
    END = "end"

    # File extensions
    EXT_ETS = ".ets"
    EXT_JSON = ".json"
    EXT_PA = ".pa"
    EXT_INFO = ".info"
    EXT_CSV = ".csv"

    # Branch types
    BRANCH_TYPES = [
        "IfStatement",
        "SwitchStatement",
        "SwitchCase",
        "ConditionalExpression",
        "ForUpdateStatement",
        "ForInStatement",
        "ForOfStatement",
        "WhileStatement",
        "DoWhileStatement",
        "TryStatement",
    ]

    # AST node types
    NODE_BLOCK_STATEMENT = "BlockStatement"
    NODE_SCRIPT_FUNCTION = "ScriptFunction"
    NODE_CATCH_CLAUSE = "CatchClause"

    # Regular expression patterns
    FUNCTION_PATTERN = (
        r"\.function\s+(?:[^\.]+\.)*([^\.\s<]+\([^)]*\))\s+<.*?>\s+\{\s*#\s+offset:\s+"
        r"(0x[0-9a-fA-F]+).*?LINE_NUMBER_TABLE:(.*?)\}"
    )
    LINE_TABLE_PATTERN = r"line\s+(\d+):\s+(\d+)"
    FUNCTION_NAME_PATTERN = r"{}\s*\("

    # Default values
    DEFAULT_ANONYMOUS_FUNC_NAME = "anonymous"
    DEFAULT_COUNTER_VALUE = 0


class Config:
    """Configuration management class"""

    # Directory configuration
    SOURCE_DIR = "src"
    AST_DIR = "ast"
    PA_DIR = "pa"
    RUNTIME_DIR = "runtime_info"
    OUTPUT_FILE = "./result.info"

    # CSV column names
    CSV_COLUMNS = ["methodid", "offset", "counter"]

    @classmethod
    def validate_directories(cls):
        """Validate required directories exist"""
        required_dirs = [cls.SOURCE_DIR, cls.AST_DIR, cls.PA_DIR, cls.RUNTIME_DIR]
        for directory in required_dirs:
            if not Path(directory).exists():
                raise FileNotFoundError(f"Directory does not exist: {directory}")


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
    def write_file(file_path: str, content: str, mode: str = "w", encoding: str = "utf-8"):
        """Safely write file"""
        Path(file_path).parent.mkdir(parents=True, exist_ok=True)
        with open(file_path, mode, encoding=encoding) as f:
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

    def set_file_path(self, src_file_path: str, pa_file_path: str,
                      ast_file_path: str, runtime_info: pd.DataFrame):
        """Set file paths and parse"""
        self.reset_stats()
        self.source_file_path = src_file_path

        # Parse files
        self.process_file_result = (
                self._process_ast_file(ast_file_path) and
                self._process_pa_file(pa_file_path) and
                self._process_runtime_info(runtime_info)
        )

    def _default_func_info(self) -> Dict:
        """Default function information template"""
        return {
            Constants.FUNCTION_NAME: "",
            Constants.LINE_TABLE: {},
        }

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
                # Function is defined in source file
                if func_name in self.function_name_to_lines:
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
        if node_type == Constants.NODE_SCRIPT_FUNCTION:
            self._process_function_node(node)

        # Process branch nodes
        elif node_type in Constants.BRANCH_TYPES:
            self._process_branch_node(node)

    def _process_function_node(self, node: Dict):
        """Process function node"""
        func_name = self._extract_function_name(node)
        body = node.get(Constants.BODY)

        if body and Constants.LOC in body:
            start_line = body[Constants.LOC][Constants.START][Constants.LINE]
            if self._validate_function_name_in_source(func_name, start_line):
                self.function_definition_line[start_line] = func_name
                self.function_name_to_lines[func_name].add(start_line)

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
            if node_type == "SwitchStatement":
                self._process_switch_statement(node)
            elif node_type == "IfStatement":
                self._process_if_statement(node)
            elif node_type in ["ForUpdateStatement", "ForInStatement", "ForOfStatement",
                               "WhileStatement", "DoWhileStatement"]:
                self._process_loop_statement(node)
            elif node_type == "TryStatement":
                self._process_try_statement(node)

        except Exception as e:
            print(f"Error processing branch node: {e}")

    def _process_switch_statement(self, node: Dict):
        """Process switch statement"""
        start_line = node[Constants.LOC][Constants.START][Constants.LINE]
        switch_cases = node.get("cases", [])

        for case_index, case in enumerate(switch_cases):
            if case.get(Constants.TYPE) == "SwitchCase":
                self._create_branch_entry(
                    case[Constants.CONSEQUENT][0],
                    start_line,
                    case_index
                )

    def _process_if_statement(self, node: Dict):
        """Process if statement"""
        start_line = node[Constants.LOC][Constants.START][Constants.LINE]

        # consequent branch
        if Constants.CONSEQUENT in node:
            self._create_branch_entry(
                node[Constants.CONSEQUENT],
                start_line,
                0
            )

        # alternate branch (else)
        if "alternate" in node:
            self._create_branch_entry(
                node["alternate"],
                start_line,
                1
            )

    def _process_loop_statement(self, node: Dict):
        """Process loop statement"""
        start_line = node[Constants.LOC][Constants.START][Constants.LINE]
        body = node.get(Constants.BODY)

        if body and "statements" in body and body["statements"]:
            # Inside loop body
            self._create_branch_entry(
                body["statements"][0],
                start_line,
                0
            )

        # Loop end
        if Constants.LOC in node and Constants.END in node[Constants.LOC]:
            end_info = {
                Constants.LOC: {Constants.START: node[Constants.LOC][Constants.END]}
            }
            self._create_branch_entry(end_info, start_line, 1, 1)

    def _process_try_statement(self, node: Dict):
        """Process try statement"""
        start_line = node[Constants.LOC][Constants.START][Constants.LINE]
        case_index = 0

        # try block
        if "block" in node:
            self._create_branch_entry(
                node["block"],
                start_line,
                case_index
            )
            case_index += 1

        # catch block
        for handler in node.get("handler", []):
            if handler.get(Constants.TYPE) == Constants.NODE_CATCH_CLAUSE:
                self._create_branch_entry(
                    handler.get(Constants.BODY, {}),
                    start_line,
                    case_index
                )
                case_index += 1

        # finally block
        if "finalizer" in node:
            self._create_branch_entry(
                node["finalizer"],
                start_line,
                case_index
            )

    def _create_branch_entry(self, node: Dict, start_line: int,
                             case_index: int, default_count: int = 0):
        """Create branch entry"""
        if node and Constants.LOC in node:
            line_num = node[Constants.LOC][Constants.START].get(Constants.LINE)
            if line_num:
                self.branch_stats[line_num] = {
                    Constants.START_LINE: start_line,
                    Constants.BRANCH_INDEX: 0,
                    Constants.CASE_INDEX: case_index,
                    Constants.COUNT: default_count,
                }

    def _process_runtime_info(self, runtime_info: pd.DataFrame) -> bool:
        """Process runtime information"""
        if runtime_info is None or runtime_info.empty:
            print(f"Warning: No runtime information available for {self.source_file_path}")
            return False

        try:
            for _, row in runtime_info.iterrows():
                self._process_runtime_row(row)
            return True
        except Exception as e:
            print(f"Error processing runtime information: {e}")
            return False

    def _process_runtime_row(self, row: pd.Series):
        """Process single row of runtime data"""
        func_id = row["methodid"]
        bc_offset = row["offset"]
        line_count = row["counter"]

        func_info = self.functions.get(func_id)
        if not func_info:
            return
        func_name = func_info[Constants.FUNCTION_NAME]
        # Record line number and branch execution count
        line_num = self._find_line_number(func_id, bc_offset)
        if line_num is not None:
            self.line_stats[line_num] += line_count
            if line_num in self.branch_stats:
                self.branch_stats[line_num][Constants.COUNT] += line_count

        # Record function execution count
        if bc_offset == 0 and func_name in self.function_name_to_lines:
            # Find function definition line
            smaller_lines = [
                line
                for line in self.function_name_to_lines[func_name]
                if line < line_num
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


# ==================== Utility Functions ====================
def create_file_mapping_dataframe(src_dir: str, ast_dir: str, pa_dir: str) -> pd.DataFrame:
    """Create file mapping dataframe"""
    src_path = Path(src_dir)
    data = []

    for ets_file in src_path.rglob(f"*{Constants.EXT_ETS}"):
        relative_path = ets_file.relative_to(src_path)

        data.append({
            "src": str(src_path / relative_path),
            "ast": str(Path(ast_dir) / relative_path.with_suffix(Constants.EXT_JSON)),
            "pa": str(Path(pa_dir) / relative_path.with_suffix(Constants.EXT_PA)),
        })

    return pd.DataFrame(data)


def summarize_runtime_data(runtime_dir: str,
                           delimiter: str = ',',
                           skip_errors: bool = True,
                           max_errors: int = 10) -> Optional[pd.DataFrame]:
    """Summarize runtime data"""
    try:
        csv_files = FileUtils.find_files(runtime_dir, f"*{Constants.EXT_CSV}")
        if not csv_files:
            print("No CSV files found")
            return None

        data = []
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

                        data.append([methodid, offset, counter])

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

        if not data:
            print("Error: No valid runtime data")
            return None

        # Create DataFrame and summarize
        df = pd.DataFrame(data, columns=Config.CSV_COLUMNS)
        result = df.groupby(["methodid", "offset"])["counter"].sum().reset_index()

        print(f"Successfully processed: {len(data)} lines of runtime data")
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


def export_lcov():
    """Export coverage information in LCOV format"""
    try:
        # Validate directories
        Config.validate_directories()

        # Create file mapping
        df = create_file_mapping_dataframe(
            Config.SOURCE_DIR,
            Config.AST_DIR,
            Config.PA_DIR
        )
        print(f"Found {len(df)} file mappings")

        # Summarize runtime data
        runtime_df = summarize_runtime_data(Config.RUNTIME_DIR)
        if runtime_df is None:
            print("No runtime data found")
            return

        # Delete old result file
        if Path(Config.OUTPUT_FILE).exists():
            Path(Config.OUTPUT_FILE).unlink()

        # Process each file
        success_count = 0
        coverage_info = CoverageInfo()
        for _, row in df.iterrows():
            try:
                coverage_info.set_file_path(
                    row["src"],
                    row["pa"],
                    row["ast"],
                    runtime_df
                )

                if coverage_info.process_file_result:
                    coverage_info.export_lcov(Config.OUTPUT_FILE)
                    success_count += 1

            except FileNotFoundError as e:
                print(f"File not found: {e}")
            except Exception as e:
                print(f"Error processing file {row['src']}: {e}")

        print(f"Successfully processed {success_count}/{len(df)} files")

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
                        added_lines_in_hunk < end_hunk   and
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


def main():
    parser = argparse.ArgumentParser(description='Coverage report generation tool')
    subparsers = parser.add_subparsers(dest='command', help='Available commands')

    # Add gen_report subcommand
    report_parser = subparsers.add_parser('gen_report', help='Generate coverage report')
    report_group = report_parser.add_mutually_exclusive_group()
    report_group.add_argument('--all', '-a', action='store_true', help='Full coverage report')
    report_group.add_argument('--diff', '-d', action='store_true', help='Incremental coverage report')

    args = parser.parse_args()

    if args.command == 'gen_report':
        genhtml_path = shutil.which('genhtml')
        if not genhtml_path:
            print("Error: genhtml command not found. Please install lcov package.")
            sys.exit(1)
        if (args.all):
            export_lcov()
            if not os.path.exists('result.info'):
                print("Error: result.info file does not exist")
                sys.exit(1)

            try:
                # Execute genhtml command to generate full coverage report
                subprocess.run(
                    [genhtml_path, 'result.info', '-o', 'coverage_report', '-s', '--branch-coverage'],
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
            export_lcov()
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
    else:
        print("Please specify a command")


if __name__ == "__main__":
    main()
