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

from abc import ABCMeta
from dataclasses import dataclass

from typing_extensions import override

from taihe.codegen.abi.mangle import DeclKind, encode
from taihe.semantics.declarations import (
    EnumDecl,
    GlobFuncDecl,
    IfaceDecl,
    IfaceMethodDecl,
    PackageDecl,
    StructDecl,
    UnionDecl,
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
    VectorType,
)
from taihe.semantics.visitor import TypeVisitor
from taihe.utils.analyses import AbstractAnalysis, AnalysisManager


class PackageABIInfo(AbstractAnalysis[PackageDecl]):
    def __init__(self, am: AnalysisManager, p: PackageDecl) -> None:
        self.header = f"{p.name}.abi.h"
        self.source = f"{p.name}.abi.c"

    @classmethod
    @override
    def create(cls, am: AnalysisManager, p: PackageDecl) -> "PackageABIInfo":
        return PackageABIInfo(am, p)


class GlobFuncABIInfo(AbstractAnalysis[GlobFuncDecl]):
    def __init__(self, am: AnalysisManager, f: GlobFuncDecl) -> None:
        segments = [*f.parent_pkg.segments, f.name]
        self.mangled_name = encode(segments, DeclKind.FUNC)

    @classmethod
    @override
    def create(cls, am: AnalysisManager, f: GlobFuncDecl) -> "GlobFuncABIInfo":
        return GlobFuncABIInfo(am, f)


class IfaceMethodABIInfo(AbstractAnalysis[IfaceMethodDecl]):
    def __init__(self, am: AnalysisManager, f: IfaceMethodDecl) -> None:
        segments = [*f.parent_pkg.segments, f.parent_iface.name, f.name]
        self.mangled_name = encode(segments, DeclKind.FUNC)

    @classmethod
    @override
    def create(cls, am: AnalysisManager, f: IfaceMethodDecl) -> "IfaceMethodABIInfo":
        return IfaceMethodABIInfo(am, f)


class EnumABIInfo(AbstractAnalysis[EnumDecl]):
    def __init__(self, am: AnalysisManager, d: EnumDecl) -> None:
        self.abi_type = "int"

    @classmethod
    @override
    def create(cls, am: AnalysisManager, d: EnumDecl) -> "EnumABIInfo":
        return EnumABIInfo(am, d)


class UnionABIInfo(AbstractAnalysis[UnionDecl]):
    def __init__(self, am: AnalysisManager, d: UnionDecl) -> None:
        segments = [*d.parent_pkg.segments, d.name]
        self.decl_header = f"{d.parent_pkg.name}.{d.name}.abi.0.h"
        self.defn_header = f"{d.parent_pkg.name}.{d.name}.abi.1.h"
        self.impl_header = f"{d.parent_pkg.name}.{d.name}.abi.2.h"
        self.tag_type = "int"
        self.union_name = encode(segments, DeclKind.UNION)
        self.mangled_name = encode(segments, DeclKind.TYPE)
        self.as_owner = f"struct {self.mangled_name}"
        self.as_param = f"struct {self.mangled_name} const*"
        self.has_data = any(field.ty_ref for field in d.fields)

    @classmethod
    @override
    def create(cls, am: AnalysisManager, d: UnionDecl) -> "UnionABIInfo":
        return UnionABIInfo(am, d)


class StructABIInfo(AbstractAnalysis[StructDecl]):
    def __init__(self, am: AnalysisManager, d: StructDecl) -> None:
        segments = [*d.parent_pkg.segments, d.name]
        self.decl_header = f"{d.parent_pkg.name}.{d.name}.abi.0.h"
        self.defn_header = f"{d.parent_pkg.name}.{d.name}.abi.1.h"
        self.impl_header = f"{d.parent_pkg.name}.{d.name}.abi.2.h"
        self.mangled_name = encode(segments, DeclKind.TYPE)
        self.as_owner = f"struct {self.mangled_name}"
        self.as_param = f"struct {self.mangled_name} const*"

    @classmethod
    @override
    def create(cls, am: AnalysisManager, d: StructDecl) -> "StructABIInfo":
        return StructABIInfo(am, d)


@dataclass
class AncestorListItemInfo:
    iface: IfaceDecl
    ftbl_ptr: str


@dataclass
class AncestorDictItemInfo:
    offset: int
    static_cast: str
    ftbl_ptr: str


