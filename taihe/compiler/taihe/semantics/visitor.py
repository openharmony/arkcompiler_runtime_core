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

"""Implements the classic visitor pattern for core types.

In most cases, you need to call `Visitor.handle_{type,decl}`.

Design:
- Each visitable type implements the "_accept" method, which delegates to the
    corresponding `VisitorBase.visit_xxx` method.
- `VisitorBase.visit_xxx` implements the default logic for each type.
    1. Calls `node._traverse` to visit the "children" nodes for declarations.
    2. Calls `self.visit_super_type` to bubble up towards the base type inside
    the type hierarchy.
- The `VisitorBase.visit_{type,decl}` is the "root" of the type hierarchy.
"""

from typing import Generic, Optional, TypeVar

from typing_extensions import override

from taihe.semantics.declarations import (
    CallbackTypeRefDecl,
    Decl,
    DeclarationImportDecl,
    DeclarationRefDecl,
    DeclProtocol,
    EnumDecl,
    EnumItemDecl,
    GenericTypeRefDecl,
    GlobFuncDecl,
    IfaceDecl,
    IfaceMethodDecl,
    IfaceParentDecl,
    ImportDecl,
    LongTypeRefDecl,
    PackageDecl,
    PackageGroup,
    PackageImportDecl,
    PackageRefDecl,
    ParamDecl,
    ShortTypeRefDecl,
    StructDecl,
    StructFieldDecl,
    TypeDecl,
    TypeRefDecl,
    UnionDecl,
    UnionFieldDecl,
)
from taihe.semantics.types import (
    ArrayType,
    BigIntType,
    BuiltinType,
    CallbackType,
    EnumType,
    GenericType,
    IfaceType,
    MapType,
    OpaqueType,
    OptionalType,
    ScalarType,
    SetType,
    StringType,
    StructType,
    Type,
    TypeProtocol,
    UnionType,
    UserType,
    VectorType,
)

T = TypeVar("T")


class TypeVisitor(Generic[T]):
    """A base visitor for traversing types in the type hierarchy.

    This visitor allows handling different types by defining specific visit methods.
    Unlike `DeclVisitor`, it does NOT recursively visit internal declarations.

    Usage:
    - Override specific `visit_*` methods for custom behavior.
    - Call `handle_type(t)` to start visiting a type.
    """

    visiting: Optional[TypeProtocol] = None
    """The current node being visited. Only for debug use."""

    def handle_type(self, t: TypeProtocol) -> T:
        """The entrance for visiting."""
        r = self.visiting
        self.visiting = t
        try:
            return t._accept(self)
        except:
            print(
                f"Internal error from {self.__class__.__name__} while handling {self.visiting}"
            )
            raise
        finally:
            self.visiting = r

    def visit_type(self, t: Type) -> T:
        """The fallback method which handles the most general type.

        Note that `TypeRef` is NOT a `Type`.
        """
        raise NotImplementedError

    ### Built-in types ###

    def visit_builtin_type(self, t: BuiltinType) -> T:
        return self.visit_type(t)

    def visit_scalar_type(self, t: ScalarType) -> T:
        return self.visit_builtin_type(t)

    def visit_opaque_type(self, t: OpaqueType) -> T:
        return self.visit_builtin_type(t)

    def visit_string_type(self, t: StringType) -> T:
        return self.visit_builtin_type(t)

    def visit_bigint_type(self, t: BigIntType) -> T:
        return self.visit_builtin_type(t)

    ### UserTypes ###

    def visit_user_type(self, t: UserType) -> T:
        return self.visit_type(t)

    def visit_enum_type(self, t: EnumType) -> T:
        return self.visit_user_type(t)

    def visit_struct_type(self, t: StructType) -> T:
        return self.visit_user_type(t)

    def visit_union_type(self, t: UnionType) -> T:
        return self.visit_user_type(t)

    def visit_iface_type(self, t: IfaceType) -> T:
        return self.visit_user_type(t)

    ### Generic Types ###

    def visit_callback_type(self, t: CallbackType) -> T:
        return self.visit_type(t)

    def visit_generic_type(self, t: GenericType) -> T:
        return self.visit_type(t)

    def visit_array_type(self, t: ArrayType) -> T:
        return self.visit_generic_type(t)

    def visit_optional_type(self, t: OptionalType) -> T:
        return self.visit_generic_type(t)

    def visit_vector_type(self, t: VectorType) -> T:
        return self.visit_generic_type(t)

    def visit_map_type(self, t: MapType) -> T:
        return self.visit_generic_type(t)

    def visit_set_type(self, t: SetType) -> T:
        return self.visit_generic_type(t)


