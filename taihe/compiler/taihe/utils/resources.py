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

"""Tools for tracking resources."""

import logging
import shutil
import subprocess
from collections.abc import Callable
from dataclasses import dataclass, field
from enum import Enum, auto
from pathlib import Path
from typing import Final


class DeploymentMode(Enum):
    DEV = auto()  # Inside the Git repository (repo_root)
    PKG = auto()  # pip install ... (site-packages/taihe)
    BUNDLE = auto()  # Bundled with a python executable (taihe-pkg)


# TODO: CLI: override and print the paths
class ResourceType(Enum):
    """Identifier of resources."""

    RUNTIME_SOURCE = "runtime-source"
    RUNTIME_HEADER = "runtime-header"
    STDLIB = "stdlib"
    DOCUMENTATION = "doc"

    # Things that should not be copied to packages should be prefixed with "DEV_"
    DEV_PANDA_VM = "panda-vm"
    DEV_ANTLR = "antlr"
    DEV_PYTHON_BUILD = "python-build"

    def is_packagable(self) -> bool:
        return not self.name.startswith("DEV_")


ResourceT = str | Path | Callable[["ResourceLocator"], Path]


class CacheManager:
    def __init__(self, root_dir: Path):
        self.root_dir = Path(root_dir)
        self.root_dir.mkdir(parents=True, exist_ok=True)

    def get_cache_dir(self, cache_key: str) -> Path:
        cache_dir = self.root_dir / cache_key
        cache_dir.mkdir(exist_ok=True)
        return cache_dir

    def fetch_url(
        self,
        cache_key: str,
        url: str,
        filename: str = "",
        curl_extra_args: list[str] | None = None,
        force_download: bool = False,
    ) -> Path:
        """Fetches a simple URL to a local cache directory if it doesn't already exist.

        Args:
            url: The simple, file-based URL to fetch (e.g., "http://example.com/foo.zip")
            cache_key: Cache directory key
            filename: Optional custom filename for the downloaded file
            curl_extra_args: Additional arguments to pass to curl
            force_download: If True, download even if file already exists

        Returns:
            Path to the downloaded file

        Raises:
            ValueError: If URL or cache_dir is invalid
            subprocess.CalledProcessError: If curl command fails
            FileNotFoundError: If curl is not available

        Example:
            fetch_url("hxxp://example.com/foo.zip", cache_dir="bar")
            --> Downloads to and returns "/path/to/root/.cache/bar/foo.zip"

            fetch_url("hxxp://example.com/foo.zip", cache_dir="bar", filename="baz.zip")
            --> Downloads to and returns "/path/to/root/.cache/bar/baz.zip"
        """
        if not filename:
            filename = url.split("/")[-1]
        output_path = self.get_cache_dir(cache_key) / filename

        # Check if file already exists and we're not forcing download
        if output_path.exists() and not force_download:
            logging.debug(f"Skip fetching, already exists: {output_path}")
            return output_path

        # Prepare curl args
        curl_args = [
            "curl",
            "--location",  # Follow redirects
            "--fail",  # Fail on HTTP errors
            "--progress-bar",
            "--retry",
            "5",  # Retry for 5 times
            "--output",
            str(output_path),
            *(curl_extra_args or []),
            url,
        ]
        ok = False
        try:
            logging.info(f"Downloading {url} to {output_path}")
            subprocess.run(curl_args, check=True)
            logging.info(f"Successfully downloaded to {output_path}")
            ok = True
            return output_path
        except subprocess.CalledProcessError as e:
            logging.error(f"curl failed for {url}")
            raise RuntimeError(f"Failed to download {url}") from e
        except FileNotFoundError as e:
            raise FileNotFoundError("curl command not found.") from e
        finally:
            if not ok:
                output_path.unlink(missing_ok=True)

    def fetch_git_repo(
        self, cache_key: str, url: str, force_refresh: bool = False
    ) -> Path:
        """Clone or update a git repository."""
        repo_dir = self.get_cache_dir(cache_key)

        def git(args: list[str]):
            try:
                subprocess.run(["git", *args], cwd=repo_dir, check=True)
            except subprocess.CalledProcessError as e:
                logging.error(f"Git command failed: {e}")
                raise

        if force_refresh and repo_dir.exists():
            shutil.rmtree(repo_dir)
            repo_dir.mkdir()

        if not (repo_dir / ".git").exists():
            logging.info(f"Downloading repo from {url}")
            git(["clone", url, str(repo_dir)])
        return repo_dir


