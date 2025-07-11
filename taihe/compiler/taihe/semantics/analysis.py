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

from collections.abc import Callable, Iterable
from typing import Any, TypeGuard, TypeVar

from typing_extensions import override

from taihe.semantics.declarations import (
    CallbackTypeRefDecl,
    DeclarationRefDecl,
    EnumDecl,
    EnumItemDecl,
    GenericTypeRefDecl,
    GlobFuncDecl,
    IfaceDecl,
    IfaceMethodDecl,
    IfaceParentDecl,
    LongTypeRefDecl,
    NamedDecl,
    PackageDecl,
    PackageGroup,
    PackageRefDecl,
    ShortTypeRefDecl,
    StructDecl,
    TypeDecl,
    TypeRefDecl,
    UnionDecl,
)
from taihe.semantics.types import (
    BUILTIN_GENERICS,
    BUILTIN_TYPES,
    CallbackType,
    ScalarKind,
    ScalarType,
    StringType,
    Type,
    UserType,
)
from taihe.semantics.visitor import RecursiveDeclVisitor
from taihe.utils.diagnostics import DiagnosticsManager
from taihe.utils.exceptions import (
    DeclarationNotInScopeError,
    DeclNotExistError,
    DeclRedefError,
    DuplicateExtendsWarn,
    EnumValueError,
    GenericArgumentsError,
    NotATypeError,
    PackageNotExistError,
    PackageNotInScopeError,
    RecursiveReferenceError,
    SymbolConflictWithNamespaceError,
    TypeUsageError,
)


def analyze_semantics(pg: PackageGroup, diag: DiagnosticsManager):
    """Runs semantic analysis passes on the given package group."""
    _check_decl_confilct_with_namespace(pg, diag)
    _ResolveImportsPass(diag).handle_decl(pg)
    _CheckFieldNameCollisionErrorPass(diag).handle_decl(pg)
    _CheckEnumTypePass(diag).handle_decl(pg)
    _CheckRecursiveInclusionPass(diag).handle_decl(pg)


def _check_decl_confilct_with_namespace(
    pg: PackageGroup,
    diag: DiagnosticsManager,
):
    """Checks for declarations conflicts with namespaces."""
    namespaces: dict[str, list[PackageDecl]] = {}
    for pkg in pg.packages:
        pkg_name = pkg.name
        # package "a.b.c" -> namespaces ["a.b.c", "a.b", "a"]
        while True:
            namespaces.setdefault(pkg_name, []).append(pkg)
            splited = pkg_name.rsplit(".", maxsplit=1)
            if len(splited) == 2:
                pkg_name = splited[0]
            else:
                break

    for p in pg.packages:
        for d in p.declarations:
            name = p.name + "." + d.name
            if packages := namespaces.get(name, []):
                diag.emit(SymbolConflictWithNamespaceError(d, name, packages))


