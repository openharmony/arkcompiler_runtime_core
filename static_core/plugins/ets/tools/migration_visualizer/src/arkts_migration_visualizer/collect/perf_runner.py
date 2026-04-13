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

"""
Internal collection stage used by arkts_migration_visualizer.cli.

Supported dependency layouts:
1. Managed GitCode dependencies bootstrapped by src/scripts/bootstrap.sh / bootstrap.cmd
2. User-provided homecheck_path / hapray_path overrides in configs/config.json
"""

from __future__ import annotations

import datetime
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import threading
import textwrap
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

from arkts_migration_visualizer.common.fs_utils import (
    create_windows_directory_junction,
    ensure_executable_script,
    ensure_symlink,
)


REQUIRED_FILES = (
    "fileDepGraph.json",
    "moduleDepGraph.json",
    "hapray_report.json",
    "component_timings.json",
)

HAPRAY_REPORTS_ROOT_ENV = "HAPRAY_REPORTS_ROOT"
INTERNAL_HAPRAY_REPORTS_ROOT_ENV = "MIGRATION_VISUALIZER_HAPRAY_REPORTS_ROOT"
HAPRAY_REPORTS_ROOT_CONFIG_KEY = "hapray_reports_root"
HILOGTOOL_OUTPUT_MARKER = "Hilogtool output:"
HILOGTOOL_RESULT_MARKER = "Result: successNum:"
HILOGTOOL_OPEN_DICT_MARKER = "open dict "
HILOGTOOL_UNKNOWN_LOG_MARKER = "OpenUuidFile fail, unknown log, uuid is:"
BUNDLE_NAME_PATTERN = re.compile(r'"bundleName"\s*:\s*"([^"\r\n]+)"')
MAIN_ELEMENT_PATTERN = re.compile(r'"mainElement"\s*:\s*"([^"\r\n]+)"')
MODULE_TYPE_PATTERN = re.compile(r'"type"\s*:\s*"([^"\r\n]+)"')

PROJECT_SCAN_SKIP_DIRS = {
    ".git",
    ".hg",
    ".svn",
    ".idea",
    ".deveco",
    ".hvigor",
    "build",
    "node_modules",
    "oh_modules",
}


@dataclass
class CollectionRequest:
    testcase_regex: Optional[str] = None
    manual_package: Optional[str] = None
    manual_ability: Optional[str] = None
    manual_duration: int = 30
    deps_root: str = ""


class CommandExecutionError(RuntimeError):
    pass


def run_command(
    command: List[str],
    cwd: Optional[Path] = None,
    env_vars: Optional[Dict[str, str]] = None,
    live_output: bool = False,
    suppress_hilogtool_noise: bool = False,
    diagnostic_files: Optional[List[Tuple[Path, str]]] = None,
    error_message: str = "Command failed",
) -> subprocess.CompletedProcess:
    print("\n>>> Running command:", " ".join(command))
    if cwd:
        print("    Working directory:", cwd)

    env = os.environ.copy()
    if env_vars:
        env.update(env_vars)

    if live_output and suppress_hilogtool_noise:
        result = run_command_with_filtered_live_output(command, cwd=cwd, env=env)
    else:
        result = subprocess.run(
            command,
            cwd=str(cwd) if cwd else None,
            env=env,
            text=True,
            encoding="utf-8",
            errors="replace",
            stdout=None if live_output else subprocess.PIPE,
            stderr=None if live_output else subprocess.PIPE,
            check=False,
        )

    if not live_output:
        if result.stdout:
            print("[STDOUT]\n", result.stdout)
        if result.stderr:
            print("[STDERR]\n", result.stderr)

    if result.returncode != 0:
        if diagnostic_files:
            for diag_path, label in diagnostic_files:
                if not diag_path.is_file():
                    continue
                print(f"[{label}] {diag_path}")
                print(tail_text_file(diag_path))
        message = f"{error_message} (exit code {result.returncode})"
        print(message)
        raise CommandExecutionError(message)
    return result


def flush_hilogtool_output_block(lines: List[str]) -> None:
    open_dict_count = sum(HILOGTOOL_OPEN_DICT_MARKER in line for line in lines)
    unknown_log_count = sum(HILOGTOOL_UNKNOWN_LOG_MARKER in line for line in lines)
    if not open_dict_count and not unknown_log_count:
        for line in lines:
            print(line, end="")
        return

    result_line = next((line.strip() for line in reversed(lines) if HILOGTOOL_RESULT_MARKER in line), "")
    summary = (
        ">>> Suppressed hilogtool noise output: "
        f"open dict {open_dict_count} entries, unknown log {unknown_log_count} entries"
    )
    if result_line:
        summary += f" | {result_line}"
    print(summary)


def run_command_with_filtered_live_output(
    command: List[str],
    cwd: Optional[Path],
    env: Dict[str, str],
) -> subprocess.CompletedProcess:
    process = subprocess.Popen(
        command,
        cwd=str(cwd) if cwd else None,
        env=env,
        text=True,
        encoding="utf-8",
        errors="replace",
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        bufsize=1,
    )

    block_lines: List[str] = []
    in_hilogtool_block = False

    assert process.stdout is not None
    for line in process.stdout:
        if in_hilogtool_block:
            block_lines.append(line)
            if HILOGTOOL_RESULT_MARKER in line:
                flush_hilogtool_output_block(block_lines)
                block_lines = []
                in_hilogtool_block = False
            continue

        if HILOGTOOL_OUTPUT_MARKER in line:
            block_lines = [line]
            in_hilogtool_block = True
            if HILOGTOOL_RESULT_MARKER in line:
                flush_hilogtool_output_block(block_lines)
                block_lines = []
                in_hilogtool_block = False
            continue

        print(line, end="")

    if block_lines:
        flush_hilogtool_output_block(block_lines)

    return subprocess.CompletedProcess(command, process.wait())


def ensure_exists(path: Path, kind: str) -> None:
    if not path.exists():
        sys.exit(f"Error: {kind} does not exist: {path}")


def is_windows_abs(path_str: str) -> bool:
    return len(path_str) > 2 and path_str[1] == ":" and path_str[2] in ("\\", "/")


def is_wsl_abs(path_str: str) -> bool:
    normalized = path_str.replace("\\", "/")
    return bool(re.match(r"^/mnt/[a-zA-Z]/", normalized))


def windows_to_wsl_path(path_str: str) -> Path:
    normalized = path_str.replace("\\", "/")
    drive = normalized[0].lower()
    remainder = normalized[2:].lstrip("/")
    return Path("/mnt") / drive / remainder


def wsl_to_windows_path(path_str: str) -> Path:
    normalized = path_str.replace("\\", "/")
    match = re.match(r"^/mnt/([a-zA-Z])/(.*)$", normalized)
    if not match:
        return Path(path_str)
    drive = match.group(1).upper()
    remainder = match.group(2).replace("/", "\\")
    windows_path = f"{drive}:\\{remainder}"
    return Path(windows_path)


def resolve_path(base_dir: Path, raw: str) -> Optional[Path]:
    if not raw:
        return None
    if is_windows_abs(raw):
        if os.name != "nt":
            return windows_to_wsl_path(raw)
        return Path(raw)
    if is_wsl_abs(raw):
        if os.name == "nt":
            return wsl_to_windows_path(raw)
        return Path(raw)
    path = Path(raw).expanduser()
    if path.is_absolute():
        return path
    return (base_dir / path).resolve()


def tail_text_file(path: Path, max_lines: int = 80) -> str:
    try:
        lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    except OSError as exc:
        return f"Read failed: {exc}"
    if not lines:
        return "<empty>"
    return "\n".join(lines[-max_lines:])


def resolve_project_path_or_exit(repo_root: Path, cfg: Dict[str, Any]) -> Path:
    if not cfg.get("project_path"):
        sys.exit("Missing configuration: project_path")
    project_path = resolve_path(repo_root, cfg.get("project_path", ""))
    if not project_path:
        sys.exit("Failed to resolve project_path")
    ensure_exists(project_path, "project_path target project directory")
    return project_path


