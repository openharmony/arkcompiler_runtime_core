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
from json import dumps

from typing_extensions import override

from taihe.codegen.abi_generator import (
    EnumABIInfo,
    IfaceABIInfo,
    IfaceMethodABIInfo,
    PackageABIInfo,
    StructABIInfo,
    TypeABIInfo,
    UnionABIInfo,
)
from taihe.semantics.declarations import (
    EnumDecl,
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
    STRING,
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
        self.header = f"{d.parent_pkg.name}.{d.name}.proj.0.hpp"

        self.namespace = "::".join(d.parent_pkg.segments)
        self.name = d.name
        self.full_name = "::" + self.namespace + "::" + self.name

        self.as_owner = self.full_name
        self.as_param = self.full_name


class StructCppInfo(AbstractAnalysis[StructDecl]):
    def __init__(self, am: AnalysisManager, d: StructDecl) -> None:
        super().__init__(am, d)
        self.decl_header = f"{d.parent_pkg.name}.{d.name}.proj.0.hpp"
        self.impl_header = f"{d.parent_pkg.name}.{d.name}.proj.1.hpp"

        self.namespace = "::".join(d.parent_pkg.segments)
        self.name = d.name
        self.full_name = "::" + self.namespace + "::" + self.name

        self.as_owner = self.full_name
        self.as_param = self.full_name + " const&"


class UnionCppInfo(AbstractAnalysis[UnionDecl]):
    def __init__(self, am: AnalysisManager, d: UnionDecl) -> None:
        super().__init__(am, d)
        self.decl_header = f"{d.parent_pkg.name}.{d.name}.proj.0.hpp"
        self.impl_header = f"{d.parent_pkg.name}.{d.name}.proj.1.hpp"

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
        self.decl_headers = [enum_cpp_info.header]
        self.impl_headers = [enum_cpp_info.header]
        self.as_owner = enum_cpp_info.as_owner
        self.as_param = enum_cpp_info.as_param


class UnionTypeCppInfo(AbstractAnalysis[UnionType], AbstractTypeCppInfo):
    def __init__(self, am: AnalysisManager, t: UnionType):
        super().__init__(am, t)
        union_cpp_info = UnionCppInfo.get(am, t.ty_decl)
        self.decl_headers = [union_cpp_info.decl_header]
        self.impl_headers = [union_cpp_info.impl_header]
        self.as_owner = union_cpp_info.as_owner
        self.as_param = union_cpp_info.as_param


class StructTypeCppInfo(AbstractAnalysis[StructType], AbstractTypeCppInfo):
    def __init__(self, am: AnalysisManager, t: StructType):
        super().__init__(am, t)
        struct_cpp_info = StructCppInfo.get(am, t.ty_decl)
        self.decl_headers = [struct_cpp_info.decl_header]
        self.impl_headers = [struct_cpp_info.impl_header]
        self.as_owner = struct_cpp_info.as_owner
        self.as_param = struct_cpp_info.as_param


class IfaceTypeCppInfo(AbstractAnalysis[IfaceType], AbstractTypeCppInfo):
    def __init__(self, am: AnalysisManager, t: IfaceType):
        super().__init__(am, t)
        iface_cpp_info = IfaceCppInfo.get(am, t.ty_decl)
        self.decl_headers = [iface_cpp_info.decl_header]
        self.impl_headers = [iface_cpp_info.impl_header]
        self.as_owner = iface_cpp_info.as_owner
        self.as_param = iface_cpp_info.as_param


class ScalarTypeCppInfo(AbstractAnalysis[ScalarType], AbstractTypeCppInfo):
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


class OpaqueTypeCppInfo(AbstractAnalysis[OpaqueType], AbstractTypeCppInfo):
    def __init__(self, am: AnalysisManager, t: OpaqueType) -> None:
        super().__init__(am, t)
        self.decl_headers = []
        self.impl_headers = []
        self.as_param = "uintptr_t"
        self.as_owner = "uintptr_t"


class StringTypeCppInfo(AbstractAnalysis[StringType], AbstractTypeCppInfo):
    def __init__(self, am: AnalysisManager, t: StringType):
        super().__init__(am, t)
        self.decl_headers = ["taihe/string.hpp"]
        self.impl_headers = ["taihe/string.hpp"]
        self.as_owner = "::taihe::string"
        self.as_param = "::taihe::string_view"


class ArrayTypeCppInfo(AbstractAnalysis[ArrayType], AbstractTypeCppInfo):
    def __init__(self, am: AnalysisManager, t: ArrayType) -> None:
        super().__init__(am, t)
        arg_ty_cpp_info = TypeCppInfo.get(am, t.item_ty)
        self.decl_headers = ["taihe/array.hpp", *arg_ty_cpp_info.decl_headers]
        self.impl_headers = ["taihe/array.hpp", *arg_ty_cpp_info.impl_headers]
        self.as_owner = f"::taihe::array<{arg_ty_cpp_info.as_owner}>"
        self.as_param = f"::taihe::array_view<{arg_ty_cpp_info.as_owner}>"


class OptionalTypeCppInfo(AbstractAnalysis[OptionalType], AbstractTypeCppInfo):
    def __init__(self, am: AnalysisManager, t: OptionalType) -> None:
        super().__init__(am, t)
        arg_ty_cpp_info = TypeCppInfo.get(am, t.item_ty)
        self.decl_headers = ["taihe/optional.hpp", *arg_ty_cpp_info.decl_headers]
        self.impl_headers = ["taihe/optional.hpp", *arg_ty_cpp_info.impl_headers]
        self.as_owner = f"::taihe::optional<{arg_ty_cpp_info.as_owner}>"
        self.as_param = f"::taihe::optional_view<{arg_ty_cpp_info.as_owner}>"


class VectorTypeCppInfo(AbstractAnalysis[VectorType], AbstractTypeCppInfo):
    def __init__(self, am: AnalysisManager, t: VectorType) -> None:
        super().__init__(am, t)
        val_ty_cpp_info = TypeCppInfo.get(am, t.val_ty)
        self.decl_headers = ["taihe/vector.hpp", *val_ty_cpp_info.decl_headers]
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
        self.impl_headers = ["taihe/set.hpp", *key_ty_cpp_info.impl_headers]
        self.as_owner = f"::taihe::set<{key_ty_cpp_info.as_owner}>"
        self.as_param = f"::taihe::set_view<{key_ty_cpp_info.as_owner}>"


class CallbackTypeCppInfo(AbstractAnalysis[CallbackType], AbstractTypeCppInfo):
    def __init__(self, am: AnalysisManager, t: CallbackType) -> None:
        super().__init__(am, t)
        if return_ty := t.return_ty:
            return_ty_cpp_info = TypeCppInfo.get(am, return_ty)
            return_ty_decl_headers = return_ty_cpp_info.decl_headers
            return_ty_defn_headers = return_ty_cpp_info.impl_headers
            return_ty_as_owner = return_ty_cpp_info.as_owner
        else:
            return_ty_decl_headers = []
            return_ty_defn_headers = []
            return_ty_as_owner = "void"
        params_ty_decl_headers = []
        params_ty_defn_headers = []
        params_ty_as_param = []
        for param_ty in t.params_ty:
            param_ty_cpp_info = TypeCppInfo.get(am, param_ty)
            params_ty_decl_headers.extend(param_ty_cpp_info.decl_headers)
            params_ty_defn_headers.extend(param_ty_cpp_info.impl_headers)
            params_ty_as_param.append(param_ty_cpp_info.as_param)
        params_fmt = ", ".join(params_ty_as_param)
        self.decl_headers = [
            "taihe/callback.hpp",
            *return_ty_decl_headers,
            *params_ty_decl_headers,
        ]
        self.impl_headers = [
            "taihe/callback.hpp",
            *return_ty_defn_headers,
            *params_ty_defn_headers,
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


class CppHeadersGenerator:
    def __init__(self, tm: OutputManager, am: AnalysisManager):
        self.tm = tm
        self.am = am

    def generate(self, pg: PackageGroup):
        for pkg in pg.packages:
            self.gen_package_files(pkg)

    def gen_package_files(self, pkg: PackageDecl):
        pkg_cpp_info = PackageCppInfo.get(self.am, pkg)
        pkg_cpp_target = COutputBuffer.create(
            self.tm, f"include/{pkg_cpp_info.header}", True
        )
        pkg_abi_info = PackageABIInfo.get(self.am, pkg)
        pkg_cpp_target.include(pkg_abi_info.header)
        for enum in pkg.enums:
            enum_abi_info = EnumABIInfo.get(self.am, enum)
            enum_cpp_info = EnumCppInfo.get(self.am, enum)
            self.gen_enum_file(enum, enum_abi_info, enum_cpp_info)
            pkg_cpp_target.include(enum_cpp_info.header)
        for struct in pkg.structs:
            struct_abi_info = StructABIInfo.get(self.am, struct)
            struct_cpp_info = StructCppInfo.get(self.am, struct)
            self.gen_struct_decl_file(struct, struct_abi_info, struct_cpp_info)
            self.gen_struct_impl_file(struct, struct_abi_info, struct_cpp_info)
            pkg_cpp_target.include(struct_cpp_info.impl_header)
        for union in pkg.unions:
            union_abi_info = UnionABIInfo.get(self.am, union)
            union_cpp_info = UnionCppInfo.get(self.am, union)
            self.gen_union_decl_file(union, union_abi_info, union_cpp_info)
            self.gen_union_impl_file(union, union_abi_info, union_cpp_info)
            pkg_cpp_target.include(union_cpp_info.impl_header)
        for iface in pkg.interfaces:
            iface_abi_info = IfaceABIInfo.get(self.am, iface)
            iface_cpp_info = IfaceCppInfo.get(self.am, iface)
            self.gen_iface_decl_file(iface, iface_abi_info, iface_cpp_info)
            self.gen_iface_defn_file(iface, iface_abi_info, iface_cpp_info)
            self.gen_iface_impl_file(iface, iface_abi_info, iface_cpp_info)
            pkg_cpp_target.include(iface_cpp_info.impl_header)

    def gen_enum_file(
        self,
        enum: EnumDecl,
        enum_abi_info: EnumABIInfo,
        enum_cpp_info: EnumCppInfo,
    ):
        enum_cpp_target = COutputBuffer.create(
            self.tm, f"include/{enum_cpp_info.header}", True
        )
        enum_cpp_target.include("taihe/common.hpp")
        self.gen_enum_defn(
            enum,
            enum_abi_info,
            enum_cpp_info,
            enum_cpp_target,
        )
        self.gen_enum_same(
            enum,
            enum_abi_info,
            enum_cpp_info,
            enum_cpp_target,
        )
        self.gen_enum_hash(
            enum,
            enum_abi_info,
            enum_cpp_info,
            enum_cpp_target,
        )
        self.gen_enum_type_traits(
            enum,
            enum_abi_info,
            enum_cpp_info,
            enum_cpp_target,
        )

    def gen_enum_defn(
        self,
        enum: EnumDecl,
        enum_abi_info: EnumABIInfo,
        enum_cpp_info: EnumCppInfo,
        enum_cpp_target: COutputBuffer,
    ):
        enum_cpp_target.writeln(
            f"namespace {enum_cpp_info.namespace} {{",
            f"struct {enum_cpp_info.name} {{",
        )
        # key type
        enum_cpp_target.writeln(
            f"    enum class key_t: {enum_abi_info.abi_type} {{",
        )
        for item in enum.items:
            enum_cpp_target.writeln(
                f"        {item.name},",
            )
        enum_cpp_target.writeln(
            f"    }};",
        )
        # basic methods
        enum_cpp_target.writeln(
            f"    {enum_cpp_info.name}(key_t key) : key(key) {{}}",
            f"    {enum_cpp_info.name}({enum_cpp_info.name} const& other) : key(other.key) {{}}",
            f"    {enum_cpp_info.name}& operator=({enum_cpp_info.name} other) {{",
            f"        key = other.key;",
            f"        return *this;",
            f"    }}",
        )
        self.gen_enum_key_getter(
            enum,
            enum_abi_info,
            enum_cpp_info,
            enum_cpp_target,
        )
        self.gen_enum_value_getter(
            enum,
            enum_abi_info,
            enum_cpp_info,
            enum_cpp_target,
        )
        # properties
        enum_cpp_target.writeln(
            f"private:",
            f"    key_t key;",
            f"}};",
            f"}}",
        )

    def gen_enum_key_getter(
        self,
        enum: EnumDecl,
        enum_abi_info: EnumABIInfo,
        enum_cpp_info: EnumCppInfo,
        enum_cpp_target: COutputBuffer,
    ):
        enum_cpp_target.writeln(
            f"    key_t get_key() const {{",
            f"        return this->key;",
            f"    }}",
        )

    def gen_enum_value_getter(
        self,
        enum: EnumDecl,
        enum_abi_info: EnumABIInfo,
        enum_cpp_info: EnumCppInfo,
        enum_cpp_target: COutputBuffer,
    ):
        if enum.ty_ref is None:
            return
        if enum.ty_ref.resolved_ty == STRING:
            as_owner = "char const*"
        else:
            # pyre-ignore
            ty_cpp_info = TypeCppInfo.get(self.am, enum.ty_ref.resolved_ty)
            as_owner = ty_cpp_info.as_owner
        enum_cpp_target.writeln(
            f"    static constexpr {as_owner} table[] = {{",
        )
        for item in enum.items:
            enum_cpp_target.writeln(
                f"        {dumps(item.value)},",
            )
        enum_cpp_target.writeln(
            f"    }};",
        )
        enum_cpp_target.writeln(
            f"    {as_owner} get_value() const {{",
            f"        return table[({enum_abi_info.abi_type})key];",
            f"    }}",
        )
        enum_cpp_target.writeln(
            f"    operator {as_owner}() const {{",
            f"        return table[({enum_abi_info.abi_type})key];",
            f"    }}",
        )

    def gen_enum_same(
        self,
        enum: EnumDecl,
        enum_abi_info: EnumABIInfo,
        enum_cpp_info: EnumCppInfo,
        enum_cpp_target: COutputBuffer,
    ):
        # others
        enum_cpp_target.writeln(
            f"namespace taihe {{",
            f"inline bool same_impl(adl_helper_t, {enum_cpp_info.full_name} lhs, {enum_cpp_info.full_name} rhs) {{",
            f"    return lhs.get_key() == rhs.get_key();",
            f"}}",
            f"}}",
        )

    def gen_enum_hash(
        self,
        enum: EnumDecl,
        enum_abi_info: EnumABIInfo,
        enum_cpp_info: EnumCppInfo,
        enum_cpp_target: COutputBuffer,
    ):
        enum_cpp_target.writeln(
            f"namespace taihe {{",
            f"inline auto hash_impl(adl_helper_t, {enum_cpp_info.as_param} val) -> ::std::size_t {{",
            f"    return ::std::hash<{enum_abi_info.abi_type}>{{}}(({enum_abi_info.abi_type})val.get_key());",
            f"}}",
            f"}}",
        )

    def gen_enum_type_traits(
        self,
        enum: EnumDecl,
        enum_abi_info: EnumABIInfo,
        enum_cpp_info: EnumCppInfo,
        enum_cpp_target: COutputBuffer,
    ):
        enum_cpp_target.writeln(
            f"namespace taihe {{",
            f"template<>",
            f"struct as_abi<{enum_cpp_info.full_name}> {{",
            f"    using type = {enum_abi_info.abi_type};",
            f"}};",
            f"template<>",
            f"struct as_param<{enum_cpp_info.full_name}> {{",
            f"    using type = {enum_cpp_info.full_name};",
            f"}};",
            f"}}",
        )

    def gen_union_decl_file(
        self,
        union: UnionDecl,
        union_abi_info: UnionABIInfo,
        union_cpp_info: UnionCppInfo,
    ):
        union_cpp_decl_target = COutputBuffer.create(
            self.tm, f"include/{union_cpp_info.decl_header}", True
        )
        union_cpp_decl_target.writeln(
            f"namespace {union_cpp_info.namespace} {{",
            f"struct {union_cpp_info.name};",
            f"}}",
        )

    def gen_union_impl_file(
        self,
        union: UnionDecl,
        union_abi_info: UnionABIInfo,
        union_cpp_info: UnionCppInfo,
    ):
        union_cpp_defn_target = COutputBuffer.create(
            self.tm, f"include/{union_cpp_info.impl_header}", True
        )
        union_cpp_defn_target.include("taihe/common.hpp")
        union_cpp_defn_target.include(union_cpp_info.decl_header)
        union_cpp_defn_target.include(union_abi_info.impl_header)
        self.gen_union_defn(
            union,
            union_abi_info,
            union_cpp_info,
            union_cpp_defn_target,
        )
        self.gen_union_same(
            union,
            union_abi_info,
            union_cpp_info,
            union_cpp_defn_target,
        )
        self.gen_union_hash(
            union,
            union_abi_info,
            union_cpp_info,
            union_cpp_defn_target,
        )
        self.gen_union_type_traits(
            union,
            union_abi_info,
            union_cpp_info,
            union_cpp_defn_target,
        )

    def gen_union_defn(
        self,
        union: UnionDecl,
        union_abi_info: UnionABIInfo,
        union_cpp_info: UnionCppInfo,
        union_cpp_defn_target: COutputBuffer,
    ):
        union_cpp_defn_target.writeln(
            f"namespace {union_cpp_info.namespace} {{",
            f"struct {union_cpp_info.name} {{",
        )
        # tag type
        union_cpp_defn_target.writeln(
            f"    enum class tag_t : {union_abi_info.tag_type} {{",
        )
        for field in union.fields:
            union_cpp_defn_target.writeln(
                f"        {field.name},",
            )
        union_cpp_defn_target.writeln(
            f"    }};",
        )
        # storage type
        union_cpp_defn_target.writeln(
            f"    union storage_t {{",
            f"        storage_t() {{}}",
            f"        ~storage_t() {{}}",
        )
        for field in union.fields:
            if field.ty_ref is None:
                continue
            type_cpp_info = TypeCppInfo.get(self.am, field.ty_ref.resolved_ty)
            union_cpp_defn_target.include(*type_cpp_info.impl_headers)
            union_cpp_defn_target.writeln(
                f"        {type_cpp_info.as_owner} {field.name};",
            )
        union_cpp_defn_target.writeln(
            f"    }};",
        )
        # copy constructor
        union_cpp_defn_target.writeln(
            f"    {union_cpp_info.name}({union_cpp_info.name} const& other) : m_tag(other.m_tag) {{",
            f"        switch (m_tag) {{",
        )
        for field in union.fields:
            if field.ty_ref is None:
                continue
            union_cpp_defn_target.writeln(
                f"        case tag_t::{field.name}:",
                f"            new (&m_data.{field.name}) decltype(m_data.{field.name})(other.m_data.{field.name});",
                f"            break;",
            )
        union_cpp_defn_target.writeln(
            f"        default:",
            f"            break;",
            f"        }}",
            f"    }}",
        )
        # move constructor
        union_cpp_defn_target.writeln(
            f"    {union_cpp_info.name}({union_cpp_info.name}&& other) : m_tag(other.m_tag) {{",
            f"        switch (m_tag) {{",
        )
        for field in union.fields:
            if field.ty_ref is None:
                continue
            union_cpp_defn_target.writeln(
                f"        case tag_t::{field.name}:",
                f"            new (&m_data.{field.name}) decltype(m_data.{field.name})(::std::move(other.m_data.{field.name}));",
                f"            break;",
            )
        union_cpp_defn_target.writeln(
            f"        default:",
            f"            break;",
            f"        }}",
            f"    }}",
        )
        # copy assignment
        union_cpp_defn_target.writeln(
            f"    {union_cpp_info.name}& operator=({union_cpp_info.name} const& other) {{",
            f"        if (this != &other) {{",
            f"            ::std::destroy_at(this);",
            f"            new (this) {union_cpp_info.name}(other);",
            f"        }}",
            f"        return *this;",
            f"    }}",
        )
        # move assignment
        union_cpp_defn_target.writeln(
            f"    {union_cpp_info.name}& operator=({union_cpp_info.name}&& other) {{",
            f"        if (this != &other) {{",
            f"            ::std::destroy_at(this);",
            f"            new (this) {union_cpp_info.name}(::std::move(other));",
            f"        }}",
            f"        return *this;",
            f"    }}",
        )
        # destructor
        union_cpp_defn_target.writeln(
            f"    ~{union_cpp_info.name}() {{",
            f"        switch (m_tag) {{",
        )
        for field in union.fields:
            if field.ty_ref is None:
                continue
            union_cpp_defn_target.writeln(
                f"        case tag_t::{field.name}:",
                f"            ::std::destroy_at(&m_data.{field.name});",
                f"            break;",
            )
        union_cpp_defn_target.writeln(
            f"        default:",
            f"            break;",
            f"        }}",
            f"    }}",
        )
        self.gen_union_methods(
            union,
            union_abi_info,
            union_cpp_info,
            union_cpp_defn_target,
        )
        # properties
        union_cpp_defn_target.writeln(
            f"private:",
            f"    tag_t m_tag;",
            f"    storage_t m_data;",
            f"}};",
            f"}}",
        )

    def gen_union_methods(
        self,
        union: UnionDecl,
        union_abi_info: UnionABIInfo,
        union_cpp_info: UnionCppInfo,
        union_cpp_defn_target: COutputBuffer,
    ):
        # in place constructor
        for field in union.fields:
            if field.ty_ref is None:
                union_cpp_defn_target.writeln(
                    f"    {union_cpp_info.name}(::taihe::static_tag_t<tag_t::{field.name}>) : m_tag(tag_t::{field.name}) {{}}",
                )
            else:
                union_cpp_defn_target.writeln(
                    f"    template<typename... Args>",
                    f"    {union_cpp_info.name}(::taihe::static_tag_t<tag_t::{field.name}>, Args&&... args) : m_tag(tag_t::{field.name}) {{",
                    f"        new (&m_data.{field.name}) decltype(m_data.{field.name})(::std::forward<Args>(args)...);",
                    f"    }}",
                )
        # creator
        union_cpp_defn_target.writeln(
            f"    template<tag_t tag, typename... Args>",
            f"    static {union_cpp_info.name} make(Args&&... args) {{",
            f"        return {union_cpp_info.name}(::taihe::static_tag<tag>, ::std::forward<Args>(args)...);",
            f"    }}",
        )
        # emplacement
        union_cpp_defn_target.writeln(
            f"    template<tag_t tag, typename... Args>",
            f"    {union_cpp_info.name} const& emplace(Args&&... args) {{",
            f"        ::std::destroy_at(this);",
            f"        new (this) {union_cpp_info.name}(::taihe::static_tag<tag>, ::std::forward<Args>(args)...);",
            f"        return *this;",
            f"    }}",
        )
        # non-const reference getter
        union_cpp_defn_target.writeln(
            f"    template<tag_t tag>",
            f"    auto& get_ref() {{",
        )
        for field in union.fields:
            if field.ty_ref:
                union_cpp_defn_target.writeln(
                    f"        if constexpr (tag == tag_t::{field.name}) {{",
                    f"            return m_data.{field.name};",
                    f"        }}",
                )
        union_cpp_defn_target.writeln(
            f"    }}",
        )
        # non-const pointer getter
        union_cpp_defn_target.writeln(
            f"    template<tag_t tag>",
            f"    auto* get_ptr() {{",
            f"        return m_tag == tag ? &get_ref<tag>() : nullptr;",
            f"    }}",
        )
        # const reference getter
        union_cpp_defn_target.writeln(
            f"    template<tag_t tag>",
            f"    auto const& get_ref() const {{",
        )
        for field in union.fields:
            if field.ty_ref:
                union_cpp_defn_target.writeln(
                    f"        if constexpr (tag == tag_t::{field.name}) {{",
                    f"            return m_data.{field.name};",
                    f"        }}",
                )
        union_cpp_defn_target.writeln(
            f"    }}",
        )
        # const pointer getter
        union_cpp_defn_target.writeln(
            f"    template<tag_t tag>",
            f"    auto const* get_ptr() const {{",
            f"        return m_tag == tag ? &get_ref<tag>() : nullptr;",
            f"    }}",
        )
        # checker
        union_cpp_defn_target.writeln(
            f"    template<tag_t tag>",
            f"    bool holds() const {{",
            f"        return m_tag == tag;",
            f"    }}",
            f"    tag_t get_tag() const {{",
            f"        return m_tag;",
            f"    }}",
        )
        # non_const visitor
        union_cpp_defn_target.writeln(
            f"    template<typename Visitor>",
            f"    auto accept_template(Visitor&& visitor) {{",
            f"        switch (m_tag) {{",
        )
        for field in union.fields:
            result = f"::taihe::static_tag<tag_t::{field.name}>"
            if field.ty_ref:
                result += f", m_data.{field.name}"
            union_cpp_defn_target.writeln(
                f"        case tag_t::{field.name}:",
                f"            return visitor({result});",
            )
        union_cpp_defn_target.writeln(
            f"        }}",
            f"    }}",
        )
        # const visitor
        union_cpp_defn_target.writeln(
            f"    template<typename Visitor>",
            f"    auto accept_template(Visitor&& visitor) const {{",
            f"        switch (m_tag) {{",
        )
        for field in union.fields:
            result = f"::taihe::static_tag<tag_t::{field.name}>"
            if field.ty_ref:
                result += f", m_data.{field.name}"
            union_cpp_defn_target.writeln(
                f"        case tag_t::{field.name}:",
                f"            return visitor({result});",
            )
        union_cpp_defn_target.writeln(
            f"        }}",
            f"    }}",
        )
        # named methods
        for field in union.fields:
            union_cpp_defn_target.writeln(
                f"    template<typename... Args>",
                f"    static {union_cpp_info.name} make_{field.name}(Args&&... args) {{",
                f"        return make<tag_t::{field.name}>(::std::forward<Args>(args)...);",
                f"    }}",
                f"    template<typename... Args>",
                f"    {union_cpp_info.name} const& emplace_{field.name}(Args&&... args) {{",
                f"        return emplace<tag_t::{field.name}>(::std::forward<Args>(args)...);",
                f"    }}",
                f"    bool holds_{field.name}() const {{",
                f"        return holds<tag_t::{field.name}>();",
                f"    }}",
            )
            if field.ty_ref:
                union_cpp_defn_target.writeln(
                    f"    auto* get_{field.name}_ptr() {{",
                    f"        return get_ptr<tag_t::{field.name}>();",
                    f"    }}",
                    f"    auto const* get_{field.name}_ptr() const {{",
                    f"        return get_ptr<tag_t::{field.name}>();",
                    f"    }}",
                    f"    auto& get_{field.name}_ref() {{",
                    f"        return get_ref<tag_t::{field.name}>();",
                    f"    }}",
                    f"    auto const& get_{field.name}_ref() const {{",
                    f"        return get_ref<tag_t::{field.name}>();",
                    f"    }}",
                )
        # named visitor
        union_cpp_defn_target.writeln(
            f"    template<typename Visitor>",
            f"    auto accept(Visitor&& visitor) {{",
            f"        switch (m_tag) {{",
        )
        for field in union.fields:
            result = "" if field.ty_ref is None else f"m_data.{field.name}"
            union_cpp_defn_target.writeln(
                f"        case tag_t::{field.name}:",
                f"            return visitor.{field.name}({result});",
            )
        union_cpp_defn_target.writeln(
            f"        }}",
            f"    }}",
        )
        # named const visitor
        union_cpp_defn_target.writeln(
            f"    template<typename Visitor>",
            f"    auto accept(Visitor&& visitor) const {{",
            f"        switch (m_tag) {{",
        )
        for field in union.fields:
            result = "" if field.ty_ref is None else f"m_data.{field.name}"
            union_cpp_defn_target.writeln(
                f"        case tag_t::{field.name}:",
                f"            return visitor.{field.name}({result});",
            )
        union_cpp_defn_target.writeln(
            f"        }}",
            f"    }}",
        )

    def gen_union_same(
        self,
        union: UnionDecl,
        union_abi_info: UnionABIInfo,
        union_cpp_info: UnionCppInfo,
        union_cpp_defn_target: COutputBuffer,
    ):
        result = "false"
        for field in union.fields:
            cond = f"lhs.holds_{field.name}() && rhs.holds_{field.name}()"
            if field.ty_ref:
                cond = f"{cond} && same(lhs.get_{field.name}_ref(), rhs.get_{field.name}_ref())"
            result = f"{result} || ({cond})"
        union_cpp_defn_target.writeln(
            f"namespace taihe {{",
            f"inline bool same_impl(adl_helper_t, {union_cpp_info.as_param} lhs, {union_cpp_info.as_param} rhs) {{",
            f"    return {result};",
            f"}}",
            f"}}",
        )

    def gen_union_hash(
        self,
        union: UnionDecl,
        union_abi_info: UnionABIInfo,
        union_cpp_info: UnionCppInfo,
        union_cpp_defn_target: COutputBuffer,
    ):
        union_cpp_defn_target.writeln(
            f"namespace taihe {{",
            f"inline auto hash_impl(adl_helper_t, {union_cpp_info.as_param} val) -> ::std::size_t {{",
            f"    switch (val.get_tag()) {{",
            f"        ::std::size_t seed;",
        )
        for field in union.fields:
            val = "0x9e3779b9 + (seed << 6) + (seed >> 2)"
            if field.ty_ref:
                val = f"{val} + hash(val.get_{field.name}_ref())"
            union_cpp_defn_target.writeln(
                f"    case {union_cpp_info.full_name}::tag_t::{field.name}:",
                f"        seed = (::std::size_t){union_cpp_info.full_name}::tag_t::{field.name};",
                f"        return seed ^ ({val});",
            )
        union_cpp_defn_target.writeln(
            f"    }}",
            f"}}",
            f"}}",
        )

    def gen_union_type_traits(
        self,
        union: UnionDecl,
        union_abi_info: UnionABIInfo,
        union_cpp_info: UnionCppInfo,
        union_cpp_defn_target: COutputBuffer,
    ):
        union_cpp_defn_target.writeln(
            f"namespace taihe {{",
            f"template<>",
            f"struct as_abi<{union_cpp_info.as_owner}> {{",
            f"    using type = {union_abi_info.as_owner};",
            f"}};",
            f"template<>",
            f"struct as_abi<{union_cpp_info.as_param}> {{",
            f"    using type = {union_abi_info.as_param};",
            f"}};",
            f"template<>",
            f"struct as_param<{union_cpp_info.as_owner}> {{",
            f"    using type = {union_cpp_info.as_param};",
            f"}};",
            f"}}",
        )

    def gen_struct_decl_file(
        self,
        struct: StructDecl,
        struct_abi_info: StructABIInfo,
        struct_cpp_info: StructCppInfo,
    ):
        struct_cpp_decl_target = COutputBuffer.create(
            self.tm, f"include/{struct_cpp_info.decl_header}", True
        )
        struct_cpp_decl_target.writeln(
            f"namespace {struct_cpp_info.namespace} {{",
            f"struct {struct_cpp_info.name};",
            f"}}",
        )

    def gen_struct_impl_file(
        self,
        struct: StructDecl,
        struct_abi_info: StructABIInfo,
        struct_cpp_info: StructCppInfo,
    ):
        struct_cpp_defn_target = COutputBuffer.create(
            self.tm, f"include/{struct_cpp_info.impl_header}", True
        )
        struct_cpp_defn_target.include("taihe/common.hpp")
        struct_cpp_defn_target.include(struct_cpp_info.decl_header)
        struct_cpp_defn_target.include(struct_abi_info.impl_header)
        self.gen_struct_defn(
            struct,
            struct_abi_info,
            struct_cpp_info,
            struct_cpp_defn_target,
        )
        self.gen_struct_hash(
            struct,
            struct_abi_info,
            struct_cpp_info,
            struct_cpp_defn_target,
        )
        self.gen_struct_same(
            struct,
            struct_abi_info,
            struct_cpp_info,
            struct_cpp_defn_target,
        )
        self.gen_struct_type_traits(
            struct,
            struct_abi_info,
            struct_cpp_info,
            struct_cpp_defn_target,
        )

    def gen_struct_defn(
        self,
        struct: StructDecl,
        struct_abi_info: StructABIInfo,
        struct_cpp_info: StructCppInfo,
        struct_cpp_defn_target: COutputBuffer,
    ):
        struct_cpp_defn_target.writeln(
            f"namespace {struct_cpp_info.namespace} {{",
            f"struct {struct_cpp_info.name} {{",
        )
        for field in struct.fields:
            type_cpp_info = TypeCppInfo.get(self.am, field.ty_ref.resolved_ty)
            struct_cpp_defn_target.include(*type_cpp_info.impl_headers)
            struct_cpp_defn_target.writeln(
                f"    {type_cpp_info.as_owner} {field.name};",
            )
        # finally
        struct_cpp_defn_target.writeln(
            f"}};",
            f"}}",
        )

    def gen_struct_same(
        self,
        struct: StructDecl,
        struct_abi_info: StructABIInfo,
        struct_cpp_info: StructCppInfo,
        struct_cpp_defn_target: COutputBuffer,
    ):
        result = "true"
        for field in struct.fields:
            result = f"{result} && same(lhs.{field.name}, rhs.{field.name})"
        struct_cpp_defn_target.writeln(
            f"namespace taihe {{",
            f"inline bool same_impl(adl_helper_t, {struct_cpp_info.as_param} lhs, {struct_cpp_info.as_param} rhs) {{",
            f"    return {result};",
            f"}}",
            f"}}",
        )

    def gen_struct_hash(
        self,
        struct: StructDecl,
        struct_abi_info: StructABIInfo,
        struct_cpp_info: StructCppInfo,
        struct_cpp_defn_target: COutputBuffer,
    ):
        struct_cpp_defn_target.writeln(
            f"namespace taihe {{",
            f"inline auto hash_impl(adl_helper_t, {struct_cpp_info.as_param} val) -> ::std::size_t {{",
            f"    ::std::size_t seed = 0;",
        )
        for field in struct.fields:
            struct_cpp_defn_target.writeln(
                f"    seed ^= hash(val.{field.name}) + 0x9e3779b9 + (seed << 6) + (seed >> 2);",
            )
        struct_cpp_defn_target.writeln(
            f"    return seed;",
            f"}}",
            f"}}",
        )

    def gen_struct_type_traits(
        self,
        struct: StructDecl,
        struct_abi_info: StructABIInfo,
        struct_cpp_info: StructCppInfo,
        struct_cpp_defn_target: COutputBuffer,
    ):
        struct_cpp_defn_target.writeln(
            f"namespace taihe {{",
            f"template<>",
            f"struct as_abi<{struct_cpp_info.as_owner}> {{",
            f"    using type = {struct_abi_info.as_owner};",
            f"}};",
            f"template<>",
            f"struct as_abi<{struct_cpp_info.as_param}> {{",
            f"    using type = {struct_abi_info.as_param};",
            f"}};",
            f"template<>",
            f"struct as_param<{struct_cpp_info.as_owner}> {{",
            f"    using type = {struct_cpp_info.as_param};",
            f"}};",
            f"}}",
        )

    def gen_iface_decl_file(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
    ):
        iface_cpp_decl_target = COutputBuffer.create(
            self.tm, f"include/{iface_cpp_info.decl_header}", True
        )
        iface_cpp_decl_target.writeln(
            f"namespace {iface_cpp_info.weakspace} {{",
            f"struct {iface_cpp_info.weak_name};",
            f"}}",
            f"namespace {iface_cpp_info.namespace} {{",
            f"struct {iface_cpp_info.norm_name};",
            f"}}",
        )

    def gen_iface_defn_file(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
    ):
        iface_cpp_defn_target = COutputBuffer.create(
            self.tm, f"include/{iface_cpp_info.defn_header}", True
        )
        iface_cpp_defn_target.include("taihe/object.hpp")
        iface_cpp_defn_target.include(iface_cpp_info.decl_header)
        iface_cpp_defn_target.include(iface_abi_info.defn_header)
        self.gen_iface_view_defn(
            iface,
            iface_abi_info,
            iface_cpp_info,
            iface_cpp_defn_target,
        )
        self.gen_iface_holder_defn(
            iface,
            iface_abi_info,
            iface_cpp_info,
            iface_cpp_defn_target,
        )
        self.gen_iface_type_traits(
            iface,
            iface_abi_info,
            iface_cpp_info,
            iface_cpp_defn_target,
        )

    def gen_iface_view_defn(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_defn_target: COutputBuffer,
    ):
        iface_cpp_defn_target.writeln(
            f"namespace {iface_cpp_info.weakspace} {{",
            f"struct {iface_cpp_info.weak_name} {{",
            f"    static constexpr bool is_holder = false;",
            f"    {iface_abi_info.as_owner} m_handle;",
            f"    explicit {iface_cpp_info.weak_name}({iface_abi_info.as_param} handle) : m_handle(handle) {{}}",
        )
        self.gen_iface_view_dynamic_cast(
            iface,
            iface_abi_info,
            iface_cpp_info,
            iface_cpp_defn_target,
        )
        self.gen_iface_view_static_cast(
            iface,
            iface_abi_info,
            iface_cpp_info,
            iface_cpp_defn_target,
        )
        self.gen_iface_user_methods_defn(
            iface,
            iface_abi_info,
            iface_cpp_info,
            iface_cpp_defn_target,
        )
        self.gen_iface_impl_methods_defn(
            iface,
            iface_abi_info,
            iface_cpp_info,
            iface_cpp_defn_target,
        )
        self.gen_iface_ftbl(
            iface,
            iface_abi_info,
            iface_cpp_info,
            iface_cpp_defn_target,
        )
        self.gen_iface_vtbl(
            iface,
            iface_abi_info,
            iface_cpp_info,
            iface_cpp_defn_target,
        )
        self.gen_iface_idmap(
            iface,
            iface_abi_info,
            iface_cpp_info,
            iface_cpp_defn_target,
        )
        self.gen_iface_infos(
            iface,
            iface_abi_info,
            iface_cpp_info,
            iface_cpp_defn_target,
        )
        self.gen_iface_utils(
            iface,
            iface_abi_info,
            iface_cpp_info,
            iface_cpp_defn_target,
        )
        iface_cpp_defn_target.writeln(
            f"}};",
            f"}}",
        )

    def gen_iface_view_dynamic_cast(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_defn_target: COutputBuffer,
    ):
        iface_cpp_defn_target.writeln(
            f"    explicit {iface_cpp_info.weak_name}(::taihe::data_view other)",
            f"        : {iface_cpp_info.weak_name}({iface_abi_info.dynamic_cast}(other.data_ptr)) {{}}",
            f"    operator ::taihe::data_view() const& {{",
            f"        {iface_abi_info.as_owner} ret_handle = m_handle;",
            f"        return ::taihe::data_view(ret_handle.data_ptr);",
            f"    }}",
            f"    operator ::taihe::data_holder() const& {{",
            f"        {iface_abi_info.as_owner} ret_handle = {iface_abi_info.copy_func}(m_handle);",
            f"        return ::taihe::data_holder(ret_handle.data_ptr);",
            f"    }}",
        )

    def gen_iface_view_static_cast(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_defn_target: COutputBuffer,
    ):
        for ancestor, info in iface_abi_info.ancestor_dict.items():
            if info.offset == 0:
                continue
            ancestor_cpp_info = IfaceCppInfo.get(self.am, ancestor)
            iface_cpp_defn_target.include(ancestor_cpp_info.defn_header)
            iface_cpp_defn_target.writeln(
                f"    operator {ancestor_cpp_info.full_weak_name}() const& {{",
                f"        {iface_abi_info.as_owner} ret_handle = m_handle;",
                f"        return {ancestor_cpp_info.full_weak_name}({info.static_cast}(ret_handle));",
                f"    }}",
                f"    operator {ancestor_cpp_info.full_norm_name}() const& {{",
                f"        {iface_abi_info.as_owner} ret_handle = {iface_abi_info.copy_func}(m_handle);",
                f"        return {ancestor_cpp_info.full_norm_name}({info.static_cast}(ret_handle));",
                f"    }}",
            )

    def gen_iface_user_methods_defn(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_defn_target: COutputBuffer,
    ):
        iface_cpp_defn_target.writeln(
            f"    struct virtual_type {{",
        )
        for method in iface.methods:
            method_cpp_info = IfaceMethodCppInfo.get(self.am, method)
            params_cpp = []
            for param in method.params:
                type_cpp_info = TypeCppInfo.get(self.am, param.ty_ref.resolved_ty)
                iface_cpp_defn_target.include(*type_cpp_info.decl_headers)
                params_cpp.append(f"{type_cpp_info.as_param} {param.name}")
            params_cpp_str = ", ".join(params_cpp)
            if return_ty_ref := method.return_ty_ref:
                type_cpp_info = TypeCppInfo.get(self.am, return_ty_ref.resolved_ty)
                iface_cpp_defn_target.include(*type_cpp_info.decl_headers)
                cpp_return_ty_name = type_cpp_info.as_owner
            else:
                cpp_return_ty_name = "void"
            iface_cpp_defn_target.writeln(
                f"        {cpp_return_ty_name} {method_cpp_info.call_name}({params_cpp_str}) const&;",
            )
        iface_cpp_defn_target.writeln(
            f"    }};",
        )

    def gen_iface_impl_methods_defn(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_defn_target: COutputBuffer,
    ):
        iface_cpp_defn_target.writeln(
            f"    template<typename Impl>",
            f"    struct methods_impl {{",
        )
        for method in iface.methods:
            params_abi = [f"{iface_abi_info.as_param} tobj"]
            for param in method.params:
                type_abi_info = TypeABIInfo.get(self.am, param.ty_ref.resolved_ty)
                params_abi.append(f"{type_abi_info.as_param} {param.name}")
            params_abi_str = ", ".join(params_abi)
            if return_ty_ref := method.return_ty_ref:
                type_abi_info = TypeABIInfo.get(self.am, return_ty_ref.resolved_ty)
                abi_return_ty_name = type_abi_info.as_owner
            else:
                abi_return_ty_name = "void"
            iface_cpp_defn_target.writeln(
                f"        static {abi_return_ty_name} {method.name}({params_abi_str});",
            )
        iface_cpp_defn_target.writeln(
            f"    }};",
        )

    def gen_iface_ftbl(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_defn_target: COutputBuffer,
    ):
        iface_cpp_defn_target.writeln(
            f"    template<typename Impl>",
            f"    static constexpr {iface_abi_info.ftable} ftbl_impl = {{",
        )
        for method in iface.methods:
            iface_cpp_defn_target.writeln(
                f"        .{method.name} = &methods_impl<Impl>::{method.name},",
            )
        iface_cpp_defn_target.writeln(
            f"    }};",
        )

    def gen_iface_vtbl(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_defn_target: COutputBuffer,
    ):
        iface_cpp_defn_target.writeln(
            f"    template<typename Impl>",
            f"    static constexpr {iface_abi_info.vtable} vtbl_impl = {{",
        )
        for ancestor_info in iface_abi_info.ancestor_list:
            ancestor_cpp_info = IfaceCppInfo.get(self.am, ancestor_info.iface)
            iface_cpp_defn_target.writeln(
                f"        .{ancestor_info.ftbl_ptr} = &{ancestor_cpp_info.full_weak_name}::template ftbl_impl<Impl>,",
            )
        iface_cpp_defn_target.writeln(
            f"    }};",
        )

    def gen_iface_idmap(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_defn_target: COutputBuffer,
    ):
        iface_cpp_defn_target.writeln(
            f"    template<typename Impl>",
            f"    static constexpr IdMapItem idmap_impl[{len(iface_abi_info.ancestor_dict)}] = {{",
        )
        for ancestor, info in iface_abi_info.ancestor_dict.items():
            ancestor_abi_info = IfaceABIInfo.get(self.am, ancestor)
            iface_cpp_defn_target.writeln(
                f"        {{&{ancestor_abi_info.iid}, &vtbl_impl<Impl>.{info.ftbl_ptr}}},",
            )
        iface_cpp_defn_target.writeln(
            f"    }};",
        )

    def gen_iface_infos(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_defn_target: COutputBuffer,
    ):
        iface_cpp_defn_target.writeln(
            f"    static constexpr void const* iid = &{iface_abi_info.iid};",
            f"    using vtable_t = {iface_abi_info.vtable};",
        )

    def gen_iface_utils(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_defn_target: COutputBuffer,
    ):
        iface_cpp_defn_target.writeln(
            f"    explicit operator bool() const& {{ return m_handle.vtbl_ptr; }}",
            f"    virtual_type const& operator*() const& {{",
            f"        return *reinterpret_cast<virtual_type const*>(&m_handle);",
            f"    }}",
            f"    virtual_type const* operator->() const& {{",
            f"        return reinterpret_cast<virtual_type const*>(&m_handle);",
            f"    }}",
        )

    def gen_iface_holder_defn(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_defn_target: COutputBuffer,
    ):
        iface_cpp_defn_target.writeln(
            f"namespace {iface_cpp_info.namespace} {{",
            f"struct {iface_cpp_info.norm_name} : public {iface_cpp_info.full_weak_name} {{",
            f"    static constexpr bool is_holder = true;",
            f"    explicit {iface_cpp_info.norm_name}({iface_abi_info.as_owner} handle) : {iface_cpp_info.full_weak_name}(handle) {{}}",
            f"    {iface_cpp_info.norm_name}& operator=({iface_cpp_info.full_norm_name} other) {{",
            f"        ::std::swap(m_handle, other.m_handle);",
            f"        return *this;",
            f"    }}",
            f"    ~{iface_cpp_info.norm_name}() {{",
            f"        {iface_abi_info.drop_func}(m_handle);",
            f"    }}",
        )
        self.gen_iface_holder_dynamic_cast(
            iface,
            iface_abi_info,
            iface_cpp_info,
            iface_cpp_defn_target,
        )
        self.gen_iface_holder_static_cast(
            iface,
            iface_abi_info,
            iface_cpp_info,
            iface_cpp_defn_target,
        )
        iface_cpp_defn_target.writeln(
            f"}};",
            f"}}",
        )

    def gen_iface_holder_dynamic_cast(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_defn_target: COutputBuffer,
    ):
        iface_cpp_defn_target.writeln(
            f"    explicit {iface_cpp_info.norm_name}(::taihe::data_holder other)",
            f"        : {iface_cpp_info.norm_name}({iface_abi_info.dynamic_cast}(other.data_ptr)) {{",
            f"        other.data_ptr = nullptr;",
            f"    }}",
            f"    operator ::taihe::data_view() const& {{",
            f"        {iface_abi_info.as_owner} ret_handle = m_handle;",
            f"        return ::taihe::data_view(ret_handle.data_ptr);",
            f"    }}",
            f"    operator ::taihe::data_holder() const& {{",
            f"        {iface_abi_info.as_owner} ret_handle = {iface_abi_info.copy_func}(m_handle);",
            f"        return ::taihe::data_holder(ret_handle.data_ptr);",
            f"    }}",
            f"    operator ::taihe::data_holder() && {{",
            f"        {iface_abi_info.as_owner} ret_handle = m_handle;",
            f"        m_handle.data_ptr = nullptr;",
            f"        return ::taihe::data_holder(ret_handle.data_ptr);",
            f"    }}",
        )

    def gen_iface_holder_static_cast(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_defn_target: COutputBuffer,
    ):
        iface_cpp_defn_target.writeln(
            f"    {iface_cpp_info.norm_name}({iface_cpp_info.full_weak_name} const& other)",
            f"        : {iface_cpp_info.norm_name}({iface_abi_info.copy_func}(other.m_handle)) {{}}",
            f"    {iface_cpp_info.norm_name}({iface_cpp_info.full_norm_name} const& other)",
            f"        : {iface_cpp_info.norm_name}({iface_abi_info.copy_func}(other.m_handle)) {{}}",
            f"    {iface_cpp_info.norm_name}({iface_cpp_info.full_norm_name}&& other)",
            f"        : {iface_cpp_info.norm_name}(other.m_handle) {{",
            f"        other.m_handle.data_ptr = nullptr;",
            f"    }}",
        )
        for ancestor, info in iface_abi_info.ancestor_dict.items():
            if info.offset == 0:
                continue
            ancestor_cpp_info = IfaceCppInfo.get(self.am, ancestor)
            iface_cpp_defn_target.writeln(
                f"    operator {ancestor_cpp_info.full_weak_name}() const& {{",
                f"        {iface_abi_info.as_owner} ret_handle = m_handle;",
                f"        return {ancestor_cpp_info.full_weak_name}({info.static_cast}(ret_handle));",
                f"    }}",
                f"    operator {ancestor_cpp_info.full_norm_name}() const& {{",
                f"        {iface_abi_info.as_owner} ret_handle = {iface_abi_info.copy_func}(m_handle);",
                f"        return {ancestor_cpp_info.full_norm_name}({info.static_cast}(ret_handle));",
                f"    }}",
                f"    operator {ancestor_cpp_info.full_norm_name}() && {{",
                f"        {iface_abi_info.as_owner} ret_handle = m_handle;",
                f"        m_handle.data_ptr = nullptr;",
                f"        return {ancestor_cpp_info.full_norm_name}({info.static_cast}(ret_handle));",
                f"    }}",
            )

    def gen_iface_type_traits(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_defn_target: COutputBuffer,
    ):
        iface_cpp_defn_target.writeln(
            f"namespace taihe {{",
            f"template<>",
            f"struct as_abi<{iface_cpp_info.as_owner}> {{",
            f"    using type = {iface_abi_info.as_owner};",
            f"}};",
            f"template<>",
            f"struct as_abi<{iface_cpp_info.as_param}> {{",
            f"    using type = {iface_abi_info.as_param};",
            f"}};",
            f"template<>",
            f"struct as_param<{iface_cpp_info.as_owner}> {{",
            f"    using type = {iface_cpp_info.as_param};",
            f"}};",
            f"}}",
        )

    def gen_iface_impl_file(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
    ):
        iface_cpp_impl_target = COutputBuffer.create(
            self.tm, f"include/{iface_cpp_info.impl_header}", True
        )
        iface_cpp_impl_target.include(iface_cpp_info.defn_header)
        iface_cpp_impl_target.include(iface_abi_info.impl_header)
        for ancestor, info in iface_abi_info.ancestor_dict.items():
            if info.offset == 0:
                continue
            ancestor_cpp_info = IfaceCppInfo.get(self.am, ancestor)
            iface_cpp_impl_target.include(ancestor_cpp_info.impl_header)
        self.gen_iface_user_methods(
            iface,
            iface_abi_info,
            iface_cpp_info,
            iface_cpp_impl_target,
        )
        self.gen_iface_author_methods(
            iface,
            iface_abi_info,
            iface_cpp_info,
            iface_cpp_impl_target,
        )

    def gen_iface_user_methods(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_impl_target: COutputBuffer,
    ):
        for method in iface.methods:
            method_abi_info = IfaceMethodABIInfo.get(self.am, method)
            method_cpp_info = IfaceMethodCppInfo.get(self.am, method)
            params_cpp = []
            args_into_abi = [
                f"*reinterpret_cast<{iface_abi_info.mangled_name} const*>(this)"
            ]
            for param in method.params:
                type_cpp_info = TypeCppInfo.get(self.am, param.ty_ref.resolved_ty)
                iface_cpp_impl_target.include(*type_cpp_info.impl_headers)
                params_cpp.append(f"{type_cpp_info.as_param} {param.name}")
                args_into_abi.append(type_cpp_info.pass_into_abi(param.name))
            params_cpp_str = ", ".join(params_cpp)
            args_into_abi_str = ", ".join(args_into_abi)
            abi_result = f"{method_abi_info.mangled_name}({args_into_abi_str})"
            if return_ty_ref := method.return_ty_ref:
                type_cpp_info = TypeCppInfo.get(self.am, return_ty_ref.resolved_ty)
                iface_cpp_impl_target.include(*type_cpp_info.impl_headers)
                cpp_return_ty_name = type_cpp_info.as_owner
                cpp_result = type_cpp_info.return_from_abi(abi_result)
            else:
                cpp_return_ty_name = "void"
                cpp_result = abi_result
            iface_cpp_impl_target.writeln(
                f"namespace {iface_cpp_info.weakspace} {{",
                f"inline {cpp_return_ty_name} {iface_cpp_info.weak_name}::virtual_type::{method_cpp_info.call_name}({params_cpp_str}) const& {{",
                f"    return {cpp_result};",
                f"}}",
                f"}}",
            )

    def gen_iface_author_methods(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_impl_target: COutputBuffer,
    ):
        for method in iface.methods:
            method_cpp_info = IfaceMethodCppInfo.get(self.am, method)
            params_abi = [f"{iface_abi_info.as_param} tobj"]
            args_from_abi = []
            for param in method.params:
                type_abi_info = TypeABIInfo.get(self.am, param.ty_ref.resolved_ty)
                type_cpp_info = TypeCppInfo.get(self.am, param.ty_ref.resolved_ty)
                params_abi.append(f"{type_abi_info.as_param} {param.name}")
                args_from_abi.append(type_cpp_info.pass_from_abi(param.name))
            params_abi_str = ", ".join(params_abi)
            args_from_abi_str = ", ".join(args_from_abi)
            cpp_result = f"::taihe::cast_data_ptr<Impl>(tobj.data_ptr)->{method_cpp_info.impl_name}({args_from_abi_str})"
            if return_ty_ref := method.return_ty_ref:
                type_abi_info = TypeABIInfo.get(self.am, return_ty_ref.resolved_ty)
                type_cpp_info = TypeCppInfo.get(self.am, return_ty_ref.resolved_ty)
                abi_return_ty_name = type_abi_info.as_owner
                abi_result = type_cpp_info.return_into_abi(cpp_result)
            else:
                abi_return_ty_name = "void"
                abi_result = cpp_result
            iface_cpp_impl_target.writeln(
                f"namespace {iface_cpp_info.weakspace} {{",
                f"template<typename Impl>",
                f"{abi_return_ty_name} {iface_cpp_info.weak_name}::methods_impl<Impl>::{method.name}({params_abi_str}) {{",
                f"    return {abi_result};",
                f"}}",
                f"}}",
            )