class _ResolveImportsPass(RecursiveDeclVisitor):
    """Resolves imports and type references within a package group."""

    diag: DiagnosticsManager

    def __init__(self, diag: DiagnosticsManager):
        self._current_pkg_group = None
        self._current_pkg = None  # Always points to the current package.
        self.diag = diag

    @property
    def pkg(self) -> PackageDecl:
        pass
        return self._current_pkg

    @property
    def pkg_group(self) -> PackageGroup:
        pass
        return self._current_pkg_group

    @override
    def visit_package_decl(self, p: PackageDecl) -> None:
        self._current_pkg = p
        super().visit_package_decl(p)
        self._current_pkg = None

    @override
    def visit_package_group(self, g: PackageGroup) -> None:
        self._current_pkg_group = g
        super().visit_package_group(g)
        self._current_pkg_group = None

    @override
    def visit_package_ref_decl(self, d: PackageRefDecl) -> None:
        if d.is_resolved:
            return
        d.is_resolved = True

        pkg = self.pkg_group.lookup(d.symbol)

        if pkg is None:
            self.diag.emit(PackageNotExistError(d.symbol, loc=d.loc))
            return

        d.maybe_resolved_pkg = pkg

    @override
    def visit_declaration_ref_decl(self, d: DeclarationRefDecl) -> None:
        if d.is_resolved:
            return
        d.is_resolved = True

        self.handle_decl(d.pkg_ref)

        pkg = d.pkg_ref.maybe_resolved_pkg

        if pkg is None:
            # No need to repeatedly throw exceptions
            return

        decl = pkg.lookup(d.symbol)

        if decl is None:
            self.diag.emit(DeclNotExistError(d.symbol, loc=d.loc))
            return

        d.maybe_resolved_decl = decl

    @override
    def visit_long_type_ref_decl(self, d: LongTypeRefDecl) -> None:
        if d.is_resolved:
            return
        d.is_resolved = True

        # Find the corresponding imported package according to the package name
        pkg_import = self.pkg.lookup_pkg_import(d.pkname)

        if pkg_import is None:
            self.diag.emit(PackageNotInScopeError(d.pkname, loc=d.loc))
            return

        # Then find the corresponding type declaration from the package
        pkg = pkg_import.pkg_ref.maybe_resolved_pkg

        if pkg is None:
            # No need to repeatedly throw exceptions
            return

        decl = pkg.lookup(d.symbol)

        if decl is None:
            self.diag.emit(DeclNotExistError(d.symbol, loc=d.loc))
            return

        if not isinstance(decl, TypeDecl):
            self.diag.emit(NotATypeError(d.symbol, loc=d.loc))
            return

        d.maybe_resolved_ty = decl.as_type(d)

    @override
    def visit_short_type_ref_decl(self, d: ShortTypeRefDecl) -> None:
        if d.is_resolved:
            return
        d.is_resolved = True

        # Find Builtin Types
        builder = BUILTIN_TYPES.get(d.symbol)

        if builder:
            d.maybe_resolved_ty = builder(d)
            return

        # Find types declared in the current package
        decl = self.pkg.lookup(d.symbol)

        if decl:
            if not isinstance(decl, TypeDecl):
                self.diag.emit(NotATypeError(d.symbol, loc=d.loc))
                return

            d.maybe_resolved_ty = decl.as_type(d)
            return

        # Look for imported type declarations
        decl_import = self.pkg.lookup_decl_import(d.symbol)

        if decl_import is None:
            self.diag.emit(DeclarationNotInScopeError(d.symbol, loc=d.loc))
            return

        decl = decl_import.decl_ref.maybe_resolved_decl

        if decl is None:
            # No need to repeatedly throw exceptions
            return

        if not isinstance(decl, TypeDecl):
            self.diag.emit(NotATypeError(d.symbol, loc=d.loc))
            return

        d.maybe_resolved_ty = decl.as_type(d)

    @override
    def visit_generic_type_ref_decl(self, d: GenericTypeRefDecl) -> None:
        if d.is_resolved:
            return
        d.is_resolved = True

        super().visit_generic_type_ref_decl(d)

        args_ty: list[Type] = []
        for arg_ty_ref in d.args_ty_ref:
            arg_ty = arg_ty_ref.maybe_resolved_ty
            if arg_ty is None:
                # No need to repeatedly throw exceptions
                return
            args_ty.append(arg_ty)

        decl_name = d.symbol

        builder = BUILTIN_GENERICS.get(decl_name)

        if builder is None:
            self.diag.emit(DeclarationNotInScopeError(decl_name, loc=d.loc))
            return

        try:
            d.maybe_resolved_ty = builder(d, *args_ty)
        except TypeError:
            self.diag.emit(GenericArgumentsError(d.text, loc=d.loc))

    @override
    def visit_callback_type_ref_decl(self, d: CallbackTypeRefDecl) -> None:
        if d.is_resolved:
            return
        d.is_resolved = True

        super().visit_callback_type_ref_decl(d)

        if d.return_ty_ref:
            return_ty = d.return_ty_ref.maybe_resolved_ty
            if return_ty is None:
                # No need to repeatedly throw exceptions
                return
        else:
            return_ty = None

        params_ty: list[Type] = []
        for param in d.params:
            arg_ty = param.ty_ref.maybe_resolved_ty
            if arg_ty is None:
                # No need to repeatedly throw exceptions
                return
            params_ty.append(arg_ty)

        d.maybe_resolved_ty = CallbackType(d, return_ty, tuple(params_ty))


