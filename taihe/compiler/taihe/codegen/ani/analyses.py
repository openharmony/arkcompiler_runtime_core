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

from abc import ABC, abstractmethod
from collections.abc import Iterable
from dataclasses import dataclass

from typing_extensions import override

from taihe.codegen.abi.writer import CSourceWriter
from taihe.codegen.ani.attributes import (
    ArrayBufferAttr,
    AsyncAttribute,
    BigIntAttr,
    ClassAttr,
    ConstAttr,
    ConstructorAttribute,
    CtorAttr,
    ExportDefaultAttr,
    ExtendsAttr,
    FixedArrayAttr,
    GenAsyncAttr,
    GenPromiseAttr,
    GetAttr,
    NamespaceAttr,
    NullAttr,
    OnOffAttr,
    OptionalAttr,
    OverloadAttr,
    PromiseAttribute,
    RecordAttr,
    RenameAttr,
    SetAttr,
    StaticAttr,
    StaticOverloadAttribute,
    StsFillAttr,
    StsInjectAttr,
    StsInjectIntoClazzAttr,
    StsInjectIntoIfaceAttr,
    StsInjectIntoModuleAttr,
    StsLastAttr,
    StsThisAttr,
    StsTypeAttr,
    TypedArrayAttr,
    UndefinedAttr,
)
from taihe.codegen.ani.writer import DefaultNaming, KeepNaming, StsWriter
from taihe.codegen.cpp.analyses import (
    EnumCppInfo,
    IfaceCppInfo,
    StructCppInfo,
    TypeCppInfo,
    UnionCppInfo,
)
from taihe.semantics.declarations import (
    EnumDecl,
    GlobFuncDecl,
    IfaceDecl,
    IfaceMethodDecl,
    IfaceParentDecl,
    PackageDecl,
    PackageGroup,
    ParamDecl,
    StructDecl,
    StructFieldDecl,
    UnionDecl,
    UnionFieldDecl,
)
from taihe.semantics.types import (
    ArrayType,
    CallbackType,
    EnumType,
    IfaceType,
    MapType,
    OpaqueType,
    OptionalType,
    ScalarKind,
    ScalarType,
    SetType,
    StringType,
    StructType,
    Type,
    UnionType,
)
from taihe.semantics.visitor import TypeVisitor
from taihe.utils.analyses import AbstractAnalysis, AnalysisManager

# ANI runtime types


@dataclass
class AniRuntimeType(ABC):
    @property
    @abstractmethod
    def sig(self) -> str:
        ...

    @property
    @abstractmethod
    def boxed(self) -> "AniRuntimeNonPrimitiveType":
        ...


@dataclass
class AniRuntimePrimitiveType(AniRuntimeType):
    sts_type: str
    type_sig: str

    @property
    def sig(self) -> str:
        return self.type_sig

    @property
    def boxed(self) -> "AniRuntimeNonPrimitiveType":
        return AniRuntimeClassType(name=f"std.core.{self.sts_type.capitalize()}")


@dataclass
class AniRuntimeNonPrimitiveType(AniRuntimeType, ABC):
    @property
    @abstractmethod
    def desc(self) -> str:
        ...

    @property
    def boxed(self) -> "AniRuntimeNonPrimitiveType":
        return self

    @abstractmethod
    def as_union_members(self) -> Iterable["AniRuntimeUnionMemberType"]:
        ...


@dataclass
class AniRuntimeUnionMemberType(AniRuntimeNonPrimitiveType, ABC):
    def as_union_members(self) -> Iterable["AniRuntimeUnionMemberType"]:
        yield self


@dataclass
class AniRuntimeUnionType(AniRuntimeNonPrimitiveType):
    members: list[AniRuntimeUnionMemberType]

    @property
    def sig(self) -> str:
        signatures = [union_member.sig for union_member in self.members]
        signatures = sorted(set(signatures))
        signatures_str = "".join(signatures)
        return f"X{{{signatures_str}}}"

    @property
    def desc(self) -> str:
        return self.sig

    @staticmethod
    def union(*sig_types: AniRuntimeType) -> AniRuntimeNonPrimitiveType:
        members = [
            member
            for sig_type in sig_types
            for member in sig_type.boxed.as_union_members()
        ]
        if len(members) == 0:
            return AniRuntimeUndefinedType()
        if len(members) == 1:
            return members[0]
        return AniRuntimeUnionType(members=members)

    def as_union_members(self) -> Iterable[AniRuntimeUnionMemberType]:
        yield from self.members


@dataclass
class AniRuntimeUndefinedType(AniRuntimeNonPrimitiveType):
    @property
    def sig(self) -> str:
        return "U"

    @property
    def desc(self) -> str:
        return self.sig

    def as_union_members(self) -> Iterable[AniRuntimeUnionMemberType]:
        yield from ()


@dataclass
class AniRuntimeNullType(AniRuntimeUnionMemberType):
    @property
    def desc(self) -> str:
        return self.sig

    @property
    def sig(self) -> str:
        return "N"


@dataclass
class AniRuntimeFixedArrayType(AniRuntimeUnionMemberType):
    element: AniRuntimeType

    @property
    def desc(self) -> str:
        return self.sig

    @property
    def sig(self) -> str:
        return f"A{{{self.element.sig}}}"


@dataclass
class AniRuntimeClassType(AniRuntimeUnionMemberType):
    name: str

    @property
    def desc(self) -> str:
        return self.name

    @property
    def sig(self) -> str:
        return f"C{{{self.name}}}"


@dataclass
class AniRuntimeEnumType(AniRuntimeUnionMemberType):
    name: str

    @property
    def desc(self) -> str:
        return self.name

    @property
    def sig(self) -> str:
        return f"E{{{self.name}}}"


# ANI Types


@dataclass(repr=False)
class ANIType:
    hint: str
    base: "ANIBaseType"

    def __repr__(self) -> str:
        return f"ani_{self.hint}"

    @property
    def suffix(self) -> str:
        return self.base.hint.capitalize()

    @property
    def fixedarray(self) -> "ANIFixedArrayType":
        pass
        return self.base.fixedarray_hint


@dataclass(repr=False)
class ANIFixedArrayType(ANIType):
    item: "ANIBaseType"

    def __init__(self, hint: str, item: "ANIBaseType"):
        super().__init__(hint, ANI_REF)
        self.item = item


@dataclass(repr=False)
class ANIBaseType(ANIType):
    fixedarray_hint: "ANIFixedArrayType"

    def __init__(self, hint: str):
        super().__init__(hint, self)


ANI_REF = ANIBaseType(hint="ref")
ANI_FIXEDARRAY_REF = ANIFixedArrayType(hint="fixedarray_ref", item=ANI_REF)
ANI_REF.fixedarray_hint = ANI_FIXEDARRAY_REF

ANI_BOOLEAN = ANIBaseType(hint="boolean")
ANI_FIXEDARRAY_BOOLEAN = ANIFixedArrayType(hint="fixedarray_boolean", item=ANI_BOOLEAN)
ANI_BOOLEAN.fixedarray_hint = ANI_FIXEDARRAY_BOOLEAN

ANI_FLOAT = ANIBaseType(hint="float")
ANI_FIXEDARRAY_FLOAT = ANIFixedArrayType(hint="fixedarray_float", item=ANI_FLOAT)
ANI_FLOAT.fixedarray_hint = ANI_FIXEDARRAY_FLOAT

ANI_DOUBLE = ANIBaseType(hint="double")
ANI_FIXEDARRAY_DOUBLE = ANIFixedArrayType(hint="fixedarray_double", item=ANI_DOUBLE)
ANI_DOUBLE.fixedarray_hint = ANI_FIXEDARRAY_DOUBLE

ANI_BYTE = ANIBaseType(hint="byte")
ANI_FIXEDARRAY_BYTE = ANIFixedArrayType(hint="fixedarray_byte", item=ANI_BYTE)
ANI_BYTE.fixedarray_hint = ANI_FIXEDARRAY_BYTE

ANI_SHORT = ANIBaseType(hint="short")
ANI_FIXEDARRAY_SHORT = ANIFixedArrayType(hint="fixedarray_short", item=ANI_SHORT)
ANI_SHORT.fixedarray_hint = ANI_FIXEDARRAY_SHORT

ANI_INT = ANIBaseType(hint="int")
ANI_FIXEDARRAY_INT = ANIFixedArrayType(hint="fixedarray_int", item=ANI_INT)
ANI_INT.fixedarray_hint = ANI_FIXEDARRAY_INT

ANI_LONG = ANIBaseType(hint="long")
ANI_FIXEDARRAY_LONG = ANIFixedArrayType(hint="fixedarray_long", item=ANI_LONG)
ANI_LONG.fixedarray_hint = ANI_FIXEDARRAY_LONG

ANI_OBJECT = ANIType(hint="object", base=ANI_REF)
ANI_ARRAY = ANIType(hint="array", base=ANI_REF)
ANI_FN_OBJECT = ANIType(hint="fn_object", base=ANI_REF)
ANI_ENUM_ITEM = ANIType(hint="enum_item", base=ANI_REF)
ANI_STRING = ANIType(hint="string", base=ANI_REF)
ANI_ARRAYBUFFER = ANIType(hint="arraybuffer", base=ANI_REF)


# ANI Function and Method


