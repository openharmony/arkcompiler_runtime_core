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

from collections.abc import Callable
from typing import TYPE_CHECKING, Any

from typing_extensions import override

from taihe.semantics.visitor import DeclVisitor
from taihe.utils.diagnostics import AnsiStyle
from taihe.utils.outputs import BaseWriter

if TYPE_CHECKING:
    from taihe.semantics.declarations import (
        CallbackTypeRefDecl,
        Decl,
        DeclarationImportDecl,
        DeclarationRefDecl,
        EnumDecl,
        EnumItemDecl,
        GenericTypeRefDecl,
        GlobFuncDecl,
        IfaceDecl,
        IfaceMethodDecl,
        IfaceParentDecl,
        LongTypeRefDecl,
        PackageDecl,
        PackageGroup,
        PackageImportDecl,
        PackageRefDecl,
        ParamDecl,
        ShortTypeRefDecl,
        StructDecl,
        StructFieldDecl,
        TypeRefDecl,
        UnionDecl,
        UnionFieldDecl,
    )

WrapF = Callable[[str], str]


class PrettyFormatter(DeclVisitor[str]):
    as_keyword: WrapF
    as_attr: WrapF
    as_comment: WrapF

    def __init__(self, show_resolved: bool = False, colorize: bool = False):
        self.show_resolved = show_resolved
        if colorize:
            self.as_keyword = lambda s: f"{AnsiStyle.CYAN}{s}{AnsiStyle.RESET}"
            self.as_attr = lambda s: f"{AnsiStyle.MAGENTA}{s}{AnsiStyle.RESET}"
            self.as_comment = lambda s: f"{AnsiStyle.GREEN}{s}{AnsiStyle.RESET}"
        else:
            self.as_keyword = lambda s: s
            self.as_attr = lambda s: s
            self.as_comment = lambda s: s

    def with_attr(self, d: "Decl", s: str) -> str:
        if not d.attrs:
            return s
        fmt_attrs = " ".join(
            self.as_attr(f"@{item}") for item in self.get_format_attr(d)
        )
        return f"{fmt_attrs} {s}"

    def get_type_ref_decl(self, d: "TypeRefDecl"):
        type_ref_repr = self.handle_decl(d)
        if not d.is_resolved or not self.show_resolved:
            return type_ref_repr
        real_type = (
            d.maybe_resolved_ty.signature if d.maybe_resolved_ty else "<error type>"
        )
        comment = self.as_comment(f"/* {real_type} */")
        return f"{type_ref_repr} {comment}"

    @override
    def visit_long_type_ref_decl(self, d: "LongTypeRefDecl") -> str:
        return self.with_attr(d, f"{d.pkname}.{d.symbol}")

    @override
    def visit_short_type_ref_decl(self, d: "ShortTypeRefDecl") -> str:
        return self.with_attr(d, d.symbol)

    @override
    def visit_generic_type_ref_decl(self, d: "GenericTypeRefDecl") -> str:
        args_fmt = ", ".join(map(self.get_type_ref_decl, d.args_ty_ref))
        return self.with_attr(d, f"{d.symbol}<{args_fmt}>")

    @override
    def visit_callback_type_ref_decl(self, d: "CallbackTypeRefDecl") -> str:
        fmt_args = ", ".join(map(self.get_param_decl, d.params))
        ret = self.get_type_ref_decl(d.return_ty_ref) if d.return_ty_ref else "void"
        return self.with_attr(d, f"({fmt_args}) => {ret}")

    def get_package_ref_decl(self, d: "PackageRefDecl") -> str:
        package_ref_repr = d.symbol
        if not d.is_resolved or not self.show_resolved:
            return package_ref_repr
        real_package = (
            d.maybe_resolved_pkg.description
            if d.maybe_resolved_pkg
            else "<error package>"
        )
        comment = self.as_comment(f"/* {real_package} */")
        return f"{package_ref_repr} {comment}"

    def get_declaration_ref_decl(self, d: "DeclarationRefDecl") -> str:
        decl_ref_repr = d.symbol
        if not d.is_resolved or not self.show_resolved:
            return decl_ref_repr
        real_decl = (
            d.maybe_resolved_decl.description
            if d.maybe_resolved_decl
            else "<error declaration>"
        )
        comment = self.as_comment(f"/* {real_decl} */")
        return f"{decl_ref_repr} {comment}"

    def get_parent_decl(self, d: "IfaceParentDecl") -> str:
        res = self.get_type_ref_decl(d.ty_ref)
        return self.with_attr(d, res)

    def get_param_decl(self, d: "ParamDecl") -> str:
        res = f"{d.name}: {self.get_type_ref_decl(d.ty_ref)}"
        return self.with_attr(d, res)

    def get_value(self, obj: Any) -> str:
        if isinstance(obj, str):
            return '"' + obj.encode("unicode_escape").decode("utf-8") + '"'
        if isinstance(obj, bool):
            return "true" if obj else "false"
        if isinstance(obj, int):
            return f"{obj:d}"
        if isinstance(obj, float):
            return f"{obj:f}"
        raise TypeError(f"Unsupported type: {type(obj)}")

    def get_format_attr(self, d: "Decl") -> list[str]:
        formatted_attributes: list[str] = []
        for key, items in d.attrs.items():
            for item in items:
                if item.args:
                    values_fmt = ", ".join(map(self.get_value, item.args))
                    formatted_attributes.append(f"{key}({values_fmt})")
                else:
                    formatted_attributes.append(key)
        return formatted_attributes


