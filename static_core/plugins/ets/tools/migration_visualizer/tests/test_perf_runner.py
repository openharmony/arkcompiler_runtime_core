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

import json
import tempfile
import unittest
from pathlib import Path
from unittest import mock

from support import load_module


class PerfRunnerTest(unittest.TestCase):
    def test_extract_bundle_name_ignores_json5_comments(self) -> None:
        perf_runner = load_module("perf_runner", "src/arkts_migration_visualizer/collect/perf_runner.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            manifest_path = Path(temp_dir) / "app.json5"
            manifest_path.write_text(
                """
                {
                  // app level comment
                  "bundleName": "com.example.demo" /* inline comment */
                }
                """,
                encoding="utf-8",
            )

            self.assertEqual(
                perf_runner.extract_bundle_name_from_app_manifest(manifest_path),
                "com.example.demo",
            )

    def test_resolve_manual_launch_ability_auto_detects_entry_module(self) -> None:
        perf_runner = load_module("perf_runner", "src/arkts_migration_visualizer/collect/perf_runner.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            project_path = repo_root / "project"
            (project_path / "AppScope").mkdir(parents=True)
            (project_path / "entry" / "src" / "main").mkdir(parents=True)
            (project_path / "feature" / "src" / "main").mkdir(parents=True)
            (project_path / "AppScope" / "app.json5").write_text(
                '{ "bundleName": "com.example.demo" }',
                encoding="utf-8",
            )
            (project_path / "entry" / "src" / "main" / "module.json5").write_text(
                '{ "type": "entry", "mainElement": "EntryAbility" }',
                encoding="utf-8",
            )
            (project_path / "feature" / "src" / "main" / "module.json5").write_text(
                '{ "type": "feature", "mainElement": "FeatureAbility" }',
                encoding="utf-8",
            )

            resolved_project, ability = perf_runner.resolve_manual_launch_ability(
                repo_root,
                {"project_path": "project"},
                "com.example.demo",
                None,
            )

            self.assertEqual(resolved_project, project_path.resolve())
            self.assertEqual(ability, "EntryAbility")

    def test_create_manual_hapray_testcase_writes_python_and_json(self) -> None:
        perf_runner = load_module("perf_runner", "src/arkts_migration_visualizer/collect/perf_runner.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            workspace = Path(temp_dir)
            case_name, case_dir = perf_runner.create_manual_hapray_testcase(
                workspace,
                "com.example.demo",
                45,
                main_ability="EntryAbility",
            )

            py_path = case_dir / f"{case_name}.py"
            json_path = case_dir / f"{case_name}.json"
            metadata = json.loads(json_path.read_text(encoding="utf-8"))

            self.assertEqual(case_name, "PerfLoad_Manual_com_example_demo")
            self.assertTrue(py_path.is_file())
            self.assertTrue(json_path.is_file())
            self.assertIn('self._app_package = "com.example.demo"', py_path.read_text(encoding="utf-8"))
            self.assertEqual(metadata["driver"]["py_file"], [py_path.name])

            perf_runner.cleanup_manual_hapray_testcase(case_dir)
            self.assertFalse(case_dir.exists())

    def test_extract_hapray_outputs_copies_latest_report_and_sorts_timings(self) -> None:
        perf_runner = load_module("perf_runner", "src/arkts_migration_visualizer/collect/perf_runner.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            base = Path(temp_dir)
            reports_root = base / "reports"
            latest_case = reports_root / "20260408112233" / "sample_case"
            older_case = reports_root / "20260407112233" / "older_case"
            run_dir = base / "artifacts" / "test_run_20260408_112233"
            (latest_case / "report").mkdir(parents=True)
            (latest_case / "hiperf").mkdir(parents=True)
            (older_case / "report").mkdir(parents=True)
            (older_case / "hiperf").mkdir(parents=True)
            run_dir.mkdir(parents=True)
            (latest_case / "report" / "hapray_report.json").write_text(
                '{"perf":{"steps":[]}}',
                encoding="utf-8",
            )
            (latest_case / "hiperf" / "hiperf_info.json").write_text(
                json.dumps(
                    {"har": [{"name": "utils.har", "count": 3}, {"name": "entry.har", "count": 10}]},
                    ensure_ascii=False,
                ),
                encoding="utf-8",
            )
            (older_case / "report" / "hapray_report.json").write_text(
                '{"perf":{"steps":[]}}',
                encoding="utf-8",
            )

            perf_runner.extract_hapray_outputs(reports_root, run_dir)

            timings = json.loads((run_dir / "component_timings.json").read_text(encoding="utf-8"))
            self.assertTrue((run_dir / "hapray_report.json").is_file())
            self.assertEqual([item["name"] for item in timings], ["entry.har", "utils.har"])

    def test_resolve_hapray_reports_root_prefers_latest_existing_candidate(self) -> None:
        perf_runner = load_module("perf_runner", "src/arkts_migration_visualizer/collect/perf_runner.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            base = Path(temp_dir)
            hapray_root = base / "deps" / "sources" / "ArkAnalyzer-HapRay"
            hapray_workspace = hapray_root / "perf_testing"
            expected_reports = hapray_workspace / "reports"
            external_reports = base / "home" / "ArkAnalyzer-HapRay" / "reports"
            (expected_reports / "20260410010101").mkdir(parents=True)
            (external_reports / "20260410020202").mkdir(parents=True)

            with mock.patch.object(perf_runner.Path, "home", return_value=base / "home"):
                resolved = perf_runner.resolve_hapray_reports_root(expected_reports, hapray_root, hapray_workspace)

            self.assertEqual(resolved, external_reports)

    def test_resolve_hapray_reports_root_checks_preferred_root_first(self) -> None:
        perf_runner = load_module("perf_runner", "src/arkts_migration_visualizer/collect/perf_runner.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            base = Path(temp_dir)
            hapray_root = base / "deps" / "sources" / "ArkAnalyzer-HapRay"
            hapray_workspace = hapray_root / "perf_testing"
            preferred_root = base / "custom" / "reports"
            builtin_reports = hapray_workspace / "reports"
            (preferred_root / "20260410030303").mkdir(parents=True)
            (builtin_reports / "20260410020202").mkdir(parents=True)

            resolved = perf_runner.resolve_hapray_reports_root(preferred_root, hapray_root, hapray_workspace)

            self.assertEqual(resolved, preferred_root)

    def test_resolve_hapray_source_reports_root_prefers_environment_over_config(self) -> None:
        perf_runner = load_module("perf_runner", "src/arkts_migration_visualizer/collect/perf_runner.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            workspace = repo_root / "deps" / "sources" / "ArkAnalyzer-HapRay" / "perf_testing"
            env_root = repo_root / "env_reports"
            cfg_root = repo_root / "cfg_reports"

            with mock.patch.dict(
                perf_runner.os.environ,
                {perf_runner.HAPRAY_REPORTS_ROOT_ENV: str(env_root)},
                clear=False,
            ):
                resolved = perf_runner.resolve_hapray_source_reports_root(
                    repo_root,
                    {perf_runner.HAPRAY_REPORTS_ROOT_CONFIG_KEY: str(cfg_root)},
                    workspace,
                )

            self.assertEqual(resolved, env_root)

    def test_resolve_hapray_source_reports_root_defaults_to_workspace_reports(self) -> None:
        perf_runner = load_module("perf_runner", "src/arkts_migration_visualizer/collect/perf_runner.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            workspace = repo_root / "deps" / "sources" / "ArkAnalyzer-HapRay" / "perf_testing"

            with mock.patch.dict(
                perf_runner.os.environ,
                {
                    perf_runner.HAPRAY_REPORTS_ROOT_ENV: "",
                    perf_runner.INTERNAL_HAPRAY_REPORTS_ROOT_ENV: "",
                },
                clear=False,
            ):
                resolved = perf_runner.resolve_hapray_source_reports_root(repo_root, {}, workspace)

            self.assertEqual(resolved, workspace / "reports")

    def test_ensure_hapray_source_launcher_writes_wrapper_script(self) -> None:
        perf_runner = load_module("perf_runner", "src/arkts_migration_visualizer/collect/perf_runner.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            workspace = Path(temp_dir) / "perf_testing"
            launcher = perf_runner.ensure_hapray_source_launcher(workspace)
            content = launcher.read_text(encoding="utf-8")

            self.assertTrue(launcher.is_file())
            self.assertIn(perf_runner.INTERNAL_HAPRAY_REPORTS_ROOT_ENV, content)
            self.assertIn("from hapray.actions.perf_action import PerfAction", content)
            self.assertIn("only 'perf' is supported", content)
            self.assertNotIn("runpy.run_module", content)

    def test_hilog_archive_quarantine_moves_only_archives_with_text_pairs(self) -> None:
        perf_runner = load_module("perf_runner", "src/arkts_migration_visualizer/collect/perf_runner.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            reports_root = Path(temp_dir) / "reports"
            paired_gzip = reports_root / "20260408112233" / "case_a" / "report" / "hilog001.gz"
            paired_text = reports_root / "20260408112233" / "case_a" / "report" / "hilog001.txt"
            lonely_gzip = reports_root / "20260408112233" / "case_a" / "report" / "hilog002.gz"
            paired_gzip.parent.mkdir(parents=True)
            paired_gzip.write_text("gz", encoding="utf-8")
            paired_text.write_text("txt", encoding="utf-8")
            lonely_gzip.write_text("gz", encoding="utf-8")

            quarantine = perf_runner.HilogArchiveQuarantine(reports_root)
            moved = quarantine.quarantine_once()

            moved_target = quarantine.backup_root / "20260408112233" / "case_a" / "report" / "hilog001.gz"
            self.assertEqual(moved, 1)
            self.assertFalse(paired_gzip.exists())
            self.assertTrue(moved_target.is_file())
            self.assertTrue(lonely_gzip.is_file())

    def test_resolve_path_converts_windows_and_relative_paths(self) -> None:
        perf_runner = load_module("perf_runner", "src/arkts_migration_visualizer/collect/perf_runner.py")
        base_dir = Path("/repo/root")

        self.assertEqual(
            perf_runner.resolve_path(base_dir, r"C:\Users\dev\demo"),
            Path("/mnt/c/Users/dev/demo"),
        )
        self.assertEqual(
            perf_runner.resolve_path(base_dir, "configs/config.json"),
            (base_dir / "configs/config.json").resolve(),
        )

    def test_find_node_executable_checks_macos_nvm_locations(self) -> None:
        perf_runner = load_module("perf_runner", "src/arkts_migration_visualizer/collect/perf_runner.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            home_dir = Path(temp_dir) / "home"
            node_path = home_dir / ".nvm" / "versions" / "node" / "v22.3.0" / "bin" / "node"
            node_path.parent.mkdir(parents=True)
            node_path.write_text("node", encoding="utf-8")

            with mock.patch.object(perf_runner.sys, "platform", "darwin"), mock.patch.object(
                perf_runner.Path,
                "home",
                return_value=home_dir,
            ), mock.patch.object(
                perf_runner.shutil,
                "which",
                return_value=None,
            ):
                self.assertEqual(perf_runner.find_node_executable(), node_path.resolve())

    def test_find_hdc_executable_checks_macos_non_exe_sdk_paths(self) -> None:
        perf_runner = load_module("perf_runner", "src/arkts_migration_visualizer/collect/perf_runner.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            sdk_path = repo_root / "sdk" / "default"
            hdc_path = sdk_path / "base" / "toolchains" / "hdc"
            hdc_path.parent.mkdir(parents=True)
            hdc_path.write_text("hdc", encoding="utf-8")

            with mock.patch.object(perf_runner.sys, "platform", "darwin"), mock.patch.object(
                perf_runner.shutil,
                "which",
                return_value=None,
            ):
                self.assertEqual(
                    perf_runner.find_hdc_executable(repo_root, {}, None, sdk_path),
                    hdc_path.resolve(),
                )

    def test_build_hapray_env_creates_workspace_tool_links(self) -> None:
        perf_runner = load_module("perf_runner", "src/arkts_migration_visualizer/collect/perf_runner.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            hapray_root = repo_root / "deps" / "sources" / "ArkAnalyzer-HapRay"
            workspace = hapray_root / "perf_testing"
            tools_root = hapray_root / "tools"
            workspace.mkdir(parents=True)
            (tools_root / "sa-cmd").mkdir(parents=True)
            (tools_root / "trace_streamer_binary").mkdir(parents=True)
            (tools_root / "web").mkdir(parents=True)
            (tools_root / "web" / "report_template.html").write_text("ok", encoding="utf-8")
            (tools_root / "web" / "hiperf_report_template.html").write_text("ok", encoding="utf-8")
            (tools_root / "xvm").mkdir(parents=True)
            (tools_root / "hilogtool").mkdir(parents=True)
            (tools_root / "hilogtool" / "hilogtool").write_text("ok", encoding="utf-8")
            node_path = repo_root / "bin" / "node"
            hdc_path = repo_root / "bin" / "hdc"
            node_path.parent.mkdir(parents=True, exist_ok=True)
            node_path.write_text("node", encoding="utf-8")
            hdc_path.write_text("hdc", encoding="utf-8")

            with mock.patch.object(perf_runner, "find_node_executable", return_value=node_path), mock.patch.object(
                perf_runner,
                "find_hdc_executable",
                return_value=hdc_path,
            ):
                env = perf_runner.build_hapray_env(repo_root, {}, workspace)

            self.assertTrue((workspace / "tools").exists())
            self.assertTrue((workspace / "sa-cmd").exists())
            self.assertTrue((workspace / "trace_streamer_binary").exists())
            self.assertIn(str(node_path.parent), env["PATH"])
            self.assertIn(str(hdc_path.parent), env["PATH"])

    def test_build_hapray_env_requires_hdc(self) -> None:
        perf_runner = load_module("perf_runner", "src/arkts_migration_visualizer/collect/perf_runner.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            hapray_root = repo_root / "deps" / "sources" / "ArkAnalyzer-HapRay"
            workspace = hapray_root / "perf_testing"
            tools_root = hapray_root / "tools"
            workspace.mkdir(parents=True)
            (tools_root / "sa-cmd").mkdir(parents=True)
            (tools_root / "trace_streamer_binary").mkdir(parents=True)
            (tools_root / "web").mkdir(parents=True)
            (tools_root / "web" / "report_template.html").write_text("ok", encoding="utf-8")
            (tools_root / "web" / "hiperf_report_template.html").write_text("ok", encoding="utf-8")
            (tools_root / "xvm").mkdir(parents=True)
            (tools_root / "hilogtool").mkdir(parents=True)
            (tools_root / "hilogtool" / "hilogtool").write_text("ok", encoding="utf-8")
            node_path = repo_root / "bin" / "node"
            node_path.parent.mkdir(parents=True, exist_ok=True)
            node_path.write_text("node", encoding="utf-8")

            with mock.patch.object(perf_runner, "find_node_executable", return_value=node_path), mock.patch.object(
                perf_runner,
                "find_hdc_executable",
                return_value=None,
            ):
                with self.assertRaises(SystemExit) as exc:
                    perf_runner.build_hapray_env(repo_root, {}, workspace)

            self.assertIn("Could not locate the hdc executable", str(exc.exception))

    def test_ensure_hapray_runtime_ready_reports_missing_supported_python_once(self) -> None:
        perf_runner = load_module("perf_runner", "src/arkts_migration_visualizer/collect/perf_runner.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            hapray_root = repo_root / "deps" / "sources" / "ArkAnalyzer-HapRay"
            workspace = hapray_root / "perf_testing"
            workspace.mkdir(parents=True)

            with mock.patch.object(
                perf_runner,
                "hapray_runtime_missing_items",
                side_effect=[
                    ["runtime assets", "macos virtual environment"],
                    ["macos virtual environment"],
                ],
            ), mock.patch.object(
                perf_runner,
                "bootstrap_dependencies",
                return_value={
                    "tools": {
                        "hapray": {
                            "status": "downloaded",
                            "note": "compatible Python 3.10-3.12 not found; source download completed",
                        }
                    }
                },
            ) as bootstrap_mock:
                with self.assertRaises(SystemExit) as exc:
                    perf_runner.ensure_hapray_runtime_ready(repo_root, {}, hapray_root)

            self.assertEqual(bootstrap_mock.call_count, 1)
            self.assertIn("compatible Python 3.10-3.12 not found", str(exc.exception))
            self.assertIn("hapray_python", str(exc.exception))

    def test_dependency_bootstrap_impl_path_prefers_src_layout(self) -> None:
        perf_runner = load_module("perf_runner", "src/arkts_migration_visualizer/collect/perf_runner.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            src_bootstrap = repo_root / "src" / "scripts" / "deps" / "internal" / "bootstrap_impl.py"
            legacy_bootstrap = repo_root / "scripts" / "deps" / "internal" / "bootstrap_impl.py"
            src_bootstrap.parent.mkdir(parents=True)
            legacy_bootstrap.parent.mkdir(parents=True)
            src_bootstrap.write_text("# src bootstrap\n", encoding="utf-8")
            legacy_bootstrap.write_text("# legacy bootstrap\n", encoding="utf-8")

            self.assertEqual(perf_runner.dependency_bootstrap_impl_path(repo_root), src_bootstrap)

    def test_resolve_tool_roots_for_mode_bootstraps_missing_homecheck_entry(self) -> None:
        perf_runner = load_module("perf_runner", "src/arkts_migration_visualizer/collect/perf_runner.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            homecheck_root = repo_root / "deps" / "runtime" / "homecheck"
            hapray_source = repo_root / "deps" / "sources" / "ArkAnalyzer-HapRay"
            homecheck_root.mkdir(parents=True)
            hapray_source.mkdir(parents=True)

            initial_state = {
                "tools": {
                    "hapray": {
                        "sourceDir": str(hapray_source),
                        "workspaceDir": str(hapray_source / "perf_testing"),
                    }
                }
            }
            repaired_state = {
                "tools": {
                    "homecheck": {
                        "runtimeDir": str(homecheck_root),
                    },
                    "hapray": {
                        "sourceDir": str(hapray_source),
                        "workspaceDir": str(hapray_source / "perf_testing"),
                    },
                }
            }

            with mock.patch.object(
                perf_runner,
                "load_dependency_state",
                return_value=initial_state,
            ), mock.patch.object(
                perf_runner,
                "bootstrap_dependencies",
                return_value=repaired_state,
            ) as bootstrap_mock:
                resolved_homecheck, resolved_hapray = perf_runner.resolve_tool_roots_for_mode(
                    repo_root,
                    {},
                    prefer_hapray_source=True,
                )

            self.assertEqual(bootstrap_mock.call_count, 1)
            self.assertEqual(bootstrap_mock.call_args.kwargs["tool"], "homecheck")
            self.assertEqual(resolved_homecheck, homecheck_root)
            self.assertEqual(resolved_hapray, hapray_source)


if __name__ == "__main__":
    unittest.main()