@dataclass(repr=False)
class ANIFuncLike:
    hint: str

    def __repr__(self) -> str:
        return f"ani_{self.hint}"

    @property
    def suffix(self) -> str:
        return self.hint.capitalize()

    @property
    def upper(self) -> str:
        return self.hint.upper()


ANI_FUNCTION = ANIFuncLike("function")
ANI_METHOD = ANIFuncLike("method")


# ANI Scopes


@dataclass(repr=False)
class ANIScope:
    hint: str
    member: ANIFuncLike

    def __repr__(self) -> str:
        return f"ani_{self.hint}"

    @property
    def suffix(self) -> str:
        return self.hint.capitalize()

    @property
    def upper(self) -> str:
        return self.hint.upper()


ANI_CLASS = ANIScope("class", ANI_METHOD)
ANI_MODULE = ANIScope("module", ANI_FUNCTION)
ANI_NAMESPACE = ANIScope("namespace", ANI_FUNCTION)


class Path:
    def __init__(self, package: str | None = None, path: str | None = None) -> None:
        self.package = package
        self.path = path
        self.ani_path = []
        if package is not None:
            self.ani_path.append(package)
        if path is not None:
            self.ani_path.extend(path.split("/"))


class Namespace:
    def __init__(self, name: str, parent: "Namespace | Path") -> None:
        self.name = name
        self.parent = parent

        self.children: dict[str, Namespace] = {}
        self.packages: list[PackageDecl] = []
        self.is_default = False
        self.injected_heads: list[str] = []
        self.injected_codes: list[str] = []

        if not isinstance(parent, Namespace):
            self.module = self
            self.path: list[str] = []
            self.scope = ANI_MODULE
            self.ani_path = [*parent.ani_path, *name.split(".")]
        else:
            self.module = parent.module
            self.path: list[str] = [*parent.path, name]
            self.scope = ANI_NAMESPACE
            self.ani_path = [*parent.ani_path, name]

        self.impl_desc = ".".join(self.ani_path)

    def add_path(
        self,
        path: list[str],
        pkg: PackageDecl,
        is_default: bool,
    ) -> "Namespace":
        if not path:
            self.packages.append(pkg)
            self.is_default |= is_default
            return self
        head, *tail = path
        child = self.children.setdefault(head, Namespace(head, self))
        return child.add_path(tail, pkg, is_default)

    def get_member(
        self,
        target: StsWriter,
        sts_name: str,
        member_is_default: bool,
    ) -> str:
        if not isinstance(self.parent, Namespace):
            filtered_name = "".join(c if c.isalnum() else "_" for c in self.name)
            if member_is_default:
                decl_name = f"_taihe_{filtered_name}_default"
                target.add_import_default(f"./{self.name}", decl_name)
                return decl_name
            scope_name = f"_taihe_{filtered_name}"
            target.add_import_module(f"./{self.name}", scope_name)
        else:
            scope_name = self.parent.get_member(target, self.name, self.is_default)
        return f"{scope_name}.{sts_name}"


class PackageGroupANIInfo(AbstractAnalysis[PackageGroup]):
    def __init__(self, am: AnalysisManager, pg: PackageGroup) -> None:
        self.am = am
        self.pg = pg

        self.module_dict: dict[str, Namespace] = {}
        self.package_map: dict[PackageDecl, Namespace] = {}

        self.path = Path(
            self.am.compiler_invocation.arkts_module_prefix,
            self.am.compiler_invocation.arkts_path_prefix,
        )

        for pkg in pg.packages:
            path = []
            if attr := NamespaceAttr.get(pkg):
                module_name = attr.module
                if ns := attr.namespace:
                    path = ns.split(".")
            else:
                module_name = pkg.name

            is_default = ExportDefaultAttr.get(pkg) is not None

            mod = self.module_dict.setdefault(
                module_name,
                Namespace(module_name, self.path),
            )
            ns = self.package_map[pkg] = mod.add_path(path, pkg, is_default)

            for attr in StsInjectIntoModuleAttr.get(pkg):
                mod.injected_heads.append(attr.sts_code)

            for attr in StsInjectAttr.get(pkg):
                ns.injected_codes.append(attr.sts_code)

    @classmethod
    @override
    def create(cls, am: AnalysisManager, pg: PackageGroup) -> "PackageGroupANIInfo":
        return PackageGroupANIInfo(am, pg)

    def get_namespace(self, pkg: PackageDecl) -> Namespace:
        return self.package_map[pkg]


class PackageANIInfo(AbstractAnalysis[PackageDecl]):
    def __init__(self, am: AnalysisManager, p: PackageDecl) -> None:
        self.am = am
        self.p = p

        self.header = f"{p.name}.ani.hpp"
        self.source = f"{p.name}.ani.cpp"

        self.cpp_ns = "::".join(p.segments)

        pg_ani_info = PackageGroupANIInfo.get(am, p.parent_group)
        self.ns = pg_ani_info.get_namespace(p)

        if self.am.compiler_invocation.sts_keep_name:
            self.naming = KeepNaming()
        else:
            self.naming = DefaultNaming()

    @classmethod
    @override
    def create(cls, am: AnalysisManager, p: PackageDecl) -> "PackageANIInfo":
        return PackageANIInfo(am, p)


class GlobFuncANIInfo(AbstractAnalysis[GlobFuncDecl]):
    def __init__(self, am: AnalysisManager, f: GlobFuncDecl) -> None:
        self.am = am
        self.f = f

        self.native_name = f"_taihe_{f.name}_native"
        self.revert_name = f"_taihe_{f.name}_revert"
        self.revert_prefix = "function "
        self.native_prefix = "native function "

        naming = PackageANIInfo.get(am, f.parent_pkg).naming

        if rename_attr := RenameAttr.get(f):
            func_name = rename_attr.name
        elif rename_attr := OverloadAttr.get(f):
            func_name = rename_attr.func_name
        else:
            func_name = naming.as_func(f.name)

        self.static_scope = None
        self.ctor_scope = None

        if old_ctor_attr := CtorAttr.get(f):
            self.ctor_scope = old_ctor_attr.cls_name
            func_name = ""
        elif ctor_attr := ConstructorAttribute.get(f):
            self.ctor_scope = ctor_attr.cls_name
        elif static_attr := StaticAttr.get(f):
            self.static_scope = static_attr.cls_name

        self.get_name = None
        self.set_name = None
        self.promise_name = None
        self.async_name = None
        self.norm_name = None

        self.gen_async_name = None
        self.gen_promise_name = None

        self.overload = None
        self.on_off = None

        if get_attr := GetAttr.get(f):
            if get_attr.member_name is not None:
                get_name = get_attr.member_name
            else:
                get_name = naming.as_field(get_attr.func_suffix)
            self.get_name = get_name
        elif set_attr := SetAttr.get(f):
            if set_attr.member_name is not None:
                set_name = set_attr.member_name
            else:
                set_name = naming.as_field(set_attr.func_suffix)
            self.set_name = set_name
        elif PromiseAttribute.get(f):
            self.promise_name = func_name
        elif AsyncAttribute.get(f):
            self.async_name = func_name
        else:
            self.norm_name = func_name
            if gen_async_attr := GenAsyncAttr.get(f):
                if gen_async_attr.func_name is not None:
                    self.gen_async_name = gen_async_attr.func_name
                else:
                    self.gen_async_name = naming.as_func(gen_async_attr.func_prefix)
            if gen_promise_attr := GenPromiseAttr.get(f):
                if gen_promise_attr.func_name is not None:
                    self.gen_promise_name = gen_promise_attr.func_name
                else:
                    self.gen_promise_name = naming.as_func(gen_promise_attr.func_prefix)

        if overload_attr := StaticOverloadAttribute.get(f):
            self.overload = overload_attr.name

        if on_off_attr := OnOffAttr.get(f):
            if on_off_attr.type is not None:
                on_off_type = on_off_attr.type
            else:
                on_off_type = naming.as_field(on_off_attr.func_suffix)
            self.on_off = (on_off_attr.overload, on_off_type)

        self.sts_params: list[ParamDecl] = []
        for param in f.params:
            if (
                StsThisAttr.get(param)
                or StsLastAttr.get(param)
                or StsFillAttr.get(param)
            ):
                continue
            self.sts_params.append(param)

        self.is_default = ExportDefaultAttr.get(f) is not None

    @classmethod
    @override
    def create(cls, am: AnalysisManager, f: GlobFuncDecl) -> "GlobFuncANIInfo":
        return GlobFuncANIInfo(am, f)

    def call_native(self, name: str) -> str:
        return name

    def call_native_with(self, sts_args: list[str], this: str = "this") -> list[str]:
        last = this
        arg = iter(sts_args)
        sts_native_args: list[str] = []
        for param in self.f.params:
            if StsThisAttr.get(param):
                sts_native_args.append(this)
            elif StsLastAttr.get(param):
                sts_native_args.append(last)
            elif fill_attr := StsFillAttr.get(param):
                sts_native_args.append(fill_attr.content)
            else:
                sts_native_args.append(last := next(arg))
        return sts_native_args

    def call_revert_with(self, sts_revert_args: list[str]) -> list[str]:
        sts_args: list[str] = []
        for param, arg in zip(self.f.params, sts_revert_args, strict=True):
            if (
                StsThisAttr.get(param)
                or StsLastAttr.get(param)
                or StsFillAttr.get(param)
            ):
                continue
            sts_args.append(arg)
        return sts_args