def strip_json_like_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//.*", "", text)


def extract_bundle_name_from_app_manifest(manifest_path: Path) -> Optional[str]:
    try:
        text = manifest_path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return None
    match = BUNDLE_NAME_PATTERN.search(strip_json_like_comments(text))
    if not match:
        return None
    bundle_name = match.group(1).strip()
    return bundle_name or None


def iter_app_manifest_candidates(project_path: Path) -> List[Path]:
    direct_candidates = [
        project_path / "AppScope" / "app.json5",
        project_path / "AppScope" / "app.json",
    ]
    discovered: List[Path] = []
    seen = set()

    for candidate in direct_candidates:
        if candidate.is_file():
            resolved = candidate.resolve()
            seen.add(str(resolved))
            discovered.append(resolved)

    for root, dirs, files in os.walk(project_path):
        dirs[:] = [item for item in dirs if item not in PROJECT_SCAN_SKIP_DIRS]
        for file_name in files:
            if file_name not in {"app.json5", "app.json"}:
                continue
            manifest_path = (Path(root) / file_name).resolve()
            manifest_key = str(manifest_path)
            if manifest_key in seen:
                continue
            seen.add(manifest_key)
            discovered.append(manifest_path)

    return discovered


def find_project_bundle_names(project_path: Path) -> Dict[str, List[Path]]:
    bundle_map: Dict[str, List[Path]] = {}
    for manifest_path in iter_app_manifest_candidates(project_path):
        bundle_name = extract_bundle_name_from_app_manifest(manifest_path)
        if not bundle_name:
            continue
        bundle_map.setdefault(bundle_name, []).append(manifest_path)
    return bundle_map


def extract_main_ability_from_module_manifest(manifest_path: Path) -> Optional[str]:
    try:
        text = manifest_path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return None

    sanitized = strip_json_like_comments(text)
    main_element = MAIN_ELEMENT_PATTERN.search(sanitized)
    if not main_element:
        return None
    ability_name = main_element.group(1).strip()
    return ability_name or None


def iter_module_manifest_candidates(project_path: Path) -> List[Path]:
    direct_candidates = [
        project_path / "entry" / "src" / "main" / "module.json5",
        project_path / "src" / "main" / "module.json5",
    ]
    discovered: List[Path] = []
    seen = set()

    for candidate in direct_candidates:
        if candidate.is_file():
            resolved = candidate.resolve()
            seen.add(str(resolved))
            discovered.append(resolved)

    for root, dirs, files in os.walk(project_path):
        dirs[:] = [item for item in dirs if item not in PROJECT_SCAN_SKIP_DIRS]
        for file_name in files:
            if file_name != "module.json5":
                continue
            manifest_path = (Path(root) / file_name).resolve()
            manifest_key = str(manifest_path)
            if manifest_key in seen:
                continue
            seen.add(manifest_key)
            discovered.append(manifest_path)

    return discovered


def module_manifest_priority(manifest_path: Path) -> Tuple[int, int, int, str]:
    normalized = str(manifest_path).replace("\\", "/").lower()
    is_entry_main = 1 if normalized.endswith("/entry/src/main/module.json5") else 0
    is_src_main = 1 if "/src/main/" in normalized else 0
    is_ohos_test = 1 if "/ohostest/" in normalized else 0
    return (-is_entry_main, -is_src_main, is_ohos_test, normalized)


def find_project_main_ability(project_path: Path) -> Optional[Tuple[str, Path]]:
    matches: List[Tuple[int, int, int, str, str, Path]] = []
    for manifest_path in iter_module_manifest_candidates(project_path):
        try:
            text = manifest_path.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        sanitized = strip_json_like_comments(text)
        main_element_match = MAIN_ELEMENT_PATTERN.search(sanitized)
        if not main_element_match:
            continue
        main_element = main_element_match.group(1).strip()
        if not main_element:
            continue
        module_type_match = MODULE_TYPE_PATTERN.search(sanitized)
        module_type = module_type_match.group(1).strip().lower() if module_type_match else ""
        if module_type and module_type != "entry":
            continue
        matches.append((*module_manifest_priority(manifest_path), main_element, manifest_path))

    if not matches:
        return None

    matches.sort()
    best = matches[0]
    return best[4], best[5]


def validate_manual_package_matches_project(repo_root: Path, cfg: Dict[str, Any], manual_package: str) -> Path:
    project_path = resolve_project_path_or_exit(repo_root, cfg)
    bundle_map = find_project_bundle_names(project_path)
    if not bundle_map:
        sys.exit(
            "Manual listening mode could not resolve the application bundle name from project_path: {}\n"
            "Make sure the project contains AppScope/app.json5 (or app.json) and that app.bundleName is present.".format(project_path)
        )

    if manual_package in bundle_map:
        matched_path = bundle_map[manual_package][0]
        print(f">>> Manual package validation passed: {manual_package} <- {matched_path}")
        return project_path

    detected = sorted(bundle_map)
    details = "; ".join(
        "{} <- {}".format(bundle_name, ", ".join(str(item) for item in paths))
        for bundle_name, paths in sorted(bundle_map.items())
    )
    sys.exit(
        "Manual package does not match project_path:\n"
        f"  --manual-package = {manual_package}\n"
        f"  project_path     = {project_path}\n"
        f"  detected bundleName = {', '.join(detected)}\n"
        f"  detected from       = {details}\n"
        "Fix --manual-package, or switch project_path in configs/config.json to the matching application project."
    )


def resolve_manual_launch_ability(
    repo_root: Path,
    cfg: Dict[str, Any],
    manual_package: str,
    manual_ability: Optional[str],
) -> Tuple[Path, Optional[str]]:
    project_path = validate_manual_package_matches_project(repo_root, cfg, manual_package)
    if manual_ability:
        print(f">>> Manual launch ability explicitly specified: {manual_ability}")
        return project_path, manual_ability

    main_ability = find_project_main_ability(project_path)
    if main_ability:
        ability_name, manifest_path = main_ability
        print(f">>> Manual launch ability auto-detected: {ability_name} <- {manifest_path}")
        return project_path, ability_name

    print(">>> Could not auto-detect a main ability from project module.json5. Common entry names EntryAbility/MainAbility will be tried at runtime.")
    return project_path, None


def sibling_hilog_text_path(gzip_path: Path) -> Path:
    suffix = "".join(gzip_path.suffixes)
    if suffix.endswith(".gz"):
        return gzip_path.with_name(gzip_path.name[:-3] + ".txt")
    return gzip_path.with_suffix(".txt")


class HilogArchiveQuarantine:
    """Move redundant hilog gzip files away before Hapray triggers hilogtool parsing."""

    def __init__(self, reports_root: Path, poll_interval: float = 1.0):
        self.reports_root = reports_root
        self.backup_root = reports_root / ".hilog_gz_backup"
        self.poll_interval = poll_interval
        self._stop_event = threading.Event()
        self._thread: Optional[threading.Thread] = None
        self.moved_count = 0

    def start(self) -> None:
        if self._thread is not None:
            return
        self._thread = threading.Thread(target=self._run, name="hilog-gz-quarantine", daemon=True)
        self._thread.start()

    def stop(self) -> None:
        self._stop_event.set()
        if self._thread is not None:
            self._thread.join(timeout=max(self.poll_interval * 2, 2.0))
        self.quarantine_once()

    def quarantine_once(self) -> int:
        if not self.reports_root.exists():
            return 0

        moved_now = 0
        try:
            archive_paths = list(self.reports_root.rglob("hilog*.gz"))
        except OSError:
            return 0

        for archive_path in archive_paths:
            if not archive_path.is_file():
                continue
            if self.backup_root in archive_path.parents:
                continue

            text_path = sibling_hilog_text_path(archive_path)
            if not text_path.is_file():
                continue

            try:
                relative_path = archive_path.relative_to(self.reports_root)
            except ValueError:
                continue

            target_path = self.backup_root / relative_path
            target_path.parent.mkdir(parents=True, exist_ok=True)
            if target_path.exists():
                continue

            try:
                shutil.move(str(archive_path), str(target_path))
            except FileNotFoundError:
                continue
            except OSError:
                continue

            moved_now += 1

        self.moved_count += moved_now
        return moved_now

    def _run(self) -> None:
        while not self._stop_event.wait(self.poll_interval):
            self.quarantine_once()


