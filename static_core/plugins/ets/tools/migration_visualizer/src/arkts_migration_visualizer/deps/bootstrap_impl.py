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
Download and bootstrap GitCode-hosted tool dependencies for the visualizer.

Layout:
  .deps/
    sources/
      homecheck/
      ArkAnalyzer-HapRay/
    runtime/
      homecheck/
    dependency-lock.json
"""

from __future__ import annotations

import argparse
import json
import os
import shutil
import re
import subprocess
import sys
import time
import zipfile
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Dict, List, Optional, Tuple

from arkts_migration_visualizer.common.fs_utils import (
    ensure_executable_script,
    ensure_symlink,
    reset_path,
)


REPO_ROOT = Path(__file__).resolve().parents[3]
DEFAULT_MANIFEST = REPO_ROOT / "configs" / "dependency_manifest.json"
LOCK_FILE_NAME = "dependency-lock.json"
# Open-AutoGLM is only required by ArkAnalyzer-HapRay's optional gui-agent command.
# arkts_migration_visualizer uses Hapray's perf/manual workflows, so gui-agent support stays opt-in.
GUI_AGENT_REPO_URL = "https://gitcode.com/zai-org/Open-AutoGLM.git"
GUI_AGENT_DIR_NAME = "Open-AutoGLM"
PACKAGE_INSTALL_ORDER = ["xdevice", "xdevice-devicetest", "xdevice-ohos", "hypium"]
PACKAGE_DOWNLOAD_ATTEMPTS = 4
WINDOWS_ABS_PATH_RE = re.compile(r"^[A-Za-z]:[\\/]")
GIT_NETWORK_ATTEMPTS = 3
PYTHON_PACKAGE_INSTALLER_CHOICES = ("auto", "pip", "uv")
HAPRAY_REQUIREMENT_OVERRIDES = {
    "opencv-python": "opencv-python==4.10.0.84",
    "opencv-contrib-python": "opencv-contrib-python==4.10.0.84",
}
HAPRAY_EXTRA_SPECS = ["psutil>=5.9.0", "rich>=13.0"]
STATIC_ANALYZER_ROOT_DEPENDENCIES = ("bjc", "sql.js")
STATIC_ANALYZER_REQUIRED_PACKAGE_FILES = {
    "copy-webpack-plugin": ("dist", "index.js"),
    "schema-utils": ("dist", "validate.js"),
    "ajv-keywords": ("dist", "index.js"),
    "ajv": ("dist", "compile", "codegen", "index.js"),
}


class CommandError(RuntimeError):
    pass


@dataclass
class ToolSpec:
    name: str
    repo: str
    ref: str
    source_dir: Path
    runtime_dir: Optional[Path] = None
    workspace_subdir: Optional[str] = None


def log(message: str) -> None:
    print(message, flush=True)


def windows_tool_candidates(name: str) -> List[str]:
    if os.name != "nt":
        candidates: List[str] = [name]
        home = Path.home()
        common_paths = {
            "git": [
                Path("/usr/bin/git"),
                Path("/opt/homebrew/bin/git"),
                Path("/usr/local/bin/git"),
            ],
            "node": [
                Path("/opt/homebrew/bin/node"),
                Path("/usr/local/bin/node"),
            ],
            "npm": [
                Path("/opt/homebrew/bin/npm"),
                Path("/usr/local/bin/npm"),
            ],
        }
        if name in {"node", "npm"}:
            nvm_versions_dir = home / ".nvm" / "versions" / "node"
            if nvm_versions_dir.is_dir():
                candidates.extend(
                    str(path)
                    for path in sorted(nvm_versions_dir.glob(f"*/bin/{name}"), reverse=True)
                )
        candidates.extend(str(path) for path in common_paths.get(name, []) if path.is_file())
        return candidates

    local_app_data = os.environ.get("LOCALAPPDATA", "")
    program_files = os.environ.get("ProgramFiles", "")
    program_files_x86 = os.environ.get("ProgramFiles(x86)", "")
    common_paths = {
        "git": [
            Path(program_files) / "Git" / "cmd" / "git.exe",
            Path(program_files) / "Git" / "bin" / "git.exe",
            Path(program_files_x86) / "Git" / "cmd" / "git.exe",
            Path(program_files_x86) / "Git" / "bin" / "git.exe",
            Path(local_app_data) / "Programs" / "Git" / "cmd" / "git.exe",
        ],
        "node": [
            Path(program_files) / "nodejs" / "node.exe",
            Path(program_files_x86) / "nodejs" / "node.exe",
            Path(local_app_data) / "Programs" / "nodejs" / "node.exe",
        ],
        "npm": [
            Path(program_files) / "nodejs" / "npm.cmd",
            Path(program_files_x86) / "nodejs" / "npm.cmd",
            Path(local_app_data) / "Programs" / "nodejs" / "npm.cmd",
        ],
    }
    candidates = []
    preferred_suffix = {
        "git": "git.exe",
        "node": "node.exe",
        "npm": "npm.cmd",
    }.get(name)
    if preferred_suffix:
        candidates.append(preferred_suffix)
    candidates.append(name)
    candidates.extend(str(path) for path in common_paths.get(name, []) if str(path) not in {"", "."})
    return candidates


def resolve_executable(name: str, env: Optional[Dict[str, str]] = None) -> str:
    path = (env or os.environ).get("PATH", os.environ.get("PATH", ""))
    searched: List[str] = []
    for candidate in windows_tool_candidates(name):
        if not candidate or candidate in searched:
            continue
        searched.append(candidate)
        resolved = shutil.which(candidate, path=path)
        if resolved:
            return resolved
        candidate_path = Path(candidate)
        if candidate_path.is_file():
            return str(candidate_path)
    raise CommandError(f"required executable not found: {name}\nsearched: {', '.join(searched)}")


def tool_cmd(name: str, *args: str, env: Optional[Dict[str, str]] = None) -> List[str]:
    return [resolve_executable(name, env=env), *args]


def git_network_cmd(*args: str, env: Optional[Dict[str, str]] = None) -> List[str]:
    return tool_cmd(
        "git",
        "-c",
        "http.version=HTTP/1.1",
        "-c",
        "core.compression=0",
        *args,
        env=env,
    )


def prepend_tool_dir_to_path(env: Dict[str, str], name: str) -> str:
    resolved = resolve_executable(name, env=env)
    tool_dir = str(Path(resolved).parent)
    current_path = env.get("PATH", os.environ.get("PATH", ""))
    env["PATH"] = os.pathsep.join([tool_dir, current_path]) if current_path else tool_dir
    return resolved


def run_cmd(
    cmd: List[str],
    cwd: Optional[Path] = None,
    env: Optional[Dict[str, str]] = None,
    capture: bool = False,
) -> subprocess.CompletedProcess:
    pretty = " ".join(str(part) for part in cmd)
    log(f"$ {pretty}")
    try:
        result = subprocess.run(
            cmd,
            cwd=str(cwd) if cwd else None,
            env=env,
            text=True,
            encoding="utf-8",
            stdout=subprocess.PIPE if capture else None,
            stderr=subprocess.STDOUT if capture else None,
            check=False,
        )
    except FileNotFoundError as exc:
        tool_name = Path(cmd[0]).name if cmd else "<empty>"
        raise CommandError(f"required executable not found while running: {tool_name}\ncommand: {pretty}") from exc
    if result.returncode != 0:
        output = result.stdout.strip() if result.stdout else ""
        raise CommandError(f"command failed ({result.returncode}): {pretty}\n{output}")
    if capture and result.stdout:
        log(result.stdout.strip())
    return result


def resolve_path(base_dir: Path, raw: str) -> Optional[Path]:
    raw = str(raw or "").strip()
    if not raw:
        return None
    if WINDOWS_ABS_PATH_RE.match(raw):
        if os.name == "nt":
            return Path(raw)
        drive = raw[0].lower()
        remainder = raw[2:].replace("\\", "/").lstrip("/")
        return Path("/mnt") / drive / remainder
    path = Path(raw).expanduser()
    if path.is_absolute():
        return path
    return (base_dir / path).resolve()


def load_deps_root_from_config(repo_root: Path) -> Optional[Path]:
    cfg_path = repo_root / "configs" / "config.json"
    if not cfg_path.is_file():
        return None
    try:
        cfg = json.loads(cfg_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return None
    return resolve_path(repo_root, str(cfg.get("deps_root", "")).strip())


def validate_windows_deps_root(deps_root: Path, tools: Dict[str, ToolSpec]) -> None:
    if os.name != "nt":
        return

    samples = [(spec.name, spec.source_dir) for spec in tools.values()]
    too_long = [(name, path) for name, path in samples if len(str(path)) >= 180]
    if not too_long:
        return

    lines = [
        "The current Windows dependency directory is too long, so git clone/fetch is likely to fail.",
        f"Current deps_root: {deps_root}",
        "Change deps_root in configs/config.json to a shorter absolute path,",
        "or rerun src\\scripts\\bootstrap.cmd with a shorter --deps-root value directly.",
        "For example: --deps-root .amv-deps\\arkts-migration-visualizer-bbb",
        "Overlong target directories:",
    ]
    for name, path in too_long:
        lines.append(f"- {name}: {path} (len={len(str(path))})")
    raise CommandError("\n".join(lines))


def load_manifest(path: Path, deps_root_override: str = "") -> Tuple[Path, Dict[str, ToolSpec]]:
    manifest = json.loads(path.read_text(encoding="utf-8"))
    repo_root = path.parent.parent
    configured_deps_root = resolve_path(repo_root, deps_root_override) if deps_root_override else None
    deps_root = configured_deps_root or load_deps_root_from_config(repo_root)
    if not deps_root:
        deps_root = (repo_root / manifest.get("depsRoot", ".deps")).resolve()
    tools = {}
    for name, raw in manifest.get("tools", {}).items():
        tools[name] = ToolSpec(
            name=name,
            repo=raw["repo"],
            ref=raw["ref"],
            source_dir=(deps_root / raw["sourceDir"]).resolve(),
            runtime_dir=(deps_root / raw["runtimeDir"]).resolve() if raw.get("runtimeDir") else None,
            workspace_subdir=raw.get("workspaceSubdir"),
        )
    validate_windows_deps_root(deps_root, tools)
    return deps_root, tools


def is_git_worktree(repo_dir: Path) -> bool:
    git_dir = repo_dir / ".git"
    if not git_dir.exists():
        return False
    result = subprocess.run(
        git_network_cmd("-C", str(repo_dir), "rev-parse", "--is-inside-work-tree"),
        text=True,
        encoding="utf-8",
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )
    return result.returncode == 0 and result.stdout.strip() == "true"


def clone_git_repo(repo: str, target_dir: Path, attempts: int = GIT_NETWORK_ATTEMPTS) -> None:
    last_error: Optional[CommandError] = None
    for attempt in range(1, attempts + 1):
        if target_dir.exists():
            reset_path(target_dir)
        target_dir.parent.mkdir(parents=True, exist_ok=True)
        try:
            run_cmd(git_network_cmd("clone", "--no-tags", repo, str(target_dir)))
            return
        except CommandError as exc:
            last_error = exc
            if attempt == attempts:
                break
            log(f"git clone failed, cleaning partial checkout and retrying ({attempt}/{attempts - 1})")
            time.sleep(2)
    if last_error is None:
        raise CommandError(f"git clone failed without captured error: {repo}")
    raise last_error


def ensure_git_checkout(spec: ToolSpec) -> str:
    spec.source_dir.parent.mkdir(parents=True, exist_ok=True)
    if not is_git_worktree(spec.source_dir):
        clone_git_repo(spec.repo, spec.source_dir)
    else:
        local_ref = subprocess.run(
            git_network_cmd("-C", str(spec.source_dir), "rev-parse", "--verify", "{}^{{commit}}".format(spec.ref)),
            text=True,
            encoding="utf-8",
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            check=False,
        )
        if local_ref.returncode != 0:
            run_cmd_retry(
                git_network_cmd("-C", str(spec.source_dir), "fetch", "--tags", "origin"),
                attempts=GIT_NETWORK_ATTEMPTS,
            )
    run_cmd(git_network_cmd("-C", str(spec.source_dir), "checkout", spec.ref))
    commit = (
        run_cmd(git_network_cmd("-C", str(spec.source_dir), "rev-parse", "HEAD"), capture=True)
        .stdout.strip()
    )
    return commit


def ensure_runtime_package_json(runtime_dir: Path) -> None:
    runtime_dir.mkdir(parents=True, exist_ok=True)
    package_json = runtime_dir / "package.json"
    if package_json.exists():
        return
    run_cmd(tool_cmd("npm", "init", "-y"), cwd=runtime_dir)


def latest_file(directory: Path, pattern: str) -> Path:
    matches = sorted(directory.glob(pattern), key=lambda p: p.stat().st_mtime, reverse=True)
    if not matches:
        raise FileNotFoundError(f"no file matching {pattern} under {directory}")
    return matches[0]


def npm_env(
    registry: Optional[str],
    fetch_timeout: int,
    fetch_retries: int,
    strict_ssl: Optional[str] = None,
) -> Dict[str, str]:
    env = os.environ.copy()
    npm_cache_dir = REPO_ROOT / ".deps" / ".npm-cache"
    npm_cache_dir.mkdir(parents=True, exist_ok=True)
    env["npm_config_cache"] = str(npm_cache_dir)
    if registry:
        env["npm_config_registry"] = registry
    if strict_ssl:
        env["npm_config_strict_ssl"] = strict_ssl
    env["npm_config_fetch_timeout"] = str(fetch_timeout)
    env["npm_config_fetch_retries"] = str(fetch_retries)
    env["npm_config_workspaces"] = "false"
    env["npm_config_include_workspace_root"] = "false"
    return env


def npm_script_env(base_env: Dict[str, str]) -> Dict[str, str]:
    env = dict(base_env)
    # Workspace-aware npm scripts can fail if the install-time no-workspaces flags leak into runtime.
    env.pop("npm_config_workspaces", None)
    env.pop("npm_config_include_workspace_root", None)
    return env


def npm_install_attempts(fetch_retries: int) -> int:
    return max(fetch_retries + 1, 3)


def npm_dependency_install_command(project_dir: Path, env: Dict[str, str]) -> List[str]:
    subcommand = "ci" if (project_dir / "package-lock.json").is_file() else "install"
    return tool_cmd("npm", subcommand, "--ignore-scripts", "--workspaces=false", "--no-audit", env=env)


def python_package_env(index_url: Optional[str], timeout: int, retries: int, cache_dir: Optional[Path] = None) -> Dict[str, str]:
    env = os.environ.copy()
    if index_url:
        env["PIP_INDEX_URL"] = index_url
    env["PIP_DEFAULT_TIMEOUT"] = str(timeout)
    env["PIP_RETRIES"] = str(retries)
    env["PIP_DISABLE_PIP_VERSION_CHECK"] = "1"
    if cache_dir:
        cache_dir.mkdir(parents=True, exist_ok=True)
        env["PIP_CACHE_DIR"] = str(cache_dir)
    return env


def python_package_install_attempts(retries: int) -> int:
    return max(retries + 1, 3)


def resolve_python_package_installer(preferred: str, env: Optional[Dict[str, str]] = None) -> str:
    normalized = (preferred or "auto").strip().lower()
    if normalized not in PYTHON_PACKAGE_INSTALLER_CHOICES:
        raise CommandError(
            "unsupported python package installer: "
            f"{preferred} (expected one of: {', '.join(PYTHON_PACKAGE_INSTALLER_CHOICES)})"
        )
    if normalized == "pip":
        return "pip"

    try:
        resolve_executable("uv", env=env)
        return "uv"
    except CommandError as exc:
        if normalized == "uv":
            raise CommandError("python package installer 'uv' was requested but the uv executable was not found") from exc
        return "pip"


def offline_install_command(
    installer: str,
    python_exe: Path,
    specs: List[str],
    wheelhouse: Path,
    env: Optional[Dict[str, str]] = None,
) -> List[str]:
    if installer == "uv":
        return tool_cmd(
            "uv",
            "pip",
            "install",
            "--python",
            str(python_exe),
            "--no-index",
            f"--find-links={wheelhouse}",
            *specs,
            env=env,
        )
    return [str(python_exe), "-m", "pip", "install", "--no-index", f"--find-links={wheelhouse}", *specs]


def local_package_install_command(
    installer: str,
    python_exe: Path,
    package_path: Path,
    env: Optional[Dict[str, str]] = None,
    *,
    no_deps: bool = False,
) -> List[str]:
    if installer == "uv":
        base_cmd = tool_cmd("uv", "pip", "install", "--python", str(python_exe), env=env)
    else:
        base_cmd = [str(python_exe), "-m", "pip", "install"]
    if no_deps:
        base_cmd.append("--no-deps")
    return [*base_cmd, str(package_path)]


def bootstrap_homecheck(
    spec: ToolSpec,
    verify: bool,
    registry: Optional[str],
    fetch_timeout: int,
    fetch_retries: int,
    npm_strict_ssl: Optional[str],
) -> Dict[str, str]:
    if not spec.runtime_dir:
        raise CommandError("homecheck runtimeDir is missing from manifest")
    npm_environment = npm_env(registry, fetch_timeout, fetch_retries, npm_strict_ssl)
    run_cmd_retry(
        npm_dependency_install_command(spec.source_dir, npm_environment),
        cwd=spec.source_dir,
        env=npm_environment,
        attempts=npm_install_attempts(fetch_retries),
    )
    pack_result = run_cmd(tool_cmd("npm", "pack", env=npm_environment), cwd=spec.source_dir, env=npm_environment, capture=True)
    tarball_name = ""
    for line in reversed(pack_result.stdout.splitlines()):
        stripped = line.strip()
        if stripped.endswith(".tgz"):
            tarball_name = stripped
            break
    tarball_path = spec.source_dir / tarball_name if tarball_name else latest_file(spec.source_dir, "homecheck-*.tgz")

    ensure_runtime_package_json(spec.runtime_dir)
    run_cmd(
        tool_cmd("npm", "install", "--no-save", str(tarball_path), env=npm_environment),
        cwd=spec.runtime_dir,
        env=npm_environment,
    )

    home_dep_js = spec.runtime_dir / "node_modules" / "homecheck" / "lib" / "tools" / "HomeDep.js"
    if not home_dep_js.is_file():
        raise FileNotFoundError(f"missing HomeDep entry: {home_dep_js}")

    if verify:
        run_cmd(tool_cmd("node", str(home_dep_js), "--help", env=npm_environment), cwd=spec.runtime_dir)

    return {
        "status": "installed",
        "homeDep": str(home_dep_js),
        "runtimeDir": str(spec.runtime_dir),
        "tarball": str(tarball_path),
    }


def parse_python_version(executable: str) -> Optional[Tuple[int, int, int]]:
    try:
        result = subprocess.run(
            [executable, "-c", "import sys; print('.'.join(map(str, sys.version_info[:3])))"],
            text=True,
            encoding="utf-8",
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            check=False,
        )
    except OSError:
        return None
    if result.returncode != 0:
        return None
    parts = result.stdout.strip().split(".")
    if len(parts) != 3:
        return None
    return int(parts[0]), int(parts[1]), int(parts[2])


def local_python_candidates() -> List[str]:
    suffix = Path(".py311") / ("Scripts/python.exe" if os.name == "nt" else "bin/python")
    candidates: List[str] = []
    for base in [REPO_ROOT] + list(REPO_ROOT.parents):
        candidate = base / suffix
        if candidate.is_file():
            candidates.append(str(candidate))
    return candidates


def platform_python_candidates() -> List[str]:
    candidates: List[str] = []
    if sys.executable:
        candidates.append(sys.executable)

    if sys.platform == "darwin":
        preferred_names = ("python3.12", "python3.11", "python3.10", "python3")
        for base_dir in (
            Path("/opt/homebrew/bin"),
            Path("/usr/local/bin"),
            Path.home() / ".pyenv" / "shims",
        ):
            for name in preferred_names:
                candidate = base_dir / name
                if candidate.is_file():
                    candidates.append(str(candidate))
    return candidates


def find_hapray_python(explicit: Optional[str], minimum: Tuple[int, int] = (3, 9)) -> Tuple[Optional[str], Optional[str]]:
    candidates: List[str] = []
    if explicit:
        candidates.append(explicit)
    env_python = os.environ.get("HAPRAY_PYTHON")
    if env_python:
        candidates.append(env_python)
    candidates.extend(local_python_candidates())
    candidates.extend(platform_python_candidates())
    candidates.extend(["python3.12", "python3.11", "python3.10", "python3.9", "python3", "python"])

    seen = set()
    for candidate in candidates:
        if candidate in seen:
            continue
        seen.add(candidate)
        version = parse_python_version(candidate)
        if not version:
            continue
        if minimum <= version[:2] <= (3, 12):
            return candidate, ".".join(str(part) for part in version)
    return None, None


def workspace_dir(spec: ToolSpec) -> Path:
    return spec.source_dir / spec.workspace_subdir if spec.workspace_subdir else spec.source_dir


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


def venv_python(workspace: Path) -> Path:
    return venv_python_in_dir(hapray_venv_dir(workspace))


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


def managed_dir(workspace: Path) -> Path:
    return workspace / ".bootstrap"


def ensure_hapray_runtime_dirs(workspace: Path) -> None:
    for relative in ("logs", "reports", "haptest_reports", "runtime"):
        (workspace / relative).mkdir(parents=True, exist_ok=True)


def run_cmd_retry(
    cmd: List[str],
    attempts: int,
    cwd: Optional[Path] = None,
    env: Optional[Dict[str, str]] = None,
    capture: bool = False,
    retry_delay_seconds: int = 2,
) -> subprocess.CompletedProcess:
    last_error: Optional[CommandError] = None
    for attempt in range(1, attempts + 1):
        try:
            return run_cmd(cmd, cwd=cwd, env=env, capture=capture)
        except CommandError as exc:
            last_error = exc
            if attempt == attempts:
                break
            log(f"retry {attempt}/{attempts - 1} failed, waiting {retry_delay_seconds}s before retry")
            time.sleep(retry_delay_seconds)
    if last_error is None:
        raise CommandError("command retry failed without captured error")
    raise last_error


def ensure_virtualenv(python_exe: str, workspace: Path) -> Path:
    current_dir = hapray_venv_dir(workspace)
    python_path = venv_python(workspace)
    if venv_matches_current_platform(current_dir):
        return python_path

    legacy_dir = legacy_hapray_venv_dir(workspace)
    if not current_dir.exists() and venv_matches_current_platform(legacy_dir):
        log(f"reusing legacy Hapray venv: {legacy_dir}")
        return venv_python_in_dir(legacy_dir)

    if current_dir.exists():
        shutil.rmtree(current_dir)
    run_cmd([python_exe, "-m", "venv", str(current_dir)], cwd=workspace)
    if not python_path.is_file():
        raise FileNotFoundError(f"missing Hapray venv python after creation: {python_path}")
    return python_path


def strip_inline_comment(line: str) -> str:
    if line.lstrip().startswith("#"):
        return ""
    return re.sub(r"\s+#.*$", "", line).strip()


def requirement_specs(requirements_path: Path) -> List[str]:
    specs: List[str] = []
    for raw_line in requirements_path.read_text(encoding="utf-8").splitlines():
        stripped = strip_inline_comment(raw_line)
        if not stripped:
            continue
        specs.append(stripped)
    return specs


def package_name_from_spec(spec: str) -> str:
    match = re.match(r"^([A-Za-z0-9_.-]+)", spec)
    if not match:
        return spec.strip().lower()
    return match.group(1).replace("_", "-").lower()


def resolve_specs(specs: List[str], overrides: Dict[str, str], extras: Optional[List[str]] = None) -> List[str]:
    resolved: List[str] = []
    seen = set()

    for spec in specs + (extras or []):
        package_name = package_name_from_spec(spec)
        resolved_spec = overrides.get(package_name, spec)
        normalized = package_name_from_spec(resolved_spec)
        if normalized in seen:
            continue
        seen.add(normalized)
        resolved.append(resolved_spec)
    return resolved


def extract_package_prefix(file_name: str) -> str:
    patterns = [
        r"^([a-zA-Z0-9_]+?)-\d.*\.(whl|tar\.gz)$",
        r"^([a-zA-Z0-9_-]+?)-\d+[a-zA-Z0-9.]*\.(whl|tar\.gz)$",
    ]
    for pattern in patterns:
        match = re.match(pattern, file_name)
        if not match:
            continue
        prefix = match.group(1)
        if any(char.isdigit() for char in prefix.split("-")[-1]):
            return "-".join(prefix.split("-")[:-1])
        return prefix
    return ""


def package_install_rank(file_name: str) -> float:
    prefix = extract_package_prefix(file_name)
    try:
        return float(PACKAGE_INSTALL_ORDER.index(prefix))
    except ValueError:
        return float("inf")


def package_files(directory: Path) -> List[Path]:
    return [file for file in directory.iterdir() if file.suffix == ".whl" or file.name.endswith(".tar.gz")]


def extract_zip_if_needed(zip_path: Path, output_dir: Path) -> Path:
    if output_dir.exists() and any(output_dir.iterdir()):
        return output_dir
    if not zip_path.is_file():
        raise FileNotFoundError(f"missing archive: {zip_path}")
    with zipfile.ZipFile(zip_path, "r") as archive:
        archive.extractall(output_dir)
    return output_dir


def ensure_file_copy(source: Path, target: Path) -> Path:
    if not source.is_file():
        raise FileNotFoundError(f"missing source file: {source}")
    target.parent.mkdir(parents=True, exist_ok=True)
    if target.exists() or target.is_symlink():
        reset_path(target)
    shutil.copy2(source, target)
    return target


def ensure_windows_cmd_script(script_path: Path, content: str) -> Path:
    script_path.parent.mkdir(parents=True, exist_ok=True)
    with script_path.open("w", encoding="utf-8", newline="\r\n") as handle:
        handle.write(content)
    return script_path


def hapray_tools_dir(repo_root: Path) -> Path:
    return repo_root / "tools"


def hapray_dist_tools_dir(repo_root: Path) -> Path:
    return repo_root / "dist" / "tools"


def ensure_hapray_tool_alias(source: Path, target: Path) -> Path:
    if not source.exists() and not source.is_symlink():
        raise FileNotFoundError(f"missing Hapray tool asset: {source}")
    return ensure_symlink(source, target)


def ensure_trace_streamer_assets(repo_root: Path) -> Path:
    tools_dir = hapray_dist_tools_dir(repo_root)
    target_dir = tools_dir / "trace_streamer_binary"
    trace_streamer_bin = target_dir / trace_streamer_executable_name()
    if trace_streamer_bin.is_file() and not trace_streamer_bin.is_symlink():
        if os.name != "nt":
            trace_streamer_bin.chmod(0o755)
        ensure_hapray_tool_alias(target_dir, hapray_tools_dir(repo_root) / "trace_streamer_binary")
        return target_dir

    if target_dir.exists() or target_dir.is_symlink():
        reset_path(target_dir)

    archive_path = repo_root / "third-party" / "trace_streamer_binary.zip"
    if not archive_path.is_file():
        raise FileNotFoundError(f"missing trace streamer archive: {archive_path}")
    tools_dir.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(archive_path, "r") as archive:
        archive.extractall(tools_dir)

    if not trace_streamer_bin.is_file():
        raise FileNotFoundError(f"missing extracted trace streamer: {trace_streamer_bin}")
    if os.name != "nt":
        trace_streamer_bin.chmod(0o755)
    ensure_hapray_tool_alias(target_dir, hapray_tools_dir(repo_root) / "trace_streamer_binary")
    return target_dir


def extract_zip_tree(zip_path: Path, target_dir: Path) -> Path:
    if target_dir.exists() and any(target_dir.iterdir()):
        return target_dir
    if not zip_path.is_file():
        raise FileNotFoundError(f"missing archive: {zip_path}")
    target_dir.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(zip_path, "r") as archive:
        archive.extractall(target_dir.parent)
    if not target_dir.exists():
        raise FileNotFoundError(f"missing extracted directory: {target_dir}")
    return target_dir


def hilogtool_platform_dir() -> str:
    if os.name == "nt":
        return "windows"
    if sys.platform == "darwin":
        return "mac"
    return "linux"


def hilogtool_executable_name() -> str:
    return "hilogtool.exe" if os.name == "nt" else "hilogtool"


def ensure_hilogtool_assets(repo_root: Path) -> Path:
    tools_dir = hapray_dist_tools_dir(repo_root)
    target_dir = extract_zip_tree(repo_root / "third-party" / "hilogtool.zip", tools_dir / "hilogtool")
    platform_executable = target_dir / hilogtool_platform_dir() / hilogtool_executable_name()
    preferred_executable = target_dir / hilogtool_executable_name()
    if platform_executable.is_file() and not preferred_executable.is_file():
        ensure_file_copy(platform_executable, preferred_executable)
    if not preferred_executable.is_file():
        raise FileNotFoundError(f"missing hilogtool executable: {preferred_executable}")
    if os.name != "nt":
        preferred_executable.chmod(0o755)
    ensure_hapray_tool_alias(target_dir, hapray_tools_dir(repo_root) / "hilogtool")
    return target_dir


def ensure_xvm_assets(repo_root: Path) -> Path:
    target_dir = extract_zip_tree(repo_root / "third-party" / "xvm.zip", hapray_dist_tools_dir(repo_root) / "xvm")
    ensure_hapray_tool_alias(target_dir, hapray_tools_dir(repo_root) / "xvm")
    return target_dir


def ensure_web_report_assets(
    repo_root: Path,
    registry: Optional[str],
    fetch_timeout: int,
    fetch_retries: int,
    npm_strict_ssl: Optional[str],
) -> Path:
    tools_web_dir = hapray_dist_tools_dir(repo_root) / "web"
    resource_web_dir = repo_root / "perf_testing" / "resource" / "web"
    report_template = resource_web_dir / "report_template.html"
    hiperf_template = repo_root / "third-party" / "report.html"
    web_dist_template = repo_root / "web" / "dist" / "index.html"

    ensure_file_copy(hiperf_template, resource_web_dir / "hiperf_report_template.html")

    if not report_template.is_file():
        web_dir = repo_root / "web"
        npm_environment = npm_env(registry, fetch_timeout, fetch_retries, npm_strict_ssl)
        npm_build_environment = npm_script_env(npm_environment)
        run_cmd_retry(
            tool_cmd("npm", "install", "--ignore-scripts", "--workspaces=false", "--no-audit", env=npm_environment),
            attempts=npm_install_attempts(fetch_retries),
            cwd=web_dir,
            env=npm_environment,
        )
        # Only web/dist/index.html is needed here as the report template; the upstream type-check is not required.
        run_cmd(tool_cmd("npm", "run", "build-only", env=npm_build_environment), cwd=web_dir, env=npm_build_environment)

    if not report_template.is_file() and web_dist_template.is_file():
        ensure_file_copy(web_dist_template, report_template)

    if not report_template.is_file():
        raise FileNotFoundError(f"missing Hapray web report template: {report_template}")

    ensure_file_copy(report_template, tools_web_dir / "report_template.html")
    ensure_file_copy(hiperf_template, tools_web_dir / "hiperf_report_template.html")
    ensure_hapray_tool_alias(tools_web_dir, hapray_tools_dir(repo_root) / "web")
    return tools_web_dir


def ensure_hapray_runtime_assets(
    repo_root: Path,
    registry: Optional[str],
    fetch_timeout: int,
    fetch_retries: int,
    npm_strict_ssl: Optional[str],
) -> Dict[str, str]:
    xvm_dir = ensure_xvm_assets(repo_root)
    hilogtool_dir = ensure_hilogtool_assets(repo_root)
    web_dir = ensure_web_report_assets(repo_root, registry, fetch_timeout, fetch_retries, npm_strict_ssl)
    return {
        "xvmDir": str(xvm_dir),
        "hilogtoolDir": str(hilogtool_dir),
        "hilogtool": str(hilogtool_dir / hilogtool_executable_name()),
        "webDir": str(web_dir),
        "reportTemplate": str(web_dir / "report_template.html"),
        "hiperfReportTemplate": str(web_dir / "hiperf_report_template.html"),
    }


def static_analyzer_ready(dist_dir: Path) -> bool:
    required_paths = [
        dist_dir / "hapray-sa-cmd.js",
        dist_dir / "node_modules" / "sql.js" / "dist" / "sql-wasm.js",
        dist_dir / "res" / "perf" / "kind.json",
    ]
    return all(path.exists() for path in required_paths)


def find_windows_launcher_compiler() -> Optional[List[str]]:
    if os.name != "nt":
        return None
    for candidate in ("csc", "csc.exe"):
        resolved = shutil.which(candidate)
        if resolved:
            return [resolved]
    system_root = os.environ.get("SystemRoot") or os.environ.get("WINDIR") or ""
    program_files = os.environ.get("ProgramFiles", "")
    program_files_x86 = os.environ.get("ProgramFiles(x86)", "")
    common_paths = [
        os.path.join(system_root, "Microsoft.NET", "Framework64", "v4.0.30319", "csc.exe"),
        os.path.join(system_root, "Microsoft.NET", "Framework", "v4.0.30319", "csc.exe"),
        os.path.join(system_root, "Microsoft.NET", "Framework64", "v3.5", "csc.exe"),
        os.path.join(system_root, "Microsoft.NET", "Framework", "v3.5", "csc.exe"),
        os.path.join(
            program_files,
            "Microsoft Visual Studio",
            "2022",
            "BuildTools",
            "MSBuild",
            "Current",
            "Bin",
            "Roslyn",
            "csc.exe",
        ),
        os.path.join(
            program_files,
            "Microsoft Visual Studio",
            "2022",
            "Community",
            "MSBuild",
            "Current",
            "Bin",
            "Roslyn",
            "csc.exe",
        ),
        os.path.join(
            program_files,
            "Microsoft Visual Studio",
            "2022",
            "Professional",
            "MSBuild",
            "Current",
            "Bin",
            "Roslyn",
            "csc.exe",
        ),
        os.path.join(
            program_files,
            "Microsoft Visual Studio",
            "2022",
            "Enterprise",
            "MSBuild",
            "Current",
            "Bin",
            "Roslyn",
            "csc.exe",
        ),
        os.path.join(program_files_x86, "MSBuild", "14.0", "Bin", "csc.exe"),
        os.path.join(program_files_x86, "MSBuild", "12.0", "Bin", "csc.exe"),
    ]
    for path in common_paths:
        if path in {"", "."}:
            continue
        if os.path.isfile(path):
            return [path]
    return None


def launcher_command_name(command: Optional[List[str]]) -> str:
    if not command:
        return ""
    return command[0]


def ensure_windows_hapray_launcher(dist_dir: Path) -> Tuple[Optional[Path], Optional[List[str]]]:
    if os.name != "nt":
        return None, None

    launcher_path = dist_dir / "hapray-sa-cmd.exe"
    if launcher_path.is_file():
        return launcher_path, None

    compiler_cmd = find_windows_launcher_compiler()
    if not compiler_cmd:
        return None, None

    source_path = dist_dir / "hapray-sa-cmd-launcher.cs"
    source_path.write_text(
        """using System;
