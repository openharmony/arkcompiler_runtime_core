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
        super().__init__(am, p)
        self.header = f"{p.name}.abi.h"
        self.src = f"{p.name}.abi.c"


class GlobFuncABIInfo(AbstractAnalysis[GlobFuncDecl]):
    def __init__(self, am: AnalysisManager, f: GlobFuncDecl) -> None:
        super().__init__(am, f)
        segments = [*f.parent_pkg.segments, f.name]
        self.mangled_name = encode(segments, DeclKind.FUNC)


class IfaceMethodABIInfo(AbstractAnalysis[IfaceMethodDecl]):
    def __init__(self, am: AnalysisManager, f: IfaceMethodDecl) -> None:
        super().__init__(am, f)
        segments = [*f.parent_pkg.segments, f.parent_iface.name, f.name]
        self.mangled_name = encode(segments, DeclKind.FUNC)


class EnumABIInfo(AbstractAnalysis[EnumDecl]):
    def __init__(self, am: AnalysisManager, d: EnumDecl) -> None:
        super().__init__(am, d)
        self.abi_type = "int"


class UnionABIInfo(AbstractAnalysis[UnionDecl]):
    def __init__(self, am: AnalysisManager, d: UnionDecl) -> None:
        super().__init__(am, d)
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


class StructABIInfo(AbstractAnalysis[StructDecl]):
    def __init__(self, am: AnalysisManager, d: StructDecl) -> None:
        super().__init__(am, d)
        segments = [*d.parent_pkg.segments, d.name]
        self.decl_header = f"{d.parent_pkg.name}.{d.name}.abi.0.h"
        self.defn_header = f"{d.parent_pkg.name}.{d.name}.abi.1.h"
        self.impl_header = f"{d.parent_pkg.name}.{d.name}.abi.2.h"
        self.mangled_name = encode(segments, DeclKind.TYPE)
        self.as_owner = f"struct {self.mangled_name}"
        self.as_param = f"struct {self.mangled_name} const*"


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
        super().__init__(am, d)
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


class AbstractTypeABIInfo(metaclass=ABCMeta):
    defn_headers: list[str]
    impl_headers: list[str]
    # type as struct field / union field / return value
    as_owner: str
    # type as parameter
    as_param: str


class EnumTypeABIInfo(AbstractAnalysis[EnumType], AbstractTypeABIInfo):
    def __init__(self, am: AnalysisManager, t: EnumType) -> None:
        super().__init__(am, t)
        enum_abi_info = EnumABIInfo.get(am, t.ty_decl)
        self.defn_headers = []
        self.impl_headers = []
        self.as_owner = enum_abi_info.abi_type
        self.as_param = enum_abi_info.abi_type


class UnionTypeABIInfo(AbstractAnalysis[UnionType], AbstractTypeABIInfo):
    def __init__(self, am: AnalysisManager, t: UnionType):
        super().__init__(am, t)
        union_abi_info = UnionABIInfo.get(am, t.ty_decl)
        self.defn_headers = [union_abi_info.defn_header]
        self.impl_headers = [union_abi_info.impl_header]
        self.as_owner = union_abi_info.as_owner
        self.as_param = union_abi_info.as_param


class StructTypeABIInfo(AbstractAnalysis[StructType], AbstractTypeABIInfo):
    def __init__(self, am: AnalysisManager, t: StructType) -> None:
        super().__init__(am, t)
        struct_abi_info = StructABIInfo.get(am, t.ty_decl)
        self.defn_headers = [struct_abi_info.defn_header]
        self.impl_headers = [struct_abi_info.impl_header]
        self.as_owner = struct_abi_info.as_owner
        self.as_param = struct_abi_info.as_param


class IfaceTypeABIInfo(AbstractAnalysis[IfaceType], AbstractTypeABIInfo):
    def __init__(self, am: AnalysisManager, t: IfaceType) -> None:
        super().__init__(am, t)
        iface_abi_info = IfaceABIInfo.get(am, t.ty_decl)
        self.defn_headers = [iface_abi_info.defn_header]
        self.impl_headers = [iface_abi_info.impl_header]
        self.as_owner = iface_abi_info.as_owner
        self.as_param = iface_abi_info.as_param


class ScalarTypeABIInfo(AbstractAnalysis[ScalarType], AbstractTypeABIInfo):
    def __init__(self, am: AnalysisManager, t: ScalarType):
        super().__init__(am, t)
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


class OpaqueTypeABIInfo(AbstractAnalysis[OpaqueType], AbstractTypeABIInfo):
    def __init__(self, am: AnalysisManager, t: OpaqueType) -> None:
        super().__init__(am, t)
        self.defn_headers = []
        self.impl_headers = []
        self.as_param = "uintptr_t"
        self.as_owner = "uintptr_t"


