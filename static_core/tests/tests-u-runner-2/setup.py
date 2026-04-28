#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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
#
import shutil
import subprocess
from pathlib import Path

from setuptools import setup

FALLBACK_VERSION = "1.0"
GIT_BIN = shutil.which("git")


def _get_git_root(project_dir: Path) -> Path | None:
    if GIT_BIN is None:
        return None

    try:
        result = subprocess.run(
            [GIT_BIN, "-C", str(project_dir), "rev-parse", "--show-toplevel"],
            check=False,
            capture_output=True,
            text=True,
        )
    except OSError:
        return None

    if result.returncode != 0:
        return None

    git_root = result.stdout.strip()
    return Path(git_root) if git_root else None


def _is_runner_tree_dirty(project_dir: Path) -> bool:
    if GIT_BIN is None:
        return False

    git_root = _get_git_root(project_dir)
    if git_root is None:
        return False

    try:
        relative_runner_path = project_dir.relative_to(git_root)
    except ValueError:
        return False

    def _has_tracked_changes(args: list[str]) -> bool:
        try:
            result = subprocess.run(args, check=False, capture_output=True, text=True)
        except OSError:
            return False
        # git diff --quiet: return code 1 means differences were found
        return result.returncode == 1

    worktree_changed = _has_tracked_changes(
        [GIT_BIN, "-C", str(git_root), "diff", "--quiet", "--", str(relative_runner_path)]
    )
    index_changed = _has_tracked_changes(
        [GIT_BIN, "-C", str(git_root), "diff", "--cached", "--quiet", "--", str(relative_runner_path)]
    )

    return worktree_changed or index_changed


def _version_scheme(_version: object) -> str:
    return FALLBACK_VERSION


def _local_scheme(version: object) -> str:
    node = getattr(version, "node", "")
    node_str = str(node) if node else ""

    if not node_str:
        return ""

    commit_hash_full = node_str[1:] if node_str.startswith("g") else node_str
    commit_hash = commit_hash_full[:8]
    suffix = "_dev" if _is_runner_tree_dirty(Path(__file__).resolve().parent) else ""
    return f"+g{commit_hash}{suffix}"


setup(
    use_scm_version={
        "root": "../../..",
        "fallback_version": FALLBACK_VERSION,
        "version_scheme": _version_scheme,
        "local_scheme": _local_scheme,
    }
)
