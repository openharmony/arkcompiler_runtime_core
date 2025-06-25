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

from collections.abc import Iterable
from typing import Any

from typing_extensions import override

from taihe.parse.antlr.TaiheAST import TaiheAST as ast
from taihe.parse.antlr.TaiheVisitor import TaiheVisitor as Visitor
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
from taihe.utils.diagnostics import DiagnosticsManager
from taihe.utils.exceptions import InvalidPackageNameError
from taihe.utils.sources import SourceBase, SourceLocation


def pkg2str(pkg_name: ast.PkgName) -> str:
    return ".".join(t.text for t in pkg_name.parts)


def is_valid_pkg_name(name: str) -> bool:
    """Checks if the package name is valid."""
    for part in name.split("."):
        if not part:
            return False
        if not all(c.isalpha() or c == "_" for c in part[:1]):
            return False
        if not all(c.isalnum() or c == "_" for c in part[1:]):
            return False
    return True


class ExprEvaluator(Visitor):
    # Bool Expr

    @override
    def visit_literal_bool_expr(self, node: ast.LiteralBoolExpr) -> bool:
        return {
            "true": True,
            "false": False,
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
        pass
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
        return node.val.text[1:-1].encode("utf-8").decode("unicode_escape")

    @override
    def visit_literal_doc_string_expr(self, node: ast.LiteralDocStringExpr) -> str:
        return node.val.text[3:-3]

    @override
    def visit_binary_string_expr(self, node: ast.BinaryStringExpr) -> str:
        return self.visit(node.left) + self.visit(node.right)

    @override
    def visit_any_expr(self, node: ast.AnyExpr) -> Any:
        return self.visit(node.expr)


class AstConverter(ExprEvaluator):
    """Converts a node on AST to the intermetiade representation.

    Note that declerations with errors are discarded.
    """

    source: SourceBase
    diag: DiagnosticsManager

    def __init__(self, source: SourceBase, diag: DiagnosticsManager):
        self.source = source
        self.diag = diag

    # Type References

    @override
    def visit_long_type(self, node: ast.LongType) -> LongTypeRefDecl:
        d = LongTypeRefDecl(node.loc, pkg2str(node.pkg_name), str(node.decl_name))
        self.diag.for_each(node.forward_attrs, lambda a: d.add_attr(self.visit(a)))
        return d

    @override
    def visit_short_type(self, node: ast.ShortType) -> ShortTypeRefDecl:
        d = ShortTypeRefDecl(node.loc, str(node.decl_name))
        self.diag.for_each(node.forward_attrs, lambda a: d.add_attr(self.visit(a)))
        return d

    @override
    def visit_generic_type(self, node: ast.GenericType) -> GenericTypeRefDecl:
        d = GenericTypeRefDecl(node.loc, str(node.decl_name))
        self.diag.for_each(node.args, lambda a: d.add_arg_ty_ref(self.visit(a)))
        self.diag.for_each(node.forward_attrs, lambda a: d.add_attr(self.visit(a)))
        return d

    @override
    def visit_callback_type(self, node: ast.CallbackType) -> CallbackTypeRefDecl:
        if ty := node.return_ty:
            d = CallbackTypeRefDecl(node.loc, self.visit(ty))
        else:
            d = CallbackTypeRefDecl(node.loc)
        self.diag.for_each(node.parameters, lambda p: d.add_param(self.visit(p)))
        self.diag.for_each(node.forward_attrs, lambda a: d.add_attr(self.visit(a)))
        return d

    # Uses

    @override
    def visit_use_package(self, node: ast.UsePackage) -> Iterable[PackageImportDecl]:
        p_ref = PackageRefDecl(node.pkg_name.loc, pkg2str(node.pkg_name))
        if node.pkg_alias:
            d = PackageImportDecl(
                p_ref,
                name=str(node.pkg_alias),
                loc=node.pkg_alias.loc,
            )
        else:
            d = PackageImportDecl(
                p_ref,
            )
        yield d

    @override
    def visit_use_symbol(self, node: ast.UseSymbol) -> Iterable[DeclarationImportDecl]:
        p_ref = PackageRefDecl(node.pkg_name.loc, pkg2str(node.pkg_name))
        for p in node.decl_alias_pairs:
            d_ref = DeclarationRefDecl(p.decl_name.loc, str(p.decl_name), p_ref)
            if p.decl_alias:
                d = DeclarationImportDecl(
                    d_ref,
                    name=str(p.decl_alias),
                    loc=p.decl_alias.loc,
                )
            else:
                d = DeclarationImportDecl(
                    d_ref,
                )
            yield d

    # Declarations

    @override
    def visit_struct_property(self, node: ast.StructProperty) -> StructFieldDecl:
        d = StructFieldDecl(node.name.loc, str(node.name), self.visit(node.ty))
        self.diag.for_each(node.forward_attrs, lambda a: d.add_attr(self.visit(a)))
        return d

    @override
    def visit_struct(self, node: ast.Struct) -> StructDecl:
        d = StructDecl(node.name.loc, str(node.name))
        self.diag.for_each(node.fields, lambda f: d.add_field(self.visit(f)))
        self.diag.for_each(node.forward_attrs, lambda a: d.add_attr(self.visit(a)))
        self.diag.for_each(node.inner_attrs, lambda a: d.add_attr(self.visit(a)))
        return d

    @override
    def visit_enum_property(self, node: ast.EnumProperty) -> EnumItemDecl:
        if node.val:
            d = EnumItemDecl(node.name.loc, str(node.name), self.visit(node.val))
        else:
            d = EnumItemDecl(node.name.loc, str(node.name))
        self.diag.for_each(node.forward_attrs, lambda a: d.add_attr(self.visit(a)))
        return d

    @override
    def visit_enum(self, node: ast.Enum) -> EnumDecl:
        d = EnumDecl(node.name.loc, str(node.name), self.visit(node.enum_ty))
        self.diag.for_each(node.fields, lambda a: d.add_item(self.visit(a)))
        self.diag.for_each(node.forward_attrs, lambda a: d.add_attr(self.visit(a)))
        return d

    @override
    def visit_union_property(self, node: ast.UnionProperty) -> UnionFieldDecl:
        if ty := node.ty:
            d = UnionFieldDecl(node.name.loc, str(node.name), self.visit(ty))
        else:
            d = UnionFieldDecl(node.name.loc, str(node.name))
        self.diag.for_each(node.forward_attrs, lambda a: d.add_attr(self.visit(a)))
        return d

    @override
    def visit_union(self, node: ast.Union) -> UnionDecl:
        d = UnionDecl(node.name.loc, str(node.name))
        self.diag.for_each(node.fields, lambda f: d.add_field(self.visit(f)))
        self.diag.for_each(node.forward_attrs, lambda a: d.add_attr(self.visit(a)))
        self.diag.for_each(node.inner_attrs, lambda a: d.add_attr(self.visit(a)))
        return d

    @override
    def visit_parameter(self, node: ast.Parameter) -> ParamDecl:
        d = ParamDecl(node.name.loc, str(node.name), self.visit(node.ty))
        self.diag.for_each(node.forward_attrs, lambda a: d.add_attr(self.visit(a)))
        return d

    @override
    def visit_interface_function(self, node: ast.InterfaceFunction) -> IfaceMethodDecl:
        if ty := node.return_ty:
            d = IfaceMethodDecl(node.name.loc, str(node.name), self.visit(ty))
        else:
            d = IfaceMethodDecl(node.name.loc, str(node.name))
        self.diag.for_each(node.parameters, lambda p: d.add_param(self.visit(p)))
        self.diag.for_each(node.forward_attrs, lambda a: d.add_attr(self.visit(a)))
        return d

    @override
    def visit_interface_parent(self, node: ast.InterfaceParent) -> IfaceParentDecl:
        p = IfaceParentDecl(node.ty.loc, self.visit(node.ty))
        return p

    @override
    def visit_interface(self, node: ast.Interface) -> IfaceDecl:
        d = IfaceDecl(node.name.loc, str(node.name))
        self.diag.for_each(node.fields, lambda f: d.add_method(self.visit(f)))
        self.diag.for_each(node.extends, lambda i: d.add_parent(self.visit(i)))
        self.diag.for_each(node.forward_attrs, lambda a: d.add_attr(self.visit(a)))
        self.diag.for_each(node.inner_attrs, lambda a: d.add_attr(self.visit(a)))
        return d

    @override
    def visit_global_function(self, node: ast.GlobalFunction) -> GlobFuncDecl:
        if ty := node.return_ty:
            d = GlobFuncDecl(node.name.loc, str(node.name), self.visit(ty))
        else:
            d = GlobFuncDecl(node.name.loc, str(node.name))
        self.diag.for_each(node.parameters, lambda p: d.add_param(self.visit(p)))
        self.diag.for_each(node.forward_attrs, lambda a: d.add_attr(self.visit(a)))
        return d

    # Attributes

    def visit_attr(self, node: ast.DeclAttr | ast.ScopeAttr) -> AttrItemDecl:
        args: list[Any] = []
        kwargs: dict[str, Any] = {}
        for arg in node.args:
            if isinstance(arg, ast.NamedAttrArg):
                kwargs[str(arg.name)] = self.visit(arg.val)
            else:
                args.append(self.visit(arg.val))
        d = AttrItemDecl(node.name.loc, str(node.name), args, kwargs)
        return d

    @override
    def visit_decl_attr(self, node: ast.DeclAttr) -> AttrItemDecl:
        return self.visit_attr(node)

    @override
    def visit_scope_attr(self, node: ast.ScopeAttr) -> AttrItemDecl:
        return self.visit_attr(node)

    # Package

    @override
    def visit_spec(self, node: ast.Spec) -> PackageDecl:
        if not is_valid_pkg_name(self.source.pkg_name):
            raise InvalidPackageNameError(
                self.source.pkg_name,
                loc=SourceLocation(self.source),
            )
        pkg = PackageDecl(self.source.pkg_name, SourceLocation(self.source))
        for u in node.uses:
            self.diag.for_each(self.visit(u), pkg.add_import)
        self.diag.for_each(node.fields, lambda n: pkg.add_declaration(self.visit(n)))
        self.diag.for_each(node.inner_attrs, lambda a: pkg.add_attr(self.visit(a)))
        return pkg

    def convert(self) -> PackageDecl:
        """Converts the whole source code buffer to a package.

        Returns:
            PackageDecl: The package declaration containing all declarations
            and imports from the source code.

        Raises:
            InvalidPackageNameError: If the package name is invalid.
        """
        ast_nodes = generate_ast(self.source, self.diag)
        return self.visit_spec(ast_nodes)