using System.Diagnostics;
using System.IO;

internal static class Program
{
    private static string Quote(string value)
    {
        if (value.IndexOfAny(new[] { ' ', '\t', '\"' }) < 0)
        {
            return value;
        }
        return "\\\"" + value.Replace("\\\\", "\\\\\\\\").Replace("\\\"", "\\\\\\\"") + "\\\"";
    }

    private static string BuildArguments(string jsPath, string[] args)
    {
        string joined = Quote(jsPath);
        foreach (string arg in args)
        {
            joined += " " + Quote(arg);
        }
        return joined;
    }

    public static int Main(string[] args)
    {
        string exeDir = AppDomain.CurrentDomain.BaseDirectory;
        string jsPath = Path.Combine(exeDir, "hapray-sa-cmd.js");
        if (!File.Exists(jsPath))
        {
            Console.Error.WriteLine("Missing hapray-sa-cmd.js: " + jsPath);
            return 1;
        }

        Process process = new Process();
        process.StartInfo.FileName = "node";
        process.StartInfo.Arguments = BuildArguments(jsPath, args);
        process.StartInfo.WorkingDirectory = exeDir;
        process.StartInfo.UseShellExecute = false;
        process.Start();
        process.WaitForExit();
        return process.ExitCode;
    }
}
""",
        encoding="utf-8",
    )

    run_cmd(
        [
            *compiler_cmd,
            "/nologo",
            f"/out:{launcher_path}",
            str(source_path),
        ],
        cwd=dist_dir,
    )
    return (launcher_path if launcher_path.is_file() else None), compiler_cmd


def ensure_hapray_tools_layout(repo_root: Path, workspace: Path) -> Dict[str, str]:
    dist_tools_dir = hapray_dist_tools_dir(repo_root)
    source_tools_dir = hapray_tools_dir(repo_root)
    created: Dict[str, str] = {}

    for name in ("sa-cmd", "trace_streamer_binary", "web", "xvm", "hilogtool"):
        candidate = dist_tools_dir / name
        if candidate.exists() or candidate.is_symlink():
            ensure_hapray_tool_alias(candidate, source_tools_dir / name)
            created[name] = str(source_tools_dir / name)

    static_analyzer_dir = source_tools_dir / "static_analyzer"
    if static_analyzer_dir.exists() or static_analyzer_dir.is_symlink():
        ensure_hapray_tool_alias(static_analyzer_dir, dist_tools_dir / "static_analyzer")
        created["static_analyzer"] = str(dist_tools_dir / "static_analyzer")

    ensure_hapray_tool_alias(source_tools_dir, workspace / "tools")
    created["workspace_tools"] = str(workspace / "tools")
    return created


def determine_hapray_runtime_patch_info(
    runtime_launcher: Optional[Path],
    launcher_compiler: Optional[List[str]],
) -> Dict[str, str]:
    compiler_name = launcher_command_name(launcher_compiler)
    if os.name != "nt":
        return {
            "status": "not_needed",
            "reason": "non_windows",
            "launcherCompiler": compiler_name,
        }
    if runtime_launcher and runtime_launcher.is_file():
        return {
            "status": "not_needed",
            "reason": "native_launcher_present",
            "launcherCompiler": compiler_name,
        }
    if launcher_compiler:
        return {
            "status": "required",
            "reason": "launcher_not_created_after_compile",
            "launcherCompiler": compiler_name,
        }
    return {
        "status": "required",
        "reason": "compiler_not_found",
        "launcherCompiler": compiler_name,
    }


def determine_hapray_runtime_patch_status(runtime_launcher: Optional[Path]) -> str:
    return determine_hapray_runtime_patch_info(runtime_launcher, None)["status"]


def ensure_static_analyzer_root_dependency_links(repo_root: Path, analyzer_dir: Path) -> Dict[str, str]:
    root_node_modules = repo_root / "node_modules"
    analyzer_node_modules = analyzer_dir / "node_modules"
    created: Dict[str, str] = {}
    for package_name in STATIC_ANALYZER_ROOT_DEPENDENCIES:
        source = analyzer_node_modules / package_name
        if not source.exists():
            raise FileNotFoundError(f"missing static_analyzer dependency: {source}")
        target = root_node_modules / package_name
        ensure_symlink(source, target)
        created[package_name] = str(target)
    return created


def static_analyzer_node_module_entry(analyzer_dir: Path, package_name: str, *segments: str) -> Path:
    path = analyzer_dir / "node_modules" / package_name
    for segment in segments:
        path /= segment
    if not path.is_file():
        raise FileNotFoundError(f"missing static_analyzer tool entry: {path}")
    return path


def static_analyzer_missing_dependency_files(analyzer_dir: Path) -> List[Path]:
    required_files = [
        analyzer_dir / "node_modules" / package_name / Path(*segments)
        for package_name, segments in STATIC_ANALYZER_REQUIRED_PACKAGE_FILES.items()
    ]
    return [path for path in required_files if not path.is_file()]


def nested_static_analyzer_package_candidates(
    analyzer_dir: Path,
    package_name: str,
    required_segments: Tuple[str, ...],
) -> List[Path]:
    node_modules_dir = analyzer_dir / "node_modules"
    if not node_modules_dir.is_dir():
        return []

    candidates: List[Path] = []
    for candidate in node_modules_dir.glob(f"**/node_modules/{package_name}"):
        if (candidate / Path(*required_segments)).is_file():
            candidates.append(candidate)
    candidates.sort(key=lambda item: (len(item.parts), str(item)))
    return candidates


def repair_static_analyzer_dependency_tree_from_nested_packages(analyzer_dir: Path) -> Dict[str, str]:
    repaired: Dict[str, str] = {}
    node_modules_dir = analyzer_dir / "node_modules"
    for package_name, segments in STATIC_ANALYZER_REQUIRED_PACKAGE_FILES.items():
        target_dir = node_modules_dir / package_name
        if (target_dir / Path(*segments)).is_file():
            continue

        candidates = nested_static_analyzer_package_candidates(analyzer_dir, package_name, segments)
        if not candidates:
            continue

        ensure_symlink(candidates[0], target_dir)
        if (target_dir / Path(*segments)).is_file():
            repaired[package_name] = str(target_dir)
    return repaired


def ensure_static_analyzer_dependency_tree(
    analyzer_dir: Path,
    env: Dict[str, str],
    fetch_retries: int,
) -> None:
    missing = static_analyzer_missing_dependency_files(analyzer_dir)
    if not missing:
        return

    repaired = repair_static_analyzer_dependency_tree_from_nested_packages(analyzer_dir)
    if repaired:
        missing = static_analyzer_missing_dependency_files(analyzer_dir)
        if not missing:
            log(
                "Repaired static_analyzer dependencies from nested node_modules: "
                + ", ".join(sorted(repaired))
            )
            return

    log(
        "Detected incomplete static_analyzer node_modules; retrying npm dependency install: "
        + ", ".join(str(path.relative_to(analyzer_dir)) for path in missing)
    )
    run_cmd_retry(
        npm_dependency_install_command(analyzer_dir, env),
        attempts=npm_install_attempts(fetch_retries),
        cwd=analyzer_dir,
        env=env,
    )
    repaired = repair_static_analyzer_dependency_tree_from_nested_packages(analyzer_dir)
    if repaired:
        log(
            "Repaired static_analyzer dependencies from nested node_modules after npm install: "
            + ", ".join(sorted(repaired))
        )
    missing = static_analyzer_missing_dependency_files(analyzer_dir)
    if missing:
        raise FileNotFoundError(
            "static_analyzer dependencies are incomplete after npm install: "
            + ", ".join(str(path.relative_to(analyzer_dir)) for path in missing)
        )


def write_static_analyzer_tsconfig_wrapper(analyzer_dir: Path, bootstrap_dir: Path) -> Path:
    bootstrap_dir.mkdir(parents=True, exist_ok=True)
    wrapper_path = bootstrap_dir / "tsconfig.static_analyzer.bootstrap.json"
    original_tsconfig = analyzer_dir / "tsconfig.prod.json"
    root_dir = analyzer_dir / "src"
    wrapper_path.write_text(
        json.dumps(
            {
                "extends": str(original_tsconfig),
                "compilerOptions": {
                    "rootDir": str(root_dir),
                },
            },
            ensure_ascii=False,
            indent=2,
        ),
        encoding="utf-8",
    )
    return wrapper_path


def write_static_analyzer_webpack_wrapper(analyzer_dir: Path, bootstrap_dir: Path, tsconfig_path: Path) -> Path:
    bootstrap_dir.mkdir(parents=True, exist_ok=True)
    wrapper_path = bootstrap_dir / "webpack.static_analyzer.bootstrap.cjs"
    original_config = analyzer_dir / "webpack.config.js"
    wrapper_path.write_text(
        f"""const defineArrayMethod = (name, implementation) => {{
    if (typeof Array.prototype[name] === 'function') {{
        return;
    }}
    Object.defineProperty(Array.prototype, name, {{
        value: implementation,
        writable: true,
        configurable: true,
    }});
}};