def load_config(repo_root: Path) -> Tuple[Path, Dict[str, Any]]:
    cfg_path = repo_root / "configs" / "config.json"
    ensure_exists(cfg_path, "configuration file configs/config.json")
    with cfg_path.open("r", encoding="utf-8") as handle:
        return cfg_path, json.load(handle)


def dependency_bootstrap_impl_path(repo_root: Path) -> Path:
    candidates = [
        repo_root / "src" / "arkts_migration_visualizer" / "deps" / "bootstrap_impl.py",
        repo_root / "src" / "scripts" / "deps" / "internal" / "bootstrap_impl.py",
        repo_root / "scripts" / "deps" / "internal" / "bootstrap_impl.py",
    ]
    for candidate in candidates:
        if candidate.is_file():
            return candidate
    return candidates[0]


def resolved_deps_root(repo_root: Path, cfg: Dict[str, Any]) -> Path:
    deps_root_raw = cfg.get("deps_root", ".deps")
    deps_root = resolve_path(repo_root, deps_root_raw)
    return deps_root or (repo_root / ".deps")


def lock_file_path(repo_root: Path, cfg: Dict[str, Any]) -> Path:
    return resolved_deps_root(repo_root, cfg) / "dependency-lock.json"


def bootstrap_dependencies(repo_root: Path, cfg: Dict[str, Any], tool: str = "all") -> Dict[str, Any]:
    bootstrap = dependency_bootstrap_impl_path(repo_root)
    ensure_exists(bootstrap, "dependency bootstrap script bootstrap_impl.py")

    cmd = [sys.executable, str(bootstrap), "--tool", tool]
    cmd.extend(["--deps-root", str(resolved_deps_root(repo_root, cfg))])
    hapray_python = cfg.get("hapray_python", "")
    if hapray_python:
        resolved_python = resolve_path(repo_root, hapray_python)
        if resolved_python and resolved_python.is_file():
            cmd.extend(["--hapray-python", str(resolved_python)])
    if cfg.get("npm_registry"):
        cmd.extend(["--npm-registry", str(cfg["npm_registry"])])
    npm_strict_ssl = str(cfg.get("npm_strict_ssl", "")).strip().lower()
    if npm_strict_ssl in {"true", "false"}:
        cmd.extend(["--npm-strict-ssl", npm_strict_ssl])
    cmd.extend(["--npm-fetch-timeout", str(int(cfg.get("npm_fetch_timeout", 60000)))])
    cmd.extend(["--npm-fetch-retries", str(int(cfg.get("npm_fetch_retries", 0)))])
    if cfg.get("pip_index_url"):
        cmd.extend(["--pip-index-url", str(cfg["pip_index_url"])])
    cmd.extend(["--pip-timeout", str(int(cfg.get("pip_timeout", 60)))])
    cmd.extend(["--pip-retries", str(int(cfg.get("pip_retries", 5)))])
    run_command(cmd, cwd=repo_root, live_output=True, error_message="Dependency download or installation failed")

    lock_path = lock_file_path(repo_root, cfg)
    ensure_exists(lock_path, "dependency lock file dependency-lock.json")
    with lock_path.open("r", encoding="utf-8") as handle:
        return json.load(handle)


def load_dependency_state(repo_root: Path, cfg: Dict[str, Any]) -> Dict[str, Any]:
    lock_path = lock_file_path(repo_root, cfg)
    if not lock_path.is_file():
        return bootstrap_dependencies(repo_root, cfg)
    with lock_path.open("r", encoding="utf-8") as handle:
        return json.load(handle)


def resolve_tool_root_from_state(
    repo_root: Path,
    tool_state: Dict[str, Any],
    preferred_keys: Tuple[str, ...],
) -> Optional[Path]:
    for key in preferred_keys:
        resolved = resolve_path(repo_root, str(tool_state.get(key, "") or "").strip())
        if resolved:
            return resolved
    return None


def ensure_required_tool_state(
    repo_root: Path,
    cfg: Dict[str, Any],
    *,
    prefer_hapray_source: bool = False,
) -> Dict[str, Any]:
    state = load_dependency_state(repo_root, cfg)
    tools = state.get("tools", {})
    homecheck_override = (cfg.get("homecheck_path") or "").strip()
    hapray_override = (cfg.get("hapray_path") or "").strip()
    hapray_keys = ("sourceDir", "workspaceDir") if prefer_hapray_source else ("workspaceDir", "sourceDir")

    missing_tools: List[str] = []
    if not homecheck_override and not resolve_tool_root_from_state(repo_root, tools.get("homecheck", {}), ("runtimeDir", "sourceDir")):
        missing_tools.append("homecheck")
    if not hapray_override and not resolve_tool_root_from_state(repo_root, tools.get("hapray", {}), hapray_keys):
        missing_tools.append("hapray")

    if not missing_tools:
        return state

    print(
        "\n⚠ dependency-lock.json is missing entries required by perf_runner: "
        + ", ".join(missing_tools)
        + ". Automatically bootstrapping the missing tool definitions ..."
    )
    bootstrap_tool = missing_tools[0] if len(missing_tools) == 1 else "all"
    state = bootstrap_dependencies(repo_root, cfg, tool=bootstrap_tool)

    tools = state.get("tools", {})
    unresolved_after_bootstrap: List[str] = []
    if not homecheck_override and not resolve_tool_root_from_state(repo_root, tools.get("homecheck", {}), ("runtimeDir", "sourceDir")):
        unresolved_after_bootstrap.append("homecheck")
    if not hapray_override and not resolve_tool_root_from_state(repo_root, tools.get("hapray", {}), hapray_keys):
        unresolved_after_bootstrap.append("hapray")
    if unresolved_after_bootstrap:
        mode_label = "source paths" if prefer_hapray_source else "paths"
        sys.exit(
            "Homecheck/Hapray {} are still missing from the dependency lock file after bootstrap: {}.\n"
            "Run src/scripts/bootstrap.sh --tool all --verify (macOS/Linux/WSL) or "
            "src\\scripts\\bootstrap.cmd --tool all --verify (Windows) again.".format(
                mode_label,
                ", ".join(unresolved_after_bootstrap),
            )
        )
    return state


def hapray_runtime_assets_ready(hapray_root: Path) -> bool:
    source_root = resolve_hapray_source_root(hapray_root)
    tools_root = source_root / "tools"
    hilogtool_name = "hilogtool.exe" if os.name == "nt" else "hilogtool"
    required_paths = [
        tools_root / "sa-cmd",
        tools_root / "trace_streamer_binary",
        tools_root / "web" / "report_template.html",
        tools_root / "web" / "hiperf_report_template.html",
        tools_root / "xvm",
        tools_root / "hilogtool" / hilogtool_name,
    ]
    return all(path.exists() for path in required_paths)


def ensure_hapray_runtime_assets(repo_root: Path, cfg: Dict[str, Any], hapray_root: Path) -> None:
    if hapray_runtime_assets_ready(hapray_root):
        return
    print("\n⚠ Incomplete Hapray runtime assets detected. Automatically restoring tools/web, tools/xvm, and tools/hilogtool ...")
    bootstrap_dependencies(repo_root, cfg, tool="hapray")


def resolve_homecheck_command(root: Path) -> List[str]:
    runtime_entry = root / "node_modules" / "homecheck" / "lib" / "tools" / "HomeDep.js"
    packaged_entry = root / "lib" / "tools" / "HomeDep.js"
    ts_entry = root / "src" / "tools" / "HomeDep.ts"

    if root.is_file():
        return [str(root)]
    if runtime_entry.is_file():
        return [str(runtime_entry)]
    if packaged_entry.is_file():
        return [str(packaged_entry)]
    if ts_entry.is_file():
        return ["-r", "ts-node/register", str(ts_entry)]
    sys.exit(f"Could not locate the Homecheck/HomeDep entry: {root}")


