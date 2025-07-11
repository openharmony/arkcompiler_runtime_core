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

from enum import Enum
from json import dumps

from taihe.codegen.abi.analyses import (
    IfaceABIInfo,
)
from taihe.codegen.ani.analyses import (
    EnumANIInfo,
    GlobFuncANIInfo,
    IfaceANIInfo,
    IfaceMethodANIInfo,
    Namespace,
    PackageGroupANIInfo,
    StructANIInfo,
    StructFieldANIInfo,
    TypeANIInfo,
    UnionANIInfo,
    UnionFieldANIInfo,
)
from taihe.codegen.ani.writer import StsWriter
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
from taihe.utils.outputs import FileKind, OutputManager


class FuncKind(Enum):
    GLOBAL = "export function "
    STATIC = "static "
    INTERFACE = ""


class STSCodeGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager):
        self.om = om
        self.am = am

    def generate(self, pg: PackageGroup):
        pg_ani_info = PackageGroupANIInfo.get(self.am, pg)
        self.gen_ohos_base()
        for module, ns in pg_ani_info.module_dict.items():
            self.gen_module_file(module, ns)

    def gen_module_file(self, module: str, ns: Namespace):
        with StsWriter(
            self.om,
            f"{module}.ets",
            FileKind.ETS,
        ) as target:
            target.add_import_decl("@ohos.base", "AsyncCallback")
            target.add_import_decl("@ohos.base", "BusinessError")
            self.gen_namespace(ns, target)
            self.gen_utils(target)

    def gen_namespace(self, ns: Namespace, target: StsWriter):
        for head in ns.injected_heads:
            target.write_block(head)
        for code in ns.injected_codes:
            target.write_block(code)
        for pkg in ns.packages:
            self.gen_package(pkg, target)
        for child_ns_name, child_ns in ns.children.items():
            sts_decl = f"namespace {child_ns_name}"
            if child_ns.is_default:
                sts_decl = f"export default {sts_decl}"
            else:
                sts_decl = f"export {sts_decl}"
            with target.indented(
                f"{sts_decl} {{",
                f"}}",
            ):
                self.gen_namespace(child_ns, target)

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

    def gen_package(self, pkg: PackageDecl, target: StsWriter):
        self.gen_native_funcs(pkg.functions, target)
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
        self.gen_global_funcs(funcs, target)
        for enum in pkg.enums:
            self.gen_enum(enum, target)
        for union in pkg.unions:
            self.gen_union(union, target)
        for struct in pkg.structs:
            self.gen_struct_interface(struct, target)
        for struct in pkg.structs:
            self.gen_struct_class(struct, target)
        for iface in pkg.interfaces:
            self.gen_iface_interface(iface, target)
        for iface in pkg.interfaces:
            self.gen_iface_class(iface, target, statics_map, ctors_map)

    def gen_native_funcs(
        self,
        funcs: list[GlobFuncDecl],
        target: StsWriter,
    ):
        # native funcs
        for func in funcs:
            func_ani_info = GlobFuncANIInfo.get(self.am, func)
            sts_native_params = []
            for param in func.params:
                type_ani_info = TypeANIInfo.get(self.am, param.ty_ref.resolved_ty)
                sts_native_params.append(
                    f"{param.name}: {type_ani_info.sts_type_in(target)}"
                )
            sts_native_params_str = ", ".join(sts_native_params)
            if return_ty_ref := func.return_ty_ref:
                type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
                sts_return_ty_name = type_ani_info.sts_type_in(target)
            else:
                sts_return_ty_name = "void"
            target.writelns(
                f"native function {func_ani_info.sts_native_name}({sts_native_params_str}): {sts_return_ty_name};",
            )

    def gen_global_funcs(
        self,
        funcs: list[GlobFuncDecl],
        target: StsWriter,
    ):
        self.gen_global_good_on_off_funcs(funcs, target)
        self.gen_global_bad_on_off_funcs(funcs, target)
        self.gen_global_regular_funcs(funcs, target)

    def gen_global_good_on_off_funcs(
        self,
        funcs: list[GlobFuncDecl],
        target: StsWriter,
    ):
        good_on_off_funcs = self.stat_good_on_off_funcs(funcs)
        for func_name, type_name, func in good_on_off_funcs:
            func_ani_info = GlobFuncANIInfo.get(self.am, func)
            sts_params = ["type: String"]
            sts_args = []
            for sts_param in func_ani_info.sts_params:
                type_ani_info = TypeANIInfo.get(self.am, sts_param.ty_ref.resolved_ty)
                sts_params.append(
                    f"{sts_param.name}: {type_ani_info.sts_type_in(target)}"
                )
                sts_args.append(sts_param.name)
            sts_params_str = ", ".join(sts_params)
            sts_native_call = func_ani_info.call_native_with(sts_args)
            if return_ty_ref := func.return_ty_ref:
                type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
                sts_return_ty_name = type_ani_info.sts_type_in(target)
            else:
                sts_return_ty_name = "void"
            with target.indented(
                f"export function {func_name}({sts_params_str}): {sts_return_ty_name} {{",
                f"}}",
            ):
                with target.indented(
                    f"if (type !== '{type_name}') {{",
                    f"}}",
                ):
                    target.writelns(
                        f"throw new Error(`Invalid type: ${{type}}`);",
                    )
                target.writelns(
                    f"return {sts_native_call};",
                )

    def gen_global_bad_on_off_funcs(
        self,
        funcs: list[GlobFuncDecl],
        target: StsWriter,
    ):
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
                        param_types.append(type_ani_info.sts_type_in(target))
                param_types_str = " | ".join(["Object", *param_types])
                param_name = f"p_{index}"
                sts_params.append(f"{param_name}?: {param_types_str}")
                sts_args.append(param_name)
            sts_params_str = ", ".join(sts_params)
            with target.indented(
                f"export function {func_name}({sts_params_str}): void {{",
                f"}}",
            ):
                with target.indented(
                    f"switch (type as String) {{",
                    f"}}",
                    indent="",
                ):
                    for type_name, func in func_list:
                        func_ani_info = GlobFuncANIInfo.get(self.am, func)
                        sts_args_fix = []
                        for sts_arg, param in zip(
                            sts_args, func_ani_info.sts_params, strict=False
                        ):
                            type_ani_info = TypeANIInfo.get(
                                self.am, param.ty_ref.resolved_ty
                            )
                            sts_args_fix.append(
                                f"{sts_arg} as {type_ani_info.sts_type_in(target)}"
                            )
                        sts_native_call = func_ani_info.call_native_with(sts_args_fix)
                        target.writelns(
                            f'case "{type_name}": return {sts_native_call};',
                        )
                    target.writelns(
                        f"default: throw new Error(`Unknown type: ${{type}}`);",
                    )

    def gen_global_regular_funcs(
        self,
        funcs: list[GlobFuncDecl],
        target: StsWriter,
    ):
        for func in funcs:
            func_ani_info = GlobFuncANIInfo.get(self.am, func)
            sts_params = []
            sts_args = []
            for sts_param in func_ani_info.sts_params:
                type_ani_info = TypeANIInfo.get(self.am, sts_param.ty_ref.resolved_ty)
                sts_params.append(
                    f"{sts_param.name}: {type_ani_info.sts_type_in(target)}"
                )
                sts_args.append(sts_param.name)
            sts_native_call = func_ani_info.call_native_with(sts_args)
            if return_ty_ref := func.return_ty_ref:
                type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
                sts_return_ty_name = type_ani_info.sts_type_in(target)
                sts_resolved_ty_name = type_ani_info.sts_type_in(target)
            else:
                sts_return_ty_name = "void"
                sts_resolved_ty_name = "undefined"
            if (
                sts_func_name := func_ani_info.sts_func_name
            ) is not None and func_ani_info.on_off_type is None:
                self.gen_normal_func(
                    sts_func_name,
                    sts_params,
                    sts_return_ty_name,
                    sts_native_call,
                    target,
                    FuncKind.GLOBAL,
                )
                if (sts_promise_name := func_ani_info.sts_promise_name) is not None:
                    self.gen_promise_function(
                        sts_promise_name,
                        sts_params,
                        sts_return_ty_name,
                        sts_native_call,
                        sts_resolved_ty_name,
                        target,
                        FuncKind.GLOBAL,
                    )
                if (sts_async_name := func_ani_info.sts_async_name) is not None:
                    self.gen_async_function(
                        sts_async_name,
                        sts_params,
                        sts_return_ty_name,
                        sts_native_call,
                        sts_resolved_ty_name,
                        target,
                        FuncKind.GLOBAL,
                    )

    def gen_enum(
        self,
        enum: EnumDecl,
        target: StsWriter,
    ):
        enum_ani_info = EnumANIInfo.get(self.am, enum)
        if enum_ani_info.const:
            type_ani_info = TypeANIInfo.get(self.am, enum.ty_ref.resolved_ty)
            for item in enum.items:
                if isinstance(item.value, float):
                    target.writelns(
                        f"export const {item.name}: {type_ani_info.sts_type_in(target)} = {dumps(item.value)}f;",
                    )
                else:
                    target.writelns(
                        f"export const {item.name}: {type_ani_info.sts_type_in(target)} = {dumps(item.value)};",
                    )
            return
        sts_decl = f"enum {enum_ani_info.sts_type_name}"
        if enum_ani_info.is_default:
            sts_decl = f"export default {sts_decl}"
        else:
            sts_decl = f"export {sts_decl}"
        with target.indented(
            f"{sts_decl} {{",
            f"}}",
        ):
            for item in enum.items:
                if item.value is None:
                    target.writelns(
                        f"{item.name},",
                    )
                else:
                    target.writelns(
                        f"{item.name} = {dumps(item.value)},",
                    )

    def gen_union(
        self,
        union: UnionDecl,
        target: StsWriter,
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
                    sts_types.append(ty_ani_info.sts_type_in(target))
        sts_types_str = " | ".join(sts_types)
        sts_decl = f"type {union_ani_info.sts_type_name}"
        if union_ani_info.is_default:
            sts_decl = f"export default {sts_decl}"
        else:
            sts_decl = f"export {sts_decl}"
        target.writelns(
            f"{sts_decl} = {sts_types_str};",
        )

    def gen_struct_interface(
        self,
        struct: StructDecl,
        target: StsWriter,
    ):
        struct_ani_info = StructANIInfo.get(self.am, struct)
        if struct_ani_info.is_class():
            # no interface
            return
        sts_decl = f"interface {struct_ani_info.sts_type_name}"
        if struct_ani_info.sts_iface_parents:
            parents = []
            for parent in struct_ani_info.sts_iface_parents:
                parent_ty = parent.ty_ref.resolved_ty
                parent_ani_info = TypeANIInfo.get(self.am, parent_ty)
                parents.append(parent_ani_info.sts_type_in(target))
            extends_str = ", ".join(parents) if parents else ""
            sts_decl = f"{sts_decl} extends {extends_str}"
        if struct_ani_info.is_default:
            sts_decl = f"export default {sts_decl}"
        else:
            sts_decl = f"export {sts_decl}"
        with target.indented(
            f"{sts_decl} {{",
            f"}}",
        ):
            # TODO: hack inject
            for injected in struct_ani_info.interface_injected_codes:
                target.write_block(injected)
            for field in struct_ani_info.sts_fields:
                field_ani_info = StructFieldANIInfo.get(self.am, field)
                readonly_str = "readonly " if field_ani_info.readonly else ""
                ty_ani_info = TypeANIInfo.get(self.am, field.ty_ref.resolved_ty)
                target.writelns(
                    f"{readonly_str}{field.name}: {ty_ani_info.sts_type_in(target)};",
                )

    def gen_struct_class(
        self,
        struct: StructDecl,
        target: StsWriter,
    ):
        struct_ani_info = StructANIInfo.get(self.am, struct)
        sts_decl = f"class {struct_ani_info.sts_impl_name}"
        if struct_ani_info.is_class():
            if struct_ani_info.sts_iface_parents:
                parents = []
                for parent in struct_ani_info.sts_iface_parents:
                    parent_ty = parent.ty_ref.resolved_ty
                    parent_ani_info = TypeANIInfo.get(self.am, parent_ty)
                    parents.append(parent_ani_info.sts_type_in(target))
                implements_str = ", ".join(parents) if parents else ""
                sts_decl = f"{sts_decl} implements {implements_str}"
            if struct_ani_info.is_default:
                sts_decl = f"export default {sts_decl}"
            else:
                sts_decl = f"export {sts_decl}"
        else:
            sts_decl = f"{sts_decl} implements {struct_ani_info.sts_type_name}"

        with target.indented(
            f"{sts_decl} {{",
            f"}}",
        ):
            # TODO: hack inject
            for injected in struct_ani_info.class_injected_codes:
                target.write_block(injected)
            for parts in struct_ani_info.sts_final_fields:
                final = parts[-1]
                final_ani_info = StructFieldANIInfo.get(self.am, final)
                readonly_str = "readonly " if final_ani_info.readonly else ""
                ty_ani_info = TypeANIInfo.get(self.am, final.ty_ref.resolved_ty)
                target.writelns(
                    f"{readonly_str}{final.name}: {ty_ani_info.sts_type_in(target)};"
                )

            params = []
            for parts in struct_ani_info.sts_final_fields:
                final = parts[-1]
                ty_ani_info = TypeANIInfo.get(self.am, final.ty_ref.resolved_ty)
                params.append(f"{final.name}: {ty_ani_info.sts_type_in(target)}")
            params_str = ", ".join(params)
            with target.indented(
                f"constructor({params_str}) {{",
                f"}}",
            ):
                for parts in struct_ani_info.sts_final_fields:
                    final = parts[-1]
                    target.writelns(
                        f"this.{final.name} = {final.name};",
                    )

    def gen_iface_interface(
        self,
        iface: IfaceDecl,
        target: StsWriter,
    ):
        iface_ani_info = IfaceANIInfo.get(self.am, iface)
        if iface_ani_info.is_class():
            # no interface
            return
        sts_decl = f"interface {iface_ani_info.sts_type_name}"
        if iface_ani_info.sts_iface_parents:
            parents = []
            for parent in iface_ani_info.sts_iface_parents:
                parent_ty = parent.ty_ref.resolved_ty
                parent_ani_info = TypeANIInfo.get(self.am, parent_ty)
                parents.append(parent_ani_info.sts_type_in(target))
            extends_str = ", ".join(parents) if parents else ""
            sts_decl = f"{sts_decl} extends {extends_str}"
        if iface_ani_info.is_default:
            sts_decl = f"export default {sts_decl}"
        else:
            sts_decl = f"export {sts_decl}"
        with target.indented(
            f"{sts_decl} {{",
            f"}}",
        ):
            # TODO: hack inject
            for injected in iface_ani_info.interface_injected_codes:
                target.write_block(injected)
            self.gen_iface_methods(iface.methods, target)

    def gen_iface_methods(
        self,
        methods: list[IfaceMethodDecl],
        target: StsWriter,
    ):
        self.gen_iface_on_off_methods(methods, target)
        self.gen_iface_regular_methods(methods, target)

    def gen_iface_on_off_methods(
        self,
        methods: list[IfaceMethodDecl],
        target: StsWriter,
    ):
        method_on_off_map = self.stat_on_off_methods(methods)
        for (
            sts_method_name,
            sts_params_ani_desc,
        ), method_list in method_on_off_map.items():
            sts_params = []
            sts_params.append("type: string")
            for index in range(len(sts_params_ani_desc)):
                param_name = f"p_{index}"
                sts_param_i_types = []
                for _, method in method_list:
                    method_ani_info = IfaceMethodANIInfo.get(self.am, method)
                    param_ty = method_ani_info.sts_params[index].ty_ref.resolved_ty
                    type_ani_info = TypeANIInfo.get(self.am, param_ty)
                    sts_param_i_types.append(type_ani_info.sts_type_in(target))
                sts_param_ty_name = " | ".join(sts_param_i_types)
                sts_params.append(f"{param_name}: {sts_param_ty_name}")
            sts_params_str = ", ".join(sts_params)
            sts_return_ty_names = set()
            for _, method in method_list:
                if return_ty_ref := method.return_ty_ref:
                    type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
                    sts_return_ty_names.add(type_ani_info.sts_type_in(target))
                else:
                    sts_return_ty_names.add("void")
            sts_return_ty_name = sts_return_ty_names.pop()
            target.writelns(
                f"{sts_method_name}({sts_params_str}): {sts_return_ty_name};",
            )

    def gen_iface_regular_methods(
        self,
        methods: list[IfaceMethodDecl],
        target: StsWriter,
    ):
        for method in methods:
            method_ani_info = IfaceMethodANIInfo.get(self.am, method)
            sts_params = []
            for sts_param in method_ani_info.sts_params:
                type_ani_info = TypeANIInfo.get(self.am, sts_param.ty_ref.resolved_ty)
                sts_params.append(
                    f"{sts_param.name}: {type_ani_info.sts_type_in(target)}"
                )
            if return_ty_ref := method.return_ty_ref:
                type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
                sts_return_ty_name = type_ani_info.sts_type_in(target)
            else:
                sts_return_ty_name = "void"
            if (
                sts_method_name := method_ani_info.sts_method_name
            ) is not None and method_ani_info.on_off_type is None:
                self.gen_iface_normal_method(
                    sts_method_name,
                    sts_params,
                    sts_return_ty_name,
                    target,
                )
                if (sts_promise_name := method_ani_info.sts_promise_name) is not None:
                    self.gen_iface_promise_method(
                        sts_promise_name,
                        sts_params,
                        sts_return_ty_name,
                        target,
                    )
                if (sts_async_name := method_ani_info.sts_async_name) is not None:
                    self.gen_iface_async_method(
                        sts_async_name,
                        sts_params,
                        sts_return_ty_name,
                        target,
                    )
            if (get_name := method_ani_info.get_name) is not None:
                self.gen_iface_get_method(
                    get_name,
                    sts_params,
                    sts_return_ty_name,
                    target,
                )
            if (set_name := method_ani_info.set_name) is not None:
                self.gen_iface_set_method(
                    set_name,
                    sts_params,
                    sts_return_ty_name,
                    target,
                )

    def gen_iface_normal_method(
        self,
        sts_method_name: str,
        sts_params: list[str],
        sts_return_ty_name: str,
        target: StsWriter,
    ):
        sts_params_str = ", ".join(sts_params)
        target.writelns(
            f"{sts_method_name}({sts_params_str}): {sts_return_ty_name};",
        )

    def gen_iface_promise_method(
        self,
        sts_promise_name: str,
        sts_params: list[str],
        sts_return_ty_name: str,
        target: StsWriter,
    ):
        sts_params_str = ", ".join(sts_params)
        target.writelns(
            f"{sts_promise_name}({sts_params_str}): Promise<{sts_return_ty_name}>;",
        )

    def gen_iface_async_method(
        self,
        sts_async_name: str,
        sts_params: list[str],
        sts_return_ty_name: str,
        target: StsWriter,
    ):
        callback_param = f"callback: AsyncCallback<{sts_return_ty_name}>"
        sts_params_with_cb_str = ", ".join([*sts_params, callback_param])
        target.writelns(
            f"{sts_async_name}({sts_params_with_cb_str}): void;",
        )

    def gen_iface_get_method(
        self,
        get_name: str,
        sts_params: list[str],
        sts_return_ty_name: str,
        target: StsWriter,
    ):
        sts_params_str = ", ".join(sts_params)
        target.writelns(
            f"get {get_name}({sts_params_str}): {sts_return_ty_name};",
        )

    def gen_iface_set_method(
        self,
        set_name: str,
        sts_params: list[str],
        sts_return_ty_name: str,
        target: StsWriter,
    ):
        sts_params_str = ", ".join(sts_params)
        target.writelns(
            f"set {set_name}({sts_params_str});",
        )

    def gen_iface_class(
        self,
        iface: IfaceDecl,
        target: StsWriter,
        statics_map: dict[str, list[GlobFuncDecl]],
        ctors_map: dict[str, list[GlobFuncDecl]],
    ):
        iface_ani_info = IfaceANIInfo.get(self.am, iface)
        sts_decl = f"class {iface_ani_info.sts_impl_name}"
        if iface_ani_info.is_class():
            if iface_ani_info.sts_iface_parents:
                parents = []
                for parent in iface_ani_info.sts_iface_parents:
                    parent_ty = parent.ty_ref.resolved_ty
                    parent_ani_info = TypeANIInfo.get(self.am, parent_ty)
                    parents.append(parent_ani_info.sts_type_in(target))
                implements_str = ", ".join(parents) if parents else ""
                sts_decl = f"{sts_decl} implements {implements_str}"
            if iface_ani_info.is_default:
                sts_decl = f"export default {sts_decl}"
            else:
                sts_decl = f"export {sts_decl}"
        else:
            sts_decl = f"{sts_decl} implements {iface_ani_info.sts_type_name}"

        with target.indented(
            f"{sts_decl} {{",
            f"}}",
        ):
            # TODO: hack inject
            for injected in iface_ani_info.class_injected_codes:
                target.write_block(injected)
            target.writelns(
                f"private _vtbl_ptr: long;",
                f"private _data_ptr: long;",
                f"private static native _obj_drop(data_ptr: long): void;",
                f"private static native _obj_dup(data_ptr: long): long;",
                f"private static _registry = new FinalizationRegistry<long>((data_ptr: long) => {{ {iface_ani_info.sts_impl_name}._obj_drop(data_ptr); }});",
            )
            with target.indented(
                f"private _register(): void {{",
                f"}}",
            ):
                target.writelns(
                    f"{iface_ani_info.sts_impl_name}._registry.register(this, this._data_ptr, this);",
                )
            with target.indented(
                f"private _unregister(): void {{",
                f"}}",
            ):
                target.writelns(
                    f"{iface_ani_info.sts_impl_name}._registry.unregister(this);",
                )
            with target.indented(
                f"private _initialize(_vtbl_ptr: long, _data_ptr: long): void {{",
                f"}}",
            ):
                target.writelns(
                    f"this._vtbl_ptr = _vtbl_ptr;",
                    f"this._data_ptr = _data_ptr;",
                    f"this._register();",
                )
            with target.indented(
                f"public _copy_from(other: {iface_ani_info.sts_impl_name}): void {{",
                f"}}",
            ):
                target.writelns(
                    f"this._initialize(other._vtbl_ptr, {iface_ani_info.sts_impl_name}._obj_dup(other._data_ptr));",
                )
            with target.indented(
                f"public _move_from(other: {iface_ani_info.sts_impl_name}): void {{",
                f"}}",
            ):
                target.writelns(
                    f"this._initialize(other._vtbl_ptr, other._data_ptr);",
                    f"other._unregister();",
                )
            ctors = ctors_map.get(iface.name, [])
            for ctor in ctors:
                self.gen_class_ctor(ctor, iface_ani_info, target)
            self.gen_static_funcs(statics_map.get(iface.name, []), target)
            iface_abi_info = IfaceABIInfo.get(self.am, iface)
            for ancestor in iface_abi_info.ancestor_dict:
                self.gen_native_methods(ancestor.methods, target)
                self.gen_class_methods(ancestor.methods, target)

    def gen_class_ctor(
        self,
        ctor: GlobFuncDecl,
        iface_ani_info: IfaceANIInfo,
        target: StsWriter,
    ):
        ctor_ani_info = GlobFuncANIInfo.get(self.am, ctor)
        sts_params = []
        sts_args = []
        for sts_param in ctor_ani_info.sts_params:
            type_ani_info = TypeANIInfo.get(self.am, sts_param.ty_ref.resolved_ty)
            sts_params.append(f"{sts_param.name}: {type_ani_info.sts_type_in(target)}")
            sts_args.append(sts_param.name)
        sts_params_str = ", ".join(sts_params)
        sts_native_call = ctor_ani_info.call_native_with(sts_args)
        with target.indented(
            f"constructor({sts_params_str}) {{",
            f"}}",
        ):
            target.writelns(
                f"this._move_from({sts_native_call} as {iface_ani_info.sts_impl_name});",
            )

    def gen_static_funcs(
        self,
        funcs: list[GlobFuncDecl],
        target: StsWriter,
    ):
        self.gen_static_on_off_funcs(funcs, target)
        self.gen_static_regular_funcs(funcs, target)

    def gen_static_on_off_funcs(
        self,
        funcs: list[GlobFuncDecl],
        target: StsWriter,
    ):
        func_on_off_map = self.stat_on_off_funcs(funcs)
        for (
            sts_func_name,
            sts_params_ani_desc,
        ), func_list in func_on_off_map.items():
            sts_params = []
            sts_args_any = []
            sts_params.append("type: string")
            for index in range(len(sts_params_ani_desc)):
                param_name = f"p_{index}"
                sts_param_i_types = []
                for _, func in func_list:
                    func_ani_info = GlobFuncANIInfo.get(self.am, func)
                    param_ty = func_ani_info.sts_params[index].ty_ref.resolved_ty
                    type_ani_info = TypeANIInfo.get(self.am, param_ty)
                    sts_param_i_types.append(type_ani_info.sts_type_in(target))
                sts_param_ty_name = " | ".join(sts_param_i_types)
                sts_params.append(f"{param_name}: {sts_param_ty_name}")
                sts_args_any.append(param_name)
            sts_params_str = ", ".join(sts_params)
            sts_return_ty_names = set()
            for _, func in func_list:
                if return_ty_ref := func.return_ty_ref:
                    type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
                    sts_return_ty_names.add(type_ani_info.sts_type_in(target))
                else:
                    sts_return_ty_names.add("void")
            sts_return_ty_name = sts_return_ty_names.pop()
            with target.indented(
                f"static {sts_func_name}({sts_params_str}): {sts_return_ty_name} {{",
                f"}}",
            ):
                with target.indented(
                    f"switch(type) {{",
                    f"}}",
                    indent="",
                ):
                    for type_name, func in func_list:
                        func_ani_info = GlobFuncANIInfo.get(self.am, func)
                        sts_args_fix = []
                        for sts_arg_any, param in zip(
                            sts_args_any, func_ani_info.sts_params, strict=True
                        ):
                            type_ani_info = TypeANIInfo.get(
                                self.am, param.ty_ref.resolved_ty
                            )
                            sts_args_fix.append(
                                f"{sts_arg_any} as {type_ani_info.sts_type_in(target)}"
                            )
                        sts_native_call = func_ani_info.call_native_with(sts_args_fix)
                        target.writelns(
                            f'case "{type_name}": return {sts_native_call};',
                        )
                    target.writelns(
                        f"default: throw new Error(`Unknown type: ${{type}}`);",
                    )

    def gen_static_regular_funcs(
        self,
        funcs: list[GlobFuncDecl],
        target: StsWriter,
    ):
        for func in funcs:
            func_ani_info = GlobFuncANIInfo.get(self.am, func)
            sts_params = []
            sts_args = []
            for sts_param in func_ani_info.sts_params:
                type_ani_info = TypeANIInfo.get(self.am, sts_param.ty_ref.resolved_ty)
                sts_params.append(
                    f"{sts_param.name}: {type_ani_info.sts_type_in(target)}"
                )
                sts_args.append(sts_param.name)
            sts_native_call = func_ani_info.call_native_with(sts_args)
            if return_ty_ref := func.return_ty_ref:
                type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
                sts_return_ty_name = type_ani_info.sts_type_in(target)
                sts_resolved_ty_name = type_ani_info.sts_type_in(target)
            else:
                sts_return_ty_name = "void"
                sts_resolved_ty_name = "undefined"
            if (
                sts_func_name := func_ani_info.sts_func_name
            ) is not None and func_ani_info.on_off_type is None:
                self.gen_normal_func(
                    sts_func_name,
                    sts_params,
                    sts_return_ty_name,
                    sts_native_call,
                    target,
                    FuncKind.STATIC,
                )
                if (sts_promise_name := func_ani_info.sts_promise_name) is not None:
                    self.gen_promise_function(
                        sts_promise_name,
                        sts_params,
                        sts_return_ty_name,
                        sts_native_call,
                        sts_resolved_ty_name,
                        target,
                        FuncKind.STATIC,
                    )
                if (sts_async_name := func_ani_info.sts_async_name) is not None:
                    self.gen_async_function(
                        sts_async_name,
                        sts_params,
                        sts_return_ty_name,
                        sts_native_call,
                        sts_resolved_ty_name,
                        target,
                        FuncKind.STATIC,
                    )
            if (get_name := func_ani_info.get_name) is not None:
                self.gen_get_func(
                    get_name,
                    sts_params,
                    sts_return_ty_name,
                    sts_native_call,
                    target,
                    FuncKind.STATIC,
                )
            if (set_name := func_ani_info.set_name) is not None:
                self.gen_set_func(
                    set_name,
                    sts_params,
                    sts_return_ty_name,
                    sts_native_call,
                    target,
                    FuncKind.STATIC,
                )

    def gen_native_methods(
        self,
        methods: list[IfaceMethodDecl],
        target: StsWriter,
    ):
        # native
        for method in methods:
            method_ani_info = IfaceMethodANIInfo.get(self.am, method)
            sts_native_params = []
            for param in method.params:
                type_ani_info = TypeANIInfo.get(self.am, param.ty_ref.resolved_ty)
                sts_native_params.append(
                    f"{param.name}: {type_ani_info.sts_type_in(target)}"
                )
            sts_native_params_str = ", ".join(sts_native_params)
            if return_ty_ref := method.return_ty_ref:
                type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
                sts_return_ty_name = type_ani_info.sts_type_in(target)
            else:
                sts_return_ty_name = "void"
            target.writelns(
                f"native {method_ani_info.sts_native_name}({sts_native_params_str}): {sts_return_ty_name};",
            )

    def gen_class_methods(
        self,
        methods: list[IfaceMethodDecl],
        target: StsWriter,
    ):
        self.gen_iface_on_off_meths(methods, target)
        self.gen_iface_regular_meths(methods, target)

    def gen_iface_on_off_meths(
        self,
        methods: list[IfaceMethodDecl],
        target: StsWriter,
    ):
        method_on_off_map = self.stat_on_off_methods(methods)
        for (
            sts_method_name,
            sts_params_ani_desc,
        ), method_list in method_on_off_map.items():
            sts_params = []
            sts_args_any = []
            sts_params.append("type: string")
            for index in range(len(sts_params_ani_desc)):
                param_name = f"p_{index}"
                sts_param_i_types = []
                for _, method in method_list:
                    method_ani_info = IfaceMethodANIInfo.get(self.am, method)
                    param_ty = method_ani_info.sts_params[index].ty_ref.resolved_ty
                    type_ani_info = TypeANIInfo.get(self.am, param_ty)
                    sts_param_i_types.append(type_ani_info.sts_type_in(target))
                sts_param_ty_name = " | ".join(sts_param_i_types)
                sts_params.append(f"{param_name}: {sts_param_ty_name}")
                sts_args_any.append(param_name)
            sts_params_str = ", ".join(sts_params)
            sts_return_ty_names = set()
            for _, method in method_list:
                if return_ty_ref := method.return_ty_ref:
                    type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
                    sts_return_ty_names.add(type_ani_info.sts_type_in(target))
                else:
                    sts_return_ty_names.add("void")
            sts_return_ty_name = sts_return_ty_names.pop()
            with target.indented(
                f"{sts_method_name}({sts_params_str}): {sts_return_ty_name} {{",
                f"}}",
            ):
                with target.indented(
                    f"switch(type) {{",
                    f"}}",
                    indent="",
                ):
                    for type_name, method in method_list:
                        method_ani_info = IfaceMethodANIInfo.get(self.am, method)
                        sts_args_fix = []
                        for sts_arg_any, param in zip(
                            sts_args_any, method_ani_info.sts_params, strict=True
                        ):
                            type_ani_info = TypeANIInfo.get(
                                self.am, param.ty_ref.resolved_ty
                            )
                            sts_args_fix.append(
                                f"{sts_arg_any} as {type_ani_info.sts_type_in(target)}"
                            )
                        sts_native_call = method_ani_info.call_native_with(sts_args_fix)
                        target.writelns(
                            f'case "{type_name}": return {sts_native_call};',
                        )
                    target.writelns(
                        f"default: throw new Error(`Unknown type: ${{type}}`);",
                    )

    def gen_iface_regular_meths(
        self,
        methods: list[IfaceMethodDecl],
        target: StsWriter,
    ):
        for method in methods:
            method_ani_info = IfaceMethodANIInfo.get(self.am, method)
            sts_params = []
            sts_args = []
            for sts_param in method_ani_info.sts_params:
                type_ani_info = TypeANIInfo.get(self.am, sts_param.ty_ref.resolved_ty)
                sts_params.append(
                    f"{sts_param.name}: {type_ani_info.sts_type_in(target)}"
                )
                sts_args.append(sts_param.name)
            sts_native_call = method_ani_info.call_native_with(sts_args)
            if return_ty_ref := method.return_ty_ref:
                type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
                sts_return_ty_name = type_ani_info.sts_type_in(target)
                sts_resolved_ty_name = type_ani_info.sts_type_in(target)
            else:
                sts_return_ty_name = "void"
                sts_resolved_ty_name = "undefined"
            if (
                sts_method_name := method_ani_info.sts_method_name
            ) is not None and method_ani_info.on_off_type is None:
                self.gen_normal_func(
                    sts_method_name,
                    sts_params,
                    sts_return_ty_name,
                    sts_native_call,
                    target,
                    FuncKind.INTERFACE,
                )
                if (sts_promise_name := method_ani_info.sts_promise_name) is not None:
                    self.gen_promise_function(
                        sts_promise_name,
                        sts_params,
                        sts_return_ty_name,
                        sts_native_call,
                        sts_resolved_ty_name,
                        target,
                        FuncKind.INTERFACE,
                    )
                if (sts_async_name := method_ani_info.sts_async_name) is not None:
                    self.gen_async_function(
                        sts_async_name,
                        sts_params,
                        sts_return_ty_name,
                        sts_native_call,
                        sts_resolved_ty_name,
                        target,
                        FuncKind.INTERFACE,
                    )
            if (get_name := method_ani_info.get_name) is not None:
                self.gen_get_func(
                    get_name,
                    sts_params,
                    sts_return_ty_name,
                    sts_native_call,
                    target,
                    FuncKind.INTERFACE,
                )
            if (set_name := method_ani_info.set_name) is not None:
                self.gen_set_func(
                    set_name,
                    sts_params,
                    sts_return_ty_name,
                    sts_native_call,
                    target,
                    FuncKind.INTERFACE,
                )

    def gen_iface_normal_meth(
        self,
        sts_method_name: str,
        sts_params: list[str],
        sts_return_ty_name: str,
        sts_native_call: str,
        target: StsWriter,
    ):
        sts_params_str = ", ".join(sts_params)
        with target.indented(
            f"{sts_method_name}({sts_params_str}): {sts_return_ty_name} {{",
            f"}}",
        ):
            target.writelns(
                f"return {sts_native_call};",
            )

    def gen_get_func(
        self,
        get_name: str,
        sts_params: list[str],
        sts_return_ty_name: str,
        sts_native_call: str,
        target: StsWriter,
        func_kind: FuncKind,
    ):
        sts_params_str = ", ".join(sts_params)
        with target.indented(
            f"{func_kind.value}get {get_name}({sts_params_str}): {sts_return_ty_name} {{",
            f"}}",
        ):
            target.writelns(
                f"return {sts_native_call};",
            )

    def gen_set_func(
        self,
        set_name: str,
        sts_params: list[str],
        sts_return_ty_name: str,
        sts_native_call: str,
        target: StsWriter,
        func_kind: FuncKind,
    ):
        sts_params_str = ", ".join(sts_params)
        with target.indented(
            f"{func_kind.value}set {set_name}({sts_params_str}) {{",
            f"}}",
        ):
            target.writelns(
                f"return {sts_native_call};",
            )

    def gen_normal_func(
        self,
        sts_func_name: str,
        sts_params: list[str],
        sts_return_ty_name: str,
        sts_native_call: str,
        target: StsWriter,
        func_kind: FuncKind,
    ):
        sts_params_str = ", ".join(sts_params)
        with target.indented(
            f"{func_kind.value}{sts_func_name}({sts_params_str}): {sts_return_ty_name} {{",
            f"}}",
        ):
            target.writelns(
                f"return {sts_native_call};",
            )

    def gen_promise_function(
        self,
        sts_promise_name: str,
        sts_params: list[str],
        sts_return_ty_name: str,
        sts_native_call: str,
        sts_resolved_ty_name: str,
        target: StsWriter,
        func_kind: FuncKind,
    ):
        sts_params_str = ", ".join(sts_params)
        with target.indented(
            f"{func_kind.value}{sts_promise_name}({sts_params_str}): Promise<{sts_return_ty_name}> {{",
            f"}}",
        ):
            with target.indented(
                f"return new Promise<{sts_return_ty_name}>((resolve, reject): void => {{",
                f"}});",
            ):
                with target.indented(
                    f"taskpool.execute((): {sts_return_ty_name} => {{",
                    f"}})",
                ):
                    target.writelns(
                        f"return {sts_native_call};",
                    )
                with target.indented(
                    f".then((ret: NullishType): void => {{",
                    f"}})",
                ):
                    target.writelns(
                        f"resolve(ret as {sts_resolved_ty_name});",
                    )
                with target.indented(
                    f".catch((ret: NullishType): void => {{",
                    f"}});",
                ):
                    target.writelns(
                        f"reject(ret as Error);",
                    )

    def gen_async_function(
        self,
        sts_async_name: str,
        sts_params: list[str],
        sts_return_ty_name: str,
        sts_native_call: str,
        sts_resolved_ty_name: str,
        target: StsWriter,
        func_kind: FuncKind,
    ):
        callback_param = f"callback: AsyncCallback<{sts_return_ty_name}>"
        sts_params_with_cb_str = ", ".join([*sts_params, callback_param])
        with target.indented(
            f"{func_kind.value}{sts_async_name}({sts_params_with_cb_str}): void {{",
            f"}}",
        ):
            with target.indented(
                f"taskpool.execute((): {sts_return_ty_name} => {{",
                f"}})",
            ):
                target.writelns(
                    f"return {sts_native_call};",
                )
            with target.indented(
                f".then((ret: NullishType): void => {{",
                f"}})",
            ):
                target.writelns(
                    f"let err: BusinessError = new BusinessError();",
                    f"callback(err, ret as {sts_resolved_ty_name});",
                )
            with target.indented(
                f".catch((ret: NullishType): void => {{",
                f"}});",
            ):
                target.writelns(
                    f"let res: {sts_resolved_ty_name};",
                    f"callback(ret as BusinessError, res);",
                )

    def gen_ohos_base(self):
        with StsWriter(
            self.om,
            "@ohos.base.ets",
            FileKind.ETS,
        ) as target:
            target.writelns(
                "export class BusinessError<T = void> extends Error {",
                "    code: number;",
                "    data?: T;",
                "    constructor() {",
                "        super();",
                "        this.code = 0;",
                "    }",
                "    constructor(code: number, error: Error) {",
                "        super(error.name, error.message, new ErrorOptions(error.cause));",
                "        this.code = code;",
                "    }",
                "    constructor(code: number, data: T, error: Error) {",
                "        super(error.name, error.message, new ErrorOptions(error.cause));",
                "        this.code = code;",
                "        this.data = data;",
                "    }",
                "}",
                "export type AsyncCallback<T, E = void> = (error: BusinessError<E>, data: T) => void;",
            )

    def gen_utils(
        self,
        target: StsWriter,
    ):
        target.writelns(
            "function __fromArrayBufferToBigInt(arr: ArrayBuffer): BigInt {",
            "    let res: BigInt = 0n;",
            "    for (let i: int = 0; i < arr.getByteLength(); i++) {",
            "        res |= BigInt(arr.at(i).toLong() & 0xff) << BigInt(i * 8);",
            "    }",
            "    let m: int = arr.getByteLength();",
            "    if (arr.at(m - 1) < 0) {",
            "        res |= -1n << BigInt(m * 8 - 1);",
            "    }",
            "    return res;",
            "}",
        )
        target.writelns(
            "function __fromBigIntToArrayBuffer(val: BigInt, blk: int): ArrayBuffer {",
            "    let n_7 = BigInt(blk * 8 - 1);",
            "    let n_8 = BigInt(blk * 8);",
            "    let ocp: BigInt = val;",
            "    let n: int = 0;",
            "    while (true) {",
            "        n += blk;",
            "        let t_7 = ocp >> n_7;",
            "        let t_8 = ocp >> n_8;",
            "        if (t_7 == t_8) {",
            "            break;",
            "        }",
            "        ocp = t_8;",
            "    }",
            "    let buf = new ArrayBuffer(n);",
            "    for (let i: int = 0; i < n; i++) {",
            "        buf.set(i, (val & 255n).getLong().toByte())",
            "        val >>= 8n;",
            "    }",
            "    return buf;",
            "}",
        )