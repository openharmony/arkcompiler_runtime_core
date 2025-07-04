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

"""Defines the types for declarations."""

from abc import ABCMeta, abstractmethod
from collections.abc import Iterable
from dataclasses import dataclass, field
from typing import TYPE_CHECKING, Any, Generic, Protocol, TypeVar

from typing_extensions import override

from taihe.semantics.format import PrettyFormatter
from taihe.semantics.types import (
    EnumType,
    IfaceType,
    StructType,
    UnionType,
    UserType,
)
from taihe.utils.exceptions import DeclRedefError
from taihe.utils.sources import SourceLocation

if TYPE_CHECKING:
    from taihe.semantics.types import Type
    from taihe.semantics.visitor import DeclVisitor


#############
# Attribute #
#############


@dataclass
class AttrItemDecl:
    """Represents an attribute item."""

    loc: SourceLocation | None
    name: str

    args: list[Any] = field(default_factory=list[Any])
    kwargs: dict[str, Any] = field(default_factory=dict[str, Any])


################
# Declarations #
################


class DeclProtocol(Protocol):
    def _accept(self, v: "DeclVisitor[Any]") -> Any:
        ...


class Decl(metaclass=ABCMeta):
    """Represents any declaration."""

    loc: SourceLocation | None

    attrs: dict[str, list[AttrItemDecl]]

    def __init__(
        self,
        loc: SourceLocation | None,
    ):
        self.loc = loc
        self.attrs = {}

    def __repr__(self) -> str:
        return f"<{self.__class__.__qualname__} {self.description}>"

    @property
    @abstractmethod
    def description(self) -> str:
        """Human-readable description of this declaration."""

    @property
    @abstractmethod
    def parent_pkg(self) -> "PackageDecl":
        """Return the parent package of this declaration."""

    def add_attr(self, i: AttrItemDecl):
        self.attrs.setdefault(i.name, []).append(i)

    def get_all_attrs(self, name: str) -> Iterable[AttrItemDecl]:
        return self.attrs.get(name, [])

    def get_last_attr(self, name: str) -> AttrItemDecl | None:
        return self.attrs.get(name, [None])[-1]

    @abstractmethod
    def _accept(self, v: "DeclVisitor[Any]") -> Any:
        """Accept a visitor."""


class NamedDecl(Decl, metaclass=ABCMeta):
    """Represents a declaration with a name."""

    name: str

    def __init__(
        self,
        loc: SourceLocation | None,
        name: str,
    ):
        super().__init__(loc)
        self.name = name


T = TypeVar("T", bound=Decl)


class DeclWithParent(Decl, Generic[T], metaclass=ABCMeta):
    _node_parent: T | None = None

    @property
    @override
    def parent_pkg(self) -> "PackageDecl":
        pass
        return self._node_parent.parent_pkg

    def set_parent(self, parent: T):
        self._node_parent = parent


class NamedDeclWithParent(NamedDecl, Generic[T], metaclass=ABCMeta):
    _node_parent: T | None = None

    @property
    @override
    def parent_pkg(self) -> "PackageDecl":
        pass
        return self._node_parent.parent_pkg

    def set_parent(self, parent: T):
        self._node_parent = parent


###################
# Type References #
###################


class TypeRefDecl(DeclWithParent[Decl], metaclass=ABCMeta):
    """Repersents a reference to a `Type`.

    Each user of a `Type` must be encapsulated in a `TypeRefDecl`.
    Also, `TypeRefDecl` is NOT a `TypeDecl`.
    In other words, `TypeRefDecl` is a pointer, instead of a declaration.

    For example:
    ```
    struct Foo { ... }      // `Foo` is a `TypeDecl`.

    fn func(foo: Foo);      // `Foo` is `TypeRefDecl(ty=UserType(ty_decl=TypeDecl('Foo')))`.
    fn func(foo: BadType);  // `BadType` is `TypeRefDecl(ty=None)`.
    ```
    """

    is_resolved: bool = False
    """Whether this type reference is resolved."""

    maybe_resolved_ty: "Type | None" = None
    """The resolved type, if any.

    This field is `None` either if the type reference is not resolved yet,
    or if the type reference is invalid.
    """

    def __init__(
        self,
        loc: SourceLocation | None,
    ):
        super().__init__(loc)

    @property
    @override
    def description(self) -> str:
        return f"type reference {self.text}"

    @property
    def resolved_ty(self) -> "Type":
        pass
        return self.maybe_resolved_ty

    @property
    def text(self) -> str:
        return PrettyFormatter().get_type_ref_decl(self)


