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

"""Convert AST to IR."""

from codecs import decode
from collections.abc import Iterable
from dataclasses import dataclass
from enum import Enum
from typing import Any, Optional

from typing_extensions import override

from taihe.parse import Visitor, ast
from taihe.parse.ast_generation import generate_ast
from taihe.semantics.declarations import (
    AttrItemDecl,
    CallbackTypeRefDecl,
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
    PackageImportDecl,
    PackageRefDecl,
    ParamDecl,
    ShortTypeRefDecl,
    StructDecl,
    StructFieldDecl,
    UnionDecl,
    UnionFieldDecl,
)
from taihe.utils.diagnostics import (
    AbstractDiagnosticsManager,
    DiagNote,
    DiagWarn,
)
from taihe.utils.sources import SourceBase, SourceLocation


class IgnoredFileReason(Enum):
    IS_DIRECTORY = "subdirectories are ignored"
    EXTENSION_MISMATCH = "unexpected file extension"
    INVALID_PKG_NAME = "invalid package name"


@dataclass
class IgnoredFileWarn(DiagWarn):
    reason: IgnoredFileReason
    note: Optional[DiagNote] = None

    @property
    @override
    def format_msg(self) -> str:
        return f"unrecognized file: {self.reason.value}"

    def notes(self):
        if self.note:
            yield self.note


def normalize_pkg_name(name: str):
    def is_allowed(char: str):
        return char.isalnum() or char == "_"

    def to_valid_identifier(s: str):
        """Converts a string to valid, C-style identifier."""
        # First, remove all non-alphanumeric characters, excluding "_".
        s = "".join(char for char in s if is_allowed(char))
        # Next, ensure that the segment doesn't begin with a digit.
        if s and s[0].isnumeric():
            # If so, we inject "_" in the beginning.
            s = "_" + s
        return s

    # First, split the package name into segments.
    segments = name.split(".")
    # Next, make sure that each segment is valid.
    translated_segments = (to_valid_identifier(s) for s in segments)
    # Finally, reconstruct the package name.
    return ".".join(s for s in translated_segments if s)


def pkg2str(pkg_name: ast.PkgName) -> str:
    if pkg_name:
        return ".".join(t.text for t in pkg_name.parts)
    else:
        return ""


