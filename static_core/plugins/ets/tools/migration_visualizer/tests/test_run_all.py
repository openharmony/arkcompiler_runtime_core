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

import os
import subprocess
import sys
import tempfile
import types
import unittest
from pathlib import Path
from unittest import mock

from support import load_module, write_json


class RunAllTest(unittest.TestCase):
    def test_package_main_runs_directly_from_src_tree_without_editable_install(self) -> None:
        project_root = Path(__file__).resolve().parents[1]
        script_path = project_root / "src" / "arkts_migration_visualizer" / "__main__.py"
        env = dict(os.environ)
        env.pop("PYTHONPATH", None)

        result = subprocess.run(
            [sys.executable, str(script_path), "--help"],
            cwd=project_root,
            env=env,
            text=True,
            capture_output=True,
            check=False,
        )

        self.assertEqual(result.returncode, 0, msg=result.stderr or result.stdout)
        self.assertIn("usage:", result.stdout)
        self.assertIn("export-timeline", result.stdout)

    def test_run_collection_stage_falls_back_to_local_module_when_package_import_is_unavailable(self) -> None:
        run_all = load_module("run_all", "src/arkts_migration_visualizer/cli.py")
        captured = {}

        class FakeCollectionRequest:
            def __init__(self, **kwargs):
                captured["request_kwargs"] = kwargs
                self.kwargs = kwargs

        fake_module = types.SimpleNamespace(
            CollectionRequest=FakeCollectionRequest,
            collect_artifacts=lambda request: "/tmp/fallback-run-dir",
        )

        def fake_import_module(name):
            if name == "arkts_migration_visualizer.collect.perf_runner":
                exc = ModuleNotFoundError("No module named 'arkts_migration_visualizer'")
                exc.name = "arkts_migration_visualizer"
                raise exc
            raise AssertionError(f"unexpected import: {name}")

        with mock.patch.object(run_all.importlib, "import_module", side_effect=fake_import_module), mock.patch.object(
            run_all,
            "_load_local_stage_module",
            return_value=fake_module,
        ) as local_loader_mock:
            result = run_all.run_collection_stage(
                ".*Demo",
                deps_root="/tmp/deps",
                manual_package="com.example.demo",
                manual_ability="EntryAbility",
                manual_duration=45,
            )

        self.assertEqual(result, Path("/tmp/fallback-run-dir"))
        local_loader_mock.assert_called_once_with(
            "_local_arkts_migration_visualizer_collect_perf_runner",
            "collect/perf_runner.py",
        )
        self.assertEqual(
            captured["request_kwargs"],
            {
                "testcase_regex": None,
                "manual_package": "com.example.demo",
                "manual_ability": "EntryAbility",
                "manual_duration": 45,
                "deps_root": "/tmp/deps",
            },
        )

    def test_run_build_stage_falls_back_to_local_module_when_package_import_is_unavailable(self) -> None:
        run_all = load_module("run_all", "src/arkts_migration_visualizer/cli.py")
        captured = {}

        class FakeIntegrationRequest:
            def __init__(self, **kwargs):
                captured["request_kwargs"] = kwargs
                self.kwargs = kwargs

        fake_module = types.SimpleNamespace(
            IntegrationRequest=FakeIntegrationRequest,
            integrate_artifact_bundle=lambda request: "/tmp/fallback-output.json",
        )

        def fake_import_module(name):
            if name == "arkts_migration_visualizer.build.integrate_dep":
                exc = ModuleNotFoundError("No module named 'arkts_migration_visualizer'")
                exc.name = "arkts_migration_visualizer"
                raise exc
            raise AssertionError(f"unexpected import: {name}")

        with mock.patch.object(run_all.importlib, "import_module", side_effect=fake_import_module), mock.patch.object(
            run_all,
            "_load_local_stage_module",
            return_value=fake_module,
        ) as local_loader_mock:
            result = run_all.run_build_stage(Path("/tmp/run-dir"), Path("/tmp/out.json"))

        self.assertEqual(result, Path("/tmp/fallback-output.json"))
        local_loader_mock.assert_called_once_with(
            "_local_arkts_migration_visualizer_build_integrate_dep",
            "build/integrate_dep.py",
        )
        self.assertEqual(
            captured["request_kwargs"],
            {
                "input_dir": Path("/tmp/run-dir"),
                "output_path": Path("/tmp/out.json"),
                "debug_enabled": True,
            },
        )

    def test_run_timeline_export_stage_falls_back_to_local_module_when_package_import_is_unavailable(self) -> None:
        run_all = load_module("run_all", "src/arkts_migration_visualizer/cli.py")
        captured = {}
        export_result = types.SimpleNamespace(output_path=Path("/tmp/fallback-trace.json"))

        class FakeTimelineExportRequest:
            def __init__(self, **kwargs):
                captured["request_kwargs"] = kwargs
                self.kwargs = kwargs

        fake_module = types.SimpleNamespace(
            TimelineExportRequest=FakeTimelineExportRequest,
            export_timeline_trace_artifact=lambda request: export_result,
        )

        def fake_import_module(name):
            if name == "arkts_migration_visualizer.collect.export_hapray_timeline_trace":
                exc = ModuleNotFoundError("No module named 'arkts_migration_visualizer'")
                exc.name = "arkts_migration_visualizer"
                raise exc
            raise AssertionError(f"unexpected import: {name}")

        with mock.patch.object(run_all.importlib, "import_module", side_effect=fake_import_module), mock.patch.object(
            run_all,
            "_load_local_stage_module",
            return_value=fake_module,
        ) as local_loader_mock:
            result = run_all.run_timeline_export_stage(
                "perf.db",
                output_path="trace.json",
                step=2,
                event_name="instructions",
                thread_ids=[101, 102],
                merge_samples=False,
            )

        self.assertIs(result, export_result)
        local_loader_mock.assert_called_once_with(
            "_local_arkts_migration_visualizer_collect_export_hapray_timeline_trace",
            "collect/export_hapray_timeline_trace.py",
        )
        self.assertEqual(
            captured["request_kwargs"],
            {
                "input_path": Path("perf.db"),
                "output_path": "trace.json",
                "step": 2,
                "event_name": "instructions",
                "thread_ids": (101, 102),
                "merge_samples": False,
            },
        )

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
