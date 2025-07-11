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

from io import StringIO

import pytest
from typing_extensions import override

from taihe.driver.backend import BackendRegistry
from taihe.driver.contexts import CompilerInstance, CompilerInvocation
from taihe.utils.diagnostics import DiagBase, DiagnosticsManager
from taihe.utils.exceptions import (
    AdhocError,
    DeclarationNotInScopeError,
    DeclNotExistError,
    DeclRedefError,
    DuplicateExtendsWarn,
    GenericArgumentsError,
    IDLSyntaxError,
    InvalidPackageNameError,
    NotATypeError,
    PackageNotExistError,
    PackageNotInScopeError,
    RecursiveReferenceError,
    SymbolConflictWithNamespaceError,
    TypeUsageError,
)
from taihe.utils.sources import SourceBuffer


class SemanticTestDiagnosticsManager(DiagnosticsManager):
    errors: list[DiagBase]

    def __init__(self):
        self.errors = []

    @override
    def emit(self, diag: DiagBase) -> None:
        self.errors.append(diag)


class SemanticTestCompilerInstance(CompilerInstance):
    def __init__(self, invocation: CompilerInvocation):
        super().__init__(invocation, dm=SemanticTestDiagnosticsManager)

    def add_source(self, pkg_name: str, source: str):
        self.source_manager.add_source(SourceBuffer(pkg_name, StringIO(source)))

    @override
    def collect(self):
        pass

    def assert_has_error(self, ty: type[DiagBase]):
        pass
        if all(not isinstance(err, ty) for err in self.diagnostics_manager.errors):
            print(f"Known: {self.diagnostics_manager.errors}")
            pytest.fail(f"Assertion mismatch: expect {ty}")


backend_registry = BackendRegistry()
backend_registry.register_all()


#############################
# Tests for semantic errors #
#############################


pre_backend_names = ["pretty-print"]
pre_invocation = CompilerInvocation(
    backends=[
        b() for b in backend_registry.collect_required_backends(pre_backend_names)
    ],
)


def test_invalid_package_name():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source("package.1", "")
    test_instance.run()
    test_instance.assert_has_error(InvalidPackageNameError)


def test_generic_arguments():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "struct A {\n"
        "    a: Array<>;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(GenericArgumentsError)


def test_package_not_exist():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "use a;\n"
    )
    test_instance.run()
    test_instance.assert_has_error(PackageNotExistError)


def test_package_not_in_scope_1():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
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
    test_instance = SemanticTestCompilerInstance(pre_invocation)
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
    test_instance = SemanticTestCompilerInstance(pre_invocation)
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
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "function bad_func(a: i32, a: i32);\n"
    )
    test_instance.run()
    test_instance.assert_has_error(DeclRedefError)


def test_decl_redef_2():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
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
    test_instance = SemanticTestCompilerInstance(pre_invocation)
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
    test_instance = SemanticTestCompilerInstance(pre_invocation)
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
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source("package", "")
    test_instance.add_source("package", "")
    test_instance.run()
    test_instance.assert_has_error(DeclRedefError)


def test_symbol_conflict_namespace():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
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
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "from package.example1 use A;\n"
    )
    test_instance.add_source("package.example1", "")
    test_instance.run()
    test_instance.assert_has_error(DeclNotExistError)


def test_decl_not_exist_2():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
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
    test_instance = SemanticTestCompilerInstance(pre_invocation)
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
    test_instance = SemanticTestCompilerInstance(pre_invocation)
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
    test_instance = SemanticTestCompilerInstance(pre_invocation)
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
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "interface BadIface: i32 {}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(TypeUsageError)


def test_duplicate_extends():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "interface TestIface {}\n"
        "interface BadIface: TestIface, TestIface {}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(DuplicateExtendsWarn)