class StringTypeABIInfo(AbstractAnalysis[StringType], AbstractTypeABIInfo):
    def __init__(self, am: AnalysisManager, t: StringType) -> None:
        super().__init__(am, t)
        self.defn_headers = ["taihe/string.abi.h"]
        self.impl_headers = ["taihe/string.abi.h"]
        self.as_owner = "struct TString"
        self.as_param = "struct TString"


class ArrayTypeABIInfo(AbstractAnalysis[ArrayType], AbstractTypeABIInfo):
    def __init__(self, am: AnalysisManager, t: ArrayType) -> None:
        super().__init__(am, t)
        self.defn_headers = ["taihe/array.abi.h"]
        self.impl_headers = ["taihe/array.abi.h"]
        self.as_owner = "struct TArray"
        self.as_param = "struct TArray"


class OptionalTypeABIInfo(AbstractAnalysis[OptionalType], AbstractTypeABIInfo):
    def __init__(self, am: AnalysisManager, t: OptionalType) -> None:
        super().__init__(am, t)
        self.defn_headers = ["taihe/optional.abi.h"]
        self.impl_headers = ["taihe/optional.abi.h"]
        self.as_owner = "struct TOptional"
        self.as_param = "struct TOptional"


class CallbackTypeABIInfo(AbstractAnalysis[CallbackType], AbstractTypeABIInfo):
    def __init__(self, am: AnalysisManager, t: CallbackType) -> None:
        super().__init__(am, t)
        self.defn_headers = ["taihe/callback.abi.h"]
        self.impl_headers = ["taihe/callback.abi.h"]
        self.as_owner = "struct TCallback"
        self.as_param = "struct TCallback"


class VectorTypeABIInfo(AbstractAnalysis[VectorType], AbstractTypeABIInfo):
    def __init__(self, am: AnalysisManager, t: VectorType) -> None:
        super().__init__(am, t)
        self.defn_headers = []
        self.impl_headers = []
        self.as_owner = "void*"
        self.as_param = "void*"


class MapTypeABIInfo(AbstractAnalysis[MapType], AbstractTypeABIInfo):
    def __init__(self, am: AnalysisManager, t: MapType) -> None:
        super().__init__(am, t)
        self.defn_headers = []
        self.impl_headers = []
        self.as_owner = "void*"
        self.as_param = "void*"


class SetTypeABIInfo(AbstractAnalysis[SetType], AbstractTypeABIInfo):
    def __init__(self, am: AnalysisManager, t: SetType) -> None:
        super().__init__(am, t)
        self.defn_headers = []
        self.impl_headers = []
        self.as_owner = "void*"
        self.as_param = "void*"


class TypeABIInfo(TypeVisitor[AbstractTypeABIInfo]):
    def __init__(self, am: AnalysisManager):
        self.am = am

    @staticmethod
    def get(am: AnalysisManager, t: Type):
        return TypeABIInfo(am).handle_type(t)

    @override
    def visit_enum_type(self, t: EnumType) -> AbstractTypeABIInfo:
        return EnumTypeABIInfo.get(self.am, t)

    @override
    def visit_union_type(self, t: UnionType) -> AbstractTypeABIInfo:
        return UnionTypeABIInfo.get(self.am, t)

    @override
    def visit_struct_type(self, t: StructType) -> AbstractTypeABIInfo:
        return StructTypeABIInfo.get(self.am, t)

    @override
    def visit_iface_type(self, t: IfaceType) -> AbstractTypeABIInfo:
        return IfaceTypeABIInfo.get(self.am, t)

    @override
    def visit_scalar_type(self, t: ScalarType) -> AbstractTypeABIInfo:
        return ScalarTypeABIInfo.get(self.am, t)

    @override
    def visit_opaque_type(self, t: OpaqueType) -> AbstractTypeABIInfo:
        return OpaqueTypeABIInfo.get(self.am, t)

    @override
    def visit_string_type(self, t: StringType) -> AbstractTypeABIInfo:
        return StringTypeABIInfo.get(self.am, t)

    @override
    def visit_array_type(self, t: ArrayType) -> AbstractTypeABIInfo:
        return ArrayTypeABIInfo.get(self.am, t)

    @override
    def visit_vector_type(self, t: VectorType) -> AbstractTypeABIInfo:
        return VectorTypeABIInfo.get(self.am, t)

    @override
    def visit_optional_type(self, t: OptionalType) -> AbstractTypeABIInfo:
        return OptionalTypeABIInfo.get(self.am, t)

    @override
    def visit_map_type(self, t: MapType) -> AbstractTypeABIInfo:
        return MapTypeABIInfo.get(self.am, t)

    @override
    def visit_set_type(self, t: SetType) -> AbstractTypeABIInfo:
        return SetTypeABIInfo.get(self.am, t)

    @override
    def visit_callback_type(self, t: CallbackType) -> AbstractTypeABIInfo:
        return CallbackTypeABIInfo.get(self.am, t)