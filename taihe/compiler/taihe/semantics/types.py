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
from typing import TYPE_CHECKING, Any, Optional, Protocol

from typing_extensions import override

if TYPE_CHECKING:
    from taihe.semantics.declarations import (
        EnumDecl,
        IfaceDecl,
        StructDecl,
        TypeDecl,
        UnionDecl,
    )
    from taihe.semantics.visitor import TypeVisitor

############################
# Infrastructure for Types #
############################


class TypeProtocol(Protocol):
    def _accept(self, v: "TypeVisitor") -> Any: 
        ...


class Type(metaclass=ABCMeta):
    """Represents a concrete type."""

    @property
    @abstractmethod
    def representation(self) -> str:
        """Return the representation of the type."""

    def __repr__(self) -> str:
        return f"<{self.__class__.__qualname__} {self.representation}>"

    @abstractmethod
    def _accept(self, v: "TypeVisitor") -> Any:
        """Accept a visitor."""


##################
# Built-in Types #
##################


@dataclass(frozen=True, repr=False)
class BuiltinType(Type, metaclass=ABCMeta):
    """Represents built-in types, including scalars and strings.

    Invariant: all primitive types must be directy obtained with `lookup` or
    using the exported values such as `VOID`. Do not copy or construct it on
    your own. This design allows tests such as `my_type is VOID`.
    """

    name: str

    @property
    @override
    def representation(self):
        return f"{self.name}"


@dataclass(frozen=True, repr=False)
class ScalarType(BuiltinType):
    width: int
    is_signed: bool
    is_float: bool = False

    @override
    def _accept(self, v: "TypeVisitor") -> Any:
        return v.visit_scalar_type(self)


@dataclass(frozen=True, repr=False)
class OpaqueType(BuiltinType):
    name: str = "Opaque"

    @override
    def _accept(self, v: "TypeVisitor") -> Any:
        return v.visit_opaque_type(self)


@dataclass(frozen=True, repr=False)
class StringType(BuiltinType):
    name: str = "String"

    @override
    def _accept(self, v: "TypeVisitor") -> Any:
        return v.visit_string_type(self)


@dataclass(frozen=True, repr=False)
class BigIntType(BuiltinType):
    name: str = "BigInt"

    @override
    def _accept(self, v: "TypeVisitor") -> Any:
        return v.visit_bigint_type(self)


BOOL = ScalarType("bool", 8, is_signed=False)

F32 = ScalarType("f32", 32, is_signed=True, is_float=True)
F64 = ScalarType("f64", 64, is_signed=True, is_float=True)

I8 = ScalarType("i8", 8, is_signed=True)
I16 = ScalarType("i16", 16, is_signed=True)
I32 = ScalarType("i32", 32, is_signed=True)
I64 = ScalarType("i64", 64, is_signed=True)

U8 = ScalarType("u8", 8, is_signed=False)
U16 = ScalarType("u16", 16, is_signed=False)
U32 = ScalarType("u32", 32, is_signed=False)
U64 = ScalarType("u64", 64, is_signed=False)

STRING = StringType()
BIGINT = BigIntType()

OPAQUE = OpaqueType()

# Builtin Types map
BUILTIN_TYPES: dict[str, Type] = {
    ty.name: ty
    for ty in [
        BOOL,
        I8,
        I16,
        I32,
        I64,
        U8,
        U16,
        U32,
        U64,
        F32,
        F64,
        STRING,
        BIGINT,
        OPAQUE,
    ]
}


####################
# Builtin Generics #
####################


@dataclass(frozen=True, repr=False)
class CallbackType(Type):
    return_ty: Optional[Type]
    params_ty: tuple[Type, ...]

    @override
    def _accept(self, v: "TypeVisitor") -> Any:
        return v.visit_callback_type(self)

    @property
    @override
    def representation(self):
        return_fmt = ty.representation if (ty := self.return_ty) else "void"
        params_fmt = ", ".join(ty.representation for ty in self.params_ty)
        return f"({params_fmt}) => {return_fmt}"


class GenericType(Type, metaclass=ABCMeta):
    pass


@dataclass(frozen=True, repr=False)
class ArrayType(GenericType):
    item_ty: Type

    @override
    def _accept(self, v: "TypeVisitor") -> Any:
        return v.visit_array_type(self)

    @property
    @override
    def representation(self):
        return f"Array<{self.item_ty.representation}>"


@dataclass(frozen=True, repr=False)
class OptionalType(GenericType):
    item_ty: Type

    @override
    def _accept(self, v: "TypeVisitor") -> Any:
        return v.visit_optional_type(self)

    @property
    @override
    def representation(self):
        return f"Optional<{self.item_ty.representation}>"


@dataclass(frozen=True, repr=False)
class VectorType(GenericType):
    val_ty: Type

    @override
    def _accept(self, v: "TypeVisitor") -> Any:
        return v.visit_vector_type(self)

    @property
    @override
    def representation(self):
        return f"Vector<{self.val_ty.representation}>"


@dataclass(frozen=True, repr=False)
class MapType(GenericType):
    key_ty: Type
    val_ty: Type

    @override
    def _accept(self, v: "TypeVisitor") -> Any:
        return v.visit_map_type(self)

    @property
    @override
    def representation(self):
        return f"Map<{self.key_ty.representation}, {self.val_ty.representation}>"


@dataclass(frozen=True, repr=False)
class SetType(GenericType):
    key_ty: Type

    @override
    def _accept(self, v: "TypeVisitor") -> Any:
        return v.visit_set_type(self)

    @property
    @override
    def representation(self):
        return f"Set<{self.key_ty.representation}>"


class GenericBuilder(Protocol):
    def __call__(self, *args: Type) -> Type: 
        ...


# Builtin Generics Map
BUILTIN_GENERICS: dict[str, GenericBuilder] = {
    "Array": lambda *args: ArrayType(*args),
    "Optional": lambda *args: OptionalType(*args),
    "Vector": lambda *args: VectorType(*args),
    "Map": lambda *args: MapType(*args),
    "Set": lambda *args: SetType(*args),
}


##############
# User Types #
##############


class UserType(Type, metaclass=ABCMeta):
    ty_decl: "TypeDecl"

    @property
    @override
    def representation(self):
        return f"{self.ty_decl.full_name}"


@dataclass(frozen=True, repr=False)
class EnumType(UserType):
    ty_decl: "EnumDecl"

    @override
    def _accept(self, v: "TypeVisitor") -> Any:
        return v.visit_enum_type(self)


@dataclass(frozen=True, repr=False)
class StructType(UserType):
    ty_decl: "StructDecl"

    @override
    def _accept(self, v: "TypeVisitor") -> Any:
        return v.visit_struct_type(self)


@dataclass(frozen=True, repr=False)
class UnionType(UserType):
    ty_decl: "UnionDecl"

    @override
    def _accept(self, v: "TypeVisitor") -> Any:
        return v.visit_union_type(self)


@dataclass(frozen=True, repr=False)
class IfaceType(UserType):
    ty_decl: "IfaceDecl"

    @override
    def _accept(self, v: "TypeVisitor") -> Any:
        return v.visit_iface_type(self)
