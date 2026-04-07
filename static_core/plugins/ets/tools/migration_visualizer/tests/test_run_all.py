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

from __future__ import annotations

import tempfile
import types
import unittest
from pathlib import Path
from unittest import mock

from support import load_module, write_json


class RunAllTest(unittest.TestCase):
    def test_pyproject_declares_tool_entrypoint_and_targets(self) -> None:
        project_root = Path(__file__).resolve().parents[1]
        content = (project_root / "pyproject.toml").read_text(encoding="utf-8")

        self.assertIn('arkts-migration-visualizer = "arkts_migration_visualizer.cli:main"', content)
        self.assertIn('test = "python -m unittest discover -s tests -v"', content)
        self.assertIn('lint = "ruff check src tests"', content)

    def test_find_latest_run_dir_and_required_files(self) -> None:
        run_all = load_module("run_all", "src/arkts_migration_visualizer/cli.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            artifacts_dir = Path(temp_dir) / "artifacts"
            older = artifacts_dir / "test_run_20260407_101010"
            newer = artifacts_dir / "test_run_20260408_101010"
            older.mkdir(parents=True)
            newer.mkdir(parents=True)
            run_all.ARTIFACTS_DIR = artifacts_dir

            self.assertEqual(run_all.find_latest_run_dir(), newer)
            self.assertFalse(run_all.check_run_dir_has_required_files(newer))

            for file_name in run_all.REQUIRED_FILES:
                write_json(newer / file_name, {})

            self.assertTrue(run_all.check_run_dir_has_required_files(newer))

    def test_choose_port_skips_busy_ports(self) -> None:
        run_all = load_module("run_all", "src/arkts_migration_visualizer/cli.py")
        attempts = []

        class FakeSocket:
            def __enter__(self):
                return self

            def __exit__(self, exc_type, exc, tb):
                return False

            def bind(self, address):
                attempts.append(address[1])
                if address[1] < 8002:
                    raise OSError("busy")

        with mock.patch.object(run_all.socket, "socket", side_effect=lambda *args, **kwargs: FakeSocket()):
            chosen = run_all.choose_port(8000)

        self.assertEqual(chosen, 8002)
        self.assertEqual(attempts, [8000, 8001, 8002])

    def test_main_export_timeline_uses_internal_stage_api(self) -> None:
        run_all = load_module("run_all", "src/arkts_migration_visualizer/cli.py")
        export_result = types.SimpleNamespace(
            perf_db_path=Path("/tmp/demo/perf.db"),
            output_path=Path("/tmp/demo/hapray_timeline_trace.json"),
            trace_obj={
                "metadata": {
                    "eventName": "instructions",
                    "intervalCount": 3,
                },
                "traceEvents": [1, 2, 3, 4],
            },
        )

        with mock.patch.object(
            run_all,
            "run_timeline_export_stage",
            return_value=export_result,
        ) as export_mock, mock.patch.object(run_all, "run_collection_stage") as collect_mock, mock.patch.object(
            run_all,
            "run_build_stage",
        ) as build_mock:
            run_all.main(
                [
                    "export-timeline",
                    "perf.db",
                    "--output",
                    "custom.json",
                    "--step",
                    "2",
                    "--event-name",
                    "instructions",
                    "--thread-id",
                    "101",
                    "--thread-id",
                    "102",
                    "--no-merge-samples",
                ]
            )

        export_mock.assert_called_once_with(
            "perf.db",
            output_path="custom.json",
            step=2,
            event_name="instructions",
            thread_ids=[101, 102],
            merge_samples=False,
        )
        self.assertFalse(collect_mock.called)
        self.assertFalse(build_mock.called)

    def test_main_export_timeline_wraps_system_exit_from_internal_stage(self) -> None:
        run_all = load_module("run_all", "src/arkts_migration_visualizer/cli.py")

        with mock.patch.object(run_all, "run_timeline_export_stage", side_effect=SystemExit("inner failure")):
            with self.assertRaises(SystemExit) as exc:
                run_all.main(["export-timeline", "perf.db"])

        self.assertEqual(str(exc.exception), "Timeline export failed. inner failure")

    def test_main_collect_wraps_system_exit_from_internal_stage(self) -> None:
        run_all = load_module("run_all", "src/arkts_migration_visualizer/cli.py")

        with mock.patch.object(run_all, "run_collection_stage", side_effect=SystemExit("inner failure")):
            with self.assertRaises(SystemExit) as exc:
                run_all.main(["--no-serve"])

        self.assertEqual(str(exc.exception), "Collection failed. inner failure")

    def test_main_skip_collect_builds_output_without_serving(self) -> None:
        run_all = load_module("run_all", "src/arkts_migration_visualizer/cli.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            project_root = Path(temp_dir)
            src_root = project_root / "src"
            artifacts_dir = project_root / "artifacts"
            web_dir = src_root / "web"
            run_dir = artifacts_dir / "test_run_20260408_101010"

            run_dir.mkdir(parents=True)
            for file_name in run_all.REQUIRED_FILES:
                write_json(run_dir / file_name, {})

            run_all.PROJECT_ROOT = project_root
            run_all.SRC_ROOT = src_root
            run_all.ARTIFACTS_DIR = artifacts_dir
            run_all.WEB_DIR = web_dir

            build_requests = []

            def fake_run_build_stage(run_dir_arg, output_path):
                build_requests.append((run_dir_arg, output_path))
                output_path.parent.mkdir(parents=True, exist_ok=True)
                output_path.write_text('{"status":"ok"}', encoding="utf-8")
                return output_path

            with mock.patch.object(run_all, "run_build_stage", side_effect=fake_run_build_stage), mock.patch.object(
                run_all,
                "serve_web",
            ) as serve_web_mock:
                run_all.main(["--skip-collect", "--no-serve"])

            self.assertFalse(serve_web_mock.called)
            self.assertEqual(len(build_requests), 1)
            self.assertTrue((web_dir / "hierarchical_integrated_data.json").is_file())
            self.assertEqual(build_requests[0][0], run_dir)

    def test_main_collect_uses_internal_stage_apis(self) -> None:
        run_all = load_module("run_all", "src/arkts_migration_visualizer/cli.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            project_root = Path(temp_dir)
            src_root = project_root / "src"
            artifacts_dir = project_root / "artifacts"
            web_dir = src_root / "web"
            run_dir = artifacts_dir / "test_run_20260408_101010"

            run_dir.mkdir(parents=True)
            for file_name in run_all.REQUIRED_FILES:
                write_json(run_dir / file_name, {})

            run_all.PROJECT_ROOT = project_root
            run_all.SRC_ROOT = src_root
            run_all.ARTIFACTS_DIR = artifacts_dir
            run_all.WEB_DIR = web_dir

            collection_requests = []
            build_requests = []

            def fake_run_collection_stage(testcase_regex, **kwargs):
                collection_requests.append((testcase_regex, kwargs))
                return run_dir

            def fake_run_build_stage(run_dir_arg, output_path):
                build_requests.append((run_dir_arg, output_path))
                output_path.parent.mkdir(parents=True, exist_ok=True)
                output_path.write_text('{"status":"ok"}', encoding="utf-8")
                return output_path

            with mock.patch.object(run_all, "run_collection_stage", side_effect=fake_run_collection_stage), mock.patch.object(
                run_all,
                "run_build_stage",
                side_effect=fake_run_build_stage,
            ), mock.patch.object(
                run_all,
                "serve_web",
            ) as serve_web_mock:
                run_all.main(["-t", ".*Demo", "--deps-root", "/tmp/deps", "--no-serve"])

            self.assertFalse(serve_web_mock.called)
            self.assertEqual(collection_requests, [(".*Demo", {"deps_root": "/tmp/deps", "manual_package": None, "manual_ability": None, "manual_duration": 30})])
            self.assertEqual(len(build_requests), 1)
            self.assertEqual(build_requests[0][0], run_dir)
            self.assertTrue((web_dir / "hierarchical_integrated_data.json").is_file())


if __name__ == "__main__":
    unittest.main()