class IfaceMethodANIInfo(AbstractAnalysis[IfaceMethodDecl]):
    def __init__(self, am: AnalysisManager, f: IfaceMethodDecl) -> None:
        self.am = am
        self.f = f

        self.native_name = f"_taihe_{f.name}_native"
        self.revert_name = f"_taihe_{f.name}_revert"
        self.revert_prefix = ""
        self.native_prefix = "native "

        naming = PackageANIInfo.get(am, f.parent_pkg).naming

        if rename_attr := RenameAttr.get(f):
            func_name = rename_attr.name
        elif rename_attr := OverloadAttr.get(f):
            func_name = rename_attr.func_name
        else:
            func_name = naming.as_func(f.name)

        self.get_name = None
        self.set_name = None
        self.promise_name = None
        self.async_name = None
        self.norm_name = None

        self.gen_async_name = None
        self.gen_promise_name = None

        self.overload = None
        self.on_off = None

        if get_attr := GetAttr.get(f):
            if get_attr.member_name is not None:
                get_name = get_attr.member_name
            else:
                get_name = naming.as_field(get_attr.func_suffix)
            self.get_name = get_name
        elif set_attr := SetAttr.get(f):
            if set_attr.member_name is not None:
                set_name = set_attr.member_name
            else:
                set_name = naming.as_field(set_attr.func_suffix)
            self.set_name = set_name
        elif PromiseAttribute.get(f):
            self.promise_name = func_name
        elif AsyncAttribute.get(f):
            self.async_name = func_name
        else:
            self.norm_name = func_name
            if gen_async_attr := GenAsyncAttr.get(f):
                if gen_async_attr.func_name is not None:
                    self.gen_async_name = gen_async_attr.func_name
                else:
                    self.gen_async_name = naming.as_func(gen_async_attr.func_prefix)
            if gen_promise_attr := GenPromiseAttr.get(f):
                if gen_promise_attr.func_name is not None:
                    self.gen_promise_name = gen_promise_attr.func_name
                else:
                    self.gen_promise_name = naming.as_func(gen_promise_attr.func_prefix)

        if overload_attr := StaticOverloadAttribute.get(f):
            self.overload = overload_attr.name

        if on_off_attr := OnOffAttr.get(f):
            if on_off_attr.type is not None:
                on_off_type = on_off_attr.type
            else:
                on_off_type = naming.as_field(on_off_attr.func_suffix)
            self.on_off = (on_off_attr.overload, on_off_type)

        self.sts_params: list[ParamDecl] = []
        for param in f.params:
            if (
                StsThisAttr.get(param)
                or StsLastAttr.get(param)
                or StsFillAttr.get(param)
            ):
                continue
            self.sts_params.append(param)

    @classmethod
    @override
    def create(cls, am: AnalysisManager, f: IfaceMethodDecl) -> "IfaceMethodANIInfo":
        return IfaceMethodANIInfo(am, f)

    def call_native(self, name: str) -> str:
        return f"this.{name}" if name else "this"

    def call_native_with(self, sts_args: list[str], this: str = "this") -> list[str]:
        last = this
        arg = iter(sts_args)
        sts_native_args: list[str] = []
        for param in self.f.params:
            if StsThisAttr.get(param):
                sts_native_args.append(this)
            elif StsLastAttr.get(param):
                sts_native_args.append(last)
            elif fill_attr := StsFillAttr.get(param):
                sts_native_args.append(fill_attr.content)
            else:
                sts_native_args.append(last := next(arg))
        return sts_native_args

    def call_revert_with(self, sts_revert_args: list[str]) -> list[str]:
        sts_args: list[str] = []
        for param, arg in zip(self.f.params, sts_revert_args, strict=True):
            if (
                StsThisAttr.get(param)
                or StsLastAttr.get(param)
                or StsFillAttr.get(param)
            ):
                continue
            sts_args.append(arg)
        return sts_args


class EnumANIInfo(AbstractAnalysis[EnumDecl]):
    def __init__(self, am: AnalysisManager, d: EnumDecl) -> None:
        self.parent_ns = PackageANIInfo.get(am, d.parent_pkg).ns
        self.sts_type_name = d.name
        self.type_desc = ".".join([*self.parent_ns.ani_path, self.sts_type_name])

        self.is_default = ExportDefaultAttr.get(d) is not None

    @classmethod
    @override
    def create(cls, am: AnalysisManager, d: EnumDecl) -> "EnumANIInfo":
        return EnumANIInfo(am, d)

    def sts_type_in(self, target: StsWriter):
        return self.parent_ns.get_member(target, self.sts_type_name, self.is_default)


class UnionANIInfo(AbstractAnalysis[UnionDecl]):
    def __init__(self, am: AnalysisManager, d: UnionDecl) -> None:
        self.decl_header = f"{d.parent_pkg.name}.{d.name}.ani.0.hpp"
        self.impl_header = f"{d.parent_pkg.name}.{d.name}.ani.1.hpp"

        self.parent_ns = PackageANIInfo.get(am, d.parent_pkg).ns
        self.sts_type_name = d.name

        self.sts_all_somes: list[list[UnionFieldDecl]] = []
        self.sts_all_nones: list[list[UnionFieldDecl]] = []
        for field in d.fields:
            if field.ty_ref and isinstance(ty := field.ty_ref.resolved_ty, UnionType):
                inner_ani_info = UnionANIInfo.get(am, ty.ty_decl)
                self.sts_all_somes.extend(
                    [field, *parts] for parts in inner_ani_info.sts_all_somes
                )
                self.sts_all_nones.extend(
                    [field, *parts] for parts in inner_ani_info.sts_all_nones
                )
            elif field.ty_ref is not None:
                self.sts_all_somes.append([field])
            else:
                self.sts_all_nones.append([field])

        self.is_default = ExportDefaultAttr.get(d) is not None

    @classmethod
    @override
    def create(cls, am: AnalysisManager, d: UnionDecl) -> "UnionANIInfo":
        return UnionANIInfo(am, d)

    def sts_type_in(self, target: StsWriter):
        return self.parent_ns.get_member(target, self.sts_type_name, self.is_default)


class StructANIInfo(AbstractAnalysis[StructDecl]):
    def __init__(self, am: AnalysisManager, d: StructDecl) -> None:
        self.decl_header = f"{d.parent_pkg.name}.{d.name}.ani.0.hpp"
        self.impl_header = f"{d.parent_pkg.name}.{d.name}.ani.1.hpp"

        self.parent_ns = PackageANIInfo.get(am, d.parent_pkg).ns
        self.sts_type_name = d.name
        if ClassAttr.get(d):
            self.sts_impl_name = self.sts_type_name
        else:
            self.sts_impl_name = f"_taihe_{d.name}_inner"
        self.type_desc = ".".join([*self.parent_ns.ani_path, self.sts_type_name])
        self.impl_desc = ".".join([*self.parent_ns.ani_path, self.sts_impl_name])

        self.interface_injected_codes: list[str] = []
        for iface_injected in StsInjectIntoIfaceAttr.get(d):
            self.interface_injected_codes.append(iface_injected.sts_code)
        self.class_injected_codes: list[str] = []
        for class_injected in StsInjectIntoClazzAttr.get(d):
            self.class_injected_codes.append(class_injected.sts_code)

        self.sts_fields: list[StructFieldDecl] = []
        self.sts_iface_parents: list[StructFieldDecl] = []
        self.sts_class_parents: list[StructFieldDecl] = []
        self.sts_all_fields: list[list[StructFieldDecl]] = []
        for field in d.fields:
            if ExtendsAttr.get(field):
                ty = field.ty_ref.resolved_ty
                pass
                parent_ani_info = StructANIInfo.get(am, ty.ty_decl)
                if parent_ani_info.is_class():
                    self.sts_class_parents.append(field)
                else:
                    self.sts_iface_parents.append(field)
                self.sts_all_fields.extend(
                    [field, *parts] for parts in parent_ani_info.sts_all_fields
                )
            else:
                self.sts_fields.append(field)
                self.sts_all_fields.append([field])

        self.is_default = ExportDefaultAttr.get(d) is not None

    @classmethod
    @override
    def create(cls, am: AnalysisManager, d: StructDecl) -> "StructANIInfo":
        return StructANIInfo(am, d)

    def is_class(self):
        return self.sts_type_name == self.sts_impl_name

    def sts_type_in(self, target: StsWriter):
        return self.parent_ns.get_member(target, self.sts_type_name, self.is_default)


class IfaceANIInfo(AbstractAnalysis[IfaceDecl]):
    def __init__(self, am: AnalysisManager, d: IfaceDecl) -> None:
        self.decl_header = f"{d.parent_pkg.name}.{d.name}.ani.0.hpp"
        self.impl_header = f"{d.parent_pkg.name}.{d.name}.ani.1.hpp"

        self.parent_ns = PackageANIInfo.get(am, d.parent_pkg).ns
        self.sts_type_name = d.name
        if ClassAttr.get(d):
            self.sts_impl_name = self.sts_type_name
        else:
            self.sts_impl_name = f"_taihe_{d.name}_inner"
        self.type_desc = ".".join([*self.parent_ns.ani_path, self.sts_type_name])
        self.impl_desc = ".".join([*self.parent_ns.ani_path, self.sts_impl_name])

        self.interface_injected_codes: list[str] = []
        for iface_injected in StsInjectIntoIfaceAttr.get(d):
            self.interface_injected_codes.append(iface_injected.sts_code)
        self.class_injected_codes: list[str] = []
        for class_injected in StsInjectIntoClazzAttr.get(d):
            self.class_injected_codes.append(class_injected.sts_code)

        self.sts_class_parents: list[IfaceParentDecl] = []
        self.sts_iface_parents: list[IfaceParentDecl] = []
        for parent in d.parents:
            ty = parent.ty_ref.resolved_ty
            pass
            parent_ani_info = IfaceANIInfo.get(am, ty.ty_decl)
            if parent_ani_info.is_class():
                self.sts_class_parents.append(parent)
            else:
                self.sts_iface_parents.append(parent)

        self.is_default = ExportDefaultAttr.get(d) is not None

    @classmethod
    @override
    def create(cls, am: AnalysisManager, d: IfaceDecl) -> "IfaceANIInfo":
        return IfaceANIInfo(am, d)

    def is_class(self):
        return self.sts_type_name == self.sts_impl_name

    def sts_type_in(self, target: StsWriter):
        return self.parent_ns.get_member(target, self.sts_type_name, self.is_default)