class PrettyPrinter(DeclVisitor[None]):
    def __init__(
        self,
        out: BaseWriter,
        show_resolved: bool = False,
        colorize: bool = False,
    ):
        self.out = out
        self.fmt = PrettyFormatter(show_resolved, colorize)

    def write_pkg_attr(self, d: "PackageDecl"):
        for item in self.fmt.get_format_attr(d):
            attr = self.fmt.as_attr(f"@!{item}")
            self.out.writeln(f"{attr}")

    def write_attr(self, d: "Decl"):
        for item in self.fmt.get_format_attr(d):
            attr = self.fmt.as_attr(f"@{item}")
            self.out.writeln(f"{attr}")

    @override
    def visit_package_import_decl(self, d: "PackageImportDecl"):
        self.write_attr(d)

        use_kw = self.fmt.as_keyword("use")
        as_kw = self.fmt.as_keyword("as")

        alias_pair = (
            f"{self.fmt.get_package_ref_decl(d.pkg_ref)} {as_kw} {d.name}"
            if d.is_alias()
            else self.fmt.get_package_ref_decl(d.pkg_ref)
        )

        self.out.writeln(f"{use_kw} {alias_pair};")

    @override
    def visit_decl_import_decl(self, d: "DeclarationImportDecl"):
        self.write_attr(d)

        from_kw = self.fmt.as_keyword("from")
        use_kw = self.fmt.as_keyword("use")
        as_kw = self.fmt.as_keyword("as")

        alias_pair = (
            f"{self.fmt.get_declaration_ref_decl(d.decl_ref)} {as_kw} {d.name}"
            if d.is_alias()
            else self.fmt.get_declaration_ref_decl(d.decl_ref)
        )

        self.out.writeln(
            f"{from_kw} {self.fmt.get_package_ref_decl(d.decl_ref.pkg_ref)} {use_kw} {alias_pair};"
        )

    @override
    def visit_glob_func_decl(self, d: "GlobFuncDecl"):
        self.write_attr(d)

        func_kw = self.fmt.as_keyword("function")

        fmt_args = ", ".join(map(self.fmt.get_param_decl, d.params))
        ret = self.fmt.get_type_ref_decl(d.return_ty_ref) if d.return_ty_ref else "void"

        self.out.writeln(f"{func_kw} {d.name}({fmt_args}): {ret};")

    @override
    def visit_enum_item_decl(self, d: "EnumItemDecl") -> None:
        self.write_attr(d)

        if d.value is None:
            self.out.writeln(f"{d.name},")
        else:
            self.out.writeln(f"{d.name} = {self.fmt.get_value(d.value)},")

    @override
    def visit_enum_decl(self, d: "EnumDecl") -> None:
        self.write_attr(d)

        enum_kw = self.fmt.as_keyword("enum")

        full_decl = f"{d.name}: {self.fmt.get_type_ref_decl(d.ty_ref)}"
        prologue = f"{enum_kw} {full_decl} {{"
        epilogue = f"}}"

        if d.items:
            with self.out.indented(prologue, epilogue):
                for i in d.items:
                    self.handle_decl(i)
        else:
            self.out.writeln(prologue + epilogue)

    @override
    def visit_union_field_decl(self, d: "UnionFieldDecl"):
        self.write_attr(d)

        if d.ty_ref:
            self.out.writeln(f"{d.name}: {self.fmt.get_type_ref_decl(d.ty_ref)};")
        else:
            self.out.writeln(f"{d.name};")

    @override
    def visit_union_decl(self, d: "UnionDecl"):
        self.write_attr(d)

        union_kw = self.fmt.as_keyword("union")
        prologue = f"{union_kw} {d.name} {{"
        epilogue = f"}}"

        if d.fields:
            with self.out.indented(prologue, epilogue):
                for f in d.fields:
                    self.handle_decl(f)
        else:
            self.out.writeln(prologue + epilogue)

    @override
    def visit_struct_field_decl(self, d: "StructFieldDecl"):
        self.write_attr(d)

        self.out.writeln(f"{d.name}: {self.fmt.get_type_ref_decl(d.ty_ref)};")

    @override
    def visit_struct_decl(self, d: "StructDecl"):
        self.write_attr(d)

        struct_kw = self.fmt.as_keyword("struct")
        prologue = f"{struct_kw} {d.name} {{"
        epilogue = f"}}"

        if d.fields:
            with self.out.indented(prologue, epilogue):
                for f in d.fields:
                    self.handle_decl(f)
        else:
            self.out.writeln(prologue + epilogue)

    @override
    def visit_iface_func_decl(self, d: "IfaceMethodDecl"):
        self.write_attr(d)

        fmt_args = ", ".join(map(self.fmt.get_param_decl, d.params))
        ret = self.fmt.get_type_ref_decl(d.return_ty_ref) if d.return_ty_ref else "void"

        self.out.writeln(f"{d.name}({fmt_args}): {ret};")

    @override
    def visit_iface_decl(self, d: "IfaceDecl"):
        self.write_attr(d)

        iface_kw = self.fmt.as_keyword("interface")

        full_decl = (
            f"{d.name}: " + ", ".join(map(self.fmt.get_parent_decl, d.parents))
            if d.parents
            else d.name
        )
        prologue = f"{iface_kw} {full_decl} {{"
        epilogue = f"}}"

        if d.methods:
            with self.out.indented(prologue, epilogue):
                for f in d.methods:
                    self.handle_decl(f)
        else:
            self.out.writeln(prologue + epilogue)

    @override
    def visit_package_decl(self, p: "PackageDecl"):
        self.out.writeln(f"// {p.name}")
        self.write_pkg_attr(p)
        for d in p.pkg_imports:
            self.handle_decl(d)
        for d in p.decl_imports:
            self.handle_decl(d)
        for d in p.declarations:
            self.handle_decl(d)

    @override
    def visit_package_group(self, g: "PackageGroup"):
        for i, p in enumerate(g.packages):
            if i != 0:
                self.out.newline()
            self.handle_decl(p)