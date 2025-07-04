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

from typing_extensions import override

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


class PackageCppInfo(AbstractAnalysis[PackageDecl]):
    def __init__(self, am: AnalysisManager, p: PackageDecl) -> None:
        super().__init__(am, p)
        self.header = f"{p.name}.proj.hpp"


class IfaceMethodCppInfo(AbstractAnalysis[IfaceMethodDecl]):
    def __init__(self, am: AnalysisManager, f: IfaceMethodDecl) -> None:
        super().__init__(am, f)
        self.call_name = f.name
        self.impl_name = f.name


class EnumCppInfo(AbstractAnalysis[EnumDecl]):
    def __init__(self, am: AnalysisManager, d: EnumDecl) -> None:
        super().__init__(am, d)
        self.decl_header = f"{d.parent_pkg.name}.{d.name}.proj.0.hpp"
        self.defn_header = f"{d.parent_pkg.name}.{d.name}.proj.1.hpp"

        self.namespace = "::".join(d.parent_pkg.segments)
        self.name = d.name
        self.full_name = "::" + self.namespace + "::" + self.name

        self.as_owner = self.full_name
        self.as_param = self.full_name


class StructCppInfo(AbstractAnalysis[StructDecl]):
    def __init__(self, am: AnalysisManager, d: StructDecl) -> None:
        super().__init__(am, d)
        self.decl_header = f"{d.parent_pkg.name}.{d.name}.proj.0.hpp"
        self.defn_header = f"{d.parent_pkg.name}.{d.name}.proj.1.hpp"
        self.impl_header = f"{d.parent_pkg.name}.{d.name}.proj.2.hpp"

        self.namespace = "::".join(d.parent_pkg.segments)
        self.name = d.name
        self.full_name = "::" + self.namespace + "::" + self.name

        self.as_owner = self.full_name
        self.as_param = self.full_name + " const&"


class UnionCppInfo(AbstractAnalysis[UnionDecl]):
    def __init__(self, am: AnalysisManager, d: UnionDecl) -> None:
        super().__init__(am, d)
        self.decl_header = f"{d.parent_pkg.name}.{d.name}.proj.0.hpp"
        self.defn_header = f"{d.parent_pkg.name}.{d.name}.proj.1.hpp"
        self.impl_header = f"{d.parent_pkg.name}.{d.name}.proj.2.hpp"

        self.namespace = "::".join(d.parent_pkg.segments)
        self.name = d.name
        self.full_name = "::" + self.namespace + "::" + self.name

        self.as_owner = self.full_name
        self.as_param = self.full_name + " const&"


class IfaceCppInfo(AbstractAnalysis[IfaceDecl]):
    def __init__(self, am: AnalysisManager, d: IfaceDecl) -> None:
        super().__init__(am, d)
        self.decl_header = f"{d.parent_pkg.name}.{d.name}.proj.0.hpp"
        self.defn_header = f"{d.parent_pkg.name}.{d.name}.proj.1.hpp"
        self.impl_header = f"{d.parent_pkg.name}.{d.name}.proj.2.hpp"

        self.namespace = "::".join(d.parent_pkg.segments)
        self.norm_name = d.name
        self.full_norm_name = "::" + self.namespace + "::" + self.norm_name

        self.weakspace = "::".join(d.parent_pkg.segments) + "::weak"
        self.weak_name = d.name
        self.full_weak_name = "::" + self.weakspace + "::" + self.weak_name

        self.as_owner = self.full_norm_name
        self.as_param = self.full_weak_name


class AbstractTypeCppInfo(metaclass=ABCMeta):
    decl_headers: list[str]
    defn_headers: list[str]
    impl_headers: list[str]
    as_owner: str
    as_param: str

    def return_from_abi(self, val):
        return f"::taihe::from_abi<{self.as_owner}>({val})"

    def return_into_abi(self, val):
        return f"::taihe::into_abi<{self.as_owner}>({val})"

    def pass_from_abi(self, val):
        return f"::taihe::from_abi<{self.as_param}>({val})"

    def pass_into_abi(self, val):
        return f"::taihe::into_abi<{self.as_param}>({val})"