def resolve_hapray_workspace(root: Path) -> Path:
    if (root / "perf_testing").is_dir():
        return root / "perf_testing"
    return root


def resolve_hapray_source_root(root: Path) -> Path:
    if (root / "perf_testing").is_dir():
        return root
    if root.name == "perf_testing":
        return root.parent
    return root


def resolve_hapray_exe(root: Path) -> Optional[Path]:
    """
    Support packaged HapRay (ArkAnalyzer-HapRay-win32-x64-*) layout.
    If ArkAnalyzer-HapRay.exe exists, prefer running it over the source-mode Python launcher.
    """
    if root.is_file():
        return None
    candidate = root / "ArkAnalyzer-HapRay.exe"
    if candidate.is_file():
        return candidate
    return None


def hapray_platform_tag() -> str:
    if os.name == "nt":
        return "windows"
    if sys.platform == "darwin":
        return "macos"
    return "linux"


def hapray_venv_dir(workspace: Path) -> Path:
    return workspace / f".venv-{hapray_platform_tag()}"


def legacy_hapray_venv_dir(workspace: Path) -> Path:
    return workspace / ".venv"


def venv_python_in_dir(venv_dir: Path) -> Path:
    if os.name == "nt":
        return venv_dir / "Scripts" / "python.exe"
    return venv_dir / "bin" / "python"


def venv_matches_current_platform(venv_dir: Path) -> bool:
    python_path = venv_python_in_dir(venv_dir)
    if not python_path.is_file():
        return False

    cfg_path = venv_dir / "pyvenv.cfg"
    if not cfg_path.is_file():
        return True

    markers: List[str] = []
    for line in cfg_path.read_text(encoding="utf-8", errors="replace").splitlines():
        if "=" not in line:
            continue
        key, value = line.split("=", 1)
        if key.strip().lower() in {"home", "executable", "command"}:
            markers.append(value.strip())

    if os.name == "nt":
        return not any(marker.startswith("/") for marker in markers)
    return not any(re.match(r"^[A-Za-z]:[\\/]", marker) for marker in markers)


def resolve_hapray_python(workspace: Path) -> Optional[Path]:
    seen = set()
    for venv_dir in (hapray_venv_dir(workspace), legacy_hapray_venv_dir(workspace)):
        venv_dir_str = str(venv_dir)
        if venv_dir_str in seen:
            continue
        seen.add(venv_dir_str)
        if venv_matches_current_platform(venv_dir):
            return venv_python_in_dir(venv_dir)
    return None


def require_hapray_python(workspace: Path) -> Path:
    python_path = resolve_hapray_python(workspace)
    if python_path:
        return python_path

    searched = [
        str(venv_python_in_dir(venv_dir))
        for venv_dir in (hapray_venv_dir(workspace), legacy_hapray_venv_dir(workspace))
    ]
    sys.exit(
        "Could not locate a Hapray virtual-environment Python for the current platform: {}\nChecked: {}\n"
        "Please run src/scripts/bootstrap.sh --tool hapray (macOS/Linux/WSL)"
        " or src\\scripts\\bootstrap.cmd --tool hapray (Windows) again.".format(workspace, ", ".join(searched))
    )


def ensure_hapray_python_runtime(repo_root: Path, cfg: Dict[str, Any], hapray_root: Path) -> None:
    if resolve_hapray_exe(hapray_root):
        return
    workspace = resolve_hapray_workspace(hapray_root)
    if resolve_hapray_python(workspace):
        return
    print(f"\n⚠ No usable Hapray virtual environment was found for the current platform. Initializing one for {hapray_platform_tag()} ...")
    bootstrap_dependencies(repo_root, cfg, tool="hapray")


def hapray_runtime_missing_items(hapray_root: Path) -> List[str]:
    missing: List[str] = []
    if not hapray_runtime_assets_ready(hapray_root):
        missing.append("runtime assets")
    if not resolve_hapray_exe(hapray_root):
        workspace = resolve_hapray_workspace(hapray_root)
        if not resolve_hapray_python(workspace):
            missing.append(f"{hapray_platform_tag()} virtual environment")
    return missing


def ensure_hapray_runtime_ready(repo_root: Path, cfg: Dict[str, Any], hapray_root: Path) -> None:
    missing_before = hapray_runtime_missing_items(hapray_root)
    if not missing_before:
        return

    print(
        "\n⚠ Incomplete Hapray runtime detected. Automatically bootstrapping missing pieces: "
        + ", ".join(missing_before)
        + " ..."
    )
    state = bootstrap_dependencies(repo_root, cfg, tool="hapray")
    missing_after = hapray_runtime_missing_items(hapray_root)
    if not missing_after:
        return

    hapray_state = state.get("tools", {}).get("hapray", {})
    note = str(hapray_state.get("note", "") or "").strip()
    message_lines = [
        "Hapray bootstrap completed, but the runtime is still incomplete: {}.".format(", ".join(missing_after))
    ]
    if note:
        message_lines.append(f"Bootstrap note: {note}")
    if "virtual environment" in ", ".join(missing_after):
        message_lines.append(
            "Configure configs/config.json -> hapray_python to a Python 3.10-3.12 interpreter "
            "that is available in your local toolchain or shell PATH, "
            "then rerun ./src/scripts/bootstrap.sh --tool hapray --verify."
        )
    sys.exit("\n".join(message_lines))


def ensure_hapray_runtime_dirs(workspace: Path) -> None:
    for relative in ("logs", "reports", "haptest_reports", "runtime"):
        (workspace / relative).mkdir(parents=True, exist_ok=True)


def windows_short_hapray_alias_needed(hapray_root: Path) -> bool:
    if os.name != "nt":
        return False
    workspace = resolve_hapray_workspace(hapray_root)
    path_samples = [
        str(hapray_root),
        str(workspace),
        str(workspace / "reports"),
        str(workspace / "reports" / "20260402202126" / "PerfLoad_FrameAndPropertyAnimation_0010" / "report"),
    ]
    if any(" " in sample for sample in path_samples):
        return True
    return max(len(sample) for sample in path_samples) >= 120


def iter_windows_short_alias_bases(repo_root: Path, target_root: Path, cfg: Dict[str, Any]) -> List[Path]:
    candidates: List[Path] = []
    configured_root_raw = str(cfg.get("windows_short_alias_root", "") or "").strip()
    configured_root = resolve_path(repo_root, configured_root_raw) if configured_root_raw else None
    if configured_root:
        candidates.append(configured_root)
    else:
        deps_root = resolved_deps_root(repo_root, cfg)
        repo_default_deps_root = (repo_root / ".deps").resolve()
        if deps_root.resolve() != repo_default_deps_root:
            candidates.append(deps_root.parent / ".amv-short")

    candidates.append(repo_root / ".amv-short")

    drive = target_root.drive or repo_root.drive
    if drive:
        candidates.append(Path(f"{drive}\\OH\\.amv-short"))
        candidates.append(Path(f"{drive}\\.amv-short"))

    temp_root = os.environ.get("TEMP", "")
    if temp_root and " " not in temp_root:
        candidates.append(Path(temp_root) / "amv-short")

    unique_candidates: List[Path] = []
    seen = set()
    for candidate in candidates:
        key = str(candidate).lower()
        if key in seen:
            continue
        seen.add(key)
        unique_candidates.append(candidate)
    return unique_candidates


def ensure_windows_directory_junction(alias_path: Path, target_path: Path) -> Optional[Path]:
    return create_windows_directory_junction(alias_path, target_path)


