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

"""Defines the type system."""

from abc import ABCMeta, abstractmethod
from dataclasses import dataclass
from enum import Enum
from typing import TYPE_CHECKING, Protocol, TypeVar

from typing_extensions import override

if TYPE_CHECKING:
    from taihe.semantics.declarations import (
        EnumDecl,
        IfaceDecl,
        StructDecl,
        TypeDecl,
        TypeRefDecl,
        UnionDecl,
    )
    from taihe.semantics.visitor import TypeVisitor

T = TypeVar("T")

############################
# Infrastructure for Types #
############################


class TypeProtocol(Protocol):
    def _accept(self, v: "TypeVisitor[T]") -> T:
        ...


@dataclass(frozen=True, repr=False)
class Type(metaclass=ABCMeta):
    """Base class for all types."""

    ty_ref: "TypeRefDecl"

    def __repr__(self) -> str:
        return f"<{self.__class__.__qualname__} {self.signature}>"

    @property
    @abstractmethod
    def signature(self) -> str:
        """Return the representation of the type."""

    @abstractmethod
    def _accept(self, v: "TypeVisitor[T]") -> T:
        """Accept a visitor."""


##################
# Built-in Types #
##################


@dataclass(frozen=True, repr=False)
class BuiltinType(Type, metaclass=ABCMeta):
    """Represents a built-in type."""


class ScalarKind(Enum):
    """Enumeration of scalar types."""

    BOOL = ("bool", 8, False, False)
    F32 = ("f32", 32, True, True)
    F64 = ("f64", 64, True, True)
    I8 = ("i8", 8, True, False)
    I16 = ("i16", 16, True, False)
    I32 = ("i32", 32, True, False)
    I64 = ("i64", 64, True, False)
    U8 = ("u8", 8, False, False)
    U16 = ("u16", 16, False, False)
    U32 = ("u32", 32, False, False)
    U64 = ("u64", 64, False, False)

    def __init__(self, symbol: str, width: int, is_signed: bool, is_float: bool):
        self.symbol = symbol
        self.width = width
        self.is_signed = is_signed
        self.is_float = is_float


@dataclass(frozen=True, repr=False)
class ScalarType(BuiltinType):
    kind: ScalarKind

    @property
    @override
    def signature(self):
        return self.kind.symbol

    @override
    def _accept(self, v: "TypeVisitor[T]") -> T:
        return v.visit_scalar_type(self)


@dataclass(frozen=True, repr=False)
class OpaqueType(BuiltinType):
    @property
    @override
    def signature(self):
        return "Opaque"

    @override
    def _accept(self, v: "TypeVisitor[T]") -> T:
        return v.visit_opaque_type(self)


@dataclass(frozen=True, repr=False)
class StringType(BuiltinType):
    @property
    @override
    def signature(self):
        return "String"

    @override
    def _accept(self, v: "TypeVisitor[T]") -> T:
        return v.visit_string_type(self)


class BuiltinBuilder(Protocol):
    def __call__(self, ty_ref: "TypeRefDecl") -> BuiltinType:
        ...


# Builtin Types Map
BUILTIN_TYPES: dict[str, BuiltinBuilder] = {
    "bool": lambda ty_ref: ScalarType(ty_ref, ScalarKind.BOOL),
    "f32": lambda ty_ref: ScalarType(ty_ref, ScalarKind.F32),
    "f64": lambda ty_ref: ScalarType(ty_ref, ScalarKind.F64),
    "i8": lambda ty_ref: ScalarType(ty_ref, ScalarKind.I8),
    "i16": lambda ty_ref: ScalarType(ty_ref, ScalarKind.I16),
    "i32": lambda ty_ref: ScalarType(ty_ref, ScalarKind.I32),
    "i64": lambda ty_ref: ScalarType(ty_ref, ScalarKind.I64),
    "u8": lambda ty_ref: ScalarType(ty_ref, ScalarKind.U8),
    "u16": lambda ty_ref: ScalarType(ty_ref, ScalarKind.U16),
    "u32": lambda ty_ref: ScalarType(ty_ref, ScalarKind.U32),
    "u64": lambda ty_ref: ScalarType(ty_ref, ScalarKind.U64),
    "String": lambda ty_ref: StringType(ty_ref),
    "Opaque": lambda ty_ref: OpaqueType(ty_ref),
}


