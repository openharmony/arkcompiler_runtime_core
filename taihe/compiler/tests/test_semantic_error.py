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

from taihe.parse.convert import (
    AstConverter,
)
from taihe.semantics.analysis import analyze_semantics
from taihe.semantics.declarations import PackageGroup
from taihe.utils.diagnostics import AbstractDiagnosticsManager, DiagBase
from taihe.utils.exceptions import (
    DeclarationNotInScopeError,
    DeclNotExistError,
    DeclRedefError,
    DuplicateExtendsWarn,
    IDLSyntaxError,
    NotATypeError,
    PackageNotExistError,
    PackageNotInScopeError,
    RecursiveReferenceError,
    SymbolConflictWithNamespaceError,
    TypeUsageError,
)
from taihe.utils.sources import SourceBuffer, SourceManager


class SemanticTestDiagnosticsManager(AbstractDiagnosticsManager):
    errors: list[DiagBase]

    def __init__(self):
        self.errors = []

    def emit(self, diag: DiagBase) -> None:
        self.errors.append(diag)


class SemanticTestCompilerInstance:
    diagnostics_manager: SemanticTestDiagnosticsManager

    source_manager: SourceManager
    package_group: PackageGroup

    test_buffers: list[tuple[str, str]]

    def __init__(self):
        self.source_manager = SourceManager()
        self.diagnostics_manager = SemanticTestDiagnosticsManager()
        self.package_group = PackageGroup()
        self.test_buffers = []

    def add_source(self, pkg_name, source):
        self.test_buffers.append((pkg_name, source))

    def collect(self):
        for pkg_name, source in self.test_buffers:
            self.source_manager.add_source(SourceBuffer(pkg_name, source))

    def parse(self):
        for src in self.source_manager.sources:
            conv = AstConverter(src, self.diagnostics_manager)
            pkg = conv.convert()
            with self.diagnostics_manager.capture_error():
                self.package_group.add(pkg)

    def validate(self):
        analyze_semantics(self.package_group, self.diagnostics_manager)

    def run(self):
        self.collect()
        self.parse()
        self.validate()

    def assert_has_error(self, error_type: type[DiagBase]):
        if not any(isinstance(err, error_type) for err in self.diagnostics_manager.errors):
            raise ValueError("No errors of the specified type found in diagnostics.")


def test_package_not_exist():
    # fmt: off
    test_instance = SemanticTestCompilerInstance()
    test_instance.add_source(
        "package",
        "use a;\n"
    )
    test_instance.run()
    test_instance.assert_has_error(PackageNotExistError)