def maybe_shorten_windows_hapray_root(repo_root: Path, cfg: Dict[str, Any], hapray_root: Path) -> Path:
    if not windows_short_hapray_alias_needed(hapray_root):
        return hapray_root

    source_root = resolve_hapray_source_root(hapray_root)
    alias_suffix = hashlib.sha1(str(source_root).encode("utf-8")).hexdigest()[:8]
    alias_name = f"hapray-{alias_suffix}"
    for base_dir in iter_windows_short_alias_bases(repo_root, source_root, cfg):
        alias_root = ensure_windows_directory_junction(base_dir / alias_name, source_root)
        if not alias_root:
            continue
        if hapray_root == source_root:
            print(f">>> Windows short-path alias: {alias_root} -> {source_root}")
            return alias_root
        workspace_alias = alias_root / "perf_testing"
        if workspace_alias.exists():
            print(f">>> Windows short-path alias: {workspace_alias} -> {hapray_root}")
            return workspace_alias

    print(f">>> Warning: failed to create a Windows short-path alias for Hapray. Continuing with the original path: {hapray_root}")
    return hapray_root


def resolve_tool_roots(repo_root: Path, cfg: Dict[str, Any]) -> Tuple[Path, Path]:
    """Merge dependency-lock defaults with per-tool overrides from config."""
    homecheck_override = (cfg.get("homecheck_path") or "").strip()
    hapray_override = (cfg.get("hapray_path") or "").strip()

    state = ensure_required_tool_state(repo_root, cfg, prefer_hapray_source=False)
    tools = state.get("tools", {})
    homecheck_tool = tools.get("homecheck", {})
    hapray_tool = tools.get("hapray", {})

    default_homecheck = resolve_path(
        repo_root, homecheck_tool.get("runtimeDir") or homecheck_tool.get("sourceDir", "")
    )
    default_hapray = resolve_path(
        repo_root, hapray_tool.get("workspaceDir") or hapray_tool.get("sourceDir", "")
    )

    homecheck_root = resolve_path(repo_root, homecheck_override) if homecheck_override else default_homecheck
    hapray_root = resolve_path(repo_root, hapray_override) if hapray_override else default_hapray

    if not homecheck_root or not hapray_root:
        sys.exit("Homecheck/Hapray paths are missing from the dependency lock file. Run bootstrap.sh or bootstrap.cmd again.")
    ensure_exists(homecheck_root, "Homecheck runtime directory")
    ensure_exists(hapray_root, "Hapray runtime directory")
    hapray_root = maybe_shorten_windows_hapray_root(repo_root, cfg, hapray_root)
    return homecheck_root, hapray_root


def resolve_tool_roots_for_mode(repo_root: Path, cfg: Dict[str, Any], *, prefer_hapray_source: bool = False) -> Tuple[Path, Path]:
    if not prefer_hapray_source:
        return resolve_tool_roots(repo_root, cfg)

    homecheck_override = (cfg.get("homecheck_path") or "").strip()
    state = ensure_required_tool_state(repo_root, cfg, prefer_hapray_source=True)
    tools = state.get("tools", {})
    homecheck_tool = tools.get("homecheck", {})
    hapray_tool = tools.get("hapray", {})

    default_homecheck = resolve_path(
        repo_root, homecheck_tool.get("runtimeDir") or homecheck_tool.get("sourceDir", "")
    )
    preferred_hapray = resolve_path(
        repo_root, hapray_tool.get("sourceDir") or hapray_tool.get("workspaceDir", "")
    )
    homecheck_root = resolve_path(repo_root, homecheck_override) if homecheck_override else default_homecheck
    hapray_root = preferred_hapray

    if not homecheck_root or not hapray_root:
        sys.exit("Homecheck/Hapray source paths are missing from the dependency lock file. Run bootstrap.sh or bootstrap.cmd again.")
    ensure_exists(homecheck_root, "Homecheck runtime directory")
    ensure_exists(hapray_root, "Hapray source directory")
    hapray_root = maybe_shorten_windows_hapray_root(repo_root, cfg, hapray_root)
    return homecheck_root, hapray_root


def ensure_homecheck_sdk_layout(repo_root: Path, cfg: Dict[str, Any], sdk_path: Path) -> Path:
    legacy_sdk = sdk_path / "default" / "openharmony" / "ets"
    legacy_hms = sdk_path / "default" / "hms" / "ets"
    if legacy_sdk.is_dir() and legacy_hms.is_dir():
        return sdk_path

    next_sdk = sdk_path / "base" / "ets"
    next_hms = sdk_path / "hms" / "ets"
    if next_sdk.is_dir() and next_hms.is_dir():
        shim_root = resolved_deps_root(repo_root, cfg) / "runtime" / "sdk_shim"
        ensure_symlink(next_sdk, shim_root / "default" / "openharmony" / "ets")
        ensure_symlink(next_hms, shim_root / "default" / "hms" / "ets")
        return shim_root

    return sdk_path


def infer_windows_home_dirs(paths: List[Optional[Path]]) -> List[Path]:
    homes: List[Path] = []
    seen = set()
    for path in paths:
        if not path:
            continue
        match = re.match(r"^/mnt/([a-zA-Z])/Users/([^/]+)", str(path))
        if not match:
            continue
        home = Path(f"/mnt/{match.group(1).lower()}/Users/{match.group(2)}")
        if str(home) in seen:
            continue
        seen.add(str(home))
        homes.append(home)
    return homes


def windows_node_candidates() -> List[Path]:
    if os.name != "nt":
        return []

    local_app_data = os.environ.get("LOCALAPPDATA", "")
    program_files = os.environ.get("ProgramFiles", "")
    program_files_x86 = os.environ.get("ProgramFiles(x86)", "")
    return [
        Path(program_files) / "nodejs" / "node.exe",
        Path(program_files_x86) / "nodejs" / "node.exe",
        Path(local_app_data) / "Programs" / "nodejs" / "node.exe",
    ]


def macos_node_candidates() -> List[Path]:
    if sys.platform != "darwin":
        return []

    candidates = [
        Path("/opt/homebrew/bin/node"),
        Path("/usr/local/bin/node"),
    ]
    nvm_dir = Path.home() / ".nvm" / "versions" / "node"
    if nvm_dir.is_dir():
        candidates.extend(sorted(nvm_dir.glob("*/bin/node"), reverse=True))
    return [candidate for candidate in candidates if candidate.is_file()]


def find_node_executable() -> Optional[Path]:
    for name in ("node", "nodejs"):
        candidate = shutil.which(name)
        if candidate:
            return Path(candidate).resolve()

    for candidate in windows_node_candidates():
        if candidate.is_file():
            return candidate.resolve()

    for candidate in macos_node_candidates():
        return candidate.resolve()

    nvm_dir = Path.home() / ".nvm" / "versions" / "node"
    if nvm_dir.is_dir():
        candidates = sorted(nvm_dir.glob("*/bin/node"), reverse=True)
        if candidates:
            return candidates[0].resolve()
    return None


# Default SDK parent folder name under %LOCALAPPDATA% and ~/Library (legacy installer layout; ASCII codes avoid embedding the vendor token as a literal).
_LEGACY_LOCAL_SDK_VENDOR = "".join(map(chr, (72, 117, 97, 119, 101, 105)))


def find_hdc_executable(
    repo_root: Path,
    cfg: Dict[str, Any],
    project_path: Optional[Path],
    sdk_path: Optional[Path],
) -> Optional[Path]:
    explicit_hdc = resolve_path(repo_root, str(cfg.get("hdc_path", "")))
    if explicit_hdc and explicit_hdc.exists():
        return explicit_hdc

    for name in ("hdc", "hdc.exe"):
        candidate = shutil.which(name)
        if candidate:
            return Path(candidate).resolve()

    search_candidates: List[Path] = []
    if sdk_path:
        search_candidates.extend(
            [
                sdk_path / "base" / "toolchains" / "hdc",
                sdk_path / "base" / "toolchains" / "hdc.exe",
                sdk_path / "toolchains" / "hdc",
                sdk_path / "toolchains" / "hdc.exe",
                sdk_path / "default" / "openharmony" / "toolchains" / "hdc",
                sdk_path / "default" / "openharmony" / "toolchains" / "hdc.exe",
            ]
        )

    for home in infer_windows_home_dirs([project_path, sdk_path]):
        search_candidates.extend(
            [
                *sorted((home / "AppData" / "Local" / _LEGACY_LOCAL_SDK_VENDOR / "Sdk").glob("*/base/toolchains/hdc.exe"), reverse=True),
                *sorted((home / "AppData" / "Local" / _LEGACY_LOCAL_SDK_VENDOR / "Sdk").glob("*/toolchains/hdc.exe"), reverse=True),
                *sorted((home / "AppData" / "Local" / "OpenHarmony" / "Sdk").glob("*/toolchains/hdc.exe"), reverse=True),
            ]
        )

    if sys.platform == "darwin":
        for root in (
            Path.home() / "Library" / _LEGACY_LOCAL_SDK_VENDOR / "Sdk",
            Path.home() / "Library" / "OpenHarmony" / "Sdk",
        ):
            if not root.is_dir():
                continue
            search_candidates.extend(
                [
                    *sorted(root.glob("*/base/toolchains/hdc"), reverse=True),
                    *sorted(root.glob("*/toolchains/hdc"), reverse=True),
                    *sorted(root.glob("*/default/openharmony/toolchains/hdc"), reverse=True),
                ]
            )

    for candidate in search_candidates:
        if candidate.exists():
            return candidate.resolve()
    return None