class TypeANIInfo(AbstractAnalysis[Type], ABC):
    ani_type: ANIType
    sig_type: AniRuntimeType

    def __init__(self, am: AnalysisManager, t: Type):
        self.cpp_info = TypeCppInfo.get(am, t)

    @property
    def type_sig(self) -> str:
        return self.sig_type.sig

    @property
    def type_sig_boxed(self) -> str:
        return self.sig_type.boxed.sig

    @property
    def type_desc(self) -> str:
        return self.sig_type.boxed.desc

    @classmethod
    @override
    def create(cls, am: AnalysisManager, t: Type) -> "TypeANIInfo":
        return TypeANIInfoDispatcher(am).handle_type(t)

    @abstractmethod
    def sts_type_in(self, target: StsWriter) -> str:
        pass

    @abstractmethod
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        pass

    @abstractmethod
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        pass

    def into_ani_boxed(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        if self.ani_type.base == ANI_REF:
            self.into_ani(target, env, cpp_value, ani_result)
        else:
            ani_value = f"{ani_result}_ani"
            target.writelns(
                f"ani_object {ani_result} = {{}};",
            )
            self.into_ani(target, env, cpp_value, ani_value)
            target.writelns(
                f'{env}->Object_New(TH_ANI_FIND_CLASS({env}, "{self.type_desc}"), TH_ANI_FIND_CLASS_METHOD({env}, "{self.type_desc}", "<ctor>", "{self.type_sig}:"), &{ani_result}, {ani_value});',
            )

    def from_ani_boxed(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        if self.ani_type.base == ANI_REF:
            self.from_ani(
                target,
                env,
                f"static_cast<{self.ani_type}>({ani_value})",
                cpp_result,
            )
        else:
            ani_result = f"{cpp_result}_ani"
            target.writelns(
                f"{self.ani_type} {ani_result} = {{}};",
                f'{env}->Object_CallMethod_{self.ani_type.suffix}(static_cast<ani_object>({ani_value}), TH_ANI_FIND_CLASS_METHOD({env}, "{self.type_desc}", "unboxed", ":{self.type_sig}"), &{ani_result});',
            )
            self.from_ani(target, env, ani_result, cpp_result)

    def from_ani_fixedarray(
        self,
        target: CSourceWriter,
        env: str,
        ani_size: str,
        ani_fixedarray_value: str,
        cpp_fixedarray_buffer: str,
    ):
        if self.ani_type.base == ANI_REF:
            ani_value = f"{cpp_fixedarray_buffer}_ani_item"
            cpp_result = f"{cpp_fixedarray_buffer}_cpp_item"
            cpp_i = f"{cpp_fixedarray_buffer}_i"
            with target.indented(
                f"for (size_t {cpp_i} = 0; {cpp_i} < {ani_size}; {cpp_i}++) {{",
                f"}}",
            ):
                target.writelns(
                    f"{self.ani_type} {ani_value} = {{}};",
                    f"{env}->FixedArray_Get_Ref({ani_fixedarray_value}, {cpp_i}, reinterpret_cast<ani_ref*>(&{ani_value}));",
                )
                self.from_ani(target, env, ani_value, cpp_result)
                target.writelns(
                    f"new (&{cpp_fixedarray_buffer}[{cpp_i}]) {self.cpp_info.as_owner}(std::move({cpp_result}));",
                )
        else:
            target.writelns(
                f"{env}->FixedArray_GetRegion_{self.ani_type.suffix}({ani_fixedarray_value}, 0, {ani_size}, reinterpret_cast<{self.ani_type}*>({cpp_fixedarray_buffer}));",
            )

    def into_ani_fixedarray(
        self,
        target: CSourceWriter,
        env: str,
        cpp_size: str,
        cpp_fixedarray_value: str,
        ani_fixedarray_result: str,
    ):
        if self.ani_type.base == ANI_REF:
            ani_result = f"{ani_fixedarray_result}_item"
            ani_undefined = f"{ani_fixedarray_result}_undef"
            cpp_i = f"{ani_fixedarray_result}_i"
            target.writelns(
                f"ani_fixedarray_ref {ani_fixedarray_result} = {{}};",
                f"ani_ref {ani_undefined} = {{}};",
                f"{env}->GetUndefined(&{ani_undefined});",
                f'{env}->FixedArray_New_Ref(TH_ANI_FIND_CLASS({env}, "{self.type_desc}"), {cpp_size}, {ani_undefined}, &{ani_fixedarray_result});',
            )
            with target.indented(
                f"for (size_t {cpp_i} = 0; {cpp_i} < {cpp_size}; {cpp_i}++) {{",
                f"}}",
            ):
                self.into_ani(
                    target,
                    env,
                    f"{cpp_fixedarray_value}[{cpp_i}]",
                    ani_result,
                )
                target.writelns(
                    f"{env}->FixedArray_Set_Ref({ani_fixedarray_result}, {cpp_i}, {ani_result});",
                )
        else:
            target.writelns(
                f"{self.ani_type.fixedarray} {ani_fixedarray_result} = {{}};",
                f"{env}->FixedArray_New_{self.ani_type.suffix}({cpp_size}, &{ani_fixedarray_result});",
                f"{env}->FixedArray_SetRegion_{self.ani_type.suffix}({ani_fixedarray_result}, 0, {cpp_size}, reinterpret_cast<{self.ani_type} const*>({cpp_fixedarray_value}));",
            )


class EnumTypeANIInfo(TypeANIInfo):
    def __init__(self, am: AnalysisManager, t: EnumType):
        super().__init__(am, t)
        self.am = am
        self.t = t
        enum_ani_info = EnumANIInfo.get(self.am, self.t.ty_decl)
        self.ani_type = ANI_ENUM_ITEM
        self.sig_type = AniRuntimeEnumType(enum_ani_info.type_desc)

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        enum_ani_info = EnumANIInfo.get(self.am, self.t.ty_decl)
        return enum_ani_info.sts_type_in(target)

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        ani_index = f"{cpp_result}_idx"
        enum_cpp_info = EnumCppInfo.get(self.am, self.t.ty_decl)
        target.writelns(
            f"ani_size {ani_index} = {{}};",
            f"{env}->EnumItem_GetIndex({ani_value}, &{ani_index});",
            f"{enum_cpp_info.full_name} {cpp_result}(static_cast<{enum_cpp_info.full_name}::key_t>({ani_index}));",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        target.writelns(
            f"ani_enum_item {ani_result} = {{}};",
            f'{env}->Enum_GetEnumItemByIndex(TH_ANI_FIND_ENUM({env}, "{self.type_desc}"), static_cast<ani_size>({cpp_value}.get_key()), &{ani_result});',
        )


class ConstEnumTypeANIInfo(TypeANIInfo):
    def __init__(self, am: AnalysisManager, t: EnumType, const_attr: ConstAttr):
        super().__init__(am, t)
        self.am = am
        self.t = t
        self.const_attr = const_attr
        ty_ani_info = TypeANIInfo.get(self.am, self.t.ty_decl.ty_ref.resolved_ty)
        self.ani_type = ty_ani_info.ani_type
        self.sig_type = ty_ani_info.sig_type

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        ty_ani_info = TypeANIInfo.get(self.am, self.t.ty_decl.ty_ref.resolved_ty)
        return ty_ani_info.sts_type_in(target)

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        cpp_temp = f"{cpp_result}_cpp_temp"
        ty_ani_info = TypeANIInfo.get(self.am, self.t.ty_decl.ty_ref.resolved_ty)
        ty_ani_info.from_ani(target, env, ani_value, cpp_temp)
        enum_cpp_info = EnumCppInfo.get(self.am, self.t.ty_decl)
        target.writelns(
            f"{enum_cpp_info.full_name} {cpp_result} = {enum_cpp_info.full_name}::from_value({cpp_temp});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        cpp_temp = f"{ani_result}_cpp_temp"
        ty_cpp_info = TypeCppInfo.get(self.am, self.t.ty_decl.ty_ref.resolved_ty)
        ty_ani_info = TypeANIInfo.get(self.am, self.t.ty_decl.ty_ref.resolved_ty)
        target.writelns(
            f"{ty_cpp_info.as_owner} {cpp_temp} = {cpp_value}.get_value();",
        )
        ty_ani_info.into_ani(target, env, cpp_temp, ani_result)


class StructTypeANIInfo(TypeANIInfo):
    def __init__(self, am: AnalysisManager, t: StructType):
        super().__init__(am, t)
        self.am = am
        self.t = t
        struct_ani_info = StructANIInfo.get(self.am, self.t.ty_decl)
        self.ani_type = ANI_OBJECT
        self.sig_type = AniRuntimeClassType(struct_ani_info.type_desc)

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        struct_ani_info = StructANIInfo.get(self.am, self.t.ty_decl)
        return struct_ani_info.sts_type_in(target)

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        struct_ani_info = StructANIInfo.get(self.am, self.t.ty_decl)
        struct_cpp_info = StructCppInfo.get(self.am, self.t.ty_decl)
        target.add_include(struct_ani_info.impl_header)
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_result} = ::taihe::from_ani<{struct_cpp_info.as_owner}>({env}, {ani_value});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        struct_ani_info = StructANIInfo.get(self.am, self.t.ty_decl)
        struct_cpp_info = StructCppInfo.get(self.am, self.t.ty_decl)
        target.add_include(struct_ani_info.impl_header)
        target.writelns(
            f"ani_object {ani_result} = ::taihe::into_ani<{struct_cpp_info.as_owner}>({env}, {cpp_value});",
        )


class UnionTypeANIInfo(TypeANIInfo):
    def __init__(self, am: AnalysisManager, t: UnionType):
        super().__init__(am, t)
        self.am = am
        self.t = t
        self.ani_type = ANI_REF
        sig_types: list[AniRuntimeType] = []
        for field in t.ty_decl.fields:
            if (field_ty_ref := field.ty_ref) is not None:
                field_ani_info = TypeANIInfo.get(self.am, field_ty_ref.resolved_ty)
                sig_types.append(field_ani_info.sig_type)
            elif NullAttr.get(field):
                sig_types.append(AniRuntimeNullType())
            elif UndefinedAttr.get(field):
                sig_types.append(AniRuntimeUndefinedType())
        self.sig_type = AniRuntimeUnionType.union(*sig_types)

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        union_ani_info = UnionANIInfo.get(self.am, self.t.ty_decl)
        return union_ani_info.sts_type_in(target)

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        union_ani_info = UnionANIInfo.get(self.am, self.t.ty_decl)
        union_cpp_info = UnionCppInfo.get(self.am, self.t.ty_decl)
        target.add_include(union_ani_info.impl_header)
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_result} = ::taihe::from_ani<{union_cpp_info.as_owner}>({env}, {ani_value});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        union_ani_info = UnionANIInfo.get(self.am, self.t.ty_decl)
        union_cpp_info = UnionCppInfo.get(self.am, self.t.ty_decl)
        target.add_include(union_ani_info.impl_header)
        target.writelns(
            f"ani_ref {ani_result} = ::taihe::into_ani<{union_cpp_info.as_owner}>({env}, {cpp_value});",
        )


class IfaceTypeANIInfo(TypeANIInfo):
    def __init__(self, am: AnalysisManager, t: IfaceType):
        super().__init__(am, t)
        self.am = am
        self.t = t
        iface_ani_info = IfaceANIInfo.get(self.am, self.t.ty_decl)
        self.ani_type = ANI_OBJECT
        self.sig_type = AniRuntimeClassType(iface_ani_info.type_desc)

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        iface_ani_info = IfaceANIInfo.get(self.am, self.t.ty_decl)
        return iface_ani_info.sts_type_in(target)

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        iface_ani_info = IfaceANIInfo.get(self.am, self.t.ty_decl)
        iface_cpp_info = IfaceCppInfo.get(self.am, self.t.ty_decl)
        target.add_include(iface_ani_info.impl_header)
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_result} = ::taihe::from_ani<{iface_cpp_info.as_owner}>({env}, {ani_value});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        iface_ani_info = IfaceANIInfo.get(self.am, self.t.ty_decl)
        iface_cpp_info = IfaceCppInfo.get(self.am, self.t.ty_decl)
        target.add_include(iface_ani_info.impl_header)
        target.writelns(
            f"ani_object {ani_result} = ::taihe::into_ani<{iface_cpp_info.as_owner}>({env}, {cpp_value});",
        )


class ScalarTypeANIInfo(TypeANIInfo):
    def __init__(self, am: AnalysisManager, t: ScalarType):
        super().__init__(am, t)
        sts_info = {
            ScalarKind.BOOL: ("boolean", ANI_BOOLEAN, "z"),
            ScalarKind.F32: ("float", ANI_FLOAT, "f"),
            ScalarKind.F64: ("double", ANI_DOUBLE, "d"),
            ScalarKind.I8: ("byte", ANI_BYTE, "b"),
            ScalarKind.I16: ("short", ANI_SHORT, "s"),
            ScalarKind.I32: ("int", ANI_INT, "i"),
            ScalarKind.I64: ("long", ANI_LONG, "l"),
            ScalarKind.U8: ("byte", ANI_BYTE, "b"),
            ScalarKind.U16: ("short", ANI_SHORT, "s"),
            ScalarKind.U32: ("int", ANI_INT, "i"),
            ScalarKind.U64: ("long", ANI_LONG, "l"),
        }.get(t.kind)
        if sts_info is None:
            raise ValueError
        sts_type, ani_type, type_sig = sts_info
        self.sts_type = sts_type
        self.ani_type = ani_type
        self.sig_type = AniRuntimePrimitiveType(sts_type, type_sig)

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        return self.sts_type

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_result} = static_cast<{self.cpp_info.as_owner}>({ani_value});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        target.writelns(
            f"{self.ani_type} {ani_result} = static_cast<{self.cpp_info.as_owner}>({cpp_value});",
        )


