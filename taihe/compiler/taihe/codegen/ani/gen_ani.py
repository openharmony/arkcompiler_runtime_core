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

from taihe.codegen.abi.analyses import (
    IfaceABIInfo,
)
from taihe.codegen.abi.writer import CHeaderWriter, CSourceWriter
from taihe.codegen.ani.analyses import (
    ANI_CLASS,
    ANINativeFuncInfo,
    ANIRegisterInfo,
    GlobFuncANIInfo,
    IfaceANIInfo,
    IfaceMethodANIInfo,
    PackageANIInfo,
    StructANIInfo,
    TypeANIInfo,
    UnionANIInfo,
    UnionFieldANIInfo,
)
from taihe.codegen.cpp.analyses import (
    GlobFuncCppUserInfo,
    IfaceCppInfo,
    IfaceMethodCppInfo,
    PackageCppUserInfo,
    StructCppInfo,
    TypeCppInfo,
    UnionCppInfo,
)
from taihe.semantics.declarations import (
    GlobFuncDecl,
    IfaceDecl,
    IfaceMethodDecl,
    PackageDecl,
    PackageGroup,
    StructDecl,
    UnionDecl,
)
from taihe.semantics.types import (
    Type,
)
from taihe.utils.analyses import AnalysisManager
from taihe.utils.outputs import FileKind, OutputManager


class ANICodeGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager):
        self.om = om
        self.am = am

    def generate(self, pg: PackageGroup):
        for pkg in pg.packages:
            self.gen_package(pkg)
        self.gen_constructor(pg)

    def gen_constructor(self, pg: PackageGroup):
        with CSourceWriter(
            self.om,
            f"temp/ani_constructor.cpp",
            FileKind.TEMPLATE,
        ) as constructor_target:
            constructor_target.writelns(
                f"#if __has_include(<ani.h>)",
                f"#include <ani.h>",
                f"#elif __has_include(<ani/ani.h>)",
                f"#include <ani/ani.h>",
                f"#else",
                f'#error "ani.h not found. Please ensure the Ani SDK is correctly installed."',
                f"#endif",
            )
            with constructor_target.indented(
                f"ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result) {{",
                f"}}",
            ):
                constructor_target.writelns(
                    # f"::taihe::set_vm(vm);",
                    f"ani_env *env;",
                )
                with constructor_target.indented(
                    f"if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {{",
                    f"}}",
                ):
                    constructor_target.writelns(
                        f"return ANI_ERROR;",
                    )
                for pkg in pg.packages:
                    pkg_ani_info = PackageANIInfo.get(self.am, pkg)
                    constructor_target.add_include(pkg_ani_info.header)
                    with constructor_target.indented(
                        f"if (ANI_OK != {pkg_ani_info.cpp_ns}::ANIRegister(env)) {{",
                        f"}}",
                    ):
                        constructor_target.writelns(
                            f'std::cerr << "Error from {pkg_ani_info.cpp_ns}::ANIRegister" << std::endl;',
                            f"return ANI_ERROR;",
                        )
                constructor_target.writelns(
                    f"*result = ANI_VERSION_1;",
                    f"return ANI_OK;",
                )

    def gen_package(self, pkg: PackageDecl):
        for iface in pkg.interfaces:
            self.gen_iface_files(iface)
        for struct in pkg.structs:
            self.gen_struct_files(struct)
        for union in pkg.unions:
            self.gen_union_files(union)
        pkg_ani_info = PackageANIInfo.get(self.am, pkg)
        pkg_cpp_user_info = PackageCppUserInfo.get(self.am, pkg)
        self.gen_package_header(pkg, pkg_ani_info, pkg_cpp_user_info)
        self.gen_package_source(pkg, pkg_ani_info, pkg_cpp_user_info)

    def gen_package_header(
        self,
        pkg: PackageDecl,
        pkg_ani_info: PackageANIInfo,
        pkg_cpp_user_info: PackageCppUserInfo,
    ):
        with CHeaderWriter(
            self.om,
            f"include/{pkg_ani_info.header}",
            FileKind.CPP_HEADER,
        ) as pkg_ani_header_target:
            pkg_ani_header_target.add_include("taihe/platform/ani.hpp")
            with pkg_ani_header_target.indented(
                f"namespace {pkg_ani_info.cpp_ns} {{",
                f"}}",
                indent="",
            ):
                pkg_ani_header_target.writelns(
                    f"ani_status ANIRegister(ani_env *env);",
                )

    def gen_package_source(
        self,
        pkg: PackageDecl,
        pkg_ani_info: PackageANIInfo,
        pkg_cpp_user_info: PackageCppUserInfo,
    ):
        with CSourceWriter(
            self.om,
            f"src/{pkg_ani_info.source}",
            FileKind.CPP_SOURCE,
        ) as pkg_ani_source_target:
            pkg_ani_source_target.add_include(pkg_ani_info.header)
            pkg_ani_source_target.add_include(pkg_cpp_user_info.header)

            # generate functions
            with pkg_ani_source_target.indented(
                f"namespace local {{",
                f"}}",
                indent="",
            ):
                for iface in pkg.interfaces:
                    with pkg_ani_source_target.indented(
                        f"namespace {iface.name} {{",
                        f"}}",
                        indent="",
                    ):
                        iface_abi_info = IfaceABIInfo.get(self.am, iface)
                        for ancestor in iface_abi_info.ancestor_dict:
                            for method in ancestor.methods:
                                self.gen_method(
                                    iface,
                                    method,
                                    pkg_ani_source_target,
                                    ancestor,
                                    method.name,
                                )
                        self.gen_obj_drop(pkg_ani_source_target, "_obj_drop")
                        self.gen_obj_dup(pkg_ani_source_target, "_obj_dup")
                for func in pkg.functions:
                    self.gen_func(func, pkg_ani_source_target, func.name)

            # register infos
            register_infos: list[ANIRegisterInfo] = []

            pkg_register_info = ANIRegisterInfo(
                parent_scope=pkg_ani_info.ns.scope,
                impl_desc=pkg_ani_info.ns.impl_desc,
                member_infos=[],
            )
            register_infos.append(pkg_register_info)
            for func in pkg.functions:
                func_ani_info = GlobFuncANIInfo.get(self.am, func)
                func_info = ANINativeFuncInfo(
                    sts_native_name=func_ani_info.sts_native_name,
                    full_name=f"local::{func.name}",
                )
                pkg_register_info.member_infos.append(func_info)
            for iface in pkg.interfaces:
                iface_abi_info = IfaceABIInfo.get(self.am, iface)
                iface_ani_info = IfaceANIInfo.get(self.am, iface)
                iface_register_info = ANIRegisterInfo(
                    parent_scope=ANI_CLASS,
                    impl_desc=iface_ani_info.impl_desc,
                    member_infos=[],
                )
                register_infos.append(iface_register_info)
                for ancestor in iface_abi_info.ancestor_dict:
                    for method in ancestor.methods:
                        method_ani_info = IfaceMethodANIInfo.get(self.am, method)
                        method_info = ANINativeFuncInfo(
                            sts_native_name=method_ani_info.sts_native_name,
                            full_name=f"local::{iface.name}::{method.name}",
                        )
                        iface_register_info.member_infos.append(method_info)
                pkg_ani_source_target.add_include("taihe/object.hpp")
                obj_drop_info = ANINativeFuncInfo(
                    sts_native_name="_obj_drop",
                    full_name=f"local::{iface.name}::_obj_drop",
                )
                iface_register_info.member_infos.append(obj_drop_info)
                obj_dup_info = ANINativeFuncInfo(
                    sts_native_name="_obj_dup",
                    full_name=f"local::{iface.name}::_obj_dup",
                )
                iface_register_info.member_infos.append(obj_dup_info)

            with pkg_ani_source_target.indented(
                f"namespace {pkg_ani_info.cpp_ns} {{",
                f"}}",
                indent="",
            ):
                with pkg_ani_source_target.indented(
                    f"ani_status ANIRegister(ani_env *env) {{",
                    f"}}",
                ):
                    # TODO: set_vm in constructor
                    with pkg_ani_source_target.indented(
                        f"if (::taihe::get_vm() == nullptr) {{",
                        f"}}",
                    ):
                        pkg_ani_source_target.writelns(
                            f"ani_vm *vm;",
                        )
                        with pkg_ani_source_target.indented(
                            f"if (ANI_OK != env->GetVM(&vm)) {{",
                            f"}}",
                        ):
                            pkg_ani_source_target.writelns(
                                f"return ANI_ERROR;",
                            )
                        pkg_ani_source_target.writelns(
                            f"::taihe::set_vm(vm);",
                        )
                    for register_info in register_infos:
                        parent_scope = register_info.parent_scope
                        with pkg_ani_source_target.indented(
                            f"{{",
                            f"}}",
                        ):
                            pkg_ani_source_target.writelns(
                                f"{parent_scope} ani_env;",
                            )
                            with pkg_ani_source_target.indented(
                                f'if (ANI_OK != env->Find{parent_scope.suffix}("{register_info.impl_desc}", &ani_env)) {{',
                                f"}}",
                            ):
                                pkg_ani_source_target.writelns(
                                    f"return ANI_ERROR;",
                                )
                            with pkg_ani_source_target.indented(
                                f"ani_native_function methods[] = {{",
                                f"}};",
                            ):
                                for member_info in register_info.member_infos:
                                    pkg_ani_source_target.writelns(
                                        f'{{"{member_info.sts_native_name}", nullptr, reinterpret_cast<void*>({member_info.full_name})}},',
                                    )
                            with pkg_ani_source_target.indented(
                                f"if (ANI_OK != env->{parent_scope.suffix}_BindNative{parent_scope.member.suffix}s(ani_env, methods, sizeof(methods) / sizeof(ani_native_function))) {{",
                                f"}}",
                            ):
                                pkg_ani_source_target.writelns(
                                    f"return ANI_ERROR;",
                                )
                    pkg_ani_source_target.writelns(
                        f"return ANI_OK;",
                    )

    def gen_func(
        self,
        func: GlobFuncDecl,
        pkg_ani_source_target: CSourceWriter,
        name: str,
    ):
        func_cpp_user_info = GlobFuncCppUserInfo.get(self.am, func)
        ani_params = []
        ani_params.append("[[maybe_unused]] ani_env *env")
        ani_args = []
        cpp_args = []
        for param in func.params:
            type_ani_info = TypeANIInfo.get(self.am, param.ty_ref.resolved_ty)
            ani_arg = f"ani_arg_{param.name}"
            cpp_arg = f"cpp_arg_{param.name}"
            ani_params.append(f"{type_ani_info.ani_type} {ani_arg}")
            ani_args.append(ani_arg)
            cpp_args.append(cpp_arg)
        ani_params_str = ", ".join(ani_params)
        if return_ty_ref := func.return_ty_ref:
            type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
            ani_return_ty_name = type_ani_info.ani_type
        else:
            ani_return_ty_name = "void"
        with pkg_ani_source_target.indented(
            f"static {ani_return_ty_name} {name}({ani_params_str}) {{",
            f"}}",
        ):
            for param, ani_arg, cpp_arg in zip(
                func.params, ani_args, cpp_args, strict=True
            ):
                type_ani_info = TypeANIInfo.get(self.am, param.ty_ref.resolved_ty)
                type_ani_info.from_ani(pkg_ani_source_target, "env", ani_arg, cpp_arg)
            cpp_args_str = ", ".join(cpp_args)
            if return_ty_ref := func.return_ty_ref:
                type_cpp_info = TypeCppInfo.get(self.am, return_ty_ref.resolved_ty)
                type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
                cpp_return_ty_name = type_cpp_info.as_owner
                cpp_res = "cpp_result"
                ani_res = "ani_result"
                pkg_ani_source_target.writelns(
                    f"{cpp_return_ty_name} {cpp_res} = {func_cpp_user_info.full_name}({cpp_args_str});",
                    f"if (::taihe::has_error()) {{ return {type_ani_info.ani_type}{{}}; }}",
                )
                type_ani_info.into_ani(pkg_ani_source_target, "env", cpp_res, ani_res)
                pkg_ani_source_target.writelns(
                    f"return {ani_res};",
                )
            else:
                pkg_ani_source_target.writelns(
                    f"{func_cpp_user_info.full_name}({cpp_args_str});",
                )

    def gen_method(
        self,
        iface: IfaceDecl,
        method: IfaceMethodDecl,
        pkg_ani_source_target: CSourceWriter,
        ancestor: IfaceDecl,
        name: str,
    ):
        method_cpp_info = IfaceMethodCppInfo.get(self.am, method)
        iface_cpp_info = IfaceCppInfo.get(self.am, iface)
        iface_abi_info = IfaceABIInfo.get(self.am, iface)
        iface_ani_info = IfaceANIInfo.get(self.am, iface)
        ancestor_cpp_info = IfaceCppInfo.get(self.am, ancestor)
        ani_params = []
        ani_params.append("[[maybe_unused]] ani_env *env")
        ani_params.append("[[maybe_unused]] ani_object object")
        ani_args = []
        cpp_args = []
        for param in method.params:
            type_ani_info = TypeANIInfo.get(self.am, param.ty_ref.resolved_ty)
            ani_arg = f"ani_arg_{param.name}"
            cpp_arg = f"cpp_arg_{param.name}"
            ani_params.append(f"{type_ani_info.ani_type} {ani_arg}")
            ani_args.append(ani_arg)
            cpp_args.append(cpp_arg)
        ani_params_str = ", ".join(ani_params)
        if return_ty_ref := method.return_ty_ref:
            type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
            ani_return_ty_name = type_ani_info.ani_type
        else:
            ani_return_ty_name = "void"
        with pkg_ani_source_target.indented(
            f"static {ani_return_ty_name} {name}({ani_params_str}) {{",
            f"}}",
        ):
            pkg_ani_source_target.writelns(
                f"ani_long ani_data_ptr;",
                f'env->Object_GetField_Long(object, TH_ANI_FIND_CLASS_FIELD(env, "{iface_ani_info.impl_desc}", "_data_ptr"), reinterpret_cast<ani_long*>(&ani_data_ptr));',
                f"ani_long ani_vtbl_ptr;",
                f'env->Object_GetField_Long(object, TH_ANI_FIND_CLASS_FIELD(env, "{iface_ani_info.impl_desc}", "_vtbl_ptr"), reinterpret_cast<ani_long*>(&ani_vtbl_ptr));',
                f"DataBlockHead* cpp_data_ptr = reinterpret_cast<DataBlockHead*>(ani_data_ptr);",
                f"{iface_abi_info.vtable}* cpp_vtbl_ptr = reinterpret_cast<{iface_abi_info.vtable}*>(ani_vtbl_ptr);",
                f"{iface_cpp_info.full_weak_name} cpp_iface = {iface_cpp_info.full_weak_name}({{cpp_vtbl_ptr, cpp_data_ptr}});",
            )
            for param, ani_arg, cpp_arg in zip(
                method.params, ani_args, cpp_args, strict=True
            ):
                type_ani_info = TypeANIInfo.get(self.am, param.ty_ref.resolved_ty)
                type_ani_info.from_ani(pkg_ani_source_target, "env", ani_arg, cpp_arg)
            cpp_args_str = ", ".join(cpp_args)
            if return_ty_ref := method.return_ty_ref:
                type_cpp_info = TypeCppInfo.get(self.am, return_ty_ref.resolved_ty)
                type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
                cpp_return_ty_name = type_cpp_info.as_owner
                cpp_res = "cpp_result"
                ani_res = "ani_result"
                pkg_ani_source_target.writelns(
                    f"{cpp_return_ty_name} {cpp_res} = {ancestor_cpp_info.as_param}(cpp_iface)->{method_cpp_info.call_name}({cpp_args_str});",
                    f"if (::taihe::has_error()) {{ return {type_ani_info.ani_type}{{}}; }}",
                )
                type_ani_info.into_ani(pkg_ani_source_target, "env", cpp_res, ani_res)
                pkg_ani_source_target.writelns(
                    f"return {ani_res};",
                )
            else:
                pkg_ani_source_target.writelns(
                    f"{ancestor_cpp_info.as_param}(cpp_iface)->{method_cpp_info.call_name}({cpp_args_str});",
                )

    def gen_obj_drop(
        self,
        pkg_ani_source_target: CSourceWriter,
        name: str,
    ):
        with pkg_ani_source_target.indented(
            f"static void {name}([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_class clazz, ani_long data_ptr) {{",
            f"}}",
        ):
            pkg_ani_source_target.writelns(
                f"tobj_drop(reinterpret_cast<DataBlockHead*>(data_ptr));",
            )

    def gen_obj_dup(
        self,
        pkg_ani_source_target: CSourceWriter,
        name: str,
    ):
        with pkg_ani_source_target.indented(
            f"static ani_long {name}([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_class clazz, ani_long data_ptr) {{",
            f"}}",
        ):
            pkg_ani_source_target.writelns(
                f"return reinterpret_cast<ani_long>(tobj_dup(reinterpret_cast<DataBlockHead*>(data_ptr)));",
            )

    def gen_iface_files(
        self,
        iface: IfaceDecl,
    ):
        iface_abi_info = IfaceABIInfo.get(self.am, iface)
        iface_cpp_info = IfaceCppInfo.get(self.am, iface)
        iface_ani_info = IfaceANIInfo.get(self.am, iface)
        self.gen_iface_conv_decl_file(
            iface,
            iface_abi_info,
            iface_cpp_info,
            iface_ani_info,
        )
        self.gen_iface_conv_impl_file(
            iface,
            iface_abi_info,
            iface_cpp_info,
            iface_ani_info,
        )

    def gen_iface_conv_decl_file(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_ani_info: IfaceANIInfo,
    ):
        with CHeaderWriter(
            self.om,
            f"include/{iface_ani_info.decl_header}",
            FileKind.C_HEADER,
        ) as iface_ani_decl_target:
            iface_ani_decl_target.add_include("taihe/platform/ani.hpp")
            iface_ani_decl_target.add_include(iface_cpp_info.defn_header)
            with iface_ani_decl_target.indented(
                f"template<> struct ::taihe::from_ani_t<{iface_cpp_info.as_owner}> {{",
                f"}};",
            ):
                iface_ani_decl_target.writelns(
                    f"inline {iface_cpp_info.as_owner} operator()(ani_env* env, ani_object ani_obj) const;",
                )
            with iface_ani_decl_target.indented(
                f"template<> struct ::taihe::into_ani_t<{iface_cpp_info.as_owner}> {{",
                f"}};",
            ):
                iface_ani_decl_target.writelns(
                    f"inline ani_object operator()(ani_env* env, {iface_cpp_info.as_owner} cpp_obj) const;",
                )

    def gen_iface_conv_impl_file(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_ani_info: IfaceANIInfo,
    ):
        with CHeaderWriter(
            self.om,
            f"include/{iface_ani_info.impl_header}",
            FileKind.C_HEADER,
        ) as iface_ani_impl_target:
            iface_ani_impl_target.add_include(iface_ani_info.decl_header)
            iface_ani_impl_target.add_include(iface_cpp_info.impl_header)
            self.gen_iface_from_ani_func(
                iface,
                iface_abi_info,
                iface_cpp_info,
                iface_ani_info,
                iface_ani_impl_target,
            )
            self.gen_iface_into_ani_func(
                iface,
                iface_abi_info,
                iface_cpp_info,
                iface_ani_info,
                iface_ani_impl_target,
            )

    def gen_iface_from_ani_func(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_ani_info: IfaceANIInfo,
        iface_ani_impl_target: CHeaderWriter,
    ):
        with iface_ani_impl_target.indented(
            f"inline {iface_cpp_info.as_owner} taihe::from_ani_t<{iface_cpp_info.as_owner}>::operator()(ani_env* env, ani_object ani_obj) const {{",
            f"}}",
        ):
            with iface_ani_impl_target.indented(
                f"struct cpp_impl_t : ::taihe::dref_guard {{",
                f"}};",
            ):
                iface_ani_impl_target.writelns(
                    f"cpp_impl_t(ani_env* env, ani_ref val) : ::taihe::dref_guard(env, val) {{}}",
                )
                for ancestor in iface_abi_info.ancestor_dict:
                    for method in ancestor.methods:
                        method_cpp_info = IfaceMethodCppInfo.get(self.am, method)
                        method_ani_info = IfaceMethodANIInfo.get(self.am, method)
                        inner_cpp_params = []
                        inner_cpp_args = []
                        inner_ani_args = []
                        for param in method.params:
                            inner_cpp_arg = f"cpp_arg_{param.name}"
                            inner_ani_arg = f"ani_arg_{param.name}"
                            type_cpp_info = TypeCppInfo.get(
                                self.am, param.ty_ref.resolved_ty
                            )
                            inner_cpp_params.append(
                                f"{type_cpp_info.as_param} {inner_cpp_arg}"
                            )
                            inner_cpp_args.append(inner_cpp_arg)
                            inner_ani_args.append(inner_ani_arg)
                        inner_cpp_params_str = ", ".join(inner_cpp_params)
                        if return_ty_ref := method.return_ty_ref:
                            type_cpp_info = TypeCppInfo.get(
                                self.am, return_ty_ref.resolved_ty
                            )
                            cpp_return_ty_name = type_cpp_info.as_owner
                        else:
                            cpp_return_ty_name = "void"
                        with iface_ani_impl_target.indented(
                            f"{cpp_return_ty_name} {method_cpp_info.impl_name}({inner_cpp_params_str}) {{",
                            f"}}",
                        ):
                            iface_ani_impl_target.writelns(
                                f"::taihe::env_guard guard;",
                                f"ani_env *env = guard.get_env();",
                            )
                            for param, inner_cpp_arg, inner_ani_arg in zip(
                                method.params,
                                inner_cpp_args,
                                inner_ani_args,
                                strict=True,
                            ):
                                type_ani_info = TypeANIInfo.get(
                                    self.am, param.ty_ref.resolved_ty
                                )
                                type_ani_info.into_ani(
                                    iface_ani_impl_target,
                                    "env",
                                    inner_cpp_arg,
                                    inner_ani_arg,
                                )
                            inner_ani_args_trailing = "".join(
                                ", " + inner_ani_arg for inner_ani_arg in inner_ani_args
                            )
                            if return_ty_ref := method.return_ty_ref:
                                inner_ani_res = "ani_result"
                                inner_cpp_res = "cpp_result"
                                type_ani_info = TypeANIInfo.get(
                                    self.am, return_ty_ref.resolved_ty
                                )
                                iface_ani_impl_target.writelns(
                                    f"{type_ani_info.ani_type} {inner_ani_res};",
                                    f'env->Object_CallMethod_{type_ani_info.ani_type.suffix}(static_cast<ani_object>(this->ref), TH_ANI_FIND_CLASS_METHOD(env, "{iface_ani_info.type_desc}", "{method_ani_info.ani_method_name}", nullptr), reinterpret_cast<{type_ani_info.ani_type.base}*>(&{inner_ani_res}){inner_ani_args_trailing});',
                                )
                                type_ani_info.from_ani(
                                    iface_ani_impl_target,
                                    "env",
                                    inner_ani_res,
                                    inner_cpp_res,
                                )
                                iface_ani_impl_target.writelns(
                                    f"return {inner_cpp_res};",
                                )
                            else:
                                iface_ani_impl_target.writelns(
                                    f'env->Object_CallMethod_Void(static_cast<ani_object>(this->ref), TH_ANI_FIND_CLASS_METHOD(env, "{iface_ani_info.type_desc}", "{method_ani_info.ani_method_name}", nullptr){inner_ani_args_trailing});',
                                )
                with iface_ani_impl_target.indented(
                    f"uintptr_t getGlobalReference() const {{",
                    f"}}",
                ):
                    iface_ani_impl_target.writelns(
                        f"return reinterpret_cast<uintptr_t>(this->ref);",
                    )
            iface_ani_impl_target.writelns(
                f"return ::taihe::make_holder<cpp_impl_t, {iface_cpp_info.as_owner}, ::taihe::platform::ani::AniObject>(env, ani_obj);",
            )

    def gen_iface_into_ani_func(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_ani_info: IfaceANIInfo,
        iface_ani_impl_target: CHeaderWriter,
    ):
        with iface_ani_impl_target.indented(
            f"inline ani_object taihe::into_ani_t<{iface_cpp_info.as_owner}>::operator()(ani_env* env, {iface_cpp_info.as_owner} cpp_obj) const {{",
            f"}}",
        ):
            iface_ani_impl_target.writelns(
                f"ani_long ani_vtbl_ptr = reinterpret_cast<ani_long>(cpp_obj.m_handle.vtbl_ptr);",
                f"ani_long ani_data_ptr = reinterpret_cast<ani_long>(cpp_obj.m_handle.data_ptr);",
                f"cpp_obj.m_handle.data_ptr = nullptr;",
                f"ani_object ani_obj;",
                f'env->Object_New(TH_ANI_FIND_CLASS(env, "{iface_ani_info.impl_desc}"), TH_ANI_FIND_CLASS_METHOD(env, "{iface_ani_info.impl_desc}", "_initialize", "JJ:V"), &ani_obj, ani_vtbl_ptr, ani_data_ptr);',
                f"return ani_obj;",
            )

    def gen_struct_files(
        self,
        struct: StructDecl,
    ):
        struct_cpp_info = StructCppInfo.get(self.am, struct)
        struct_ani_info = StructANIInfo.get(self.am, struct)
        self.gen_struct_conv_decl_file(
            struct,
            struct_cpp_info,
            struct_ani_info,
        )
        self.gen_struct_conv_impl_file(
            struct,
            struct_cpp_info,
            struct_ani_info,
        )

    def gen_struct_conv_decl_file(
        self,
        struct: StructDecl,
        struct_cpp_info: StructCppInfo,
        struct_ani_info: StructANIInfo,
    ):
        with CHeaderWriter(
            self.om,
            f"include/{struct_ani_info.decl_header}",
            FileKind.C_HEADER,
        ) as struct_ani_decl_target:
            struct_ani_decl_target.add_include("taihe/platform/ani.hpp")
            struct_ani_decl_target.add_include(struct_cpp_info.defn_header)
            with struct_ani_decl_target.indented(
                f"template<> struct ::taihe::from_ani_t<{struct_cpp_info.as_owner}> {{",
                f"}};",
            ):
                struct_ani_decl_target.writelns(
                    f"inline {struct_cpp_info.as_owner} operator()(ani_env* env, ani_object ani_obj) const;",
                )
            with struct_ani_decl_target.indented(
                f"template<> struct ::taihe::into_ani_t<{struct_cpp_info.as_owner}> {{",
                f"}};",
            ):
                struct_ani_decl_target.writelns(
                    f"inline ani_object operator()(ani_env* env, {struct_cpp_info.as_owner} cpp_obj) const;",
                )

    def gen_struct_conv_impl_file(
        self,
        struct: StructDecl,
        struct_cpp_info: StructCppInfo,
        struct_ani_info: StructANIInfo,
    ):
        with CHeaderWriter(
            self.om,
            f"include/{struct_ani_info.impl_header}",
            FileKind.C_HEADER,
        ) as struct_ani_impl_target:
            struct_ani_impl_target.add_include(struct_ani_info.decl_header)
            struct_ani_impl_target.add_include(struct_cpp_info.impl_header)
            self.gen_struct_from_ani_func(
                struct,
                struct_cpp_info,
                struct_ani_info,
                struct_ani_impl_target,
            )
            self.gen_struct_into_ani_func(
                struct,
                struct_cpp_info,
                struct_ani_info,
                struct_ani_impl_target,
            )

    def gen_struct_from_ani_func(
        self,
        struct: StructDecl,
        struct_cpp_info: StructCppInfo,
        struct_ani_info: StructANIInfo,
        struct_ani_impl_target: CHeaderWriter,
    ):
        with struct_ani_impl_target.indented(
            f"inline {struct_cpp_info.as_owner} taihe::from_ani_t<{struct_cpp_info.as_owner}>::operator()(ani_env* env, ani_object ani_obj) const {{",
            f"}}",
        ):
            cpp_field_results = []
            for parts in struct_ani_info.sts_final_fields:
                final = parts[-1]
                type_ani_info = TypeANIInfo.get(self.am, final.ty_ref.resolved_ty)
                ani_field_value = f"ani_field_{final.name}"
                cpp_field_result = f"cpp_field_{final.name}"
                struct_ani_impl_target.writelns(
                    f"{type_ani_info.ani_type} {ani_field_value};",
                )
                if struct_ani_info.is_class():
                    struct_ani_impl_target.writelns(
                        f'env->Object_GetField_{type_ani_info.ani_type.suffix}(ani_obj, TH_ANI_FIND_CLASS_FIELD(env, "{struct_ani_info.type_desc}", "{final.name}"), reinterpret_cast<{type_ani_info.ani_type.base}*>(&{ani_field_value}));',
                    )
                else:
                    struct_ani_impl_target.writelns(
                        f'env->Object_CallMethod_{type_ani_info.ani_type.suffix}(ani_obj, TH_ANI_FIND_CLASS_METHOD(env, "{struct_ani_info.type_desc}", "<get>{final.name}", nullptr), reinterpret_cast<{type_ani_info.ani_type.base}*>(&{ani_field_value}));',
                    )
                type_ani_info.from_ani(
                    struct_ani_impl_target, "env", ani_field_value, cpp_field_result
                )
                cpp_field_results.append(cpp_field_result)
            cpp_moved_fields_str = ", ".join(
                f"std::move({cpp_field_result})"
                for cpp_field_result in cpp_field_results
            )
            struct_ani_impl_target.writelns(
                f"return {struct_cpp_info.as_owner}{{{cpp_moved_fields_str}}};",
            )

    def gen_struct_into_ani_func(
        self,
        struct: StructDecl,
        struct_cpp_info: StructCppInfo,
        struct_ani_info: StructANIInfo,
        struct_ani_impl_target: CHeaderWriter,
    ):
        ani_field_results = []
        with struct_ani_impl_target.indented(
            f"inline ani_object taihe::into_ani_t<{struct_cpp_info.as_owner}>::operator()(ani_env* env, {struct_cpp_info.as_owner} cpp_obj) const {{",
            f"}}",
        ):
            for parts in struct_ani_info.sts_final_fields:
                final = parts[-1]
                ani_field_result = f"ani_field_{final.name}"
                type_ani_info = TypeANIInfo.get(self.am, final.ty_ref.resolved_ty)
                type_ani_info.into_ani(
                    struct_ani_impl_target,
                    "env",
                    ".".join(("cpp_obj", *(part.name for part in parts))),
                    ani_field_result,
                )
                ani_field_results.append(ani_field_result)
            ani_field_results_trailing = "".join(
                ", " + ani_field_result for ani_field_result in ani_field_results
            )
            struct_ani_impl_target.writelns(
                f"ani_object ani_obj;",
                f'env->Object_New(TH_ANI_FIND_CLASS(env, "{struct_ani_info.impl_desc}"), TH_ANI_FIND_CLASS_METHOD(env, "{struct_ani_info.impl_desc}", "<ctor>", nullptr), &ani_obj{ani_field_results_trailing});',
                f"return ani_obj;",
            )

    def gen_union_files(
        self,
        union: UnionDecl,
    ):
        union_cpp_info = UnionCppInfo.get(self.am, union)
        union_ani_info = UnionANIInfo.get(self.am, union)
        self.gen_union_conv_decl_file(
            union,
            union_cpp_info,
            union_ani_info,
        )
        self.gen_union_conv_impl_file(
            union,
            union_cpp_info,
            union_ani_info,
        )

    def gen_union_conv_decl_file(
        self,
        union: UnionDecl,
        union_cpp_info: UnionCppInfo,
        union_ani_info: UnionANIInfo,
    ):
        with CHeaderWriter(
            self.om,
            f"include/{union_ani_info.decl_header}",
            FileKind.C_HEADER,
        ) as union_ani_decl_target:
            union_ani_decl_target.add_include("taihe/platform/ani.hpp")
            union_ani_decl_target.add_include(union_cpp_info.defn_header)
            with union_ani_decl_target.indented(
                f"template<> struct ::taihe::from_ani_t<{union_cpp_info.as_owner}> {{",
                f"}};",
            ):
                union_ani_decl_target.writelns(
                    f"inline {union_cpp_info.as_owner} operator()(ani_env* env, ani_ref ani_value) const;",
                )
            with union_ani_decl_target.indented(
                f"template<> struct ::taihe::into_ani_t<{union_cpp_info.as_owner}> {{",
                f"}};",
            ):
                union_ani_decl_target.writelns(
                    f"inline ani_ref operator()(ani_env* env, {union_cpp_info.as_owner} cpp_value) const;",
                )

    def gen_union_conv_impl_file(
        self,
        union: UnionDecl,
        union_cpp_info: UnionCppInfo,
        union_ani_info: UnionANIInfo,
    ):
        with CHeaderWriter(
            self.om,
            f"include/{union_ani_info.impl_header}",
            FileKind.C_HEADER,
        ) as union_ani_impl_target:
            union_ani_impl_target.add_include(union_ani_info.decl_header)
            union_ani_impl_target.add_include(union_cpp_info.impl_header)
            self.gen_union_from_ani_func(
                union,
                union_cpp_info,
                union_ani_info,
                union_ani_impl_target,
            )
            self.gen_union_into_ani_func(
                union,
                union_cpp_info,
                union_ani_info,
                union_ani_impl_target,
            )

    def gen_union_from_ani_func(
        self,
        union: UnionDecl,
        union_cpp_info: UnionCppInfo,
        union_ani_info: UnionANIInfo,
        union_ani_impl_target: CHeaderWriter,
    ):
        with union_ani_impl_target.indented(
            f"inline {union_cpp_info.as_owner} taihe::from_ani_t<{union_cpp_info.as_owner}>::operator()(ani_env* env, ani_ref ani_value) const {{",
            f"}}",
        ):
            for parts in union_ani_info.sts_final_fields:
                final = parts[-1]
                static_tags = []
                for part in parts:
                    path_cpp_info = UnionCppInfo.get(self.am, part.parent_union)
                    static_tags.append(
                        f"::taihe::static_tag<{path_cpp_info.full_name}::tag_t::{part.name}>"
                    )
                static_tags_str = ", ".join(static_tags)
                full_name = "_".join(part.name for part in parts)
                is_field = f"ani_is_{full_name}"
                final_ani_info = UnionFieldANIInfo.get(self.am, final)
                if final_ani_info.field_ty == "null":
                    union_ani_impl_target.writelns(
                        f"ani_boolean {is_field};",
                        f"env->Reference_IsNull(ani_value, &{is_field});",
                    )
                    with union_ani_impl_target.indented(
                        f"if ({is_field}) {{",
                        f"}}",
                    ):
                        union_ani_impl_target.writelns(
                            f"return {union_cpp_info.full_name}({static_tags_str});",
                        )
                if final_ani_info.field_ty == "undefined":
                    union_ani_impl_target.writelns(
                        f"ani_boolean {is_field};",
                        f"env->Reference_IsUndefined(ani_value, &{is_field});",
                    )
                    with union_ani_impl_target.indented(
                        f"if ({is_field}) {{",
                        f"}}",
                    ):
                        union_ani_impl_target.writelns(
                            f"return {union_cpp_info.full_name}({static_tags_str});",
                        )
            for parts in union_ani_info.sts_final_fields:
                final = parts[-1]
                static_tags = []
                for part in parts:
                    path_cpp_info = UnionCppInfo.get(self.am, part.parent_union)
                    static_tags.append(
                        f"::taihe::static_tag<{path_cpp_info.full_name}::tag_t::{part.name}>"
                    )
                static_tags_str = ", ".join(static_tags)
                full_name = "_".join(part.name for part in parts)
                is_field = f"ani_is_{full_name}"
                final_ani_info = UnionFieldANIInfo.get(self.am, final)
                if isinstance(final_ty := final_ani_info.field_ty, Type):
                    type_ani_info = TypeANIInfo.get(self.am, final_ty)
                    union_ani_impl_target.writelns(
                        f"ani_boolean {is_field};",
                        f'env->Object_InstanceOf(static_cast<ani_object>(ani_value), TH_ANI_FIND_CLASS(env, "{type_ani_info.type_desc_boxed}"), &{is_field});',
                    )
                    with union_ani_impl_target.indented(
                        f"if ({is_field}) {{",
                        f"}}",
                    ):
                        cpp_result_spec = f"cpp_field_{full_name}"
                        type_ani_info.from_ani_boxed(
                            union_ani_impl_target,
                            "env",
                            "ani_value",
                            cpp_result_spec,
                        )
                        union_ani_impl_target.writelns(
                            f"return {union_cpp_info.full_name}({static_tags_str}, std::move({cpp_result_spec}));",
                        )
            union_ani_impl_target.writelns(
                f"__builtin_unreachable();",
            )

    def gen_union_into_ani_func(
        self,
        union: UnionDecl,
        union_cpp_info: UnionCppInfo,
        union_ani_info: UnionANIInfo,
        union_ani_impl_target: CHeaderWriter,
    ):
        with union_ani_impl_target.indented(
            f"inline ani_ref taihe::into_ani_t<{union_cpp_info.as_owner}>::operator()(ani_env* env, {union_cpp_info.as_owner} cpp_value) const {{",
            f"}}",
        ):
            union_ani_impl_target.writelns(
                f"ani_ref ani_value;",
            )
            with union_ani_impl_target.indented(
                f"switch (cpp_value.get_tag()) {{",
                f"}}",
                indent="",
            ):
                for field in union.fields:
                    field_ani_info = UnionFieldANIInfo.get(self.am, field)
                    with union_ani_impl_target.indented(
                        f"case {union_cpp_info.full_name}::tag_t::{field.name}: {{",
                        f"}}",
                    ):
                        match field_ani_info.field_ty:
                            case "null":
                                union_ani_impl_target.writelns(
                                    f"env->GetNull(&ani_value);",
                                )
                            case "undefined":
                                union_ani_impl_target.writelns(
                                    f"env->GetUndefined(&ani_value);",
                                )
                            case field_ty if isinstance(field_ty, Type):
                                ani_result_spec = f"ani_field_{field.name}"
                                type_ani_info = TypeANIInfo.get(self.am, field_ty)
                                type_ani_info.into_ani_boxed(
                                    union_ani_impl_target,
                                    "env",
                                    f"cpp_value.get_{field.name}_ref()",
                                    ani_result_spec,
                                )
                                union_ani_impl_target.writelns(
                                    f"ani_value = {ani_result_spec};",
                                )
                        union_ani_impl_target.writelns(
                            f"break;",
                        )
            union_ani_impl_target.writelns(
                f"return ani_value;",
            )