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
import unittest
from pathlib import Path
from typing import Optional, Tuple
from unittest import mock

from support import load_module


class BootstrapImplTest(unittest.TestCase):
    def test_clone_git_repo_retries_after_partial_checkout_failure(self) -> None:
        bootstrap_impl = load_module("bootstrap_impl", "src/arkts_migration_visualizer/deps/bootstrap_impl.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            target_dir = Path(temp_dir) / "ArkAnalyzer-HapRay"
            target_dir.mkdir(parents=True)
            (target_dir / "stale.txt").write_text("stale", encoding="utf-8")
            clone_calls = []

            def fake_run_cmd(cmd, cwd=None, env=None, capture=False):
                clone_calls.append(cmd)
                if len(clone_calls) == 1:
                    raise bootstrap_impl.CommandError("early EOF")
                target_dir.mkdir(parents=True, exist_ok=True)
                (target_dir / ".git").mkdir(parents=True, exist_ok=True)
                return mock.Mock(returncode=0, stdout="")

            with mock.patch.object(bootstrap_impl, "run_cmd", side_effect=fake_run_cmd), mock.patch.object(
                bootstrap_impl.time,
                "sleep",
                return_value=None,
            ):
                bootstrap_impl.clone_git_repo("https://example.com/ArkAnalyzer-HapRay.git", target_dir, attempts=2)

            self.assertEqual(len(clone_calls), 2)
            self.assertFalse((target_dir / "stale.txt").exists())
            self.assertTrue((target_dir / ".git").exists())

    def test_ensure_gui_agent_checkout_reuses_retrying_clone_helper(self) -> None:
        bootstrap_impl = load_module("bootstrap_impl", "src/arkts_migration_visualizer/deps/bootstrap_impl.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            third_party_dir = Path(temp_dir)
            gui_agent_dir = third_party_dir / bootstrap_impl.GUI_AGENT_DIR_NAME

            with mock.patch.object(bootstrap_impl, "clone_git_repo") as clone_git_repo_mock, mock.patch.object(
                bootstrap_impl,
                "repo_commit",
                return_value="abc123",
            ):
                repo_dir, commit = bootstrap_impl.ensure_gui_agent_checkout(third_party_dir)

            clone_git_repo_mock.assert_called_once_with(bootstrap_impl.GUI_AGENT_REPO_URL, gui_agent_dir)
            self.assertEqual(repo_dir, gui_agent_dir)
            self.assertEqual(commit, "abc123")

    def test_resolve_python_package_installer_prefers_uv_when_available(self) -> None:
        bootstrap_impl = load_module("bootstrap_impl", "src/arkts_migration_visualizer/deps/bootstrap_impl.py")

        def fake_resolve_executable(name: str, env=None) -> str:
            if name == "uv":
                return "/usr/bin/uv"
            raise bootstrap_impl.CommandError("missing")

        with mock.patch.object(bootstrap_impl, "resolve_executable", side_effect=fake_resolve_executable):
            self.assertEqual(bootstrap_impl.resolve_python_package_installer("auto"), "uv")

    def test_install_optional_gui_agent_skips_when_not_requested(self) -> None:
        bootstrap_impl = load_module("bootstrap_impl", "src/arkts_migration_visualizer/deps/bootstrap_impl.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            result = bootstrap_impl.install_optional_gui_agent(
                enabled=False,
                third_party_dir=Path(temp_dir),
                python_exe=Path("/tmp/python"),
                wheelhouse=Path(temp_dir) / "wheelhouse",
                env={},
                installer="pip",
                install_attempts=3,
            )

        self.assertEqual(
            result,
            {
                "status": "skipped",
                "reason": "not_requested",
                "dir": "",
                "commit": "",
                "diff": "not_requested",
            },
        )

    def test_ensure_hapray_tools_layout_creates_compatibility_links(self) -> None:
        bootstrap_impl = load_module("bootstrap_impl", "src/arkts_migration_visualizer/deps/bootstrap_impl.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir) / "ArkAnalyzer-HapRay"
            workspace = repo_root / "perf_testing"
            dist_tools = repo_root / "dist" / "tools"
            source_tools = repo_root / "tools"

            (dist_tools / "sa-cmd").mkdir(parents=True)
            (dist_tools / "trace_streamer_binary").mkdir(parents=True)
            (dist_tools / "web").mkdir(parents=True)
            (dist_tools / "xvm").mkdir(parents=True)
            (dist_tools / "hilogtool").mkdir(parents=True)
            (source_tools / "static_analyzer").mkdir(parents=True)
            workspace.mkdir(parents=True)

            layout = bootstrap_impl.ensure_hapray_tools_layout(repo_root, workspace)

            self.assertTrue((source_tools / "sa-cmd").exists())
            self.assertTrue((source_tools / "trace_streamer_binary").exists())
            self.assertTrue((dist_tools / "static_analyzer").exists())
            self.assertTrue((workspace / "tools").exists())
            self.assertIn("workspace_tools", layout)
            self.assertIn("static_analyzer", layout)

    def test_determine_hapray_runtime_patch_status_prefers_native_windows_launcher(self) -> None:
        bootstrap_impl = load_module("bootstrap_impl", "src/arkts_migration_visualizer/deps/bootstrap_impl.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            launcher_path = Path(temp_dir) / "hapray-sa-cmd.exe"
            launcher_path.write_text("stub", encoding="utf-8")

            with mock.patch.object(bootstrap_impl.os, "name", "nt"):
                patch_info = bootstrap_impl.determine_hapray_runtime_patch_info(
                    launcher_path,
                    [r"C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe"],
                )
                self.assertEqual(patch_info["status"], "not_needed")
                self.assertEqual(patch_info["reason"], "native_launcher_present")
                self.assertEqual(
                    patch_info["launcherCompiler"],
                    r"C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe",
                )
                self.assertEqual(
                    bootstrap_impl.determine_hapray_runtime_patch_status(launcher_path),
                    "not_needed",
                )
                self.assertEqual(
                    bootstrap_impl.determine_hapray_runtime_patch_status(None),
                    "required",
                )

    def test_find_windows_launcher_compiler_checks_common_framework_paths(self) -> None:
        bootstrap_impl = load_module("bootstrap_impl", "src/arkts_migration_visualizer/deps/bootstrap_impl.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            windir = Path(temp_dir) / "Windows"
            program_files = str(Path(temp_dir) / "Program Files")
            program_files_x86 = str(Path(temp_dir) / "Program Files (x86)")
            compiler_path = windir / "Microsoft.NET" / "Framework64" / "v4.0.30319" / "csc.exe"
            compiler_path.parent.mkdir(parents=True)
            compiler_path.write_text("stub", encoding="utf-8")

            with mock.patch.object(bootstrap_impl.os, "name", "nt"), mock.patch.object(
                bootstrap_impl.shutil,
                "which",
                return_value=None,
            ), mock.patch.dict(
                bootstrap_impl.os.environ,
                {
                    "SystemRoot": str(windir),
                    "WINDIR": str(windir),
                    "ProgramFiles": program_files,
                    "ProgramFiles(x86)": program_files_x86,
                },
                clear=False,
            ):
                self.assertEqual(
                    bootstrap_impl.find_windows_launcher_compiler(),
                    [str(compiler_path)],
                )

    def test_resolve_executable_checks_macos_nvm_locations_for_node(self) -> None:
        bootstrap_impl = load_module("bootstrap_impl", "src/arkts_migration_visualizer/deps/bootstrap_impl.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            home_dir = Path(temp_dir) / "home"
            node_path = home_dir / ".nvm" / "versions" / "node" / "v22.3.0" / "bin" / "node"
            node_path.parent.mkdir(parents=True)
            node_path.write_text("node", encoding="utf-8")

            with mock.patch.object(bootstrap_impl.os, "name", "posix"), mock.patch.object(
                bootstrap_impl.Path,
                "home",
                return_value=home_dir,
            ), mock.patch.object(
                bootstrap_impl.shutil,
                "which",
                return_value=None,
            ):
                self.assertEqual(
                    bootstrap_impl.resolve_executable("node", env={"PATH": ""}),
                    str(node_path),
                )

    def test_find_hapray_python_checks_macos_homebrew_candidates(self) -> None:
        bootstrap_impl = load_module("bootstrap_impl", "src/arkts_migration_visualizer/deps/bootstrap_impl.py")
        homebrew_python = "opt/homebrew/bin/python3.11"

        def fake_parse_python_version(candidate: str) -> Optional[Tuple[int, int, int]]:
            if candidate == homebrew_python:
                return (3, 11, 9)
            return None

        with mock.patch.object(bootstrap_impl.sys, "platform", "darwin"), mock.patch.object(
            bootstrap_impl,
            "local_python_candidates",
            return_value=[],
        ), mock.patch.object(
            bootstrap_impl,
            "platform_python_candidates",
            return_value=[homebrew_python],
        ), mock.patch.object(
            bootstrap_impl,
            "parse_python_version",
            side_effect=fake_parse_python_version,
        ):
            self.assertEqual(
                bootstrap_impl.find_hapray_python(None, minimum=(3, 10)),
                (homebrew_python, "3.11.9"),
            )

    def test_npm_dependency_install_command_prefers_ci_with_package_lock(self) -> None:
        bootstrap_impl = load_module("bootstrap_impl", "src/arkts_migration_visualizer/deps/bootstrap_impl.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            project_dir = Path(temp_dir)
            (project_dir / "package-lock.json").write_text("{}", encoding="utf-8")

            command = bootstrap_impl.npm_dependency_install_command(project_dir, {})

            self.assertEqual(Path(command[0]).name, "npm")
            self.assertEqual(command[1], "ci")
            self.assertIn("--ignore-scripts", command)
            self.assertIn("--workspaces=false", command)

    def test_npm_script_env_removes_install_time_workspace_flags(self) -> None:
        bootstrap_impl = load_module("bootstrap_impl", "src/arkts_migration_visualizer/deps/bootstrap_impl.py")

        env = bootstrap_impl.npm_script_env(
            {
                "npm_config_workspaces": "false",
                "npm_config_include_workspace_root": "false",
                "npm_config_cache": "/tmp/npm-cache",
            }
        )

        self.assertNotIn("npm_config_workspaces", env)
        self.assertNotIn("npm_config_include_workspace_root", env)
        self.assertEqual(env["npm_config_cache"], "/tmp/npm-cache")

    def test_ensure_static_analyzer_dependency_tree_retries_when_ajv_codegen_is_missing(self) -> None:
        bootstrap_impl = load_module("bootstrap_impl", "src/arkts_migration_visualizer/deps/bootstrap_impl.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            analyzer_dir = Path(temp_dir) / "tools" / "static_analyzer"
            (analyzer_dir / "package-lock.json").parent.mkdir(parents=True, exist_ok=True)
            (analyzer_dir / "package-lock.json").write_text("{}", encoding="utf-8")
            for path in (
                analyzer_dir / "node_modules" / "copy-webpack-plugin" / "dist" / "index.js",
                analyzer_dir / "node_modules" / "schema-utils" / "dist" / "validate.js",
                analyzer_dir / "node_modules" / "ajv-keywords" / "dist" / "index.js",
            ):
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text("stub", encoding="utf-8")

            commands = []

            def fake_run_cmd(cmd, cwd=None, env=None, capture=False):
                commands.append(cmd)
                repaired = analyzer_dir / "node_modules" / "ajv" / "dist" / "compile" / "codegen" / "index.js"
                repaired.parent.mkdir(parents=True, exist_ok=True)
                repaired.write_text("stub", encoding="utf-8")
                return mock.Mock(returncode=0, stdout="")

            with mock.patch.object(bootstrap_impl, "run_cmd", side_effect=fake_run_cmd):
                bootstrap_impl.ensure_static_analyzer_dependency_tree(analyzer_dir, {}, fetch_retries=0)

            self.assertTrue(commands)
            self.assertEqual(Path(commands[0][0]).name, "npm")
            self.assertEqual(commands[0][1], "ci")

    def test_ensure_static_analyzer_dependency_tree_repairs_nested_ajv_without_retry(self) -> None:
        bootstrap_impl = load_module("bootstrap_impl", "src/arkts_migration_visualizer/deps/bootstrap_impl.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            analyzer_dir = Path(temp_dir) / "tools" / "static_analyzer"
            for path in (
                analyzer_dir / "node_modules" / "copy-webpack-plugin" / "dist" / "index.js",
                analyzer_dir / "node_modules" / "schema-utils" / "dist" / "validate.js",
                analyzer_dir / "node_modules" / "ajv-keywords" / "dist" / "index.js",
                analyzer_dir / "node_modules" / "schema-utils" / "node_modules" / "ajv" / "dist" / "compile" / "codegen" / "index.js",
            ):
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text("stub", encoding="utf-8")

            with mock.patch.object(bootstrap_impl, "run_cmd") as run_cmd_mock:
                bootstrap_impl.ensure_static_analyzer_dependency_tree(analyzer_dir, {}, fetch_retries=0)

            self.assertFalse(run_cmd_mock.called)
            self.assertTrue(
                (analyzer_dir / "node_modules" / "ajv" / "dist" / "compile" / "codegen" / "index.js").is_file()
            )

    def test_ensure_windows_cmd_script_writes_crlf_without_path_write_text_newline(self) -> None:
        bootstrap_impl = load_module("bootstrap_impl", "src/arkts_migration_visualizer/deps/bootstrap_impl.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            script_path = Path(temp_dir) / "hapray-sa-cmd.cmd"

            bootstrap_impl.ensure_windows_cmd_script(script_path, "@echo off\nnode test.js %*\n")

            self.assertEqual(script_path.read_bytes(), b"@echo off\r\nnode test.js %*\r\n")

    def test_ensure_windows_hapray_launcher_writes_legacy_compatible_source(self) -> None:
        bootstrap_impl = load_module("bootstrap_impl", "src/arkts_migration_visualizer/deps/bootstrap_impl.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            dist_dir = Path(temp_dir)
            compiler_cmd = [r"C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe"]

            def fake_run_cmd(cmd, cwd=None, env=None, capture=False):
                (dist_dir / "hapray-sa-cmd.exe").write_text("stub", encoding="utf-8")
                return mock.Mock(returncode=0, stdout="")

            with mock.patch.object(bootstrap_impl.os, "name", "nt"), mock.patch.object(
                bootstrap_impl,
                "find_windows_launcher_compiler",
                return_value=compiler_cmd,
            ), mock.patch.object(
                bootstrap_impl,
                "run_cmd",
                side_effect=fake_run_cmd,
            ):
                launcher_path, used_compiler = bootstrap_impl.ensure_windows_hapray_launcher(dist_dir)

            source_text = (dist_dir / "hapray-sa-cmd-launcher.cs").read_text(encoding="utf-8")
            self.assertEqual(launcher_path, dist_dir / "hapray-sa-cmd.exe")
            self.assertEqual(used_compiler, compiler_cmd)
            self.assertIn('return "\\"" + value.Replace("\\\\", "\\\\\\\\").Replace("\\"", "\\\\\\"") + "\\"";', source_text)
            self.assertIn("AppDomain.CurrentDomain.BaseDirectory", source_text)
            self.assertNotIn('$"', source_text)
            self.assertNotIn("AppContext.BaseDirectory", source_text)

    def test_ensure_static_analyzer_root_dependency_links_creates_repo_root_aliases(self) -> None:
        bootstrap_impl = load_module("bootstrap_impl", "src/arkts_migration_visualizer/deps/bootstrap_impl.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir) / "ArkAnalyzer-HapRay"
            analyzer_dir = repo_root / "tools" / "static_analyzer"
            (analyzer_dir / "node_modules" / "bjc").mkdir(parents=True)
            (analyzer_dir / "node_modules" / "sql.js").mkdir(parents=True)

            links = bootstrap_impl.ensure_static_analyzer_root_dependency_links(repo_root, analyzer_dir)

            self.assertTrue((repo_root / "node_modules" / "bjc").exists())
            self.assertTrue((repo_root / "node_modules" / "sql.js").exists())
            self.assertEqual(links["bjc"], str(repo_root / "node_modules" / "bjc"))
            self.assertEqual(links["sql.js"], str(repo_root / "node_modules" / "sql.js"))

    def test_build_static_analyzer_assets_uses_generated_wrappers_and_prod_tsconfig(self) -> None:
        bootstrap_impl = load_module("bootstrap_impl", "src/arkts_migration_visualizer/deps/bootstrap_impl.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir) / "ArkAnalyzer-HapRay"
            analyzer_dir = repo_root / "tools" / "static_analyzer"
            bootstrap_dir = repo_root / "perf_testing" / ".bootstrap"
            tsc_entry = analyzer_dir / "node_modules" / "typescript" / "bin" / "tsc"
            webpack_entry = analyzer_dir / "node_modules" / "webpack-cli" / "bin" / "cli.js"
            for path in (
                analyzer_dir / "node_modules" / "bjc",
                analyzer_dir / "node_modules" / "sql.js",
                tsc_entry.parent,
                webpack_entry.parent,
            ):
                path.mkdir(parents=True, exist_ok=True)
            tsc_entry.write_text("tsc", encoding="utf-8")
            webpack_entry.write_text("webpack", encoding="utf-8")
            (analyzer_dir / "webpack.config.js").write_text("module.exports = { module: { rules: [ { test: /\\\\.tsx?$/, use: 'ts-loader' } ] } };", encoding="utf-8")
            (analyzer_dir / "tsconfig.prod.json").write_text("{}", encoding="utf-8")

            commands = []

            def fake_tool_cmd(name, *args, env=None):
                return [name, *args]

            def fake_run_cmd(cmd, cwd=None, env=None, capture=False):
                commands.append((cmd, cwd))
                return mock.Mock(returncode=0, stdout="")

            with mock.patch.object(bootstrap_impl, "tool_cmd", side_effect=fake_tool_cmd), mock.patch.object(
                bootstrap_impl,
                "run_cmd",
                side_effect=fake_run_cmd,
            ):
                details = bootstrap_impl.build_static_analyzer_assets(repo_root, analyzer_dir, bootstrap_dir, {})

            tsconfig_wrapper = bootstrap_dir / "tsconfig.static_analyzer.bootstrap.json"
            wrapper_path = bootstrap_dir / "webpack.static_analyzer.bootstrap.cjs"
            tsconfig_wrapper_text = tsconfig_wrapper.read_text(encoding="utf-8")
            wrapper_text = wrapper_path.read_text(encoding="utf-8")
            self.assertEqual(commands[0][0], ["node", str(tsc_entry), "-p", str(tsconfig_wrapper)])
            self.assertEqual(commands[0][1], analyzer_dir)
            self.assertEqual(commands[1][0], ["node", str(webpack_entry), "--config", str(wrapper_path)])
            self.assertEqual(commands[1][1], analyzer_dir)
            self.assertIn(str(analyzer_dir / "tsconfig.prod.json"), tsconfig_wrapper_text)
            self.assertIn(str(analyzer_dir / "src"), tsconfig_wrapper_text)
            self.assertIn("defineArrayMethod('toSorted'", wrapper_text)
            self.assertIn("defineArrayMethod('toReversed'", wrapper_text)
            self.assertIn("defineArrayMethod('toSpliced'", wrapper_text)
            self.assertIn("defineArrayMethod('with'", wrapper_text)
            self.assertIn("configFile", wrapper_text)
            self.assertIn(str(tsconfig_wrapper), wrapper_text)
            self.assertEqual(details["tsconfigWrapper"], str(tsconfig_wrapper))
            self.assertEqual(details["webpackWrapper"], str(wrapper_path))
            self.assertEqual(details["rootNodeModuleBjc"], str(repo_root / "node_modules" / "bjc"))
            self.assertEqual(details["rootNodeModuleSqlJs"], str(repo_root / "node_modules" / "sql.js"))

    def test_determine_hapray_runtime_patch_info_records_fallback_reasons(self) -> None:
        bootstrap_impl = load_module("bootstrap_impl", "src/arkts_migration_visualizer/deps/bootstrap_impl.py")

        with mock.patch.object(bootstrap_impl.os, "name", "nt"):
            missing_compiler = bootstrap_impl.determine_hapray_runtime_patch_info(None, None)
            self.assertEqual(missing_compiler["status"], "required")
            self.assertEqual(missing_compiler["reason"], "compiler_not_found")
            self.assertEqual(missing_compiler["launcherCompiler"], "")

            compile_no_output = bootstrap_impl.determine_hapray_runtime_patch_info(
                None,
                [r"C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe"],
            )
            self.assertEqual(compile_no_output["status"], "required")
            self.assertEqual(compile_no_output["reason"], "launcher_not_created_after_compile")
            self.assertEqual(
                compile_no_output["launcherCompiler"],
                r"C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe",
            )

        with mock.patch.object(bootstrap_impl.os, "name", "posix"):
            non_windows = bootstrap_impl.determine_hapray_runtime_patch_info(None, None)
            self.assertEqual(non_windows["status"], "not_needed")
            self.assertEqual(non_windows["reason"], "non_windows")
            self.assertEqual(non_windows["launcherCompiler"], "")

    def test_build_hapray_env_no_longer_exports_ext_tools_path(self) -> None:
        perf_runner = load_module("perf_runner", "src/arkts_migration_visualizer/collect/perf_runner.py")
        with tempfile.TemporaryDirectory() as temp_dir:
            source_root = Path(temp_dir) / "ArkAnalyzer-HapRay"
            workspace = source_root / "perf_testing"
            tools_root = source_root / "tools"
            (tools_root / "sa-cmd").mkdir(parents=True)
            (tools_root / "trace_streamer_binary").mkdir(parents=True)
            (tools_root / "web").mkdir(parents=True)
            (tools_root / "xvm").mkdir(parents=True)
            (tools_root / "hilogtool").mkdir(parents=True)
            (tools_root / "web" / "report_template.html").write_text("report", encoding="utf-8")
            (tools_root / "web" / "hiperf_report_template.html").write_text("hiperf", encoding="utf-8")
            (tools_root / "hilogtool" / "hilogtool").write_text("bin", encoding="utf-8")
            workspace.mkdir(parents=True)

            fake_node = source_root / "node" / "bin" / "node"
            fake_node.parent.mkdir(parents=True)
            fake_node.write_text("node", encoding="utf-8")

            fake_hdc = source_root / "bin" / "hdc"
            fake_hdc.parent.mkdir(parents=True, exist_ok=True)
            fake_hdc.write_text("hdc", encoding="utf-8")

            with mock.patch.object(perf_runner, "find_node_executable", return_value=fake_node), mock.patch.object(
                perf_runner,
                "find_hdc_executable",
                return_value=fake_hdc,
            ):
                env = perf_runner.build_hapray_env(source_root, {}, workspace)

            self.assertNotIn("HAPRAY_EXT_TOOLS_PATH", env)
            self.assertIn(str(fake_node.parent), env.get("PATH", ""))


if __name__ == "__main__":
    unittest.main()
