#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
import logging
import re
from datetime import date
from os import path
from pathlib import Path
from typing import List, Optional, Set
import yaml

from runner.test_base import Test
from runner.reports.spec_node import SpecNode
from runner.reports.pdf_loader import PdfLoader
from runner.utils import write_2_file
from runner.logger import Log

_LOGGER = logging.getLogger("runner.reports.report_by_folders")
_DIR_PATTERN = re.compile(r"([0-9][0-9])\.(.*)")


class SpecReport:
    REPORT_DATE = "${Date}"
    REPORT_TEST_SUITE = "${TestSuite}"
    REPORT_EXCLUDED_HEADER = "${ExcludedHeader}"
    REPORT_EXCLUDED_DIVIDER = "${ExcludedDivider}"
    REPORT_RESULT = "${Result}"
    REPORT_SPEC_FILE = "${SpecFile}"
    REPORT_SPEC_CREATION_DATE = "${SpecCreationDate}"

    TEMPLATE = "spec_report_template.md"
    NON_TESTABLE = "arkts-non-testable.yaml"
    EXCLUDED_HEADER = " Excluded after |"
    EXCLUDED_DIVIDER = "-------|"

    def __init__(
        self, results: List[Test], test_suite: str, report_path: Path, report_file: Optional[Path], spec_file: Path
    ):
        self.tests = results
        self.test_suite = test_suite
        if report_file is None:
            self.report_file = path.join(report_path, f"{test_suite}_spec-report.md")
        else:
            self.report_file = str(report_file)
        self.spec_file = spec_file
        self.has_excluded = False

        # get spec data
        pdf_loader = PdfLoader(spec_file).parse()
        self.spec: SpecNode = pdf_loader.get_root_node()
        self.spec_creation_date: str = pdf_loader.get_creation_date()

        # get non-testable data
        self.non_testable: Set = self.__load_non_testable()

        # process test results
        self.__calculate()

    def __load_non_testable(self) -> Set:
        fpath = Path(path.dirname(path.abspath(__file__)), self.NON_TESTABLE)
        data = Path.read_text(fpath)
        return set(yaml.safe_load(data))

    def __calculate_one_test(self, test: Test) -> None:
        node = self.spec
        self.__update_summary(test, node)

        dir_parts = test.test_id.split("/")
        out_dir = ""
        for dir_part in dir_parts:
            matches = _DIR_PATTERN.match(dir_part)
            if matches is None or len(matches.groups()) != 2:
                break
            num = int(matches.group(1))
            out_dir = dir_part if out_dir == "" else out_dir + "/" + dir_part
            for _ in range(len(node.children), num):
                SpecNode("?", "", node)
            node = node.children[num - 1]
            node.dir = out_dir
            self.__update_summary(test, node)

    def __update_summary(self, test: Test, node: SpecNode) -> None:
        node.total += 1
        if test.excluded:
            node.excluded_after += 1
            self.has_excluded = True
        elif test.passed and not test.ignored:
            node.passed += 1
        elif not test.passed and not test.ignored:
            node.failed += 1
        elif test.passed and test.ignored:
            node.ignored_but_passed += 1
        elif not test.passed and test.ignored:
            node.ignored += 1

    def __calculate(self) -> None:
        for test in self.tests:
            self.__calculate_one_test(test)

    @staticmethod
    def __fmt_str(src: str) -> str:
        "Escape some characters for markdown document"
        res = src.replace("\\", "\\\\")
        res = res.replace("|", "\\|")
        res = res.replace("_", "\\_")
        res = res.replace("<", "\\<")
        res = res.replace(">", "\\>")
        res = res.replace("*", "\\*")
        res = res.replace("`", "\\`")
        res = res.replace("{", "\\{")
        res = res.replace("}", "\\}")
        res = res.replace("[", "\\[")
        res = res.replace("]", "\\]")
        res = res.replace("(", "\\(")
        res = res.replace(")", "\\)")
        res = res.replace("#", "\\#")
        res = res.replace("+", "\\+")
        res = res.replace("-", "\\-")
        res = res.replace(".", "\\.")
        res = res.replace("!", "\\!")
        return res

    @staticmethod
    def __fmt_num(src: int) -> str:
        "Replace 0 with space"
        return str(src) if src > 0 else ""

    def __report_node(self, node: SpecNode) -> str:
        non_testable = node.prefix in self.non_testable
        title = self.__fmt_str(node.prefix + " " + node.title)
        dirs = self.__fmt_str(node.dir)
        total = str(node.total) if node.total > 0 or not non_testable else "N/A"
        passed = self.__fmt_num(node.passed)
        failed = self.__fmt_num(node.failed)
        ignored_passed = self.__fmt_num(node.ignored_but_passed)
        ignored = self.__fmt_num(node.ignored)
        excluded = (" " + self.__fmt_num(node.excluded_after) + " |") if self.has_excluded else ""
        return f"| {title} | {dirs} | {total} | {passed} | {failed} | {ignored_passed} | {ignored} |{excluded}"

    def __report_nodes(self, nodes: List[SpecNode], lines: List[str]) -> None:
        for node in nodes:
            lines.append(self.__report_node(node))
            self.__report_nodes(node.children, lines)

    def populate_report(self) -> None:
        template_path = path.join(path.dirname(path.abspath(__file__)), self.TEMPLATE)
        with open(template_path, "r", encoding="utf-8") as file_pointer:
            report = file_pointer.read()
        report = report.replace(self.REPORT_DATE, str(date.today()))
        report = report.replace(self.REPORT_TEST_SUITE, self.test_suite)
        report = report.replace(self.REPORT_SPEC_FILE, self.__fmt_str(str(self.spec_file)))
        report = report.replace(self.REPORT_SPEC_CREATION_DATE, self.spec_creation_date)

        if not self.has_excluded:
            report = report.replace(self.REPORT_EXCLUDED_HEADER, "")
            report = report.replace(self.REPORT_EXCLUDED_DIVIDER, "")
        else:
            report = report.replace(self.REPORT_EXCLUDED_HEADER, self.EXCLUDED_HEADER)
            report = report.replace(self.REPORT_EXCLUDED_DIVIDER, self.EXCLUDED_DIVIDER)

        lines: List[str] = []
        self.__report_nodes(self.spec.children, lines)
        lines.append(self.__report_node(self.spec))  # root node - summary line at the bottom
        result = "\n".join(lines)
        report = report.replace(self.REPORT_RESULT, result)
        Log.default(_LOGGER, f"Spec coverage report is saved to '{self.report_file}'")
        write_2_file(self.report_file, report)