def test_package_not_in_scope_1():
    # fmt: off
    test_instance = SemanticTestCompilerInstance()
    test_instance.add_source(
        "package",
        "struct BadStruct {\n"
        "    a: unimported.package.Type;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(PackageNotInScopeError)


def test_package_not_in_scope_2():
    # fmt: off
    test_instance = SemanticTestCompilerInstance()
    test_instance.add_source(
        "package",
        "struct A {\n"
        "    a: i32;\n"
        "}\n"
        "struct B{\n"
        "    b: A.Type;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(PackageNotInScopeError)


def test_package_not_in_scope_3():
    # fmt: off
    test_instance = SemanticTestCompilerInstance()
    test_instance.add_source(
        "package",
        "from package.example use B;\n"
        "struct A {\n"
        "    a: B.Type;\n"
        "}\n"
    )
    test_instance.add_source(
        "package.example",
        "struct B{\n"
        "    b: i32;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(PackageNotInScopeError)


def test_decl_redef_1():
    # fmt: off
    test_instance = SemanticTestCompilerInstance()
    test_instance.add_source(
        "package",
        "function bad_func(a: i32, a: i32);\n"
    )
    test_instance.run()
    test_instance.assert_has_error(DeclRedefError)


def test_decl_redef_2():
    # fmt: off
    test_instance = SemanticTestCompilerInstance()
    test_instance.add_source(
        "package",
        "from package.example1 use A;\n"
        "from package.example2 use A;\n"
    )
    test_instance.add_source(
        "package.example1",
        "struct A {\n"
        "    a: i32;\n"
        "}\n"
    )
    test_instance.add_source(
        "package.example2",
        "struct A {\n"
        "    a: i32;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(DeclRedefError)


def test_decl_redef_3():
    # fmt: off
    test_instance = SemanticTestCompilerInstance()
    test_instance.add_source(
        "package",
        "use package.example1 as example;\n"
        "use package.example2 as example;\n"
    )
    test_instance.add_source("package.example1", "")
    test_instance.add_source("package.example2", "")
    test_instance.run()
    test_instance.assert_has_error(DeclRedefError)


def test_decl_redef_4():
    # fmt: off
    test_instance = SemanticTestCompilerInstance()
    test_instance.add_source(
        "package",
        "struct BadStruct {\n"
        "    a: i32;\n"
        "    a: i32;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(DeclRedefError)


def test_package_redef():
    # fmt: off
    test_instance = SemanticTestCompilerInstance()
    test_instance.add_source("package", "")
    test_instance.add_source("package", "")
    test_instance.run()
    test_instance.assert_has_error(DeclRedefError)


def test_symbol_conflict_namespace():
    # fmt: off
    test_instance = SemanticTestCompilerInstance()
    test_instance.add_source(
        "package",
        "use package.example1.a;\n"
    )
    test_instance.add_source(
        "package.example1",
        "struct a {\n"
        "    A: String;\n"
        "}\n"
    )
    test_instance.add_source("package.example1.a", "")
    test_instance.run()
    test_instance.assert_has_error(SymbolConflictWithNamespaceError)


def test_decl_not_exist_1():
    # fmt: off
    test_instance = SemanticTestCompilerInstance()
    test_instance.add_source(
        "package",
        "from package.example1 use A;\n"
    )
    test_instance.add_source("package.example1", "")
    test_instance.run()
    test_instance.assert_has_error(DeclNotExistError)


def test_decl_not_exist_2():
    # fmt: off
    test_instance = SemanticTestCompilerInstance()
    test_instance.add_source(
        "package",
        "use package.example1;\n"
        "struct BadStruct {\n"
        "    a: package.example1.B;\n"
        "}\n"
    )
    test_instance.add_source(
        "package.example1",
        "struct A {\n"
        "    a: i32;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(DeclNotExistError)


def test_declaration_not_in_scope_1():
    # fmt: off
    test_instance = SemanticTestCompilerInstance()
    test_instance.add_source(
        "package",
        "struct BadStruct {\n"
        "    a: UnimportedType;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(DeclarationNotInScopeError)


def test_declaration_not_in_scope_2():
    # fmt: off
    test_instance = SemanticTestCompilerInstance()
    test_instance.add_source(
        "package",
        "use package.example1 as example\n"
        "struct A {\n"
        "    a: example;\n"
        "}\n"
    )
    test_instance.add_source("package.example1", "")
    test_instance.run()
    test_instance.assert_has_error(DeclarationNotInScopeError)


def test_recursive_inclusion():
    # fmt: off
    test_instance = SemanticTestCompilerInstance()
    test_instance.add_source(
        "package",
        "struct A {\n"
        "    a: B;\n"
        "}\n"
        "struct B {\n"
        "    a: C;\n"
        "}\n"
        "struct C {\n"
        "    a: A;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(RecursiveReferenceError)


def test_extends_type():
    # fmt: off
    test_instance = SemanticTestCompilerInstance()
    test_instance.add_source(
        "package",
        "interface BadIface: i32 {}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(TypeUsageError)


def test_duplicate_extends():
    # fmt: off
    test_instance = SemanticTestCompilerInstance()
    test_instance.add_source(
        "package",
        "interface TestIface {}\n"
        "interface BadIface: TestIface, TestIface {}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(DuplicateExtendsWarn)


def test_idl_syntax():
    # fmt: off
    test_instance = SemanticTestCompilerInstance()
    test_instance.add_source(
        "package",
        "struct A {\n"
        "    a: $;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(IDLSyntaxError)


def test_not_a_type():
    # fmt: off
    test_instance = SemanticTestCompilerInstance()
    test_instance.add_source(
        "package",
        "function good_func(a: i32): void;\n"
        "struct A {\n"
        "    a: good_func;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(NotATypeError)
