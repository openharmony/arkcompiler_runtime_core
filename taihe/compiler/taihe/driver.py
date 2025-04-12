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

"""Orchestrates the compilation process."""

import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional

from taihe.codegen.abi_generator import (
    ABIHeadersGenerator,
    ABISourcesGenerator,
)
from taihe.codegen.ani_generator import (
    ANICodeGenerator,
)
from taihe.codegen.c_impl_generator import (
    CImplHeadersGenerator,
    CImplSourcesGenerator,
)
from taihe.codegen.cpp_generator import (
    CppHeadersGenerator,
)
from taihe.codegen.cpp_impl_generator import (
    CppImplHeadersGenerator,
    CppImplSourcesGenerator,
)
from taihe.codegen.cpp_user_generator import (
    CppUserHeadersGenerator,
)
from taihe.codegen.sts_generator import (
    STSCodeGenerator,
)
from taihe.parse.convert import (
    AstConverter,
    IgnoredFileReason,
    IgnoredFileWarn,
    normalize_pkg_name,
)
from taihe.semantics.analysis import analyze_semantics
from taihe.semantics.declarations import AttrItemDecl, PackageGroup
from taihe.semantics.format import pretty_print
from taihe.utils.analyses import AnalysisManager
from taihe.utils.diagnostics import DiagnosticsManager
from taihe.utils.exceptions import AdhocNote
from taihe.utils.outputs import OutputManager
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

    # TODO: implement "CompilerBackend" and store the backend-specific options there?
    out_dir: Optional[Path] = None

    gen_author: bool = False
    gen_user: bool = False
    gen_ani: bool = False
    gen_c_impl: bool = False
    gen_cpp_impl: bool = True

    debug: bool = False
    sts_keep_name: bool = False


class CompilerInstance:
    """Helper class for storing key objects.

    CompilerInstance holds key intermediate objects across the compilation
    process, such as the source manager and the diagnostics manager.

    It also provides utility methods for driving the compilation process.
    """

    invocation: CompilerInvocation
    diagnostics_manager: DiagnosticsManager

    source_manager: SourceManager
    package_group: PackageGroup

    analysis_manager: AnalysisManager

    target_manager: OutputManager

    def __init__(self, invocation: CompilerInvocation):
        self.invocation = invocation
        self.source_manager = SourceManager()
        self.diagnostics_manager = DiagnosticsManager()
        self.package_group = PackageGroup()
        self.analysis_manager = AnalysisManager()
        self.target_manager = OutputManager()

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
            with self.diagnostics_manager.capture_error():
                conv = AstConverter(src, self.diagnostics_manager)
                pkg = conv.convert()
                self.package_group.add(pkg)

    def validate(self):
        analyze_semantics(self.package_group, self.diagnostics_manager)

        if self.invocation.debug:
            pretty_print(self.package_group, sys.stdout)

    def generate(self):
        if self.diagnostics_manager.has_errors():
            return

        if not self.invocation.out_dir:
            return

        ABIHeadersGenerator(self.target_manager, self.analysis_manager).generate(
            self.package_group
        )
        CppHeadersGenerator(self.target_manager, self.analysis_manager).generate(
            self.package_group
        )
        if self.invocation.gen_author:
            ABISourcesGenerator(self.target_manager, self.analysis_manager).generate(
                self.package_group
            )
            if self.invocation.gen_cpp_impl:
                CppImplHeadersGenerator(
                    self.target_manager, self.analysis_manager
                ).generate(self.package_group)
                CppImplSourcesGenerator(
                    self.target_manager, self.analysis_manager
                ).generate(self.package_group)
            if self.invocation.gen_c_impl:
                CImplHeadersGenerator(
                    self.target_manager, self.analysis_manager
                ).generate(self.package_group)
                CImplSourcesGenerator(
                    self.target_manager, self.analysis_manager
                ).generate(self.package_group)
        if self.invocation.gen_user or self.invocation.gen_ani:
            CppUserHeadersGenerator(
                self.target_manager, self.analysis_manager
            ).generate(self.package_group)
        if self.invocation.gen_ani:
            ANICodeGenerator(self.target_manager, self.analysis_manager).generate(
                self.package_group
            )
            STSCodeGenerator(self.target_manager, self.analysis_manager).generate(
                self.package_group
            )

        self.target_manager.output_to(self.invocation.out_dir)

    def add_pkg_attr(self):
        if not self.invocation.sts_keep_name:
            return
        for pkg in self.package_group.packages:
            if not pkg.get_attr_item("sts_keep_name"):
                d = AttrItemDecl(None, "sts_keep_name", ())
                pkg.add_attr(d)

    def run(self):
        self.diagnostics_manager.reset_max_level()

        self.scan()
        self.parse()
        self.validate()
        self.add_pkg_attr()
        self.generate()

        return not self.diagnostics_manager.has_errors()