class _CheckFieldNameCollisionErrorPass(RecursiveDeclVisitor):
    """Check for duplicate field names in declarations and name anonymous declarations."""

    diag: DiagnosticsManager

    def __init__(self, diag: DiagnosticsManager):
        self.diag = diag

    @override
    def visit_glob_func_decl(self, d: GlobFuncDecl) -> None:
        self.check_collision_helper(d.params)
        return super().visit_glob_func_decl(d)

    @override
    def visit_iface_func_decl(self, d: IfaceMethodDecl) -> None:
        self.check_collision_helper(d.params)
        return super().visit_iface_func_decl(d)

    @override
    def visit_enum_decl(self, d: EnumDecl) -> None:
        self.check_collision_helper(d.items)
        return super().visit_enum_decl(d)

    @override
    def visit_struct_decl(self, d: StructDecl) -> None:
        self.check_collision_helper(d.fields)
        return super().visit_struct_decl(d)

    @override
    def visit_union_decl(self, d: UnionDecl) -> None:
        self.check_collision_helper(d.fields)
        return super().visit_union_decl(d)

    @override
    def visit_iface_decl(self, d: IfaceDecl) -> None:
        self.check_collision_helper(d.methods)
        return super().visit_iface_decl(d)

    @override
    def visit_package_decl(self, p: PackageDecl) -> None:
        self.check_collision_helper(p.declarations)
        return super().visit_package_decl(p)

    def check_collision_helper(self, children: Iterable[NamedDecl]):
        names: dict[str, NamedDecl] = {}
        for f in children:
            if (prev := names.setdefault(f.name, f)) != f:
                self.diag.emit(DeclRedefError(prev, f))


class _CheckEnumTypePass(RecursiveDeclVisitor):
    """Validated enum item types."""

    diag: DiagnosticsManager

    def __init__(self, diag: DiagnosticsManager):
        self.diag = diag

    def visit_enum_decl(self, d: EnumDecl) -> None:
        def is_int(val: Any) -> TypeGuard[int]:
            return not isinstance(val, bool) and isinstance(val, int)

        valid: Callable[[Any], bool]
        increment: Callable[[Any, EnumItemDecl], Any]
        default: Callable[[EnumItemDecl], Any]

        match d.ty_ref.maybe_resolved_ty:
            case ScalarType(_, ScalarKind.I8):
                valid = lambda val: is_int(val) and -(2**7) <= val < 2**7
                increment = lambda prev, item: prev + 1
                default = lambda item: 0
            case ScalarType(_, ScalarKind.I16):
                valid = lambda val: is_int(val) and -(2**15) <= val < 2**15
                increment = lambda prev, item: prev + 1
                default = lambda item: 0
            case ScalarType(_, ScalarKind.I32):
                valid = lambda val: is_int(val) and -(2**31) <= val < 2**31
                increment = lambda prev, item: prev + 1
                default = lambda item: 0
            case ScalarType(_, ScalarKind.I64):
                valid = lambda val: is_int(val) and -(2**63) <= val < 2**63
                increment = lambda prev, item: prev + 1
                default = lambda item: 0
            case ScalarType(_, ScalarKind.U8):
                valid = lambda val: is_int(val) and 0 <= val < 2**8
                increment = lambda prev, item: prev + 1
                default = lambda item: 0
            case ScalarType(_, ScalarKind.U16):
                valid = lambda val: is_int(val) and 0 <= val < 2**16
                increment = lambda prev, item: prev + 1
                default = lambda item: 0
            case ScalarType(_, ScalarKind.U32):
                valid = lambda val: is_int(val) and 0 <= val < 2**32
                increment = lambda prev, item: prev + 1
                default = lambda item: 0
            case ScalarType(_, ScalarKind.U64):
                valid = lambda val: is_int(val) and 0 <= val < 2**64
                increment = lambda prev, item: prev + 1
                default = lambda item: 0
            case ScalarType(_, ScalarKind.BOOL):
                valid = lambda val: isinstance(val, bool)
                increment = lambda prev, item: False
                default = lambda item: False
            case ScalarType(_, ScalarKind.F32):
                valid = lambda val: isinstance(val, float)
                increment = lambda prev, item: 0.0
                default = lambda item: 0.0
            case ScalarType(_, ScalarKind.F64):
                valid = lambda val: isinstance(val, float)
                increment = lambda prev, item: 0.0
                default = lambda item: 0.0
            case StringType():
                valid = lambda val: isinstance(val, str)
                increment = lambda prev, item: item.name
                default = lambda item: item.name
            case None:
                return
            case _:
                self.diag.emit(TypeUsageError(d.ty_ref))
                return

        prev = None
        for item in d.items:
            if item.value is None:
                item.value = default(item) if prev is None else increment(prev, item)
            if not valid(item.value):
                self.diag.emit(EnumValueError(item, d))
                prev = None
            else:
                prev = item.value


