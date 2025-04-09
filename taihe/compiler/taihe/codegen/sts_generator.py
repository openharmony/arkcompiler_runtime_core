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
from typing import TYPE_CHECKING

from taihe.codegen.abi_generator import (
    IfaceABIInfo,
)
from taihe.codegen.ani_generator import (
    EnumANIInfo,
    GlobFuncANIInfo,
    IfaceANIInfo,
    IfaceMethodANIInfo,
    PackageANIInfo,
    StructANIInfo,
    TypeANIInfo,
    UnionANIInfo,
)
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
from taihe.semantics.types import IfaceType, StructType
from taihe.utils.analyses import AnalysisManager
from taihe.utils.outputs import OutputManager, STSOutputBuffer

if TYPE_CHECKING:
    from taihe.semantics.declarations import (
        Type,
    )


class Namespace:
    def __init__(self):
        self.children: dict[str, Namespace] = {}
        self.packages: list[PackageDecl] = []

    def add_path(self, path_parts: list[str], pkg: PackageDecl):
        if not path_parts:
            self.packages.append(pkg)
            return
        head, *tail = path_parts
        child = self.children.setdefault(head, Namespace())
        child.add_path(tail, pkg)


class STSCodeGenerator:
    def __init__(self, tm: OutputManager, am: AnalysisManager):
        self.tm = tm
        self.am = am

    def generate(self, pg: PackageGroup):
        ns_dict: dict[str, Namespace] = {}
        for pkg in pg.packages:
            pkg_ani_info = PackageANIInfo.get(self.am, pkg)
            ns_dict.setdefault(pkg_ani_info.module, Namespace()).add_path(
                pkg_ani_info.sts_ns_parts, pkg
            )
        for module, ns in ns_dict.items():
            self.gen_module_file(module, ns)

    def gen_module_file(self, module: str, ns: Namespace):
        module_sts_file = f"{module}.ets"
        target = STSOutputBuffer.create(self.tm, module_sts_file)
        self.gen_namespace(ns, target)

    def gen_namespace(self, ns: Namespace, target: STSOutputBuffer):
        for child_ns_name, child_ns in ns.children.items():
            target.write(f"export namespace {child_ns_name} {{\n")
            with target.indent_manager.offset(4):
                self.gen_namespace(child_ns, target)
            target.write(f"}}\n")
        for pkg in ns.packages:
            self.gen_package(pkg, target)

    def stat_on_off_funcs(self, funcs: list[GlobFuncDecl]):
        glob_func_on_off_map: dict[
            tuple[str, tuple[Type, ...]], list[tuple[str, GlobFuncDecl]]
        ] = {}
        for func in funcs:
            func_ani_info = GlobFuncANIInfo.get(self.am, func)
            if func_ani_info.on_off_type is not None:
                func_name, type_name = func_ani_info.on_off_type
                real_params_ty = []
                for real_param in func_ani_info.sts_real_params:
                    assert real_param.ty_ref.resolved_ty
                    real_params_ty.append(real_param.ty_ref.resolved_ty)
                glob_func_on_off_map.setdefault(
                    (func_name, tuple(real_params_ty)), []
                ).append((type_name, func))
        return glob_func_on_off_map

    def stat_on_off_methods(self, methods: list[IfaceMethodDecl]):
        method_on_off_map: dict[
            tuple[str, tuple[Type, ...]], list[tuple[str, IfaceMethodDecl]]
        ] = {}
        for method in methods:
            method_ani_info = IfaceMethodANIInfo.get(self.am, method)
            if method_ani_info.on_off_type is not None:
                method_name, type_name = method_ani_info.on_off_type
                real_params_ty = []
                for real_param in method_ani_info.sts_real_params:
                    assert real_param.ty_ref.resolved_ty
                    real_params_ty.append(real_param.ty_ref.resolved_ty)
                method_on_off_map.setdefault(
                    (method_name, tuple(real_params_ty)), []
                ).append((type_name, method))
        return method_on_off_map

    def gen_package(self, pkg: PackageDecl, target: STSOutputBuffer):
        # TODO: hack inject
        pkg_ani_info = PackageANIInfo.get(self.am, pkg)
        for injected in pkg_ani_info.injected_codes:
            target.write(injected)

        self.gen_native_funcs(pkg, pkg.functions, target)
        ctors_map: dict[str, list[GlobFuncDecl]] = {}
        statics_map: dict[str, list[GlobFuncDecl]] = {}
        funcs: list[GlobFuncDecl] = []
        for func in pkg.functions:
            func_ani_info = GlobFuncANIInfo.get(self.am, func)
            if class_name := func_ani_info.sts_static_scope:
                statics_map.setdefault(class_name, []).append(func)
            elif class_name := func_ani_info.sts_ctor_scope:
                ctors_map.setdefault(class_name, []).append(func)
            else:
                funcs.append(func)
        self.gen_global_funcs(pkg, funcs, target)
        for enum in pkg.enums:
            self.gen_enum(pkg, enum, target)
        for union in pkg.unions:
            self.gen_union(pkg, union, target)
        for struct in pkg.structs:
            self.gen_struct_interface(pkg, struct, target)
        for struct in pkg.structs:
            self.gen_struct_class(pkg, struct, target)
        for iface in pkg.interfaces:
            self.gen_iface_interface(pkg, iface, target)
        for iface in pkg.interfaces:
            self.gen_iface_class(pkg, iface, target, statics_map, ctors_map)

    def gen_native_funcs(
        self,
        pkg: PackageDecl,
        funcs: list[GlobFuncDecl],
        target: STSOutputBuffer,
    ):
        # native funcs
        for func in funcs:
            func_ani_info = GlobFuncANIInfo.get(self.am, func)
            sts_native_params = []
            for param in func.params:
                type_ani_info = TypeANIInfo.get(self.am, param.ty_ref.resolved_ty)
                sts_native_params.append(
                    f"{param.name}: {type_ani_info.sts_type_in(pkg, target)}"
                )
            sts_native_params_str = ", ".join(sts_native_params)
            if return_ty_ref := func.return_ty_ref:
                type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
                sts_return_ty_name = type_ani_info.sts_type_in(pkg, target)
            else:
                sts_return_ty_name = "void"
            target.write(
                f"native function {func_ani_info.sts_native_name}({sts_native_params_str}): {sts_return_ty_name};\n"
            )

    def gen_global_funcs(
        self,
        pkg: PackageDecl,
        funcs: list[GlobFuncDecl],
        target: STSOutputBuffer,
    ):
        # on_off
        glob_func_on_off_map = self.stat_on_off_funcs(funcs)
        for (func_name, real_params_ty), func_list in glob_func_on_off_map.items():
            sts_real_params = []
            sts_real_args = []
            sts_real_params.append("type: string")
            for index, param_ty in enumerate(real_params_ty):
                type_ani_info = TypeANIInfo.get(self.am, param_ty)
                param_name = f"p_{index}"
                sts_real_params.append(
                    f"{param_name}: {type_ani_info.sts_type_in(pkg, target)}"
                )
                sts_real_args.append(param_name)
            sts_real_params_str = ", ".join(sts_real_params)
            target.write(
                f"export function {func_name}({sts_real_params_str}): void {{\n"
                f"    switch(type) {{"
            )
            for type_name, func in func_list:
                func_ani_info = GlobFuncANIInfo.get(self.am, func)
                sts_native_call = func_ani_info.call_native_with(sts_real_args)
                target.write(f'        case "{type_name}": return {sts_native_call};\n')
            target.write(
                f"        default: throw new Error(`Unknown type: ${{type}}`);\n"
                f"    }}\n"
                f"}}\n"
            )
        # other
        for func in funcs:
            func_ani_info = GlobFuncANIInfo.get(self.am, func)
            sts_real_params = []
            sts_real_args = []
            for sts_real_param in func_ani_info.sts_real_params:
                type_ani_info = TypeANIInfo.get(
                    self.am, sts_real_param.ty_ref.resolved_ty
                )
                sts_real_params.append(
                    f"{sts_real_param.name}: {type_ani_info.sts_type_in(pkg, target)}"
                )
                sts_real_args.append(sts_real_param.name)
            sts_real_params_str = ", ".join(sts_real_params)
            sts_native_call = func_ani_info.call_native_with(sts_real_args)
            if return_ty_ref := func.return_ty_ref:
                type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
                sts_return_ty_name = type_ani_info.sts_type_in(pkg, target)
            else:
                sts_return_ty_name = "void"
            # real
            if func_ani_info.sts_func_name is not None:
                target.write(
                    f"export function {func_ani_info.sts_func_name}({sts_real_params_str}): {sts_return_ty_name} {{\n"
                    f"    return {sts_native_call};\n"
                    f"}}\n"
                )
                # promise
                if (promise_func_name := func_ani_info.sts_promise_name) is not None:
                    if return_ty_ref := func.return_ty_ref:
                        resolve_params = f"data: {sts_return_ty_name}"
                        resolve_args = f"ret as {sts_return_ty_name}"
                    else:
                        resolve_params = ""
                        resolve_args = ""
                    target.write(
                        f"export function {promise_func_name}({sts_real_params_str}): Promise<{sts_return_ty_name}> {{\n"
                        f"    return new Promise<{sts_return_ty_name}>((resolve: ({resolve_params}) => void, reject: (err: Error) => void): void => {{\n"
                        f"        taskpool.execute((): {sts_return_ty_name} => {{ return {sts_native_call}; }})\n"
                        f"        .then((ret: NullishType): void => {{\n"
                        f"            resolve({resolve_args});\n"
                        f"        }})\n"
                        f"        .catch((ret: NullishType): void => {{\n"
                        f"            reject(ret as Error);\n"
                        f"        }});\n"
                        f"    }});\n"
                        f"}}\n"
                    )
                # async
                if (async_func_name := func_ani_info.sts_async_name) is not None:
                    if return_ty_ref := func.return_ty_ref:
                        callback = f"callback: (err: Error, data?: {sts_return_ty_name}) => void"
                        then_args = f"new Error(), ret as {sts_return_ty_name}"
                    else:
                        callback = "callback: (err: Error) => void"
                        then_args = "new Error()"
                    sts_real_params_with_cb = [*sts_real_params, callback]
                    sts_real_params_with_cb_str = ", ".join(sts_real_params_with_cb)
                    target.write(
                        f"export function {async_func_name}({sts_real_params_with_cb_str}): void {{\n"
                        f"    taskpool.execute((): {sts_return_ty_name} => {{ return {sts_native_call}; }})\n"
                        f"    .then((ret: NullishType): void => {{\n"
                        f"        callback({then_args});\n"
                        f"    }})\n"
                        f"    .catch((ret: NullishType): void => {{\n"
                        f"        callback(ret as Error);\n"
                        f"    }});\n"
                        f"}}\n"
                    )

    def gen_enum(
        self,
        pkg: PackageDecl,
        enum: EnumDecl,
        target: STSOutputBuffer,
    ):
        enum_ani_info = EnumANIInfo.get(self.am, enum)
        if enum_ani_info.const:
            assert enum.ty_ref
            type_ani_info = TypeANIInfo.get(self.am, enum.ty_ref.resolved_ty)
            for item in enum.items:
                target.write(
                    f"export const {item.name}: {type_ani_info.sts_type_in(pkg, target)} = {dumps(item.value)};\n"
                )
            return
        target.write(f"export enum {enum_ani_info.sts_type_name} {{\n")
        with target.indent_manager.offset(4):
            for item in enum.items:
                if item.value is None:
                    target.write(f"{item.name},\n")
                else:
                    target.write(f"{item.name} = {dumps(item.value)},\n")
        target.write(f"}}\n")

    def gen_union(
        self,
        pkg: PackageDecl,
        union: UnionDecl,
        target: STSOutputBuffer,
    ):
        union_ani_info = UnionANIInfo.get(self.am, union)
        sts_types = []
        for field in union.fields:
            if field.ty_ref is None:
                sts_types.append("undefined")
                continue
            ty_ani_info = TypeANIInfo.get(self.am, field.ty_ref.resolved_ty)
            sts_types.append(f"{ty_ani_info.sts_type_in(pkg, target)}")
        sts_types_str = " | ".join(sts_types)
        target.write(f"export type {union_ani_info.sts_type_name} = {sts_types_str};\n")

    def gen_struct_interface(
        self,
        pkg: PackageDecl,
        struct: StructDecl,
        target: STSOutputBuffer,
    ):
        struct_ani_info = StructANIInfo.get(self.am, struct)
        if struct_ani_info.is_class():
            # no interface
            return
        parents = []
        for parent in struct_ani_info.sts_parents:
            ty = parent.ty_ref.resolved_ty
            assert isinstance(ty, StructType)
            parent_ani_info = StructANIInfo.get(self.am, ty.ty_decl)
            parents.append(parent_ani_info.sts_type_name)
        extends_str = " extends " + ", ".join(parents) if parents else ""
        target.write(
            f"export interface {struct_ani_info.sts_type_name}{extends_str} {{\n"
        )
        with target.indent_manager.offset(4):
            for field in struct_ani_info.sts_fields:
                ty_ani_info = TypeANIInfo.get(self.am, field.ty_ref.resolved_ty)
                target.write(f"{field.name}: {ty_ani_info.sts_type_in(pkg, target)};\n")
        target.write(f"}}\n")

    def gen_struct_class(
        self,
        pkg: PackageDecl,
        struct: StructDecl,
        target: STSOutputBuffer,
    ):
        struct_ani_info = StructANIInfo.get(self.am, struct)
        if struct_ani_info.is_class():
            parents = []
            for parent in struct_ani_info.sts_parents:
                ty = parent.ty_ref.resolved_ty
                assert isinstance(ty, StructType)
                parent_ani_info = StructANIInfo.get(self.am, ty.ty_decl)
                parents.append(parent_ani_info.sts_type_name)
            implements_str = " implements " + ", ".join(parents) if parents else ""
            target.write(
                f"export class {struct_ani_info.sts_impl_name}{implements_str} {{\n"
            )
        else:
            target.write(
                f"class {struct_ani_info.sts_impl_name} implements {struct_ani_info.sts_type_name} {{\n"
            )
        with target.indent_manager.offset(4):
            for parts in struct_ani_info.sts_final_fields:
                final = parts[-1]
                ty_ani_info = TypeANIInfo.get(self.am, final.ty_ref.resolved_ty)
                target.write(f"{final.name}: {ty_ani_info.sts_type_in(pkg, target)};\n")
            target.write(f"constructor(\n")
            for parts in struct_ani_info.sts_final_fields:
                final = parts[-1]
                ty_ani_info = TypeANIInfo.get(self.am, final.ty_ref.resolved_ty)
                target.write(
                    f"    {final.name}: {ty_ani_info.sts_type_in(pkg, target)},\n"
                )
            target.write(f") {{\n")
            for parts in struct_ani_info.sts_final_fields:
                final = parts[-1]
                target.write(f"    this.{final.name} = {final.name};\n")
            target.write(f"}}\n")
        target.write(f"}}\n")

    def gen_iface_interface(
        self,
        pkg: PackageDecl,
        iface: IfaceDecl,
        target: STSOutputBuffer,
    ):
        iface_ani_info = IfaceANIInfo.get(self.am, iface)
        if iface_ani_info.is_class():
            # no interface
            return
        parents = []
        for parent in iface.parents:
            ty = parent.ty_ref.resolved_ty
            assert isinstance(ty, IfaceType)
            parent_ani_info = IfaceANIInfo.get(self.am, ty.ty_decl)
            parents.append(parent_ani_info.sts_type_name)
        extends_str = " extends " + ", ".join(parents) if parents else ""
        target.write(
            f"export interface {iface_ani_info.sts_type_name}{extends_str} {{\n"
        )
        with target.indent_manager.offset(4):
            # TODO: hack inject
            for injected in iface_ani_info.iface_injected_codes:
                target.write(injected)
            self.gen_iface_methods_decl(pkg, iface.methods, target)
        target.write(f"}}\n")

    def gen_iface_methods_decl(
        self,
        pkg: PackageDecl,
        methods: list[IfaceMethodDecl],
        target: STSOutputBuffer,
    ):
        # on_off
        method_on_off_map = self.stat_on_off_methods(methods)
        for (method_name, real_params_ty), _ in method_on_off_map.items():
            sts_real_params = []
            sts_real_params.append("type: string")
            for index, param_ty in enumerate(real_params_ty):
                type_ani_info = TypeANIInfo.get(self.am, param_ty)
                param_name = f"p_{index}"
                sts_real_params.append(
                    f"{param_name}: {type_ani_info.sts_type_in(pkg, target)}"
                )
            sts_real_params_str = ", ".join(sts_real_params)
            target.write(f"{method_name}({sts_real_params_str}): void;\n")
        # other
        for method in methods:
            method_ani_info = IfaceMethodANIInfo.get(self.am, method)
            sts_real_params = []
            for real_param in method_ani_info.sts_real_params:
                type_ani_info = TypeANIInfo.get(self.am, real_param.ty_ref.resolved_ty)
                sts_real_params.append(
                    f"{real_param.name}: {type_ani_info.sts_type_in(pkg, target)}"
                )
            sts_real_params_str = ", ".join(sts_real_params)
            if return_ty_ref := method.return_ty_ref:
                type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
                sts_return_ty_name = type_ani_info.sts_type_in(pkg, target)
            else:
                sts_return_ty_name = "void"
            # real
            if real_method := method_ani_info.sts_method_name:
                target.write(
                    f"{real_method}({sts_real_params_str}): {sts_return_ty_name};\n"
                )
                # promise
                if (promise_func_name := method_ani_info.sts_promise_name) is not None:
                    target.write(
                        f"{promise_func_name}({sts_real_params_str}): Promise<{sts_return_ty_name}>;\n"
                    )
                # async
                if (async_func_name := method_ani_info.sts_async_name) is not None:
                    if return_ty_ref := method.return_ty_ref:
                        callback = f"callback: (err: Error, data?: {sts_return_ty_name}) => void"
                    else:
                        callback = "callback: (err: Error) => void"
                    sts_real_params_with_cb = [*sts_real_params, callback]
                    sts_real_params_with_cb_str = ", ".join(sts_real_params_with_cb)
                    target.write(
                        f"{async_func_name}({sts_real_params_with_cb_str}): void;\n"
                    )
            # getter
            if get_name := method_ani_info.get_name:
                target.write(
                    f"get {get_name}({sts_real_params_str}): {sts_return_ty_name};\n"
                )
            # setter
            if set_name := method_ani_info.set_name:
                target.write(f"set {set_name}({sts_real_params_str});\n")

    def gen_iface_class(
        self,
        pkg: PackageDecl,
        iface: IfaceDecl,
        target: STSOutputBuffer,
        statics_map: dict[str, list[GlobFuncDecl]],
        ctors_map: dict[str, list[GlobFuncDecl]],
    ):
        iface_ani_info = IfaceANIInfo.get(self.am, iface)
        if iface_ani_info.is_class():
            parents = []
            for parent in iface.parents:
                ty = parent.ty_ref.resolved_ty
                assert isinstance(ty, IfaceType)
                parent_ani_info = IfaceANIInfo.get(self.am, ty.ty_decl)
                parents.append(parent_ani_info.sts_type_name)
            implements_str = " implements " + ", ".join(parents) if parents else ""
            target.write(
                f"export class {iface_ani_info.sts_impl_name}{implements_str} {{\n"
            )
        else:
            target.write(
                f"class {iface_ani_info.sts_impl_name} implements {iface_ani_info.sts_type_name} {{\n"
            )
        with target.indent_manager.offset(4):
            # TODO: hack inject
            for injected in iface_ani_info.class_injected_codes:
                target.write(injected)
            target.write(
                f"private _vtbl_ptr: long;\n"
                f"private _data_ptr: long;\n"
                f"private constructor(_vtbl_ptr: long, _data_ptr: long) {{\n"
                f"    this._vtbl_ptr = _vtbl_ptr;\n"
                f"    this._data_ptr = _data_ptr;\n"
                f"}}\n"
            )
            ctors = ctors_map.get(iface.name, [])
            for tor in ctors:
                ctor_ani_info = GlobFuncANIInfo.get(self.am, tor)
                sts_real_params = []
                sts_real_args = []
                for sts_real_param in ctor_ani_info.sts_real_params:
                    type_ani_info = TypeANIInfo.get(
                        self.am, sts_real_param.ty_ref.resolved_ty
                    )
                    sts_real_params.append(
                        f"{sts_real_param.name}: {type_ani_info.sts_type_in(pkg, target)}"
                    )
                    sts_real_args.append(sts_real_param.name)
                sts_real_params_str = ", ".join(sts_real_params)
                sts_native_call = ctor_ani_info.call_native_with(sts_real_args)
                target.write(
                    f"constructor({sts_real_params_str}) {{\n"
                    f"    let temp = {sts_native_call} as {iface_ani_info.sts_impl_name};\n"
                    f"    this._data_ptr = temp._data_ptr;\n"
                    f"    this._vtbl_ptr = temp._vtbl_ptr;\n"
                    f"}}\n"
                )
            self.gen_static_funcs(pkg, statics_map.get(iface.name, []), target)
            iface_abi_info = IfaceABIInfo.get(self.am, iface)
            for ancestor in iface_abi_info.ancestor_dict:
                self.gen_native_methods(pkg, ancestor.methods, target)
                self.gen_iface_methods(pkg, ancestor.methods, target)
        target.write(f"}}\n")

    def gen_static_funcs(
        self,
        pkg: PackageDecl,
        funcs: list[GlobFuncDecl],
        target: STSOutputBuffer,
    ):
        # on_off
        func_on_off_map = self.stat_on_off_funcs(funcs)
        for (func_name, real_params_ty), func_list in func_on_off_map.items():
            sts_real_params = []
            sts_real_args = []
            sts_real_params.append("type: string")
            for index, param_ty in enumerate(real_params_ty):
                type_ani_info = TypeANIInfo.get(self.am, param_ty)
                param_name = f"p_{index}"
                sts_real_params.append(
                    f"{param_name}: {type_ani_info.sts_type_in(pkg, target)}"
                )
                sts_real_args.append(param_name)
            sts_real_params_str = ", ".join(sts_real_params)
            target.write(
                f"static {func_name}({sts_real_params_str}): void {{\n"
                f"    switch(type) {{"
            )
            for type_name, func in func_list:
                func_ani_info = GlobFuncANIInfo.get(self.am, func)
                sts_native_call = func_ani_info.call_native_with(sts_real_args)
                target.write(f'        case "{type_name}": return {sts_native_call};\n')
            target.write(
                f"        default: throw new Error(`Unknown type: ${{type}}`);\n"
                f"    }}\n"
                f"}}\n"
            )
        # other
        for func in funcs:
            func_ani_info = GlobFuncANIInfo.get(self.am, func)
            sts_real_params = []
            sts_real_args = []
            for sts_real_param in func_ani_info.sts_real_params:
                type_ani_info = TypeANIInfo.get(
                    self.am, sts_real_param.ty_ref.resolved_ty
                )
                sts_real_params.append(
                    f"{sts_real_param.name}: {type_ani_info.sts_type_in(pkg, target)}"
                )
                sts_real_args.append(sts_real_param.name)
            sts_real_params_str = ", ".join(sts_real_params)
            sts_native_call = func_ani_info.call_native_with(sts_real_args)
            if return_ty_ref := func.return_ty_ref:
                type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
                sts_return_ty_name = type_ani_info.sts_type_in(pkg, target)
            else:
                sts_return_ty_name = "void"
            # real
            if func_ani_info.sts_func_name is not None:
                target.write(
                    f"static {func_ani_info.sts_func_name}({sts_real_params_str}): {sts_return_ty_name} {{\n"
                    f"    return {sts_native_call};\n"
                    f"}}\n"
                )
                # promise
                if (promise_func_name := func_ani_info.sts_promise_name) is not None:
                    if return_ty_ref := func.return_ty_ref:
                        resolve_params = f"data: {sts_return_ty_name}"
                        resolve_args = f"ret as {sts_return_ty_name}"
                    else:
                        resolve_params = ""
                        resolve_args = ""
                    target.write(
                        f"static {promise_func_name}({sts_real_params_str}): Promise<{sts_return_ty_name}> {{\n"
                        f"    return new Promise<{sts_return_ty_name}>((resolve: ({resolve_params}) => void, reject: (err: Error) => void): void => {{\n"
                        f"        taskpool.execute((): {sts_return_ty_name} => {{ return {sts_native_call}; }})\n"
                        f"        .then((ret: NullishType): void => {{\n"
                        f"            resolve({resolve_args});\n"
                        f"        }})\n"
                        f"        .catch((ret: NullishType): void => {{\n"
                        f"            reject(ret as Error);\n"
                        f"        }});\n"
                        f"    }});\n"
                        f"}}\n"
                    )
                # async
                if (async_func_name := func_ani_info.sts_async_name) is not None:
                    if return_ty_ref := func.return_ty_ref:
                        callback = f"callback: (err: Error, data?: {sts_return_ty_name}) => void"
                        then_args = f"new Error(), ret as {sts_return_ty_name}"
                    else:
                        callback = "callback: (err: Error) => void"
                        then_args = "new Error()"
                    sts_real_params_with_cb = [*sts_real_params, callback]
                    sts_real_params_with_cb_str = ", ".join(sts_real_params_with_cb)
                    target.write(
                        f"static {async_func_name}({sts_real_params_with_cb_str}): void {{\n"
                        f"    taskpool.execute((): {sts_return_ty_name} => {{ return {sts_native_call}; }})\n"
                        f"    .then((ret: NullishType): void => {{\n"
                        f"        callback({then_args});\n"
                        f"    }})\n"
                        f"    .catch((ret: NullishType): void => {{\n"
                        f"        callback(ret as Error);\n"
                        f"    }});\n"
                        f"}}\n"
                    )
            # getter
            if get_name := func_ani_info.get_name:
                target.write(
                    f"static get {get_name}({sts_real_params_str}): {sts_return_ty_name} {{\n"
                    f"    return {sts_native_call};\n"
                    f"}}\n"
                )
            # setter
            if set_name := func_ani_info.set_name:
                target.write(
                    f"static set {set_name}({sts_real_params_str}) {{\n"
                    f"    return {sts_native_call};\n"
                    f"}}\n"
                )

    def gen_native_methods(
        self,
        pkg: PackageDecl,
        methods: list[IfaceMethodDecl],
        target: STSOutputBuffer,
    ):
        # native
        for method in methods:
            method_ani_info = IfaceMethodANIInfo.get(self.am, method)
            sts_native_params = []
            for param in method.params:
                type_ani_info = TypeANIInfo.get(self.am, param.ty_ref.resolved_ty)
                sts_native_params.append(
                    f"{param.name}: {type_ani_info.sts_type_in(pkg, target)}"
                )
            sts_native_params_str = ", ".join(sts_native_params)
            if return_ty_ref := method.return_ty_ref:
                type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
                sts_return_ty_name = type_ani_info.sts_type_in(pkg, target)
            else:
                sts_return_ty_name = "void"
            target.write(
                f"native {method_ani_info.sts_native_name}({sts_native_params_str}): {sts_return_ty_name};\n"
            )

    def gen_iface_methods(
        self,
        pkg: PackageDecl,
        methods: list[IfaceMethodDecl],
        target: STSOutputBuffer,
    ):
        # on_off
        method_on_off_map = self.stat_on_off_methods(methods)
        for (method_name, real_params_ty), method_list in method_on_off_map.items():
            sts_real_params = []
            sts_real_args = []
            sts_real_params.append("type: string")
            for index, param_ty in enumerate(real_params_ty):
                type_ani_info = TypeANIInfo.get(self.am, param_ty)
                param_name = f"p_{index}"
                sts_real_params.append(
                    f"{param_name}: {type_ani_info.sts_type_in(pkg, target)}"
                )
                sts_real_args.append(param_name)
            sts_real_params_str = ", ".join(sts_real_params)
            target.write(
                f"{method_name}({sts_real_params_str}): void {{\n"
                f"    switch(type) {{"
            )
            for type_name, method in method_list:
                method_ani_info = IfaceMethodANIInfo.get(self.am, method)
                sts_native_call = method_ani_info.call_native_with(
                    "this", sts_real_args
                )
                target.write(f'        case "{type_name}": return {sts_native_call};\n')
            target.write(
                f"        default: throw new Error(`Unknown type: ${{type}}`);\n"
                f"    }}\n"
                f"}}\n"
            )
        # other
        for method in methods:
            method_ani_info = IfaceMethodANIInfo.get(self.am, method)
            sts_real_params = []
            sts_real_args = []
            for sts_real_param in method_ani_info.sts_real_params:
                type_ani_info = TypeANIInfo.get(
                    self.am, sts_real_param.ty_ref.resolved_ty
                )
                sts_real_params.append(
                    f"{sts_real_param.name}: {type_ani_info.sts_type_in(pkg, target)}"
                )
                sts_real_args.append(sts_real_param.name)
            sts_real_params_str = ", ".join(sts_real_params)
            sts_native_call = method_ani_info.call_native_with("this", sts_real_args)
            if return_ty_ref := method.return_ty_ref:
                type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
                sts_return_ty_name = type_ani_info.sts_type_in(pkg, target)
            else:
                sts_return_ty_name = "void"
            # real
            if real_method := method_ani_info.sts_method_name:
                target.write(
                    f"{real_method}({sts_real_params_str}): {sts_return_ty_name} {{\n"
                    f"    return {sts_native_call};\n"
                    f"}}\n"
                )
                # promise
                if (promise_func_name := method_ani_info.sts_promise_name) is not None:
                    if return_ty_ref := method.return_ty_ref:
                        resolve_params = f"data: {sts_return_ty_name}"
                        resolve_args = f"ret as {sts_return_ty_name}"
                    else:
                        resolve_params = ""
                        resolve_args = ""
                    target.write(
                        f"{promise_func_name}({sts_real_params_str}): Promise<{sts_return_ty_name}> {{\n"
                        f"    return new Promise<{sts_return_ty_name}>((resolve: ({resolve_params}) => void, reject: (err: Error) => void): void => {{\n"
                        f"        taskpool.execute((): {sts_return_ty_name} => {{ return {sts_native_call}; }})\n"
                        f"        .then((ret: NullishType): void => {{\n"
                        f"            resolve({resolve_args});\n"
                        f"        }})\n"
                        f"        .catch((ret: NullishType): void => {{\n"
                        f"            reject(ret as Error);\n"
                        f"        }});\n"
                        f"    }});\n"
                        f"}}\n"
                    )
                # async
                if (async_func_name := method_ani_info.sts_async_name) is not None:
                    if return_ty_ref := method.return_ty_ref:
                        callback = f"callback: (err: Error, data?: {sts_return_ty_name}) => void"
                        then_args = f"new Error(), ret as {sts_return_ty_name}"
                    else:
                        callback = "callback: (err: Error) => void"
                        then_args = "new Error()"
                    sts_real_params_with_cb = [*sts_real_params, callback]
                    sts_real_params_with_cb_str = ", ".join(sts_real_params_with_cb)
                    target.write(
                        f"{async_func_name}({sts_real_params_with_cb_str}): void {{\n"
                        f"    taskpool.execute((): {sts_return_ty_name} => {{ return {sts_native_call}; }})\n"
                        f"    .then((ret: NullishType): void => {{\n"
                        f"        callback({then_args});\n"
                        f"    }})\n"
                        f"    .catch((ret: NullishType): void => {{\n"
                        f"        callback(ret as Error);\n"
                        f"    }});\n"
                        f"}}\n"
                    )
            # getter
            if get_name := method_ani_info.get_name:
                target.write(
                    f"get {get_name}({sts_real_params_str}): {sts_return_ty_name} {{\n"
                    f"    return {sts_native_call};\n"
                    f"}}\n"
                )
            # setter
            if set_name := method_ani_info.set_name:
                target.write(
                    f"set {set_name}({sts_real_params_str}) {{\n"
                    f"    return {sts_native_call};\n"
                    f"}}\n"
                )