class IfaceABIInfo(AbstractAnalysis[IfaceDecl]):
    def __init__(self, am: AnalysisManager, d: IfaceDecl) -> None:
        segments = [*d.parent_pkg.segments, d.name]
        self.decl_header = f"{d.parent_pkg.name}.{d.name}.abi.0.h"
        self.defn_header = f"{d.parent_pkg.name}.{d.name}.abi.1.h"
        self.impl_header = f"{d.parent_pkg.name}.{d.name}.abi.2.h"
        self.mangled_name = encode(segments, DeclKind.TYPE)
        self.as_owner = f"struct {self.mangled_name}"
        self.as_param = f"struct {self.mangled_name}"
        self.ftable = encode(segments, DeclKind.FTABLE)
        self.vtable = encode(segments, DeclKind.VTABLE)
        self.iid = encode(segments, DeclKind.IID)
        self.dynamic_cast = encode(segments, DeclKind.DYNAMIC_CAST)
        self.ancestor_list: list[AncestorListItemInfo] = []
        self.ancestor_dict: dict[IfaceDecl, AncestorDictItemInfo] = {}
        self.ancestors = [d]
        for extend in d.parents:
            ty = extend.ty_ref.resolved_ty
            pass
            extend_abi_info = IfaceABIInfo.get(am, ty.ty_decl)
            self.ancestors.extend(extend_abi_info.ancestors)
        for i, ancestor in enumerate(self.ancestors):
            ftbl_ptr = f"ftbl_ptr_{i}"
            self.ancestor_list.append(
                AncestorListItemInfo(
                    iface=ancestor,
                    ftbl_ptr=ftbl_ptr,
                )
            )
            self.ancestor_dict.setdefault(
                ancestor,
                AncestorDictItemInfo(
                    offset=i,
                    static_cast=encode([*segments, str(i)], DeclKind.STATIC_CAST),
                    ftbl_ptr=ftbl_ptr,
                ),
            )

    @classmethod
    @override
    def create(cls, am: AnalysisManager, d: IfaceDecl) -> "IfaceABIInfo":
        return IfaceABIInfo(am, d)


class TypeABIInfo(AbstractAnalysis[Type], metaclass=ABCMeta):
    defn_headers: list[str]
    impl_headers: list[str]
    # type as struct field / union field / return value
    as_owner: str
    # type as parameter
    as_param: str

    @classmethod
    @override
    def create(cls, am: AnalysisManager, t: Type) -> "TypeABIInfo":
        return TypeABIInfoDispatcher(am).handle_type(t)


class EnumTypeABIInfo(TypeABIInfo):
    def __init__(self, am: AnalysisManager, t: EnumType) -> None:
        enum_abi_info = EnumABIInfo.get(am, t.ty_decl)
        self.defn_headers = []
        self.impl_headers = []
        self.as_owner = enum_abi_info.abi_type
        self.as_param = enum_abi_info.abi_type


class UnionTypeABIInfo(TypeABIInfo):
    def __init__(self, am: AnalysisManager, t: UnionType):
        union_abi_info = UnionABIInfo.get(am, t.ty_decl)
        self.defn_headers = [union_abi_info.defn_header]
        self.impl_headers = [union_abi_info.impl_header]
        self.as_owner = union_abi_info.as_owner
        self.as_param = union_abi_info.as_param


class StructTypeABIInfo(TypeABIInfo):
    def __init__(self, am: AnalysisManager, t: StructType) -> None:
        struct_abi_info = StructABIInfo.get(am, t.ty_decl)
        self.defn_headers = [struct_abi_info.defn_header]
        self.impl_headers = [struct_abi_info.impl_header]
        self.as_owner = struct_abi_info.as_owner
        self.as_param = struct_abi_info.as_param


class IfaceTypeABIInfo(TypeABIInfo):
    def __init__(self, am: AnalysisManager, t: IfaceType) -> None:
        iface_abi_info = IfaceABIInfo.get(am, t.ty_decl)
        self.defn_headers = [iface_abi_info.defn_header]
        self.impl_headers = [iface_abi_info.impl_header]
        self.as_owner = iface_abi_info.as_owner
        self.as_param = iface_abi_info.as_param


class ScalarTypeABIInfo(TypeABIInfo):
    def __init__(self, am: AnalysisManager, t: ScalarType):
        res = {
            ScalarKind.BOOL: "bool",
            ScalarKind.F32: "float",
            ScalarKind.F64: "double",
            ScalarKind.I8: "int8_t",
            ScalarKind.I16: "int16_t",
            ScalarKind.I32: "int32_t",
            ScalarKind.I64: "int64_t",
            ScalarKind.U8: "uint8_t",
            ScalarKind.U16: "uint16_t",
            ScalarKind.U32: "uint32_t",
            ScalarKind.U64: "uint64_t",
        }.get(t.kind)
        if res is None:
            raise ValueError
        self.defn_headers = []
        self.impl_headers = []
        self.as_param = res
        self.as_owner = res


class OpaqueTypeABIInfo(TypeABIInfo):
    def __init__(self, am: AnalysisManager, t: OpaqueType) -> None:
        self.defn_headers = []
        self.impl_headers = []
        self.as_param = "uintptr_t"
        self.as_owner = "uintptr_t"