class _CheckRecursiveInclusionPass(RecursiveDeclVisitor):
    """Validates struct fields for type correctness and cycles."""

    diag: DiagnosticsManager

    def __init__(self, diag: DiagnosticsManager):
        self.diag = diag
        self.type_table: dict[
            TypeDecl,
            list[tuple[tuple[TypeDecl, TypeRefDecl], TypeDecl]],
        ] = {}

    def visit_package_group(self, g: PackageGroup) -> None:
        self.type_table = {}
        super().visit_package_group(g)
        cycles = detect_cycles(self.type_table)
        for cycle in cycles:
            last, *other = cycle[::-1]
            self.diag.emit(RecursiveReferenceError(last, other))

    def visit_enum_decl(self, d: EnumDecl) -> None:
        self.type_table[d] = []

    def visit_iface_decl(self, d: IfaceDecl) -> None:
        parent_iface_list = self.type_table.setdefault(d, [])
        parent_iface_dict: dict[IfaceDecl, IfaceParentDecl] = {}
        for parent in d.parents:
            if (parent_ty := parent.ty_ref.maybe_resolved_ty) is None:
                continue
            if not isinstance(parent_ty, UserType):
                self.diag.emit(TypeUsageError(parent.ty_ref))
                continue
            if not isinstance(parent_iface := parent_ty.ty_decl, IfaceDecl):
                self.diag.emit(TypeUsageError(parent.ty_ref))
                continue
            parent_iface_list.append(((d, parent.ty_ref), parent_iface))
            prev = parent_iface_dict.setdefault(parent_iface, parent)
            if prev != parent:
                self.diag.emit(
                    DuplicateExtendsWarn(
                        d,
                        parent_iface,
                        loc=parent.ty_ref.loc,
                        prev_loc=prev.ty_ref.loc,
                    )
                )

    def visit_struct_decl(self, d: StructDecl) -> None:
        type_list = self.type_table.setdefault(d, [])
        for f in d.fields:
            if isinstance(ty := f.ty_ref.maybe_resolved_ty, UserType):
                type_list.append(((d, f.ty_ref), ty.ty_decl))

    def visit_union_decl(self, d: UnionDecl) -> None:
        type_list = self.type_table.setdefault(d, [])
        for i in d.fields:
            if i.ty_ref is None:
                continue
            if isinstance(ty := i.ty_ref.maybe_resolved_ty, UserType):
                type_list.append(((d, i.ty_ref), ty.ty_decl))


V = TypeVar("V")
E = TypeVar("E")


def detect_cycles(graph: dict[V, list[tuple[E, V]]]) -> list[list[E]]:
    """Detects and returns all cycles in a directed graph.

    Example:
    -------
    >>> graph = {
            "A": [("A.b_0", "B")],
            "B": [("B.c_0", "C")],
            "C": [("C.a_0", "A"), ("C.a_1", "A")],
        }
    >>> detect_cycles(graph)
    [["A.b_0", "B.c_0", "C.a_0"], ["A.b_0", "B.c_0", "C.a_1"]]
    """
    cycles: list[list[E]] = []

    order = {point: i for i, point in enumerate(graph)}
    glist = [
        [(edge, order[child]) for edge, child in children]
        for children in graph.values()
    ]
    visited = [False for _ in glist]
    edges: list[E] = []

    def visit(i: int):
        if i < k:
            return
        if visited[i]:
            if i == k:
                cycles.append(edges.copy())
            return
        visited[i] = True
        for edge, j in glist[i]:
            edges.append(edge)
            visit(j)
            edges.pop()
        visited[i] = False

    for k in range(len(glist)):
        visit(k)

    return cycles