defineArrayMethod('toSorted', function(compareFn) {{
    return Array.prototype.slice.call(this).sort(compareFn);
}});

defineArrayMethod('toReversed', function() {{
    return Array.prototype.slice.call(this).reverse();
}});

defineArrayMethod('toSpliced', function(start, deleteCount, ...items) {{
    const copy = Array.prototype.slice.call(this);
    copy.splice(start, deleteCount, ...items);
    return copy;
}});

defineArrayMethod('with', function(index, value) {{
    const copy = Array.prototype.slice.call(this);
    const normalizedIndex = index < 0 ? copy.length + index : index;
    if (normalizedIndex < 0 || normalizedIndex >= copy.length) {{
        throw new RangeError('Invalid index');
    }}
    copy[normalizedIndex] = value;
    return copy;
}});

const config = require({json.dumps(str(original_config))});
const rules = (((config || {{}}).module || {{}}).rules || []);
for (const rule of rules) {{
    if (!rule || String(rule.test) !== String(/\\.tsx?$/)) {{
        continue;
    }}
    const currentUse = rule.use;
    if (currentUse === 'ts-loader') {{
        rule.use = {{
            loader: 'ts-loader',
            options: {{
                configFile: {json.dumps(str(tsconfig_path))},
            }},
        }};
        continue;
    }}
    if (currentUse && currentUse.loader === 'ts-loader') {{
        rule.use = {{
            loader: currentUse.loader,
            options: Object.assign({{}}, currentUse.options || {{}}, {{
                configFile: {json.dumps(str(tsconfig_path))},
            }}),
        }};
    }}
}}
module.exports = config;
""",
        encoding="utf-8",
    )
    return wrapper_path


def build_static_analyzer_assets(
    repo_root: Path,
    analyzer_dir: Path,
    bootstrap_dir: Path,
    env: Dict[str, str],
) -> Dict[str, str]:
    compatibility_links = ensure_static_analyzer_root_dependency_links(repo_root, analyzer_dir)
    tsc_entry = static_analyzer_node_module_entry(analyzer_dir, "typescript", "bin", "tsc")
    webpack_cli = static_analyzer_node_module_entry(analyzer_dir, "webpack-cli", "bin", "cli.js")
    tsconfig_wrapper = write_static_analyzer_tsconfig_wrapper(analyzer_dir, bootstrap_dir)
    webpack_wrapper = write_static_analyzer_webpack_wrapper(analyzer_dir, bootstrap_dir, tsconfig_wrapper)
    run_cmd(tool_cmd("node", str(tsc_entry), "-p", str(tsconfig_wrapper), env=env), cwd=analyzer_dir, env=env)
    run_cmd(tool_cmd("node", str(webpack_cli), "--config", str(webpack_wrapper), env=env), cwd=analyzer_dir, env=env)
    return {
        "tsconfigWrapper": str(tsconfig_wrapper),
        "webpackWrapper": str(webpack_wrapper),
        **{f"rootNodeModule{key.title().replace('.', '')}": value for key, value in compatibility_links.items()},
    }


def ensure_static_analyzer_assets(
    repo_root: Path,
    bootstrap_dir: Path,
    registry: Optional[str],
    fetch_timeout: int,
    fetch_retries: int,
    npm_strict_ssl: Optional[str],
) -> Tuple[Path, Path, Path, Optional[Path], Optional[List[str]], Dict[str, str]]:
    analyzer_dir = repo_root / "tools" / "static_analyzer"
    dist_dir = hapray_dist_tools_dir(repo_root) / "sa-cmd"
    js_path = dist_dir / "hapray-sa-cmd.js"
    build_details: Dict[str, str] = {}
    if not static_analyzer_ready(dist_dir):
        npm_environment = npm_env(registry, fetch_timeout, fetch_retries, npm_strict_ssl)
        run_cmd_retry(
            npm_dependency_install_command(analyzer_dir, npm_environment),
            attempts=npm_install_attempts(fetch_retries),
            cwd=analyzer_dir,
            env=npm_environment,
        )
        ensure_static_analyzer_dependency_tree(analyzer_dir, npm_environment, fetch_retries)
        build_details = build_static_analyzer_assets(repo_root, analyzer_dir, bootstrap_dir, npm_environment)

    if not js_path.is_file():
        raise FileNotFoundError(f"missing built static analyzer entry: {js_path}")

    script_path = ensure_executable_script(
        dist_dir / "hapray-sa-cmd",
        """#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