class StringTypeABIInfo(TypeABIInfo):
    def __init__(self, am: AnalysisManager, t: StringType) -> None:
        self.defn_headers = ["taihe/string.abi.h"]
        self.impl_headers = ["taihe/string.abi.h"]
        self.as_owner = "struct TString"
        self.as_param = "struct TString"


class ArrayTypeABIInfo(TypeABIInfo):
    def __init__(self, am: AnalysisManager, t: ArrayType) -> None:
        self.defn_headers = ["taihe/array.abi.h"]
        self.impl_headers = ["taihe/array.abi.h"]
        self.as_owner = "struct TArray"
        self.as_param = "struct TArray"


class OptionalTypeABIInfo(TypeABIInfo):
    def __init__(self, am: AnalysisManager, t: OptionalType) -> None:
        self.defn_headers = ["taihe/optional.abi.h"]
        self.impl_headers = ["taihe/optional.abi.h"]
        self.as_owner = "struct TOptional"
        self.as_param = "struct TOptional"


class CallbackTypeABIInfo(TypeABIInfo):
    def __init__(self, am: AnalysisManager, t: CallbackType) -> None:
        self.defn_headers = ["taihe/callback.abi.h"]
        self.impl_headers = ["taihe/callback.abi.h"]
        self.as_owner = "struct TCallback"
        self.as_param = "struct TCallback"


class VectorTypeABIInfo(TypeABIInfo):
    def __init__(self, am: AnalysisManager, t: VectorType) -> None:
        self.defn_headers = []
        self.impl_headers = []
        self.as_owner = "void*"
        self.as_param = "void*"


class MapTypeABIInfo(TypeABIInfo):
    def __init__(self, am: AnalysisManager, t: MapType) -> None:
        self.defn_headers = []
        self.impl_headers = []
        self.as_owner = "void*"
        self.as_param = "void*"


class SetTypeABIInfo(TypeABIInfo):
    def __init__(self, am: AnalysisManager, t: SetType) -> None:
        self.defn_headers = []
        self.impl_headers = []
        self.as_owner = "void*"
        self.as_param = "void*"


class TypeABIInfoDispatcher(TypeVisitor[TypeABIInfo]):
    def __init__(self, am: AnalysisManager):
        self.am = am

    @override
    def visit_enum_type(self, t: EnumType) -> TypeABIInfo:
        return EnumTypeABIInfo(self.am, t)

    @override
    def visit_union_type(self, t: UnionType) -> TypeABIInfo:
        return UnionTypeABIInfo(self.am, t)

    @override
    def visit_struct_type(self, t: StructType) -> TypeABIInfo:
        return StructTypeABIInfo(self.am, t)

    @override
    def visit_iface_type(self, t: IfaceType) -> TypeABIInfo:
        return IfaceTypeABIInfo(self.am, t)

    @override
    def visit_scalar_type(self, t: ScalarType) -> TypeABIInfo:
        return ScalarTypeABIInfo(self.am, t)

    @override
    def visit_opaque_type(self, t: OpaqueType) -> TypeABIInfo:
        return OpaqueTypeABIInfo(self.am, t)

    @override
    def visit_string_type(self, t: StringType) -> TypeABIInfo:
        return StringTypeABIInfo(self.am, t)

    @override
    def visit_array_type(self, t: ArrayType) -> TypeABIInfo:
        return ArrayTypeABIInfo(self.am, t)

    @override
    def visit_vector_type(self, t: VectorType) -> TypeABIInfo:
        return VectorTypeABIInfo(self.am, t)

    @override
    def visit_optional_type(self, t: OptionalType) -> TypeABIInfo:
        return OptionalTypeABIInfo(self.am, t)

    @override
    def visit_map_type(self, t: MapType) -> TypeABIInfo:
        return MapTypeABIInfo(self.am, t)

    @override
    def visit_set_type(self, t: SetType) -> TypeABIInfo:
        return SetTypeABIInfo(self.am, t)

    @override
    def visit_callback_type(self, t: CallbackType) -> TypeABIInfo:
        return CallbackTypeABIInfo(self.am, t)


class PackageCImplInfo(AbstractAnalysis[PackageDecl]):
    def __init__(self, am: AnalysisManager, p: PackageDecl) -> None:
        self.header = f"{p.name}.impl.h"
        self.source = f"{p.name}.impl.c"

    @classmethod
    @override
    def create(cls, am: AnalysisManager, p: PackageDecl) -> "PackageCImplInfo":
        return PackageCImplInfo(am, p)


class GlobFuncCImplInfo(AbstractAnalysis[GlobFuncDecl]):
    def __init__(self, am: AnalysisManager, f: GlobFuncDecl) -> None:
        self.macro = f"TH_EXPORT_C_API_{f.name}"

    @classmethod
    @override
    def create(cls, am: AnalysisManager, f: GlobFuncDecl) -> "GlobFuncCImplInfo":
        return GlobFuncCImplInfo(am, f)