class EnumTypeCppInfo(AbstractAnalysis[EnumType], AbstractTypeCppInfo):
    def __init__(self, am: AnalysisManager, t: EnumType):
        super().__init__(am, t)
        enum_cpp_info = EnumCppInfo.get(am, t.ty_decl)
        self.decl_headers = [enum_cpp_info.decl_header]
        self.defn_headers = [enum_cpp_info.defn_header]
        self.impl_headers = [enum_cpp_info.defn_header]
        self.as_owner = enum_cpp_info.as_owner
        self.as_param = enum_cpp_info.as_param


class UnionTypeCppInfo(AbstractAnalysis[UnionType], AbstractTypeCppInfo):
    def __init__(self, am: AnalysisManager, t: UnionType):
        super().__init__(am, t)
        union_cpp_info = UnionCppInfo.get(am, t.ty_decl)
        self.decl_headers = [union_cpp_info.decl_header]
        self.defn_headers = [union_cpp_info.defn_header]
        self.impl_headers = [union_cpp_info.impl_header]
        self.as_owner = union_cpp_info.as_owner
        self.as_param = union_cpp_info.as_param


class StructTypeCppInfo(AbstractAnalysis[StructType], AbstractTypeCppInfo):
    def __init__(self, am: AnalysisManager, t: StructType):
        super().__init__(am, t)
        struct_cpp_info = StructCppInfo.get(am, t.ty_decl)
        self.decl_headers = [struct_cpp_info.decl_header]
        self.defn_headers = [struct_cpp_info.defn_header]
        self.impl_headers = [struct_cpp_info.impl_header]
        self.as_owner = struct_cpp_info.as_owner
        self.as_param = struct_cpp_info.as_param


class IfaceTypeCppInfo(AbstractAnalysis[IfaceType], AbstractTypeCppInfo):
    def __init__(self, am: AnalysisManager, t: IfaceType):
        super().__init__(am, t)
        iface_cpp_info = IfaceCppInfo.get(am, t.ty_decl)
        self.decl_headers = [iface_cpp_info.decl_header]
        self.defn_headers = [iface_cpp_info.defn_header]
        self.impl_headers = [iface_cpp_info.impl_header]
        self.as_owner = iface_cpp_info.as_owner
        self.as_param = iface_cpp_info.as_param


class ScalarTypeCppInfo(AbstractAnalysis[ScalarType], AbstractTypeCppInfo):
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
        self.decl_headers = []
        self.defn_headers = []
        self.impl_headers = []
        self.as_param = res
        self.as_owner = res


class OpaqueTypeCppInfo(AbstractAnalysis[OpaqueType], AbstractTypeCppInfo):
    def __init__(self, am: AnalysisManager, t: OpaqueType) -> None:
        super().__init__(am, t)
        self.decl_headers = []
        self.defn_headers = []
        self.impl_headers = []
        self.as_param = "uintptr_t"
        self.as_owner = "uintptr_t"


class StringTypeCppInfo(AbstractAnalysis[StringType], AbstractTypeCppInfo):
    def __init__(self, am: AnalysisManager, t: StringType):
        super().__init__(am, t)
        self.decl_headers = ["taihe/string.hpp"]
        self.defn_headers = ["taihe/string.hpp"]
        self.impl_headers = ["taihe/string.hpp"]
        self.as_owner = "::taihe::string"
        self.as_param = "::taihe::string_view"


class ArrayTypeCppInfo(AbstractAnalysis[ArrayType], AbstractTypeCppInfo):
    def __init__(self, am: AnalysisManager, t: ArrayType) -> None:
        super().__init__(am, t)
        arg_ty_cpp_info = TypeCppInfo.get(am, t.item_ty)
        self.decl_headers = ["taihe/array.hpp", *arg_ty_cpp_info.decl_headers]
        self.defn_headers = ["taihe/array.hpp", *arg_ty_cpp_info.decl_headers]
        self.impl_headers = ["taihe/array.hpp", *arg_ty_cpp_info.impl_headers]
        self.as_owner = f"::taihe::array<{arg_ty_cpp_info.as_owner}>"
        self.as_param = f"::taihe::array_view<{arg_ty_cpp_info.as_owner}>"


