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

from dataclasses import dataclass, field
from enum import Enum
from types import UnionType
from typing import TYPE_CHECKING

from typing_extensions import override

from taihe.utils.diagnostics import DiagError, DiagFatalError, DiagNote, DiagWarn
from taihe.utils.sources import SourceLocation

if TYPE_CHECKING:
    from taihe.semantics.attributes import (
        AnyAttribute,
        Argument,
    )
    from taihe.semantics.declarations import (
        Decl,
        EnumDecl,
        EnumItemDecl,
        IfaceDecl,
        NamedDecl,
        PackageDecl,
        PackageLevelDecl,
        Type,
        TypeDecl,
        TypeRefDecl,
    )


@dataclass
class AttrArgOrderError(DiagError):
    def __init__(self, arg: "Argument"):
        super().__init__(loc=arg.loc)

    @override
    def describe(self) -> str:
        return (
            "Positioned arguments cannot follow keyword arguments in attribute calls."
        )


@dataclass
class AttrArgRedefNote(DiagNote):
    prev: "Argument"

    def __init__(self, prev: "Argument"):
        super().__init__(loc=prev.loc)
        self.prev = prev

    @override
    def describe(self) -> str:
        return "previously defined here"


@dataclass
class AttrArgRedefError(DiagError):
    prev: "Argument"
    current: "Argument"

    def __init__(self, prev: "Argument", current: "Argument"):
        super().__init__(loc=current.loc)
        self.prev = prev
        self.current = current

    @override
    def describe(self) -> str:
        return f"redefinition of key {self.current.key!r}"

    @override
    def notes(self):
        if self.prev.loc:
            yield AttrArgRedefNote(self.prev)


@dataclass
class AttrArgMissingError(DiagError):
    attr_name: str
    arg_name: str
    kw_only: bool = False

    @override
    def describe(self) -> str:
        if self.kw_only:
            kind = "keyword-only"
        else:
            kind = "positional or keyword"
        return f"Missing {kind} argument {self.arg_name!r} in attribute {self.attr_name!r}."


@dataclass
class AttrArgUnrequiredError(DiagError):
    attr_name: str
    attr_arg: "Argument"

    def __init__(self, attr_name: str, arg: "Argument"):
        super().__init__(loc=arg.loc)
        self.attr_name = attr_name
        self.attr_arg = arg

    @override
    def describe(self) -> str:
        if self.attr_arg.key is None:
            argument = "positional argument"
        else:
            argument = f"keyword argument {self.attr_arg.key!r}"
        return f"Unexpected {argument} in attribute {self.attr_name!r}."


@dataclass
class AttrArgTypeError(DiagError):
    attr_name: str
    arg_name: str
    arg_type: type | UnionType
    attr_arg: "Argument"

    def __init__(
        self,
        attr_name: str,
        arg_name: str,
        arg_type: type | UnionType,
        arg: "Argument",
    ):
        super().__init__(loc=arg.loc)
        self.attr_name = attr_name
        self.arg_name = arg_name
        self.arg_type = arg_type
        self.attr_arg = arg

    @override
    def describe(self) -> str:
        if isinstance(self.arg_type, UnionType):
            readable_type = " or ".join(t.__name__ for t in self.arg_type.__args__)
        else:
            readable_type = self.arg_type.__name__
        return f"Argument {self.arg_name!r} in attribute {self.attr_name} must be of type {readable_type}, but got {self.attr_arg.value!r}"


@dataclass
class AttrNotExistError(DiagError):
    name: str
    suggestion: list[str]

    @override
    def describe(self) -> str:
        msg = f"Unknown attribute: {self.name!r}"
        if self.suggestion:
            msg += ", possible candidates: "
            msg += ", ".join(f"{s!r}" for s in self.suggestion)
        return msg


@dataclass
class AttrConflictNote(DiagNote):
    prev: "AnyAttribute"

    def __init__(self, prev: "AnyAttribute"):
        super().__init__(loc=prev.loc)
        self.prev = prev

    @override
    def describe(self) -> str:
        return f"conflicting with {self.prev.description}"


@dataclass
class AttrConflictError(DiagError):
    current: "AnyAttribute"
    prev: "AnyAttribute"

    def __init__(
        self,
        current: "AnyAttribute",
        prev: "AnyAttribute",
    ):
        super().__init__(loc=current.loc)
        self.current = current
        self.prev = prev

    @override
    def describe(self) -> str:
        return f"cannot attach {self.current.description} due to conflict"

    @override
    def notes(self):
        yield AttrConflictNote(self.prev)


@dataclass
class AttrTargetError(DiagError):
    decl: "Decl"
    attr: "AnyAttribute"

    def __init__(self, decl: "Decl", attr: "AnyAttribute"):
        super().__init__(loc=decl.loc)
        self.decl = decl
        self.attr = attr

    @override
    def describe(self) -> str:
        return f"{self.attr.description} cannot be attached to {self.decl.description}"


@dataclass
class DeclRedefNote(DiagNote):
    prev: "NamedDecl"

    def __init__(self, prev: "NamedDecl"):
        super().__init__(loc=prev.loc)
        self.prev = prev

    @override
    def describe(self) -> str:
        return "previously defined here"


@dataclass
class DeclRedefError(DiagError):
    prev: "NamedDecl"
    current: "NamedDecl"

    def __init__(self, prev: "NamedDecl", current: "NamedDecl"):
        super().__init__(loc=current.loc)
        self.prev = prev
        self.current = current

    @override
    def describe(self) -> str:
        return f"redefinition of {self.current.description}"

    @override
    def notes(self):
        if self.prev.loc:
            yield DeclRedefNote(self.prev)


@dataclass
class IDLSyntaxError(DiagError):
    token: str

    @override
    def describe(self) -> str:
        return f"unexpected {self.token!r}"