class ExprEvaluator(Visitor):
    # Bool Expr

    @override
    def visit_literal_bool_expr(self, node: ast.LiteralBoolExpr) -> bool:
        return {
            "TRUE": True,
            "FALSE": False,
        }[node.val.text]

    @override
    def visit_int_comparison_bool_expr(self, node: ast.IntComparisonBoolExpr) -> bool:
        return {
            ">": int.__gt__,
            "<": int.__lt__,
            ">=": int.__ge__,
            "<=": int.__le__,
            "==": int.__eq__,
            "!=": int.__ne__,
        }[node.op.text](
            int(self.visit(node.left)),
            int(self.visit(node.right)),
        )

    @override
    def visit_float_comparison_bool_expr(
        self, node: ast.FloatComparisonBoolExpr
    ) -> bool:
        return {
            ">": float.__gt__,
            "<": float.__lt__,
            ">=": float.__ge__,
            "<=": float.__le__,
            "==": float.__eq__,
            "!=": float.__ne__,
        }[node.op.text](
            float(self.visit(node.left)),
            float(self.visit(node.right)),
        )

    @override
    def visit_unary_bool_expr(self, node: ast.UnaryBoolExpr) -> bool:
        return not self.visit(node.expr)

    @override
    def visit_binary_bool_expr(self, node: ast.BinaryBoolExpr) -> bool:
        return {
            "&&": bool.__and__,
            "||": bool.__or__,
        }[node.op.text](
            bool(self.visit(node.left)),
            bool(self.visit(node.right)),
        )

    @override
    def visit_parenthesis_bool_expr(self, node: ast.ParenthesisBoolExpr) -> bool:
        return self.visit(node.expr)

    @override
    def visit_conditional_bool_expr(self, node: ast.ConditionalBoolExpr) -> bool:
        return (
            self.visit(node.then_expr)
            if self.visit(node.cond)
            else self.visit(node.else_expr)
        )

    # Int Expr

    @override
    def visit_literal_int_expr(self, node: ast.LiteralIntExpr) -> int:
        text = node.val.text
        if text.startswith("0b"):
            return int(text, 2)
        if text.startswith("0o"):
            return int(text, 8)
        if text.startswith("0x"):
            return int(text, 16)
        return int(text)

    @override
    def visit_parenthesis_int_expr(self, node: ast.ParenthesisIntExpr) -> int:
        return self.visit(node.expr)

    @override
    def visit_conditional_int_expr(self, node: ast.ConditionalIntExpr) -> int:
        return (
            self.visit(node.then_expr)
            if self.visit(node.cond)
            else self.visit(node.else_expr)
        )

    @override
    def visit_unary_int_expr(self, node: ast.UnaryIntExpr) -> int:
        return {
            "-": int.__neg__,
            "+": int.__pos__,
            "~": int.__invert__,
        }[node.op.text](
            int(self.visit(node.expr)),
        )

    @override
    def visit_binary_int_expr(self, node: ast.BinaryIntExpr) -> int:
        return {
            "+": int.__add__,
            "-": int.__sub__,
            "*": int.__mul__,
            "/": int.__floordiv__,
            "%": int.__mod__,
            "<<": int.__lshift__,
            ">>": int.__rshift__,
            "&": int.__and__,
            "|": int.__or__,
            "^": int.__xor__,
        }[node.op.text](
            int(self.visit(node.left)),
            int(self.visit(node.right)),
        )

    @override
    def visit_binary_int_shift_expr(self, node: ast.BinaryIntShiftExpr) -> int:
        return {
            "<": int.__lshift__,
            ">": int.__rshift__,
        }[node.ch.text](
            int(self.visit(node.left)),
            int(self.visit(node.right)),
        )

    # Float Expr

    @override
    def visit_literal_float_expr(self, node: ast.LiteralFloatExpr) -> float:
        return float(node.val.text)

    @override
    def visit_parenthesis_float_expr(self, node: ast.ParenthesisFloatExpr) -> float:
        return self.visit(node.expr)

    @override
    def visit_conditional_float_expr(self, node: ast.ConditionalFloatExpr) -> Any:
        return (
            self.visit(node.then_expr)
            if self.visit(node.cond)
            else self.visit(node.else_expr)
        )

    @override
    def visit_unary_float_expr(self, node: ast.UnaryFloatExpr) -> float:
        return {
            "-": float.__neg__,
            "+": float.__pos__,
        }[node.op.text](
            float(self.visit(node.expr)),
        )

    @override
    def visit_binary_float_expr(self, node: ast.BinaryFloatExpr) -> float:
        return {
            "+": float.__add__,
            "-": float.__sub__,
            "*": float.__mul__,
            "/": float.__truediv__,
        }[node.op.text](
            float(self.visit(node.left)),
            float(self.visit(node.right)),
        )

    # String Expr

    @override
    def visit_literal_string_expr(self, node: ast.LiteralStringExpr) -> str:
        return "".join(
            decode(
                (
                    val.text[3:-3]
                    if len(val.text) > 2
                    and val.text.startswith('"""')
                    and val.text.endswith('"""')
                    else val.text[1:-1]
                ),
                "unicode-escape",
            )
            for val in node.vals
        )