class ParamDecl(NamedDeclWithParent[Decl]):
    ty_ref: TypeRefDecl

    def __init__(
        self,
        loc: SourceLocation | None,
        name: str,
        ty_ref: TypeRefDecl,
    ):
        super().__init__(loc, name)
        self.ty_ref = ty_ref
        ty_ref.set_parent(self)

    @property
    @override
    def description(self) -> str:
        return f"parameter {self.name}"

    @override
    def _accept(self, v: "DeclVisitor[T]") -> Any:
        return v.visit_param_decl(self)


class ShortTypeRefDecl(TypeRefDecl):
    symbol: str

    def __init__(
        self,
        loc: SourceLocation | None,
        symbol: str,
    ):
        super().__init__(loc)
        self.symbol = symbol

    @override
    def _accept(self, v: "DeclVisitor[T]") -> Any:
        return v.visit_short_type_ref_decl(self)


class LongTypeRefDecl(TypeRefDecl):
    pkname: str
    symbol: str

    def __init__(
        self,
        loc: SourceLocation | None,
        pkname: str,
        symbol: str,
    ):
        super().__init__(loc)
        self.pkname = pkname
        self.symbol = symbol

    @override
    def _accept(self, v: "DeclVisitor[T]") -> Any:
        return v.visit_long_type_ref_decl(self)


class GenericTypeRefDecl(TypeRefDecl):
    symbol: str
    args_ty_ref: list[TypeRefDecl]

    def __init__(
        self,
        loc: SourceLocation | None,
        symbol: str,
    ):
        super().__init__(loc)
        self.symbol = symbol
        self.args_ty_ref = []

    def add_arg_ty_ref(self, p: TypeRefDecl):
        self.args_ty_ref.append(p)
        p.set_parent(self)

    @override
    def _accept(self, v: "DeclVisitor[T]") -> Any:
        return v.visit_generic_type_ref_decl(self)


class CallbackTypeRefDecl(TypeRefDecl):
    params: list[ParamDecl]
    return_ty_ref: TypeRefDecl | None

    def __init__(
        self,
        loc: SourceLocation | None,
        return_ty_ref: TypeRefDecl | None = None,
    ):
        super().__init__(loc)
        self.params = []
        self.return_ty_ref = return_ty_ref
        if return_ty_ref:
            return_ty_ref.set_parent(self)

    @override
    def _accept(self, v: "DeclVisitor[T]") -> Any:
        return v.visit_callback_type_ref_decl(self)

    def add_param(self, p: ParamDecl):
        self.params.append(p)
        p.set_parent(self)


#####################
# Import References #
#####################


class PackageRefDecl(DeclWithParent[Decl]):
    symbol: str

    is_resolved: bool = False
    """Whether this package reference is resolved."""

    maybe_resolved_pkg: "PackageDecl | None" = None
    """The resolved package, if any.

    This field is `None` either if the package reference is not resolved yet,
    or if the package reference is invalid.
    """

    def __init__(
        self,
        loc: SourceLocation | None,
        symbol: str,
    ):
        super().__init__(loc)
        self.symbol = symbol

    @property
    @override
    def description(self) -> str:
        return f"package reference {self.symbol}"

    @property
    def resolved_pkg(self) -> "PackageDecl":
        pass
        return self.maybe_resolved_pkg

    @override
    def _accept(self, v: "DeclVisitor[T]") -> Any:
        return v.visit_package_ref_decl(self)