class OpaqueTypeANIInfo(TypeANIInfo):
    def __init__(self, am: AnalysisManager, t: OpaqueType) -> None:
        super().__init__(am, t)
        self.am = am
        self.t = t
        self.ani_type = ANI_OBJECT
        if sts_type_attr := StsTypeAttr.get(self.t.ty_ref):
            self.sts_type = sts_type_attr.type_name
        else:
            self.sts_type = "Object"
        self.sig_type = AniRuntimeClassType("std.core.Object")

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        return self.sts_type

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_result} = reinterpret_cast<{self.cpp_info.as_owner}>({ani_value});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        target.writelns(
            f"{self.ani_type} {ani_result} = reinterpret_cast<{self.ani_type}>({cpp_value});",
        )


class StringTypeANIInfo(TypeANIInfo):
    def __init__(self, am: AnalysisManager, t: StringType):
        super().__init__(am, t)
        self.ani_type = ANI_STRING
        self.sig_type = AniRuntimeClassType("std.core.String")

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        return "string"

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        ani_length = f"{cpp_result}_len"
        cpp_tstr = f"{cpp_result}_tstr"
        cpp_buffer = f"{cpp_result}_buf"
        target.writelns(
            f"ani_size {ani_length} = {{}};",
            f"{env}->String_GetUTF8Size({ani_value}, &{ani_length});",
            f"TString {cpp_tstr};",
            f"char* {cpp_buffer} = tstr_initialize(&{cpp_tstr}, {ani_length} + 1);",
            f"{env}->String_GetUTF8({ani_value}, {cpp_buffer}, {ani_length} + 1, &{ani_length});",
            f"{cpp_buffer}[{ani_length}] = '\\0';",
            f"{cpp_tstr}.length = {ani_length};",
            f"::taihe::string {cpp_result} = ::taihe::string({cpp_tstr});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        target.writelns(
            f"ani_string {ani_result} = {{}};",
            f"{env}->String_NewUTF8({cpp_value}.c_str(), {cpp_value}.size(), &{ani_result});",
        )


class OptionalTypeANIInfo(TypeANIInfo):
    def __init__(self, am: AnalysisManager, t: OptionalType) -> None:
        super().__init__(am, t)
        self.am = am
        self.t = t
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        self.ani_type = ANI_REF
        self.sig_type = item_ty_ani_info.sig_type.boxed

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        sts_type = item_ty_ani_info.sts_type_in(target)
        return f"({sts_type} | undefined)"

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        ani_is_undefined = f"{cpp_result}_flag"
        cpp_pointer = f"{cpp_result}_ptr"
        cpp_spec = f"{cpp_result}_spec"
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        target.writelns(
            f"ani_boolean {ani_is_undefined} = {{}};",
            f"{item_ty_cpp_info.as_owner}* {cpp_pointer} = nullptr;",
            f"{env}->Reference_IsUndefined({ani_value}, &{ani_is_undefined});",
        )
        with target.indented(
            f"if (!{ani_is_undefined}) {{",
            f"}};",
        ):
            item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
            item_ty_ani_info.from_ani_boxed(target, env, ani_value, cpp_spec)
            target.writelns(
                f"{cpp_pointer} = new {item_ty_cpp_info.as_owner}(std::move({cpp_spec}));",
            )
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_result}({cpp_pointer});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        ani_spec = f"{ani_result}_spec"
        target.writelns(
            f"ani_ref {ani_result} = {{}};",
        )
        with target.indented(
            f"if (!{cpp_value}) {{",
            f"}}",
        ):
            target.writelns(
                f"{env}->GetUndefined(&{ani_result});",
            )
        with target.indented(
            f"else {{",
            f"}}",
        ):
            item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
            item_ty_ani_info.into_ani_boxed(target, env, f"(*{cpp_value})", ani_spec)
            target.writelns(
                f"{ani_result} = {ani_spec};",
            )


