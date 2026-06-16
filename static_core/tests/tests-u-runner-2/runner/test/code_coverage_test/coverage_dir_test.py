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

import shutil
import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace
from typing import cast

from runner.code_coverage.coverage_dir import CoverageDir
from runner.options.options_general import GeneralOptions


class CoverageDirTest(unittest.TestCase):
    def setUp(self) -> None:
        self.work_dir = Path(tempfile.mkdtemp())

    def tearDown(self) -> None:
        shutil.rmtree(self.work_dir, ignore_errors=True)

    def test_root_is_created_under_work_dir(self) -> None:
        cov = self._make()
        self.assertEqual(cov.root, self.work_dir / "coverage")
        self.assertTrue(cov.root.is_dir())

    def test_coverage_work_dir(self) -> None:
        cov = self._make()
        self.assertEqual(cov.coverage_work_dir, self.work_dir / "coverage" / "work_dir")
        self.assertTrue(cov.coverage_work_dir.is_dir())

    def test_named_files_live_under_work_dir(self) -> None:
        cov = self._make()
        work = cov.coverage_work_dir
        self.assertEqual(cov.info_file, work / "coverage_lcov_format.info")
        self.assertEqual(cov.json_file, work / "coverage_json_format.json")
        self.assertEqual(cov.profdata_files_list_file, work / "profdatalist.txt")
        self.assertEqual(cov.profdata_merged_file, work / "merged.profdata")

    def test_html_report_dir_default(self) -> None:
        cov = self._make()
        self.assertEqual(cov.html_report_dir, cov.root / "html_report_dir")
        self.assertTrue(cov.html_report_dir.is_dir())

    def test_html_report_dir_custom(self) -> None:
        custom = self.work_dir / "custom_html"
        cov = self._make(html_report_dir=custom)
        self.assertEqual(cov.html_report_dir, custom)
        self.assertTrue(custom.is_dir())

    def test_profdata_dir_default(self) -> None:
        cov = self._make()
        self.assertEqual(cov.profdata_dir, cov.root / "profdata_dir")
        self.assertTrue(cov.profdata_dir.is_dir())

    def test_profdata_dir_custom(self) -> None:
        custom = self.work_dir / "custom_profdata"
        cov = self._make(profdata_files_dir=custom)
        self.assertEqual(cov.profdata_dir, custom)
        self.assertTrue(custom.is_dir())

    def test_root_is_cached(self) -> None:
        cov = self._make()
        self.assertIs(cov.root, cov.root)

    def _make(self, html_report_dir: Path | None = None, profdata_files_dir: Path | None = None) -> CoverageDir:
        general = SimpleNamespace(coverage=SimpleNamespace(
            coverage_html_report_dir=html_report_dir,
            profdata_files_dir=profdata_files_dir,
        ))
        return CoverageDir(cast(GeneralOptions, general), self.work_dir)


if __name__ == "__main__":
    unittest.main()
