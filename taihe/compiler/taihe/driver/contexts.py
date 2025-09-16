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

"""Orchestrates the compilation process.

- BackendRegistry: initializes all known backends
- CompilerInvocation: constructs the invocation from cmdline
    - Parses the general command line arguments
    - Enables user specified backends
    - Parses backend-specific arguments
    - Sets backend options
- CompilerInstance: runs the compilation
    - CompilerInstance: scans and parses sources files
    - Backends: post-process the IR
    - Backends: validate the IR
    - Backends: generate the output
"""

from dataclasses import dataclass, field
from itertools import chain
from pathlib import Path

from typing_extensions import Self

from taihe.driver.backend import Backend, BackendConfig
from taihe.semantics.analysis import analyze_semantics
from taihe.semantics.attributes import AttributeRegistry
from taihe.semantics.declarations import PackageGroup
from taihe.utils.analyses import AnalysisManager
from taihe.utils.diagnostics import ConsoleDiagnosticsManager, DiagnosticsManager
from taihe.utils.exceptions import IgnoredFileReason, IgnoredFileWarn
from taihe.utils.outputs import OutputConfig
from taihe.utils.sources import SourceFile, SourceLocation, SourceManager


def validate_source_file(path: Path) -> IgnoredFileReason | None:
    # not exist
    if not path.exists():
        return IgnoredFileReason.NOT_EXIST
    # subdirectories are ignored
    if not path.is_file():
        return IgnoredFileReason.IS_DIRECTORY
    # unexpected file extension
    if path.suffix != ".taihe":
        return IgnoredFileReason.EXTENSION_MISMATCH
    return None


@dataclass
class CompilerInvocation:
    """Describes the options and intents for a compiler invocation.

    CompilerInvocation stores the high-level intent in a structured way, such
    as the input paths, the target for code generation. Generally speaking, it
    can be considered as the parsed and verified version of a compiler's
    command line flags.

    CompilerInvocation does not manage the internal state. Use
    `CompilerInstance` instead.
    """

    src_files: list[Path] = field(default_factory=lambda: [])
    src_dirs: list[Path] = field(default_factory=lambda: [])
    output_config: OutputConfig = field(default_factory=OutputConfig)
    backends: list[BackendConfig] = field(default_factory=lambda: [])

    extra: dict[str, str | None] = field(default_factory=lambda: {})


# TODO: refactor this
@dataclass
class CompilerConfig:
    sts_keep_name: bool = False
    arkts_module_prefix: str | None = None
    arkts_path_prefix: str | None = None

    @classmethod
    def construct(cls, configure: dict[str, str | None]) -> Self:
        res = cls()
        for config in configure:
            k, *v = config.split("=", 1)
            if k == "sts:keep-name":
                res.sts_keep_name = True
            elif k == "arkts:module-prefix":
                res.arkts_module_prefix = v[0] if v else None
            elif k == "arkts:path-prefix":
                res.arkts_path_prefix = v[0] if v else None
            else:
                raise ValueError(f"unknown codegen config {k!r}")
        return res


class CompilerInstance:
    """Helper class for storing key objects.

    CompilerInstance holds key intermediate objects across the compilation
    process, such as the source manager and the diagnostics manager.

    It also provides utility methods for driving the compilation process.
    """

    invocation: CompilerInvocation
    backends: list[Backend]
    diagnostics_manager: DiagnosticsManager
    source_manager: SourceManager
    package_group: PackageGroup
    analysis_manager: AnalysisManager
    attribute_registry: AttributeRegistry
    config: CompilerConfig

    def __init__(
        self,
        invocation: CompilerInvocation,
        *,
        dm: type[DiagnosticsManager] = ConsoleDiagnosticsManager,
    ):
        self.invocation = invocation
        self.diagnostics_manager = dm()
        self.source_manager = SourceManager()
        self.package_group = PackageGroup()
        self.output_manager = invocation.output_config.construct(self)
        self.attribute_registry = AttributeRegistry()
        self.backends = [backend.construct(self) for backend in invocation.backends]
        self.config = CompilerConfig.construct(invocation.extra)
        self.analysis_manager = AnalysisManager(self.config)

    ##########################
    # The compilation phases #
    ##########################

    def collect(self):
        """Adds all `.taihe` files inside a directory. Subdirectories are ignored."""
        direct = self.invocation.src_files
        scanned = chain.from_iterable(p.iterdir() for p in self.invocation.src_dirs)

        for file in chain(direct, scanned):
            source = SourceFile(file)
            if warning := validate_source_file(file):
                warn = IgnoredFileWarn(
                    reason=warning,
                    loc=SourceLocation(source),
                )
                self.diagnostics_manager.emit(warn)
            else:
                self.source_manager.add_source(source)

    def parse(self):
        from taihe.parse.convert import AstConverter

        for src in self.source_manager.sources:
            conv = AstConverter(src, self.diagnostics_manager)
            with self.diagnostics_manager.capture_error():
                pkg = conv.convert()
                self.package_group.add(pkg)

        for b in self.backends:
            b.post_process()

    def validate(self):
        analyze_semantics(
            self.package_group,
            self.diagnostics_manager,
            self.attribute_registry,
        )

        for b in self.backends:
            b.validate()

    def generate(self):
        if self.diagnostics_manager.has_error:
            return

        for b in self.backends:
            b.generate()

        self.output_manager.post_generate()

    def run(self):
        self.collect()
        self.parse()
        self.validate()
        self.generate()
        return not self.diagnostics_manager.has_error