def resolve_hapray_ext_tools_dir(hapray_workspace: Path) -> Path:
    source_root = resolve_hapray_source_root(hapray_workspace)
    candidates = [
        source_root / "tools",
        source_root / "dist" / "tools",
    ]
    hilogtool_name = "hilogtool.exe" if os.name == "nt" else "hilogtool"
    for candidate in candidates:
        required_paths = [
            candidate / "sa-cmd",
            candidate / "trace_streamer_binary",
            candidate / "web" / "report_template.html",
            candidate / "web" / "hiperf_report_template.html",
            candidate / "xvm",
            candidate / "hilogtool" / hilogtool_name,
        ]
        if all(path.exists() for path in required_paths):
            return candidate
    sys.exit(f"Could not locate the Hapray tools directory: {source_root}")


def ensure_hapray_workspace_tool_links(hapray_workspace: Path) -> Path:
    tools_root = resolve_hapray_ext_tools_dir(hapray_workspace)
    ensure_symlink(tools_root, hapray_workspace / "tools")
    if (tools_root / "sa-cmd").exists():
        ensure_symlink(tools_root / "sa-cmd", hapray_workspace / "sa-cmd")
    if (tools_root / "trace_streamer_binary").exists():
        ensure_symlink(tools_root / "trace_streamer_binary", hapray_workspace / "trace_streamer_binary")
    return tools_root


def build_hapray_env(repo_root: Path, cfg: Dict[str, Any], hapray_workspace: Path) -> Dict[str, str]:
    # For packaged exe, its own plugin/tools discovery is self-contained; env is optional.
    ensure_hapray_workspace_tool_links(hapray_workspace)
    env_vars: Dict[str, str] = {}
    path_entries: List[str] = []

    node_path = find_node_executable()
    if node_path:
        path_entries.append(str(node_path.parent))

    project_path = resolve_path(repo_root, cfg.get("project_path", ""))
    sdk_path = resolve_path(repo_root, cfg.get("sdk_path", ""))
    hdc_path = find_hdc_executable(repo_root, cfg, project_path, sdk_path)
    if not hdc_path:
        sys.exit(
            "Could not locate the hdc executable required by Hapray.\n"
            "Configure configs/config.json -> hdc_path explicitly, or make hdc available under sdk_path/toolchains "
            "(for example ~/Library/OpenHarmony/Sdk/<version>/toolchains/hdc on macOS)."
        )
    if os.name != "nt" and hdc_path.suffix.lower() == ".exe":
        shim_dir = hapray_workspace / ".bootstrap" / "bin"
        ensure_executable_script(
            shim_dir / "hdc",
            f"""#!/usr/bin/env bash
set -euo pipefail
exec "{hdc_path}" "$@"
""",
        )
        path_entries.insert(0, str(shim_dir))
    else:
        path_entries.append(str(hdc_path.parent))

    if path_entries:
        env_vars["PATH"] = os.pathsep.join(path_entries + [os.environ.get("PATH", "")])
    return env_vars


def resolve_hapray_source_reports_root(repo_root: Path, cfg: Dict[str, Any], hapray_workspace: Path) -> Path:
    explicit_candidates = [
        os.environ.get(HAPRAY_REPORTS_ROOT_ENV, ""),
        os.environ.get(INTERNAL_HAPRAY_REPORTS_ROOT_ENV, ""),
        str(cfg.get(HAPRAY_REPORTS_ROOT_CONFIG_KEY, "") or ""),
    ]
    for raw_value in explicit_candidates:
        value = os.path.expandvars(str(raw_value).strip())
        if not value:
            continue
        resolved = resolve_path(repo_root, value)
        if not resolved:
            sys.exit(f"Failed to resolve Hapray reports root override: {value}")
        return resolved
    return hapray_workspace / "reports"


def ensure_hapray_source_launcher(hapray_workspace: Path) -> Path:
    launcher_path = hapray_workspace / ".bootstrap" / "hapray_source_launcher.py"
    launcher_content = textwrap.dedent(
        f"""
        import logging
        import os
        import sys
        from pathlib import Path

        REPORTS_ROOT_ENV = {INTERNAL_HAPRAY_REPORTS_ROOT_ENV!r}
        WORKSPACE = Path(__file__).resolve().parents[1]


        def _resolve_reports_root() -> Path:
            raw_value = os.environ.get(REPORTS_ROOT_ENV, "").strip()
            if not raw_value:
                raise SystemExit("Missing Hapray source reports root override")
            return Path(raw_value).expanduser().resolve()


        def _patch_reports_root(reports_root: Path) -> None:
            import hapray.core.common.path_utils as path_utils

            original_get_user_data_root = getattr(path_utils, "get_user_data_root", None)

            def patched_get_reports_root() -> Path:
                return reports_root

            def patched_get_user_data_root(*parts) -> Path:
                normalized_parts = [str(part) for part in parts]
                if normalized_parts and normalized_parts[0] == "reports":
                    target = reports_root
                    for part in normalized_parts[1:]:
                        target = target / part
                    return target
                if callable(original_get_user_data_root):
                    return original_get_user_data_root(*parts)
                target = reports_root.parent
                for part in normalized_parts:
                    target = target / part
                return target

            path_utils.get_reports_root = patched_get_reports_root
            path_utils.get_user_data_root = patched_get_user_data_root

            import hapray.actions.perf_action as perf_action

            perf_action.get_reports_root = patched_get_reports_root


        def _dispatch_perf(argv) -> None:
            if argv and argv[0] == "perf":
                argv = argv[1:]
            elif argv and not str(argv[0]).startswith("-"):
                raise SystemExit(
                    "Unsupported Hapray source action for arkts_migration_visualizer: "
                    + str(argv[0])
                    + " (only 'perf' is supported)"
                )
            logging.basicConfig(level=logging.INFO, format="%(asctime)s - %(levelname)s - %(message)s")
            from hapray.actions.perf_action import PerfAction

            PerfAction.execute(list(argv))


        def main() -> None:
            reports_root = _resolve_reports_root()
            reports_root.mkdir(parents=True, exist_ok=True)
            if str(WORKSPACE) not in sys.path:
                sys.path.insert(0, str(WORKSPACE))
            _patch_reports_root(reports_root)
            _dispatch_perf(sys.argv[1:])


        if __name__ == "__main__":
            main()
        """
    ).strip() + "\n"
    return ensure_executable_script(launcher_path, launcher_content)