exec node "$SCRIPT_DIR/hapray-sa-cmd.js" "$@"
""",
    )
    cmd_path = ensure_windows_cmd_script(
        dist_dir / "hapray-sa-cmd.cmd",
        """@echo off
node "%~dp0hapray-sa-cmd.js" %*
""",
    )
    exe_path, launcher_compiler = ensure_windows_hapray_launcher(dist_dir)
    ensure_hapray_tool_alias(dist_dir, hapray_tools_dir(repo_root) / "sa-cmd")
    preferred_path = cmd_path if os.name == "nt" else script_path
    return preferred_path, js_path, cmd_path, exe_path, launcher_compiler, build_details


def ensure_workspace_tool_links(workspace: Path, repo_root: Path) -> Tuple[Path, Path, Path]:
    source_tools_dir = hapray_tools_dir(repo_root)
    workspace_tools_dir = ensure_symlink(source_tools_dir, workspace / "tools")
    sa_cmd_link = ensure_symlink(source_tools_dir / "sa-cmd", workspace / "sa-cmd")
    trace_streamer_link = ensure_symlink(source_tools_dir / "trace_streamer_binary", workspace / "trace_streamer_binary")
    return workspace_tools_dir, sa_cmd_link, trace_streamer_link


def trace_streamer_executable_name() -> str:
    if sys.platform == "win32":
        return "trace_streamer_windows.exe"
    if sys.platform == "darwin":
        return "trace_streamer_mac"
    return "trace_streamer_linux"


def sa_cmd_command(command_path: Path, *args: str) -> List[str]:
    suffix = command_path.suffix.lower()
    if suffix == ".js":
        return tool_cmd("node", str(command_path), *args)
    if suffix in {".cmd", ".bat"}:
        js_path = command_path.with_suffix(".js")
        if js_path.is_file():
            return tool_cmd("node", str(js_path), *args)
    return [str(command_path), *args]


def pip_download_specs(
    python_exe: Path,
    specs: List[str],
    wheelhouse: Path,
    env: Dict[str, str],
) -> None:
    if not specs:
        return
    wheelhouse.mkdir(parents=True, exist_ok=True)
    for spec in specs:
        run_cmd_retry(
            [str(python_exe), "-m", "pip", "download", "--dest", str(wheelhouse), "--prefer-binary", spec],
            cwd=wheelhouse,
            env=env,
            attempts=PACKAGE_DOWNLOAD_ATTEMPTS,
        )


def pip_install_offline_specs(
    python_exe: Path,
    specs: List[str],
    wheelhouse: Path,
    env: Dict[str, str],
    installer: str,
    install_attempts: int,
) -> None:
    if not specs:
        return
    run_cmd_retry(
        offline_install_command(installer, python_exe, specs, wheelhouse, env=env),
        cwd=wheelhouse,
        env=env,
        attempts=install_attempts,
    )


def install_local_packages(
    python_exe: Path,
    packages_dir: Path,
    env: Dict[str, str],
    installer: str,
    install_attempts: int,
) -> None:
    ordered_packages = package_files(packages_dir)
    ordered_packages.sort(key=lambda path: package_install_rank(path.name))
    for package in ordered_packages:
        run_cmd_retry(
            local_package_install_command(installer, python_exe, package, env=env),
            cwd=packages_dir,
            env=env,
            attempts=install_attempts,
        )


def repo_commit(repo_dir: Path) -> str:
    return run_cmd(tool_cmd("git", "-C", str(repo_dir), "rev-parse", "HEAD"), capture=True).stdout.strip()


def install_requirement_specs(
    python_exe: Path,
    requirements_path: Path,
    wheelhouse: Path,
    env: Dict[str, str],
    installer: str,
    install_attempts: int,
    overrides: Optional[Dict[str, str]] = None,
    extras: Optional[List[str]] = None,
) -> List[str]:
    if not requirements_path.is_file():
        return []
    specs = resolve_specs(
        requirement_specs(requirements_path),
        overrides or {},
        extras=extras,
    )
    pip_download_specs(python_exe, specs, wheelhouse, env)
    pip_install_offline_specs(
        python_exe,
        specs,
        wheelhouse,
        env,
        installer=installer,
        install_attempts=install_attempts,
    )
    return specs


def ensure_gui_agent_checkout(third_party_dir: Path) -> Tuple[Path, str]:
    repo_dir = third_party_dir / GUI_AGENT_DIR_NAME
    if not repo_dir.exists():
        clone_git_repo(GUI_AGENT_REPO_URL, repo_dir)
    return repo_dir, repo_commit(repo_dir)


def apply_git_diff(repo_dir: Path, diff_path: Path) -> str:
    if not diff_path.is_file():
        return "missing"

    check_apply = subprocess.run(
        tool_cmd("git", "-C", str(repo_dir), "apply", "--check", str(diff_path)),
        text=True,
        encoding="utf-8",
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )
    if check_apply.returncode == 0:
        run_cmd(tool_cmd("git", "-C", str(repo_dir), "apply", str(diff_path)))
        return "applied"

    check_reverse = subprocess.run(
        tool_cmd("git", "-C", str(repo_dir), "apply", "--reverse", "--check", str(diff_path)),
        text=True,
        encoding="utf-8",
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )
    if check_reverse.returncode == 0:
        return "already_applied"
    output = (check_apply.stdout or "").strip()
    raise CommandError(f"failed to apply patch: {diff_path}\n{output}")


def install_optional_gui_agent(
    *,
    enabled: bool,
    third_party_dir: Path,
    python_exe: Path,
    wheelhouse: Path,
    env: Dict[str, str],
    installer: str,
    install_attempts: int,
) -> Dict[str, str]:
    if not enabled:
        return {
            "status": "skipped",
            "reason": "not_requested",
            "dir": "",
            "commit": "",
            "diff": "not_requested",
        }

    repo_dir, commit = ensure_gui_agent_checkout(third_party_dir)
    diff_status = apply_git_diff(repo_dir, third_party_dir / "Open-AutoGLM-swipe.diff")
    install_requirement_specs(
        python_exe,
        repo_dir / "requirements.txt",
        wheelhouse,
        env,
        installer=installer,
        install_attempts=install_attempts,
    )
    run_cmd_retry(
        local_package_install_command(installer, python_exe, repo_dir, env=env, no_deps=True),
        cwd=repo_dir,
        env=env,
        attempts=install_attempts,
    )
    return {
        "status": "installed",
        "reason": "requested",
        "dir": str(repo_dir),
        "commit": commit,
        "diff": diff_status,
    }


def bootstrap_hapray(
    spec: ToolSpec,
    verify: bool,
    python_exe: Optional[str],
    registry: Optional[str],
    fetch_timeout: int,
    fetch_retries: int,
    npm_strict_ssl: Optional[str],
    pip_index_url: Optional[str],
    pip_timeout: int,
    pip_retries: int,
    python_package_installer: str,
    with_gui_agent: bool,
) -> Dict[str, str]:
    workspace = workspace_dir(spec)
    if not workspace.is_dir():
        raise FileNotFoundError(f"missing Hapray workspace: {workspace}")

    chosen_python, version = find_hapray_python(python_exe, minimum=(3, 10))
    if not chosen_python:
        return {
            "status": "downloaded",
            "workspaceDir": str(workspace),
            "note": "compatible Python 3.10-3.12 not found; source download completed",
        }

    patch_statuses: Dict[str, str] = {}
    npm_environment = npm_env(registry, fetch_timeout, fetch_retries, npm_strict_ssl)
    run_cmd_retry(
        npm_dependency_install_command(workspace, npm_environment),
        attempts=npm_install_attempts(fetch_retries),
        cwd=workspace,
        env=npm_environment,
    )
    trace_streamer_dir = ensure_trace_streamer_assets(spec.source_dir)
    bootstrap_dir = managed_dir(workspace)
    sa_cmd_path, sa_cmd_js, sa_cmd_cmd, sa_cmd_exe, launcher_compiler, static_analyzer_build = ensure_static_analyzer_assets(
        spec.source_dir,
        bootstrap_dir,
        registry,
        fetch_timeout,
        fetch_retries,
        npm_strict_ssl,
    )
    runtime_assets = ensure_hapray_runtime_assets(
        spec.source_dir,
        registry,
        fetch_timeout,
        fetch_retries,
        npm_strict_ssl,
    )
    tools_layout = ensure_hapray_tools_layout(spec.source_dir, workspace)
    workspace_tools, workspace_sa_cmd, workspace_trace_streamer = ensure_workspace_tool_links(workspace, spec.source_dir)
    ext_tools_dir = spec.source_dir / "tools"
    ensure_hapray_runtime_dirs(workspace)
    runtime_patch_info = determine_hapray_runtime_patch_info(sa_cmd_exe, launcher_compiler)
    runtime_patch_status = runtime_patch_info["status"]
    log(f"Hapray runtime patch decision: {runtime_patch_status} ({runtime_patch_info['reason']})")

    hapray_python = ensure_virtualenv(chosen_python, workspace)
    wheelhouse_dir = bootstrap_dir / "wheelhouse"
    third_party_dir = workspace.parent / "third-party"
    package_environment = python_package_env(
        pip_index_url,
        pip_timeout,
        pip_retries,
        cache_dir=bootstrap_dir / "pip-cache",
    )
    install_backend = resolve_python_package_installer(python_package_installer, env=package_environment)
    install_attempts = python_package_install_attempts(pip_retries)
    log(f"Using Python package installer backend: {install_backend}")
    base_specs = ["wheel", "setuptools<82"]
    pip_download_specs(hapray_python, base_specs, wheelhouse_dir, package_environment)
    pip_install_offline_specs(
        hapray_python,
        base_specs,
        wheelhouse_dir,
        package_environment,
        installer=install_backend,
        install_attempts=install_attempts,
    )

    requirements_path = workspace / "requirements.txt"
    install_requirement_specs(
        hapray_python,
        requirements_path,
        wheelhouse_dir,
        package_environment,
        installer=install_backend,
        install_attempts=install_attempts,
        overrides=HAPRAY_REQUIREMENT_OVERRIDES,
        extras=HAPRAY_EXTRA_SPECS,
    )

    hypium_zip = latest_file(third_party_dir, "hypium-*.zip")
    hypium_dir = extract_zip_if_needed(hypium_zip, workspace / hypium_zip.stem)
    install_local_packages(
        hapray_python,
        hypium_dir,
        package_environment,
        installer=install_backend,
        install_attempts=install_attempts,
    )
    gui_agent_result = install_optional_gui_agent(
        enabled=with_gui_agent,
        third_party_dir=third_party_dir,
        python_exe=hapray_python,
        wheelhouse=wheelhouse_dir,
        env=package_environment,
        installer=install_backend,
        install_attempts=install_attempts,
    )

    if verify:
        verify_env = os.environ.copy()
        prepend_tool_dir_to_path(verify_env, "node")
        run_cmd(sa_cmd_command(sa_cmd_path, "--help"), cwd=workspace, env=verify_env)
        run_cmd(
            [
                str(hapray_python),
                "-c",
                "from hapray.actions.perf_action import PerfAction; PerfAction.execute(['--help'])",
            ],
            cwd=workspace,
            env=verify_env,
        )

    return {
        "status": "installed",
        "workspaceDir": str(workspace),
        "python": chosen_python,
        "pythonVersion": version or "",
        "venvPython": str(hapray_python),
        "wheelhouseDir": str(wheelhouse_dir),
        "pythonPackageInstaller": install_backend,
        "extToolsDir": str(ext_tools_dir),
        "traceStreamerDir": str(trace_streamer_dir),
        "traceStreamer": str(trace_streamer_dir / trace_streamer_executable_name()),
        "saCmd": str(sa_cmd_path),
        "saCmdJs": str(sa_cmd_js),
        "saCmdCmd": str(sa_cmd_cmd),
        "saCmdExe": str(sa_cmd_exe) if sa_cmd_exe else "",
        "launcherCompiler": runtime_patch_info["launcherCompiler"],
        "runtimePatchStatus": runtime_patch_status,
        "runtimePatchReason": runtime_patch_info["reason"],
        "workspaceToolsDir": str(workspace_tools),
        "workspaceSaCmdDir": str(workspace_sa_cmd),
        "workspaceTraceStreamerDir": str(workspace_trace_streamer),
        "toolsLayout": tools_layout,
        "staticAnalyzerBuild": static_analyzer_build,
        "guiAgentStatus": gui_agent_result["status"],
        "guiAgentReason": gui_agent_result["reason"],
        "guiAgentDir": gui_agent_result["dir"],
        "guiAgentCommit": gui_agent_result["commit"],
        "guiAgentDiff": gui_agent_result["diff"],
        "sourcePatches": patch_statuses,
        **runtime_assets,
    }


def write_lock_file(deps_root: Path, payload: Dict[str, object]) -> Path:
    deps_root.mkdir(parents=True, exist_ok=True)
    lock_path = deps_root / LOCK_FILE_NAME
    lock_path.write_text(json.dumps(payload, ensure_ascii=False, indent=2), encoding="utf-8")
    return lock_path


def selected_tools(all_tools: Dict[str, ToolSpec], tool_name: str) -> List[ToolSpec]:
    if tool_name == "all":
        return [all_tools["homecheck"], all_tools["hapray"]]
    return [all_tools[tool_name]]


def existing_lock_payload(deps_root: Path) -> Dict[str, object]:
    lock_path = deps_root / LOCK_FILE_NAME
    if not lock_path.is_file():
        return {}
    try:
        return json.loads(lock_path.read_text(encoding="utf-8"))
    except (json.JSONDecodeError, OSError):
        return {}


def main() -> None:
    parser = argparse.ArgumentParser(description="Bootstrap Homecheck/Hapray dependencies from GitCode")
    parser.add_argument("--manifest", default=str(DEFAULT_MANIFEST), help="dependency manifest path")
    parser.add_argument("--deps-root", default="", help="override dependency root directory")
    parser.add_argument(
        "--tool",
        choices=["all", "homecheck", "hapray"],
        default="all",
        help="tool to process",
    )
    parser.add_argument("--fetch-only", action="store_true", help="only clone/update source repositories")
    parser.add_argument("--verify", action="store_true", help="run lightweight entrypoint verification")
    parser.add_argument("--hapray-python", default="", help="Python executable for Hapray bootstrap")
    parser.add_argument("--npm-registry", default="", help="override npm registry for Homecheck/Hapray npm install")
    parser.add_argument(
        "--npm-strict-ssl",
        choices=["true", "false"],
        default="",
        help="override npm strict-ssl for Homecheck/Hapray npm steps",
    )
    parser.add_argument("--npm-fetch-timeout", type=int, default=60000, help="npm fetch timeout in milliseconds")
    parser.add_argument("--npm-fetch-retries", type=int, default=0, help="npm fetch retries")
    parser.add_argument("--pip-index-url", default="", help="override pip index url for Hapray bootstrap")
    parser.add_argument("--pip-timeout", type=int, default=60, help="pip timeout in seconds")
    parser.add_argument("--pip-retries", type=int, default=5, help="pip retries")
    parser.add_argument(
        "--python-package-installer",
        choices=PYTHON_PACKAGE_INSTALLER_CHOICES,
        default="auto",
        help="python package installer backend for local/offline installs",
    )
    parser.add_argument(
        "--with-gui-agent",
        action="store_true",
        help="also install the optional Open-AutoGLM dependency used by Hapray gui-agent",
    )
    args = parser.parse_args()

    manifest_path = Path(args.manifest).resolve()
    deps_root, tools = load_manifest(manifest_path, deps_root_override=args.deps_root)
    previous_payload = existing_lock_payload(deps_root)
    results: Dict[str, object] = {
        "generatedAt": datetime.now(timezone.utc).isoformat(timespec="seconds").replace("+00:00", "Z"),
        "manifest": str(manifest_path),
        "depsRoot": str(deps_root),
        "tools": dict(previous_payload.get("tools", {})),
    }

    for spec in selected_tools(tools, args.tool):
        commit = ensure_git_checkout(spec)
        tool_result: Dict[str, str] = {
            "repo": spec.repo,
            "ref": spec.ref,
            "commit": commit,
            "sourceDir": str(spec.source_dir),
            "status": "downloaded",
        }
        if not args.fetch_only:
            if spec.name == "homecheck":
                tool_result.update(
                    bootstrap_homecheck(
                        spec,
                        verify=args.verify,
                        registry=args.npm_registry or None,
                        fetch_timeout=args.npm_fetch_timeout,
                        fetch_retries=args.npm_fetch_retries,
                        npm_strict_ssl=args.npm_strict_ssl or None,
                    )
                )
            elif spec.name == "hapray":
                tool_result.update(
                    bootstrap_hapray(
                        spec,
                        verify=args.verify,
                        python_exe=args.hapray_python or None,
                        registry=args.npm_registry or None,
                        fetch_timeout=args.npm_fetch_timeout,
                        fetch_retries=args.npm_fetch_retries,
                        npm_strict_ssl=args.npm_strict_ssl or None,
                        pip_index_url=args.pip_index_url or None,
                        pip_timeout=args.pip_timeout,
                        pip_retries=args.pip_retries,
                        python_package_installer=args.python_package_installer,
                        with_gui_agent=args.with_gui_agent,
                    )
                )
        results["tools"][spec.name] = tool_result

    lock_path = write_lock_file(deps_root, results)
    log(f"dependency lock written to: {lock_path}")


if __name__ == "__main__":
    try:
        main()
    except (CommandError, FileNotFoundError, json.JSONDecodeError) as exc:
        print(str(exc), file=sys.stderr)
        sys.exit(1)