@dataclass
class ResourceLocator:
    mode: DeploymentMode
    root_dir: Path = field(default_factory=Path)
    caches: CacheManager = field(init=False)

    # Path means a overridden value.
    # str means a pre-configured relative path.
    _layout: dict[ResourceType, ResourceT] = field(init=False)

    def __post_init__(self):
        # Clone the configuration for later modification.
        self._layout = dict(_MODE_TO_LAYOUT[self.mode])
        self.caches = CacheManager(self._get_cache_root())

    def _get_cache_root(self) -> Path:
        cache_dirs = {
            DeploymentMode.DEV: self.root_dir / ".cache",
            DeploymentMode.PKG: Path("~/.cache/taihe").expanduser(),
            DeploymentMode.BUNDLE: self.root_dir / "cache",
        }
        return cache_dirs[self.mode]

    def get(self, t: ResourceType) -> Path:
        descriptor = self._layout[t]
        match descriptor:
            case Path():
                return descriptor
            case str():
                resolved = self.root_dir / descriptor
            case Callable():
                resolved = descriptor(self)
        self._layout[t] = resolved
        return resolved

    def override(self, t: ResourceType, p: Path):
        self._layout[t] = p.resolve()

    @classmethod
    def detect(cls, file: str = __file__):
        # The directory looks like:
        #   7    6     5   4      3            2         1     0
        #                      repo_root/     compiler/taihe/utils/resources.py
        #           .venv/lib/python3.12/site-packages/taihe/utils/resources.py
        # taihe/lib/ pyrt/lib/python3.11/site-packages/taihe/utils/resources.py
        #            ^^^^                ^^^^^^^^^^^^^
        #     python_runtime_dir            repo_dir
        #
        # We use the heuristics based on the name of repository and python runtime.
        DEPTH_REPO = 2
        DEPTH_PYRT = 5
        DEPTH_PKG_ROOT = 7
        parents = Path(file).absolute().parents

        def get(i: int) -> Path:
            if i < len(parents):
                return parents[i]
            return Path()

        repo_dir = get(DEPTH_REPO)
        if repo_dir.name == "compiler":
            return ResourceLocator(DeploymentMode.DEV, get(DEPTH_REPO + 1))

        if repo_dir.name == "site-packages":
            if get(DEPTH_PYRT).name == BUNDLE_PYTHON_RUNTIME_DIR_NAME:
                return ResourceLocator(DeploymentMode.BUNDLE, get(DEPTH_PKG_ROOT))
            else:
                return ResourceLocator(DeploymentMode.PKG, get(DEPTH_REPO - 1))

        raise RuntimeError(f"cannot determine deployment layout ({repo_dir=})")


BUNDLE_PYTHON_RUNTIME_DIR_NAME: Final = "pyrt"

ANTLR_VERSION: Final = "4.13.2"
ANTLR_MAVEN_REPO: Final = "https://mirrors.huaweicloud.com/repository/maven"

PYTHON_REPO_URL: Final = "https://gitee.com/ASeaSalt/python-multi-platform.git"


def _resolve_antlr(locator: ResourceLocator) -> Path:
    url = f"{ANTLR_MAVEN_REPO}/org/antlr/antlr4/{ANTLR_VERSION}/antlr4-{ANTLR_VERSION}-complete.jar"
    return locator.caches.fetch_url(cache_key="antlr", url=url)


def _resolve_python_build(locator: ResourceLocator) -> Path:
    return locator.caches.fetch_git_repo(
        cache_key="python-packages", url=PYTHON_REPO_URL
    )


_MODE_TO_LAYOUT: Final[dict[DeploymentMode, dict[ResourceType, ResourceT]]] = {
    DeploymentMode.DEV: {
        ResourceType.RUNTIME_SOURCE: "runtime/src",
        ResourceType.RUNTIME_HEADER: "runtime/include",
        ResourceType.STDLIB: "stdlib",
        ResourceType.DOCUMENTATION: "cookbook",
        ResourceType.DEV_PANDA_VM: ".panda_vm",
        ResourceType.DEV_ANTLR: _resolve_antlr,
        ResourceType.DEV_PYTHON_BUILD: _resolve_python_build,
    },
    # Python packaging is not supported yet
    DeploymentMode.PKG: {},
    DeploymentMode.BUNDLE: {
        ResourceType.RUNTIME_SOURCE: "src/taihe/runtime",
        ResourceType.RUNTIME_HEADER: "include",
        ResourceType.STDLIB: "lib/taihe/stdlib",
        ResourceType.DOCUMENTATION: "share/doc/taihe",
        ResourceType.DEV_PANDA_VM: "var/lib/panda_vm",
    },
}