class OptionalTypeCppInfo(AbstractAnalysis[OptionalType], AbstractTypeCppInfo):
    def __init__(self, am: AnalysisManager, t: OptionalType) -> None:
        super().__init__(am, t)
        arg_ty_cpp_info = TypeCppInfo.get(am, t.item_ty)
        self.decl_headers = ["taihe/optional.hpp", *arg_ty_cpp_info.decl_headers]
        self.defn_headers = ["taihe/optional.hpp", *arg_ty_cpp_info.decl_headers]
        self.impl_headers = ["taihe/optional.hpp", *arg_ty_cpp_info.impl_headers]
        self.as_owner = f"::taihe::optional<{arg_ty_cpp_info.as_owner}>"
        self.as_param = f"::taihe::optional_view<{arg_ty_cpp_info.as_owner}>"


class VectorTypeCppInfo(AbstractAnalysis[VectorType], AbstractTypeCppInfo):
    def __init__(self, am: AnalysisManager, t: VectorType) -> None:
        super().__init__(am, t)
        val_ty_cpp_info = TypeCppInfo.get(am, t.val_ty)
        self.decl_headers = ["taihe/vector.hpp", *val_ty_cpp_info.decl_headers]
        self.defn_headers = ["taihe/vector.hpp", *val_ty_cpp_info.decl_headers]
        self.impl_headers = ["taihe/vector.hpp", *val_ty_cpp_info.impl_headers]
        self.as_owner = f"::taihe::vector<{val_ty_cpp_info.as_owner}>"
        self.as_param = f"::taihe::vector_view<{val_ty_cpp_info.as_owner}>"


class MapTypeCppInfo(AbstractAnalysis[MapType], AbstractTypeCppInfo):
    def __init__(self, am: AnalysisManager, t: MapType) -> None:
        super().__init__(am, t)
        key_ty_cpp_info = TypeCppInfo.get(am, t.key_ty)
        val_ty_cpp_info = TypeCppInfo.get(am, t.val_ty)
        self.decl_headers = [
            "taihe/map.hpp",
            *key_ty_cpp_info.decl_headers,
            *val_ty_cpp_info.decl_headers,
        ]
        self.defn_headers = [
            "taihe/map.hpp",
            *key_ty_cpp_info.decl_headers,
            *val_ty_cpp_info.decl_headers,
        ]
        self.impl_headers = [
            "taihe/map.hpp",
            *key_ty_cpp_info.impl_headers,
            *val_ty_cpp_info.impl_headers,
        ]
        self.as_owner = (
            f"::taihe::map<{key_ty_cpp_info.as_owner}, {val_ty_cpp_info.as_owner}>"
        )
        self.as_param = (
            f"::taihe::map_view<{key_ty_cpp_info.as_owner}, {val_ty_cpp_info.as_owner}>"
        )


class SetTypeCppInfo(AbstractAnalysis[SetType], AbstractTypeCppInfo):
    def __init__(self, am: AnalysisManager, t: SetType) -> None:
        super().__init__(am, t)
        key_ty_cpp_info = TypeCppInfo.get(am, t.key_ty)
        self.decl_headers = ["taihe/set.hpp", *key_ty_cpp_info.decl_headers]
        self.defn_headers = ["taihe/set.hpp", *key_ty_cpp_info.decl_headers]
        self.impl_headers = ["taihe/set.hpp", *key_ty_cpp_info.impl_headers]
        self.as_owner = f"::taihe::set<{key_ty_cpp_info.as_owner}>"
        self.as_param = f"::taihe::set_view<{key_ty_cpp_info.as_owner}>"