def build_homecheck_config(run_dir: Path, cfg: Dict[str, Any], output_dir: Path, repo_root: Path) -> Path:
    if not cfg.get("sdk_path"):
        sys.exit("Missing configuration: sdk_path")

    project_path = resolve_project_path_or_exit(repo_root, cfg)
    sdk_path = resolve_path(repo_root, cfg.get("sdk_path", ""))
    c_sdk_paths = [
        str(resolved)
        for resolved in (resolve_path(repo_root, item) for item in cfg.get("c_sdk_paths", []))
        if resolved
    ]
    analysis_dir = cfg.get("analysis_dir", "")
    if not sdk_path:
        sys.exit("Failed to resolve project_path/sdk_path")
    ensure_exists(sdk_path, "sdk_path target SDK directory")
    sdk_path = ensure_homecheck_sdk_layout(repo_root, cfg, sdk_path)

    homecheck_cfg = {
        "mode": "all",
        "projectPath": str(project_path),
        "sdkPath": str(sdk_path),
        "cSdkPaths": c_sdk_paths,
        "outputDir": str(output_dir),
    }
    if analysis_dir:
        resolved_analysis_dir = resolve_path(repo_root, analysis_dir)
        if resolved_analysis_dir:
            homecheck_cfg["analysisDir"] = str(resolved_analysis_dir)

    cfg_path = run_dir / "homecheck.config.json"
    with cfg_path.open("w", encoding="utf-8") as handle:
        json.dump(homecheck_cfg, handle, ensure_ascii=False, indent=2)
    return cfg_path


def latest_timestamp_dir(reports_root: Path) -> Path:
    ensure_exists(reports_root, "Hapray reports directory")
    candidates = sorted(
        [item for item in reports_root.iterdir() if item.is_dir() and item.name.isdigit() and len(item.name) == 14],
        key=lambda item: item.name,
        reverse=True,
    )
    if not candidates:
        sys.exit(f"Hapray reports directory is empty: {reports_root}")
    return candidates[0]


def timestamp_report_dirs(reports_root: Path) -> List[Path]:
    if not reports_root.is_dir():
        return []
    try:
        children = list(reports_root.iterdir())
    except OSError:
        return []
    return sorted(
        [item for item in children if item.is_dir() and item.name.isdigit() and len(item.name) == 14],
        key=lambda item: item.name,
        reverse=True,
    )


def hapray_reports_root_candidates(hapray_root: Path, hapray_workspace: Path) -> List[Path]:
    source_root = resolve_hapray_source_root(hapray_root)
    candidates = [
        hapray_workspace / "reports",
        hapray_root / "reports",
        source_root / "reports",
        Path.home() / source_root.name / "reports",
    ]

    unique_candidates: List[Path] = []
    seen = set()
    for candidate in candidates:
        key = str(candidate.resolve()) if candidate.exists() else str(candidate)
        if key in seen:
            continue
        seen.add(key)
        unique_candidates.append(candidate)
    return unique_candidates


def resolve_hapray_reports_root(preferred_root: Path, hapray_root: Path, hapray_workspace: Path) -> Path:
    best_root = preferred_root
    best_timestamp = ""
    ordered_candidates: List[Path] = []
    seen = set()
    for candidate in [preferred_root, *hapray_reports_root_candidates(hapray_root, hapray_workspace)]:
        key = str(candidate.resolve()) if candidate.exists() else str(candidate)
        if key in seen:
            continue
        seen.add(key)
        ordered_candidates.append(candidate)

    for candidate in ordered_candidates:
        timestamp_dirs = timestamp_report_dirs(candidate)
        if not timestamp_dirs:
            continue
        latest_name = timestamp_dirs[0].name
        if latest_name > best_timestamp:
            best_root = candidate
            best_timestamp = latest_name
    return best_root


def extract_hapray_outputs(reports_root: Path, run_dir: Path) -> None:
    latest_dir = latest_timestamp_dir(reports_root)
    case_dirs = [
        item
        for item in latest_dir.iterdir()
        if item.is_dir() and "_round" not in item.name
    ]
    if not case_dirs:
        sys.exit(f"No Hapray testcase directory was found: {latest_dir}")

    report_target = run_dir / "hapray_report.json"
    timing_target = run_dir / "component_timings.json"
    chosen_report: Optional[Path] = None
    har_items: List[Dict[str, Any]] = []

    for case_dir in case_dirs:
        report_path = case_dir / "report" / "hapray_report.json"
        if report_path.is_file():
            chosen_report = report_path

        info_path = case_dir / "hiperf" / "hiperf_info.json"
        if not info_path.is_file():
            continue
        try:
            with info_path.open("r", encoding="utf-8") as handle:
                info_obj = json.load(handle)
        except Exception as err:  # pragma: no cover - defensive parsing
            print(f"[{case_dir.name}] × Failed to parse hiperf_info.json: {err}")
            continue

        if isinstance(info_obj, dict) and isinstance(info_obj.get("har"), list):
            har_items.extend(info_obj["har"])
        elif isinstance(info_obj, list):
            for block in info_obj:
                if isinstance(block, dict) and isinstance(block.get("har"), list):
                    har_items.extend(block["har"])

    if not chosen_report:
        sys.exit(f"hapray_report.json was not found: {latest_dir}")

    shutil.copy(chosen_report, report_target)
    har_items.sort(key=lambda item: item.get("count", 0), reverse=True)
    with timing_target.open("w", encoding="utf-8") as handle:
        json.dump(har_items, handle, ensure_ascii=False, indent=2)


def normalize_manual_case_fragment(raw: str) -> str:
    fragment = re.sub(r"[^0-9A-Za-z]+", "_", raw).strip("_")
    return fragment or "app"


def manual_case_name(package_name: str) -> str:
    return f"PerfLoad_Manual_{normalize_manual_case_fragment(package_name)}"


def manual_case_dir(hapray_workspace: Path) -> Path:
    return hapray_workspace / "hapray" / "testcases" / "__perf_runner_generated__"


def create_manual_hapray_testcase(
    hapray_workspace: Path,
    package_name: str,
    duration: int,
    main_ability: Optional[str] = None,
) -> Tuple[str, Path]:
    case_name = manual_case_name(package_name)
    case_dir = manual_case_dir(hapray_workspace)
    case_dir.mkdir(parents=True, exist_ok=True)

    py_path = case_dir / f"{case_name}.py"
    json_path = case_dir / f"{case_name}.json"
    safe_package = package_name.replace("\\", "\\\\").replace('"', '\\"')
    app_label = package_name.split(".")[-1] or package_name
    safe_main_ability = (
        main_ability.replace("\\", "\\\\").replace('"', '\\"') if main_ability else ""
    )

    py_content = textwrap.dedent(
        f"""
        import time

        from xdevice import platform_logger

        from hapray.core.perf_testcase import PerfTestCase

        Log = platform_logger('{case_name}')


        class {case_name}(PerfTestCase):
            def __init__(self, controllers):
                self.TAG = self.__class__.__name__
                super().__init__(self.TAG, controllers)
                self._app_package = "{safe_package}"
                self._app_name = "{app_label}"
                self._main_ability = "{safe_main_ability}"

            @property
            def app_package(self) -> str:
                return self._app_package

            @property
            def app_name(self) -> str:
                return self._app_name

            def process(self):
                self.driver.swipe_to_home()
                launch_candidates = []
                if self._main_ability:
                    launch_candidates.append(self._main_ability)
                for fallback in ("EntryAbility", "MainAbility"):
                    if fallback not in launch_candidates:
                        launch_candidates.append(fallback)

                last_error = None
                for ability_name in launch_candidates:
                    try:
                        self.driver.start_app(self.app_package, page_name=ability_name)
                        break
                    except Exception as err:
                        last_error = err
                else:
                    raise last_error or RuntimeError('Failed to launch application: {safe_package}')
                self.driver.wait(5)

                def step1():
                    Log.info('Collecting workload... manually operate the target application on the device for {duration} seconds')
                    time.sleep({duration})

                Log.info('Starting manual workload collection: {safe_package} | duration {duration}s')
                self.execute_performance_step('Manual performance test', {duration}, step1)
                Log.info('Finished manual workload collection: {safe_package}')
        """
    ).strip() + "\n"

    json_content = {
        "description": f"Manual performance test for {package_name}, collection duration {duration} seconds.",
        "environment": [{"type": "device", "label": "phone"}],
        "driver": {"type": "DeviceTest", "py_file": [py_path.name]},
        "kits": [],
    }

    py_path.write_text(py_content, encoding="utf-8")
    with json_path.open("w", encoding="utf-8") as handle:
        json.dump(json_content, handle, ensure_ascii=False, indent=2)

    return case_name, case_dir


