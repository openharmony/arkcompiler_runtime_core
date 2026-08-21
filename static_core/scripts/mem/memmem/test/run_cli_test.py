# -- coding: utf-8 --
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

import contextlib
import io
import json
import pathlib
import subprocess
import sys
import tempfile
import unittest

from run import build_parser


_ROOT = pathlib.Path(__file__).resolve().parent.parent


class RunCliTest(unittest.TestCase):
    def test_help_describes_flow_and_options(self) -> None:
        result = subprocess.run(
            [sys.executable, str(_ROOT.joinpath("run.py")), "--help"],
            cwd=_ROOT,
            capture_output=True,
            text=True,
            check=False,
        )

        self.assertEqual(result.returncode, 0)
        self.assertIn("--flow", result.stdout)
        self.assertIn("--out-dir", result.stdout)
        self.assertIn("--repeats", result.stdout)
        self.assertIn("--reboot", result.stdout)
        self.assertIn("--no-reboot", result.stdout)
        self.assertIn("--hilog", result.stdout)
        self.assertIn("--no-hilog", result.stdout)
        self.assertIn("--memmem-log-level", result.stdout)
        self.assertIn("--memmem-log-file", result.stdout)
        self.assertIn("--smaps-filter", result.stdout)
        self.assertIn(r".*\.so", result.stdout)

    def test_option_defaults(self) -> None:
        args = build_parser().parse_args(["--flow", "flow.json"])

        self.assertFalse(args.reboot)
        self.assertTrue(args.hilog)
        self.assertEqual(args.repeats, 1)
        self.assertEqual(args.memmem_log_level, "err")
        self.assertEqual(args.memmem_log_file, None)
        self.assertIsNone(args.smaps_filter)
        self.assertIsNone(args.out_dir)

    def test_out_dir_parses(self) -> None:
        args = build_parser().parse_args(
            ["--flow", "flow.json", "--out-dir", "results/bench1"])

        self.assertEqual(args.out_dir, pathlib.Path("results/bench1"))

    def test_smaps_filter_parses_compiled_regex(self) -> None:
        args = build_parser().parse_args(
            ["--flow", "flow.json", "--smaps-filter", r".*\.so"])

        self.assertEqual(args.smaps_filter.pattern, r".*\.so")
        self.assertIsNotNone(
            args.smaps_filter.match("/usr/lib/x86_64-linux-gnu/libc.so.6"))
        self.assertIsNone(args.smaps_filter.match("[anonymous]"))

    def test_invalid_smaps_filter_regex_rejected(self) -> None:
        with contextlib.redirect_stderr(io.StringIO()):
            with self.assertRaises(SystemExit):
                build_parser().parse_args(
                    ["--flow", "flow.json", "--smaps-filter", "["])

    def test_repeats_parses_positive(self) -> None:
        args = build_parser().parse_args(
            ["--flow", "flow.json", "--repeats", "2"])

        self.assertEqual(args.repeats, 2)

    def test_repeats_zero_rejected(self) -> None:
        with contextlib.redirect_stderr(io.StringIO()):
            with self.assertRaises(SystemExit):
                build_parser().parse_args(
                    ["--flow", "flow.json", "--repeats", "0"])

    def test_repeats_negative_rejected(self) -> None:
        with contextlib.redirect_stderr(io.StringIO()):
            with self.assertRaises(SystemExit):
                build_parser().parse_args(
                    ["--flow", "flow.json", "--repeats", "-1"])

    def test_no_reboot_disables_reboot(self) -> None:
        args = build_parser().parse_args(
            ["--flow", "flow.json", "--no-reboot"])

        self.assertFalse(args.reboot)

    def test_no_hilog_disables_hilog(self) -> None:
        args = build_parser().parse_args(
            ["--flow", "flow.json", "--no-hilog"])

        self.assertFalse(args.hilog)

    def test_memmem_log_options_parse(self) -> None:
        args = build_parser().parse_args(
            [
                "--flow", "flow.json",
                "--memmem-log-level", "info",
                "--memmem-log-file", "memmem.log",
            ]
        )

        self.assertEqual(args.memmem_log_level, "info")
        self.assertEqual(str(args.memmem_log_file), "memmem.log")

    def test_invalid_memmem_log_level_fails(self) -> None:
        with contextlib.redirect_stderr(io.StringIO()):
            with self.assertRaises(SystemExit):
                build_parser().parse_args(
                    ["--flow", "flow.json", "--memmem-log-level", "debug"])

    def test_removed_logs_options_fail(self) -> None:
        with contextlib.redirect_stderr(io.StringIO()):
            with self.assertRaises(SystemExit):
                build_parser().parse_args(["--flow", "flow.json", "--logs"])
            with self.assertRaises(SystemExit):
                build_parser().parse_args(["--flow", "flow.json", "--no-logs"])

    def test_contradictory_boolean_options_fail(self) -> None:
        with contextlib.redirect_stderr(io.StringIO()):
            with self.assertRaises(SystemExit):
                build_parser().parse_args(
                    ["--flow", "flow.json", "--hilog", "--no-hilog"])
            with self.assertRaises(SystemExit):
                build_parser().parse_args(
                    ["--flow", "flow.json", "--reboot", "--no-reboot"])

    def test_missing_flow_fails(self) -> None:
        result = subprocess.run(
            [sys.executable, str(_ROOT / "run.py")],
            cwd=_ROOT,
            capture_output=True,
            text=True,
            check=False,
        )

        self.assertNotEqual(result.returncode, 0)
        self.assertIn("--flow", result.stderr)

    def test_invalid_argument_fails(self) -> None:
        result = subprocess.run(
            [sys.executable, str(_ROOT / "run.py"), "--flow",
             "flow.json", "--unknown"],
            cwd=_ROOT,
            capture_output=True,
            text=True,
            check=False,
        )

        self.assertNotEqual(result.returncode, 0)
        self.assertIn("--unknown", result.stderr)

    def test_missing_flow_path_fails_without_traceback(self) -> None:
        result = subprocess.run(
            [sys.executable, str(_ROOT / "run.py"), "--flow", "missing.json"],
            cwd=_ROOT,
            capture_output=True,
            text=True,
            check=False,
        )

        self.assertNotEqual(result.returncode, 0)
        self.assertIn("memmem error:", result.stderr)
        self.assertIn("missing.json", result.stderr)
        self.assertNotIn("Traceback", result.stderr)

    def test_invalid_flow_json_fails_without_traceback(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            root.joinpath(".env").write_text(
                "HDC_PATH=/bin/false\n", encoding="utf-8")
            flow_path = root.joinpath("flow.json")
            flow_path.write_text("{", encoding="utf-8")

            result = subprocess.run(
                [sys.executable, str(_ROOT.joinpath("run.py")),
                 "--flow", str(flow_path)],
                cwd=root,
                capture_output=True,
                text=True,
                check=False,
            )

            self.assertEqual(result.returncode, 1)
            self.assertIn("memmem error:", result.stderr)
            self.assertNotIn("Traceback", result.stderr)

    def test_invalid_macro_flow_fails_validation_without_traceback(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            root.joinpath(".env").write_text(
                "HDC_PATH=/bin/false\n", encoding="utf-8")
            flow_path = root.joinpath("flow.json")
            flow_path.write_text(
                json.dumps(
                    {
                        "flow": [
                            {
                                "label": "app",
                                "bundle": "com.example",
                                "ability": "EntryAbility",
                                "terminate": False,
                                "commands": [
                                    {
                                        "macro": "repeat",
                                        "payload": {
                                            "iter_var": "a-b",
                                            "n_iter": 1,
                                            "commands": [],
                                        },
                                    }
                                ],
                            }
                        ]
                    }
                ),
                encoding="utf-8",
            )

            result = subprocess.run(
                [sys.executable, str(_ROOT.joinpath("run.py")),
                 "--flow", str(flow_path)],
                cwd=root,
                capture_output=True,
                text=True,
                check=False,
            )

            self.assertEqual(result.returncode, 1)
            self.assertIn("memmem error:", result.stderr)
            self.assertIn("validation error", result.stderr.lower())
            self.assertNotIn("Traceback", result.stderr)

    def test_valid_macro_flow_passes_schema_validation(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            root.joinpath(".env").write_text(
                "HDC_PATH=/bin/false\n", encoding="utf-8")
            flow_path = root.joinpath("flow.json")
            flow_path.write_text(
                json.dumps(
                    {
                        "$desc": "Macro flow.",
                        "flow": [
                            {
                                "label": "app",
                                "bundle": "com.example",
                                "ability": "EntryAbility",
                                "terminate": False,
                                "commands": [
                                    {
                                        "macro": "repeat",
                                        "payload": {
                                            "iter_var": "i",
                                            "n_iter": 2,
                                            "commands": [
                                                {
                                                    "action": "snapshot",
                                                    "payload": "shot_{i}",
                                                }
                                            ],
                                        },
                                    }
                                ],
                            }
                        ]
                    }
                ),
                encoding="utf-8",
            )

            result = subprocess.run(
                [sys.executable, str(_ROOT.joinpath("run.py")),
                 "--flow", str(flow_path)],
                cwd=root,
                capture_output=True,
                text=True,
                check=False,
            )

            self.assertEqual(result.returncode, 1)
            self.assertIn("memmem error:", result.stderr)
            self.assertNotIn("validation error", result.stderr.lower())
            self.assertNotIn("Traceback", result.stderr)


if __name__ == "__main__":
    unittest.main()