def test_idl_syntax():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(pre_invocation)
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
    test_instance = SemanticTestCompilerInstance(pre_invocation)
    test_instance.add_source(
        "package",
        "function good_func(a: i32): void;\n"
        "struct A {\n"
        "    a: good_func;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(NotATypeError)


#################################
# Tests for ANI-specific errors #
#################################


ani_backend_names = ["cpp-author", "ani-bridge", "pretty-print"]
ani_invocation = CompilerInvocation(
    backends=[
        b() for b in backend_registry.collect_required_backends(ani_backend_names)
    ],
)


def test_namespace():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "@!namespace(0)\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_iface_set_1():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "interface IFoo {\n"
        "    @set SetName(): String;\n"
        "}"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_iface_set_2():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "interface IFoo {\n"
        "    @set Name(name: String): void;\n"
        "}"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_set_1():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "interface IFoo {\n"
        "    f(): String;\n"
        "}\n"
        "@set\n"
        '@static("IFoo")\n'
        "function setName(): String;\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_set_2():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "interface IFoo {\n"
        "    f(): String;\n"
        "}\n"
        "@set\n"
        '@static("IFoo")\n'
        "function name(name: String): void;\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_iface_get_1():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "interface IFoo {\n"
        "    @get GetName(name: String): void;\n"
        "}"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_iface_get_2():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "interface IFoo {\n"
        "    @get Name(): String;\n"
        "}"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_get_1():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "interface IFoo {\n"
        "    f(): String;\n"
        "}\n"
        "@get\n"
        '@static("IFoo")\n'
        "function getName(name: String): void;\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_get_2():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "interface IFoo {\n"
        "    f(): String;\n"
        "}\n"
        "@get\n"
        '@static("IFoo")\n'
        "function name(): String;\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_onoff_1():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "@on_off\n"
        "function a(): void;\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_onoff_2():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "@on_off(0)\n"
        "function a(): void;\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_onoff_overload_1():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "@on_off\n"
        "@overload\n"
        "function ona(): void;\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_onoff_overload_2():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "@on_off\n"
        '@overload("reon")\n'
        "function ona(): void;\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_iface_onoff_1():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "interface IFoo {\n"
        "    @on_off(0)\n"
        "    a(): void;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_iface_onoff_2():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "interface IFoo {\n"
        "    @on_off\n"
        "    a(): void;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_iface_onoff_overload_1():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "interface IFoo {\n"
        "    @on_off\n"
        "    @overload\n"
        "    a(): void;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_iface_onoff_overload_2():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "interface IFoo {\n"
        "    @on_off\n"
        '    @overload("reon")\n'
        "    a(): void;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_iface_overload():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "interface IFoo {\n"
        "    @overload\n"
        "    a(): void;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_iface_async_overload():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "interface IFoo {\n"
        "    @gen_async\n"
        "    @overload\n"
        "    a(): void;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_iface_async():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "interface IFoo {\n"
        "    @gen_async\n"
        "    a(): void;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_iface_promise_overload():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "interface IFoo {\n"
        "    @gen_promise\n"
        "    @overload\n"
        "    a(): void;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_iface_promise():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "interface IFoo {\n"
        "    @gen_promise\n"
        "    a(): void;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_async_overload():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "@gen_async\n"
        "@overload\n"
        "function a(): void;\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_async():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "@gen_async\n"
        "function a(): void;\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_promise_overload():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "@gen_promise\n"
        "@overload\n"
        "function a(): void;\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_promise():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "@gen_promise\n"
        "function a(): void;\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_overload():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "@overload\n"
        "function a(): void;\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_ctor():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "@ctor\n"
        "function f(): String;\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_static():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "@static\n"
        "function f(): String;\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_bigint():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "struct A {\n"
        "    a: @bigint Array<f32>;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_typedarray():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "struct A {\n"
        "    a: @typedarray Array<String>;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_arraybuffer():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "struct A {\n"
        "    a: @arraybuffer Array<u64>;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_sts_type():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "struct A {\n"
        '    a: @sts_type("xxx", "yyy") Opaque;\n'
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_struct_extend():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "@class\n"
        "struct A {\n"
        "    @extends base: i32;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_const_enum():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "@const\n"
        "enum E: i32 {\n"
        "    A = 1,\n"
        "    B = 2,\n"
        "}\n"
        "function f(a: E): void;\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_const():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "enum A: f32 {}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_union():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "union U {\n"
        "    a;\n"
        "}\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)


def test_map():
    # fmt: off
    test_instance = SemanticTestCompilerInstance(ani_invocation)
    test_instance.add_source(
        "package",
        "function a(x: Map<String, i32>): Map<String, i32>;\n"
    )
    test_instance.run()
    test_instance.assert_has_error(AdhocError)