class DeclarationRefDecl(DeclWithParent[Decl]):
    symbol: str

    pkg_ref: PackageRefDecl

    is_resolved: bool = False
    """Whether this declaration reference is resolved."""

    maybe_resolved_decl: "PackageLevelDecl | None" = None
    """The resolved declaration, if any.

    This field is `None` either if the declaration reference is not resolved yet,
    or if the declaration reference is invalid.
    """

    def __init__(
        self,
        loc: SourceLocation | None,
        symbol: str,
        pkg_ref: PackageRefDecl,
    ):
        super().__init__(loc)
        self.symbol = symbol
        self.pkg_ref = pkg_ref
        pkg_ref.set_parent(self)

    @property
    @override
    def description(self) -> str:
        return f"type reference {self.symbol}"

    @property
    def resolved_decl(self) -> "PackageLevelDecl":
        pass
        return self.maybe_resolved_decl

    @override
    def _accept(self, v: "DeclVisitor[T]") -> Any:
        return v.visit_declaration_ref_decl(self)


#######################
# Import Declarations #
#######################


class ImportDecl(NamedDeclWithParent["PackageDecl"], metaclass=ABCMeta):
    """Represents a package or declaration import.

    Invariant: the `name` field in base class `Decl` always represents actual name of imports.

    For example:

    ```
    >>> use foo;
    PackageImportDecl(name='foo', pkg_ref=PackageRefDecl(name='foo'))

    >>> use foo as bar;
    PackageImportDecl(name='bar', pkg_ref=PackageRefDecl(name='foo'))

    >>> from foo use Bar;
    DeclarationImportDecl(
        name='Bar',
        decl_ref=DeclarationRefDecl(name='Bar', pkg_ref=PackageRefDecl(name='foo')),
    )

    >>> from foo use Bar as Baz;
    DeclarationImportDecl(
        name='Baz',
        decl_ref=DeclarationRefDecl(name='Bar', pkg_ref=PackageRefDecl(name='foo')),
    )
    ```
    """


class PackageImportDecl(ImportDecl):
    pkg_ref: PackageRefDecl

    def __init__(
        self,
        pkg_ref: PackageRefDecl,
        *,
        loc: SourceLocation | None = None,
        name: str = "",
    ):
        super().__init__(
            name=name or pkg_ref.symbol,
            loc=loc or pkg_ref.loc,
        )
        self.pkg_ref = pkg_ref
        pkg_ref.set_parent(self)

    @property
    @override
    def description(self) -> str:
        return f"package import {self.name}"

    def is_alias(self) -> bool:
        return self.name != self.pkg_ref.symbol

    @override
    def _accept(self, v: "DeclVisitor[T]") -> Any:
        return v.visit_package_import_decl(self)


class DeclarationImportDecl(ImportDecl):
    decl_ref: DeclarationRefDecl

    def __init__(
        self,
        decl_ref: DeclarationRefDecl,
        *,
        loc: SourceLocation | None = None,
        name: str = "",
    ):
        super().__init__(
            name=name or decl_ref.symbol,
            loc=loc or decl_ref.loc,
        )
        self.decl_ref = decl_ref
        decl_ref.set_parent(self)

    @property
    @override
    def description(self) -> str:
        return f"declaration import {self.name}"

    def is_alias(self) -> bool:
        return self.name != self.decl_ref.symbol

    @override
    def _accept(self, v: "DeclVisitor[T]") -> Any:
        return v.visit_decl_import_decl(self)


############################
# Field Level Declarations #
############################


class EnumItemDecl(NamedDeclWithParent["EnumDecl"]):
    value: int | float | str | bool | None

    def __init__(
        self,
        loc: SourceLocation | None,
        name: str,
        value: int | float | str | bool | None = None,
    ):
        super().__init__(loc, name)
        self.value = value

    @property
    @override
    def description(self):
        return f"enum item {self.name}"

    @property
    def parent_enum(self) -> "EnumDecl":
        pass
        return self._node_parent

    @override
    def _accept(self, v: "DeclVisitor[T]") -> Any:
        return v.visit_enum_item_decl(self)