class FixedArrayTypeANIInfo(TypeANIInfo):
    def __init__(
        self,
        am: AnalysisManager,
        t: ArrayType,
        fixedarray_attr: FixedArrayAttr,
    ) -> None:
        super().__init__(am, t)
        self.am = am
        self.t = t
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        self.ani_type = item_ty_ani_info.ani_type.fixedarray
        self.sig_type = AniRuntimeFixedArrayType(item_ty_ani_info.sig_type)

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        sts_type = item_ty_ani_info.sts_type_in(target)
        return f"FixedArray<{sts_type}>"

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        ani_size = f"{cpp_result}_size"
        cpp_buffer = f"{cpp_result}_buffer"
        target.writelns(
            f"ani_size {ani_size} = {{}};",
            f"{env}->FixedArray_GetLength({ani_value}, &{ani_size});",
            f"{item_ty_cpp_info.as_owner}* {cpp_buffer} = reinterpret_cast<{item_ty_cpp_info.as_owner}*>(malloc({ani_size} * sizeof({item_ty_cpp_info.as_owner})));",
        )
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        item_ty_ani_info.from_ani_fixedarray(
            target,
            env,
            ani_size,
            ani_value,
            cpp_buffer,
        )
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_result}({cpp_buffer}, {ani_size});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        cpp_size = f"{ani_result}_size"
        target.writelns(
            f"size_t {cpp_size} = {cpp_value}.size();",
        )
        item_ty_ani_info.into_ani_fixedarray(
            target,
            env,
            cpp_size,
            f"{cpp_value}.data()",
            ani_result,
        )


class ArrayTypeANIInfo(TypeANIInfo):
    def __init__(self, am: AnalysisManager, t: ArrayType) -> None:
        super().__init__(am, t)
        self.am = am
        self.t = t
        self.ani_type = ANI_ARRAY
        self.sig_type = AniRuntimeClassType("escompat.Array")

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        sts_type = item_ty_ani_info.sts_type_in(target)
        return f"Array<{sts_type}>"

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        ani_size = f"{cpp_result}_size"
        cpp_buffer = f"{cpp_result}_buffer"
        ani_item = f"{cpp_buffer}_ani_item"
        cpp_item = f"{cpp_buffer}_cpp_item"
        cpp_ctr = f"{cpp_buffer}_i"
        target.writelns(
            f"ani_size {ani_size} = {{}};",
            f"{env}->Array_GetLength({ani_value}, &{ani_size});",
            f"{item_ty_cpp_info.as_owner}* {cpp_buffer} = reinterpret_cast<{item_ty_cpp_info.as_owner}*>(malloc({ani_size} * sizeof({item_ty_cpp_info.as_owner})));",
        )
        with target.indented(
            f"for (size_t {cpp_ctr} = 0; {cpp_ctr} < {ani_size}; {cpp_ctr}++) {{",
            f"}}",
        ):
            target.writelns(
                f"ani_ref {ani_item} = {{}};",
                f"{env}->Array_Get({ani_value}, {cpp_ctr}, &{ani_item});",
            )
            item_ty_ani_info.from_ani_boxed(target, env, ani_item, cpp_item)
            target.writelns(
                f"new (&{cpp_buffer}[{cpp_ctr}]) {item_ty_ani_info.cpp_info.as_owner}(std::move({cpp_item}));",
            )
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_result}({cpp_buffer}, {ani_size});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        cpp_size = f"{ani_result}_size"
        ani_item = f"{ani_result}_item"
        ani_undefined = f"{ani_result}_undef"
        cpp_ctr = f"{ani_result}_i"
        target.writelns(
            f"size_t {cpp_size} = {cpp_value}.size();",
            f"ani_array {ani_result} = {{}};",
            f"ani_ref {ani_undefined} = {{}};",
            f"{env}->GetUndefined(&{ani_undefined});",
            f"{env}->Array_New({cpp_size}, {ani_undefined}, &{ani_result});",
        )
        with target.indented(
            f"for (size_t {cpp_ctr} = 0; {cpp_ctr} < {cpp_size}; {cpp_ctr}++) {{",
            f"}}",
        ):
            item_ty_ani_info.into_ani_boxed(
                target,
                env,
                f"{cpp_value}[{cpp_ctr}]",
                ani_item,
            )
            target.writelns(
                f"{env}->Array_Set({ani_result}, {cpp_ctr}, {ani_item});",
            )


