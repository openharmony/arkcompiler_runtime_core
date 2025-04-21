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
    StructFieldANIInfo,
    TypeANIInfo,
    UnionANIInfo,
    UnionFieldANIInfo,
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
from taihe.semantics.types import Type
from taihe.utils.analyses import AnalysisManager
from taihe.utils.outputs import OutputManager, STSOutputBuffer


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
            ns_dict.setdefault(pkg_ani_info.module_name, Namespace()).add_path(
                pkg_ani_info.sts_ns_parts, pkg
            )
        for module, ns in ns_dict.items():
            self.gen_module_file(module, ns)

    def gen_module_file(self, module: str, ns: Namespace):
        module_sts_file = f"{module}.ets"
        target = STSOutputBuffer.create(self.tm, module_sts_file)
        self.gen_module_injected_codes(ns, target)
        self.gen_namespace(ns, target)

    def gen_module_injected_codes(self, ns: Namespace, target: STSOutputBuffer):
        for pkg in ns.packages:
            pkg_ani_info = PackageANIInfo.get(self.am, pkg)
            for injected in pkg_ani_info.module_injected_codes:
                target.writeln(injected)
        for _, child_ns in ns.children.items():
            self.gen_module_injected_codes(child_ns, target)

    def gen_namespace(self, ns: Namespace, target: STSOutputBuffer):
        for pkg in ns.packages:
            self.gen_package(pkg, target)
        for child_ns_name, child_ns in ns.children.items():
            target.writeln(
                f"export namespace {child_ns_name} {{",
            )
            with target.indent_manager.offset(4):
                self.gen_namespace(child_ns, target)
            target.writeln(
                f"}}",
            )
        # TODO: BigInt
        self.gen_utils(target)

    def stat_on_off_funcs(
        self,
        funcs: list[GlobFuncDecl],
    ):
        glob_func_on_off_map: dict[
            tuple[str, tuple[str, ...]],
            list[tuple[str, GlobFuncDecl]],
        ] = {}
        for func in funcs:
            func_ani_info = GlobFuncANIInfo.get(self.am, func)
            if func_ani_info.sts_func_name is None:
                continue
            if func_ani_info.on_off_type is None:
                continue
            func_name = func_ani_info.sts_func_name
            type_name = func_ani_info.on_off_type
            sts_params_ty: list[str] = []
            for sts_param in func_ani_info.sts_params:
                ty_ani_info = TypeANIInfo.get(self.am, sts_param.ty_ref.resolved_ty)
                sts_params_ty.append(ty_ani_info.type_desc)
            glob_func_on_off_map.setdefault(
                (func_name, tuple(sts_params_ty)), []
            ).append((type_name, func))
        return glob_func_on_off_map

    def stat_good_on_off_funcs(
        self,
        funcs: list[GlobFuncDecl],
    ):
        on_off_funcs = self.stat_on_off_funcs(funcs)
        return [
            (method_name, type_name, method)
            for (method_name, _), method_list in on_off_funcs.items()
            if len(method_list) == 1
            for type_name, method in method_list
        ]

    def stat_bad_on_off_funcs(
        self,
        funcs: list[GlobFuncDecl],
    ):
        on_off_funcs = self.stat_on_off_funcs(funcs)
        bad_on_off_funcs: dict[str, list[tuple[str, GlobFuncDecl]]] = {}
        for (method_name, _), method_list in on_off_funcs.items():
            if len(method_list) <= 1:
                continue
            for type_name, method in method_list:
                bad_on_off_funcs.setdefault(method_name, []).append((type_name, method))
        return bad_on_off_funcs

    def stat_on_off_methods(
        self,
        methods: list[IfaceMethodDecl],
    ):
        method_on_off_map: dict[
            tuple[str, tuple[str, ...]],
            list[tuple[str, IfaceMethodDecl]],
        ] = {}
        for method in methods:
            method_ani_info = IfaceMethodANIInfo.get(self.am, method)
            if method_ani_info.sts_method_name is None:
                continue
            if method_ani_info.on_off_type is None:
                continue
            method_name = method_ani_info.sts_method_name
            type_name = method_ani_info.on_off_type
            sts_params_ty: list[str] = []
            for sts_param in method_ani_info.sts_params:
                ty_ani_info = TypeANIInfo.get(self.am, sts_param.ty_ref.resolved_ty)
                sts_params_ty.append(ty_ani_info.type_desc)
            method_on_off_map.setdefault(
                (method_name, tuple(sts_params_ty)), []
            ).append((type_name, method))
        return method_on_off_map

    def stat_good_on_off_methods(
        self,
        methods: list[IfaceMethodDecl],
    ):
        on_off_methods = self.stat_on_off_methods(methods)
        return [
            (method_name, type_name, method)
            for (method_name, _), method_list in on_off_methods.items()
            if len(method_list) == 1
            for type_name, method in method_list
        ]

    def stat_bad_on_off_methods(
        self,
        methods: list[IfaceMethodDecl],
    ):
        on_off_methods = self.stat_on_off_methods(methods)
        bad_on_off_methods: dict[str, list[tuple[str, IfaceMethodDecl]]] = {}
        for (method_name, _), method_list in on_off_methods.items():
            if len(method_list) <= 1:
                continue
            for type_name, method in method_list:
                bad_on_off_methods.setdefault(method_name, []).append(
                    (type_name, method)
                )
        return bad_on_off_methods

    def gen_package(self, pkg: PackageDecl, target: STSOutputBuffer):
        # TODO: hack inject
        pkg_ani_info = PackageANIInfo.get(self.am, pkg)
        for injected in pkg_ani_info.injected_codes:
            target.writeln(injected)

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
            target.writeln(
                f"native function {func_ani_info.sts_native_name}({sts_native_params_str}): {sts_return_ty_name};",
            )

    def gen_global_funcs(
        self,
        pkg: PackageDecl,
        funcs: list[GlobFuncDecl],
        target: STSOutputBuffer,
    ):
        # good on_off
        good_on_off_funcs = self.stat_good_on_off_funcs(funcs)
        for func_name, type_name, func in good_on_off_funcs:
            func_ani_info = GlobFuncANIInfo.get(self.am, func)
            sts_params = ["type: String"]
            sts_args = []
            for sts_param in func_ani_info.sts_params:
                type_ani_info = TypeANIInfo.get(self.am, sts_param.ty_ref.resolved_ty)
                sts_params.append(
                    f"{sts_param.name}: {type_ani_info.sts_type_in(pkg, target)}"
                )
                sts_args.append(sts_param.name)
            sts_params_str = ", ".join(sts_params)
            sts_native_call = func_ani_info.call_native_with(sts_args)
            if return_ty_ref := func.return_ty_ref:
                type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
                sts_return_ty_name = type_ani_info.sts_type_in(pkg, target)
            else:
                sts_return_ty_name = "void"
            target.writeln(
                f"export function {func_name}({sts_params_str}): {sts_return_ty_name} {{",
            )
            target.writeln(
                f"    if (type !== '{type_name}') {{",
                f"        throw new Error(`Invalid type: ${{type}}`);",
                f"    }}",
                f"    return {sts_native_call};",
            )
            target.writeln(
                f"}}",
            )
        # bad on_off
        bad_on_off_funcs = self.stat_bad_on_off_funcs(funcs)
        for func_name, func_list in bad_on_off_funcs.items():
            max_sts_params = max(
                len(GlobFuncANIInfo.get(self.am, func).sts_params)
                for _, func in func_list
            )
            sts_params = ["type: Object | String"]
            sts_args = []
            for index in range(max_sts_params):
                param_types = []
                for _, func in func_list:
                    func_ani_info = GlobFuncANIInfo.get(self.am, func)
                    if index < len(func_ani_info.sts_params):
                        param_ty = func_ani_info.sts_params[index].ty_ref.resolved_ty
                        type_ani_info = TypeANIInfo.get(self.am, param_ty)
                        param_types.append(type_ani_info.sts_type_in(pkg, target))
                param_types_str = " | ".join(["Object", *param_types])
                param_name = f"p_{index}"
                sts_params.append(f"{param_name}?: {param_types_str}")
                sts_args.append(param_name)
            sts_params_str = ", ".join(sts_params)
            target.writeln(
                f"export function {func_name}({sts_params_str}): void {{",
            )
            target.writeln(
                f"    switch (type as String) {{",
            )
            for type_name, func in func_list:
                func_ani_info = GlobFuncANIInfo.get(self.am, func)
                sts_args_fix = []
                for sts_arg, param in zip(
                    sts_args, func_ani_info.sts_params, strict=False
                ):
                    type_ani_info = TypeANIInfo.get(self.am, param.ty_ref.resolved_ty)
                    sts_args_fix.append(
                        f"{sts_arg} as {type_ani_info.sts_type_in(pkg, target)}"
                    )
                sts_native_call = func_ani_info.call_native_with(sts_args_fix)
                target.writeln(
                    f'        case "{type_name}": return {sts_native_call};',
                )
            target.writeln(
                f"        default: throw new Error(`Unknown type: ${{type}}`);",
                f"    }}",
            )
            target.writeln(
                f"}}",
            )
        # other
        for func in funcs:
            func_ani_info = GlobFuncANIInfo.get(self.am, func)
            sts_params = []
            sts_args = []
            for sts_param in func_ani_info.sts_params:
                type_ani_info = TypeANIInfo.get(self.am, sts_param.ty_ref.resolved_ty)
                sts_params.append(
                    f"{sts_param.name}: {type_ani_info.sts_type_in(pkg, target)}"
                )
                sts_args.append(sts_param.name)
            sts_params_str = ", ".join(sts_params)
            sts_native_call = func_ani_info.call_native_with(sts_args)
            if return_ty_ref := func.return_ty_ref:
                type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
                sts_return_ty_name = type_ani_info.sts_type_in(pkg, target)
                sts_resolved_ty_name = type_ani_info.sts_type_in(pkg, target)
            else:
                sts_return_ty_name = "void"
                sts_resolved_ty_name = "undefined"
            # normal
            if (
                sts_func_name := func_ani_info.sts_func_name
            ) is not None and func_ani_info.on_off_type is None:
                target.writeln(
                    f"export function {sts_func_name}({sts_params_str}): {sts_return_ty_name} {{",
                )
                target.writeln(
                    f"    return {sts_native_call};",
                )
                target.writeln(
                    f"}}",
                )
                # promise
                if (sts_promise_name := func_ani_info.sts_promise_name) is not None:
                    target.writeln(
                        f"export function {sts_promise_name}({sts_params_str}): Promise<{sts_return_ty_name}> {{",
                        f"    return new Promise<{sts_return_ty_name}>((resolve, reject): void => {{",
                        f"        taskpool.execute((): {sts_return_ty_name} => {{",
                    )
                    target.writeln(
                        f"            return {sts_native_call};",
                    )
                    target.writeln(
                        f"        }})",
                        f"        .then((ret: NullishType): void => {{",
                        f"            resolve(ret as {sts_resolved_ty_name});",
                        f"        }})",
                        f"        .catch((ret: NullishType): void => {{",
                        f"            reject(ret as Error);",
                        f"        }});",
                        f"    }});",
                        f"}}",
                    )
                # async
                if (sts_async_name := func_ani_info.sts_async_name) is not None:
                    callback_param = f"callback: __AsyncCallback<{sts_return_ty_name}>"
                    sts_params_with_cb_str = ", ".join([*sts_params, callback_param])
                    target.writeln(
                        f"export function {sts_async_name}({sts_params_with_cb_str}): void {{",
                        f"    taskpool.execute((): {sts_return_ty_name} => {{",
                    )
                    target.writeln(
                        f"        return {sts_native_call};",
                    )
                    target.writeln(
                        f"    }})",
                        f"    .then((ret: NullishType): void => {{",
                        f"        callback(new Error(), ret as {sts_resolved_ty_name});",
                        f"    }})",
                        f"    .catch((ret: NullishType): void => {{",
                        f"        callback(ret as Error, undefined);",
                        f"    }});",
                        f"}}",
                    )

    def gen_enum(
        self,
        pkg: PackageDecl,
        enum: EnumDecl,
        target: STSOutputBuffer,
    ):
        enum_ani_info = EnumANIInfo.get(self.am, enum)
        if enum_ani_info.const:
            type_ani_info = TypeANIInfo.get(self.am, enum.ty_ref.resolved_ty)
            for item in enum.items:
                target.writeln(
                    f"export const {item.name}: {type_ani_info.sts_type_in(pkg, target)} = {dumps(item.value)};",
                )
            return
        target.writeln(
            f"export enum {enum_ani_info.sts_type_name} {{",
        )
        with target.indent_manager.offset(4):
            for item in enum.items:
                if item.value is None:
                    target.writeln(
                        f"{item.name},",
                    )
                else:
                    target.writeln(
                        f"{item.name} = {dumps(item.value)},",
                    )
        target.writeln(
            f"}}",
        )

    def gen_union(
        self,
        pkg: PackageDecl,
        union: UnionDecl,
        target: STSOutputBuffer,
    ):
        union_ani_info = UnionANIInfo.get(self.am, union)
        sts_types = []
        for field in union.fields:
            field_ani_info = UnionFieldANIInfo.get(self.am, field)
            match field_ani_info.field_ty:
                case "null":
                    sts_types.append("null")
                case "undefined":
                    sts_types.append("undefined")
                case field_ty if isinstance(field_ty, Type):
                    ty_ani_info = TypeANIInfo.get(self.am, field_ty)
                    sts_types.append(ty_ani_info.sts_type_in(pkg, target))
        sts_types_str = " | ".join(sts_types)
        target.writeln(
            f"export type {union_ani_info.sts_type_name} = {sts_types_str};",
        )

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
            parent_ty = parent.ty_ref.resolved_ty
            parent_ani_info = TypeANIInfo.get(self.am, parent_ty)
            parents.append(parent_ani_info.sts_type_in(pkg, target))
        extends_str = " extends " + ", ".join(parents) if parents else ""
        target.writeln(
            f"export interface {struct_ani_info.sts_type_name}{extends_str} {{",
        )
        with target.indent_manager.offset(4):
            # TODO: hack inject
            for injected in struct_ani_info.interface_injected_codes:
                target.writeln(injected)
            for field in struct_ani_info.sts_fields:
                field_ani_info = StructFieldANIInfo.get(self.am, field)
                readonly_str = "readonly " if field_ani_info.readonly else ""
                ty_ani_info = TypeANIInfo.get(self.am, field.ty_ref.resolved_ty)
                target.writeln(
                    f"{readonly_str}{field.name}: {ty_ani_info.sts_type_in(pkg, target)};",
                )
        target.writeln(
            f"}}",
        )

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
                parent_ty = parent.ty_ref.resolved_ty
                parent_ani_info = TypeANIInfo.get(self.am, parent_ty)
                parents.append(parent_ani_info.sts_type_in(pkg, target))
            implements_str = " implements " + ", ".join(parents) if parents else ""
            target.writeln(
                f"export class {struct_ani_info.sts_impl_name}{implements_str} {{",
            )
        else:
            target.writeln(
                f"class {struct_ani_info.sts_impl_name} implements {struct_ani_info.sts_type_name} {{",
            )
        with target.indent_manager.offset(4):
            # TODO: hack inject
            for injected in struct_ani_info.class_injected_codes:
                target.writeln(injected)
            for parts in struct_ani_info.sts_final_fields:
                final = parts[-1]
                final_ani_info = StructFieldANIInfo.get(self.am, final)
                readonly_str = "readonly " if final_ani_info.readonly else ""
                ty_ani_info = TypeANIInfo.get(self.am, final.ty_ref.resolved_ty)
                target.writeln(
                    f"{readonly_str}{final.name}: {ty_ani_info.sts_type_in(pkg, target)};",
                )
            target.writeln(
                f"constructor(",
            )
            for parts in struct_ani_info.sts_final_fields:
                final = parts[-1]
                ty_ani_info = TypeANIInfo.get(self.am, final.ty_ref.resolved_ty)
                target.writeln(
                    f"    {final.name}: {ty_ani_info.sts_type_in(pkg, target)},",
                )
            target.writeln(
                f") {{",
            )
            for parts in struct_ani_info.sts_final_fields:
                final = parts[-1]
                target.writeln(
                    f"    this.{final.name} = {final.name};",
                )
            target.writeln(
                f"}}",
            )
        target.writeln(
            f"}}",
        )

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
            parent_ty = parent.ty_ref.resolved_ty
            parent_ani_info = TypeANIInfo.get(self.am, parent_ty)
            parents.append(parent_ani_info.sts_type_in(pkg, target))
        extends_str = " extends " + ", ".join(parents) if parents else ""
        target.writeln(
            f"export interface {iface_ani_info.sts_type_name}{extends_str} {{",
        )
        with target.indent_manager.offset(4):
            # TODO: hack inject
            for injected in iface_ani_info.interface_injected_codes:
                target.writeln(injected)
            self.gen_iface_methods_decl(pkg, iface.methods, target)
        target.writeln(
            f"}}",
        )

    def gen_iface_methods_decl(
        self,
        pkg: PackageDecl,
        methods: list[IfaceMethodDecl],
        target: STSOutputBuffer,
    ):
        # good on_off
        good_on_off_methods = self.stat_good_on_off_methods(methods)
        for method_name, _, method in good_on_off_methods:
            method_ani_info = IfaceMethodANIInfo.get(self.am, method)
            sts_params = ["type: String"]
            for sts_param in method_ani_info.sts_params:
                type_ani_info = TypeANIInfo.get(self.am, sts_param.ty_ref.resolved_ty)
                sts_params.append(
                    f"{sts_param.name}: {type_ani_info.sts_type_in(pkg, target)}"
                )
            sts_params_str = ", ".join(sts_params)
            if return_ty_ref := method.return_ty_ref:
                type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
                sts_return_ty_name = type_ani_info.sts_type_in(pkg, target)
            else:
                sts_return_ty_name = "void"
            target.writeln(
                f"{method_name}({sts_params_str}): {sts_return_ty_name};",
            )
        # bad on_off
        bad_on_off_methods = self.stat_bad_on_off_methods(methods)
        for method_name, method_list in bad_on_off_methods.items():
            max_sts_params = max(
                len(IfaceMethodANIInfo.get(self.am, method).sts_params)
                for _, method in method_list
            )
            sts_params = ["type: Object | String"]
            for index in range(max_sts_params):
                param_types = []
                for _, method in method_list:
                    method_ani_info = IfaceMethodANIInfo.get(self.am, method)
                    if index < len(method_ani_info.sts_params):
                        param_ty = method_ani_info.sts_params[index].ty_ref.resolved_ty
                        type_ani_info = TypeANIInfo.get(self.am, param_ty)
                        param_types.append(type_ani_info.sts_type_in(pkg, target))
                param_types_str = " | ".join(["Object", *param_types])
                param_name = f"p_{index}"
                sts_params.append(f"{param_name}?: {param_types_str}")
            sts_params_str = ", ".join(sts_params)
            target.writeln(
                f"{method_name}({sts_params_str}): void;",
            )
        # other
        for method in methods:
            method_ani_info = IfaceMethodANIInfo.get(self.am, method)
            sts_params = []
            for sts_param in method_ani_info.sts_params:
                type_ani_info = TypeANIInfo.get(self.am, sts_param.ty_ref.resolved_ty)
                sts_params.append(
                    f"{sts_param.name}: {type_ani_info.sts_type_in(pkg, target)}"
                )
            sts_params_str = ", ".join(sts_params)
            if return_ty_ref := method.return_ty_ref:
                type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
                sts_return_ty_name = type_ani_info.sts_type_in(pkg, target)
            else:
                sts_return_ty_name = "void"
            # normal
            if (
                sts_method_name := method_ani_info.sts_method_name
            ) is not None and method_ani_info.on_off_type is None:
                target.writeln(
                    f"{sts_method_name}({sts_params_str}): {sts_return_ty_name};",
                )
                # promise
                if (sts_promise_name := method_ani_info.sts_promise_name) is not None:
                    target.writeln(
                        f"{sts_promise_name}({sts_params_str}): Promise<{sts_return_ty_name}>;",
                    )
                # async
                if (sts_async_name := method_ani_info.sts_async_name) is not None:
                    callback_param = f"callback: __AsyncCallback<{sts_return_ty_name}>"
                    sts_params_with_cb_str = ", ".join([*sts_params, callback_param])
                    target.writeln(
                        f"{sts_async_name}({sts_params_with_cb_str}): void;",
                    )
            # getter
            if (get_name := method_ani_info.get_name) is not None:
                target.writeln(
                    f"get {get_name}({sts_params_str}): {sts_return_ty_name};",
                )
            # setter
            if (set_name := method_ani_info.set_name) is not None:
                target.writeln(
                    f"set {set_name}({sts_params_str});",
                )

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
                parent_ty = parent.ty_ref.resolved_ty
                parent_ani_info = TypeANIInfo.get(self.am, parent_ty)
                parents.append(parent_ani_info.sts_type_in(pkg, target))
            implements_str = " implements " + ", ".join(parents) if parents else ""
            target.writeln(
                f"export class {iface_ani_info.sts_impl_name}{implements_str} {{",
            )
        else:
            target.writeln(
                f"class {iface_ani_info.sts_impl_name} implements {iface_ani_info.sts_type_name} {{",
            )
        with target.indent_manager.offset(4):
            # TODO: hack inject
            for injected in iface_ani_info.class_injected_codes:
                target.writeln(injected)
            target.writeln(
                f"private _vtbl_ptr: long;",
                f"private _data_ptr: long;",
                f"private static native _finalize(data_ptr: long): void;",
                f"private static _registry = new FinalizationRegistry<long>((data_ptr: long) => {{ {iface_ani_info.sts_impl_name}._finalize(data_ptr); }});",
                f"private constructor(_vtbl_ptr: long, _data_ptr: long) {{",
                f"    this._vtbl_ptr = _vtbl_ptr;",
                f"    this._data_ptr = _data_ptr;",
                f"    {iface_ani_info.sts_impl_name}._registry.register(this, this._data_ptr)",
                f"}}",
            )
            ctors = ctors_map.get(iface.name, [])
            for tor in ctors:
                ctor_ani_info = GlobFuncANIInfo.get(self.am, tor)
                sts_params = []
                sts_args = []
                for sts_param in ctor_ani_info.sts_params:
                    type_ani_info = TypeANIInfo.get(
                        self.am, sts_param.ty_ref.resolved_ty
                    )
                    sts_params.append(
                        f"{sts_param.name}: {type_ani_info.sts_type_in(pkg, target)}"
                    )
                    sts_args.append(sts_param.name)
                sts_params_str = ", ".join(sts_params)
                sts_native_call = ctor_ani_info.call_native_with(sts_args)
                target.writeln(
                    f"constructor({sts_params_str}) {{",
                    f"    let temp = {sts_native_call} as {iface_ani_info.sts_impl_name};",
                    f"    this._data_ptr = temp._data_ptr;",
                    f"    this._vtbl_ptr = temp._vtbl_ptr;",
                    f"}}",
                )
            self.gen_static_funcs(pkg, statics_map.get(iface.name, []), target)
            iface_abi_info = IfaceABIInfo.get(self.am, iface)
            for ancestor in iface_abi_info.ancestor_dict:
                self.gen_native_methods(pkg, ancestor.methods, target)
                self.gen_iface_methods(pkg, ancestor.methods, target)
        target.writeln(
            f"}}",
        )

    def gen_static_funcs(
        self,
        pkg: PackageDecl,
        funcs: list[GlobFuncDecl],
        target: STSOutputBuffer,
    ):
        # good on_off
        good_on_off_funcs = self.stat_good_on_off_funcs(funcs)
        for func_name, type_name, func in good_on_off_funcs:
            func_ani_info = GlobFuncANIInfo.get(self.am, func)
            sts_params = ["type: String"]
            sts_args = []
            for sts_param in func_ani_info.sts_params:
                type_ani_info = TypeANIInfo.get(self.am, sts_param.ty_ref.resolved_ty)
                sts_params.append(
                    f"{sts_param.name}: {type_ani_info.sts_type_in(pkg, target)}"
                )
                sts_args.append(sts_param.name)
            sts_params_str = ", ".join(sts_params)
            sts_native_call = func_ani_info.call_native_with(sts_args)
            if return_ty_ref := func.return_ty_ref:
                type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
                sts_return_ty_name = type_ani_info.sts_type_in(pkg, target)
            else:
                sts_return_ty_name = "void"
            target.writeln(
                f"static {func_name}({sts_params_str}): {sts_return_ty_name} {{",
            )
            target.writeln(
                f"    if (type !== '{type_name}') {{",
                f"        throw new Error(`Invalid type: ${{type}}`);",
                f"    }}",
                f"    return {sts_native_call};",
            )
            target.writeln(
                f"}}",
            )
        # bad on_off
        bad_on_off_funcs = self.stat_bad_on_off_funcs(funcs)
        for func_name, func_list in bad_on_off_funcs.items():
            max_sts_params = max(
                len(GlobFuncANIInfo.get(self.am, func).sts_params)
                for _, func in func_list
            )
            sts_params = ["type: Object | String"]
            sts_args = []
            for index in range(max_sts_params):
                param_types = []
                for _, func in func_list:
                    func_ani_info = GlobFuncANIInfo.get(self.am, func)
                    if index < len(func_ani_info.sts_params):
                        param_ty = func_ani_info.sts_params[index].ty_ref.resolved_ty
                        type_ani_info = TypeANIInfo.get(self.am, param_ty)
                        param_types.append(type_ani_info.sts_type_in(pkg, target))
                param_types_str = " | ".join(["Object", *param_types])
                param_name = f"p_{index}"
                sts_params.append(f"{param_name}?: {param_types_str}")
                sts_args.append(param_name)
            sts_params_str = ", ".join(sts_params)
            target.writeln(
                f"static {func_name}({sts_params_str}): void {{",
            )
            target.writeln(
                f"    switch (type as String) {{",
            )
            for type_name, func in func_list:
                func_ani_info = GlobFuncANIInfo.get(self.am, func)
                sts_args_fix = []
                for sts_arg, param in zip(
                    sts_args, func_ani_info.sts_params, strict=False
                ):
                    type_ani_info = TypeANIInfo.get(self.am, param.ty_ref.resolved_ty)
                    sts_args_fix.append(
                        f"{sts_arg} as {type_ani_info.sts_type_in(pkg, target)}"
                    )
                sts_native_call = func_ani_info.call_native_with(sts_args_fix)
                target.writeln(
                    f'        case "{type_name}": return {sts_native_call};',
                )
            target.writeln(
                f"        default: throw new Error(`Unknown type: ${{type}}`);",
                f"    }}",
            )
            target.writeln(
                f"}}",
            )
        # other
        for func in funcs:
            func_ani_info = GlobFuncANIInfo.get(self.am, func)
            sts_params = []
            sts_args = []
            for sts_param in func_ani_info.sts_params:
                type_ani_info = TypeANIInfo.get(self.am, sts_param.ty_ref.resolved_ty)
                sts_params.append(
                    f"{sts_param.name}: {type_ani_info.sts_type_in(pkg, target)}"
                )
                sts_args.append(sts_param.name)
            sts_params_str = ", ".join(sts_params)
            sts_native_call = func_ani_info.call_native_with(sts_args)
            if return_ty_ref := func.return_ty_ref:
                type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
                sts_return_ty_name = type_ani_info.sts_type_in(pkg, target)
                sts_resolved_ty_name = type_ani_info.sts_type_in(pkg, target)
            else:
                sts_return_ty_name = "void"
                sts_resolved_ty_name = "undefined"
            # normal
            if (
                sts_func_name := func_ani_info.sts_func_name
            ) is not None and func_ani_info.on_off_type is None:
                target.writeln(
                    f"static {sts_func_name}({sts_params_str}): {sts_return_ty_name} {{",
                )
                target.writeln(
                    f"    return {sts_native_call};",
                )
                target.writeln(
                    f"}}",
                )
                # promise
                if (sts_promise_name := func_ani_info.sts_promise_name) is not None:
                    target.writeln(
                        f"static {sts_promise_name}({sts_params_str}): Promise<{sts_return_ty_name}> {{",
                        f"    return new Promise<{sts_return_ty_name}>((resolve, reject): void => {{",
                        f"        taskpool.execute((): {sts_return_ty_name} => {{",
                    )
                    target.writeln(
                        f"            return {sts_native_call};",
                    )
                    target.writeln(
                        f"        }})",
                        f"        .then((ret: NullishType): void => {{",
                        f"            resolve(ret as {sts_resolved_ty_name});",
                        f"        }})",
                        f"        .catch((ret: NullishType): void => {{",
                        f"            reject(ret as Error);",
                        f"        }});",
                        f"    }});",
                        f"}}",
                    )
                # async
                if (sts_async_name := func_ani_info.sts_async_name) is not None:
                    callback_param = f"callback: __AsyncCallback<{sts_return_ty_name}>"
                    sts_params_with_cb_str = ", ".join([*sts_params, callback_param])
                    target.writeln(
                        f"static {sts_async_name}({sts_params_with_cb_str}): void {{",
                        f"    taskpool.execute((): {sts_return_ty_name} => {{",
                    )
                    target.writeln(
                        f"        return {sts_native_call};",
                    )
                    target.writeln(
                        f"    }})",
                        f"    .then((ret: NullishType): void => {{",
                        f"        callback(new Error(), ret as {sts_resolved_ty_name});",
                        f"    }})",
                        f"    .catch((ret: NullishType): void => {{",
                        f"        callback(ret as Error, undefined);",
                        f"    }});",
                        f"}}",
                    )
            # getter
            if (get_name := func_ani_info.get_name) is not None:
                target.writeln(
                    f"static get {get_name}({sts_params_str}): {sts_return_ty_name} {{",
                    f"    return {sts_native_call};",
                    f"}}",
                )
            # setter
            if (set_name := func_ani_info.set_name) is not None:
                target.writeln(
                    f"static set {set_name}({sts_params_str}) {{",
                    f"    return {sts_native_call};",
                    f"}}",
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
            target.writeln(
                f"native {method_ani_info.sts_native_name}({sts_native_params_str}): {sts_return_ty_name};",
            )

    def gen_iface_methods(
        self,
        pkg: PackageDecl,
        methods: list[IfaceMethodDecl],
        target: STSOutputBuffer,
    ):
        # good on_off
        good_on_off_methods = self.stat_good_on_off_methods(methods)
        for method_name, type_name, method in good_on_off_methods:
            method_ani_info = IfaceMethodANIInfo.get(self.am, method)
            sts_params = ["type: String"]
            sts_args = []
            for sts_param in method_ani_info.sts_params:
                type_ani_info = TypeANIInfo.get(self.am, sts_param.ty_ref.resolved_ty)
                sts_params.append(
                    f"{sts_param.name}: {type_ani_info.sts_type_in(pkg, target)}"
                )
                sts_args.append(sts_param.name)
            sts_params_str = ", ".join(sts_params)
            sts_native_call = method_ani_info.call_native_with("this", sts_args)
            if return_ty_ref := method.return_ty_ref:
                type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
                sts_return_ty_name = type_ani_info.sts_type_in(pkg, target)
            else:
                sts_return_ty_name = "void"
            target.writeln(
                f"{method_name}({sts_params_str}): {sts_return_ty_name} {{",
            )
            target.writeln(
                f"    if (type !== '{type_name}') {{",
                f"        throw new Error(`Invalid type: ${{type}}`);",
                f"    }}",
                f"    return {sts_native_call};",
            )
            target.writeln(
                f"}}",
            )
        # bad on_off
        bad_on_off_methods = self.stat_bad_on_off_methods(methods)
        for method_name, method_list in bad_on_off_methods.items():
            max_sts_params = max(
                len(IfaceMethodANIInfo.get(self.am, method).sts_params)
                for _, method in method_list
            )
            sts_params = ["type: Object | String"]
            sts_args = []
            for index in range(max_sts_params):
                param_types = []
                for _, method in method_list:
                    method_ani_info = IfaceMethodANIInfo.get(self.am, method)
                    if index < len(method_ani_info.sts_params):
                        param_ty = method_ani_info.sts_params[index].ty_ref.resolved_ty
                        type_ani_info = TypeANIInfo.get(self.am, param_ty)
                        param_types.append(type_ani_info.sts_type_in(pkg, target))
                param_types_str = " | ".join(["Object", *param_types])
                param_name = f"p_{index}"
                sts_params.append(f"{param_name}?: {param_types_str}")
                sts_args.append(param_name)
            sts_params_str = ", ".join(sts_params)
            target.writeln(
                f"{method_name}({sts_params_str}): void {{",
            )
            target.writeln(
                f"    switch (type as String) {{",
            )
            for type_name, method in method_list:
                method_ani_info = IfaceMethodANIInfo.get(self.am, method)
                sts_args_fix = []
                for sts_arg, param in zip(
                    sts_args, method_ani_info.sts_params, strict=False
                ):
                    type_ani_info = TypeANIInfo.get(self.am, param.ty_ref.resolved_ty)
                    sts_args_fix.append(
                        f"{sts_arg} as {type_ani_info.sts_type_in(pkg, target)}"
                    )
                sts_native_call = method_ani_info.call_native_with("this", sts_args_fix)
                target.writeln(
                    f'        case "{type_name}": return {sts_native_call};',
                )
            target.writeln(
                f"        default: throw new Error(`Unknown type: ${{type}}`);",
                f"    }}",
            )
            target.writeln(
                f"}}",
            )
        # other
        for method in methods:
            method_ani_info = IfaceMethodANIInfo.get(self.am, method)
            sts_params = []
            sts_args = []
            for sts_param in method_ani_info.sts_params:
                type_ani_info = TypeANIInfo.get(self.am, sts_param.ty_ref.resolved_ty)
                sts_params.append(
                    f"{sts_param.name}: {type_ani_info.sts_type_in(pkg, target)}"
                )
                sts_args.append(sts_param.name)
            sts_params_str = ", ".join(sts_params)
            sts_native_call = method_ani_info.call_native_with("this", sts_args)
            if return_ty_ref := method.return_ty_ref:
                type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
                sts_return_ty_name = type_ani_info.sts_type_in(pkg, target)
                sts_resolved_ty_name = type_ani_info.sts_type_in(pkg, target)
            else:
                sts_return_ty_name = "void"
                sts_resolved_ty_name = "undefined"
            # normal
            if (
                sts_method_name := method_ani_info.sts_method_name
            ) is not None and method_ani_info.on_off_type is None:
                target.writeln(
                    f"{sts_method_name}({sts_params_str}): {sts_return_ty_name} {{",
                )
                target.writeln(
                    f"    return {sts_native_call};",
                )
                target.writeln(
                    f"}}",
                )
                # promise
                if (sts_promise_name := method_ani_info.sts_promise_name) is not None:
                    target.writeln(
                        f"{sts_promise_name}({sts_params_str}): Promise<{sts_return_ty_name}> {{",
                        f"    return new Promise<{sts_return_ty_name}>((resolve, reject): void => {{",
                        f"        taskpool.execute((): {sts_return_ty_name} => {{",
                    )
                    target.writeln(
                        f"            return {sts_native_call};",
                    )
                    target.writeln(
                        f"        }})",
                        f"        .then((ret: NullishType): void => {{",
                        f"            resolve(ret as {sts_resolved_ty_name});",
                        f"        }})",
                        f"        .catch((ret: NullishType): void => {{",
                        f"            reject(ret as Error);",
                        f"        }});",
                        f"    }});",
                        f"}}",
                    )
                # async
                if (sts_async_name := method_ani_info.sts_async_name) is not None:
                    callback_param = f"callback: __AsyncCallback<{sts_return_ty_name}>"
                    sts_params_with_cb_str = ", ".join([*sts_params, callback_param])
                    target.writeln(
                        f"{sts_async_name}({sts_params_with_cb_str}): void {{",
                        f"    taskpool.execute((): {sts_return_ty_name} => {{",
                    )
                    target.writeln(
                        f"        return {sts_native_call};",
                    )
                    target.writeln(
                        f"    }})",
                        f"    .then((ret: NullishType): void => {{",
                        f"        callback(new Error(), ret as {sts_resolved_ty_name});",
                        f"    }})",
                        f"    .catch((ret: NullishType): void => {{",
                        f"        callback(ret as Error, undefined);",
                        f"    }});",
                        f"}}",
                    )
            # getter
            if (get_name := method_ani_info.get_name) is not None:
                target.writeln(
                    f"get {get_name}({sts_params_str}): {sts_return_ty_name} {{",
                    f"    return {sts_native_call};",
                    f"}}",
                )
            # setter
            if (set_name := method_ani_info.set_name) is not None:
                target.writeln(
                    f"set {set_name}({sts_params_str}) {{",
                    f"    return {sts_native_call};",
                    f"}}",
                )

    def gen_utils(
        self,
        target: STSOutputBuffer,
    ):
        target.writeln(
            "type __AsyncCallback<T> = (err: Error, result: T | undefined) => void;",
        )
        target.writeln(
            "function __fromArrayBufferToBigInt(arr: ArrayBuffer): BigInt {",
            "    let res: BigInt = 0n;",
            "    for (let i: int = 0; i < arr.getByteLength(); i++) {",
            "        res |= BigInt(arr.at(i) as long & 0xff) << BigInt(i * 8);",
            "    }",
            "    let m: int = arr.getByteLength();",
            "    if (arr.at(m - 1) < 0) {",
            "        res |= -1n << BigInt(m * 8 - 1);",
            "    }",
            "    return res;",
            "}",
        )
        target.writeln(
            "function __fromBigIntToArrayBuffer(val: BigInt, blk: int): ArrayBuffer {",
            "    let n_7 = BigInt(blk * 8 - 1);",
            "    let n_8 = BigInt(blk * 8);",
            "    let tmp: BigInt = val;",
            "    let n: int = 0;",
            "    while (true) {",
            "        n += blk;",
            "        let t_7 = tmp >> n_7;",
            "        let t_8 = tmp >> n_8;",
            "        if (t_7 == t_8) {",
            "            break;",
            "        }",
            "        tmp = t_8;",
            "    }",
            "    let buf = new ArrayBuffer(n);",
            "    for (let i: int = 0; i < n; i++) {",
            "        buf.set(i, (val & 255n).getLong() as byte)",
            "        val >>= 8n;",
            "    }",
            "    return buf;",
            "}",
        )