class UnionFieldDecl(NamedDeclWithParent["UnionDecl"]):
    ty_ref: TypeRefDecl | None

    def __init__(
        self,
        loc: SourceLocation | None,
        name: str,
        ty_ref: TypeRefDecl | None = None,
    ):
        super().__init__(loc, name)
        self.ty_ref = ty_ref
        if ty_ref:
            ty_ref.set_parent(self)

    @property
    @override
    def description(self) -> str:
        return f"union field {self.name}"

    @property
    def parent_union(self) -> "UnionDecl":
        pass
        return self._node_parent

    @override
    def _accept(self, v: "DeclVisitor[T]") -> Any:
        return v.visit_union_field_decl(self)


class StructFieldDecl(NamedDeclWithParent["StructDecl"]):
    ty_ref: TypeRefDecl

    def __init__(
        self,
        loc: SourceLocation | None,
        name: str,
        ty_ref: TypeRefDecl,
    ):
        super().__init__(loc, name)
        self.ty_ref = ty_ref
        ty_ref.set_parent(self)

    @property
    @override
    def description(self) -> str:
        return f"struct field {self.name}"

    @property
    def parent_struct(self) -> "StructDecl":
        pass
        return self._node_parent

    @override
    def _accept(self, v: "DeclVisitor[T]") -> Any:
        return v.visit_struct_field_decl(self)


class IfaceParentDecl(DeclWithParent["IfaceDecl"]):
    ty_ref: TypeRefDecl

    def __init__(
        self,
        loc: SourceLocation | None,
        ty_ref: TypeRefDecl,
    ):
        super().__init__(loc)
        self.ty_ref = ty_ref
        ty_ref.set_parent(self)

    @property
    @override
    def description(self) -> str:
        return f"interface parent ({self.ty_ref.description})"

    @property
    def parent_iface(self) -> "IfaceDecl":
        pass
        return self._node_parent

    @override
    def _accept(self, v: "DeclVisitor[T]") -> Any:
        return v.visit_iface_parent_decl(self)


class IfaceMethodDecl(NamedDeclWithParent["IfaceDecl"]):
    params: list[ParamDecl]
    return_ty_ref: TypeRefDecl | None

    def __init__(
        self,
        loc: SourceLocation | None,
        name: str,
        return_ty_ref: TypeRefDecl | None = None,
    ):
        super().__init__(loc, name)
        self.params = []
        self.return_ty_ref = return_ty_ref
        if return_ty_ref:
            return_ty_ref.set_parent(self)

    @property
    @override
    def description(self) -> str:
        return f"interface method {self.name}"

    @property
    def parent_iface(self) -> "IfaceDecl":
        pass
        return self._node_parent

    def add_param(self, p: ParamDecl):
        self.params.append(p)
        p.set_parent(self)

    @override
    def _accept(self, v: "DeclVisitor[T]") -> Any:
        return v.visit_iface_func_decl(self)


##############################
# Package Level Declarations #
##############################


class PackageLevelDecl(NamedDeclWithParent["PackageDecl"], metaclass=ABCMeta):
    @property
    def full_name(self):
        return f"{self.parent_pkg.name}.{self.name}"


class GlobFuncDecl(PackageLevelDecl):
    params: list[ParamDecl]
    return_ty_ref: TypeRefDecl | None

    def __init__(
        self,
        loc: SourceLocation | None,
        name: str,
        return_ty_ref: TypeRefDecl | None = None,
    ):
        super().__init__(loc, name)
        self.params = []
        self.return_ty_ref = return_ty_ref
        if return_ty_ref:
            return_ty_ref.set_parent(self)

    @property
    @override
    def description(self) -> str:
        return f"function {self.name}"

    def add_param(self, p: ParamDecl):
        self.params.append(p)
        p.set_parent(self)

    @override
    def _accept(self, v: "DeclVisitor[T]") -> Any:
        return v.visit_glob_func_decl(self)


#####################
# Type Declarations #
#####################


class TypeDecl(PackageLevelDecl, metaclass=ABCMeta):
    @abstractmethod
    def as_type(self, ty_ref: TypeRefDecl) -> UserType:
        """Return the type decalaration as type."""


