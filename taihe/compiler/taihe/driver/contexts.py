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
from pathlib import Path

from taihe.driver.backend import Backend, BackendConfig
from taihe.parse.convert import (
    AstConverter,
    IgnoredFileReason,
    IgnoredFileWarn,
    normalize_pkg_name,
)
from taihe.semantics.analysis import analyze_semantics
from taihe.semantics.declarations import PackageGroup
from taihe.utils.analyses import AnalysisManager
from taihe.utils.diagnostics import ConsoleDiagnosticsManager, DiagnosticsManager, Level
from taihe.utils.exceptions import AdhocNote
from taihe.utils.outputs import DebugLevel, OutputConfig
from taihe.utils.sources import SourceFile, SourceLocation, SourceManager


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

    src_dirs: list[Path] = field(default_factory=lambda: [])
    out_dir: Path | None = None
    out_debug_level: DebugLevel = DebugLevel.NONE
    backends: list[BackendConfig] = field(default_factory=lambda: [])


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

    output_config: OutputConfig

    def __init__(self, invocation: CompilerInvocation):
        self.invocation = invocation
        self.diagnostics_manager = ConsoleDiagnosticsManager()
        self.analysis_manager = AnalysisManager(self.diagnostics_manager)
        self.source_manager = SourceManager()
        self.package_group = PackageGroup()
        self.output_config = OutputConfig(
            self.invocation.out_dir,
            self.invocation.out_debug_level,
        )
        self.backends = [conf.construct(self) for conf in invocation.backends]

    ##########################
    # The compilation phases #
    ##########################

    def scan(self):
        """Adds all `.taihe` files inside a directory. Subdirectories are ignored."""
        for src_dir in self.invocation.src_dirs:
            d = Path(src_dir)
            for file in d.iterdir():
                loc = SourceLocation.with_path(file)
                # subdirectories are ignored
                if not file.is_file():
                    w = IgnoredFileWarn(IgnoredFileReason.IS_DIRECTORY, loc=loc)
                    self.diagnostics_manager.emit(w)

                # unexpected file extension
                elif file.suffix != ".taihe":
                    target = d.with_suffix(".taihe").name
                    w = IgnoredFileWarn(
                        IgnoredFileReason.EXTENSION_MISMATCH,
                        loc=loc,
                        note=AdhocNote(f"consider renaming to `{target}`", loc=loc),
                    )
                    self.diagnostics_manager.emit(w)

                else:
                    source = SourceFile(file)
                    orig_name = source.pkg_name
                    norm_name = normalize_pkg_name(orig_name)

                    # invalid package name
                    if norm_name != orig_name:
                        loc = SourceLocation(source)
                        self.diagnostics_manager.emit(
                            IgnoredFileWarn(
                                IgnoredFileReason.INVALID_PKG_NAME,
                                note=AdhocNote(
                                    f"consider using `{norm_name}` instead of `{orig_name}`",
                                    loc=loc,
                                ),
                                loc=loc,
                            )
                        )

                    # Okay...
                    else:
                        self.source_manager.add_source(source)

    def parse(self):
        for src in self.source_manager.sources:
            conv = AstConverter(src, self.diagnostics_manager)
            pkg = conv.convert()
            with self.diagnostics_manager.capture_error():
                self.package_group.add(pkg)

        for b in self.backends:
            b.post_process()

    def validate(self):
        analyze_semantics(self.package_group, self.diagnostics_manager)

        for b in self.backends:
            b.validate()

    def generate(self):
        if not self.invocation.out_dir:
            return

        if self.diagnostics_manager.current_max_level >= Level.ERROR:
            return

        for b in self.backends:
            b.generate()

    def run(self):
        self.scan()
        self.parse()
        self.validate()
        self.generate()
        return not self.diagnostics_manager.current_max_level >= Level.ERROR