class DeclVisitor(Generic[T]):
    """A base visitor for traversing declarations, including nested structures.

    This visitor walks through the declaration hierarchy, visiting each declaration
    and its associated types where applicable.

    Usage:
    - Override `visit_*` methods for specific behavior.
    - Call `handle_decl(d)` to start visiting a declaration.
    """

    visiting: Optional[DeclProtocol] = None
    """The current node being visited. Only for debug use."""

    def handle_decl(self, d: DeclProtocol) -> T:
        """The entrance for visiting anything "acceptable"."""
        r = self.visiting
        self.visiting = d
        try:
            return d._accept(self)
        except:
            print(
                f"Internal error from {self.__class__.__name__} while handling {self.visiting}"
            )
            raise
        finally:
            self.visiting = r

    def visit_decl(self, d: Decl) -> T:
        """The fallback method which handles the most general cases."""
        raise NotImplementedError

    def visit_param_decl(self, d: ParamDecl) -> T:
        return self.visit_decl(d)

    ### Type References ###

    def visit_type_ref_decl(self, d: TypeRefDecl) -> T:
        return self.visit_decl(d)

    def visit_short_type_ref_decl(self, d: ShortTypeRefDecl) -> T:
        return self.visit_type_ref_decl(d)

    def visit_long_type_ref_decl(self, d: LongTypeRefDecl) -> T:
        return self.visit_type_ref_decl(d)

    def visit_generic_type_ref_decl(self, d: GenericTypeRefDecl) -> T:
        return self.visit_type_ref_decl(d)

    def visit_callback_type_ref_decl(self, d: CallbackTypeRefDecl) -> T:
        return self.visit_type_ref_decl(d)

    ### Other References ###

    def visit_package_ref_decl(self, d: PackageRefDecl) -> T:
        return self.visit_decl(d)

    def visit_decl_ref_decl(self, d: DeclarationRefDecl) -> T:
        return self.visit_decl(d)

    ### Imports ###

    def visit_import_decl(self, d: ImportDecl) -> T:
        return self.visit_decl(d)

    def visit_package_import_decl(self, d: PackageImportDecl) -> T:
        return self.visit_import_decl(d)

    def visit_decl_import_decl(self, d: DeclarationImportDecl) -> T:
        return self.visit_import_decl(d)

    ### Package Level Function ###

    def visit_glob_func_decl(self, d: GlobFuncDecl) -> T:
        return self.visit_decl(d)

    ### Package Level Type ###

    def visit_type_decl(self, d: TypeDecl) -> T:
        return self.visit_decl(d)

    ### Enum ###

    def visit_enum_item_decl(self, d: EnumItemDecl) -> T:
        return self.visit_decl(d)

    def visit_enum_decl(self, d: EnumDecl) -> T:
        return self.visit_type_decl(d)

    ### Struct ###

    def visit_struct_field_decl(self, d: StructFieldDecl) -> T:
        return self.visit_decl(d)

    def visit_struct_decl(self, d: StructDecl) -> T:
        return self.visit_type_decl(d)

    ### Union ###

    def visit_union_field_decl(self, d: UnionFieldDecl) -> T:
        return self.visit_decl(d)

    def visit_union_decl(self, d: UnionDecl) -> T:
        return self.visit_type_decl(d)

    ### Interface ###

    def visit_iface_parent_decl(self, d: IfaceParentDecl) -> T:
        return self.visit_decl(d)

    def visit_iface_func_decl(self, d: IfaceMethodDecl) -> T:
        return self.visit_decl(d)

    def visit_iface_decl(self, d: IfaceDecl) -> T:
        return self.visit_type_decl(d)

    ### Package ###

    def visit_package_decl(self, p: PackageDecl) -> T:
        return self.visit_decl(p)

    def visit_package_group(self, g: PackageGroup) -> T:
        raise NotImplementedError