class ArrayBufferTypeANIInfo(TypeANIInfo):
    def __init__(
        self,
        am: AnalysisManager,
        t: ArrayType,
        arraybuffer_attr: ArrayBufferAttr,
    ) -> None:
        super().__init__(am, t)
        self.am = am
        self.t = t
        self.arraybuffer_attr = arraybuffer_attr
        self.ani_type = ANI_ARRAYBUFFER
        self.sig_type = AniRuntimeClassType("escompat.ArrayBuffer")

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        return "ArrayBuffer"

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        ani_data = f"{cpp_result}_data"
        ani_length = f"{cpp_result}_length"
        target.writelns(
            f"void* {ani_data} = {{}};",
            f"ani_size {ani_length} = {{}};",
            f"{env}->ArrayBuffer_GetInfo({ani_value}, &{ani_data}, &{ani_length});",
            f"{self.cpp_info.as_param} {cpp_result}(reinterpret_cast<{item_ty_cpp_info.as_owner}*>({ani_data}), {ani_length});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        ani_data = f"{ani_result}_data"
        target.writelns(
            f"void* {ani_data} = {{}};",
            f"ani_arraybuffer {ani_result} = {{}};",
            f"{env}->CreateArrayBuffer({cpp_value}.size() * (sizeof({item_ty_cpp_info.as_owner}) / sizeof(char)), &{ani_data}, &{ani_result});",
            f"std::copy({cpp_value}.begin(), {cpp_value}.end(), reinterpret_cast<{item_ty_cpp_info.as_owner}*>({ani_data}));",
        )


class TypedArrayTypeANIInfo(TypeANIInfo):
    def __init__(
        self,
        am: AnalysisManager,
        t: ArrayType,
        typedarray_attr: TypedArrayAttr,
    ) -> None:
        super().__init__(am, t)
        self.am = am
        self.t = t
        self.typedarray_attr = typedarray_attr
        self.ani_type = ANI_OBJECT
        self.sig_type = AniRuntimeClassType(f"escompat.{self.typedarray_attr.sts_type}")

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        return self.typedarray_attr.sts_type

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        ani_byte_length = f"{cpp_result}_bylen"
        ani_byte_offset = f"{cpp_result}_byoff"
        ani_arrbuf = f"{cpp_result}_arrbuf"
        ani_data = f"{cpp_result}_data"
        ani_length = f"{cpp_result}_length"
        target.writelns(
            f"ani_double {ani_byte_length} = {{}};",
            f"ani_double {ani_byte_offset} = {{}};",
            f"ani_arraybuffer {ani_arrbuf} = {{}};",
            f'{env}->Object_GetPropertyByName_Double({ani_value}, "byteLength", &{ani_byte_length});',
            f'{env}->Object_GetPropertyByName_Double({ani_value}, "byteOffset", &{ani_byte_offset});',
            f'{env}->Object_GetPropertyByName_Ref({ani_value}, "buffer", reinterpret_cast<ani_ref*>(&{ani_arrbuf}));',
            f"void* {ani_data} = {{}};",
            f"ani_size {ani_length} = {{}};",
            f"{env}->ArrayBuffer_GetInfo({ani_arrbuf}, &{ani_data}, &{ani_length});",
            f"{self.cpp_info.as_param} {cpp_result}(reinterpret_cast<{item_ty_cpp_info.as_owner}*>({ani_data}) + static_cast<size_t>({ani_byte_offset}), static_cast<size_t>({ani_byte_length}) / (sizeof({item_ty_cpp_info.as_owner}) / sizeof(char)));",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        ani_data = f"{ani_result}_data"
        ani_arrbuf = f"{ani_result}_arrbuf"
        ani_byte_length = f"{ani_result}_bylen"
        ani_byte_offset = f"{ani_result}_byoff"
        target.writelns(
            f"void* {ani_data} = {{}};",
            f"ani_arraybuffer {ani_arrbuf} = {{}};",
            f"{env}->CreateArrayBuffer({cpp_value}.size() * (sizeof({item_ty_cpp_info.as_owner}) / sizeof(char)), &{ani_data}, &{ani_arrbuf});",
            f"std::copy({cpp_value}.begin(), {cpp_value}.end(), reinterpret_cast<{item_ty_cpp_info.as_owner}*>({ani_data}));",
            f"ani_ref {ani_byte_length} = {{}};",
            f"{env}->GetUndefined(&{ani_byte_length});",
            f"ani_ref {ani_byte_offset} = {{}};",
            f"{env}->GetUndefined(&{ani_byte_offset});",
            f"ani_object {ani_result} = {{}};",
            f'{env}->Object_New(TH_ANI_FIND_CLASS({env}, "{self.type_desc}"), TH_ANI_FIND_CLASS_METHOD({env}, "{self.type_desc}", "<ctor>", "C{{escompat.ArrayBuffer}}C{{std.core.Double}}C{{std.core.Double}}:"), &{ani_result}, {ani_arrbuf}, {ani_byte_length}, {ani_byte_offset});',
        )


class BigIntTypeANIInfo(TypeANIInfo):
    def __init__(
        self,
        am: AnalysisManager,
        t: ArrayType,
        bigint_attr: BigIntAttr,
    ) -> None:
        super().__init__(am, t)
        self.am = am
        self.t = t
        self.bigint_attr = bigint_attr
        self.ani_type = ANI_OBJECT
        self.sig_type = AniRuntimeClassType("escompat.BigInt")

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        return "BigInt"

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        pkg_ani_info = PackageANIInfo.get(self.am, self.t.ty_ref.parent_pkg)
        ani_arrbuf = f"{cpp_result}_arrbuf"
        ani_data = f"{cpp_result}_data"
        ani_length = f"{cpp_result}_length"
        target.writelns(
            f"ani_arraybuffer {ani_arrbuf} = {{}};",
            f'{env}->Function_Call_Ref(TH_ANI_FIND_MODULE_FUNCTION({env}, "{pkg_ani_info.ns.module.impl_desc}", "_taihe_fromBigIntToArrayBuffer", nullptr), reinterpret_cast<ani_ref*>(&{ani_arrbuf}), {ani_value}, sizeof({item_ty_cpp_info.as_owner}) / sizeof(char));'
            f"void* {ani_data} = {{}};",
            f"ani_size {ani_length} = {{}};",
            f"{env}->ArrayBuffer_GetInfo({ani_arrbuf}, &{ani_data}, &{ani_length});",
            f"{self.cpp_info.as_param} {cpp_result}(reinterpret_cast<{item_ty_cpp_info.as_owner}*>({ani_data}), {ani_length} / (sizeof({item_ty_cpp_info.as_owner}) / sizeof(char)));",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        pkg_ani_info = PackageANIInfo.get(self.am, self.t.ty_ref.parent_pkg)
        ani_data = f"{ani_result}_data"
        ani_arrbuf = f"{ani_result}_arrbuf"
        target.writelns(
            f"void* {ani_data} = {{}};",
            f"ani_arraybuffer {ani_arrbuf} = {{}};",
            f"{env}->CreateArrayBuffer({cpp_value}.size() * (sizeof({item_ty_cpp_info.as_owner}) / sizeof(char)), &{ani_data}, &{ani_arrbuf});",
            f"std::copy({cpp_value}.begin(), {cpp_value}.end(), reinterpret_cast<{item_ty_cpp_info.as_owner}*>({ani_data}));",
            f"ani_object {ani_result} = {{}};",
            f'{env}->Function_Call_Ref(TH_ANI_FIND_MODULE_FUNCTION({env}, "{pkg_ani_info.ns.module.impl_desc}", "_taihe_fromArrayBufferToBigInt", nullptr), reinterpret_cast<ani_ref*>(&{ani_result}), {ani_arrbuf});',
        )


class RecordTypeANIInfo(TypeANIInfo):
    def __init__(
        self,
        am: AnalysisManager,
        t: MapType,
        record_attr: RecordAttr,
    ) -> None:
        super().__init__(am, t)
        self.am = am
        self.t = t
        self.record_attr = record_attr
        self.ani_type = ANI_OBJECT
        self.sig_type = AniRuntimeClassType("escompat.Record")

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        key_ty_ani_info = TypeANIInfo.get(self.am, self.t.key_ty)
        val_ty_ani_info = TypeANIInfo.get(self.am, self.t.val_ty)
        key_sts_type = key_ty_ani_info.sts_type_in(target)
        val_sts_type = val_ty_ani_info.sts_type_in(target)
        return f"Record<{key_sts_type}, {val_sts_type}>"

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        ani_iter = f"{cpp_result}_iter"
        ani_item = f"{cpp_result}_item"
        ani_next = f"{cpp_result}_next"
        ani_done = f"{cpp_result}_done"
        ani_key = f"{cpp_result}_ani_key"
        ani_val = f"{cpp_result}_ani_val"
        cpp_key = f"{cpp_result}_cpp_key"
        cpp_val = f"{cpp_result}_cpp_val"
        key_ty_ani_info = TypeANIInfo.get(self.am, self.t.key_ty)
        val_ty_ani_info = TypeANIInfo.get(self.am, self.t.val_ty)
        target.writelns(
            f"ani_object {ani_iter} = {{}};",
            f'{env}->Object_CallMethod_Ref({ani_value}, TH_ANI_FIND_CLASS_METHOD({env}, "escompat.Record", "$_iterator", nullptr), reinterpret_cast<ani_ref*>(&{ani_iter}));',
            f"{self.cpp_info.as_owner} {cpp_result};",
        )
        with target.indented(
            f"while (true) {{",
            f"}}",
        ):
            target.writelns(
                f"ani_object {ani_next} = {{}};",
                f"ani_boolean {ani_done} = {{}};",
                f'{env}->Object_CallMethod_Ref({ani_iter}, TH_ANI_FIND_CLASS_METHOD({env}, "escompat.MapIterator", "next", nullptr), reinterpret_cast<ani_ref*>(&{ani_next}));',
                f'{env}->Object_GetField_Boolean({ani_next}, TH_ANI_FIND_CLASS_FIELD({env}, "escompat.IteratorResult", "done"), &{ani_done});',
            )
            with target.indented(
                f"if ({ani_done}) {{;",
                f"}};",
            ):
                target.writelns(
                    f"break;",
                )
            target.writelns(
                f"ani_tuple_value {ani_item} = {{}};",
                f'{env}->Object_GetField_Ref({ani_next},  TH_ANI_FIND_CLASS_FIELD({env}, "escompat.IteratorResult", "value"), reinterpret_cast<ani_ref*>(&{ani_item}));',
                f"ani_ref {ani_key} = {{}};",
                f"{env}->TupleValue_GetItem_Ref({ani_item}, 0, &{ani_key});",
                f"ani_ref {ani_val} = {{}};",
                f"{env}->TupleValue_GetItem_Ref({ani_item}, 1, &{ani_val});",
            )
            key_ty_ani_info.from_ani_boxed(target, env, ani_key, cpp_key)
            val_ty_ani_info.from_ani_boxed(target, env, ani_val, cpp_val)
            target.writelns(
                f"{cpp_result}.emplace({cpp_key}, {cpp_val});",
            )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        key_ty_ani_info = TypeANIInfo.get(self.am, self.t.key_ty)
        val_ty_ani_info = TypeANIInfo.get(self.am, self.t.val_ty)
        cpp_key = f"{ani_result}_cpp_key"
        cpp_val = f"{ani_result}_cpp_val"
        ani_key = f"{ani_result}_ani_key"
        ani_val = f"{ani_result}_ani_val"
        target.writelns(
            f"ani_object {ani_result} = {{}};",
            f'{env}->Object_New(TH_ANI_FIND_CLASS({env}, "escompat.Record"), TH_ANI_FIND_CLASS_METHOD({env}, "escompat.Record", "<ctor>", nullptr), &{ani_result});',
        )
        with target.indented(
            f"for (const auto& [{cpp_key}, {cpp_val}] : {cpp_value}) {{",
            f"}}",
        ):
            key_ty_ani_info.into_ani_boxed(target, env, cpp_key, ani_key)
            val_ty_ani_info.into_ani_boxed(target, env, cpp_val, ani_val)
            target.writelns(
                f'{env}->Object_CallMethod_Void({ani_result}, TH_ANI_FIND_CLASS_METHOD({env}, "escompat.Record", "$_set", nullptr), {ani_key}, {ani_val});',
            )


class CallbackTypeANIInfo(TypeANIInfo):
    def __init__(self, am: AnalysisManager, t: CallbackType) -> None:
        super().__init__(am, t)
        self.am = am
        self.t = t
        self.ani_type = ANI_FN_OBJECT
        self.sig_type = AniRuntimeClassType(f"std.core.Function{len(t.ty_ref.params)}")

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        sts_params = []
        for param in self.t.ty_ref.params:
            opt = "?" if OptionalAttr.get(param) else ""
            type_ani_info = TypeANIInfo.get(self.am, param.ty_ref.resolved_ty)
            sts_params.append(f"{param.name}{opt}: {type_ani_info.sts_type_in(target)}")
        sts_params_str = ", ".join(sts_params)
        if return_ty_ref := self.t.ty_ref.return_ty_ref:
            type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
            sts_return_ty_name = type_ani_info.sts_type_in(target)
        else:
            sts_return_ty_name = "void"
        return f"(({sts_params_str}) => {sts_return_ty_name})"

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        cpp_impl_class = f"{cpp_result}_cpp_impl_t"
        with target.indented(
            f"struct {cpp_impl_class} : ::taihe::dref_guard {{",
            f"}};",
        ):
            target.writelns(
                f"{cpp_impl_class}(ani_env* env, ani_ref val) : ::taihe::dref_guard(env, val) {{}}",
            )
            self.gen_invoke_operator(target)
            with target.indented(
                f"uintptr_t getGlobalReference() const {{",
                f"}}",
            ):
                target.writelns(
                    f"return reinterpret_cast<uintptr_t>(this->ref);",
                )
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_result} = ::taihe::make_holder<{cpp_impl_class}, {self.cpp_info.as_owner}, ::taihe::platform::ani::AniObject>({env}, {ani_value});",
        )

    def gen_invoke_operator(
        self,
        target: CSourceWriter,
    ):
        inner_cpp_params = []
        inner_ani_args = []
        inner_cpp_args = []
        for param in self.t.ty_ref.params:
            inner_cpp_arg = f"cpp_arg_{param.name}"
            inner_ani_arg = f"ani_arg_{param.name}"
            type_cpp_info = TypeCppInfo.get(self.am, param.ty_ref.resolved_ty)
            inner_cpp_params.append(f"{type_cpp_info.as_param} {inner_cpp_arg}")
            inner_cpp_args.append(inner_cpp_arg)
            inner_ani_args.append(inner_ani_arg)
        cpp_params_str = ", ".join(inner_cpp_params)
        if return_ty_ref := self.t.ty_ref.return_ty_ref:
            type_cpp_info = TypeCppInfo.get(self.am, return_ty_ref.resolved_ty)
            return_ty_as_owner = type_cpp_info.as_owner
        else:
            return_ty_as_owner = "void"
        with target.indented(
            f"{return_ty_as_owner} operator()({cpp_params_str}) {{",
            f"}}",
        ):
            target.writelns(
                f"::taihe::env_guard guard;",
                f"ani_env *env = guard.get_env();",
            )
            for param, inner_cpp_arg, inner_ani_arg in zip(
                self.t.ty_ref.params,
                inner_cpp_args,
                inner_ani_args,
                strict=True,
            ):
                param_ty_ani_info = TypeANIInfo.get(self.am, param.ty_ref.resolved_ty)
                param_ty_ani_info.into_ani_boxed(
                    target,
                    "env",
                    inner_cpp_arg,
                    inner_ani_arg,
                )
            inner_ani_args_str = ", ".join(inner_ani_args)
            if return_ty_ref := self.t.ty_ref.return_ty_ref:
                inner_ani_res = "ani_result"
                inner_cpp_res = "cpp_result"
                type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
                target.writelns(
                    f"ani_ref ani_argv[] = {{{inner_ani_args_str}}};",
                    f"ani_ref {inner_ani_res} = {{}};",
                    f"env->FunctionalObject_Call(static_cast<ani_fn_object>(this->ref), {len(self.t.ty_ref.params)}, ani_argv, &{inner_ani_res});",
                )
                type_ani_info.from_ani_boxed(
                    target,
                    "env",
                    inner_ani_res,
                    inner_cpp_res,
                )
                target.writelns(
                    f"return {inner_cpp_res};",
                )
            else:
                inner_ani_res = "ani_result"
                target.writelns(
                    f"ani_ref ani_argv[] = {{{inner_ani_args_str}}};",
                    f"ani_ref {inner_ani_res} = {{}};",
                    f"env->FunctionalObject_Call(static_cast<ani_fn_object>(this->ref), {len(self.t.ty_ref.params)}, ani_argv, &{inner_ani_res});",
                    f"return;",
                )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        cpp_value_copy = f"{ani_result}_cpp_copy"
        cpp_struct = f"{ani_result}_cpp_struct"
        invoke_name = "invoke"
        ani_cast_ptr = f"{ani_result}_ani_cast_ptr"
        ani_func_ptr = f"{ani_result}_ani_func_ptr"
        ani_data_ptr = f"{ani_result}_ani_data_ptr"
        pkg_ani_info = PackageANIInfo.get(self.am, self.t.ty_ref.parent_pkg)
        with target.indented(
            f"struct {cpp_struct} {{",
            f"}};",
        ):
            self.gen_native_invoke(target, invoke_name)
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_value_copy} = {cpp_value};",
            f"ani_long {ani_cast_ptr} = reinterpret_cast<ani_long>(&{cpp_struct}::{invoke_name});",
            f"ani_long {ani_func_ptr} = reinterpret_cast<ani_long>({cpp_value_copy}.m_handle.vtbl_ptr);",
            f"ani_long {ani_data_ptr} = reinterpret_cast<ani_long>({cpp_value_copy}.m_handle.data_ptr);",
            f"{cpp_value_copy}.m_handle.data_ptr = nullptr;",
            f"ani_fn_object {ani_result} = {{}};",
            f'{env}->Function_Call_Ref(TH_ANI_FIND_MODULE_FUNCTION({env}, "{pkg_ani_info.ns.module.impl_desc}", "_taihe_makeCallback", nullptr), reinterpret_cast<ani_ref*>(&{ani_result}), {ani_cast_ptr}, {ani_func_ptr}, {ani_data_ptr});',
        )

    def gen_native_invoke(
        self,
        target: CSourceWriter,
        cpp_cast_ptr: str,
    ):
        ani_params = []
        ani_args = []
        ani_params.append("[[maybe_unused]] ani_env* env")
        ani_params.append("[[maybe_unused]] ani_long ani_func_ptr")
        ani_params.append("[[maybe_unused]] ani_long ani_data_ptr")
        for i in range(16):
            ani_param_name = f"ani_arg_{i}"
            ani_params.append(f"[[maybe_unused]] ani_ref {ani_param_name}")
            ani_args.append(ani_param_name)
        ani_params_str = ", ".join(ani_params)
        cpp_args = []
        for i in self.t.ty_ref.params:
            cpp_arg = f"cpp_arg_{i.name}"
            cpp_args.append(cpp_arg)
        ani_return_type = "ani_ref"
        with target.indented(
            f"static {ani_return_type} {cpp_cast_ptr}({ani_params_str}) {{",
            f"}};",
        ):
            target.writelns(
                f"{self.cpp_info.as_param}::vtable_type* cpp_vtbl_ptr = reinterpret_cast<{self.cpp_info.as_param}::vtable_type*>(ani_func_ptr);",
                f"DataBlockHead* cpp_data_ptr = reinterpret_cast<DataBlockHead*>(ani_data_ptr);",
                f"{self.cpp_info.as_param} cpp_func = {self.cpp_info.as_param}({{cpp_vtbl_ptr, cpp_data_ptr}});",
            )
            for param, ani_arg, cpp_arg in zip(
                self.t.ty_ref.params, ani_args, cpp_args, strict=False
            ):
                type_ani_info = TypeANIInfo.get(self.am, param.ty_ref.resolved_ty)
                type_ani_info.from_ani_boxed(target, "env", ani_arg, cpp_arg)
            cpp_args_str = ", ".join(cpp_args)
            if return_ty_ref := self.t.ty_ref.return_ty_ref:
                type_cpp_info = TypeCppInfo.get(self.am, return_ty_ref.resolved_ty)
                type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
                cpp_res = "cpp_result"
                ani_res = "ani_result"
                target.writelns(
                    f"{type_cpp_info.as_owner} {cpp_res} = cpp_func({cpp_args_str});",
                    f"if (::taihe::has_error()) {{ return ani_ref{{}}; }}",
                )
                type_ani_info.into_ani_boxed(target, "env", cpp_res, ani_res)
                target.writelns(
                    f"return {ani_res};",
                )
            else:
                target.writelns(
                    f"cpp_func({cpp_args_str});",
                    f"return ani_ref{{}};",
                )