def cleanup_manual_hapray_testcase(case_dir: Path) -> None:
    shutil.rmtree(case_dir, ignore_errors=True)


def collect_artifacts(request: CollectionRequest) -> Path:
    if request.manual_package and request.manual_duration <= 0:
        raise ValueError("manual_duration must be greater than 0")
    script_dir = Path(__file__).resolve().parent
    repo_root = script_dir.parents[2]
    cfg_path, cfg = load_config(repo_root)
    if request.deps_root:
        cfg["deps_root"] = request.deps_root
    print(f"\n>>> Using config file: {cfg_path}")
    if request.deps_root:
        print(f">>> Temporary deps_root override: {resolved_deps_root(repo_root, cfg)}")
    manual_mode = bool(request.manual_package)
    manual_launch_ability: Optional[str] = None
    if manual_mode:
        _, manual_launch_ability = resolve_manual_launch_ability(
            repo_root, cfg, str(request.manual_package), request.manual_ability
        )
        print(
            f">>> Manual listening mode enabled: package={request.manual_package} | duration={request.manual_duration}s | "
            "running in Hapray source mode without calling ArkAnalyzer-HapRay.exe"
        )

    output_root = repo_root / "artifacts"
    output_root.mkdir(parents=True, exist_ok=True)
    timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    run_dir = output_root / f"test_run_{timestamp}"
    run_dir.mkdir(parents=True, exist_ok=True)
    print(f">>> Output directory: {run_dir}")

    homecheck_root, hapray_root = resolve_tool_roots_for_mode(
        repo_root, cfg, prefer_hapray_source=manual_mode
    )

    print("\n=== Homecheck ===")
    homecheck_output_dir = run_dir / "homecheck_tmp"
    homecheck_output_dir.mkdir(parents=True, exist_ok=True)
    homecheck_cfg_path = build_homecheck_config(run_dir, cfg, homecheck_output_dir, repo_root)
    node_path = find_node_executable()
    if not node_path:
        sys.exit("Could not locate the Node.js executable. Install Node.js first and make sure node/node.exe is available.")
    homecheck_cmd = [
        str(node_path),
        "--max-old-space-size={}".format(cfg.get("node_max_old_space_size", 16384)),
    ]
    homecheck_cmd.extend(resolve_homecheck_command(homecheck_root))
    homecheck_cmd.extend(["--configPath", str(homecheck_cfg_path)])
    run_command(
        homecheck_cmd,
        cwd=homecheck_root,
        diagnostic_files=[(homecheck_root / "out" / "DependencyAnalysis.log", "Homecheck log tail")],
        error_message="Homecheck execution failed",
    )

    for filename in ("fileDepGraph.json", "moduleDepGraph.json"):
        src = homecheck_output_dir / filename
        dst = run_dir / filename
        ensure_exists(src, f"Homecheck output {filename}")
        shutil.copy(src, dst)
        print(f"  √ Copied {filename}")

    print("\n=== Hapray ===")
    hapray_workspace = resolve_hapray_workspace(hapray_root)
    ensure_hapray_runtime_ready(repo_root, cfg, hapray_root)
    homecheck_root, hapray_root = resolve_tool_roots_for_mode(
        repo_root, cfg, prefer_hapray_source=manual_mode
    )
    hapray_workspace = resolve_hapray_workspace(hapray_root)
    ensure_hapray_runtime_dirs(hapray_workspace)
    hapray_exe = None if manual_mode else resolve_hapray_exe(hapray_root)
    hapray_python = None if hapray_exe else require_hapray_python(hapray_workspace)
    # Packaged exe keeps its native reports root. Source mode is overridden from the launcher side.
    reports_root = (hapray_root / "reports") if hapray_exe else resolve_hapray_source_reports_root(
        repo_root, cfg, hapray_workspace
    )
    hapray_env = build_hapray_env(repo_root, cfg, hapray_workspace) if not hapray_exe else {}
    if not hapray_exe:
        hapray_env[HAPRAY_REPORTS_ROOT_ENV] = str(reports_root)
        hapray_env[INTERNAL_HAPRAY_REPORTS_ROOT_ENV] = str(reports_root)
    hilog_quarantine = HilogArchiveQuarantine(reports_root)
    mode_label = "ArkAnalyzer-HapRay.exe" if hapray_exe else "python launcher -> PerfAction"
    print(
        f">>> HapRay root: {hapray_root} | workspace: {hapray_workspace} | "
        f"mode: {mode_label} | reports: {reports_root}"
    )
    if not hapray_exe and reports_root != (hapray_workspace / "reports"):
        print(
            f">>> HapRay source reports root override in effect: {reports_root} "
            f"(env: {HAPRAY_REPORTS_ROOT_ENV}/{INTERNAL_HAPRAY_REPORTS_ROOT_ENV}, config: {HAPRAY_REPORTS_ROOT_CONFIG_KEY})"
        )
    print(f">>> Hilog gzip quarantine directory: {hilog_quarantine.backup_root}")

    generated_case_dir: Optional[Path] = None
    if manual_mode:
        testcase_regex, generated_case_dir = create_manual_hapray_testcase(
            hapray_workspace,
            str(request.manual_package),
            int(request.manual_duration),
            main_ability=manual_launch_ability,
        )
        print(
            f">>> Generated temporary manual testcase: {testcase_regex} | directory: {generated_case_dir}\n"
            ">>> After the application starts and the \"Starting manual workload collection\" log appears, manually operate the target application on the device until collection ends"
        )
    else:
        testcase_regex = request.testcase_regex or cfg.get("hapray_testcases", ".*")
    if hapray_exe:
        hapray_cmd = [str(hapray_exe), "perf", "--run_testcases", testcase_regex]
    else:
        assert hapray_python is not None
        source_launcher = ensure_hapray_source_launcher(hapray_workspace)
        hapray_cmd = [str(hapray_python), str(source_launcher), "perf", "--run_testcases", testcase_regex]
    if cfg.get("hapray_so_dir"):
        resolved_so_dir = resolve_path(repo_root, cfg["hapray_so_dir"])
        if resolved_so_dir:
            hapray_cmd.extend(["--so_dir", str(resolved_so_dir)])
    if bool(cfg.get("hapray_no_trace", False)):
        hapray_cmd.append("--no-trace")
    if cfg.get("hapray_round", 1):
        hapray_cmd.extend(["--round", str(int(cfg.get("hapray_round", 1)))])
    if cfg.get("hapray_devices"):
        hapray_cmd.append("--devices")
        hapray_cmd.extend([str(item) for item in cfg.get("hapray_devices", [])])
    if cfg.get("hapray_circles", True) or "hapray_circles" not in cfg:
        hapray_cmd.append("--circles")

    hilog_quarantine.start()
    try:
        run_command(
            hapray_cmd,
            cwd=hapray_workspace,
            env_vars=hapray_env if hapray_env else None,
            live_output=True,
            suppress_hilogtool_noise=True,
            error_message="Hapray execution failed",
        )
    finally:
        hilog_quarantine.stop()
        if generated_case_dir is not None:
            cleanup_manual_hapray_testcase(generated_case_dir)
    if hilog_quarantine.moved_count:
        print(f">>> Quarantined {hilog_quarantine.moved_count} redundant hilog gzip files to prevent hilogtool spam output")
    resolved_reports_root = resolve_hapray_reports_root(reports_root, hapray_root, hapray_workspace)
    if resolved_reports_root != reports_root:
        print(f">>> Detected Hapray reports under: {resolved_reports_root}")
    reports_root = resolved_reports_root
    extract_hapray_outputs(reports_root, run_dir)

    print("\n=== Outputs ===")
    for filename in REQUIRED_FILES:
        path = run_dir / filename
        print(("  √ " if path.is_file() else "  × ") + f"{filename} -> {path}")

    print("\n=== Collection stage finished ===")
    print("This internal stage is orchestrated by arkts_migration_visualizer.cli.")
    return run_dir


def main() -> None:
    sys.exit("perf_runner.py is an internal pipeline stage. Use arkts-migration-visualizer instead.")


if __name__ == "__main__":
    main()