class AstConverter(ExprEvaluator):
    """Converts a node on AST to the intermetiade representation.

    Note that declerations with errors are discarded.
    """

    source: SourceBase
    diag: AbstractDiagnosticsManager

    def __init__(self, source: SourceBase, diag: AbstractDiagnosticsManager):
        self.source = source
        self.diag = diag

    def loc(self, t: ast.any):
        # Remember, token.column is 0-based.
        return SourceLocation(self.source, *t._beg, *t._end)

    # Attributes

    @override
    def visit_attr_val(self, node: ast.AttrVal) -> Any:
        return self.visit(node.expr)

    # Type References

    @override
    def visit_long_type(self, node: ast.LongType) -> LongTypeRefDecl:
        loc = self.loc(node)
        pkname = pkg2str(node.pkg_name)
        symbol = str(node.decl_name)
        return LongTypeRefDecl(loc, pkname, symbol)

    @override
    def visit_short_type(self, node: ast.ShortType) -> ShortTypeRefDecl:
        loc = self.loc(node)
        symbol = str(node.decl_name)
        return ShortTypeRefDecl(loc, symbol)

    @override
    def visit_generic_type(self, node: ast.GenericType) -> GenericTypeRefDecl:
        loc = self.loc(node)
        symbol = str(node.decl_name)
        args = [self.visit(arg) for arg in node.args]
        return GenericTypeRefDecl(loc, symbol, args)

    @override
    def visit_callback_type(self, node: ast.CallbackType) -> CallbackTypeRefDecl:
        if ty := node.return_ty:
            d = CallbackTypeRefDecl(self.loc(node), self.visit(ty))
        else:
            d = CallbackTypeRefDecl(self.loc(node))
        self.diag.for_each(node.parameters, lambda p: d.add_param(self.visit(p)))
        return d

    # Uses

    @override
    def visit_use_package(self, node: ast.UsePackage) -> Iterable[PackageImportDecl]:
        p_ref = PackageRefDecl(self.loc(node.pkg_name), pkg2str(node.pkg_name))
        if node.pkg_alias:
            d = PackageImportDecl(
                p_ref,
                name=str(node.pkg_alias),
                loc=self.loc(node.pkg_alias),
            )
        else:
            d = PackageImportDecl(
                p_ref,
            )
        yield d

    @override
    def visit_use_symbol(self, node: ast.UseSymbol) -> Iterable[DeclarationImportDecl]:
        p_ref = PackageRefDecl(self.loc(node.pkg_name), pkg2str(node.pkg_name))
        for p in node.decl_alias_pairs:
            d_ref = DeclarationRefDecl(self.loc(p.decl_name), str(p.decl_name), p_ref)
            if p.decl_alias:
                d = DeclarationImportDecl(
                    d_ref,
                    name=str(p.decl_alias),
                    loc=self.loc(p.decl_alias),
                )
            else:
                d = DeclarationImportDecl(
                    d_ref,
                )
            yield d

    # Declarations

    @override
    def visit_struct_property(self, node: ast.StructProperty) -> StructFieldDecl:
        d = StructFieldDecl(self.loc(node.name), str(node.name), self.visit(node.ty))
        self.diag.for_each(node.forward_attrs, lambda a: d.add_attr(self.visit(a)))
        return d

    @override
    def visit_struct(self, node: ast.Struct) -> StructDecl:
        d = StructDecl(self.loc(node.name), str(node.name))
        self.diag.for_each(node.fields, lambda f: d.add_field(self.visit(f)))
        self.diag.for_each(node.forward_attrs, lambda a: d.add_attr(self.visit(a)))
        self.diag.for_each(node.inner_attrs, lambda a: d.add_attr(self.visit(a)))
        return d

    @override
    def visit_enum_property(self, node: ast.EnumProperty) -> EnumItemDecl:
        if node.val:
            d = EnumItemDecl(self.loc(node.name), str(node.name), self.visit(node.val))
        else:
            d = EnumItemDecl(self.loc(node.name), str(node.name))
        self.diag.for_each(node.forward_attrs, lambda a: d.add_attr(self.visit(a)))
        return d

    @override
    def visit_enum(self, node: ast.Enum) -> EnumDecl:
        if ty := node.enum_ty:
            d = EnumDecl(self.loc(node.name), str(node.name), self.visit(ty))
        else:
            d = EnumDecl(self.loc(node.name), str(node.name))
        self.diag.for_each(node.fields, lambda a: d.add_item(self.visit(a)))
        self.diag.for_each(node.forward_attrs, lambda a: d.add_attr(self.visit(a)))
        return d

    @override
    def visit_union_property(self, node: ast.UnionProperty) -> UnionFieldDecl:
        if ty := node.ty:
            d = UnionFieldDecl(self.loc(node.name), str(node.name), self.visit(ty))
        else:
            d = UnionFieldDecl(self.loc(node.name), str(node.name))
        self.diag.for_each(node.forward_attrs, lambda a: d.add_attr(self.visit(a)))
        return d

    @override
    def visit_union(self, node: ast.Union) -> UnionDecl:
        d = UnionDecl(self.loc(node.name), str(node.name))
        self.diag.for_each(node.fields, lambda f: d.add_field(self.visit(f)))
        self.diag.for_each(node.forward_attrs, lambda a: d.add_attr(self.visit(a)))
        self.diag.for_each(node.inner_attrs, lambda a: d.add_attr(self.visit(a)))
        return d

    @override
    def visit_parameter(self, node: ast.Parameter) -> ParamDecl:
        d = ParamDecl(self.loc(node.name), str(node.name), self.visit(node.ty))
        self.diag.for_each(node.forward_attrs, lambda a: d.add_attr(self.visit(a)))
        return d

    @override
    def visit_interface_function(self, node: ast.InterfaceFunction) -> IfaceMethodDecl:
        if ty := node.return_ty:
            d = IfaceMethodDecl(self.loc(node.name), str(node.name), self.visit(ty))
        else:
            d = IfaceMethodDecl(self.loc(node.name), str(node.name))
        self.diag.for_each(node.parameters, lambda p: d.add_param(self.visit(p)))
        self.diag.for_each(node.forward_attrs, lambda a: d.add_attr(self.visit(a)))
        return d

    @override
    def visit_interface_parent(self, node: ast.InterfaceParent) -> IfaceParentDecl:
        p = IfaceParentDecl(self.loc(node.ty), self.visit(node.ty))
        return p

    @override
    def visit_interface(self, node: ast.Interface) -> IfaceDecl:
        d = IfaceDecl(self.loc(node.name), str(node.name))
        self.diag.for_each(node.fields, lambda f: d.add_method(self.visit(f)))
        self.diag.for_each(node.extends, lambda i: d.add_parent(self.visit(i)))
        self.diag.for_each(node.forward_attrs, lambda a: d.add_attr(self.visit(a)))
        self.diag.for_each(node.inner_attrs, lambda a: d.add_attr(self.visit(a)))
        return d

    @override
    def visit_global_function(self, node: ast.GlobalFunction) -> GlobFuncDecl:
        if ty := node.return_ty:
            d = GlobFuncDecl(self.loc(node.name), str(node.name), self.visit(ty))
        else:
            d = GlobFuncDecl(self.loc(node.name), str(node.name))
        self.diag.for_each(node.parameters, lambda p: d.add_param(self.visit(p)))
        self.diag.for_each(node.forward_attrs, lambda a: d.add_attr(self.visit(a)))
        return d

    @override
    def visit_decl_attr(self, node: ast.DeclAttr) -> AttrItemDecl:
        value = tuple(self.visit(val) for val in node.vals)
        d = AttrItemDecl(self.loc(node.name), str(node.name), value)
        return d

    @override
    def visit_scope_attr(self, node: ast.ScopeAttr) -> AttrItemDecl:
        value = tuple(self.visit(val) for val in node.vals)
        d = AttrItemDecl(self.loc(node.name), str(node.name), value)
        return d

    # Package

    @override
    def visit_spec(self, node: ast.Spec) -> PackageDecl:
        pkg = PackageDecl(self.source.pkg_name, SourceLocation(self.source))
        for u in node.uses:
            self.diag.for_each(self.visit(u), pkg.add_import)
        self.diag.for_each(node.fields, lambda n: pkg.add_declaration(self.visit(n)))
        self.diag.for_each(node.inner_attrs, lambda a: pkg.add_attr(self.visit(a)))
        return pkg

    def convert(self) -> PackageDecl:
        """Converts the whole source code buffer to a package."""
        ast_nodes = generate_ast(self.source, self.diag)
        return self.visit_spec(ast_nodes)
