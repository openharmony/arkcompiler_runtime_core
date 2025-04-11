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

"""Format the IDL files."""

from json import dumps
from typing import TextIO

from typing_extensions import override

from taihe.semantics.declarations import (
    Decl,
    DeclarationImportDecl,
    DeclarationRefDecl,
    DeclProtocol,
    EnumDecl,
    EnumItemDecl,
    GlobFuncDecl,
    IfaceDecl,
    IfaceMethodDecl,
    IfaceParentDecl,
    PackageDecl,
    PackageGroup,
    PackageImportDecl,
    PackageRefDecl,
    ParamDecl,
    StructDecl,
    StructFieldDecl,
    TypeRefDecl,
    UnionDecl,
    UnionFieldDecl,
)
from taihe.semantics.visitor import RecursiveDeclVisitor
from taihe.utils.diagnostics import AnsiStyle
from taihe.utils.outputs import IndentManager


def pretty_print(x: DeclProtocol, buffer: TextIO):
    printer = _PrettyPrinter(buffer)
    printer.handle_decl(x)


class _PrettyPrinter(RecursiveDeclVisitor):
    def __init__(self, buffer: TextIO):
        self.buffer = buffer
        self.indent_manager = IndentManager()

    def writeln(self, content):
        self.buffer.write(self.indent_manager.current + content + "\n")

    def get_type_ref_decl(self, d: TypeRefDecl) -> str:
        if not d.is_resolved:
            return d.unresolved_repr
        real_type = (
            d.maybe_resolved_ty.representation
            if d.maybe_resolved_ty
            else "<error type>"
        )
        return (
            f"{d.unresolved_repr} {AnsiStyle.GREEN}/* {real_type} */{AnsiStyle.RESET}"
        )

    def get_package_ref_decl(self, d: PackageRefDecl) -> str:
        return d.symbol

    def get_decl_ref_decl(self, d: DeclarationRefDecl) -> str:
        return d.symbol

    def get_parent_decl(self, d: IfaceParentDecl) -> str:
        res = self.get_type_ref_decl(d.ty_ref)
        return self.with_attr(d, res)

    def get_param_decl(self, d: ParamDecl) -> str:
        res = f"{d.name}: {self.get_type_ref_decl(d.ty_ref)}"
        return self.with_attr(d, res)

    def get_value(self, d: bool | str | float | int) -> str:
        if isinstance(d, bool):
            return "TRUE" if d else "FALSE"
        return dumps(d)

    def as_keyword(self, s) -> str:
        return f"{AnsiStyle.CYAN}{s}{AnsiStyle.RESET}"

    def get_format_attr(self, d: Decl) -> list[str]:
        formatted_attributes = []
        for key, items in d.attrs.items():
            for item in items:
                if item.args:
                    values_fmt = ", ".join(self.get_value(arg) for arg in item.args)
                    formatted_attributes.append(f"{key}({values_fmt})")
                else:
                    formatted_attributes.append(key)
        return formatted_attributes

    def with_attr(self, d: Decl, s: str) -> str:
        if not d.attrs:
            return s
        fmt_attrs = " ".join(f"@{item}" for item in self.get_format_attr(d))
        attr = f"{AnsiStyle.MAGENTA}{fmt_attrs}{AnsiStyle.RESET_ALL}"
        return f"{attr} {s}"

    def write_pkg_attr(self, d: PackageDecl):
        for item in self.get_format_attr(d):
            attr = f"{AnsiStyle.MAGENTA}@!{item}{AnsiStyle.RESET_ALL}"
            self.writeln(f"{attr}")

    def write_attr(self, d: Decl):
        for item in self.get_format_attr(d):
            attr = f"{AnsiStyle.MAGENTA}@{item}{AnsiStyle.RESET_ALL}"
            self.writeln(f"{attr}")

    @override
    def visit_package_import_decl(self, d: PackageImportDecl):
        self.write_attr(d)

        use_kw = self.as_keyword("use")
        as_kw = self.as_keyword("as")

        as_ = f" {as_kw} {d.name}" if d.is_alias() else ""

        self.writeln(f"{use_kw} {self.get_package_ref_decl(d.pkg_ref)}{as_};")

    @override
    def visit_decl_import_decl(self, d: DeclarationImportDecl):
        self.write_attr(d)

        from_kw = self.as_keyword("from")
        use_kw = self.as_keyword("use")
        as_kw = self.as_keyword("as")

        as_ = f" {as_kw} {d.name}" if d.is_alias() else ""

        self.writeln(
            f"{from_kw} {self.get_package_ref_decl(d.decl_ref.pkg_ref)} {use_kw} {self.get_decl_ref_decl(d.decl_ref)}{as_};"
        )

    @override
    def visit_glob_func_decl(self, d: GlobFuncDecl):
        self.write_attr(d)

        func_kw = self.as_keyword("function")

        fmt_args = ", ".join(self.get_param_decl(x) for x in d.params)
        ret = self.get_type_ref_decl(d.return_ty_ref) if d.return_ty_ref else "void"

        self.writeln(f"{func_kw} {d.name}({fmt_args}): {ret};")

    @override
    def visit_union_field_decl(self, d: UnionFieldDecl):
        self.write_attr(d)

        if d.ty_ref:
            self.writeln(f"{d.name}: {self.get_type_ref_decl(d.ty_ref)};")
        else:
            self.writeln(f"{d.name};")

    @override
    def visit_enum_item_decl(self, d: EnumItemDecl) -> None:
        self.write_attr(d)

        if d.value is None:
            self.writeln(f"{d.name},")
        else:
            self.writeln(f"{d.name} = {self.get_value(d.value)},")

    @override
    def visit_enum_decl(self, d: EnumDecl) -> None:
        self.write_attr(d)

        enum_kw = self.as_keyword("enum")

        full_decl = (
            f"{d.name}: {self.get_type_ref_decl(d.ty_ref)}" if d.ty_ref else d.name
        )

        if d.items:
            self.writeln(f"{enum_kw} {full_decl} {{")
            with self.indent_manager.offset():
                for i in d.items:
                    self.handle_decl(i)
            self.writeln(f"}}")
        else:
            self.writeln(f"{enum_kw} {full_decl} {{}}")

    @override
    def visit_union_decl(self, d: UnionDecl):
        self.write_attr(d)

        union_kw = self.as_keyword("union")

        if d.fields:
            self.writeln(f"{union_kw} {d.name} {{")
            with self.indent_manager.offset():
                for f in d.fields:
                    self.handle_decl(f)
            self.writeln(f"}}")
        else:
            self.writeln(f"{union_kw} {d.name} {{}}")

    @override
    def visit_struct_field_decl(self, d: StructFieldDecl):
        self.write_attr(d)

        self.writeln(f"{d.name}: {self.get_type_ref_decl(d.ty_ref)};")

    @override
    def visit_struct_decl(self, d: StructDecl):
        self.write_attr(d)

        struct_kw = self.as_keyword("struct")

        if d.fields:
            self.writeln(f"{struct_kw} {d.name} {{")
            with self.indent_manager.offset():
                for f in d.fields:
                    self.handle_decl(f)
            self.writeln(f"}}")
        else:
            self.writeln(f"{struct_kw} {d.name} {{}}")

    @override
    def visit_iface_func_decl(self, d: IfaceMethodDecl):
        self.write_attr(d)

        fmt_args = ", ".join(self.get_param_decl(x) for x in d.params)
        ret = self.get_type_ref_decl(d.return_ty_ref) if d.return_ty_ref else "void"

        self.writeln(f"{d.name}({fmt_args}): {ret};")

    @override
    def visit_iface_decl(self, d: IfaceDecl):
        self.write_attr(d)

        iface_kw = self.as_keyword("interface")

        full_decl = (
            f"{d.name}: " + ", ".join(self.get_parent_decl(e) for e in d.parents)
            if d.parents
            else d.name
        )

        if d.methods:
            self.writeln(f"{iface_kw} {full_decl} {{")
            with self.indent_manager.offset():
                for f in d.methods:
                    self.handle_decl(f)
            self.writeln(f"}}")
        else:
            self.writeln(f"{iface_kw} {full_decl} {{}}")

    @override
    def visit_package_decl(self, p: PackageDecl):
        self.writeln(f"// {p.name}")
        self.write_pkg_attr(p)
        for d in p.pkg_imports.values():
            self.handle_decl(d)
        for d in p.decl_imports.values():
            self.handle_decl(d)
        for d in p.structs:
            self.handle_decl(d)
        for d in p.unions:
            self.handle_decl(d)
        for d in p.interfaces:
            self.handle_decl(d)
        for d in p.enums:
            self.handle_decl(d)
        for d in p.functions:
            self.handle_decl(d)

    @override
    def visit_package_group(self, g: PackageGroup):
        for i, p in enumerate(g.packages):
            if i != 0:
                self.writeln(f"")
            self.handle_decl(p)