class CallbackTypeCppInfo(AbstractAnalysis[CallbackType], AbstractTypeCppInfo):
    def __init__(self, am: AnalysisManager, t: CallbackType) -> None:
        super().__init__(am, t)
        if return_ty := t.return_ty:
            return_ty_cpp_info = TypeCppInfo.get(am, return_ty)
            return_ty_decl_headers = return_ty_cpp_info.decl_headers
            return_ty_impl_headers = return_ty_cpp_info.impl_headers
            return_ty_as_owner = return_ty_cpp_info.as_owner
        else:
            return_ty_decl_headers = []
            return_ty_impl_headers = []
            return_ty_as_owner = "void"
        params_ty_decl_headers = []
        params_ty_impl_headers = []
        params_ty_as_param = []
        for param_ty in t.params_ty:
            param_ty_cpp_info = TypeCppInfo.get(am, param_ty)
            params_ty_decl_headers.extend(param_ty_cpp_info.decl_headers)
            params_ty_impl_headers.extend(param_ty_cpp_info.impl_headers)
            params_ty_as_param.append(param_ty_cpp_info.as_param)
        params_fmt = ", ".join(params_ty_as_param)
        self.decl_headers = [
            "taihe/callback.hpp",
            *return_ty_decl_headers,
            *params_ty_decl_headers,
        ]
        self.defn_headers = [
            "taihe/callback.hpp",
            *return_ty_decl_headers,
            *params_ty_decl_headers,
        ]
        self.impl_headers = [
            "taihe/callback.hpp",
            *return_ty_impl_headers,
            *params_ty_impl_headers,
        ]
        self.as_owner = f"::taihe::callback<{return_ty_as_owner}({params_fmt})>"
        self.as_param = f"::taihe::callback_view<{return_ty_as_owner}({params_fmt})>"


class TypeCppInfo(TypeVisitor[AbstractTypeCppInfo]):
    def __init__(self, am: AnalysisManager):
        self.am = am

    @staticmethod
    def get(am: AnalysisManager, t: Type) -> AbstractTypeCppInfo:
        return TypeCppInfo(am).handle_type(t)

    @override
    def visit_enum_type(self, t: EnumType) -> AbstractTypeCppInfo:
        return EnumTypeCppInfo.get(self.am, t)

    @override
    def visit_union_type(self, t: UnionType) -> AbstractTypeCppInfo:
        return UnionTypeCppInfo.get(self.am, t)

    @override
    def visit_struct_type(self, t: StructType) -> AbstractTypeCppInfo:
        return StructTypeCppInfo.get(self.am, t)

    @override
    def visit_iface_type(self, t: IfaceType) -> AbstractTypeCppInfo:
        return IfaceTypeCppInfo.get(self.am, t)

    @override
    def visit_scalar_type(self, t: ScalarType) -> AbstractTypeCppInfo:
        return ScalarTypeCppInfo.get(self.am, t)

    @override
    def visit_opaque_type(self, t: OpaqueType) -> AbstractTypeCppInfo:
        return OpaqueTypeCppInfo.get(self.am, t)

    @override
    def visit_string_type(self, t: StringType) -> AbstractTypeCppInfo:
        return StringTypeCppInfo.get(self.am, t)

    @override
    def visit_array_type(self, t: ArrayType) -> AbstractTypeCppInfo:
        return ArrayTypeCppInfo.get(self.am, t)

    @override
    def visit_optional_type(self, t: OptionalType) -> AbstractTypeCppInfo:
        return OptionalTypeCppInfo.get(self.am, t)

    @override
    def visit_vector_type(self, t: VectorType) -> AbstractTypeCppInfo:
        return VectorTypeCppInfo.get(self.am, t)

    @override
    def visit_map_type(self, t: MapType) -> AbstractTypeCppInfo:
        return MapTypeCppInfo.get(self.am, t)

    @override
    def visit_set_type(self, t: SetType) -> AbstractTypeCppInfo:
        return SetTypeCppInfo.get(self.am, t)

    @override
    def visit_callback_type(self, t: CallbackType) -> AbstractTypeCppInfo:
        return CallbackTypeCppInfo.get(self.am, t)


class PackageCppUserInfo(AbstractAnalysis[PackageDecl]):
    def __init__(self, am: AnalysisManager, p: PackageDecl) -> None:
        super().__init__(am, p)
        self.header = f"{p.name}.user.hpp"


class GlobFuncCppUserInfo(AbstractAnalysis[GlobFuncDecl]):
    def __init__(self, am: AnalysisManager, f: GlobFuncDecl) -> None:
        super().__init__(am, f)
        self.namespace = "::".join(f.parent_pkg.segments)
        self.call_name = f.name
        self.full_name = "::" + self.namespace + "::" + self.call_name