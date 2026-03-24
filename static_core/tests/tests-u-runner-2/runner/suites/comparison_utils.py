#!/usr/bin/env python3
# -- coding: utf-8 --
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
#
import re
from pathlib import Path

from runner.logger import Log

_LOGGER = Log.get_logger(__file__)


class ComparisonUtils:
    @staticmethod
    def compare_line_sets(expected: list[str], actual: list[str], expected_path: Path) -> tuple[bool, str]:
        expected_lines = set(expected)
        actual_lines = set(actual)

        if not actual_lines and not expected_lines:
            return True, "Nothing to compare"

        if not expected_lines:
            _LOGGER.all(f"{expected_path} is empty after normalization")
            return False, f"{expected_path} is empty after normalization"
        if not actual_lines:
            _LOGGER.all(f"{expected_path} is not empty, but actual output is")
            return False, f"{expected_path} is not empty, but actual output is"

        is_subset = expected_lines.issubset(actual_lines)
        if is_subset:
            return is_subset, "Comparison with expected output/error has passed\n"
        return is_subset, "Comparison with expected output/error has failed\n"

    @staticmethod
    def normalize_output_lines(output: list[str]) -> list[str]:
        norm_output = [ComparisonUtils.normalize_line(line) for line in output if line]
        return norm_output

    @staticmethod
    def normalize_line(line: str) -> str:
        return ComparisonUtils._remove_file_info_from_error(ComparisonUtils._normalize_error_report(line))

    @staticmethod
    def log_comparison_difference(actual_output: list[str] | None, expected_output: list[str] | None) -> list[str]:
        def find_difference(expected: list[str] | None, actual: list[str] | None) -> set[str]:
            if expected is None or actual is None:
                return set()
            expected_lines = set(expected)
            actual_lines = set(actual)
            return actual_lines.difference(expected_lines)

        differences = []

        dif_exp_output = sorted(list(find_difference(actual_output, expected_output)))
        if dif_exp_output:
            differences.append("Actual output doesn't contain expected lines:")
            differences.append(f"{dif_exp_output}")

        return differences

    @staticmethod
    def _normalize_error_report(report: str) -> str:
        pattern = r"\[TID [0-9a-fA-F]{6,}\]\s*"
        result = re.sub(pattern, "", report).strip()
        return ComparisonUtils._remove_tabs_and_spaces_from_begin(result)

    @staticmethod
    def _remove_tabs_and_spaces_from_begin(report: str) -> str:
        pattern = r"^\s+"
        return re.sub(pattern, "", report, flags=re.MULTILINE)

    @staticmethod
    def _remove_file_info_from_error(error_message: str) -> str:
        pattern = r'\s*[\[\(]\s*[^]\()]+\.ets:\d+:\d+\s*[\]\)]\s*|\s*[\[\(]\s*[^]\()]+\.abc\s*[\]\)]\s*'
        return re.sub(pattern, '', error_message)
