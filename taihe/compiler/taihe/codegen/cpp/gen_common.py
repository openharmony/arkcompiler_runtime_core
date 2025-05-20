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

from json import dumps

from taihe.codegen.abi.analyses import (
    EnumABIInfo,
    IfaceABIInfo,
    IfaceMethodABIInfo,
    StructABIInfo,
    TypeABIInfo,
    UnionABIInfo,
)
from taihe.codegen.abi.writer import CHeaderWriter
from taihe.codegen.cpp.analyses import (
    EnumCppInfo,
    IfaceCppInfo,
    IfaceMethodCppInfo,
    PackageCppInfo,
    StructCppInfo,
    TypeCppInfo,
    UnionCppInfo,
)
from taihe.driver.backend import Backend
from taihe.driver.contexts import CompilerInstance
from taihe.semantics.declarations import (
    EnumDecl,
    IfaceDecl,
    PackageDecl,
    StructDecl,
    UnionDecl,
)
from taihe.semantics.types import (
    ScalarType,
    StringType,
)


class CppHeadersGenerator(Backend):
    def __init__(self, ci: CompilerInstance):
        super().__init__(ci)
        self.oc = ci.output_config
        self.am = ci.analysis_manager
        self.pg = ci.package_group

    def generate(self):
        for pkg in self.pg.packages:
            self.gen_package_files(pkg)

    def gen_package_files(self, pkg: PackageDecl):
        pkg_cpp_info = PackageCppInfo.get(self.am, pkg)
        with CHeaderWriter(
            self.oc,
            f"include/{pkg_cpp_info.header}",
        ) as pkg_cpp_target:
            for enum in pkg.enums:
                enum_abi_info = EnumABIInfo.get(self.am, enum)
                enum_cpp_info = EnumCppInfo.get(self.am, enum)
                self.gen_enum_file(enum, enum_abi_info, enum_cpp_info)
                pkg_cpp_target.add_include(enum_cpp_info.header)
            for struct in pkg.structs:
                struct_abi_info = StructABIInfo.get(self.am, struct)
                struct_cpp_info = StructCppInfo.get(self.am, struct)
                self.gen_struct_decl_file(struct, struct_abi_info, struct_cpp_info)
                self.gen_struct_impl_file(struct, struct_abi_info, struct_cpp_info)
                pkg_cpp_target.add_include(struct_cpp_info.impl_header)
            for union in pkg.unions:
                union_abi_info = UnionABIInfo.get(self.am, union)
                union_cpp_info = UnionCppInfo.get(self.am, union)
                self.gen_union_decl_file(union, union_abi_info, union_cpp_info)
                self.gen_union_impl_file(union, union_abi_info, union_cpp_info)
                pkg_cpp_target.add_include(union_cpp_info.impl_header)
            for iface in pkg.interfaces:
                iface_abi_info = IfaceABIInfo.get(self.am, iface)
                iface_cpp_info = IfaceCppInfo.get(self.am, iface)
                self.gen_iface_decl_file(iface, iface_abi_info, iface_cpp_info)
                self.gen_iface_defn_file(iface, iface_abi_info, iface_cpp_info)
                self.gen_iface_impl_file(iface, iface_abi_info, iface_cpp_info)
                pkg_cpp_target.add_include(iface_cpp_info.impl_header)

    def gen_enum_file(
        self,
        enum: EnumDecl,
        enum_abi_info: EnumABIInfo,
        enum_cpp_info: EnumCppInfo,
    ):
        with CHeaderWriter(
            self.oc,
            f"include/{enum_cpp_info.header}",
        ) as enum_cpp_target:
            enum_cpp_target.add_include("taihe/common.hpp")
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
        enum_cpp_target: CHeaderWriter,
    ):
        with enum_cpp_target.indented(
            f"namespace {enum_cpp_info.namespace} {{",
            f"}}",
            indent="",
        ):
            with enum_cpp_target.indented(
                f"struct {enum_cpp_info.name} {{",
                f"}};",
            ):
                enum_cpp_target.writelns(
                    f"public:",
                )
                self.gen_enum_key_type(
                    enum,
                    enum_abi_info,
                    enum_cpp_info,
                    enum_cpp_target,
                )
                self.gen_enum_basic_methods(
                    enum,
                    enum_abi_info,
                    enum_cpp_info,
                    enum_cpp_target,
                )
                self.gen_enum_key_utils(
                    enum,
                    enum_abi_info,
                    enum_cpp_info,
                    enum_cpp_target,
                )
                self.gen_enum_value_utils(
                    enum,
                    enum_abi_info,
                    enum_cpp_info,
                    enum_cpp_target,
                )
                enum_cpp_target.writelns(
                    f"private:",
                )
                self.gen_enum_properties(
                    enum,
                    enum_abi_info,
                    enum_cpp_info,
                    enum_cpp_target,
                )

    def gen_enum_key_type(
        self,
        enum: EnumDecl,
        enum_abi_info: EnumABIInfo,
        enum_cpp_info: EnumCppInfo,
        enum_cpp_target: CHeaderWriter,
    ):
        with enum_cpp_target.indented(
            f"enum class key_t: {enum_abi_info.abi_type} {{",
            f"}};",
        ):
            for item in enum.items:
                enum_cpp_target.writelns(
                    f"{item.name},",
                )

    def gen_enum_properties(
        self,
        enum: EnumDecl,
        enum_abi_info: EnumABIInfo,
        enum_cpp_info: EnumCppInfo,
        enum_cpp_target: CHeaderWriter,
    ):
        enum_cpp_target.writelns(
            f"key_t key;",
        )

    def gen_enum_basic_methods(
        self,
        enum: EnumDecl,
        enum_abi_info: EnumABIInfo,
        enum_cpp_info: EnumCppInfo,
        enum_cpp_target: CHeaderWriter,
    ):
        # copy constructor
        enum_cpp_target.writelns(
            f"{enum_cpp_info.name}({enum_cpp_info.name} const& other) : key(other.key) {{}}",
        )
        # copy assignment
        with enum_cpp_target.indented(
            f"{enum_cpp_info.name}& operator=({enum_cpp_info.name} other) {{",
            f"}}",
        ):
            enum_cpp_target.writelns(
                f"key = other.key;",
                f"return *this;",
            )

    def gen_enum_key_utils(
        self,
        enum: EnumDecl,
        enum_abi_info: EnumABIInfo,
        enum_cpp_info: EnumCppInfo,
        enum_cpp_target: CHeaderWriter,
    ):
        # constructor
        enum_cpp_target.writelns(
            f"{enum_cpp_info.name}(key_t key) : key(key) {{}}",
        )
        # key getter
        with enum_cpp_target.indented(
            f"key_t get_key() const {{",
            f"}}",
        ):
            enum_cpp_target.writelns(
                f"return this->key;",
            )
        # validity checker
        with enum_cpp_target.indented(
            f"bool is_valid() const {{",
            f"}}",
        ):
            enum_cpp_target.writelns(
                f"return ({enum_abi_info.abi_type})key >= 0 && ({enum_abi_info.abi_type})key < {len(enum.items)};",
            )

    def gen_enum_value_utils(
        self,
        enum: EnumDecl,
        enum_abi_info: EnumABIInfo,
        enum_cpp_info: EnumCppInfo,
        enum_cpp_target: CHeaderWriter,
    ):
        ty_cpp_info = TypeCppInfo.get(self.am, enum.ty_ref.resolved_ty)
        match enum.ty_ref.resolved_ty:
            case StringType():
                as_owner = "char const*"
                as_param = ty_cpp_info.as_param
            case ScalarType():
                as_owner = ty_cpp_info.as_owner
                as_param = ty_cpp_info.as_param
            case _:
                raise ValueError("invalid enum type")
        enum_cpp_target.add_include(*ty_cpp_info.impl_headers)
        # table
        with enum_cpp_target.indented(
            f"static constexpr {as_owner} table[] = {{",
            f"}};",
        ):
            for item in enum.items:
                enum_cpp_target.writelns(
                    f"{dumps(item.value)},",
                )
        # value getter
        with enum_cpp_target.indented(
            f"{as_owner} get_value() const {{",
            f"}}",
        ):
            enum_cpp_target.writelns(
                f"return table[({enum_abi_info.abi_type})key];",
            )
        # value converter
        with enum_cpp_target.indented(
            f"operator {as_owner}() const {{",
            f"}}",
        ):
            enum_cpp_target.writelns(
                f"return table[({enum_abi_info.abi_type})key];",
            )
        # creator from value
        with enum_cpp_target.indented(
            f"static {enum_cpp_info.as_owner} from_value({as_param} value) {{",
            f"}}",
        ):
            with enum_cpp_target.indented(
                f"for (size_t i = 0; i < {len(enum.items)}; ++i) {{",
                f"}}",
            ):
                with enum_cpp_target.indented(
                    f"if (::taihe::same(table[i], value)) {{",
                    f"}}",
                ):
                    enum_cpp_target.writelns(
                        f"return {enum_cpp_info.as_owner}((key_t)i);",
                    )
            enum_cpp_target.writelns(
                f"return {enum_cpp_info.as_owner}((key_t)-1);",
            )

    def gen_enum_same(
        self,
        enum: EnumDecl,
        enum_abi_info: EnumABIInfo,
        enum_cpp_info: EnumCppInfo,
        enum_cpp_target: CHeaderWriter,
    ):
        # others
        with enum_cpp_target.indented(
            f"namespace taihe {{",
            f"}}",
            indent="",
        ):
            with enum_cpp_target.indented(
                f"inline bool same_impl(adl_helper_t, {enum_cpp_info.full_name} lhs, {enum_cpp_info.full_name} rhs) {{",
                f"}}",
            ):
                enum_cpp_target.writelns(
                    f"return lhs.get_key() == rhs.get_key();",
                )

    def gen_enum_hash(
        self,
        enum: EnumDecl,
        enum_abi_info: EnumABIInfo,
        enum_cpp_info: EnumCppInfo,
        enum_cpp_target: CHeaderWriter,
    ):
        with enum_cpp_target.indented(
            f"namespace taihe {{",
            f"}}",
            indent="",
        ):
            with enum_cpp_target.indented(
                f"inline auto hash_impl(adl_helper_t, {enum_cpp_info.as_param} val) -> ::std::size_t {{",
                f"}}",
            ):
                enum_cpp_target.writelns(
                    f"return ::std::hash<{enum_abi_info.abi_type}>{{}}(({enum_abi_info.abi_type})val.get_key());",
                )

    def gen_enum_type_traits(
        self,
        enum: EnumDecl,
        enum_abi_info: EnumABIInfo,
        enum_cpp_info: EnumCppInfo,
        enum_cpp_target: CHeaderWriter,
    ):
        with enum_cpp_target.indented(
            f"namespace taihe {{",
            f"}}",
            indent="",
        ):
            enum_cpp_target.writelns(
                f"template<>",
            )
            with enum_cpp_target.indented(
                f"struct as_abi<{enum_cpp_info.full_name}> {{",
                f"}};",
            ):
                enum_cpp_target.writelns(
                    f"using type = {enum_abi_info.abi_type};",
                )
            enum_cpp_target.writelns(
                f"template<>",
            )
            with enum_cpp_target.indented(
                f"struct as_param<{enum_cpp_info.full_name}> {{",
                f"}};",
            ):
                enum_cpp_target.writelns(
                    f"using type = {enum_cpp_info.full_name};",
                )

    def gen_union_decl_file(
        self,
        union: UnionDecl,
        union_abi_info: UnionABIInfo,
        union_cpp_info: UnionCppInfo,
    ):
        with CHeaderWriter(
            self.oc,
            f"include/{union_cpp_info.decl_header}",
        ) as union_cpp_decl_target:
            with union_cpp_decl_target.indented(
                f"namespace {union_cpp_info.namespace} {{",
                f"}}",
                indent="",
            ):
                union_cpp_decl_target.writelns(
                    f"struct {union_cpp_info.name};",
                )

    def gen_union_impl_file(
        self,
        union: UnionDecl,
        union_abi_info: UnionABIInfo,
        union_cpp_info: UnionCppInfo,
    ):
        with CHeaderWriter(
            self.oc,
            f"include/{union_cpp_info.impl_header}",
        ) as union_cpp_defn_target:
            union_cpp_defn_target.add_include("taihe/common.hpp")
            union_cpp_defn_target.add_include(union_cpp_info.decl_header)
            union_cpp_defn_target.add_include(union_abi_info.impl_header)
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
        union_cpp_defn_target: CHeaderWriter,
    ):
        with union_cpp_defn_target.indented(
            f"namespace {union_cpp_info.namespace} {{",
            f"}}",
            indent="",
        ):
            with union_cpp_defn_target.indented(
                f"struct {union_cpp_info.name} {{",
                f"}};",
            ):
                union_cpp_defn_target.writelns(
                    f"public:",
                )
                self.gen_union_tag_type(
                    union,
                    union_abi_info,
                    union_cpp_info,
                    union_cpp_defn_target,
                )
                self.gen_union_storage_type(
                    union,
                    union_abi_info,
                    union_cpp_info,
                    union_cpp_defn_target,
                )
                self.gen_union_basic_methods(
                    union,
                    union_abi_info,
                    union_cpp_info,
                    union_cpp_defn_target,
                )
                self.gen_union_utils(
                    union,
                    union_abi_info,
                    union_cpp_info,
                    union_cpp_defn_target,
                )
                self.gen_union_named_utils(
                    union,
                    union_abi_info,
                    union_cpp_info,
                    union_cpp_defn_target,
                )
                union_cpp_defn_target.writelns(
                    f"private:",
                )
                self.gen_union_properties(
                    union,
                    union_abi_info,
                    union_cpp_info,
                    union_cpp_defn_target,
                )

    def gen_union_tag_type(
        self,
        union: UnionDecl,
        union_abi_info: UnionABIInfo,
        union_cpp_info: UnionCppInfo,
        union_cpp_defn_target: CHeaderWriter,
    ):
        with union_cpp_defn_target.indented(
            f"enum class tag_t : {union_abi_info.tag_type} {{",
            f"}};",
        ):
            for field in union.fields:
                union_cpp_defn_target.writelns(
                    f"{field.name},",
                )

    def gen_union_storage_type(
        self,
        union: UnionDecl,
        union_abi_info: UnionABIInfo,
        union_cpp_info: UnionCppInfo,
        union_cpp_defn_target: CHeaderWriter,
    ):
        with union_cpp_defn_target.indented(
            f"union storage_t {{",
            f"}};",
        ):
            union_cpp_defn_target.writelns(
                f"storage_t() {{}}",
                f"~storage_t() {{}}",
            )
            for field in union.fields:
                if field.ty_ref is None:
                    continue
                type_cpp_info = TypeCppInfo.get(self.am, field.ty_ref.resolved_ty)
                union_cpp_defn_target.add_include(*type_cpp_info.impl_headers)
                union_cpp_defn_target.writelns(
                    f"{type_cpp_info.as_owner} {field.name};",
                )

    def gen_union_properties(
        self,
        union: UnionDecl,
        union_abi_info: UnionABIInfo,
        union_cpp_info: UnionCppInfo,
        union_cpp_defn_target: CHeaderWriter,
    ):
        union_cpp_defn_target.writelns(
            f"tag_t m_tag;",
            f"storage_t m_data;",
        )

    def gen_union_basic_methods(
        self,
        union: UnionDecl,
        union_abi_info: UnionABIInfo,
        union_cpp_info: UnionCppInfo,
        union_cpp_defn_target: CHeaderWriter,
    ):
        # copy constructor
        with union_cpp_defn_target.indented(
            f"{union_cpp_info.name}({union_cpp_info.name} const& other) : m_tag(other.m_tag) {{",
            f"}}",
        ):
            with union_cpp_defn_target.indented(
                f"switch (m_tag) {{",
                f"}}",
                indent="",
            ):
                for field in union.fields:
                    if field.ty_ref is None:
                        continue
                    with union_cpp_defn_target.indented(
                        f"case tag_t::{field.name}: {{",
                        f"}}",
                    ):
                        union_cpp_defn_target.writelns(
                            f"new (&m_data.{field.name}) decltype(m_data.{field.name})(other.m_data.{field.name});",
                            f"break;",
                        )
                with union_cpp_defn_target.indented(
                    f"default: {{",
                    f"}}",
                ):
                    union_cpp_defn_target.writelns(
                        f"break;",
                    )
        # move constructor
        with union_cpp_defn_target.indented(
            f"{union_cpp_info.name}({union_cpp_info.name}&& other) : m_tag(other.m_tag) {{",
            f"}}",
        ):
            with union_cpp_defn_target.indented(
                f"switch (m_tag) {{",
                f"}}",
                indent="",
            ):
                for field in union.fields:
                    if field.ty_ref is None:
                        continue
                    with union_cpp_defn_target.indented(
                        f"case tag_t::{field.name}: {{",
                        f"}}",
                    ):
                        union_cpp_defn_target.writelns(
                            f"new (&m_data.{field.name}) decltype(m_data.{field.name})(::std::move(other.m_data.{field.name}));",
                            f"break;",
                        )
                with union_cpp_defn_target.indented(
                    f"default: {{",
                    f"}}",
                ):
                    union_cpp_defn_target.writelns(
                        f"break;",
                    )
        # copy assignment
        with union_cpp_defn_target.indented(
            f"{union_cpp_info.name}& operator=({union_cpp_info.name} const& other) {{",
            f"}}",
        ):
            with union_cpp_defn_target.indented(
                f"if (this != &other) {{",
                f"}}",
            ):
                union_cpp_defn_target.writelns(
                    f"::std::destroy_at(this);",
                    f"new (this) {union_cpp_info.name}(other);",
                )
            union_cpp_defn_target.writelns(
                f"return *this;",
            )
        # move assignment
        with union_cpp_defn_target.indented(
            f"{union_cpp_info.name}& operator=({union_cpp_info.name}&& other) {{",
            f"}}",
        ):
            with union_cpp_defn_target.indented(
                f"if (this != &other) {{",
                f"}}",
            ):
                union_cpp_defn_target.writelns(
                    f"::std::destroy_at(this);",
                    f"new (this) {union_cpp_info.name}(::std::move(other));",
                )
            union_cpp_defn_target.writelns(
                f"return *this;",
            )
        # destructor
        with union_cpp_defn_target.indented(
            f"~{union_cpp_info.name}() {{",
            f"}}",
        ):
            with union_cpp_defn_target.indented(
                f"switch (m_tag) {{",
                f"}}",
                indent="",
            ):
                for field in union.fields:
                    if field.ty_ref is None:
                        continue
                    with union_cpp_defn_target.indented(
                        f"case tag_t::{field.name}: {{",
                        f"}}",
                    ):
                        union_cpp_defn_target.writelns(
                            f"::std::destroy_at(&m_data.{field.name});",
                            f"break;",
                        )
                with union_cpp_defn_target.indented(
                    f"default: {{",
                    f"}}",
                ):
                    union_cpp_defn_target.writelns(
                        f"break;",
                    )

    def gen_union_utils(
        self,
        union: UnionDecl,
        union_abi_info: UnionABIInfo,
        union_cpp_info: UnionCppInfo,
        union_cpp_defn_target: CHeaderWriter,
    ):
        # in place constructor
        for field in union.fields:
            if field.ty_ref is None:
                union_cpp_defn_target.writelns(
                    f"{union_cpp_info.name}(::taihe::static_tag_t<tag_t::{field.name}>) : m_tag(tag_t::{field.name}) {{}}",
                )
            else:
                union_cpp_defn_target.writelns(
                    f"template<typename... Args>",
                )
                with union_cpp_defn_target.indented(
                    f"{union_cpp_info.name}(::taihe::static_tag_t<tag_t::{field.name}>, Args&&... args) : m_tag(tag_t::{field.name}) {{",
                    f"}}",
                ):
                    union_cpp_defn_target.writelns(
                        f"new (&m_data.{field.name}) decltype(m_data.{field.name})(::std::forward<Args>(args)...);",
                    )
        # creator
        union_cpp_defn_target.writelns(
            f"template<tag_t tag, typename... Args>",
        )
        with union_cpp_defn_target.indented(
            f"static {union_cpp_info.name} make(Args&&... args) {{",
            f"}}",
        ):
            union_cpp_defn_target.writelns(
                f"return {union_cpp_info.name}(::taihe::static_tag<tag>, ::std::forward<Args>(args)...);",
            )
        # emplacement
        union_cpp_defn_target.writelns(
            f"template<tag_t tag, typename... Args>",
        )
        with union_cpp_defn_target.indented(
            f"{union_cpp_info.name} const& emplace(Args&&... args) {{",
            f"}}",
        ):
            union_cpp_defn_target.writelns(
                f"::std::destroy_at(this);",
                f"new (this) {union_cpp_info.name}(::taihe::static_tag<tag>, ::std::forward<Args>(args)...);",
                f"return *this;",
            )
        # non-const reference getter
        union_cpp_defn_target.writelns(
            f"template<tag_t tag>",
        )
        with union_cpp_defn_target.indented(
            f"auto& get_ref() {{",
            f"}}",
        ):
            for field in union.fields:
                if field.ty_ref is None:
                    continue
                with union_cpp_defn_target.indented(
                    f"if constexpr (tag == tag_t::{field.name}) {{",
                    f"}}",
                ):
                    union_cpp_defn_target.writelns(
                        f"return m_data.{field.name};",
                    )
        # non-const pointer getter
        union_cpp_defn_target.writelns(
            f"template<tag_t tag>",
        )
        with union_cpp_defn_target.indented(
            f"auto* get_ptr() {{",
            f"}}",
        ):
            union_cpp_defn_target.writelns(
                f"return m_tag == tag ? &get_ref<tag>() : nullptr;",
            )
        # const reference getter
        union_cpp_defn_target.writelns(
            f"template<tag_t tag>",
        )
        with union_cpp_defn_target.indented(
            f"auto const& get_ref() const {{",
            f"}}",
        ):
            for field in union.fields:
                if field.ty_ref is None:
                    continue
                with union_cpp_defn_target.indented(
                    f"if constexpr (tag == tag_t::{field.name}) {{",
                    f"}}",
                ):
                    union_cpp_defn_target.writelns(
                        f"return m_data.{field.name};",
                    )
        # const pointer getter
        union_cpp_defn_target.writelns(
            f"template<tag_t tag>",
        )
        with union_cpp_defn_target.indented(
            f"auto const* get_ptr() const {{",
            f"}}",
        ):
            union_cpp_defn_target.writelns(
                f"return m_tag == tag ? &get_ref<tag>() : nullptr;",
            )
        # tag checker
        union_cpp_defn_target.writelns(
            f"template<tag_t tag>",
        )
        with union_cpp_defn_target.indented(
            f"bool holds() const {{",
            f"}}",
        ):
            union_cpp_defn_target.writelns(
                f"return m_tag == tag;",
            )
        # tag getter
        with union_cpp_defn_target.indented(
            f"tag_t get_tag() const {{",
            f"}}",
        ):
            union_cpp_defn_target.writelns(
                f"return m_tag;",
            )
        # non-const visitor
        union_cpp_defn_target.writelns(
            f"template<typename Visitor>",
        )
        with union_cpp_defn_target.indented(
            f"auto accept_template(Visitor&& visitor) {{",
            f"}}",
        ):
            with union_cpp_defn_target.indented(
                f"switch (m_tag) {{",
                f"}}",
                indent="",
            ):
                for field in union.fields:
                    with union_cpp_defn_target.indented(
                        f"case tag_t::{field.name}: {{",
                        f"}}",
                    ):
                        result = f"::taihe::static_tag<tag_t::{field.name}>"
                        if field.ty_ref:
                            result += f", m_data.{field.name}"
                        union_cpp_defn_target.writelns(
                            f"return visitor({result});",
                        )
        # const visitor
        union_cpp_defn_target.writelns(
            f"template<typename Visitor>",
        )
        with union_cpp_defn_target.indented(
            f"auto accept_template(Visitor&& visitor) const {{",
            f"}}",
        ):
            with union_cpp_defn_target.indented(
                f"switch (m_tag) {{",
                f"}}",
                indent="",
            ):
                for field in union.fields:
                    with union_cpp_defn_target.indented(
                        f"case tag_t::{field.name}: {{",
                        f"}}",
                    ):
                        result = f"::taihe::static_tag<tag_t::{field.name}>"
                        if field.ty_ref:
                            result += f", m_data.{field.name}"
                        union_cpp_defn_target.writelns(
                            f"return visitor({result});",
                        )

    def gen_union_named_utils(
        self,
        union: UnionDecl,
        union_abi_info: UnionABIInfo,
        union_cpp_info: UnionCppInfo,
        union_cpp_defn_target: CHeaderWriter,
    ):
        for field in union.fields:
            # creator
            union_cpp_defn_target.writelns(
                f"template<typename... Args>",
            )
            with union_cpp_defn_target.indented(
                f"static {union_cpp_info.name} make_{field.name}(Args&&... args) {{",
                f"}}",
            ):
                union_cpp_defn_target.writelns(
                    f"return make<tag_t::{field.name}>(::std::forward<Args>(args)...);",
                )
            # emplacement
            union_cpp_defn_target.writelns(
                f"template<typename... Args>",
            )
            with union_cpp_defn_target.indented(
                f"{union_cpp_info.name} const& emplace_{field.name}(Args&&... args) {{",
                f"}}",
            ):
                union_cpp_defn_target.writelns(
                    f"return emplace<tag_t::{field.name}>(::std::forward<Args>(args)...);",
                )
            # tag checker
            with union_cpp_defn_target.indented(
                f"bool holds_{field.name}() const {{",
                f"}}",
            ):
                union_cpp_defn_target.writelns(
                    f"return holds<tag_t::{field.name}>();",
                )
            if field.ty_ref is None:
                continue
            # non-const pointer getter
            with union_cpp_defn_target.indented(
                f"auto* get_{field.name}_ptr() {{",
                f"}}",
            ):
                union_cpp_defn_target.writelns(
                    f"return get_ptr<tag_t::{field.name}>();",
                )
            # const pointer getter
            with union_cpp_defn_target.indented(
                f"auto const* get_{field.name}_ptr() const {{",
                f"}}",
            ):
                union_cpp_defn_target.writelns(
                    f"return get_ptr<tag_t::{field.name}>();",
                )
            # non-const reference getter
            with union_cpp_defn_target.indented(
                f"auto& get_{field.name}_ref() {{",
                f"}}",
            ):
                union_cpp_defn_target.writelns(
                    f"return get_ref<tag_t::{field.name}>();",
                )
            # const reference getter
            with union_cpp_defn_target.indented(
                f"auto const& get_{field.name}_ref() const {{",
                f"}}",
            ):
                union_cpp_defn_target.writelns(
                    f"return get_ref<tag_t::{field.name}>();",
                )
        # non-const visitor
        union_cpp_defn_target.writelns(
            f"template<typename Visitor>",
        )
        with union_cpp_defn_target.indented(
            f"auto accept(Visitor&& visitor) {{",
            f"}}",
        ):
            with union_cpp_defn_target.indented(
                f"switch (m_tag) {{",
                f"}}",
                indent="",
            ):
                for field in union.fields:
                    with union_cpp_defn_target.indented(
                        f"case tag_t::{field.name}: {{",
                        f"}}",
                    ):
                        result = "" if field.ty_ref is None else f"m_data.{field.name}"
                        union_cpp_defn_target.writelns(
                            f"return visitor.{field.name}({result});",
                        )
        # const visitor
        union_cpp_defn_target.writelns(
            f"template<typename Visitor>",
        )
        with union_cpp_defn_target.indented(
            f"auto accept(Visitor&& visitor) const {{",
            f"}}",
        ):
            with union_cpp_defn_target.indented(
                f"switch (m_tag) {{",
                f"}}",
                indent="",
            ):
                for field in union.fields:
                    with union_cpp_defn_target.indented(
                        f"case tag_t::{field.name}: {{",
                        f"}}",
                    ):
                        result = "" if field.ty_ref is None else f"m_data.{field.name}"
                        union_cpp_defn_target.writelns(
                            f"return visitor.{field.name}({result});",
                        )

    def gen_union_same(
        self,
        union: UnionDecl,
        union_abi_info: UnionABIInfo,
        union_cpp_info: UnionCppInfo,
        union_cpp_defn_target: CHeaderWriter,
    ):
        with union_cpp_defn_target.indented(
            f"namespace taihe {{",
            f"}}",
            indent="",
        ):
            with union_cpp_defn_target.indented(
                f"inline bool same_impl(adl_helper_t, {union_cpp_info.as_param} lhs, {union_cpp_info.as_param} rhs) {{",
                f"}}",
            ):
                result = "false"
                for field in union.fields:
                    cond = f"lhs.holds_{field.name}() && rhs.holds_{field.name}()"
                    if field.ty_ref:
                        cond = f"{cond} && same(lhs.get_{field.name}_ref(), rhs.get_{field.name}_ref())"
                    result = f"{result} || ({cond})"
                union_cpp_defn_target.writelns(
                    f"return {result};",
                )

    def gen_union_hash(
        self,
        union: UnionDecl,
        union_abi_info: UnionABIInfo,
        union_cpp_info: UnionCppInfo,
        union_cpp_defn_target: CHeaderWriter,
    ):
        with union_cpp_defn_target.indented(
            f"namespace taihe {{",
            f"}}",
            indent="",
        ):
            with union_cpp_defn_target.indented(
                f"inline auto hash_impl(adl_helper_t, {union_cpp_info.as_param} val) -> ::std::size_t {{",
                f"}}",
            ):
                with union_cpp_defn_target.indented(
                    f"switch (val.get_tag()) {{",
                    f"}}",
                    indent="",
                ):
                    for field in union.fields:
                        with union_cpp_defn_target.indented(
                            f"case {union_cpp_info.full_name}::tag_t::{field.name}: {{",
                            f"}}",
                        ):
                            val = "0x9e3779b9 + (seed << 6) + (seed >> 2)"
                            if field.ty_ref:
                                val = f"{val} + hash(val.get_{field.name}_ref())"
                            val = f"seed ^ ({val})"
                            union_cpp_defn_target.writelns(
                                f"::std::size_t seed = (::std::size_t){union_cpp_info.full_name}::tag_t::{field.name};",
                                f"return {val};",
                            )

    def gen_union_type_traits(
        self,
        union: UnionDecl,
        union_abi_info: UnionABIInfo,
        union_cpp_info: UnionCppInfo,
        union_cpp_defn_target: CHeaderWriter,
    ):
        with union_cpp_defn_target.indented(
            f"namespace taihe {{",
            f"}}",
            indent="",
        ):
            union_cpp_defn_target.writelns(
                f"template<>",
            )
            with union_cpp_defn_target.indented(
                f"struct as_abi<{union_cpp_info.as_owner}> {{",
                f"}};",
            ):
                union_cpp_defn_target.writelns(
                    f"using type = {union_abi_info.as_owner};",
                )
            union_cpp_defn_target.writelns(
                f"template<>",
            )
            with union_cpp_defn_target.indented(
                f"struct as_abi<{union_cpp_info.as_param}> {{",
                f"}};",
            ):
                union_cpp_defn_target.writelns(
                    f"using type = {union_abi_info.as_param};",
                )
            union_cpp_defn_target.writelns(
                f"template<>",
            )
            with union_cpp_defn_target.indented(
                f"struct as_param<{union_cpp_info.as_owner}> {{",
                f"}};",
            ):
                union_cpp_defn_target.writelns(
                    f"using type = {union_cpp_info.as_param};",
                )

    def gen_struct_decl_file(
        self,
        struct: StructDecl,
        struct_abi_info: StructABIInfo,
        struct_cpp_info: StructCppInfo,
    ):
        with CHeaderWriter(
            self.oc,
            f"include/{struct_cpp_info.decl_header}",
        ) as struct_cpp_decl_target:
            struct_cpp_decl_target.add_include("taihe/common.hpp")
            with struct_cpp_decl_target.indented(
                f"namespace {struct_cpp_info.namespace} {{",
                f"}}",
                indent="",
            ):
                struct_cpp_decl_target.writelns(
                    f"struct {struct_cpp_info.name};",
                )

    def gen_struct_impl_file(
        self,
        struct: StructDecl,
        struct_abi_info: StructABIInfo,
        struct_cpp_info: StructCppInfo,
    ):
        with CHeaderWriter(
            self.oc,
            f"include/{struct_cpp_info.impl_header}",
        ) as struct_cpp_defn_target:
            struct_cpp_defn_target.add_include("taihe/common.hpp")
            struct_cpp_defn_target.add_include(struct_cpp_info.decl_header)
            struct_cpp_defn_target.add_include(struct_abi_info.impl_header)
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
        struct_cpp_defn_target: CHeaderWriter,
    ):
        with struct_cpp_defn_target.indented(
            f"namespace {struct_cpp_info.namespace} {{",
            f"}}",
            indent="",
        ):
            with struct_cpp_defn_target.indented(
                f"struct {struct_cpp_info.name} {{",
                f"}};",
            ):
                for field in struct.fields:
                    type_cpp_info = TypeCppInfo.get(self.am, field.ty_ref.resolved_ty)
                    struct_cpp_defn_target.add_include(*type_cpp_info.impl_headers)
                    struct_cpp_defn_target.writelns(
                        f"{type_cpp_info.as_owner} {field.name};",
                    )

    def gen_struct_same(
        self,
        struct: StructDecl,
        struct_abi_info: StructABIInfo,
        struct_cpp_info: StructCppInfo,
        struct_cpp_defn_target: CHeaderWriter,
    ):
        with struct_cpp_defn_target.indented(
            f"namespace taihe {{",
            f"}}",
            indent="",
        ):
            with struct_cpp_defn_target.indented(
                f"inline bool same_impl(adl_helper_t, {struct_cpp_info.as_param} lhs, {struct_cpp_info.as_param} rhs) {{",
                f"}}",
            ):
                result = "true"
                for field in struct.fields:
                    result = f"{result} && same(lhs.{field.name}, rhs.{field.name})"
                struct_cpp_defn_target.writelns(
                    f"return {result};",
                )

    def gen_struct_hash(
        self,
        struct: StructDecl,
        struct_abi_info: StructABIInfo,
        struct_cpp_info: StructCppInfo,
        struct_cpp_defn_target: CHeaderWriter,
    ):
        with struct_cpp_defn_target.indented(
            f"namespace taihe {{",
            f"}}",
            indent="",
        ):
            with struct_cpp_defn_target.indented(
                f"inline auto hash_impl(adl_helper_t, {struct_cpp_info.as_param} val) -> ::std::size_t {{",
                f"}}",
            ):
                struct_cpp_defn_target.writelns(
                    f"::std::size_t seed = 0;",
                )
                for field in struct.fields:
                    struct_cpp_defn_target.writelns(
                        f"seed ^= hash(val.{field.name}) + 0x9e3779b9 + (seed << 6) + (seed >> 2);",
                    )
                struct_cpp_defn_target.writelns(
                    f"return seed;",
                )

    def gen_struct_type_traits(
        self,
        struct: StructDecl,
        struct_abi_info: StructABIInfo,
        struct_cpp_info: StructCppInfo,
        struct_cpp_defn_target: CHeaderWriter,
    ):
        with struct_cpp_defn_target.indented(
            f"namespace taihe {{",
            f"}}",
            indent="",
        ):
            struct_cpp_defn_target.writelns(
                f"template<>",
            )
            with struct_cpp_defn_target.indented(
                f"struct as_abi<{struct_cpp_info.as_owner}> {{",
                f"}};",
            ):
                struct_cpp_defn_target.writelns(
                    f"using type = {struct_abi_info.as_owner};",
                )
            struct_cpp_defn_target.writelns(
                f"template<>",
            )
            with struct_cpp_defn_target.indented(
                f"struct as_abi<{struct_cpp_info.as_param}> {{",
                f"}};",
            ):
                struct_cpp_defn_target.writelns(
                    f"using type = {struct_abi_info.as_param};",
                )
            struct_cpp_defn_target.writelns(
                f"template<>",
            )
            with struct_cpp_defn_target.indented(
                f"struct as_param<{struct_cpp_info.as_owner}> {{",
                f"}};",
            ):
                struct_cpp_defn_target.writelns(
                    f"using type = {struct_cpp_info.as_param};",
                )

    def gen_iface_decl_file(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
    ):
        with CHeaderWriter(
            self.oc,
            f"include/{iface_cpp_info.decl_header}",
        ) as iface_cpp_decl_target:
            with iface_cpp_decl_target.indented(
                f"namespace {iface_cpp_info.weakspace} {{",
                f"}}",
                indent="",
            ):
                iface_cpp_decl_target.writelns(
                    f"struct {iface_cpp_info.weak_name};",
                )
            with iface_cpp_decl_target.indented(
                f"namespace {iface_cpp_info.namespace} {{",
                f"}}",
                indent="",
            ):
                iface_cpp_decl_target.writelns(
                    f"struct {iface_cpp_info.norm_name};",
                )

    def gen_iface_defn_file(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
    ):
        with CHeaderWriter(
            self.oc,
            f"include/{iface_cpp_info.defn_header}",
        ) as iface_cpp_defn_target:
            iface_cpp_defn_target.add_include("taihe/object.hpp")
            iface_cpp_defn_target.add_include(iface_cpp_info.decl_header)
            iface_cpp_defn_target.add_include(iface_abi_info.defn_header)
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
        iface_cpp_defn_target: CHeaderWriter,
    ):
        with iface_cpp_defn_target.indented(
            f"namespace {iface_cpp_info.weakspace} {{",
            f"}}",
            indent="",
        ):
            with iface_cpp_defn_target.indented(
                f"struct {iface_cpp_info.weak_name} {{",
                f"}};",
            ):
                iface_cpp_defn_target.writelns(
                    f"static constexpr bool is_holder = false;",
                )
                iface_cpp_defn_target.writelns(
                    f"{iface_abi_info.as_owner} m_handle;",
                )
                iface_cpp_defn_target.writelns(
                    f"explicit {iface_cpp_info.weak_name}({iface_abi_info.as_param} handle) : m_handle(handle) {{}}",
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
                self.gen_iface_rtti(
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

    def gen_iface_view_dynamic_cast(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_defn_target: CHeaderWriter,
    ):
        iface_cpp_defn_target.writelns(
            f"explicit {iface_cpp_info.weak_name}(::taihe::data_view other) : {iface_cpp_info.weak_name}({iface_abi_info.dynamic_cast}(other.data_ptr)) {{}}",
        )
        with iface_cpp_defn_target.indented(
            f"operator ::taihe::data_view() const& {{",
            f"}}",
        ):
            iface_cpp_defn_target.writelns(
                f"{iface_abi_info.as_owner} ret_handle = m_handle;",
                f"return ::taihe::data_view(ret_handle.data_ptr);",
            )
        with iface_cpp_defn_target.indented(
            f"operator ::taihe::data_holder() const& {{",
            f"}}",
        ):
            iface_cpp_defn_target.writelns(
                f"{iface_abi_info.as_owner} ret_handle = {iface_abi_info.copy_func}(m_handle);",
                f"return ::taihe::data_holder(ret_handle.data_ptr);",
            )

    def gen_iface_view_static_cast(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_defn_target: CHeaderWriter,
    ):
        for ancestor, info in iface_abi_info.ancestor_dict.items():
            if info.offset == 0:
                continue
            ancestor_cpp_info = IfaceCppInfo.get(self.am, ancestor)
            iface_cpp_defn_target.add_include(ancestor_cpp_info.defn_header)
            with iface_cpp_defn_target.indented(
                f"operator {ancestor_cpp_info.full_weak_name}() const& {{",
                f"}}",
            ):
                iface_cpp_defn_target.writelns(
                    f"{iface_abi_info.as_owner} ret_handle = m_handle;",
                    f"return {ancestor_cpp_info.full_weak_name}({info.static_cast}(ret_handle));",
                )
            with iface_cpp_defn_target.indented(
                f"operator {ancestor_cpp_info.full_norm_name}() const& {{",
                f"}}",
            ):
                iface_cpp_defn_target.writelns(
                    f"{iface_abi_info.as_owner} ret_handle = {iface_abi_info.copy_func}(m_handle);",
                    f"return {ancestor_cpp_info.full_norm_name}({info.static_cast}(ret_handle));",
                )

    def gen_iface_user_methods_defn(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_defn_target: CHeaderWriter,
    ):
        with iface_cpp_defn_target.indented(
            f"struct virtual_type {{",
            f"}};",
        ):
            for method in iface.methods:
                method_cpp_info = IfaceMethodCppInfo.get(self.am, method)
                params_cpp = []
                for param in method.params:
                    type_cpp_info = TypeCppInfo.get(self.am, param.ty_ref.resolved_ty)
                    iface_cpp_defn_target.add_include(*type_cpp_info.decl_headers)
                    params_cpp.append(f"{type_cpp_info.as_param} {param.name}")
                params_cpp_str = ", ".join(params_cpp)
                if return_ty_ref := method.return_ty_ref:
                    type_cpp_info = TypeCppInfo.get(self.am, return_ty_ref.resolved_ty)
                    iface_cpp_defn_target.add_include(*type_cpp_info.decl_headers)
                    cpp_return_ty_name = type_cpp_info.as_owner
                else:
                    cpp_return_ty_name = "void"
                iface_cpp_defn_target.writelns(
                    f"{cpp_return_ty_name} {method_cpp_info.call_name}({params_cpp_str}) const&;",
                )

    def gen_iface_impl_methods_defn(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_defn_target: CHeaderWriter,
    ):
        iface_cpp_defn_target.writelns(
            f"template<typename Impl>",
        )
        with iface_cpp_defn_target.indented(
            f"struct methods_impl {{",
            f"}};",
        ):
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
                iface_cpp_defn_target.writelns(
                    f"static {abi_return_ty_name} {method.name}({params_abi_str});",
                )

    def gen_iface_ftbl(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_defn_target: CHeaderWriter,
    ):
        iface_cpp_defn_target.writelns(
            f"template<typename Impl>",
        )
        with iface_cpp_defn_target.indented(
            f"static constexpr {iface_abi_info.ftable} ftbl_impl = {{",
            f"}};",
        ):
            for method in iface.methods:
                iface_cpp_defn_target.writelns(
                    f".{method.name} = &methods_impl<Impl>::{method.name},",
                )

    def gen_iface_vtbl(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_defn_target: CHeaderWriter,
    ):
        iface_cpp_defn_target.writelns(
            f"template<typename Impl>",
        )
        with iface_cpp_defn_target.indented(
            f"static constexpr {iface_abi_info.vtable} vtbl_impl = {{",
            f"}};",
        ):
            for ancestor_info in iface_abi_info.ancestor_list:
                ancestor_cpp_info = IfaceCppInfo.get(self.am, ancestor_info.iface)
                iface_cpp_defn_target.writelns(
                    f".{ancestor_info.ftbl_ptr} = &{ancestor_cpp_info.full_weak_name}::template ftbl_impl<Impl>,",
                )

    def gen_iface_rtti(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_defn_target: CHeaderWriter,
    ):
        with iface_cpp_defn_target.indented(
            f"struct rtti_type {{",
            f"}};",
        ):
            iface_cpp_defn_target.writelns(
                f"uint64_t version;",
                f"void (*free)(struct DataBlockHead *);",
                f"uint64_t len;",
                f"struct IdMapItem idmap[{len(iface_abi_info.ancestor_dict)}];",
            )
        iface_cpp_defn_target.writelns(
            f"template<typename Impl>",
        )
        with iface_cpp_defn_target.indented(
            f"static constexpr rtti_type rtti_impl = {{",
            f"}};",
        ):
            iface_cpp_defn_target.writelns(
                f".version = 0,",
                f".free = &::taihe::del_data_ptr<Impl>,",
                f".len = {len(iface_abi_info.ancestor_dict)},",
            )
            with iface_cpp_defn_target.indented(
                f".idmap = {{",
                f"}},",
            ):
                for ancestor, info in iface_abi_info.ancestor_dict.items():
                    ancestor_abi_info = IfaceABIInfo.get(self.am, ancestor)
                    iface_cpp_defn_target.writelns(
                        f"{{&{ancestor_abi_info.iid}, &vtbl_impl<Impl>.{info.ftbl_ptr}}},",
                    )

    def gen_iface_infos(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_defn_target: CHeaderWriter,
    ):
        iface_cpp_defn_target.writelns(
            f"static constexpr void const* iid = &{iface_abi_info.iid};",
            f"using vtable_t = {iface_abi_info.vtable};",
        )

    def gen_iface_utils(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_defn_target: CHeaderWriter,
    ):
        iface_cpp_defn_target.writelns(
            f"explicit operator bool() const& {{ return m_handle.vtbl_ptr; }}",
        )
        with iface_cpp_defn_target.indented(
            f"virtual_type const& operator*() const& {{",
            f"}}",
        ):
            iface_cpp_defn_target.writelns(
                f"return *reinterpret_cast<virtual_type const*>(&m_handle);",
            )
        with iface_cpp_defn_target.indented(
            f"virtual_type const* operator->() const& {{",
            f"}}",
        ):
            iface_cpp_defn_target.writelns(
                f"return reinterpret_cast<virtual_type const*>(&m_handle);",
            )

    def gen_iface_holder_defn(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_defn_target: CHeaderWriter,
    ):
        with iface_cpp_defn_target.indented(
            f"namespace {iface_cpp_info.namespace} {{",
            f"}}",
            indent="",
        ):
            with iface_cpp_defn_target.indented(
                f"struct {iface_cpp_info.norm_name} : public {iface_cpp_info.full_weak_name} {{",
                f"}};",
            ):
                iface_cpp_defn_target.writelns(
                    f"static constexpr bool is_holder = true;",
                )
                iface_cpp_defn_target.writelns(
                    f"explicit {iface_cpp_info.norm_name}({iface_abi_info.as_owner} handle) : {iface_cpp_info.full_weak_name}(handle) {{}}",
                )
                with iface_cpp_defn_target.indented(
                    f"{iface_cpp_info.norm_name}& operator=({iface_cpp_info.full_norm_name} other) {{",
                    f"}}",
                ):
                    iface_cpp_defn_target.writelns(
                        f"::std::swap(m_handle, other.m_handle);",
                        f"return *this;",
                    )
                with iface_cpp_defn_target.indented(
                    f"~{iface_cpp_info.norm_name}() {{",
                    f"}}",
                ):
                    iface_cpp_defn_target.writelns(
                        f"{iface_abi_info.drop_func}(m_handle);",
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
                self.gen_iface_from_impl(
                    iface,
                    iface_abi_info,
                    iface_cpp_info,
                    iface_cpp_defn_target,
                )

    def gen_iface_holder_dynamic_cast(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_defn_target: CHeaderWriter,
    ):
        with iface_cpp_defn_target.indented(
            f"explicit {iface_cpp_info.norm_name}(::taihe::data_holder other) : {iface_cpp_info.norm_name}({iface_abi_info.dynamic_cast}(other.data_ptr)) {{",
            f"}}",
        ):
            iface_cpp_defn_target.writelns(
                f"other.data_ptr = nullptr;",
            )
        with iface_cpp_defn_target.indented(
            f"operator ::taihe::data_view() const& {{",
            f"}}",
        ):
            iface_cpp_defn_target.writelns(
                f"{iface_abi_info.as_owner} ret_handle = m_handle;",
                f"return ::taihe::data_view(ret_handle.data_ptr);",
            )
        with iface_cpp_defn_target.indented(
            f"operator ::taihe::data_holder() const& {{",
            f"}}",
        ):
            iface_cpp_defn_target.writelns(
                f"{iface_abi_info.as_owner} ret_handle = {iface_abi_info.copy_func}(m_handle);",
                f"return ::taihe::data_holder(ret_handle.data_ptr);",
            )
        with iface_cpp_defn_target.indented(
            f"operator ::taihe::data_holder() && {{",
            f"}}",
        ):
            iface_cpp_defn_target.writelns(
                f"{iface_abi_info.as_owner} ret_handle = m_handle;",
                f"m_handle.data_ptr = nullptr;",
                f"return ::taihe::data_holder(ret_handle.data_ptr);",
            )

    def gen_iface_holder_static_cast(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_defn_target: CHeaderWriter,
    ):
        iface_cpp_defn_target.writelns(
            f"{iface_cpp_info.norm_name}({iface_cpp_info.full_weak_name} const& other) : {iface_cpp_info.norm_name}({iface_abi_info.copy_func}(other.m_handle)) {{}}",
            f"{iface_cpp_info.norm_name}({iface_cpp_info.full_norm_name} const& other) : {iface_cpp_info.norm_name}({iface_abi_info.copy_func}(other.m_handle)) {{}}",
        )
        with iface_cpp_defn_target.indented(
            f"{iface_cpp_info.norm_name}({iface_cpp_info.full_norm_name}&& other) : {iface_cpp_info.norm_name}(other.m_handle) {{",
            f"}}",
        ):
            iface_cpp_defn_target.writelns(
                f"other.m_handle.data_ptr = nullptr;",
            )
        for ancestor, info in iface_abi_info.ancestor_dict.items():
            if info.offset == 0:
                continue
            ancestor_cpp_info = IfaceCppInfo.get(self.am, ancestor)
            with iface_cpp_defn_target.indented(
                f"operator {ancestor_cpp_info.full_weak_name}() const& {{",
                f"}}",
            ):
                iface_cpp_defn_target.writelns(
                    f"{iface_abi_info.as_owner} ret_handle = m_handle;",
                    f"return {ancestor_cpp_info.full_weak_name}({info.static_cast}(ret_handle));",
                )
            with iface_cpp_defn_target.indented(
                f"operator {ancestor_cpp_info.full_norm_name}() const& {{",
                f"}}",
            ):
                iface_cpp_defn_target.writelns(
                    f"{iface_abi_info.as_owner} ret_handle = {iface_abi_info.copy_func}(m_handle);",
                    f"return {ancestor_cpp_info.full_norm_name}({info.static_cast}(ret_handle));",
                )
            with iface_cpp_defn_target.indented(
                f"operator {ancestor_cpp_info.full_norm_name}() && {{",
                f"}}",
            ):
                iface_cpp_defn_target.writelns(
                    f"{iface_abi_info.as_owner} ret_handle = m_handle;",
                    f"m_handle.data_ptr = nullptr;",
                    f"return {ancestor_cpp_info.full_norm_name}({info.static_cast}(ret_handle));",
                )

    def gen_iface_from_impl(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_defn_target: CHeaderWriter,
    ):
        iface_cpp_defn_target.writelns(
            f"template<typename Impl, typename... Args>",
        )
        with iface_cpp_defn_target.indented(
            f"static {iface_cpp_info.as_owner} from(Args&&... args) {{",
            f"}}",
        ):
            with iface_cpp_defn_target.indented(
                f"return {iface_cpp_info.as_owner}{{{{",
                f"}}}};",
            ):
                iface_cpp_defn_target.writelns(
                    f".vtbl_ptr = &vtbl_impl<Impl>,",
                    f".data_ptr = ::taihe::new_data_ptr<Impl>(reinterpret_cast<TypeInfo const*>(&rtti_impl<Impl>), std::forward<Args>(args)...),",
                )
        iface_cpp_defn_target.writelns(
            "template<typename Impl>",
        )
        with iface_cpp_defn_target.indented(
            f"static {iface_cpp_info.as_owner} from(Impl&& impl) {{",
            f"}}",
        ):
            with iface_cpp_defn_target.indented(
                f"return {iface_cpp_info.as_owner}{{{{",
                f"}}}};",
            ):
                iface_cpp_defn_target.writelns(
                    f".vtbl_ptr = &vtbl_impl<Impl>,",
                    f".data_ptr = ::taihe::new_data_ptr<Impl>(reinterpret_cast<TypeInfo const*>(&rtti_impl<Impl>), std::forward<Impl>(impl)),",
                )

    def gen_iface_type_traits(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_defn_target: CHeaderWriter,
    ):
        with iface_cpp_defn_target.indented(
            f"namespace taihe {{",
            f"}}",
            indent="",
        ):
            iface_cpp_defn_target.writelns(
                f"template<>",
            )
            with iface_cpp_defn_target.indented(
                f"struct as_abi<{iface_cpp_info.as_owner}> {{",
                f"}};",
            ):
                iface_cpp_defn_target.writelns(
                    f"using type = {iface_abi_info.as_owner};",
                )
            iface_cpp_defn_target.writelns(
                f"template<>",
            )
            with iface_cpp_defn_target.indented(
                f"struct as_abi<{iface_cpp_info.as_param}> {{",
                f"}};",
            ):
                iface_cpp_defn_target.writelns(
                    f"using type = {iface_abi_info.as_param};",
                )
            iface_cpp_defn_target.writelns(
                f"template<>",
            )
            with iface_cpp_defn_target.indented(
                f"struct as_param<{iface_cpp_info.as_owner}> {{",
                f"}};",
            ):
                iface_cpp_defn_target.writelns(
                    f"using type = {iface_cpp_info.as_param};",
                )

    def gen_iface_impl_file(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
    ):
        with CHeaderWriter(
            self.oc,
            f"include/{iface_cpp_info.impl_header}",
        ) as iface_cpp_impl_target:
            iface_cpp_impl_target.add_include(iface_cpp_info.defn_header)
            iface_cpp_impl_target.add_include(iface_abi_info.impl_header)
            for ancestor, info in iface_abi_info.ancestor_dict.items():
                if info.offset == 0:
                    continue
                ancestor_cpp_info = IfaceCppInfo.get(self.am, ancestor)
                iface_cpp_impl_target.add_include(ancestor_cpp_info.impl_header)
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
        iface_cpp_impl_target: CHeaderWriter,
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
                iface_cpp_impl_target.add_include(*type_cpp_info.impl_headers)
                params_cpp.append(f"{type_cpp_info.as_param} {param.name}")
                args_into_abi.append(type_cpp_info.pass_into_abi(param.name))
            params_cpp_str = ", ".join(params_cpp)
            args_into_abi_str = ", ".join(args_into_abi)
            abi_result = f"{method_abi_info.mangled_name}({args_into_abi_str})"
            if return_ty_ref := method.return_ty_ref:
                type_cpp_info = TypeCppInfo.get(self.am, return_ty_ref.resolved_ty)
                iface_cpp_impl_target.add_include(*type_cpp_info.impl_headers)
                cpp_return_ty_name = type_cpp_info.as_owner
                cpp_result = type_cpp_info.return_from_abi(abi_result)
            else:
                cpp_return_ty_name = "void"
                cpp_result = abi_result
            with iface_cpp_impl_target.indented(
                f"namespace {iface_cpp_info.weakspace} {{",
                f"}}",
                indent="",
            ):
                with iface_cpp_impl_target.indented(
                    f"inline {cpp_return_ty_name} {iface_cpp_info.weak_name}::virtual_type::{method_cpp_info.call_name}({params_cpp_str}) const& {{",
                    f"}}",
                ):
                    iface_cpp_impl_target.writelns(
                        f"return {cpp_result};",
                    )

    def gen_iface_author_methods(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_cpp_impl_target: CHeaderWriter,
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
            with iface_cpp_impl_target.indented(
                f"namespace {iface_cpp_info.weakspace} {{",
                f"}}",
                indent="",
            ):
                iface_cpp_impl_target.writelns(
                    f"template<typename Impl>",
                )
                with iface_cpp_impl_target.indented(
                    f"{abi_return_ty_name} {iface_cpp_info.weak_name}::methods_impl<Impl>::{method.name}({params_abi_str}) {{",
                    f"}}",
                ):
                    iface_cpp_impl_target.writelns(
                        f"return {abi_result};",
                    )