####################
# Builtin Generics #
####################


@dataclass(frozen=True, repr=False)
class CallbackType(Type):
    return_ty: Type | None
    params_ty: tuple[Type, ...]

    @property
    @override
    def signature(self):
        return_fmt = ty.signature if (ty := self.return_ty) else "void"
        params_fmt = ", ".join(ty.signature for ty in self.params_ty)
        return f"({params_fmt}) => {return_fmt}"

    @override
    def _accept(self, v: "TypeVisitor[T]") -> T:
        return v.visit_callback_type(self)


class GenericType(Type, metaclass=ABCMeta):
    pass


@dataclass(frozen=True, repr=False)
class ArrayType(GenericType):
    item_ty: Type

    @property
    @override
    def signature(self):
        return f"Array<{self.item_ty.signature}>"

    @override
    def _accept(self, v: "TypeVisitor[T]") -> T:
        return v.visit_array_type(self)


@dataclass(frozen=True, repr=False)
class OptionalType(GenericType):
    item_ty: Type

    @property
    @override
    def signature(self):
        return f"Optional<{self.item_ty.signature}>"

    @override
    def _accept(self, v: "TypeVisitor[T]") -> T:
        return v.visit_optional_type(self)


@dataclass(frozen=True, repr=False)
class VectorType(GenericType):
    val_ty: Type

    @property
    @override
    def signature(self):
        return f"Vector<{self.val_ty.signature}>"

    @override
    def _accept(self, v: "TypeVisitor[T]") -> T:
        return v.visit_vector_type(self)


@dataclass(frozen=True, repr=False)
class MapType(GenericType):
    key_ty: Type
    val_ty: Type

    @property
    @override
    def signature(self):
        return f"Map<{self.key_ty.signature}, {self.val_ty.signature}>"

    @override
    def _accept(self, v: "TypeVisitor[T]") -> T:
        return v.visit_map_type(self)


@dataclass(frozen=True, repr=False)
class SetType(GenericType):
    key_ty: Type

    @property
    @override
    def signature(self):
        return f"Set<{self.key_ty.signature}>"

    @override
    def _accept(self, v: "TypeVisitor[T]") -> T:
        return v.visit_set_type(self)


class GenericBuilder(Protocol):
    def __call__(self, ty_ref: "TypeRefDecl", *args: Type) -> GenericType:
        ...


# Builtin Generics Map
BUILTIN_GENERICS: dict[str, GenericBuilder] = {
    "Array": lambda ty_ref, *args: ArrayType(ty_ref, *args),
    "Optional": lambda ty_ref, *args: OptionalType(ty_ref, *args),
    "Vector": lambda ty_ref, *args: VectorType(ty_ref, *args),
    "Map": lambda ty_ref, *args: MapType(ty_ref, *args),
    "Set": lambda ty_ref, *args: SetType(ty_ref, *args),
}


##############
# User Types #
##############


@dataclass(frozen=True, repr=False)
class UserType(Type, metaclass=ABCMeta):
    ty_decl: "TypeDecl"

    @property
    @override
    def signature(self):
        return f"{self.ty_decl.full_name}"


@dataclass(frozen=True, repr=False)
class EnumType(UserType):
    ty_decl: "EnumDecl"

    @override
    def _accept(self, v: "TypeVisitor[T]") -> T:
        return v.visit_enum_type(self)


@dataclass(frozen=True, repr=False)
class StructType(UserType):
    ty_decl: "StructDecl"

    @override
    def _accept(self, v: "TypeVisitor[T]") -> T:
        return v.visit_struct_type(self)


@dataclass(frozen=True, repr=False)
class UnionType(UserType):
    ty_decl: "UnionDecl"

    @override
    def _accept(self, v: "TypeVisitor[T]") -> T:
        return v.visit_union_type(self)


@dataclass(frozen=True, repr=False)
class IfaceType(UserType):
    ty_decl: "IfaceDecl"

    @override
    def _accept(self, v: "TypeVisitor[T]") -> T:
        return v.visit_iface_type(self)