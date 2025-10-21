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
import os
import re
import shutil
import subprocess
import unittest
from pathlib import Path


class TestCoverageScript(unittest.TestCase):
    def setUp(self):
        """SetUp for the Tests"""
        self.current_source_dir = os.getenv("CURRENT_SOURCE_DIR")
        self.panda_bin_root = os.getenv("PANDA_BINARY_ROOT")
        print(f"current_source_dir: {self.current_source_dir}, panda_bin_root: {self.panda_bin_root}")

        self.src_path = "."
        self.es2panda_path = os.path.join(self.panda_bin_root, "bin/es2panda")
        self.ark_disasm_path = os.path.join(self.panda_bin_root, "bin/ark_disasm")
        self.ark_path = os.path.join(self.panda_bin_root, "bin/ark")
        self.etsstdlib_path = os.path.join(self.panda_bin_root, "plugins/ets/etsstdlib.abc")
        self.script_path = os.path.join(
            self.current_source_dir,
            "../../../../../../runtime/tooling/coverage/coverage.py"
        )
        self.output_path = os.path.join(self.panda_bin_root, "runtime/tooling/coverage")

        if os.path.isdir(self.output_path):
            try:
                shutil.rmtree(self.output_path)
            except Exception as e:
                print(f"delete directory Error: {e}")
        pass

    def tearDown(self):
        """clean up"""
        pass

    def test_host_output(self):
        """test the coverage script"""
        self.assertTrue(os.path.exists(self.es2panda_path), f"{self.es2panda_path} not exists")
        self.assertTrue(os.path.exists(self.ark_disasm_path), f"{self.ark_disasm_path} not exists")
        self.assertTrue(os.path.exists(self.ark_path), f"{self.ark_path} not exists")

        # execute coverage.py
        subprocess.run(
            [
                "python", self.script_path, "gen_ast", f"--src={self.src_path}", f"--output={self.output_path}",
                f"--es2panda={self.es2panda_path}", "--mode=host"
            ],
            check=True
        )
        subprocess.run(
            [
                "python", self.script_path, "gen_pa", f"--src={self.output_path}/abc",
                f"--output={self.output_path}/pa",
                f"--ark_disasm-path={self.ark_disasm_path}"
            ],
            check=True
        )
        ast_output = Path(f"{self.output_path}/runtime_info/coverageBytecodeInfo.csv")
        ast_output.parent.mkdir(parents=True, exist_ok=True)
        cmd = [
            f"{self.ark_path}", f"--boot-panda-files={self.etsstdlib_path}",
            "--load-runtimes=ets", f"{self.output_path}/abc/coverage_spec.abc", "coverage_spec.ETSGLOBAL::main",
            "--code-coverage-enabled=true", f"--code-coverage-output={ast_output}"
        ]
        print(*cmd)
        subprocess.run(cmd, check=True)
        subprocess.run(
            [
                "python", self.script_path, "gen_report", f"--src={self.src_path}", f"--workdir={self.output_path}",
                "--a"
            ],
            check=True
        )

        self.assertTrue(os.path.exists(f"{self.output_path}/report/index.html"),
                        "The report/index.html file is not generated")
        self.assertTrue(os.path.exists(f"{self.output_path}/result.info"), "The result.info file is not generated")

        cmd = ['lcov', '--summary', f"{self.output_path}/result.info", '--rc', 'lcov_branch_coverage=1']
        print(*cmd)
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            check=True
        )
        print(result.stdout)
        summary = self.parse_lcov_summary(result.stdout)

        # Check if all required coverage data exists
        required_categories = ['lines', 'functions', 'branches']
        for category in required_categories:
            self.assertIn(category, summary, f"{category} coverage data not found")

        # Line coverage: > 70%
        lines_percent = summary['lines']['percentage'] * 100
        self.assertGreater(lines_percent, 70.0,
                           f"Line coverage {lines_percent:.1f}% is not greater than 70%")

        # Function coverage: >= 90%
        functions_percent = summary['functions']['percentage'] * 100
        self.assertGreaterEqual(functions_percent, 90.0,
                                f"Function coverage {functions_percent:.1f}% is not greater than 90%")

        # Branch coverage: >= 55%
        branches_percent = summary['branches']['percentage'] * 100
        self.assertGreaterEqual(branches_percent, 55.0,
                                f"Branch coverage {branches_percent:.1f}% is not greater than 55%")

    def parse_lcov_summary(self, output):
        summary = {}

        pattern = r'(\w+)\.+:\s*([\d.]+)%\s*\((\d+)\s+of\s+(\d+)\s+\w+\)'

        for line in output.split('\n'):
            match = re.search(pattern, line)
            if match:
                category = match.group(1)
                percentage = float(match.group(2)) / 100
                covered = int(match.group(3))
                total = int(match.group(4))

                summary[category] = {
                    'percentage': percentage,
                    'covered': covered,
                    'total': total
                }

        return summary


if __name__ == "__main__":
    unittest.main()