class RecursiveDeclVisitor(DeclVisitor[None]):
    """A visitor that recursively traverses all declarations and their sub-declarations.

    This class is useful for full-tree traversal scenarios.
    """

    @override
    def visit_decl(self, d: Decl) -> None:
        pass

    @override
    def visit_param_decl(self, d: ParamDecl) -> None:
        self.handle_decl(d.ty_ref)

        return self.visit_decl(d)

    ### Type References ###

    @override
    def visit_type_ref_decl(self, d: TypeRefDecl) -> None:
        return self.visit_decl(d)

    @override
    def visit_short_type_ref_decl(self, d: ShortTypeRefDecl) -> None:
        return self.visit_type_ref_decl(d)

    @override
    def visit_long_type_ref_decl(self, d: LongTypeRefDecl) -> None:
        return self.visit_type_ref_decl(d)

    @override
    def visit_generic_type_ref_decl(self, d: GenericTypeRefDecl) -> None:
        for i in d.args_ty_ref:
            self.handle_decl(i)

        return self.visit_type_ref_decl(d)

    @override
    def visit_callback_type_ref_decl(self, d: CallbackTypeRefDecl) -> None:
        for i in d.params:
            self.handle_decl(i)

        if d.return_ty_ref:
            self.handle_decl(d.return_ty_ref)

        return self.visit_type_ref_decl(d)

    ### Other References ###

    @override
    def visit_package_ref_decl(self, d: PackageRefDecl) -> None:
        return self.visit_decl(d)

    @override
    def visit_decl_ref_decl(self, d: DeclarationRefDecl) -> None:
        self.handle_decl(d.pkg_ref)

        return self.visit_decl(d)

    ### Imports ###

    @override
    def visit_import_decl(self, d: ImportDecl) -> None:
        return self.visit_decl(d)

    @override
    def visit_package_import_decl(self, d: PackageImportDecl) -> None:
        self.handle_decl(d.pkg_ref)

        return self.visit_import_decl(d)

    @override
    def visit_decl_import_decl(self, d: DeclarationImportDecl) -> None:
        self.handle_decl(d.decl_ref)

        return self.visit_import_decl(d)

    ### Functions ###

    @override
    def visit_glob_func_decl(self, d: GlobFuncDecl) -> None:
        for i in d.params:
            self.handle_decl(i)

        if d.return_ty_ref:
            self.handle_decl(d.return_ty_ref)

        return self.visit_decl(d)

    ### Type Decl ###

    @override
    def visit_type_decl(self, d: TypeDecl) -> None:
        return self.visit_decl(d)

    ### Enum ###

    @override
    def visit_enum_item_decl(self, d: EnumItemDecl) -> None:
        return self.visit_decl(d)

    @override
    def visit_enum_decl(self, d: EnumDecl) -> None:
        if d.ty_ref:
            self.handle_decl(d.ty_ref)

        for i in d.items:
            self.handle_decl(i)

        return self.visit_type_decl(d)

    ### Struct ###

    @override
    def visit_struct_field_decl(self, d: StructFieldDecl) -> None:
        self.handle_decl(d.ty_ref)

        return self.visit_decl(d)

    @override
    def visit_struct_decl(self, d: StructDecl) -> None:
        for i in d.fields:
            self.handle_decl(i)

        return self.visit_type_decl(d)

    ### Union ###

    @override
    def visit_union_field_decl(self, d: UnionFieldDecl) -> None:
        if d.ty_ref:
            self.handle_decl(d.ty_ref)

        return self.visit_decl(d)

    @override
    def visit_union_decl(self, d: UnionDecl) -> None:
        for i in d.fields:
            self.handle_decl(i)

        return self.visit_type_decl(d)

    ### Interface ###

    @override
    def visit_iface_parent_decl(self, d: IfaceParentDecl) -> None:
        self.handle_decl(d.ty_ref)

        return self.visit_decl(d)

    @override
    def visit_iface_func_decl(self, d: IfaceMethodDecl) -> None:
        for i in d.params:
            self.handle_decl(i)

        if d.return_ty_ref:
            self.handle_decl(d.return_ty_ref)

        return self.visit_decl(d)

    @override
    def visit_iface_decl(self, d: IfaceDecl) -> None:
        for i in d.parents:
            self.handle_decl(i)

        for i in d.methods:
            self.handle_decl(i)

        return self.visit_type_decl(d)

    ### Package ###

    @override
    def visit_package_decl(self, p: PackageDecl) -> None:
        for i in p.pkg_imports.values():
            self.handle_decl(i)
        for i in p.decl_imports.values():
            self.handle_decl(i)

        for i in p.functions:
            self.handle_decl(i)
        for i in p.enums:
            self.handle_decl(i)
        for i in p.structs:
            self.handle_decl(i)
        for i in p.unions:
            self.handle_decl(i)
        for i in p.interfaces:
            self.handle_decl(i)

        return self.visit_decl(p)

    @override
    def visit_package_group(self, g: PackageGroup) -> None:
        for i in g.packages:
            self.handle_decl(i)