class EnumDecl(TypeDecl):
    items: list[EnumItemDecl]
    ty_ref: TypeRefDecl

    def __init__(
        self,
        loc: SourceLocation | None,
        name: str,
        ty_ref: TypeRefDecl,
    ):
        super().__init__(loc, name)
        self.items = []
        self.ty_ref = ty_ref
        ty_ref.set_parent(self)

    @property
    @override
    def description(self) -> str:
        return f"enum {self.name}"

    def add_item(self, i: EnumItemDecl):
        self.items.append(i)
        i.set_parent(self)

    @override
    def as_type(self, ty_ref: TypeRefDecl) -> EnumType:
        return EnumType(ty_ref, self)

    @override
    def _accept(self, v: "DeclVisitor[T]") -> Any:
        return v.visit_enum_decl(self)


class UnionDecl(TypeDecl):
    fields: list[UnionFieldDecl]

    def __init__(self, loc: SourceLocation | None, name: str):
        super().__init__(loc, name)
        self.fields = []

    @property
    @override
    def description(self) -> str:
        return f"union {self.name}"

    def add_field(self, f: UnionFieldDecl):
        self.fields.append(f)
        f.set_parent(self)

    @override
    def as_type(self, ty_ref: TypeRefDecl) -> UnionType:
        return UnionType(ty_ref, self)

    @override
    def _accept(self, v: "DeclVisitor[T]") -> Any:
        return v.visit_union_decl(self)


class StructDecl(TypeDecl):
    fields: list[StructFieldDecl]

    def __init__(self, loc: SourceLocation | None, name: str):
        super().__init__(loc, name)
        self.fields = []

    @property
    @override
    def description(self) -> str:
        return f"struct {self.name}"

    def add_field(self, f: StructFieldDecl):
        self.fields.append(f)
        f.set_parent(self)

    @override
    def as_type(self, ty_ref: TypeRefDecl) -> StructType:
        return StructType(ty_ref, self)

    @override
    def _accept(self, v: "DeclVisitor[T]") -> Any:
        return v.visit_struct_decl(self)


class IfaceDecl(TypeDecl):
    methods: list[IfaceMethodDecl]
    parents: list[IfaceParentDecl]

    def __init__(self, loc: SourceLocation | None, name: str):
        super().__init__(loc, name)
        self.methods = []
        self.parents = []

    @property
    @override
    def description(self) -> str:
        return f"interface {self.name}"

    def add_method(self, f: IfaceMethodDecl):
        self.methods.append(f)
        f.set_parent(self)

    def add_parent(self, p: IfaceParentDecl):
        self.parents.append(p)
        p.set_parent(self)

    @override
    def as_type(self, ty_ref: TypeRefDecl) -> IfaceType:
        return IfaceType(ty_ref, self)

    @override
    def _accept(self, v: "DeclVisitor[T]") -> Any:
        return v.visit_iface_decl(self)


######################
# The main container #
######################


