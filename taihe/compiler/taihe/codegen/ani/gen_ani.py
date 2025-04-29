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
from taihe.codegen.abi.mangle import DeclKind, encode
from taihe.codegen.ani.analyses import (
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
from taihe.utils.outputs import COutputBuffer, OutputManager


class ANICodeGenerator:
    def __init__(self, tm: OutputManager, am: AnalysisManager):
        self.tm = tm
        self.am = am

    def generate(self, pg: PackageGroup):
        for pkg in pg.packages:
            self.gen_package(pkg)
        self.gen_constructor(pg)

    def gen_constructor(self, pg: PackageGroup):
        constructor_file = "ani_constructor.cpp"
        constructor_target = COutputBuffer.create(
            self.tm, f"src/{constructor_file}", False
        )
        constructor_target.include("taihe/runtime.hpp")
        constructor_target.writeln(
            f"ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result) {{",
            # f"    ::taihe::set_vm(vm);",
            f"    ani_env *env;",
            f"    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {{",
            f"        return ANI_ERROR;",
            f"    }}",
        )
        for pkg in pg.packages:
            pkg_ani_info = PackageANIInfo.get(self.am, pkg)
            constructor_target.include(pkg_ani_info.header)
            constructor_target.writeln(
                f"    if (ANI_OK != {pkg_ani_info.cpp_ns}::ANIRegister(env)) {{",
                f'        std::cerr << "Error from {pkg_ani_info.cpp_ns}::ANIRegister" << std::endl;',
                f"        return ANI_ERROR;",
                f"    }}",
            )
        constructor_target.writeln(
            f"    *result = ANI_VERSION_1;",
            f"    return ANI_OK;",
            f"}}",
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
        pkg_ani_header_target = COutputBuffer.create(
            self.tm, f"include/{pkg_ani_info.header}", True
        )
        pkg_ani_header_target.include("taihe/runtime.hpp")
        pkg_ani_header_target.writeln(
            f"namespace {pkg_ani_info.cpp_ns} {{",
            f"ani_status ANIRegister(ani_env *env);",
            f"}}",
        )

    def gen_package_source(
        self,
        pkg: PackageDecl,
        pkg_ani_info: PackageANIInfo,
        pkg_cpp_user_info: PackageCppUserInfo,
    ):
        pkg_ani_source_target = COutputBuffer.create(
            self.tm, f"src/{pkg_ani_info.source}", False
        )
        pkg_ani_source_target.include(pkg_ani_info.header)
        pkg_ani_source_target.include(pkg_cpp_user_info.header)

        # generate functions
        for func in pkg.functions:
            segments = [*pkg.segments, func.name]
            mangled_name = encode(segments, DeclKind.ANI_FUNC)
            self.gen_func(func, pkg_ani_source_target, mangled_name)
        for iface in pkg.interfaces:
            iface_abi_info = IfaceABIInfo.get(self.am, iface)
            for ancestor in iface_abi_info.ancestor_dict:
                for method in ancestor.methods:
                    segments = [*iface.parent_pkg.segments, iface.name, method.name]
                    mangled_name = encode(segments, DeclKind.ANI_FUNC)
                    self.gen_method(
                        iface, method, pkg_ani_source_target, ancestor, mangled_name
                    )
            # TODO: finalizer
            pkg_ani_source_target.include("taihe/object.hpp")
            segments = [*iface.parent_pkg.segments, iface.name, "static_finalize"]
            mangled_name = encode(segments, DeclKind.ANI_FUNC)
            self.gen_finalizer(pkg_ani_source_target, mangled_name)

        # register infos
        register_infos: list[ANIRegisterInfo] = []

        pkg_register_info = ANIRegisterInfo(
            impl_desc=pkg_ani_info.impl_desc,
            member_infos=[],
            parent_scope=pkg_ani_info.scope,
        )
        register_infos.append(pkg_register_info)
        for func in pkg.functions:
            segments = [*pkg.segments, func.name]
            mangled_name = encode(segments, DeclKind.ANI_FUNC)
            func_ani_info = GlobFuncANIInfo.get(self.am, func)
            func_info = ANINativeFuncInfo(
                sts_native_name=func_ani_info.sts_native_name,
                mangled_name=mangled_name,
            )
            pkg_register_info.member_infos.append(func_info)
        for iface in pkg.interfaces:
            iface_abi_info = IfaceABIInfo.get(self.am, iface)
            iface_ani_info = IfaceANIInfo.get(self.am, iface)
            iface_register_info = ANIRegisterInfo(
                impl_desc=iface_ani_info.impl_desc,
                member_infos=[],
                parent_scope=iface_ani_info.scope,
            )
            register_infos.append(iface_register_info)
            for ancestor in iface_abi_info.ancestor_dict:
                for method in ancestor.methods:
                    segments = [*iface.parent_pkg.segments, iface.name, method.name]
                    mangled_name = encode(segments, DeclKind.ANI_FUNC)
                    method_ani_info = IfaceMethodANIInfo.get(self.am, method)
                    method_info = ANINativeFuncInfo(
                        sts_native_name=method_ani_info.sts_native_name,
                        mangled_name=mangled_name,
                    )
                    iface_register_info.member_infos.append(method_info)
            # TODO: finalizer
            pkg_ani_source_target.include("taihe/object.hpp")
            segments = [*iface.parent_pkg.segments, iface.name, "static_finalize"]
            mangled_name = encode(segments, DeclKind.ANI_FUNC)
            finalizer_info = ANINativeFuncInfo(
                sts_native_name="_finalize",
                mangled_name=mangled_name,
            )
            iface_register_info.member_infos.append(finalizer_info)

        pkg_ani_source_target.writeln(
            f"namespace {pkg_ani_info.cpp_ns} {{",
            f"ani_status ANIRegister(ani_env *env) {{",
        )
        # TODO: set_vm in constructor
        pkg_ani_source_target.writeln(
            f"    if (::taihe::get_vm() == nullptr) {{",
            f"        ani_vm *vm;",
            f"        if (ANI_OK != env->GetVM(&vm)) {{",
            f"            return ANI_ERROR;",
            f"        }}",
            f"        ::taihe::set_vm(vm);",
            f"    }}",
        )
        for register_info in register_infos:
            parent_scope = register_info.parent_scope
            pkg_ani_source_target.writeln(
                f"    {{",
                f"        {parent_scope} ani_env;",
                f'        if (ANI_OK != env->Find{parent_scope.suffix}("{register_info.impl_desc}", &ani_env)) {{',
                f"            return ANI_ERROR;",
                f"        }}",
                f"        ani_native_function methods[] = {{",
            )
            for member_info in register_info.member_infos:
                pkg_ani_source_target.writeln(
                    f'            {{"{member_info.sts_native_name}", nullptr, reinterpret_cast<void*>({member_info.mangled_name})}},',
                )
            pkg_ani_source_target.writeln(
                f"        }};",
                f"        if (ANI_OK != env->{parent_scope.suffix}_BindNative{parent_scope.member.suffix}s(ani_env, methods, sizeof(methods) / sizeof(ani_native_function))) {{",
                f"            return ANI_ERROR;",
                f"        }}",
                f"    }}",
            )
        pkg_ani_source_target.writeln(
            f"    return ANI_OK;",
            f"}}",
            f"}}",
        )

    def gen_finalizer(
        self,
        pkg_ani_source_target: COutputBuffer,
        mangled_name: str,
    ):
        pkg_ani_source_target.writeln(
            f"static void {mangled_name}([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_class clazz, ani_long data_ptr) {{",
            f"    ::taihe::data_holder(reinterpret_cast<DataBlockHead*>(data_ptr));",
            f"}}",
        )

    def gen_func(
        self,
        func: GlobFuncDecl,
        pkg_ani_source_target: COutputBuffer,
        mangled_name: str,
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
        pkg_ani_source_target.writeln(
            f"static {ani_return_ty_name} {mangled_name}({ani_params_str}) {{",
        )
        for param, ani_arg, cpp_arg in zip(
            func.params, ani_args, cpp_args, strict=True
        ):
            type_ani_info = TypeANIInfo.get(self.am, param.ty_ref.resolved_ty)
            type_ani_info.from_ani(pkg_ani_source_target, 4, "env", ani_arg, cpp_arg)
        cpp_args_str = ", ".join(cpp_args)
        if return_ty_ref := func.return_ty_ref:
            type_cpp_info = TypeCppInfo.get(self.am, return_ty_ref.resolved_ty)
            type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
            cpp_return_ty_name = type_cpp_info.as_owner
            cpp_res = "cpp_result"
            ani_res = "ani_result"
            pkg_ani_source_target.writeln(
                f"    {cpp_return_ty_name} {cpp_res} = {func_cpp_user_info.full_name}({cpp_args_str});",
                f"    if (::taihe::has_error()) {{ return {type_ani_info.ani_type}{{}}; }}",
            )
            type_ani_info.into_ani(pkg_ani_source_target, 4, "env", cpp_res, ani_res)
            pkg_ani_source_target.writeln(
                f"    return {ani_res};",
            )
        else:
            pkg_ani_source_target.writeln(
                f"    {func_cpp_user_info.full_name}({cpp_args_str});",
            )
        pkg_ani_source_target.writeln(
            f"}}",
        )

    def gen_method(
        self,
        iface: IfaceDecl,
        method: IfaceMethodDecl,
        pkg_ani_source_target: COutputBuffer,
        ancestor: IfaceDecl,
        mangled_name: str,
    ):
        method_cpp_info = IfaceMethodCppInfo.get(self.am, method)
        iface_cpp_info = IfaceCppInfo.get(self.am, iface)
        iface_abi_info = IfaceABIInfo.get(self.am, iface)
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
        pkg_ani_source_target.writeln(
            f"static {ani_return_ty_name} {mangled_name}({ani_params_str}) {{",
            f"    ani_long ani_data_ptr;",
            f'    env->Object_GetPropertyByName_Long(object, "_data_ptr", reinterpret_cast<ani_long*>(&ani_data_ptr));',
            f"    ani_long ani_vtbl_ptr;",
            f'    env->Object_GetPropertyByName_Long(object, "_vtbl_ptr", reinterpret_cast<ani_long*>(&ani_vtbl_ptr));',
            f"    DataBlockHead* cpp_data_ptr = reinterpret_cast<DataBlockHead*>(ani_data_ptr);",
            f"    {iface_abi_info.vtable}* cpp_vtbl_ptr = reinterpret_cast<{iface_abi_info.vtable}*>(ani_vtbl_ptr);",
            f"    {iface_cpp_info.full_weak_name} cpp_iface = {iface_cpp_info.full_weak_name}({{cpp_vtbl_ptr, cpp_data_ptr}});",
        )
        for param, ani_arg, cpp_arg in zip(
            method.params, ani_args, cpp_args, strict=True
        ):
            type_ani_info = TypeANIInfo.get(self.am, param.ty_ref.resolved_ty)
            type_ani_info.from_ani(pkg_ani_source_target, 4, "env", ani_arg, cpp_arg)
        cpp_args_str = ", ".join(cpp_args)
        if return_ty_ref := method.return_ty_ref:
            type_cpp_info = TypeCppInfo.get(self.am, return_ty_ref.resolved_ty)
            type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
            cpp_return_ty_name = type_cpp_info.as_owner
            cpp_res = "cpp_result"
            ani_res = "ani_result"
            pkg_ani_source_target.writeln(
                f"    {cpp_return_ty_name} {cpp_res} = {ancestor_cpp_info.as_param}(cpp_iface)->{method_cpp_info.call_name}({cpp_args_str});",
                f"    if (::taihe::has_error()) {{ return {type_ani_info.ani_type}{{}}; }}",
            )
            type_ani_info.into_ani(pkg_ani_source_target, 4, "env", cpp_res, ani_res)
            pkg_ani_source_target.writeln(
                f"    return {ani_res};",
            )
        else:
            pkg_ani_source_target.writeln(
                f"    {ancestor_cpp_info.as_param}(cpp_iface)->{method_cpp_info.call_name}({cpp_args_str});",
            )
        pkg_ani_source_target.writeln(
            f"}}",
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
        iface_ani_decl_target = COutputBuffer.create(
            self.tm, f"include/{iface_ani_info.decl_header}", True
        )
        iface_ani_decl_target.include("ani.h")
        iface_ani_decl_target.include(iface_cpp_info.decl_header)
        iface_ani_decl_target.writeln(
            f"{iface_cpp_info.as_owner} {iface_ani_info.from_ani_func_name}(ani_env* env, ani_object ani_obj);",
            f"ani_object {iface_ani_info.into_ani_func_name}(ani_env* env, {iface_cpp_info.as_owner} cpp_obj);",
        )

    def gen_iface_conv_impl_file(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_ani_info: IfaceANIInfo,
    ):
        iface_ani_impl_target = COutputBuffer.create(
            self.tm, f"include/{iface_ani_info.impl_header}", True
        )
        iface_ani_impl_target.include(iface_ani_info.decl_header)
        iface_ani_impl_target.include(iface_cpp_info.impl_header)
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
        iface_ani_impl_target: COutputBuffer,
    ):
        iface_ani_impl_target.writeln(
            f"inline {iface_cpp_info.as_owner} {iface_ani_info.from_ani_func_name}(ani_env* env, ani_object ani_obj) {{",
            f"    struct cpp_impl_t {{",
            f"        ani_ref ref;",
            f"        cpp_impl_t(ani_env* env, ani_object obj) {{",
            f"            env->GlobalReference_Create(obj, &this->ref);",
            f"        }}",
            f"        ~cpp_impl_t() {{",
            f"            ::taihe::env_guard guard;",
            f"            ani_env *env = guard.get_env();",
            f"            env->GlobalReference_Delete(this->ref);",
            f"        }}",
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
                    type_cpp_info = TypeCppInfo.get(self.am, param.ty_ref.resolved_ty)
                    inner_cpp_params.append(f"{type_cpp_info.as_param} {inner_cpp_arg}")
                    inner_cpp_args.append(inner_cpp_arg)
                    inner_ani_args.append(inner_ani_arg)
                inner_cpp_params_str = ", ".join(inner_cpp_params)
                if return_ty_ref := method.return_ty_ref:
                    type_cpp_info = TypeCppInfo.get(self.am, return_ty_ref.resolved_ty)
                    cpp_return_ty_name = type_cpp_info.as_owner
                else:
                    cpp_return_ty_name = "void"
                iface_ani_impl_target.writeln(
                    f"        {cpp_return_ty_name} {method_cpp_info.impl_name}({inner_cpp_params_str}) {{",
                    f"            ::taihe::env_guard guard;",
                    f"            ani_env *env = guard.get_env();",
                )
                for param, inner_cpp_arg, inner_ani_arg in zip(
                    method.params, inner_cpp_args, inner_ani_args, strict=True
                ):
                    type_ani_info = TypeANIInfo.get(self.am, param.ty_ref.resolved_ty)
                    type_ani_info.into_ani(
                        iface_ani_impl_target,
                        12,
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
                    type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
                    iface_ani_impl_target.writeln(
                        f"            {type_ani_info.ani_type} {inner_ani_res};",
                        f'            env->Object_CallMethodByName_{type_ani_info.ani_type.suffix}(static_cast<ani_object>(this->ref), "{method_ani_info.ani_method_name}", nullptr, reinterpret_cast<{type_ani_info.ani_type.base}*>(&{inner_ani_res}){inner_ani_args_trailing});',
                    )
                    type_ani_info.from_ani(
                        iface_ani_impl_target,
                        12,
                        "env",
                        inner_ani_res,
                        inner_cpp_res,
                    )
                    iface_ani_impl_target.writeln(
                        f"            return {inner_cpp_res};",
                    )
                else:
                    iface_ani_impl_target.writeln(
                        f'            env->Object_CallMethodByName_Void(static_cast<ani_object>(this->ref), "{method_ani_info.ani_method_name}", nullptr{inner_ani_args_trailing});',
                    )
                iface_ani_impl_target.writeln(
                    f"        }}",
                )
        iface_ani_impl_target.writeln(
            f"    }};",
            f"    return ::taihe::make_holder<cpp_impl_t, {iface_cpp_info.as_owner}>(env, ani_obj);",
            f"}}",
        )

    def gen_iface_into_ani_func(
        self,
        iface: IfaceDecl,
        iface_abi_info: IfaceABIInfo,
        iface_cpp_info: IfaceCppInfo,
        iface_ani_info: IfaceANIInfo,
        iface_ani_impl_target: COutputBuffer,
    ):
        iface_ani_impl_target.writeln(
            f"inline ani_object {iface_ani_info.into_ani_func_name}(ani_env* env, {iface_cpp_info.as_owner} cpp_obj) {{",
            f"    ani_long ani_vtbl_ptr = reinterpret_cast<ani_long>(cpp_obj.m_handle.vtbl_ptr);",
            f"    ani_long ani_data_ptr = reinterpret_cast<ani_long>(cpp_obj.m_handle.data_ptr);",
            f"    cpp_obj.m_handle.data_ptr = nullptr;",
            f"    ani_class ani_obj_cls;",
            f'    env->FindClass("{iface_ani_info.impl_desc}", &ani_obj_cls);',
            f"    ani_method ani_obj_ctor;",
            f'    env->Class_FindMethod(ani_obj_cls, "<ctor>", "JJ:V", &ani_obj_ctor);',
            f"    ani_object ani_obj;",
            f"    env->Object_New(ani_obj_cls, ani_obj_ctor, &ani_obj, ani_vtbl_ptr, ani_data_ptr);",
            f"    return ani_obj;",
            f"}}",
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
        struct_ani_decl_target = COutputBuffer.create(
            self.tm, f"include/{struct_ani_info.decl_header}", True
        )
        struct_ani_decl_target.include("ani.h")
        struct_ani_decl_target.include(struct_cpp_info.decl_header)
        struct_ani_decl_target.writeln(
            f"{struct_cpp_info.as_owner} {struct_ani_info.from_ani_func_name}(ani_env* env, ani_object ani_obj);",
            f"ani_object {struct_ani_info.into_ani_func_name}(ani_env* env, {struct_cpp_info.as_param} cpp_obj);",
        )

    def gen_struct_conv_impl_file(
        self,
        struct: StructDecl,
        struct_cpp_info: StructCppInfo,
        struct_ani_info: StructANIInfo,
    ):
        struct_ani_impl_target = COutputBuffer.create(
            self.tm, f"include/{struct_ani_info.impl_header}", True
        )
        struct_ani_impl_target.include(struct_ani_info.decl_header)
        struct_ani_impl_target.include(struct_cpp_info.impl_header)
        # TODO: ignore compiler warning
        struct_ani_impl_target.writeln(
            '#pragma clang diagnostic ignored "-Wmissing-braces"',
        )
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
        struct_ani_impl_target: COutputBuffer,
    ):
        struct_ani_impl_target.writeln(
            f"inline {struct_cpp_info.as_owner} {struct_ani_info.from_ani_func_name}(ani_env* env, ani_object ani_obj) {{",
        )
        cpp_field_results = []
        for parts in struct_ani_info.sts_final_fields:
            final = parts[-1]
            type_ani_info = TypeANIInfo.get(self.am, final.ty_ref.resolved_ty)
            ani_field_value = f"ani_field_{final.name}"
            cpp_field_result = f"cpp_field_{final.name}"
            struct_ani_impl_target.writeln(
                f"    {type_ani_info.ani_type} {ani_field_value};",
                f'    env->Object_GetPropertyByName_{type_ani_info.ani_type.suffix}(ani_obj, "{final.name}", reinterpret_cast<{type_ani_info.ani_type.base}*>(&{ani_field_value}));',
            )
            type_ani_info.from_ani(
                struct_ani_impl_target, 4, "env", ani_field_value, cpp_field_result
            )
            cpp_field_results.append(cpp_field_result)
        cpp_moved_fields_str = ", ".join(
            f"std::move({cpp_field_result})" for cpp_field_result in cpp_field_results
        )
        struct_ani_impl_target.writeln(
            f"    return {struct_cpp_info.as_owner}{{{cpp_moved_fields_str}}};",
            f"}}",
        )

    def gen_struct_into_ani_func(
        self,
        struct: StructDecl,
        struct_cpp_info: StructCppInfo,
        struct_ani_info: StructANIInfo,
        struct_ani_impl_target: COutputBuffer,
    ):
        ani_field_results = []
        struct_ani_impl_target.writeln(
            f"inline ani_object {struct_ani_info.into_ani_func_name}(ani_env* env, {struct_cpp_info.as_param} cpp_obj) {{",
        )
        for parts in struct_ani_info.sts_final_fields:
            final = parts[-1]
            ani_field_result = f"ani_field_{final.name}"
            type_ani_info = TypeANIInfo.get(self.am, final.ty_ref.resolved_ty)
            type_ani_info.into_ani(
                struct_ani_impl_target,
                4,
                "env",
                ".".join(("cpp_obj", *(part.name for part in parts))),
                ani_field_result,
            )
            ani_field_results.append(ani_field_result)
        ani_field_results_trailing = "".join(
            ", " + ani_field_result for ani_field_result in ani_field_results
        )
        struct_ani_impl_target.writeln(
            f"    ani_class ani_obj_cls;",
            f'    env->FindClass("{struct_ani_info.impl_desc}", &ani_obj_cls);',
            f"    ani_method ani_obj_ctor;",
            f'    env->Class_FindMethod(ani_obj_cls, "<ctor>", nullptr, &ani_obj_ctor);',
            f"    ani_object ani_obj;",
            f"    env->Object_New(ani_obj_cls, ani_obj_ctor, &ani_obj{ani_field_results_trailing});",
            f"    return ani_obj;",
            f"}}",
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
        union_ani_decl_target = COutputBuffer.create(
            self.tm, f"include/{union_ani_info.decl_header}", True
        )
        union_ani_decl_target.include("ani.h")
        union_ani_decl_target.include(union_cpp_info.decl_header)
        union_ani_decl_target.writeln(
            f"{union_cpp_info.as_owner} {union_ani_info.from_ani_func_name}(ani_env* env, ani_ref ani_obj);",
            f"ani_ref {union_ani_info.into_ani_func_name}(ani_env* env, {union_cpp_info.as_param} cpp_obj);",
        )

    def gen_union_conv_impl_file(
        self,
        union: UnionDecl,
        union_cpp_info: UnionCppInfo,
        union_ani_info: UnionANIInfo,
    ):
        union_ani_impl_target = COutputBuffer.create(
            self.tm, f"include/{union_ani_info.impl_header}", True
        )
        union_ani_impl_target.include(union_ani_info.decl_header)
        union_ani_impl_target.include(union_cpp_info.impl_header)
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
        union_ani_impl_target: COutputBuffer,
    ):
        union_ani_impl_target.writeln(
            f"inline {union_cpp_info.as_owner} {union_ani_info.from_ani_func_name}(ani_env* env, ani_ref ani_value) {{",
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
            if final_ani_info.field_ty == "null":
                union_ani_impl_target.writeln(
                    f"    ani_boolean {is_field};",
                    f"    env->Reference_IsNull(ani_value, &{is_field});",
                    f"    if ({is_field}) {{",
                    f"        return {union_cpp_info.full_name}({static_tags_str});",
                    f"    }}",
                )
            if final_ani_info.field_ty == "undefined":
                union_ani_impl_target.writeln(
                    f"    ani_boolean {is_field};",
                    f"    env->Reference_IsUndefined(ani_value, &{is_field});",
                    f"    if ({is_field}) {{",
                    f"        return {union_cpp_info.full_name}({static_tags_str});",
                    f"    }}",
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
            field_class = f"ani_cls_{full_name}"
            final_ani_info = UnionFieldANIInfo.get(self.am, final)
            if isinstance(final_ty := final_ani_info.field_ty, Type):
                type_ani_info = TypeANIInfo.get(self.am, final_ty)
                union_ani_impl_target.writeln(
                    f"    ani_class {field_class};",
                    f'    env->FindClass("{type_ani_info.type_desc_boxed}", &{field_class});',
                    f"    ani_boolean {is_field};",
                    f"    env->Object_InstanceOf(static_cast<ani_object>(ani_value), {field_class}, &{is_field});",
                    f"    if ({is_field}) {{",
                )
                cpp_result_spec = f"cpp_field_{full_name}"
                type_ani_info.from_ani_boxed(
                    union_ani_impl_target,
                    8,
                    "env",
                    "ani_value",
                    cpp_result_spec,
                )
                union_ani_impl_target.writeln(
                    f"        return {union_cpp_info.full_name}({static_tags_str}, std::move({cpp_result_spec}));",
                    f"    }}",
                )
        union_ani_impl_target.writeln(
            f"    __builtin_unreachable();",
            f"}}",
        )

    def gen_union_into_ani_func(
        self,
        union: UnionDecl,
        union_cpp_info: UnionCppInfo,
        union_ani_info: UnionANIInfo,
        union_ani_impl_target: COutputBuffer,
    ):
        union_ani_impl_target.writeln(
            f"inline ani_ref {union_ani_info.into_ani_func_name}(ani_env* env, {union_cpp_info.as_param} cpp_value) {{",
            f"    ani_ref ani_value;",
            f"    switch (cpp_value.get_tag()) {{",
        )
        for field in union.fields:
            field_ani_info = UnionFieldANIInfo.get(self.am, field)
            union_ani_impl_target.writeln(
                f"    case {union_cpp_info.full_name}::tag_t::{field.name}: {{",
            )
            match field_ani_info.field_ty:
                case "null":
                    union_ani_impl_target.writeln(
                        f"        env->GetNull(&ani_value);",
                    )
                case "undefined":
                    union_ani_impl_target.writeln(
                        f"        env->GetUndefined(&ani_value);",
                    )
                case field_ty if isinstance(field_ty, Type):
                    ani_result_spec = f"ani_field_{field.name}"
                    type_ani_info = TypeANIInfo.get(self.am, field_ty)
                    type_ani_info.into_ani_boxed(
                        union_ani_impl_target,
                        8,
                        "env",
                        f"cpp_value.get_{field.name}_ref()",
                        ani_result_spec,
                    )
                    union_ani_impl_target.writeln(
                        f"        ani_value = {ani_result_spec};",
                    )
            union_ani_impl_target.writeln(
                f"        break;",
                f"    }}",
            )
        union_ani_impl_target.writeln(
            f"    }}",
            f"    return ani_value;",
            f"}}",
        )