@dataclass
class PackageNotExistError(DiagError):
    name: str

    @override
    def describe(self) -> str:
        return f"package {self.name!r} not exist"


@dataclass
class DeclNotExistError(DiagError):
    name: str

    @override
    def describe(self) -> str:
        return f"declaration {self.name!r} not exist"


@dataclass
class NotATypeError(DiagError):
    name: str

    @override
    def describe(self) -> str:
        return f"{self.name!r} is not a type name"


@dataclass
class DeclarationNotInScopeError(DiagError):
    name: str

    @override
    def describe(self) -> str:
        return (
            f"declaration name {self.name!r} is not declared or imported in this scope"
        )


@dataclass
class PackageNotInScopeError(DiagError):
    name: str

    @override
    def describe(self) -> str:
        return f"package name {self.name!r} is not imported in this scope"


@dataclass
class GenericArgumentsError(DiagError):
    ty_ref: "TypeRefDecl"

    def __init__(self, ty_ref: "TypeRefDecl", expected: int, got: int):
        super().__init__(loc=ty_ref.loc)
        self.ty_ref = ty_ref
        self.expected = expected
        self.got = got

    @override
    def describe(self) -> str:
        return f"Invalid generic arguments in {self.ty_ref.description!r}, expected {self.expected}, got {self.got}"


@dataclass
class SymbolConflictWithNamespaceNote(DiagNote):
    name: str
    package: "PackageDecl"

    def __init__(
        self,
        name: str,
        package: "PackageDecl",
    ):
        super().__init__(loc=package.loc)
        self.name = name
        self.package = package

    def describe(self) -> str:
        return f"namespace {self.name!r} is implied by {self.package.description}"


@dataclass
class SymbolConflictWithNamespaceError(DiagError):
    decl: "PackageLevelDecl"
    name: str
    packages: list["PackageDecl"]

    def __init__(
        self,
        decl: "PackageLevelDecl",
        name: str,
        packages: list["PackageDecl"],
    ):
        super().__init__(loc=decl.loc)
        self.decl = decl
        self.name = name
        self.packages = packages

    @override
    def describe(self) -> str:
        return f"declaration of {self.decl.description} conflicts with namespace {self.name!r}"

    def notes(self):
        for pkg in self.packages:
            yield SymbolConflictWithNamespaceNote(self.name, pkg)


@dataclass
class TypeUsageError(DiagError):
    ty: "Type"

    def __init__(self, ty_ref: "TypeRefDecl"):
        super().__init__(loc=ty_ref.loc)
        self.ty = ty_ref.resolved_ty

    @override
    def describe(self) -> str:
        return f"{self.ty.signature} cannot be used in this context"


@dataclass
class EnumValueError(DiagError):
    item: "EnumItemDecl"
    enum: "EnumDecl"

    def __init__(self, item: "EnumItemDecl", enum: "EnumDecl"):
        super().__init__(loc=item.loc)
        self.item = item
        self.enum = enum

    @override
    def describe(self) -> str:
        return f"value of {self.item.description} ({self.item.value}) is conflict with {self.enum.description} ({self.enum.ty_ref.resolved_ty.signature})"


@dataclass
class DuplicateExtendsNote(DiagNote):
    @override
    def describe(self) -> str:
        return "previously extended here"


@dataclass
class DuplicateExtendsWarn(DiagWarn):
    iface: "IfaceDecl"
    parent_iface: "IfaceDecl"
    prev_loc: SourceLocation | None = field(kw_only=True)

    @override
    def describe(self) -> str:
        return f"{self.parent_iface.description} is extended multiple times by {self.iface.description}"

    @override
    def notes(self):
        if self.prev_loc:
            yield DuplicateExtendsNote(loc=self.prev_loc)


@dataclass
class RecursiveReferenceNote(DiagNote):
    decl: "TypeDecl"

    def __init__(
        self,
        last: tuple["TypeDecl", "TypeRefDecl"],
    ):
        super().__init__(loc=last[1].loc)
        self.decl = last[0]

    @override
    def describe(self) -> str:
        return f"referenced by {self.decl.description}"


@dataclass
class RecursiveReferenceError(DiagError):
    decl: "TypeDecl"
    other: list[tuple["TypeDecl", "TypeRefDecl"]]

    def __init__(
        self,
        last: tuple["TypeDecl", "TypeRefDecl"],
        other: list[tuple["TypeDecl", "TypeRefDecl"]],
    ):
        super().__init__(loc=last[1].loc)
        self.decl = last[0]
        self.other = other

    @override
    def describe(self) -> str:
        return f"cycle detected in {self.decl.description}"

    @override
    def notes(self):
        for n in self.other:
            yield RecursiveReferenceNote(n)


class IgnoredFileReason(Enum):
    IS_DIRECTORY = "subdirectories are ignored"
    EXTENSION_MISMATCH = "unexpected file extension"


@dataclass
class IgnoredFileWarn(DiagWarn):
    reason: IgnoredFileReason

    @override
    def describe(self) -> str:
        return f"unrecognized file: {self.reason.value}"


@dataclass
class InvalidPackageNameError(DiagError):
    name: str

    @override
    def describe(self) -> str:
        return f"invalid package name {self.name!r}"


@dataclass
class AdhocNote(DiagNote):
    msg: str

    @override
    def describe(self) -> str:
        return self.msg


@dataclass
class AdhocWarn(DiagWarn):
    msg: str

    @override
    def describe(self) -> str:
        return self.msg


@dataclass
class AdhocError(DiagError):
    msg: str

    @override
    def describe(self) -> str:
        return self.msg


@dataclass
class AdhocFatalError(DiagFatalError):
    msg: str

    @override
    def describe(self) -> str:
        return self.msg