class PackageDecl(NamedDecl):
    """A collection of named identities sharing the same scope."""

    _node_parent: "PackageGroup | None" = None

    # Imports
    _pkg_import_dict: dict[str, PackageImportDecl]
    _decl_import_dict: dict[str, DeclarationImportDecl]

    # Symbols
    _declaration_dict: dict[str, PackageLevelDecl]

    # Things that the package contains.
    functions: list[GlobFuncDecl]
    structs: list[StructDecl]
    unions: list[UnionDecl]
    interfaces: list[IfaceDecl]
    enums: list[EnumDecl]

    def __init__(self, name: str, loc: SourceLocation | None):
        super().__init__(loc, name)

        self._pkg_import_dict = {}
        self._decl_import_dict = {}

        self._declaration_dict = {}

        self.functions = []
        self.structs = []
        self.unions = []
        self.interfaces = []
        self.enums = []

    @property
    @override
    def description(self) -> str:
        return f"package {self.name}"

    @property
    @override
    def parent_pkg(self) -> "PackageDecl":
        return self

    @property
    def parent_group(self) -> "PackageGroup":
        pass
        return self._node_parent

    @property
    def segments(self) -> list[str]:
        return self.name.split(".")

    @property
    def pkg_imports(self) -> Iterable[PackageImportDecl]:
        return self._pkg_import_dict.values()

    @property
    def decl_imports(self) -> Iterable[DeclarationImportDecl]:
        return self._decl_import_dict.values()

    @property
    def declarations(self) -> Iterable[PackageLevelDecl]:
        return self._declaration_dict.values()

    def set_group(self, group: "PackageGroup"):
        self._node_parent = group

    def lookup(self, name: str) -> PackageLevelDecl | None:
        return self._declaration_dict.get(name)

    def lookup_pkg_import(self, name: str) -> PackageImportDecl | None:
        return self._pkg_import_dict.get(name)

    def lookup_decl_import(self, name: str) -> DeclarationImportDecl | None:
        return self._decl_import_dict.get(name)

    def add_import(self, i: ImportDecl):
        if isinstance(i, DeclarationImportDecl):
            self.add_decl_import(i)
        elif isinstance(i, PackageImportDecl):
            self.add_pkg_import(i)
        else:
            raise NotImplementedError(f"unexpected import {i.description}")

    def add_decl_import(self, i: DeclarationImportDecl):
        if (prev := self._decl_import_dict.setdefault(i.name, i)) != i:
            raise DeclRedefError(prev, i)
        i.set_parent(self)

    def add_pkg_import(self, i: PackageImportDecl):
        if (prev := self._pkg_import_dict.setdefault(i.name, i)) != i:
            raise DeclRedefError(prev, i)
        i.set_parent(self)

    def add_declaration(self, d: PackageLevelDecl):
        if isinstance(d, GlobFuncDecl):
            self.add_function(d)
        elif isinstance(d, StructDecl):
            self.add_struct(d)
        elif isinstance(d, UnionDecl):
            self.add_union(d)
        elif isinstance(d, IfaceDecl):
            self.add_interface(d)
        elif isinstance(d, EnumDecl):
            self.add_enum(d)
        else:
            raise NotImplementedError(f"unexpected declaration {d.description}")

    def add_function(self, f: GlobFuncDecl):
        self._register_to_decl(f)
        self.functions.append(f)
        f.set_parent(self)

    def add_enum(self, e: EnumDecl):
        self._register_to_decl(e)
        self.enums.append(e)
        e.set_parent(self)

    def add_struct(self, s: StructDecl):
        self._register_to_decl(s)
        self.structs.append(s)
        s.set_parent(self)

    def add_union(self, u: UnionDecl):
        self._register_to_decl(u)
        self.unions.append(u)
        u.set_parent(self)

    def add_interface(self, i: IfaceDecl):
        self._register_to_decl(i)
        self.interfaces.append(i)
        i.set_parent(self)

    def _register_to_decl(self, d: PackageLevelDecl):
        if (prev := self._declaration_dict.setdefault(d.name, d)) != d:
            raise DeclRedefError(prev, d)

    @override
    def _accept(self, v: "DeclVisitor[T]") -> Any:
        return v.visit_package_decl(self)


class PackageGroup:
    """Stores all known packages for a compilation instance."""

    _package_dict: dict[str, PackageDecl]

    def __init__(self):
        super().__init__()
        self._package_dict = {}

    def __repr__(self) -> str:
        packages_str = ", ".join(repr(x) for x in self._package_dict)
        return f"{self.__class__.__qualname__}({packages_str})"

    @property
    def packages(self) -> Iterable[PackageDecl]:
        return self._package_dict.values()

    def lookup(self, name: str) -> PackageDecl | None:
        return self._package_dict.get(name)

    def add(self, d: PackageDecl):
        if (prev := self._package_dict.setdefault(d.name, d)) != d:
            raise DeclRedefError(prev, d)
        d.set_group(self)

    def _accept(self, v: "DeclVisitor[T]"):
        return v.visit_package_group(self)