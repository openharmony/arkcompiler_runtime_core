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

from collections.abc import Callable
from itertools import chain
from typing import Any, TypeGuard, TypeVar

from typing_extensions import override

from taihe.semantics.attributes import CheckedAttributeManager, UncheckedAttribute
from taihe.semantics.declarations import (
    CallbackTypeRefDecl,
    Decl,
    DeclarationRefDecl,
    EnumDecl,
    EnumItemDecl,
    GenericTypeRefDecl,
    IfaceDecl,
    IfaceParentDecl,
    LongTypeRefDecl,
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
    InvalidType,
    ScalarKind,
    ScalarType,
    StringType,
    UserType,
)
from taihe.semantics.visitor import RecursiveDeclVisitor
from taihe.utils.diagnostics import DiagnosticsManager
from taihe.utils.exceptions import (
    DeclarationNotInScopeError,
    DeclNotExistError,
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


def analyze_semantics(
    pg: PackageGroup,
    dm: DiagnosticsManager,
    am: CheckedAttributeManager,
):
    """Runs semantic analysis passes on the given package group."""
    # Namespace and declaration checks
    _check_decl_confilct_with_namespace(pg, dm)

    # Type related checks
    _ResolveImportsPass(dm).handle_decl(pg)
    _CheckEnumTypePass(dm).handle_decl(pg)
    _CheckRecursiveInclusionPass(dm).handle_decl(pg)

    # Resolve types and attributes, this pass must be run after all other passes
    _ConvertAttrPass(dm, am).handle_decl(pg)
    _CheckAttrPass(dm).handle_decl(pg)


def _check_decl_confilct_with_namespace(
    pg: PackageGroup,
    dm: DiagnosticsManager,
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
                dm.emit(SymbolConflictWithNamespaceError(d, name, packages))


class _ResolveImportsPass(RecursiveDeclVisitor):
    """Resolves imports and type references within a package group."""

    dm: DiagnosticsManager

    def __init__(self, dm: DiagnosticsManager):
        self._current_pg = None
        self._current_pkg = None  # Always points to the current package.
        self.dm = dm

    @property
    def current_pkg(self) -> PackageDecl:
        pass
        return self._current_pkg

    @property
    def current_pg(self) -> PackageGroup:
        pass
        return self._current_pg

    @override
    def visit_package_decl(self, p: PackageDecl) -> None:
        self._current_pkg = p
        try:
            super().visit_package_decl(p)
        finally:
            self._current_pkg = None

    @override
    def visit_package_group(self, g: PackageGroup) -> None:
        self._current_pg = g
        try:
            super().visit_package_group(g)
        finally:
            self._current_pg = None

    @override
    def visit_package_ref_decl(self, d: PackageRefDecl) -> None:
        if d.is_resolved:
            return
        d.is_resolved = True

        super().visit_package_ref_decl(d)

        pkg = self.current_pg.lookup(d.symbol)

        if pkg is not None:
            d.maybe_resolved_pkg = pkg
            return

        self.dm.emit(PackageNotExistError(d.symbol, loc=d.loc))
        d.maybe_resolved_pkg = None
        return

    @override
    def visit_declaration_ref_decl(self, d: DeclarationRefDecl) -> None:
        if d.is_resolved:
            return
        d.is_resolved = True

        super().visit_declaration_ref_decl(d)

        pkg = d.pkg_ref.maybe_resolved_pkg

        if pkg is not None:
            decl = pkg.lookup(d.symbol)

            if decl is not None:
                d.maybe_resolved_decl = decl
                return

            self.dm.emit(DeclNotExistError(d.symbol, loc=d.loc))
            d.maybe_resolved_decl = None
            return

        # No need to repeatedly throw exceptions
        d.maybe_resolved_decl = None
        return

    @override
    def visit_long_type_ref_decl(self, d: LongTypeRefDecl) -> None:
        if d.maybe_resolved_ty is not None:
            return

        super().visit_long_type_ref_decl(d)

        # Look for imported package declarations
        pkg_import = self.current_pkg.lookup_pkg_import(d.pkname)

        if pkg_import is not None:
            pkg = pkg_import.pkg_ref.maybe_resolved_pkg

            if pkg is not None:
                decl = pkg.lookup(d.symbol)

                if decl is not None:
                    if isinstance(decl, TypeDecl):
                        d.maybe_resolved_ty = decl.as_type(d)
                        return

                    self.dm.emit(NotATypeError(d.symbol, loc=d.loc))
                    d.maybe_resolved_ty = InvalidType(d)
                    return

                self.dm.emit(DeclNotExistError(d.symbol, loc=d.loc))
                d.maybe_resolved_ty = InvalidType(d)
                return

            # No need to repeatedly throw exceptions
            d.maybe_resolved_ty = InvalidType(d)
            return

        self.dm.emit(PackageNotInScopeError(d.pkname, loc=d.loc))
        d.maybe_resolved_ty = InvalidType(d)
        return

    @override
    def visit_short_type_ref_decl(self, d: ShortTypeRefDecl) -> None:
        if d.maybe_resolved_ty is not None:
            return

        super().visit_short_type_ref_decl(d)

        # Find Builtin Types
        builtin_type = BUILTIN_TYPES.get(d.symbol)

        if builtin_type is not None:
            d.maybe_resolved_ty = builtin_type(d)
            return

        # Find types declared in the current package
        decl = self.current_pkg.lookup(d.symbol)

        if decl is not None:
            if isinstance(decl, TypeDecl):
                d.maybe_resolved_ty = decl.as_type(d)
                return

            self.dm.emit(NotATypeError(d.symbol, loc=d.loc))
            d.maybe_resolved_ty = InvalidType(d)
            return

        # Look for imported type declarations
        decl_import = self.current_pkg.lookup_decl_import(d.symbol)

        if decl_import is not None:
            decl = decl_import.decl_ref.maybe_resolved_decl

            if decl is not None:
                if isinstance(decl, TypeDecl):
                    d.maybe_resolved_ty = decl.as_type(d)
                    return

                self.dm.emit(NotATypeError(d.symbol, loc=d.loc))
                d.maybe_resolved_ty = InvalidType(d)
                return

            # No need to repeatedly throw exceptions
            d.maybe_resolved_ty = InvalidType(d)
            return

        self.dm.emit(DeclarationNotInScopeError(d.symbol, loc=d.loc))
        d.maybe_resolved_ty = InvalidType(d)
        return

    @override
    def visit_generic_type_ref_decl(self, d: GenericTypeRefDecl) -> None:
        if d.maybe_resolved_ty is not None:
            return

        super().visit_generic_type_ref_decl(d)

        # Find Builtin Generics
        builtin_generic = BUILTIN_GENERICS.get(d.symbol)

        if builtin_generic is not None:
            args_ty = [arg_ty_ref.resolved_ty for arg_ty_ref in d.args_ty_ref]
            try:
                generic_type = builtin_generic.try_construct(d, *args_ty)
            except GenericArgumentsError as e:
                self.dm.emit(e)
                generic_type = InvalidType(d)
            d.maybe_resolved_ty = generic_type
            return

        self.dm.emit(DeclarationNotInScopeError(d.symbol, loc=d.loc))
        d.maybe_resolved_ty = InvalidType(d)
        return

    @override
    def visit_callback_type_ref_decl(self, d: CallbackTypeRefDecl) -> None:
        if d.maybe_resolved_ty is not None:
            return

        super().visit_callback_type_ref_decl(d)

        d.maybe_resolved_ty = CallbackType(d)
        return


class _CheckEnumTypePass(RecursiveDeclVisitor):
    """Validated enum item types."""

    dm: DiagnosticsManager

    def __init__(self, dm: DiagnosticsManager):
        self.dm = dm

    def visit_enum_decl(self, d: EnumDecl) -> None:
        def is_int(val: Any) -> TypeGuard[int]:
            return not isinstance(val, bool) and isinstance(val, int)

        valid: Callable[[Any], bool]
        increment: Callable[[Any, EnumItemDecl], Any]
        default: Callable[[EnumItemDecl], Any]

        match d.ty_ref.resolved_ty:
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
            case _:
                self.dm.emit(TypeUsageError(d.ty_ref))
                return

        prev = None
        for item in d.items:
            if item.value is None:
                item.value = default(item) if prev is None else increment(prev, item)
            if not valid(item.value):
                self.dm.emit(EnumValueError(item, d))
                prev = None
            else:
                prev = item.value


class _CheckRecursiveInclusionPass(RecursiveDeclVisitor):
    """Validates struct fields for type correctness and cycles."""

    dm: DiagnosticsManager

    def __init__(self, dm: DiagnosticsManager):
        self.dm = dm
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
            self.dm.emit(RecursiveReferenceError(last, other))

    def visit_enum_decl(self, d: EnumDecl) -> None:
        self.type_table[d] = []

    def visit_iface_decl(self, d: IfaceDecl) -> None:
        parent_iface_list = self.type_table.setdefault(d, [])
        parent_iface_dict: dict[IfaceDecl, IfaceParentDecl] = {}
        for parent in d.parents:
            if not isinstance(parent_ty := parent.ty_ref.resolved_ty, UserType):
                self.dm.emit(TypeUsageError(parent.ty_ref))
                continue
            if not isinstance(parent_iface := parent_ty.ty_decl, IfaceDecl):
                self.dm.emit(TypeUsageError(parent.ty_ref))
                continue
            parent_iface_list.append(((d, parent.ty_ref), parent_iface))
            prev = parent_iface_dict.setdefault(parent_iface, parent)
            if prev != parent:
                self.dm.emit(DuplicateExtendsWarn(prev, parent, d, parent_iface))

    def visit_struct_decl(self, d: StructDecl) -> None:
        type_list = self.type_table.setdefault(d, [])
        for f in d.fields:
            if isinstance(ty := f.ty_ref.resolved_ty, UserType):
                type_list.append(((d, f.ty_ref), ty.ty_decl))

    def visit_union_decl(self, d: UnionDecl) -> None:
        type_list = self.type_table.setdefault(d, [])
        for i in d.fields:
            if i.ty_ref is None:
                continue
            if isinstance(ty := i.ty_ref.resolved_ty, UserType):
                type_list.append(((d, i.ty_ref), ty.ty_decl))


class _ConvertAttrPass(RecursiveDeclVisitor):
    dm: DiagnosticsManager
    am: CheckedAttributeManager

    def __init__(self, dm: DiagnosticsManager, am: CheckedAttributeManager):
        self.dm = dm
        self.am = am

    def visit_decl(self, d: Decl) -> None:
        for unchecked_attr in UncheckedAttribute.consume(d):
            if (checked_attr := self.am.attach(unchecked_attr, self.dm)) is not None:
                d.add_attribute(checked_attr)


class _CheckAttrPass(RecursiveDeclVisitor):
    dm: DiagnosticsManager

    def __init__(self, dm: DiagnosticsManager):
        self.dm = dm

    def visit_decl(self, d: Decl) -> None:
        for attr in chain(*d.attributes.values()):
            attr.check_context(d, self.dm)


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