class TypeANIInfoDispatcher(TypeVisitor[TypeANIInfo]):
    def __init__(self, am: AnalysisManager):
        self.am = am

    @override
    def visit_enum_type(self, t: EnumType) -> TypeANIInfo:
        if const_attr := ConstAttr.get(t.ty_decl):
            return ConstEnumTypeANIInfo(self.am, t, const_attr)
        return EnumTypeANIInfo(self.am, t)

    @override
    def visit_union_type(self, t: UnionType) -> TypeANIInfo:
        return UnionTypeANIInfo(self.am, t)

    @override
    def visit_struct_type(self, t: StructType) -> TypeANIInfo:
        return StructTypeANIInfo(self.am, t)

    @override
    def visit_iface_type(self, t: IfaceType) -> TypeANIInfo:
        return IfaceTypeANIInfo(self.am, t)

    @override
    def visit_scalar_type(self, t: ScalarType) -> TypeANIInfo:
        return ScalarTypeANIInfo(self.am, t)

    @override
    def visit_string_type(self, t: StringType) -> TypeANIInfo:
        return StringTypeANIInfo(self.am, t)

    @override
    def visit_array_type(self, t: ArrayType) -> TypeANIInfo:
        if bigint_attr := BigIntAttr.get(t.ty_ref):
            return BigIntTypeANIInfo(self.am, t, bigint_attr)
        if typedarray_attr := TypedArrayAttr.get(t.ty_ref):
            return TypedArrayTypeANIInfo(self.am, t, typedarray_attr)
        if arraybuffer_attr := ArrayBufferAttr.get(t.ty_ref):
            return ArrayBufferTypeANIInfo(self.am, t, arraybuffer_attr)
        if fixedarray_attr := FixedArrayAttr.get(t.ty_ref):
            return FixedArrayTypeANIInfo(self.am, t, fixedarray_attr)
        return ArrayTypeANIInfo(self.am, t)

    @override
    def visit_optional_type(self, t: OptionalType) -> TypeANIInfo:
        return OptionalTypeANIInfo(self.am, t)

    @override
    def visit_opaque_type(self, t: OpaqueType) -> TypeANIInfo:
        return OpaqueTypeANIInfo(self.am, t)

    @override
    def visit_map_type(self, t: MapType) -> TypeANIInfo:
        if record_attr := RecordAttr.get(t.ty_ref):
            return RecordTypeANIInfo(self.am, t, record_attr)
        raise NotImplementedError("MapType is not supported in ANI yet.")

    @override
    def visit_set_type(self, t: SetType) -> TypeANIInfo:
        raise NotImplementedError("SetType is not supported in ANI yet.")

    @override
    def visit_callback_type(self, t: CallbackType) -> TypeANIInfo:
        return CallbackTypeANIInfo(self.am, t)