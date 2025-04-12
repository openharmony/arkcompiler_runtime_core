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

from taihe.codegen.mangle import DeclKind, encode
from taihe.semantics.declarations import (
    EnumDecl,
    GlobFuncDecl,
    IfaceDecl,
    IfaceMethodDecl,
    PackageDecl,
    PackageGroup,
    StructDecl,
    UnionDecl,
)
from taihe.semantics.types import (
    BOOL,
    F32,
    F64,
    I8,
    I16,
    I32,
    I64,
    U8,
    U16,
    U32,
    U64,
    ArrayType,
    CallbackType,
    EnumType,
    IfaceType,
    MapType,
    OpaqueType,
    OptionalType,
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
from taihe.utils.outputs import COutputBuffer, OutputManager


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
        self.impl_header = f"{d.parent_pkg.name}.{d.name}.abi.1.h"
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
        self.impl_header = f"{d.parent_pkg.name}.{d.name}.abi.1.h"
        self.mangled_name = encode(segments, DeclKind.TYPE)
        self.as_owner = f"struct {self.mangled_name}"
        self.as_param = f"struct {self.mangled_name} const*"


@dataclass
class AncestorItemInfo:
    iface: IfaceDecl
    ftbl_ptr: str


@dataclass
class UniqueAncestorInfo:
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
        self.copy_func = encode(segments, DeclKind.COPY)
        self.drop_func = encode(segments, DeclKind.DROP)
        self.ftable = encode(segments, DeclKind.FTABLE)
        self.vtable = encode(segments, DeclKind.VTABLE)
        self.iid = encode(segments, DeclKind.IID)
        self.dynamic_cast = encode(segments, DeclKind.DYNAMIC_CAST)
        self.ancestor_list: list[AncestorItemInfo] = []
        self.ancestor_dict: dict[IfaceDecl, UniqueAncestorInfo] = {}
        self.ancestors = [d]
        for extend in d.parents:
            ty = extend.ty_ref.resolved_ty
            extend_abi_info = IfaceABIInfo.get(am, ty.ty_decl)
            self.ancestors.extend(extend_abi_info.ancestors)
        for i, ancestor in enumerate(self.ancestors):
            ftbl_ptr = f"ftbl_ptr_{i}"
            self.ancestor_list.append(
                AncestorItemInfo(
                    iface=ancestor,
                    ftbl_ptr=ftbl_ptr,
                )
            )
            self.ancestor_dict.setdefault(
                ancestor,
                UniqueAncestorInfo(
                    offset=i,
                    static_cast=encode([*segments, str(i)], DeclKind.STATIC_CAST),
                    ftbl_ptr=ftbl_ptr,
                ),
            )


class AbstractTypeABIInfo(metaclass=ABCMeta):
    decl_headers: list[str]
    impl_headers: list[str]
    # type as struct field / union field / return value
    as_owner: str
    # type as parameter
    as_param: str


class EnumTypeABIInfo(AbstractAnalysis[EnumType], AbstractTypeABIInfo):
    def __init__(self, am: AnalysisManager, t: EnumType) -> None:
        super().__init__(am, t)
        enum_abi_info = EnumABIInfo.get(am, t.ty_decl)
        self.decl_headers = []
        self.impl_headers = []
        self.as_owner = enum_abi_info.abi_type
        self.as_param = enum_abi_info.abi_type


class UnionTypeABIInfo(AbstractAnalysis[UnionType], AbstractTypeABIInfo):
    def __init__(self, am: AnalysisManager, t: UnionType):
        super().__init__(am, t)
        union_abi_info = UnionABIInfo.get(am, t.ty_decl)
        self.decl_headers = [union_abi_info.decl_header]
        self.impl_headers = [union_abi_info.impl_header]
        self.as_owner = union_abi_info.as_owner
        self.as_param = union_abi_info.as_param


class StructTypeABIInfo(AbstractAnalysis[StructType], AbstractTypeABIInfo):
    def __init__(self, am: AnalysisManager, t: StructType) -> None:
        super().__init__(am, t)
        struct_abi_info = StructABIInfo.get(am, t.ty_decl)
        self.decl_headers = [struct_abi_info.decl_header]
        self.impl_headers = [struct_abi_info.impl_header]
        self.as_owner = struct_abi_info.as_owner
        self.as_param = struct_abi_info.as_param


class IfaceTypeABIInfo(AbstractAnalysis[IfaceType], AbstractTypeABIInfo):
    def __init__(self, am: AnalysisManager, t: IfaceType) -> None:
        super().__init__(am, t)
        iface_abi_info = IfaceABIInfo.get(am, t.ty_decl)
        self.decl_headers = [iface_abi_info.decl_header]
        self.impl_headers = [iface_abi_info.impl_header]
        self.as_owner = iface_abi_info.as_owner
        self.as_param = iface_abi_info.as_param


class ScalarTypeABIInfo(AbstractAnalysis[ScalarType], AbstractTypeABIInfo):
    def __init__(self, am: AnalysisManager, t: ScalarType):
        super().__init__(am, t)
        res = {
            BOOL: "bool",
            F32: "float",
            F64: "double",
            I8: "int8_t",
            I16: "int16_t",
            I32: "int32_t",
            I64: "int64_t",
            U8: "uint8_t",
            U16: "uint16_t",
            U32: "uint32_t",
            U64: "uint64_t",
        }.get(t)
        if res is None:
            raise ValueError
        self.decl_headers = []
        self.impl_headers = []
        self.as_param = res
        self.as_owner = res


class OpaqueTypeABIInfo(AbstractAnalysis[OpaqueType], AbstractTypeABIInfo):
    def __init__(self, am: AnalysisManager, t: OpaqueType) -> None:
        super().__init__(am, t)
        self.decl_headers = []
        self.impl_headers = []
        self.as_param = "uintptr_t"
        self.as_owner = "uintptr_t"


class StringTypeABIInfo(AbstractAnalysis[StringType], AbstractTypeABIInfo):
    def __init__(self, am: AnalysisManager, t: StringType) -> None:
        super().__init__(am, t)
        self.decl_headers = ["taihe/string.abi.h"]
        self.impl_headers = ["taihe/string.abi.h"]
        self.as_owner = "struct TString"
        self.as_param = "struct TString"


class ArrayTypeABIInfo(AbstractAnalysis[ArrayType], AbstractTypeABIInfo):
    def __init__(self, am: AnalysisManager, t: ArrayType) -> None:
        super().__init__(am, t)
        self.decl_headers = ["taihe/array.abi.h"]
        self.impl_headers = ["taihe/array.abi.h"]
        self.as_owner = "struct TArray"
        self.as_param = "struct TArray"


class OptionalTypeABIInfo(AbstractAnalysis[OptionalType], AbstractTypeABIInfo):
    def __init__(self, am: AnalysisManager, t: OptionalType) -> None:
        super().__init__(am, t)
        self.decl_headers = ["taihe/optional.abi.h"]
        self.impl_headers = ["taihe/optional.abi.h"]
        self.as_owner = "struct TOptional"
        self.as_param = "struct TOptional"


class CallbackTypeABIInfo(AbstractAnalysis[CallbackType], AbstractTypeABIInfo):
    def __init__(self, am: AnalysisManager, t: CallbackType) -> None:
        super().__init__(am, t)
        self.decl_headers = []
        self.impl_headers = []
        self.as_owner = "void*"
        self.as_param = "void*"


class VectorTypeABIInfo(AbstractAnalysis[VectorType], AbstractTypeABIInfo):
    def __init__(self, am: AnalysisManager, t: VectorType) -> None:
        super().__init__(am, t)
        self.decl_headers = []
        self.impl_headers = []
        self.as_owner = "void*"
        self.as_param = "void*"


class MapTypeABIInfo(AbstractAnalysis[MapType], AbstractTypeABIInfo):
    def __init__(self, am: AnalysisManager, t: MapType) -> None:
        super().__init__(am, t)
        self.decl_headers = []
        self.impl_headers = []
        self.as_owner = "void*"
        self.as_param = "void*"


class SetTypeABIInfo(AbstractAnalysis[SetType], AbstractTypeABIInfo):
    def __init__(self, am: AnalysisManager, t: SetType) -> None:
        super().__init__(am, t)
        self.decl_headers = []
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


class ABIHeadersGenerator:
    def __init__(self, tm: OutputManager, am: AnalysisManager):
        self.tm = tm
        self.am = am

    def generate(self, pg: PackageGroup):
        for pkg in pg.packages:
            self.gen_package_file(pkg)

    def gen_package_file(self, pkg: PackageDecl):
        pkg_abi_info = PackageABIInfo.get(self.am, pkg)
        pkg_abi_target = COutputBuffer.create(
            self.tm, f"include/{pkg_abi_info.header}", True
        )
        for struct in pkg.structs:
            struct_abi_info = StructABIInfo.get(self.am, struct)
            self.gen_struct_decl_file(struct, struct_abi_info)
            self.gen_struct_defn_file(struct, struct_abi_info)
            pkg_abi_target.include(struct_abi_info.impl_header)
        for union in pkg.unions:
            union_abi_info = UnionABIInfo.get(self.am, union)
            self.gen_union_decl_file(union, union_abi_info)
            self.gen_union_defn_file(union, union_abi_info)
            pkg_abi_target.include(union_abi_info.impl_header)
        for iface in pkg.interfaces:
            iface_abi_info = IfaceABIInfo.get(self.am, iface)
            self.gen_iface_decl_file(iface, iface_abi_info)
            self.gen_iface_defn_file(iface, iface_abi_info)
            self.gen_iface_impl_file(iface, iface_abi_info)
            pkg_abi_target.include(iface_abi_info.impl_header)
        pkg_abi_target.include("taihe/common.h")
        for func in pkg.functions:
            self.gen_func(func, pkg_abi_target)

    def gen_func(
        self,
        func: GlobFuncDecl,
        pkg_abi_target: COutputBuffer,
    ):
        func_abi_info = GlobFuncABIInfo.get(self.am, func)
        params = []
        for param in func.params:
            type_abi_info = TypeABIInfo.get(self.am, param.ty_ref.resolved_ty)
            pkg_abi_target.include(*type_abi_info.decl_headers)
            params.append(f"{type_abi_info.as_param} {param.name}")
        params_str = ", ".join(params)
        if return_ty_ref := func.return_ty_ref:
            type_abi_info = TypeABIInfo.get(self.am, return_ty_ref.resolved_ty)
            pkg_abi_target.include(*type_abi_info.decl_headers)
            return_ty_name = type_abi_info.as_owner
        else:
            return_ty_name = "void"
        pkg_abi_target.writeln(
            f"TH_EXPORT {return_ty_name} {func_abi_info.mangled_name}({params_str});",
        )

    def gen_struct_decl_file(
        self,
        struct: StructDecl,
        struct_abi_info: StructABIInfo,
    ):
        struct_abi_decl_target = COutputBuffer.create(
            self.tm, f"include/{struct_abi_info.decl_header}", True
        )
        struct_abi_decl_target.writeln(
            f"struct {struct_abi_info.mangled_name};",
        )

    def gen_struct_defn_file(
        self,
        struct: StructDecl,
        struct_abi_info: StructABIInfo,
    ):
        struct_abi_defn_target = COutputBuffer.create(
            self.tm, f"include/{struct_abi_info.impl_header}", True
        )
        struct_abi_defn_target.include("taihe/common.h")
        struct_abi_defn_target.include(struct_abi_info.decl_header)
        self.gen_struct_defn(struct, struct_abi_info, struct_abi_defn_target)

    def gen_struct_defn(
        self,
        struct: StructDecl,
        struct_abi_info: StructABIInfo,
        struct_abi_defn_target: COutputBuffer,
    ):
        struct_abi_defn_target.writeln(
            f"struct {struct_abi_info.mangled_name} {{",
        )
        for field in struct.fields:
            type_abi_info = TypeABIInfo.get(self.am, field.ty_ref.resolved_ty)
            struct_abi_defn_target.include(*type_abi_info.impl_headers)
            struct_abi_defn_target.writeln(
                f"    {type_abi_info.as_owner} {field.name};",
            )
        struct_abi_defn_target.writeln(
            f"}};",
        )

    def gen_union_decl_file(
        self,
        union: UnionDecl,
        union_abi_info: UnionABIInfo,
    ):
        union_abi_decl_target = COutputBuffer.create(
            self.tm, f"include/{union_abi_info.decl_header}", True
        )
        union_abi_decl_target.writeln(
            f"struct {union_abi_info.mangled_name};",
        )

    def gen_union_defn_file(
        self,
        union: UnionDecl,
        union_abi_info: UnionABIInfo,
    ):
        union_abi_defn_target = COutputBuffer.create(
            self.tm, f"include/{union_abi_info.impl_header}", True
        )
        union_abi_defn_target.include("taihe/common.h")
        union_abi_defn_target.include(union_abi_info.decl_header)
        self.gen_union_defn(union, union_abi_info, union_abi_defn_target)

    def gen_union_defn(
        self,
        union: UnionDecl,
        union_abi_info: UnionABIInfo,
        union_abi_defn_target: COutputBuffer,
    ):
        union_abi_defn_target.writeln(
            f"union {union_abi_info.union_name} {{",
        )
        for field in union.fields:
            if field.ty_ref is None:
                union_abi_defn_target.writeln(
                    f"    // {field.name}",
                )
                continue
            type_abi_info = TypeABIInfo.get(self.am, field.ty_ref.resolved_ty)
            union_abi_defn_target.include(*type_abi_info.impl_headers)
            union_abi_defn_target.writeln(
                f"    {type_abi_info.as_owner} {field.name};",
            )
        union_abi_defn_target.writeln(
            f"}};",
            f"struct {union_abi_info.mangled_name} {{",
            f"    {union_abi_info.tag_type} m_tag;",
            f"    union {union_abi_info.union_name} m_data;",
            f"}};",
        )

    def gen_iface_decl_file(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
    ):
        iface_abi_decl_target = COutputBuffer.create(
            self.tm, f"include/{iface_abi_info.decl_header}", True
        )
        iface_abi_decl_target.writeln(
            f"struct {iface_abi_info.mangled_name};",
        )

    def gen_iface_defn_file(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
    ):
        iface_abi_defn_target = COutputBuffer.create(
            self.tm, f"include/{iface_abi_info.defn_header}", True
        )
        iface_abi_defn_target.include("taihe/object.abi.h")
        iface_abi_defn_target.include(iface_abi_info.decl_header)
        iface_abi_defn_target.writeln(
            f"TH_EXPORT void const* const {iface_abi_info.iid};",
        )
        self.gen_iface_ftable(iface, iface_abi_info, iface_abi_defn_target)
        self.gen_iface_vtable(iface, iface_abi_info, iface_abi_defn_target)
        self.gen_iface_defn(iface, iface_abi_info, iface_abi_defn_target)
        self.gen_iface_static_cast_funcs(iface, iface_abi_info, iface_abi_defn_target)
        self.gen_iface_dynamic_cast_func(iface, iface_abi_info, iface_abi_defn_target)
        self.gen_iface_copy_func(iface, iface_abi_info, iface_abi_defn_target)
        self.gen_iface_drop_func(iface, iface_abi_info, iface_abi_defn_target)

    def gen_iface_ftable(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_abi_defn_target: COutputBuffer,
    ):
        iface_abi_defn_target.writeln(
            f"struct {iface_abi_info.ftable} {{",
        )
        for method in iface.methods:
            params = [f"{iface_abi_info.as_param} tobj"]
            for param in method.params:
                type_abi_info = TypeABIInfo.get(self.am, param.ty_ref.resolved_ty)
                iface_abi_defn_target.include(*type_abi_info.decl_headers)
                params.append(f"{type_abi_info.as_param} {param.name}")
            params_str = ", ".join(params)
            if return_ty_ref := method.return_ty_ref:
                type_abi_info = TypeABIInfo.get(self.am, return_ty_ref.resolved_ty)
                iface_abi_defn_target.include(*type_abi_info.decl_headers)
                return_ty_name = type_abi_info.as_owner
            else:
                return_ty_name = "void"
            iface_abi_defn_target.writeln(
                f"    {return_ty_name} (*{method.name})({params_str});",
            )
        iface_abi_defn_target.writeln(
            f"}};",
        )

    def gen_iface_vtable(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_abi_defn_target: COutputBuffer,
    ):
        iface_abi_defn_target.writeln(
            f"struct {iface_abi_info.vtable} {{",
        )
        for ancestor_item_info in iface_abi_info.ancestor_list:
            ancestor_abi_info = IfaceABIInfo.get(self.am, ancestor_item_info.iface)
            iface_abi_defn_target.writeln(
                f"    struct {ancestor_abi_info.ftable} const* {ancestor_item_info.ftbl_ptr};",
            )
        iface_abi_defn_target.writeln(
            f"}};",
        )

    def gen_iface_defn(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_abi_defn_target: COutputBuffer,
    ):
        iface_abi_defn_target.writeln(
            f"struct {iface_abi_info.mangled_name} {{",
            f"    struct {iface_abi_info.vtable} const* vtbl_ptr;",
            f"    struct DataBlockHead* data_ptr;",
            f"}};",
        )

    def gen_iface_static_cast_funcs(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_abi_defn_target: COutputBuffer,
    ):
        for ancestor, info in iface_abi_info.ancestor_dict.items():
            if ancestor is iface:
                continue
            ancestor_abi_info = IfaceABIInfo.get(self.am, ancestor)
            iface_abi_defn_target.include(ancestor_abi_info.defn_header)
            iface_abi_defn_target.writeln(
                f"TH_INLINE struct {ancestor_abi_info.mangled_name} {info.static_cast}(struct {iface_abi_info.mangled_name} tobj) {{",
                f"    struct {ancestor_abi_info.mangled_name} result;",
                f"    result.vtbl_ptr = (struct {ancestor_abi_info.vtable} const*)((void* const*)tobj.vtbl_ptr + {info.offset});",
                f"    result.data_ptr = tobj.data_ptr;",
                f"    return result;",
                f"}}",
            )

    def gen_iface_dynamic_cast_func(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_abi_defn_target: COutputBuffer,
    ):
        iface_abi_defn_target.writeln(
            f"TH_INLINE struct {iface_abi_info.mangled_name} {iface_abi_info.dynamic_cast}(struct DataBlockHead* data_ptr) {{",
            f"    struct TypeInfo const* rtti_ptr = data_ptr->rtti_ptr;",
            f"    struct {iface_abi_info.mangled_name} result;",
            f"    result.data_ptr = data_ptr;"
            f"    for (size_t i = 0; i < rtti_ptr->len; i++) {{",
            f"        if (rtti_ptr->idmap[i].id == {iface_abi_info.iid}) {{",
            f"            result.vtbl_ptr = (struct {iface_abi_info.vtable}*)rtti_ptr->idmap[i].vtbl_ptr;",
            f"            return result;",
            f"        }}",
            f"    }}",
            f"    result.vtbl_ptr = NULL;",
            f"    return result;",
            f"}}",
        )

    def gen_iface_copy_func(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_abi_defn_target: COutputBuffer,
    ):
        iface_abi_defn_target.writeln(
            f"TH_INLINE struct {iface_abi_info.mangled_name} {iface_abi_info.copy_func}(struct {iface_abi_info.mangled_name} tobj) {{",
            f"    struct DataBlockHead* data_ptr = tobj.data_ptr;",
            f"    if (data_ptr) {{",
            f"        tref_inc(&data_ptr->m_count);",
            f"    }}",
            f"    return tobj;",
            f"}}",
        )

    def gen_iface_drop_func(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_abi_defn_target: COutputBuffer,
    ):
        iface_abi_defn_target.writeln(
            f"TH_INLINE void {iface_abi_info.drop_func}(struct {iface_abi_info.mangled_name} tobj) {{",
            f"    struct DataBlockHead* data_ptr = tobj.data_ptr;",
            f"    if (data_ptr && tref_dec(&data_ptr->m_count)) {{",
            f"        data_ptr->rtti_ptr->free(data_ptr);",
            f"    }}",
            f"}}",
        )

    def gen_iface_impl_file(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
    ):
        iface_abi_impl_target = COutputBuffer.create(
            self.tm, f"include/{iface_abi_info.impl_header}", True
        )
        iface_abi_impl_target.include(iface_abi_info.defn_header)
        self.gen_iface_methods(iface, iface_abi_info, iface_abi_impl_target)

    def gen_iface_methods(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_abi_impl_target: COutputBuffer,
    ):
        for method in iface.methods:
            method_abi_info = IfaceMethodABIInfo.get(self.am, method)
            params = [f"{iface_abi_info.as_param} tobj"]
            args = ["tobj"]
            for param in method.params:
                type_abi_info = TypeABIInfo.get(self.am, param.ty_ref.resolved_ty)
                iface_abi_impl_target.include(*type_abi_info.impl_headers)
                params.append(f"{type_abi_info.as_param} {param.name}")
                args.append(param.name)
            params_str = ", ".join(params)
            args_str = ", ".join(args)
            if return_ty_ref := method.return_ty_ref:
                type_abi_info = TypeABIInfo.get(self.am, return_ty_ref.resolved_ty)
                iface_abi_impl_target.include(*type_abi_info.impl_headers)
                return_ty_name = type_abi_info.as_owner
            else:
                return_ty_name = "void"
            iface_abi_impl_target.writeln(
                f"TH_INLINE {return_ty_name} {method_abi_info.mangled_name}({params_str}) {{",
                f"    return tobj.vtbl_ptr->{iface_abi_info.ancestor_dict[iface].ftbl_ptr}->{method.name}({args_str});",
                f"}}",
            )


class ABISourcesGenerator:
    def __init__(self, tm: OutputManager, am: AnalysisManager):
        self.tm = tm
        self.am = am

    def generate(self, pg: PackageGroup):
        for pkg in pg.packages:
            self.gen_package_file(pkg)

    def gen_package_file(self, pkg: PackageDecl):
        pkg_abi_info = PackageABIInfo.get(self.am, pkg)
        pkg_abi_src_target = COutputBuffer.create(
            self.tm, f"src/{pkg_abi_info.src}", False
        )
        for iface in pkg.interfaces:
            self.gen_iface_file(iface, pkg_abi_src_target)

    def gen_iface_file(
        self,
        iface: IfaceDecl,
        pkg_abi_src_target: COutputBuffer,
    ):
        iface_abi_info = IfaceABIInfo.get(self.am, iface)
        pkg_abi_src_target.include(iface_abi_info.defn_header)
        pkg_abi_src_target.writeln(
            f"void const* const {iface_abi_info.iid} = &{iface_abi_info.iid};",
        )
