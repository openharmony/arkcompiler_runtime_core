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

from abc import ABC, abstractmethod
from dataclasses import dataclass
from json import dumps
from typing import ClassVar

from taihe.codegen.abi.analyses import (
    IfaceAbiInfo,
)
from taihe.codegen.ani.analyses import (
    ArkTsModule,
    ArkTsModuleOrNamespace,
    EnumAniInfo,
    GlobFuncAniInfo,
    IfaceAniInfo,
    IfaceMethodAniInfo,
    PackageGroupAniInfo,
    StructAniInfo,
    TypeAniInfo,
    UnionAniInfo,
)
from taihe.codegen.ani.attributes import (
    ConstAttr,
    OptionalAttr,
    ReadOnlyAttr,
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
from taihe.semantics.types import NonVoidType
from taihe.utils.analyses import AnalysisManager
from taihe.utils.outputs import FileKind, OutputManager


@dataclass(frozen=True)
class FuncKind(ABC):
    func_prefix: ClassVar[str]
    get_prefix: ClassVar[str]
    set_prefix: ClassVar[str]
    overload_prefix: ClassVar[str]

    @abstractmethod
    def call_from_local(self, name: str) -> str:
        ...

    @abstractmethod
    def reverse_base_params(self) -> list[str]:
        ...

    @abstractmethod
    def call_from_reverse(self, name: str) -> str:
        ...


@dataclass(frozen=True)
class CtorKind:
    func_prefix = "constructor "
    overload_prefix = "overload constructor "

    class_name: str

    def call_from_local(self, name: str) -> str:
        return f"this.{name}" if name else "this"

    def reverse_base_params(self) -> list[str]:
        return []

    def call_from_reverse(self, name: str) -> str:
        return f"new {self.class_name}.{name}" if name else f"new {self.class_name}"


@dataclass(frozen=True)
class InterfaceKind(FuncKind):
    func_prefix = ""
    get_prefix = "get "
    set_prefix = "set "
    overload_prefix = "overload "

    type_name: str

    def call_from_local(self, name: str) -> str:
        return f"this.{name}"

    def reverse_base_params(self) -> list[str]:
        return [f"self: {self.type_name}"]

    def call_from_reverse(self, name: str) -> str:
        return f"self.{name}"


@dataclass(frozen=True)
class StaticKind(FuncKind):
    func_prefix = "static "
    get_prefix = "static get "
    set_prefix = "static set "
    overload_prefix = "static overload "

    class_name: str

    def call_from_local(self, name) -> str:
        return f"{self.class_name}.{name}"

    def reverse_base_params(self) -> list[str]:
        return []

    def call_from_reverse(self, name: str) -> str:
        return f"{self.class_name}.{name}"


@dataclass(frozen=True)
class GlobalKind(FuncKind):
    func_prefix = "function "
    get_prefix = "get "
    set_prefix = "set "
    overload_prefix = "overload "

    def call_from_local(self, name: str) -> str:
        return name

    def reverse_base_params(self) -> list[str]:
        return []

    def call_from_reverse(self, name: str) -> str:
        return name


OverloadInfo = list[str]


class OverloadRegister:
    def __init__(self):
        self.infos: dict[str, OverloadInfo] = {}

    def register(self, orig_name: str, name: str):
        info = self.infos.setdefault(name, [])
        info.append(orig_name)


OnOffInfo = dict[tuple[str, ...], dict[str, tuple[str, list[str], str]]]


class OnOffRegister:
    def __init__(self):
        self.infos: dict[str, OnOffInfo] = {}

    def register(
        self,
        pair: tuple[str, str],
        params_ty_sts_sig: list[str],
        orig_name: str,
        params_ty_sts_name: list[str],
        return_ty_sts_name: str,
    ):
        name, type = pair
        info = self.infos.setdefault(name, {})
        cases = info.setdefault(tuple(params_ty_sts_sig), {})
        cases.setdefault(type, (orig_name, params_ty_sts_name, return_ty_sts_name))


class StsCodeGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager):
        self.om = om
        self.am = am

    def generate(self, pg: PackageGroup):
        pg_ani_info = PackageGroupAniInfo.get(self.am, pg)
        for mod_name, mod in pg_ani_info.mods.items():
            self.gen_module_file(mod_name, mod)

    def gen_module_file(self, mod_name: str, mod: ArkTsModule):
        with StsWriter(
            self.om,
            f"{mod_name}.ets",
            FileKind.ETS,
        ) as target:
            self.gen_namespace(mod, target)
            self.gen_utils(target)

    def gen_namespace(self, ns: ArkTsModuleOrNamespace, target: StsWriter):
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

    def gen_package(self, pkg: PackageDecl, target: StsWriter):
        for func in pkg.functions:
            func_ani_info = GlobFuncAniInfo.get(self.am, func)
            self.gen_native_func(func, func_ani_info, target)

        ctors_map: dict[str, list[GlobFuncDecl]] = {}
        statics_map: dict[str, list[GlobFuncDecl]] = {}
        glob_funcs: list[GlobFuncDecl] = []
        for func in pkg.functions:
            func_ani_info = GlobFuncAniInfo.get(self.am, func)
            if class_name := func_ani_info.static_scope:
                statics_map.setdefault(class_name, []).append(func)
            elif class_name := func_ani_info.ctor_scope:
                ctors_map.setdefault(class_name, []).append(func)
            else:
                glob_funcs.append(func)

        func_overload_register = OverloadRegister()
        func_on_off_register = OnOffRegister()
        func_kind = GlobalKind()
        for func in glob_funcs:
            func_ani_info = GlobFuncAniInfo.get(self.am, func)
            self.gen_any_func(
                func,
                func_ani_info,
                target,
                func_kind,
                "export default " if func_ani_info.is_default else "export ",
                func_overload_register,
                func_on_off_register,
            )
        for name, info in func_overload_register.infos.items():
            self.gen_overload_func(name, info, target, func_kind, "export ")
        for name, info in func_on_off_register.infos.items():
            self.gen_full_on_off_func(name, info, target, func_kind, "export ")

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

        for func in glob_funcs:
            func_ani_info = GlobFuncAniInfo.get(self.am, func)
            self.gen_reverse_func(func, func_ani_info, target, func_kind)
        for iface in pkg.interfaces:
            iface_ani_info = IfaceAniInfo.get(self.am, iface)
            ctor_kind = CtorKind(iface_ani_info.sts_impl_name)
            for ctor in ctors_map.get(iface.name, []):
                ctor_ani_info = GlobFuncAniInfo.get(self.am, ctor)
                self.gen_reverse_func(ctor, ctor_ani_info, target, ctor_kind)
            func_kind = StaticKind(iface_ani_info.sts_impl_name)
            for func in statics_map.get(iface.name, []):
                func_ani_info = GlobFuncAniInfo.get(self.am, func)
                self.gen_reverse_func(func, func_ani_info, target, func_kind)
            meth_kind = InterfaceKind(iface_ani_info.sts_type_name)
            for method in iface.methods:
                method_ani_info = IfaceMethodAniInfo.get(self.am, method)
                self.gen_reverse_func(method, method_ani_info, target, meth_kind)

    def gen_enum(
        self,
        enum: EnumDecl,
        target: StsWriter,
    ):
        if ConstAttr.get(enum) is not None:
            enum_ty_ani_info = TypeAniInfo.get(self.am, enum.ty)
            for item in enum.items:
                target.writelns(
                    f"export const {item.name}: {enum_ty_ani_info.sts_type_in(target)} = {dumps(item.value)};",
                )
            return

        enum_ani_info = EnumAniInfo.get(self.am, enum)

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
                target.writelns(
                    f"{item.name} = {dumps(item.value)},",
                )

    def gen_union(
        self,
        union: UnionDecl,
        target: StsWriter,
    ):
        union_ani_info = UnionAniInfo.get(self.am, union)

        sts_decl = f"type {union_ani_info.sts_type_name}"
        if union_ani_info.is_default:
            sts_decl = f"export default {sts_decl}"
        else:
            sts_decl = f"export {sts_decl}"

        sts_types = []
        for field in union.fields:
            field_ty_ani_info = TypeAniInfo.get(self.am, field.ty)
            sts_types.append(field_ty_ani_info.sts_type_in(target))
        sts_types_str = " | ".join(sts_types)
        target.writelns(
            f"{sts_decl} = {sts_types_str};",
        )

    def gen_struct_interface(
        self,
        struct: StructDecl,
        target: StsWriter,
    ):
        struct_ani_info = StructAniInfo.get(self.am, struct)
        if struct_ani_info.is_class():
            # no interface
            return

        sts_decl = f"interface {struct_ani_info.sts_type_name}"
        if struct_ani_info.sts_iface_extends:
            extends = []
            for extend in struct_ani_info.sts_iface_extends:
                extend_ty = extend.ty
                extend_ani_info = TypeAniInfo.get(self.am, extend_ty)
                extends.append(extend_ani_info.sts_type_in(target))
            extends_str = ", ".join(extends) if extends else ""
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
                readonly = "readonly " if ReadOnlyAttr.get(field) is not None else ""
                opt = "?" if OptionalAttr.get(field) else ""
                field_ty_ani_info = TypeAniInfo.get(self.am, field.ty)
                target.writelns(
                    f"{readonly}{field.name}{opt}: {field_ty_ani_info.sts_type_in(target)};",
                )

    def gen_struct_class(
        self,
        struct: StructDecl,
        target: StsWriter,
    ):
        struct_ani_info = StructAniInfo.get(self.am, struct)

        sts_decl = f"class {struct_ani_info.sts_impl_name}"
        if struct_ani_info.is_class():
            if struct_ani_info.sts_iface_extends:
                extends = []
                for extend in struct_ani_info.sts_iface_extends:
                    extend_ty = extend.ty
                    extend_ani_info = TypeAniInfo.get(self.am, extend_ty)
                    extends.append(extend_ani_info.sts_type_in(target))
                implements_str = ", ".join(extends) if extends else ""
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

            for parts in struct_ani_info.sts_all_fields:
                final = parts[-1]
                readonly = "readonly " if ReadOnlyAttr.get(final) is not None else ""
                opt = "?" if OptionalAttr.get(final) else ""
                final_ty_ani_info = TypeAniInfo.get(self.am, final.ty)
                target.writelns(
                    f"{readonly}{final.name}{opt}: {final_ty_ani_info.sts_type_in(target)};",
                )

            with target.indented(
                f"constructor(",
                f")",
            ):
                for parts in struct_ani_info.sts_all_fields:
                    final = parts[-1]
                    opt = "?" if OptionalAttr.get(final) else ""
                    final_ty_ani_info = TypeAniInfo.get(self.am, final.ty)
                    target.writelns(
                        f"{final.name}{opt}: {final_ty_ani_info.sts_type_in(target)},",
                    )
            with target.indented(
                f"{{",
                f"}}",
            ):
                for parts in struct_ani_info.sts_all_fields:
                    final = parts[-1]
                    target.writelns(
                        f"this.{final.name} = {final.name};",
                    )

    def gen_iface_interface(
        self,
        iface: IfaceDecl,
        target: StsWriter,
    ):
        iface_ani_info = IfaceAniInfo.get(self.am, iface)
        if iface_ani_info.is_class():
            # no interface
            return

        sts_decl = f"interface {iface_ani_info.sts_type_name}"
        if iface_ani_info.sts_iface_extends:
            extends = []
            for extend in iface_ani_info.sts_iface_extends:
                extend_ty = extend.ty
                extend_ani_info = TypeAniInfo.get(self.am, extend_ty)
                extends.append(extend_ani_info.sts_type_in(target))
            extends_str = ", ".join(extends) if extends else ""
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

            # methods
            meth_overload_register = OverloadRegister()
            meth_on_off_register = OnOffRegister()
            meth_kind = InterfaceKind(iface_ani_info.sts_type_name)
            for method in iface.methods:
                method_ani_info = IfaceMethodAniInfo.get(self.am, method)
                self.gen_any_func_decl(
                    method,
                    method_ani_info,
                    target,
                    meth_kind,
                    "",
                    meth_overload_register,
                    meth_on_off_register,
                )
            for name, info in meth_overload_register.infos.items():
                self.gen_overload_func(name, info, target, meth_kind, "")
            for name, info in meth_on_off_register.infos.items():
                self.gen_half_on_off_func(name, info, target, meth_kind, "")

    def gen_iface_class(
        self,
        iface: IfaceDecl,
        target: StsWriter,
        statics_map: dict[str, list[GlobFuncDecl]],
        ctors_map: dict[str, list[GlobFuncDecl]],
    ):
        iface_ani_info = IfaceAniInfo.get(self.am, iface)

        sts_decl = f"class {iface_ani_info.sts_impl_name}"
        if iface_ani_info.is_class():
            if iface_ani_info.sts_iface_extends:
                extends = []
                for extend in iface_ani_info.sts_iface_extends:
                    extend_ty = extend.ty
                    extend_ani_info = TypeAniInfo.get(self.am, extend_ty)
                    extends.append(extend_ani_info.sts_type_in(target))
                implements_str = ", ".join(extends) if extends else ""
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
                f"private _taihe_vtblPtr: long;",
                f"private _taihe_dataPtr: long;",
            )
            with target.indented(
                f"private _taihe_register(): void {{",
                f"}}",
            ):
                target.writelns(
                    f"_taihe_registry.register(this, this._taihe_dataPtr, this);",
                )
            with target.indented(
                f"private _taihe_unregister(): void {{",
                f"}}",
            ):
                target.writelns(
                    f"_taihe_registry.unregister(this);",
                )
            with target.indented(
                f"private _taihe_initialize(vtblPtr: long, dataPtr: long): void {{",
                f"}}",
            ):
                target.writelns(
                    f"this._taihe_vtblPtr = vtblPtr;",
                    f"this._taihe_dataPtr = dataPtr;",
                    f"this._taihe_register();",
                )
            with target.indented(
                f"public _taihe_copyFrom(other: {iface_ani_info.sts_impl_name}): void {{",
                f"}}",
            ):
                target.writelns(
                    f"this._taihe_initialize(other._taihe_vtblPtr, _taihe_objDup(other._taihe_dataPtr));",
                )
            with target.indented(
                f"public _taihe_moveFrom(other: {iface_ani_info.sts_impl_name}): void {{",
                f"}}",
            ):
                target.writelns(
                    f"this._taihe_initialize(other._taihe_vtblPtr, other._taihe_dataPtr);",
                    f"other._taihe_unregister();",
                )

            iface_abi_info = IfaceAbiInfo.get(self.am, iface)
            for ancestor in iface_abi_info.ancestor_dict:
                for method in ancestor.methods:
                    method_ani_info = IfaceMethodAniInfo.get(self.am, method)
                    self.gen_native_func(method, method_ani_info, target)

            # ctors
            ctor_overload_register = OverloadRegister()
            ctor_on_off_register = OnOffRegister()
            ctor_kind = CtorKind(iface_ani_info.sts_impl_name)
            for ctor in ctors_map.get(iface.name, []):
                ctor_ani_info = GlobFuncAniInfo.get(self.am, ctor)
                self.gen_any_ctor(
                    ctor,
                    ctor_ani_info,
                    target,
                    ctor_kind,
                    "",
                    ctor_overload_register,
                    ctor_on_off_register,
                )
            for name, info in ctor_overload_register.infos.items():
                self.gen_overload_ctor(name, info, target, ctor_kind, "")
            for name, info in ctor_on_off_register.infos.items():
                self.gen_full_on_off_ctor(name, info, target, ctor_kind, "")

            # funcs
            func_overload_register = OverloadRegister()
            func_on_off_register = OnOffRegister()
            func_kind = StaticKind(iface_ani_info.sts_impl_name)
            for func in statics_map.get(iface.name, []):
                func_ani_info = GlobFuncAniInfo.get(self.am, func)
                self.gen_any_func(
                    func,
                    func_ani_info,
                    target,
                    func_kind,
                    "",
                    func_overload_register,
                    func_on_off_register,
                )
            for name, info in func_overload_register.infos.items():
                self.gen_overload_func(name, info, target, func_kind, "")
            for name, info in func_on_off_register.infos.items():
                self.gen_full_on_off_func(name, info, target, func_kind, "")

            # methods
            meth_overload_register = OverloadRegister()
            meth_on_off_register = OnOffRegister()
            meth_kind = InterfaceKind(iface_ani_info.sts_type_name)
            for ancestor in iface_abi_info.ancestor_dict:
                for method in ancestor.methods:
                    method_ani_info = IfaceMethodAniInfo.get(self.am, method)
                    self.gen_any_func(
                        method,
                        method_ani_info,
                        target,
                        meth_kind,
                        "",
                        meth_overload_register,
                        meth_on_off_register,
                    )
            for name, info in meth_overload_register.infos.items():
                self.gen_overload_func(name, info, target, meth_kind, "")
            for name, info in meth_on_off_register.infos.items():
                self.gen_half_on_off_func(name, info, target, meth_kind, "")

    def gen_native_func(
        self,
        func: GlobFuncDecl | IfaceMethodDecl,
        func_ani_info: GlobFuncAniInfo | IfaceMethodAniInfo,
        target: StsWriter,
    ):
        params_sts = []
        for param in func.params:
            param_ty_ani_info = TypeAniInfo.get(self.am, param.ty)
            params_sts.append(f"{param.name}: {param_ty_ani_info.sts_type_in(target)}")
        params_sts_str = ", ".join(params_sts)
        if isinstance(return_ty := func.return_ty, NonVoidType):
            return_ty_ani_info = TypeAniInfo.get(self.am, return_ty)
            return_ty_sts_name = return_ty_ani_info.sts_type_in(target)
        else:
            return_ty_sts_name = "void"

        target.writelns(
            f"{func_ani_info.native_prefix}{func_ani_info.native_name}({params_sts_str}): {return_ty_sts_name};",
        )

    def gen_any_func_decl(
        self,
        func: GlobFuncDecl | IfaceMethodDecl,
        func_ani_info: GlobFuncAniInfo | IfaceMethodAniInfo,
        target: StsWriter,
        func_kind: FuncKind,
        decl_prefix: str,
        overload_register: OverloadRegister,
        on_off_register: OnOffRegister,
    ):
        params_ty_sts_sig = []
        params_ty_sts_name = []
        params_sts = []
        args_sts = []
        for param in func_ani_info.sts_params:
            opt = "?" if OptionalAttr.get(param) else ""
            param_ty_ani_info = TypeAniInfo.get(self.am, param.ty)
            params_ty_sts_sig.append(param_ty_ani_info.type_sig)
            params_ty_sts_name.append(param_ty_ani_info.sts_type_in(target))
            params_sts.append(
                f"{param.name}{opt}: {param_ty_ani_info.sts_type_in(target)}"
            )
            args_sts.append(param.name)
        if isinstance(return_ty := func.return_ty, NonVoidType):
            return_ty_ani_info = TypeAniInfo.get(self.am, return_ty)
            return_ty_sts_name = return_ty_ani_info.sts_type_in(target)
        else:
            return_ty_sts_name = "void"

        if (norm_name := func_ani_info.norm_name) is not None:
            self.gen_normal_func_decl(
                norm_name,
                func_ani_info.gen_async_name,
                func_ani_info.gen_promise_name,
                params_ty_sts_sig,
                params_sts,
                params_ty_sts_name,
                return_ty_sts_name,
                target,
                func_kind,
                decl_prefix,
                func_ani_info.overload_name,
                func_ani_info.on_off_pair,
                overload_register,
                on_off_register,
            )
        if (async_name := func_ani_info.async_name) is not None:
            self.gen_async_func_decl(
                async_name,
                params_ty_sts_sig,
                params_sts,
                params_ty_sts_name,
                return_ty_sts_name,
                target,
                func_kind,
                decl_prefix,
                func_ani_info.overload_name,
                func_ani_info.on_off_pair,
                overload_register,
                on_off_register,
            )
        if (promise_name := func_ani_info.promise_name) is not None:
            self.gen_promise_func_decl(
                promise_name,
                params_ty_sts_sig,
                params_sts,
                params_ty_sts_name,
                return_ty_sts_name,
                target,
                func_kind,
                decl_prefix,
                func_ani_info.overload_name,
                func_ani_info.on_off_pair,
                overload_register,
                on_off_register,
            )
        if (get_name := func_ani_info.get_name) is not None:
            self.gen_getter_decl(
                get_name,
                params_sts,
                return_ty_sts_name,
                target,
                func_kind,
                decl_prefix,
            )
        if (set_name := func_ani_info.set_name) is not None:
            self.gen_setter_decl(
                set_name,
                params_sts,
                return_ty_sts_name,
                target,
                func_kind,
                decl_prefix,
            )

    def gen_any_func(
        self,
        func: GlobFuncDecl | IfaceMethodDecl,
        func_ani_info: GlobFuncAniInfo | IfaceMethodAniInfo,
        target: StsWriter,
        func_kind: FuncKind,
        mode_prefix: str,
        overload_register: OverloadRegister,
        on_off_register: OnOffRegister,
    ):
        params_ty_sts_sig = []
        params_ty_sts_name = []
        params_sts = []
        args_sts = []
        for param in func_ani_info.sts_params:
            opt = "?" if OptionalAttr.get(param) else ""
            param_ty_ani_info = TypeAniInfo.get(self.am, param.ty)
            params_ty_sts_sig.append(param_ty_ani_info.type_sig)
            params_ty_sts_name.append(param_ty_ani_info.sts_type_in(target))
            params_sts.append(
                f"{param.name}{opt}: {param_ty_ani_info.sts_type_in(target)}"
            )
            args_sts.append(param.name)
        args_sts = func_ani_info.as_native_args(args_sts)
        args_sts_str = ", ".join(args_sts)
        result_sts = (
            f"{func_ani_info.call_native(func_ani_info.native_name)}({args_sts_str})"
        )
        if isinstance(return_ty := func.return_ty, NonVoidType):
            return_ty_ani_info = TypeAniInfo.get(self.am, return_ty)
            return_ty_sts_name = return_ty_ani_info.sts_type_in(target)
            return_ty_sts_real = return_ty_ani_info.sts_type_in(target)
        else:
            return_ty_sts_name = "void"
            return_ty_sts_real = "undefined"

        if (norm_name := func_ani_info.norm_name) is not None:
            self.gen_normal_func(
                norm_name,
                func_ani_info.gen_async_name,
                func_ani_info.gen_promise_name,
                params_ty_sts_sig,
                params_sts,
                params_ty_sts_name,
                return_ty_sts_name,
                return_ty_sts_real,
                result_sts,
                target,
                func_kind,
                mode_prefix,
                func_ani_info.overload_name,
                func_ani_info.on_off_pair,
                overload_register,
                on_off_register,
            )
        if (promise_name := func_ani_info.promise_name) is not None:
            self.gen_promise_func(
                promise_name,
                params_ty_sts_sig,
                params_sts,
                params_ty_sts_name,
                return_ty_sts_name,
                return_ty_sts_real,
                result_sts,
                target,
                func_kind,
                mode_prefix,
                func_ani_info.overload_name,
                func_ani_info.on_off_pair,
                overload_register,
                on_off_register,
            )
        if (async_name := func_ani_info.async_name) is not None:
            self.gen_async_func(
                async_name,
                params_ty_sts_sig,
                params_sts,
                params_ty_sts_name,
                return_ty_sts_name,
                return_ty_sts_real,
                result_sts,
                target,
                func_kind,
                mode_prefix,
                func_ani_info.overload_name,
                func_ani_info.on_off_pair,
                overload_register,
                on_off_register,
            )
        if (get_name := func_ani_info.get_name) is not None:
            self.gen_getter(
                get_name,
                params_sts,
                return_ty_sts_name,
                result_sts,
                target,
                func_kind,
                mode_prefix,
            )
        if (set_name := func_ani_info.set_name) is not None:
            self.gen_setter(
                set_name,
                params_sts,
                return_ty_sts_name,
                result_sts,
                target,
                func_kind,
                mode_prefix,
            )

    def gen_any_ctor(
        self,
        ctor: GlobFuncDecl,
        ctor_ani_info: GlobFuncAniInfo,
        target: StsWriter,
        ctor_func: CtorKind,
        mode_prefix: str,
        overload_register: OverloadRegister,
        on_off_register: OnOffRegister,
    ):
        params_ty_sts_sig = []
        params_ty_sts_name = []
        params_sts = []
        args_sts = []
        for param in ctor_ani_info.sts_params:
            opt = "?" if OptionalAttr.get(param) else ""
            param_ty_ani_info = TypeAniInfo.get(self.am, param.ty)
            params_ty_sts_sig.append(param_ty_ani_info.type_sig)
            params_ty_sts_name.append(param_ty_ani_info.sts_type_in(target))
            params_sts.append(
                f"{param.name}{opt}: {param_ty_ani_info.sts_type_in(target)}"
            )
            args_sts.append(param.name)
        args_sts = ctor_ani_info.as_native_args(args_sts)
        args_sts_str = ", ".join(args_sts)
        result_sts = (
            f"{ctor_ani_info.call_native(ctor_ani_info.native_name)}({args_sts_str})"
        )

        if (ctor_name := ctor_ani_info.norm_name) is not None:
            self.gen_ctor(
                ctor_name,
                params_ty_sts_sig,
                params_sts,
                params_ty_sts_name,
                result_sts,
                target,
                ctor_func,
                mode_prefix,
                ctor_ani_info.overload_name,
                ctor_ani_info.on_off_pair,
                overload_register,
                on_off_register,
            )

    def gen_getter_decl(
        self,
        get_name: str,
        params_sts: list[str],
        return_ty_sts_name: str,
        target: StsWriter,
        func_kind: FuncKind,
        decl_prefix: str,
    ):
        params_sts_str = ", ".join(params_sts)

        target.writelns(
            f"{decl_prefix}{func_kind.get_prefix}{get_name}({params_sts_str}): {return_ty_sts_name};",
        )

    def gen_setter_decl(
        self,
        set_name: str,
        params_sts: list[str],
        return_ty_sts_name: str,
        target: StsWriter,
        func_kind: FuncKind,
        decl_prefix: str,
    ):
        params_sts_str = ", ".join(params_sts)

        target.writelns(
            f"{decl_prefix}{func_kind.set_prefix}{set_name}({params_sts_str});",
        )

    def gen_normal_func_decl(
        self,
        norm_name: str,
        async_name: str | None,
        promise_name: str | None,
        params_ty_sts_sig: list[str],
        params_sts: list[str],
        params_ty_sts_name: list[str],
        return_ty_sts_name: str,
        target: StsWriter,
        func_kind: FuncKind,
        decl_prefix: str,
        overload_name: str | None,
        on_off_pair: tuple[str, str] | None,
        overload_register: OverloadRegister,
        on_off_register: OnOffRegister,
    ):
        params_sts_str = ", ".join(params_sts)

        target.writelns(
            f"{decl_prefix}{func_kind.func_prefix}{norm_name}({params_sts_str}): {return_ty_sts_name};",
        )

        if overload_name is not None:
            overload_register.register(norm_name, overload_name)
        if on_off_pair is not None:
            on_off_register.register(
                on_off_pair,
                params_ty_sts_sig,
                norm_name,
                params_ty_sts_name,
                return_ty_sts_name,
            )

        if promise_name is not None:
            self.gen_promise_func_decl(
                promise_name,
                params_ty_sts_sig,
                params_sts,
                params_ty_sts_name,
                return_ty_sts_name,
                target,
                func_kind,
                decl_prefix,
                None,
                None,
                overload_register,
                on_off_register,
            )
        if async_name is not None:
            self.gen_async_func_decl(
                async_name,
                params_ty_sts_sig,
                params_sts,
                params_ty_sts_name,
                return_ty_sts_name,
                target,
                func_kind,
                decl_prefix,
                None,
                None,
                overload_register,
                on_off_register,
            )

    def gen_promise_func_decl(
        self,
        promise_name: str,
        params_ty_sts_sig: list[str],
        params_sts: list[str],
        params_ty_sts_name: list[str],
        return_ty_sts_name: str,
        target: StsWriter,
        func_kind: FuncKind,
        decl_prefix: str,
        overload_name: str | None,
        on_off_pair: tuple[str, str] | None,
        overload_register: OverloadRegister,
        on_off_register: OnOffRegister,
    ):
        params_sts_str = ", ".join(params_sts)
        promise_ty = f"Promise<{return_ty_sts_name}>"

        target.writelns(
            f"{decl_prefix}{func_kind.func_prefix}{promise_name}({params_sts_str}): {promise_ty};",
        )

        if overload_name is not None:
            overload_register.register(promise_name, overload_name)
        if on_off_pair is not None:
            on_off_register.register(
                on_off_pair,
                params_ty_sts_sig,
                promise_name,
                params_ty_sts_name,
                promise_ty,
            )

    def gen_async_func_decl(
        self,
        async_name: str,
        params_ty_sts_sig: list[str],
        params_sts: list[str],
        params_ty_sts_name: list[str],
        return_ty_sts_name: str,
        target: StsWriter,
        func_kind: FuncKind,
        decl_prefix: str,
        overload_name: str | None,
        on_off_pair: tuple[str, str] | None,
        overload_register: OverloadRegister,
        on_off_register: OnOffRegister,
    ):
        cbname = "callback"
        callback_ty_sts_sig = "C{std.core.Function2}"
        callback_ty_sts_name = f"_taihe_AsyncCallback<{return_ty_sts_name}>"
        callback_sts = f"{cbname}: {callback_ty_sts_name}"
        params_with_callback_sts_str = ", ".join([*params_sts, callback_sts])

        target.writelns(
            f"{decl_prefix}{func_kind.func_prefix}{async_name}({params_with_callback_sts_str}): void;",
        )

        if overload_name is not None:
            overload_register.register(async_name, overload_name)
        if on_off_pair is not None:
            on_off_register.register(
                on_off_pair,
                [*params_ty_sts_sig, callback_ty_sts_sig],
                async_name,
                [*params_ty_sts_name, callback_ty_sts_name],
                "void",
            )

    def gen_getter(
        self,
        get_name: str,
        params_sts: list[str],
        return_ty_sts_name: str,
        result_sts: str,
        target: StsWriter,
        func_kind: FuncKind,
        mode_prefix: str,
    ):
        params_sts_str = ", ".join(params_sts)

        with target.indented(
            f"{mode_prefix}{func_kind.get_prefix}{get_name}({params_sts_str}): {return_ty_sts_name} {{",
            f"}}",
        ):
            target.writelns(
                f"return {result_sts};",
            )

    def gen_setter(
        self,
        set_name: str,
        params_sts: list[str],
        return_ty_sts_name: str,
        result_sts: str,
        target: StsWriter,
        func_kind: FuncKind,
        mode_prefix: str,
    ):
        params_sts_str = ", ".join(params_sts)

        with target.indented(
            f"{mode_prefix}{func_kind.set_prefix}{set_name}({params_sts_str}) {{",
            f"}}",
        ):
            target.writelns(
                f"return {result_sts};",
            )

    def gen_normal_func(
        self,
        norm_name: str,
        async_name: str | None,
        promise_name: str | None,
        params_ty_sts_sig: list[str],
        params_sts: list[str],
        params_ty_sts_name: list[str],
        return_ty_sts_name: str,
        return_ty_sts_real: str,
        result_sts: str,
        target: StsWriter,
        func_kind: FuncKind,
        mode_prefix: str,
        overload_name: str | None,
        on_off_pair: tuple[str, str] | None,
        overload_register: OverloadRegister,
        on_off_register: OnOffRegister,
    ):
        params_sts_str = ", ".join(params_sts)

        with target.indented(
            f"{mode_prefix}{func_kind.func_prefix}{norm_name}({params_sts_str}): {return_ty_sts_name} {{",
            f"}}",
        ):
            target.writelns(
                f"return {result_sts};",
            )

        if overload_name is not None:
            overload_register.register(norm_name, overload_name)
        if on_off_pair is not None:
            on_off_register.register(
                on_off_pair,
                params_ty_sts_sig,
                norm_name,
                params_ty_sts_name,
                return_ty_sts_name,
            )

        if promise_name is not None:
            self.gen_promise_func(
                promise_name,
                params_ty_sts_sig,
                params_sts,
                params_ty_sts_name,
                return_ty_sts_name,
                return_ty_sts_real,
                result_sts,
                target,
                func_kind,
                mode_prefix,
                None,
                None,
                overload_register,
                on_off_register,
            )
        if async_name is not None:
            self.gen_async_func(
                async_name,
                params_ty_sts_sig,
                params_sts,
                params_ty_sts_name,
                return_ty_sts_name,
                return_ty_sts_real,
                result_sts,
                target,
                func_kind,
                mode_prefix,
                None,
                None,
                overload_register,
                on_off_register,
            )

    def gen_promise_func(
        self,
        promise_name: str,
        params_ty_sts_sig: list[str],
        params_sts: list[str],
        params_ty_sts_name: list[str],
        return_ty_sts_name: str,
        return_ty_sts_real: str,
        result_sts: str,
        target: StsWriter,
        func_kind: FuncKind,
        mode_prefix: str,
        overload_name: str | None,
        on_off_pair: tuple[str, str] | None,
        overload_register: OverloadRegister,
        on_off_register: OnOffRegister,
    ):
        params_sts_str = ", ".join(params_sts)
        promise_ty_sts_name = f"Promise<{return_ty_sts_name}>"

        with target.indented(
            f"{mode_prefix}{func_kind.func_prefix}{promise_name}({params_sts_str}): {promise_ty_sts_name} {{",
            f"}}",
        ):
            with target.indented(
                f"return new {promise_ty_sts_name}((resolve, reject): void => {{",
                f"}});",
            ):
                with target.indented(
                    f"taskpool.execute((): {return_ty_sts_name} => {{",
                    f"}})",
                ):
                    target.writelns(
                        f"return {result_sts};",
                    )
                with target.indented(
                    f".then((ret: Any): void => {{",
                    f"}})",
                ):
                    target.writelns(
                        f"resolve(ret as {return_ty_sts_real});",
                    )
                with target.indented(
                    f".catch((ret: Any): void => {{",
                    f"}});",
                ):
                    target.writelns(
                        f"reject(ret as Error);",
                    )

        if overload_name is not None:
            overload_register.register(promise_name, overload_name)
        if on_off_pair is not None:
            on_off_register.register(
                on_off_pair,
                params_ty_sts_sig,
                promise_name,
                params_ty_sts_name,
                promise_ty_sts_name,
            )

    def gen_async_func(
        self,
        async_name: str,
        params_ty_sts_sig: list[str],
        params_sts: list[str],
        params_ty_sts_name: list[str],
        return_ty_sts_name: str,
        return_ty_sts_real: str,
        result_sts: str,
        target: StsWriter,
        func_kind: FuncKind,
        mode_prefix: str,
        overload_name: str | None,
        on_off_pair: tuple[str, str] | None,
        overload_register: OverloadRegister,
        on_off_register: OnOffRegister,
    ):
        cbname = "callback"
        callback_ty_sts_sig = "C{std.core.Function2}"
        callback_ty_sts_name = f"_taihe_AsyncCallback<{return_ty_sts_name}>"
        callback_sts = f"{cbname}: {callback_ty_sts_name}"
        params_with_callback_sts_str = ", ".join([*params_sts, callback_sts])

        with target.indented(
            f"{mode_prefix}{func_kind.func_prefix}{async_name}({params_with_callback_sts_str}): void {{",
            f"}}",
        ):
            with target.indented(
                f"taskpool.execute((): {return_ty_sts_name} => {{",
                f"}})",
            ):
                target.writelns(
                    f"return {result_sts};",
                )
            with target.indented(
                f".then((ret: Any): void => {{",
                f"}})",
            ):
                target.writelns(
                    f"{cbname}(null, ret as {return_ty_sts_real});",
                )
            with target.indented(
                f".catch((ret: Any): void => {{",
                f"}});",
            ):
                target.writelns(
                    f"{cbname}(ret as _taihe_BusinessError, undefined);",
                )

        if overload_name is not None:
            overload_register.register(async_name, overload_name)
        if on_off_pair is not None:
            on_off_register.register(
                on_off_pair,
                [*params_ty_sts_sig, callback_ty_sts_sig],
                async_name,
                [*params_ty_sts_name, callback_ty_sts_name],
                "void",
            )

    def gen_ctor(
        self,
        ctor_name: str,
        params_ty_sts_sig: list[str],
        params_sts: list[str],
        params_ty_sts_name: list[str],
        result_sts: str,
        target: StsWriter,
        ctor_kind: CtorKind,
        mode_prefix: str,
        overload_name: str | None,
        on_off_pair: tuple[str, str] | None,
        overload_register: OverloadRegister,
        on_off_register: OnOffRegister,
    ):
        params_sts_str = ", ".join(params_sts)

        with target.indented(
            f"{mode_prefix}{ctor_kind.func_prefix}{ctor_name}({params_sts_str}) {{",
            f"}}",
        ):
            target.writelns(
                f"this._taihe_moveFrom({result_sts});",
            )

        if overload_name is not None:
            overload_register.register(ctor_name, overload_name)
        if on_off_pair is not None:
            on_off_register.register(
                on_off_pair,
                params_ty_sts_sig,
                ctor_name,
                params_ty_sts_name,
                "void",
            )

    def gen_overload_func(
        self,
        overload_name: str,
        overload_info: OverloadInfo,
        target: StsWriter,
        func_kind: FuncKind,
        mode_prefix: str,
    ):
        with target.indented(
            f"{mode_prefix}{func_kind.overload_prefix}{overload_name} {{",
            f"}}",
        ):
            for orig in overload_info:
                target.writelns(
                    f"{orig},",
                )

    def gen_full_on_off_func(
        self,
        on_off_name: str,
        on_off_info: OnOffInfo,
        target: StsWriter,
        func_kind: FuncKind,
        mode_prefix: str,
    ):
        if not any(len(funcs) > 1 for funcs in on_off_info.values()):
            return self.gen_half_on_off_func(
                on_off_name,
                on_off_info,
                target,
                func_kind,
                mode_prefix,
            )
        params_len = max(len(params_ty_sts_sig) for params_ty_sts_sig in on_off_info)
        args_sts = [f"p_{i}" for i in range(params_len)]
        params_sts = ["type: Object", *(f"{arg_sts}?: Object" for arg_sts in args_sts)]
        params_sts_str = ", ".join(params_sts)

        with target.indented(
            f"{mode_prefix}{func_kind.func_prefix}{on_off_name}({params_sts_str}): Any {{",
            f"}}",
        ):
            with target.indented(
                f"switch (type as string) {{",
                f"}}",
                indent="",
            ):
                for on_off_cases in on_off_info.values():
                    for on_off_type, (
                        orig_name,
                        params_ty_sts_name,
                        _,
                    ) in on_off_cases.items():
                        args_sts_str = ", ".join(
                            f"{arg_sts} as {param_ty_sts_name}"
                            for arg_sts, param_ty_sts_name in zip(
                                args_sts, params_ty_sts_name, strict=False
                            )
                        )
                        target.writelns(
                            f'case "{on_off_type}": return {func_kind.call_from_local(orig_name)}({args_sts_str});',
                        )
                target.writelns(
                    f"default: throw new Error(`Unknown tag: ${{type}}`);",
                )
        return None

    def gen_half_on_off_func(
        self,
        on_off_name: str,
        on_off_info: OnOffInfo,
        target: StsWriter,
        func_kind: FuncKind,
        mode_prefix: str,
    ):
        for params_ty_sts_sig, on_off_cases in on_off_info.items():
            params_ty_sts_sum: list[set[str]] = [
                set() for _ in range(len(params_ty_sts_sig))
            ]
            return_ty_sts_sum: set[str] = set()
            for _, params_ty_sts_name, return_ty_sts_name in on_off_cases.values():
                for param_ty_sts_sum, param_ty_sts_name in zip(
                    params_ty_sts_sum, params_ty_sts_name, strict=True
                ):
                    param_ty_sts_sum.add(param_ty_sts_name)
                return_ty_sts_sum.add(return_ty_sts_name)
            params_sts = ["type: string"]
            args_sts = []
            for index, param_ty_sts_sum in enumerate(params_ty_sts_sum):
                arg_sts = f"p_{index}"
                param_ty = " | ".join(param_ty_sts_sum)
                params_sts.append(f"{arg_sts}?: {param_ty}")
                args_sts.append(arg_sts)
            params_sts_str = ", ".join(params_sts)
            args_sts_str = ", ".join(args_sts)
            return_ty = " | ".join(return_ty_sts_sum)

            with target.indented(
                f"{mode_prefix}{func_kind.func_prefix}{on_off_name}({params_sts_str}): {return_ty} {{",
                f"}}",
            ):
                with target.indented(
                    f"switch (type) {{",
                    f"}}",
                    indent="",
                ):
                    for on_off_type, (
                        orig_name,
                        params_ty_sts_name,
                        _,
                    ) in on_off_cases.items():
                        args_sts_str = ", ".join(
                            f"{arg_sts} as {param_ty_sts_name}"
                            for arg_sts, param_ty_sts_name in zip(
                                args_sts, params_ty_sts_name, strict=True
                            )
                        )
                        target.writelns(
                            f'case "{on_off_type}": return {func_kind.call_from_local(orig_name)}({args_sts_str});',
                        )
                    target.writelns(
                        f"default: throw new Error(`Unknown tag: ${{type}}`);",
                    )

    def gen_overload_ctor(
        self,
        overload_name: str,
        overload_info: OverloadInfo,
        target: StsWriter,
        ctor_kind: CtorKind,
        mode_prefix: str,
    ):
        with target.indented(
            f"{mode_prefix}{ctor_kind.overload_prefix}{overload_name} {{",
            f"}}",
        ):
            for orig in overload_info:
                target.writelns(
                    f"{orig},",
                )

    def gen_full_on_off_ctor(
        self,
        on_off_name: str,
        on_off_info: OnOffInfo,
        target: StsWriter,
        ctor_kind: CtorKind,
        mode_prefix: str,
    ):
        if not any(len(funcs) > 1 for funcs in on_off_info.values()):
            return self.gen_half_on_off_ctor(
                on_off_name,
                on_off_info,
                target,
                ctor_kind,
                mode_prefix,
            )
        params_len = max(len(params_ty_sts_sig) for params_ty_sts_sig in on_off_info)
        args_sts = [f"p_{i}" for i in range(params_len)]
        params_sts = ["type: Object", *(f"{arg_sts}?: Object" for arg_sts in args_sts)]
        params_sts_str = ", ".join(params_sts)

        with target.indented(
            f"{mode_prefix}{ctor_kind.func_prefix}{on_off_name}({params_sts_str}) {{",
            f"}}",
        ):
            with target.indented(
                f"switch (type as object) {{",
                f"}}",
                indent="",
            ):
                for on_off_cases in on_off_info.values():
                    for on_off_type, (
                        orig_name,
                        params_ty_sts_name,
                        _,
                    ) in on_off_cases.items():
                        args_sts_str = ", ".join(
                            f"{arg_sts} as {param_ty_sts_name}"
                            for arg_sts, param_ty_sts_name in zip(
                                args_sts, params_ty_sts_name, strict=False
                            )
                        )
                        target.writelns(
                            f'case "{on_off_type}": {ctor_kind.call_from_local(orig_name)}({args_sts_str});',
                        )
                target.writelns(
                    f"default: throw new Error(`Unknown tag: ${{type}}`);",
                )
        return None

    def gen_half_on_off_ctor(
        self,
        on_off_name: str,
        on_off_func: OnOffInfo,
        target: StsWriter,
        ctor_kind: CtorKind,
        mode_prefix: str,
    ):
        for params_ty_sts_sig, on_off_cases in on_off_func.items():
            params_ty_sts_sum: list[set[str]] = [
                set() for _ in range(len(params_ty_sts_sig))
            ]
            return_ty_sts_sum: set[str] = set()
            for _, params_ty_sts_name, return_ty_sts_name in on_off_cases.values():
                for param_ty_sts_sum, param_ty_sts_name in zip(
                    params_ty_sts_sum, params_ty_sts_name, strict=True
                ):
                    param_ty_sts_sum.add(param_ty_sts_name)
                return_ty_sts_sum.add(return_ty_sts_name)
            params_sts = ["type: string"]
            args_sts = []
            for index, param_ty_sts_sum in enumerate(params_ty_sts_sum):
                arg_sts = f"p_{index}"
                param_ty = " | ".join(param_ty_sts_sum)
                params_sts.append(f"{arg_sts}?: {param_ty}")
                args_sts.append(arg_sts)
            params_sts_str = ", ".join(params_sts)
            args_sts_str = ", ".join(args_sts)
            return_ty = " | ".join(return_ty_sts_sum)  # noqa: F841

            with target.indented(
                f"{mode_prefix}{ctor_kind.func_prefix}{on_off_name}({params_sts_str}) {{",
                f"}}",
            ):
                with target.indented(
                    f"switch (type) {{",
                    f"}}",
                    indent="",
                ):
                    for on_off_type, (
                        orig_name,
                        params_ty_sts_name,
                        _,
                    ) in on_off_cases.items():
                        args_sts_str = ", ".join(
                            f"{arg_sts} as {param_ty_sts_name}"
                            for arg_sts, param_ty_sts_name in zip(
                                args_sts, params_ty_sts_name, strict=True
                            )
                        )
                        target.writelns(
                            f'case "{on_off_type}": {ctor_kind.call_from_local(orig_name)}({args_sts_str});',
                        )
                    target.writelns(
                        f"default: throw new Error(`Unknown tag: ${{type}}`);",
                    )

    def gen_reverse_func(
        self,
        func: GlobFuncDecl | IfaceMethodDecl,
        func_ani_info: GlobFuncAniInfo | IfaceMethodAniInfo,
        target: StsWriter,
        func_kind: FuncKind | CtorKind,
    ):
        args_sts = []
        params_sts = func_kind.reverse_base_params()
        for param in func.params:
            param_ty_ani_info = TypeAniInfo.get(self.am, param.ty)
            args_sts.append(param.name)
            params_sts.append(f"{param.name}: {param_ty_ani_info.sts_type_in(target)}")
        args_sts = func_ani_info.as_normal_args(args_sts)
        params_sts_str = ", ".join(params_sts)
        if isinstance(return_ty := func.return_ty, NonVoidType):
            return_ty_ani_info = TypeAniInfo.get(self.am, return_ty)
            return_ty_sts_name = return_ty_ani_info.sts_type_in(target)
            return_ty_sts_real = return_ty_ani_info.sts_type_in(target)
        else:
            return_ty_sts_name = "void"
            return_ty_sts_real = "undefined"

        with target.indented(
            f"function {func_ani_info.reverse_name}({params_sts_str}): {return_ty_sts_name} {{",
            f"}}",
        ):
            if (norm_name := func_ani_info.norm_name) is not None:
                args_sts_str = ", ".join(args_sts)
                target.writelns(
                    f"return {func_kind.call_from_reverse(norm_name)}({args_sts_str});",
                )
            elif (get_name := func_ani_info.get_name) is not None:
                target.writelns(
                    f"return {func_kind.call_from_reverse(get_name)};",
                )
            elif (set_name := func_ani_info.set_name) is not None:
                target.writelns(
                    f"{func_kind.call_from_reverse(set_name)} = {args_sts[0]};",
                )
            elif (promise_name := func_ani_info.promise_name) is not None:
                args_sts_str = ", ".join(args_sts)
                target.writelns(
                    f"return await {func_kind.call_from_reverse(promise_name)}({args_sts_str});",
                )
            elif (async_name := func_ani_info.async_name) is not None:
                with target.indented(
                    f"return await new Promise<{return_ty_sts_name}>((resolve, reject) => {{",
                    f"}});",
                ):
                    with target.indented(
                        f"let callback: _taihe_AsyncCallback<{return_ty_sts_name}> = (err: _taihe_BusinessError | null, res?: {return_ty_sts_real}): void => {{",
                        f"}}",
                    ):
                        with target.indented(
                            f"if (err !== null) {{",
                            f"}}",
                        ):
                            target.writelns(
                                f"reject(err);",
                            )
                        with target.indented(
                            f"else {{",
                            f"}}",
                        ):
                            target.writelns(
                                f"resolve(res as {return_ty_sts_real});",
                            )
                    args_with_cbname_sts_str = ", ".join([*args_sts, "callback"])
                    target.writelns(
                        f"{func_kind.call_from_reverse(async_name)}({args_with_cbname_sts_str});",
                    )
            else:
                target.writelns(
                    f"throw new Error(`No valid reverse function found`);",
                )

    def gen_utils(
        self,
        target: StsWriter,
    ):
        target.add_import_decl("@ohos.base", "BusinessError", "_taihe_BusinessError")
        target.writelns(
            "type _taihe_AsyncCallback<T, E = void> = (error: _taihe_BusinessError<E> | null, data: T | undefined) => void;",
        )

        target.writelns(
            "function _taihe_fromArrayBufferToBigInt(arr: ArrayBuffer): BigInt {",
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
            "function _taihe_fromBigIntToArrayBuffer(val: BigInt, blk: int): ArrayBuffer {",
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

        target.writelns(
            "native function _taihe_nativeInvoke(",
            "    castPtr: long, funcPtr: long, dataPtr: long,",
            "    arg_0?: Object, arg_1?: Object, arg_2?: Object, arg_3?: Object,",
            "    arg_4?: Object, arg_5?: Object, arg_6?: Object, arg_7?: Object,",
            "    arg_8?: Object, arg_9?: Object, arg_a?: Object, arg_b?: Object,",
            "    arg_c?: Object, arg_d?: Object, arg_e?: Object, arg_f?: Object,",
            "): Object | null | undefined;",
        )
        target.writelns(
            "function _taihe_makeCallback(castPtr: long, funcPtr: long, dataPtr: long) {",
            "    let callback = (",
            "        arg_0?: Object, arg_1?: Object, arg_2?: Object, arg_3?: Object,",
            "        arg_4?: Object, arg_5?: Object, arg_6?: Object, arg_7?: Object,",
            "        arg_8?: Object, arg_9?: Object, arg_a?: Object, arg_b?: Object,",
            "        arg_c?: Object, arg_d?: Object, arg_e?: Object, arg_f?: Object,",
            "    ): Object | null | undefined => {",
            "        return _taihe_nativeInvoke(",
            "            castPtr, funcPtr, dataPtr,",
            "            arg_0, arg_1, arg_2, arg_3,",
            "            arg_4, arg_5, arg_6, arg_7,",
            "            arg_8, arg_9, arg_a, arg_b,",
            "            arg_c, arg_d, arg_e, arg_f,",
            "        );",
            "    };",
            "    _taihe_registry.register(callback, dataPtr, callback);",
            "    return callback;",
            "}",
        )

        target.writelns(
            "native function _taihe_objDrop(dataPtr: long): void;",
            "native function _taihe_objDup(dataPtr: long): long;",
            "const _taihe_registry = new FinalizationRegistry<long>(_taihe_objDrop);",
        )