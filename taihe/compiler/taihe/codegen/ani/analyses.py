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

from abc import ABCMeta, abstractmethod
from dataclasses import dataclass
from typing import Literal

from typing_extensions import override

from taihe.codegen.abi.writer import CSourceWriter
from taihe.codegen.ani.writer import StsWriter
from taihe.codegen.cpp.analyses import (
    EnumCppInfo,
    IfaceCppInfo,
    StructCppInfo,
    TypeCppInfo,
    UnionCppInfo,
)
from taihe.semantics.declarations import (
    AttrItemDecl,
    EnumDecl,
    GlobFuncDecl,
    IfaceDecl,
    IfaceMethodDecl,
    IfaceParentDecl,
    PackageDecl,
    PackageGroup,
    ParamDecl,
    StructDecl,
    StructFieldDecl,
    UnionDecl,
    UnionFieldDecl,
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
    StringType,
    StructType,
    Type,
    UnionType,
)
from taihe.semantics.visitor import TypeVisitor
from taihe.utils.analyses import AbstractAnalysis, AnalysisManager
from taihe.utils.exceptions import AdhocError
from taihe.utils.sources import SourceLocation


def raise_adhoc_error(
    am: AnalysisManager,
    msg: str,
    loc: SourceLocation | None,
):
    am.diagnostics_manager.emit(AdhocError(msg, loc=loc))


def check_attr_args(
    am: AnalysisManager,
    attr: AttrItemDecl,
    pattern: str,
) -> bool:
    expect_dict = {"s": str, "i": int, "f": float, "b": bool}
    trailing = "/"
    if len(pattern) >= 2 and pattern[-1] == "*":
        trailing = pattern[-2]
        pattern = pattern[:-2]
    if trailing == "/" and len(attr.args) != len(pattern):
        raise_adhoc_error(
            am,
            f"@{attr.name} expects exactly {len(pattern)} arguments",
            attr.loc,
        )
        return False
    if len(attr.args) < len(pattern):
        raise_adhoc_error(
            am,
            f"@{attr.name} expects at least {len(pattern)} arguments",
            attr.loc,
        )
        return False
    result = []
    for i, arg in enumerate(attr.args):
        p = pattern[i] if i < len(pattern) else trailing
        if not isinstance(arg, expect_dict[p]):
            raise_adhoc_error(
                am,
                f"@{attr.name} expects {i + 1}th argument to be {expect_dict[p].__name__}",
                attr.loc,
            )
            return False
        result.append(arg)
    return True


@dataclass(repr=False)
class ANIType:
    hint: str
    base: "ANIBaseType"

    def __repr__(self) -> str:
        return f"ani_{self.hint}"

    def __hash__(self) -> int:
        return hash(self.hint)

    @property
    def suffix(self) -> str:
        return self.base.hint[0].upper() + self.base.hint[1:]

    @property
    def fixedarray(self) -> "ANIFixedArrayType":
        pass
        return self.base.fixedarray_hint


@dataclass(repr=False)
class ANIFixedArrayType(ANIType):
    def __hash__(self) -> int:
        return hash(self.hint)


@dataclass(repr=False)
class ANIBaseType(ANIType):
    fixedarray_hint: "ANIFixedArrayType | None"

    def __init__(self, hint: str):
        super().__init__(hint, self)
        self.fixedarray_hint = None

    def __hash__(self) -> int:
        return hash(self.hint)


ANI_REF = ANIBaseType(hint="ref")
ANI_FIXEDARRAY_REF = ANIFixedArrayType(hint="fixedarray_ref", base=ANI_REF)
ANI_REF.fixedarray_hint = ANI_FIXEDARRAY_REF

ANI_BOOLEAN = ANIBaseType(hint="boolean")
ANI_FIXEDARRAY_BOOLEAN = ANIFixedArrayType(hint="fixedarray_boolean", base=ANI_REF)
ANI_BOOLEAN.fixedarray_hint = ANI_FIXEDARRAY_BOOLEAN

ANI_FLOAT = ANIBaseType(hint="float")
ANI_FIXEDARRAY_FLOAT = ANIFixedArrayType(hint="fixedarray_float", base=ANI_REF)
ANI_FLOAT.fixedarray_hint = ANI_FIXEDARRAY_FLOAT

ANI_DOUBLE = ANIBaseType(hint="double")
ANI_FIXEDARRAY_DOUBLE = ANIFixedArrayType(hint="fixedarray_double", base=ANI_REF)
ANI_DOUBLE.fixedarray_hint = ANI_FIXEDARRAY_DOUBLE

ANI_BYTE = ANIBaseType(hint="byte")
ANI_FIXEDARRAY_BYTE = ANIFixedArrayType(hint="fixedarray_byte", base=ANI_REF)
ANI_BYTE.fixedarray_hint = ANI_FIXEDARRAY_BYTE

ANI_SHORT = ANIBaseType(hint="short")
ANI_FIXEDARRAY_SHORT = ANIFixedArrayType(hint="fixedarray_short", base=ANI_REF)
ANI_SHORT.fixedarray_hint = ANI_FIXEDARRAY_SHORT

ANI_INT = ANIBaseType(hint="int")
ANI_FIXEDARRAY_INT = ANIFixedArrayType(hint="fixedarray_int", base=ANI_REF)
ANI_INT.fixedarray_hint = ANI_FIXEDARRAY_INT

ANI_LONG = ANIBaseType(hint="long")
ANI_FIXEDARRAY_LONG = ANIFixedArrayType(hint="fixedarray_long", base=ANI_REF)
ANI_LONG.fixedarray_hint = ANI_FIXEDARRAY_LONG

ANI_OBJECT = ANIType(hint="object", base=ANI_REF)
ANI_ARRAY = ANIType(hint="array_ref", base=ANI_REF)  # TODO: Array
ANI_FN_OBJECT = ANIType(hint="fn_object", base=ANI_REF)
ANI_ENUM_ITEM = ANIType(hint="enum_item", base=ANI_REF)
ANI_STRING = ANIType(hint="string", base=ANI_REF)
ANI_ARRAYBUFFER = ANIType(hint="arraybuffer", base=ANI_REF)


@dataclass(repr=False)
class ANIFuncLike:
    hint: str

    def __repr__(self) -> str:
        return f"ani_{self.hint}"

    def __hash__(self) -> int:
        return hash(self.hint)

    @property
    def suffix(self) -> str:
        return self.hint[0].upper() + self.hint[1:]

    @property
    def upper(self) -> str:
        return self.hint.upper()


ANI_FUNCTION = ANIFuncLike("function")
ANI_METHOD = ANIFuncLike("method")


@dataclass(repr=False)
class ANIScope:
    hint: str
    member: ANIFuncLike

    def __repr__(self) -> str:
        return f"ani_{self.hint}"

    def __hash__(self) -> int:
        return hash(self.hint)

    @property
    def suffix(self) -> str:
        return self.hint[0].upper() + self.hint[1:]

    @property
    def upper(self) -> str:
        return self.hint.upper()


ANI_CLASS = ANIScope("class", ANI_METHOD)
ANI_MODULE = ANIScope("module", ANI_FUNCTION)
ANI_NAMESPACE = ANIScope("namespace", ANI_FUNCTION)


class Path:
    def __init__(self, package: str | None = None, path: str | None = None) -> None:
        self.package = package
        self.path = path
        self.ani_path = []
        if self.package is not None:
            self.ani_path.append(self.package)
        if self.path is not None:
            self.ani_path.extend(self.path.split("/"))


class Namespace:
    def __init__(self, name: str, parent: "Namespace | Path") -> None:
        self.name = name
        self.parent = parent

        self.children: dict[str, Namespace] = {}
        self.packages: list[PackageDecl] = []
        self.is_default = False
        self.injected_heads: list[str] = []
        self.injected_codes: list[str] = []

        if not isinstance(parent, Namespace):
            self.module = self
            self.path: list[str] = []
            self.scope = ANI_MODULE
            self.ani_path = [*parent.ani_path, *name.split(".")]
        else:
            self.module = parent.module
            self.path: list[str] = [*parent.path, name]
            self.scope = ANI_NAMESPACE
            self.ani_path = [*parent.ani_path, name]

        self.impl_desc = "L" + "/".join(self.ani_path) + ";"

    def add_path(
        self,
        path: list[str],
        pkg: PackageDecl,
        is_default: bool,
    ) -> "Namespace":
        if not path:
            self.packages.append(pkg)
            self.is_default |= is_default
            return self
        head, *tail = path
        child = self.children.setdefault(head, Namespace(head, self))
        return child.add_path(tail, pkg, is_default)

    def get_member(
        self,
        target: StsWriter,
        sts_name: str,
        member_is_default: bool,
    ) -> str:
        if not isinstance(self.parent, Namespace):
            scope_name = "__" + "".join(c if c.isalnum() else "_" for c in self.name)
            if member_is_default:
                decl_name = f"{scope_name}_default"
                target.add_import_default(f"{self.name}", decl_name)
                return decl_name
            target.add_import_module(f"./{self.name}", scope_name)  # TODO: Remove `./`
        else:
            scope_name = self.parent.get_member(target, self.name, self.is_default)
        return f"{scope_name}.{sts_name}"


class PackageGroupANIInfo(AbstractAnalysis[PackageGroup]):
    def __init__(self, am: AnalysisManager, pg: PackageGroup) -> None:
        super().__init__(am, pg)
        self.am = am
        self.pg = pg

        self.module_dict: dict[str, Namespace] = {}
        self.package_map: dict[PackageDecl, Namespace] = {}

        self.path = Path(
            self.am.compiler_invocation.arkts_module_prefix,
            self.am.compiler_invocation.arkts_path_prefix,
        )

        for pkg in pg.packages:
            if (namespace_attr := pkg.get_last_attr("namespace")) and check_attr_args(
                am, namespace_attr, "ss*"
            ):
                module_name, *segments = namespace_attr.args
                path = [part for segment in segments for part in segment.split(".")]
            else:
                module_name = pkg.name
                path = []

            is_default = pkg.get_last_attr("sts_export_default") is not None

            mod = self.module_dict.setdefault(
                module_name,
                Namespace(module_name, self.path),
            )
            ns = self.package_map[pkg] = mod.add_path(path, pkg, is_default)

            for sts_inject in pkg.get_all_attrs("sts_inject_into_module"):
                if check_attr_args(am, sts_inject, "s"):
                    (head,) = sts_inject.args
                    mod.injected_heads.append(head)

            for sts_inject in pkg.get_all_attrs("sts_inject"):
                if check_attr_args(am, sts_inject, "s"):
                    (code,) = sts_inject.args
                    ns.injected_codes.append(code)

    def get_namespace(self, pkg: PackageDecl) -> Namespace:
        return self.package_map[pkg]


@dataclass
class ANINativeFuncInfo:
    sts_native_name: str
    full_name: str


@dataclass
class ANIRegisterInfo:
    parent_scope: ANIScope
    impl_desc: str
    member_infos: list[ANINativeFuncInfo]


class PackageANIInfo(AbstractAnalysis[PackageDecl]):
    def __init__(self, am: AnalysisManager, p: PackageDecl) -> None:
        super().__init__(am, p)
        self.am = am
        self.p = p

        self.header = f"{p.name}.ani.hpp"
        self.source = f"{p.name}.ani.cpp"

        self.cpp_ns = "::".join(p.segments)

        pg_ani_info = PackageGroupANIInfo.get(am, p.parent_group)
        self.ns = pg_ani_info.get_namespace(p)


class GlobFuncANIInfo(AbstractAnalysis[GlobFuncDecl]):
    def __init__(self, am: AnalysisManager, f: GlobFuncDecl) -> None:
        super().__init__(am, f)
        self.am = am
        self.f = f

        self.sts_native_name = f"{f.name}_inner"

        self.sts_static_scope = None
        self.sts_ctor_scope = None

        self.sts_func_name = None
        self.on_off_type = None
        self.get_name = None
        self.set_name = None

        self.sts_async_name = None
        self.sts_promise_name = None

        if self.resolve_ctor() or (
            self.resolve_static() and (self.resolve_getter() or self.resolve_setter())
        ):
            pass
        elif self.resolve_on_off() or self.resolve_normal():
            self.resolve_async()
            self.resolve_promise()

        self.sts_params: list[ParamDecl] = []
        for param in f.params:
            if param.get_last_attr("sts_this"):
                continue
            self.sts_params.append(param)

    def resolve_ctor(self) -> bool:
        if (ctor_attr := self.f.get_last_attr("ctor")) is None:
            return False
        if not check_attr_args(self.am, ctor_attr, "s"):
            return True
        (self.sts_ctor_scope,) = ctor_attr.args
        return True

    def resolve_static(self) -> bool:
        if (static_attr := self.f.get_last_attr("static")) is None:
            return False
        if not check_attr_args(self.am, static_attr, "s"):
            return True
        (self.sts_static_scope,) = static_attr.args
        return True

    def resolve_getter(self) -> bool:
        if (get_attr := self.f.get_last_attr("get")) is None:
            return False
        if len(self.f.params) != 0 or self.f.return_ty_ref is None:
            raise_adhoc_error(
                self.am,
                "@get method should take no parameters and return non-void",
                self.f.loc,
            )
            return True
        if get_attr.args and check_attr_args(self.am, get_attr, "s"):
            (get_name,) = get_attr.args
        elif self.f.name[:3].lower() == "get":
            get_name = self.f.name[3:]
            get_name = get_name[0].lower() + get_name[1:]
        else:
            raise_adhoc_error(
                self.am,
                '@get method name must start with "Get/get" or have @get argument',
                self.f.loc,
            )
            return True
        self.get_name = get_name
        return True

    def resolve_setter(self) -> bool:
        if (set_attr := self.f.get_last_attr("set")) is None:
            return False
        if len(self.f.params) != 1 or self.f.return_ty_ref is not None:
            raise_adhoc_error(
                self.am,
                "@set method should have one parameter and return void",
                self.f.loc,
            )
            return True
        if set_attr.args and check_attr_args(self.am, set_attr, "s"):
            (set_name,) = set_attr.args
        elif self.f.name[:3].lower() == "set":
            set_name = self.f.name[3:]
            set_name = set_name[0].lower() + set_name[1:]
        else:
            raise_adhoc_error(
                self.am,
                '@set method name must start with "Set/set" or have @set argument',
                self.f.loc,
            )
            return True
        self.set_name = set_name
        return True

    def resolve_on_off(self) -> bool:
        if (on_off_attr := self.f.get_last_attr("on_off")) is None:
            return False
        if on_off_attr.args:
            if not check_attr_args(self.am, on_off_attr, "s"):
                return True
            (type_name,) = on_off_attr.args
        else:
            type_name = None
        if overload_attr := self.f.get_last_attr("overload"):
            if not check_attr_args(self.am, overload_attr, "s"):
                return True
            (func_name,) = overload_attr.args
            if type_name is None:
                if self.f.name[: len(func_name)].lower() == func_name.lower():
                    type_name = self.f.name[len(func_name) :]
                    type_name = type_name[0].lower() + type_name[1:]
                else:
                    raise_adhoc_error(
                        self.am,
                        f"@on_off method name must start with {func_name}",
                        self.f.loc,
                    )
                    return True
        else:
            for func_name in ("on", "off"):
                if self.f.name[: len(func_name)].lower() == func_name.lower():
                    if type_name is None:
                        type_name = self.f.name[len(func_name) :]
                        type_name = type_name[0].lower() + type_name[1:]
                    break
            else:
                raise_adhoc_error(
                    self.am,
                    '@on_off method name must start with "On/on/Off/off" or use together with @overload',
                    self.f.loc,
                )
                return True
        self.sts_func_name = func_name
        self.on_off_type = type_name
        return True

    def resolve_normal(self) -> bool:
        if overload_attr := self.f.get_last_attr("overload"):
            if not check_attr_args(self.am, overload_attr, "s"):
                return True
            (func_name,) = overload_attr.args
        else:
            if self.am.compiler_invocation.sts_keep_name:
                func_name = self.f.name
            else:
                func_name = self.f.name[0].lower() + self.f.name[1:]
        self.sts_func_name = func_name
        return True

    def resolve_async(self) -> bool:
        if (async_attr := self.f.get_last_attr("gen_async")) is None:
            return False
        if self.sts_func_name is None:
            return True
        if async_attr.args and check_attr_args(self.am, async_attr, "s"):
            (self.sts_async_name,) = async_attr.args
        elif self.sts_func_name[-4:].lower() == "sync":
            self.sts_async_name = self.sts_func_name[:-4]
        else:
            raise_adhoc_error(
                self.am,
                '@gen_async method name must end with "Sync" or have @gen_async argument',
                self.f.loc,
            )
        return True

    def resolve_promise(self) -> bool:
        if (promise_attr := self.f.get_last_attr("gen_promise")) is None:
            return False
        if self.sts_func_name is None:
            return True
        if promise_attr.args and check_attr_args(self.am, promise_attr, "s"):
            (self.sts_promise_name,) = promise_attr.args
        elif self.sts_func_name[-4:].lower() == "sync":
            self.sts_promise_name = self.sts_func_name[:-4]
        else:
            raise_adhoc_error(
                self.am,
                '@gen_promise method name must end with "Sync" or have @gen_promise argument',
                self.f.loc,
            )
        return True

    def call_native_with(self, sts_args: list[str], this: str = "this") -> str:
        arg = iter(sts_args)
        sts_native_args: list[str] = []
        for param in self.f.params:
            if param.get_last_attr("sts_this"):
                sts_native_args.append(this)
                continue
            sts_native_args.append(next(arg))
        sts_native_args_str = ", ".join(sts_native_args)
        return f"{self.sts_native_name}({sts_native_args_str})"


class IfaceMethodANIInfo(AbstractAnalysis[IfaceMethodDecl]):
    def __init__(self, am: AnalysisManager, f: IfaceMethodDecl) -> None:
        super().__init__(am, f)
        self.am = am
        self.f = f

        self.sts_native_name = f"{f.name}_inner"

        self.ani_method_name = None

        self.sts_method_name = None
        self.get_name = None
        self.set_name = None
        self.on_off_type = None

        self.sts_async_name = None
        self.sts_promise_name = None

        if self.resolve_getter() or self.resolve_setter():
            pass
        elif self.resolve_on_off() or self.resolve_normal():
            self.resolve_async()
            self.resolve_promise()

        self.sts_params: list[ParamDecl] = []
        for param in f.params:
            if param.get_last_attr("sts_this"):
                continue
            self.sts_params.append(param)

    def resolve_getter(self) -> bool:
        if (get_attr := self.f.get_last_attr("get")) is None:
            return False
        if len(self.f.params) != 0 or self.f.return_ty_ref is None:
            raise_adhoc_error(
                self.am,
                "@get method should take no parameters and return non-void",
                self.f.loc,
            )
            return True
        if get_attr.args and check_attr_args(self.am, get_attr, "s"):
            (get_name,) = get_attr.args
        elif self.f.name[:3].lower() == "get":
            get_name = self.f.name[3:]
            get_name = get_name[0].lower() + get_name[1:]
        else:
            raise_adhoc_error(
                self.am,
                '@get method name must start with "Get/get" or have @get argument',
                self.f.loc,
            )
            return True
        self.ani_method_name = f"<get>{get_name}"
        self.get_name = get_name
        return True

    def resolve_setter(self) -> bool:
        if (set_attr := self.f.get_last_attr("set")) is None:
            return False
        if len(self.f.params) != 1 or self.f.return_ty_ref is not None:
            raise_adhoc_error(
                self.am,
                "@set method should have one parameter and return void",
                self.f.loc,
            )
            return True
        if set_attr.args and check_attr_args(self.am, set_attr, "s"):
            (set_name,) = set_attr.args
        elif self.f.name[:3].lower() == "set":
            set_name = self.f.name[3:]
            set_name = set_name[0].lower() + set_name[1:]
        else:
            raise_adhoc_error(
                self.am,
                '@set method name must start with "Set/set" or have @set argument',
                self.f.loc,
            )
            return True
        self.ani_method_name = f"<set>{set_name}"
        self.set_name = set_name
        return True

    def resolve_on_off(self) -> bool:
        if (on_off_attr := self.f.get_last_attr("on_off")) is None:
            return False
        if on_off_attr.args:
            if not check_attr_args(self.am, on_off_attr, "s"):
                return True
            (type_name,) = on_off_attr.args
        else:
            type_name = None
        if overload_attr := self.f.get_last_attr("overload"):
            if not check_attr_args(self.am, overload_attr, "s"):
                return True
            (method_name,) = overload_attr.args
            if type_name is None:
                if self.f.name[: len(method_name)].lower() == method_name.lower():
                    type_name = self.f.name[len(method_name) :]
                    type_name = type_name[0].lower() + type_name[1:]
                else:
                    raise_adhoc_error(
                        self.am,
                        f"@on_off method name must start with {method_name}",
                        self.f.loc,
                    )
                    return True
        else:
            for method_name in ("on", "off"):
                if self.f.name[: len(method_name)].lower() == method_name.lower():
                    if type_name is None:
                        type_name = self.f.name[len(method_name) :]
                        type_name = type_name[0].lower() + type_name[1:]
                    break
            else:
                raise_adhoc_error(
                    self.am,
                    '@on_off method name must start with "On/on/Off/off" or use together with @overload',
                    self.f.loc,
                )
                return True
        self.ani_method_name = method_name
        self.sts_method_name = method_name
        self.on_off_type = type_name
        return True

    def resolve_normal(self) -> bool:
        if overload_attr := self.f.get_last_attr("overload"):
            if not check_attr_args(self.am, overload_attr, "s"):
                return True
            (method_name,) = overload_attr.args
        else:
            if self.am.compiler_invocation.sts_keep_name:
                method_name = self.f.name
            else:
                method_name = self.f.name[0].lower() + self.f.name[1:]
        self.ani_method_name = method_name
        self.sts_method_name = method_name
        return True

    def resolve_async(self) -> bool:
        if (async_attr := self.f.get_last_attr("gen_async")) is None:
            return False
        if self.sts_method_name is None:
            return True
        if async_attr.args and check_attr_args(self.am, async_attr, "s"):
            (self.sts_async_name,) = async_attr.args
        elif self.sts_method_name[-4:].lower() == "sync":
            self.sts_async_name = self.sts_method_name[:-4]
        else:
            raise_adhoc_error(
                self.am,
                '@gen_async method name must end with "Sync" or have @gen_async argument',
                self.f.loc,
            )
        return True

    def resolve_promise(self) -> bool:
        if (promise_attr := self.f.get_last_attr("gen_promise")) is None:
            return False
        if self.sts_method_name is None:
            return True
        if promise_attr.args and check_attr_args(self.am, promise_attr, "s"):
            (self.sts_promise_name,) = promise_attr.args
        elif self.sts_method_name[-4:].lower() == "sync":
            self.sts_promise_name = self.sts_method_name[:-4]
        else:
            raise_adhoc_error(
                self.am,
                '@gen_promise method name must end with "Sync" or have @gen_promise argument',
                self.f.loc,
            )
        return True

    def call_native_with(self, sts_args: list[str], this: str = "this") -> str:
        arg = iter(sts_args)
        sts_native_args: list[str] = []
        for param in self.f.params:
            if param.get_last_attr("sts_this"):
                sts_native_args.append(this)
                continue
            sts_native_args.append(next(arg))
        sts_native_args_str = ", ".join(sts_native_args)
        return f"{this}.{self.sts_native_name}({sts_native_args_str})"


class EnumANIInfo(AbstractAnalysis[EnumDecl]):
    def __init__(self, am: AnalysisManager, d: EnumDecl) -> None:
        super().__init__(am, d)

        self.parent_ns = PackageANIInfo.get(am, d.parent_pkg).ns
        self.sts_type_name = d.name
        self.type_desc = (
            "L" + "/".join([*self.parent_ns.ani_path, self.sts_type_name]) + ";"
        )

        self.const = d.get_last_attr("const") is not None

        if (
            not self.const
            and isinstance(d.ty_ref.resolved_ty, ScalarType)
            and d.ty_ref.resolved_ty.kind
            in (
                ScalarKind.BOOL,
                ScalarKind.F32,
                ScalarKind.F64,
            )
        ):
            raise_adhoc_error(
                am,
                f"{d.description} without @const cannot have type {d.ty_ref.resolved_ty.signature}",
                d.loc,
            )

        self.is_default = d.get_last_attr("sts_export_default") is not None

    def sts_type_in(self, target: StsWriter):
        return self.parent_ns.get_member(
            target,
            self.sts_type_name,
            self.is_default,
        )


class UnionFieldANIInfo(AbstractAnalysis[UnionFieldDecl]):
    field_ty: Type | None | Literal["null", "undefined"]

    def __init__(self, am: AnalysisManager, d: UnionFieldDecl) -> None:
        super().__init__(am, d)
        if d.ty_ref is None:
            if d.get_last_attr("null"):
                self.field_ty = "null"
                return
            if d.get_last_attr("undefined"):
                self.field_ty = "undefined"
                return
            raise_adhoc_error(
                am,
                f"union field {d.name} must have a type or have @null/@undefined attribute",
                d.loc,
            )
            self.field_ty = None
        else:
            self.field_ty = d.ty_ref.resolved_ty


class UnionANIInfo(AbstractAnalysis[UnionDecl]):
    def __init__(self, am: AnalysisManager, d: UnionDecl) -> None:
        super().__init__(am, d)
        self.decl_header = f"{d.parent_pkg.name}.{d.name}.ani.1.h"
        self.impl_header = f"{d.parent_pkg.name}.{d.name}.ani.2.h"

        self.parent_ns = PackageANIInfo.get(am, d.parent_pkg).ns
        self.sts_type_name = d.name
        self.type_desc = "Lstd/core/Object;"

        self.sts_final_fields: list[list[UnionFieldDecl]] = []
        for field in d.fields:
            if field.ty_ref and isinstance(ty := field.ty_ref.resolved_ty, UnionType):
                inner_ani_info = UnionANIInfo.get(am, ty.ty_decl)
                self.sts_final_fields.extend(
                    [field, *parts] for parts in inner_ani_info.sts_final_fields
                )
            else:
                self.sts_final_fields.append([field])

        self.is_default = d.get_last_attr("sts_export_default") is not None

    def sts_type_in(self, target: StsWriter):
        return self.parent_ns.get_member(
            target,
            self.sts_type_name,
            self.is_default,
        )


class StructFieldANIInfo(AbstractAnalysis[StructFieldDecl]):
    def __init__(self, am: AnalysisManager, d: StructFieldDecl) -> None:
        super().__init__(am, d)
        self.readonly = d.get_last_attr("readonly") is not None


class StructANIInfo(AbstractAnalysis[StructDecl]):
    def __init__(self, am: AnalysisManager, d: StructDecl) -> None:
        super().__init__(am, d)
        self.decl_header = f"{d.parent_pkg.name}.{d.name}.ani.1.h"
        self.impl_header = f"{d.parent_pkg.name}.{d.name}.ani.2.h"

        self.parent_ns = PackageANIInfo.get(am, d.parent_pkg).ns
        self.sts_type_name = d.name
        if d.get_last_attr("class"):
            self.sts_impl_name = f"{d.name}"
        else:
            self.sts_impl_name = f"{d.name}_inner"
        self.type_desc = (
            "L" + "/".join([*self.parent_ns.ani_path, self.sts_type_name]) + ";"
        )
        self.impl_desc = (
            "L" + "/".join([*self.parent_ns.ani_path, self.sts_impl_name]) + ";"
        )

        self.interface_injected_codes: list[str] = []
        for iface_injected in d.get_all_attrs("sts_inject_into_interface"):
            if check_attr_args(am, iface_injected, "s"):
                (code,) = iface_injected.args
                self.interface_injected_codes.append(code)
        self.class_injected_codes: list[str] = []
        for class_injected in d.get_all_attrs("sts_inject_into_class"):
            if check_attr_args(am, class_injected, "s"):
                (code,) = class_injected.args
                self.class_injected_codes.append(code)

        self.sts_fields: list[StructFieldDecl] = []
        self.sts_iface_parents: list[StructFieldDecl] = []
        self.sts_class_parents: list[StructFieldDecl] = []
        self.sts_final_fields: list[list[StructFieldDecl]] = []
        for field in d.fields:
            if field.get_last_attr("extends"):
                ty = field.ty_ref.resolved_ty
                if not isinstance(ty, StructType):
                    raise_adhoc_error(
                        am,
                        "struct cannot extend non-struct type",
                        field.loc,
                    )
                    continue
                parent_ani_info = StructANIInfo.get(am, ty.ty_decl)
                if parent_ani_info.is_class():
                    self.sts_class_parents.append(field)
                else:
                    self.sts_iface_parents.append(field)
                self.sts_final_fields.extend(
                    [field, *parts] for parts in parent_ani_info.sts_final_fields
                )
            else:
                self.sts_fields.append(field)
                self.sts_final_fields.append([field])

        self.is_default = d.get_last_attr("sts_export_default") is not None

    def is_class(self):
        return self.sts_type_name == self.sts_impl_name

    def sts_type_in(self, target: StsWriter):
        return self.parent_ns.get_member(
            target,
            self.sts_type_name,
            self.is_default,
        )


class IfaceANIInfo(AbstractAnalysis[IfaceDecl]):
    def __init__(self, am: AnalysisManager, d: IfaceDecl) -> None:
        super().__init__(am, d)
        self.decl_header = f"{d.parent_pkg.name}.{d.name}.ani.1.h"
        self.impl_header = f"{d.parent_pkg.name}.{d.name}.ani.2.h"

        self.parent_ns = PackageANIInfo.get(am, d.parent_pkg).ns
        self.sts_type_name = d.name
        if d.get_last_attr("class"):
            self.sts_impl_name = f"{d.name}"
        else:
            self.sts_impl_name = f"{d.name}_inner"
        self.type_desc = (
            "L" + "/".join([*self.parent_ns.ani_path, self.sts_type_name]) + ";"
        )
        self.impl_desc = (
            "L" + "/".join([*self.parent_ns.ani_path, self.sts_impl_name]) + ";"
        )

        self.interface_injected_codes: list[str] = []
        for iface_injected in d.get_all_attrs("sts_inject_into_interface"):
            if check_attr_args(am, iface_injected, "s"):
                (code,) = iface_injected.args
                self.interface_injected_codes.append(code)
        self.class_injected_codes: list[str] = []
        for class_injected in d.get_all_attrs("sts_inject_into_class"):
            if check_attr_args(am, class_injected, "s"):
                (code,) = class_injected.args
                self.class_injected_codes.append(code)

        self.sts_class_parents: list[IfaceParentDecl] = []
        self.sts_iface_parents: list[IfaceParentDecl] = []
        for parent in d.parents:
            ty = parent.ty_ref.resolved_ty
            pass
            parent_ani_info = IfaceANIInfo.get(am, ty.ty_decl)
            if parent_ani_info.is_class():
                self.sts_class_parents.append(parent)
            else:
                self.sts_iface_parents.append(parent)

        self.is_default = d.get_last_attr("sts_export_default") is not None

    def is_class(self):
        return self.sts_type_name == self.sts_impl_name

    def sts_type_in(self, target: StsWriter):
        return self.parent_ns.get_member(
            target,
            self.sts_type_name,
            self.is_default,
        )


class AbstractTypeANIInfo(metaclass=ABCMeta):
    ani_type: ANIType
    type_desc: str

    def __init__(self, am: AnalysisManager, t: Type):
        self.cpp_info = TypeCppInfo.get(am, t)

    @property
    def type_desc_boxed(self) -> str:
        if self.ani_type.base == ANI_REF:
            return self.type_desc
        return f"Lstd/core/{self.ani_type.suffix};"

    @abstractmethod
    def sts_type_in(self, target: StsWriter) -> str:
        pass

    @abstractmethod
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        pass

    @abstractmethod
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        pass

    def into_ani_boxed(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        if self.ani_type.base == ANI_REF:
            self.into_ani(target, env, cpp_value, ani_result)
        else:
            ani_value = f"{ani_result}_ani"
            target.writelns(
                f"ani_object {ani_result};",
            )
            self.into_ani(target, env, cpp_value, ani_value)
            target.writelns(
                f'{env}->Object_New(TH_ANI_FIND_CLASS({env}, "{self.type_desc_boxed}"), TH_ANI_FIND_CLASS_METHOD({env}, "{self.type_desc_boxed}", "<ctor>", "{self.type_desc}:V"), &{ani_result}, {ani_value});',
            )

    def from_ani_boxed(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        if self.ani_type.base == ANI_REF:
            self.from_ani(
                target,
                env,
                f"static_cast<{self.ani_type}>({ani_value})",
                cpp_result,
            )
        else:
            ani_result = f"{cpp_result}_ani"
            target.writelns(
                f"{self.ani_type} {ani_result};",
                f'{env}->Object_CallMethod_{self.ani_type.suffix}((ani_object){ani_value}, TH_ANI_FIND_CLASS_METHOD({env}, "{self.type_desc_boxed}", "unboxed", ":{self.type_desc}"), &{ani_result});',
            )
            self.from_ani(target, env, ani_result, cpp_result)

    def from_ani_fixedarray(
        self,
        target: CSourceWriter,
        env: str,
        ani_size: str,
        ani_fixedarray_value: str,
        cpp_fixedarray_buffer: str,
    ):
        if self.ani_type.base == ANI_REF:
            ani_value = f"{cpp_fixedarray_buffer}_ani_item"
            cpp_result = f"{cpp_fixedarray_buffer}_cpp_item"
            cpp_i = f"{cpp_fixedarray_buffer}_i"
            with target.indented(
                f"for (size_t {cpp_i} = 0; {cpp_i} < {ani_size}; {cpp_i}++) {{",
                f"}}",
            ):
                target.writelns(
                    f"{self.ani_type} {ani_value};",
                    f"{env}->FixedArray_Get_Ref({ani_fixedarray_value}, {cpp_i}, reinterpret_cast<ani_ref*>(&{ani_value}));",
                )
                self.from_ani(target, env, ani_value, cpp_result)
                target.writelns(
                    f"new (&{cpp_fixedarray_buffer}[{cpp_i}]) {self.cpp_info.as_owner}(std::move({cpp_result}));",
                )
        else:
            target.writelns(
                f"{env}->FixedArray_GetRegion_{self.ani_type.suffix}({ani_fixedarray_value}, 0, {ani_size}, reinterpret_cast<{self.ani_type}*>({cpp_fixedarray_buffer}));",
            )

    def into_ani_fixedarray(
        self,
        target: CSourceWriter,
        env: str,
        cpp_size: str,
        cpp_fixedarray_value: str,
        ani_fixedarray_result: str,
    ):
        if self.ani_type.base == ANI_REF:
            ani_class = f"{ani_fixedarray_result}_cls"
            ani_result = f"{ani_fixedarray_result}_item"
            ani_undefined = f"{ani_fixedarray_result}_undef"
            cpp_i = f"{ani_fixedarray_result}_i"
            target.writelns(
                f"ani_fixedarray_ref {ani_fixedarray_result};",
                f"ani_class {ani_class};",
                f'{env}->FindClass("{self.type_desc}", &{ani_class});',
                f"ani_ref {ani_undefined};",
                f"{env}->GetUndefined(&{ani_undefined});",
                f"{env}->FixedArray_New_Ref({ani_class}, {cpp_size}, {ani_undefined}, &{ani_fixedarray_result});",
            )
            with target.indented(
                f"for (size_t {cpp_i} = 0; {cpp_i} < {cpp_size}; {cpp_i}++) {{",
                f"}}",
            ):
                self.into_ani(
                    target, env, f"{cpp_fixedarray_value}[{cpp_i}]", ani_result
                )
                target.writelns(
                    f"{env}->FixedArray_Set_Ref({ani_fixedarray_result}, {cpp_i}, {ani_result});",
                )
        else:
            target.writelns(
                f"{self.ani_type.fixedarray} {ani_fixedarray_result};",
                f"{env}->FixedArray_New_{self.ani_type.suffix}({cpp_size}, &{ani_fixedarray_result});",
                f"{env}->FixedArray_SetRegion_{self.ani_type.suffix}({ani_fixedarray_result}, 0, {cpp_size}, reinterpret_cast<{self.ani_type} const*>({cpp_fixedarray_value}));",
            )


class EnumTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[EnumType]):
    def __init__(self, am: AnalysisManager, t: EnumType):
        super().__init__(am, t)
        self.am = am
        self.t = t
        enum_ani_info = EnumANIInfo.get(self.am, self.t.ty_decl)
        self.ani_type = ANI_ENUM_ITEM
        self.type_desc = enum_ani_info.type_desc
        if enum_ani_info.const:
            raise_adhoc_error(
                am,
                f"@const {t.ty_decl.description} cannot be used as type",
                t.ty_ref.loc,
            )

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        enum_ani_info = EnumANIInfo.get(self.am, self.t.ty_decl)
        return enum_ani_info.sts_type_in(target)

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        ani_index = f"{cpp_result}_idx"
        enum_cpp_info = EnumCppInfo.get(self.am, self.t.ty_decl)
        target.writelns(
            f"ani_size {ani_index};",
            f"{env}->EnumItem_GetIndex({ani_value}, &{ani_index});",
            f"{enum_cpp_info.full_name} {cpp_result}(({enum_cpp_info.full_name}::key_t){ani_index});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        ani_class = f"{ani_result}_cls"
        target.writelns(
            f"ani_enum {ani_class};",
            f'{env}->FindEnum("{self.type_desc}", &{ani_class});',
            f"ani_enum_item {ani_result};",
            f"{env}->Enum_GetEnumItemByIndex({ani_class}, (ani_size){cpp_value}.get_key(), &{ani_result});",
        )


class StructTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[StructType]):
    def __init__(self, am: AnalysisManager, t: StructType):
        super().__init__(am, t)
        self.am = am
        self.t = t
        struct_ani_info = StructANIInfo.get(self.am, self.t.ty_decl)
        self.ani_type = ANI_OBJECT
        self.type_desc = struct_ani_info.type_desc

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        struct_ani_info = StructANIInfo.get(self.am, self.t.ty_decl)
        return struct_ani_info.sts_type_in(target)

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        struct_ani_info = StructANIInfo.get(self.am, self.t.ty_decl)
        struct_cpp_info = StructCppInfo.get(self.am, self.t.ty_decl)
        target.add_include(struct_ani_info.impl_header)
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_result} = ::taihe::from_ani<{struct_cpp_info.as_owner}>({env}, {ani_value});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        struct_ani_info = StructANIInfo.get(self.am, self.t.ty_decl)
        struct_cpp_info = StructCppInfo.get(self.am, self.t.ty_decl)
        target.add_include(struct_ani_info.impl_header)
        target.writelns(
            f"ani_object {ani_result} = ::taihe::into_ani<{struct_cpp_info.as_owner}>({env}, {cpp_value});",
        )


class UnionTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[UnionType]):
    def __init__(self, am: AnalysisManager, t: UnionType):
        super().__init__(am, t)
        self.am = am
        self.t = t
        union_ani_info = UnionANIInfo.get(self.am, self.t.ty_decl)
        self.ani_type = ANI_REF
        self.type_desc = union_ani_info.type_desc

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        union_ani_info = UnionANIInfo.get(self.am, self.t.ty_decl)
        return union_ani_info.sts_type_in(target)

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        union_ani_info = UnionANIInfo.get(self.am, self.t.ty_decl)
        union_cpp_info = UnionCppInfo.get(self.am, self.t.ty_decl)
        target.add_include(union_ani_info.impl_header)
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_result} = ::taihe::from_ani<{union_cpp_info.as_owner}>({env}, {ani_value});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        union_ani_info = UnionANIInfo.get(self.am, self.t.ty_decl)
        union_cpp_info = UnionCppInfo.get(self.am, self.t.ty_decl)
        target.add_include(union_ani_info.impl_header)
        target.writelns(
            f"ani_ref {ani_result} = ::taihe::into_ani<{union_cpp_info.as_owner}>({env}, {cpp_value});",
        )


class IfaceTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[IfaceType]):
    def __init__(self, am: AnalysisManager, t: IfaceType):
        super().__init__(am, t)
        self.am = am
        self.t = t
        iface_ani_info = IfaceANIInfo.get(self.am, self.t.ty_decl)
        self.ani_type = ANI_OBJECT
        self.type_desc = iface_ani_info.type_desc

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        iface_ani_info = IfaceANIInfo.get(self.am, self.t.ty_decl)
        return iface_ani_info.sts_type_in(target)

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        iface_ani_info = IfaceANIInfo.get(self.am, self.t.ty_decl)
        iface_cpp_info = IfaceCppInfo.get(self.am, self.t.ty_decl)
        target.add_include(iface_ani_info.impl_header)
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_result} = ::taihe::from_ani<{iface_cpp_info.as_owner}>({env}, {ani_value});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        iface_ani_info = IfaceANIInfo.get(self.am, self.t.ty_decl)
        iface_cpp_info = IfaceCppInfo.get(self.am, self.t.ty_decl)
        target.add_include(iface_ani_info.impl_header)
        target.writelns(
            f"ani_object {ani_result} = ::taihe::into_ani<{iface_cpp_info.as_owner}>({env}, {cpp_value});",
        )


class ScalarTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[ScalarType]):
    def __init__(self, am: AnalysisManager, t: ScalarType):
        super().__init__(am, t)
        sts_info = {
            ScalarKind.BOOL: ("boolean", ANI_BOOLEAN, "Z"),
            ScalarKind.F32: ("float", ANI_FLOAT, "F"),
            ScalarKind.F64: ("double", ANI_DOUBLE, "D"),
            ScalarKind.I8: ("byte", ANI_BYTE, "B"),
            ScalarKind.I16: ("short", ANI_SHORT, "S"),
            ScalarKind.I32: ("int", ANI_INT, "I"),
            ScalarKind.I64: ("long", ANI_LONG, "J"),
            ScalarKind.U8: ("byte", ANI_BYTE, "B"),
            ScalarKind.U16: ("short", ANI_SHORT, "S"),
            ScalarKind.U32: ("int", ANI_INT, "I"),
            ScalarKind.U64: ("long", ANI_LONG, "J"),
        }.get(t.kind)
        if sts_info is None:
            raise ValueError
        sts_type, ani_type, type_desc = sts_info
        self.sts_type = sts_type
        self.ani_type = ani_type
        self.type_desc = type_desc

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        return self.sts_type

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_result} = ({self.cpp_info.as_owner}){ani_value};",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        target.writelns(
            f"{self.ani_type} {ani_result} = ({self.cpp_info.as_owner}){cpp_value};",
        )


class OpaqueTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[OpaqueType]):
    def __init__(self, am: AnalysisManager, t: OpaqueType) -> None:
        super().__init__(am, t)
        self.am = am
        self.t = t
        self.ani_type = ANI_OBJECT
        if (
            sts_type_attr := self.t.ty_ref.get_last_attr("sts_type")
        ) and check_attr_args(self.am, sts_type_attr, "s"):
            (sts_type,) = sts_type_attr.args
            self.sts_type = sts_type
        else:
            self.sts_type = "Object"
        self.type_desc = "Lstd/core/Object;"

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        return self.sts_type

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_result} = ({self.cpp_info.as_owner}){ani_value};",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        target.writelns(
            f"{self.ani_type} {ani_result} = ({self.ani_type}){cpp_value};",
        )


class StringTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[StringType]):
    def __init__(self, am: AnalysisManager, t: StringType):
        super().__init__(am, t)
        self.ani_type = ANI_STRING
        self.type_desc = "Lstd/core/String;"

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        return "string"

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        ani_length = f"{cpp_result}_len"
        cpp_tstr = f"{cpp_result}_tstr"
        cpp_buffer = f"{cpp_result}_buf"
        target.writelns(
            f"ani_size {ani_length};",
            f"{env}->String_GetUTF8Size({ani_value}, &{ani_length});",
            f"TString {cpp_tstr};",
            f"char* {cpp_buffer} = tstr_initialize(&{cpp_tstr}, {ani_length} + 1);",
            f"{env}->String_GetUTF8({ani_value}, {cpp_buffer}, {ani_length} + 1, &{ani_length});",
            f"{cpp_buffer}[{ani_length}] = '\\0';",
            f"{cpp_tstr}.length = {ani_length};",
            f"::taihe::string {cpp_result} = ::taihe::string({cpp_tstr});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        target.writelns(
            f"ani_string {ani_result};",
            f"{env}->String_NewUTF8({cpp_value}.c_str(), {cpp_value}.size(), &{ani_result});",
        )


class OptionalTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[OptionalType]):
    def __init__(self, am: AnalysisManager, t: OptionalType) -> None:
        super().__init__(am, t)
        self.am = am
        self.t = t
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        self.ani_type = ANI_REF
        self.type_desc = item_ty_ani_info.type_desc_boxed

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        sts_type = item_ty_ani_info.sts_type_in(target)
        return f"({sts_type} | undefined)"

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        ani_is_undefined = f"{cpp_result}_flag"
        cpp_pointer = f"{cpp_result}_ptr"
        cpp_spec = f"{cpp_result}_spec"
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        target.writelns(
            f"ani_boolean {ani_is_undefined};",
            f"{item_ty_cpp_info.as_owner}* {cpp_pointer} = nullptr;",
            f"{env}->Reference_IsUndefined({ani_value}, &{ani_is_undefined});",
        )
        with target.indented(
            f"if (!{ani_is_undefined}) {{",
            f"}};",
        ):
            item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
            item_ty_ani_info.from_ani_boxed(target, env, ani_value, cpp_spec)
            target.writelns(
                f"{cpp_pointer} = new {item_ty_cpp_info.as_owner}(std::move({cpp_spec}));",
            )
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_result}({cpp_pointer});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        ani_spec = f"{ani_result}_spec"
        target.writelns(
            f"ani_ref {ani_result};",
        )
        with target.indented(
            f"if (!{cpp_value}) {{",
            f"}}",
        ):
            target.writelns(
                f"{env}->GetUndefined(&{ani_result});",
            )
        with target.indented(
            f"else {{",
            f"}}",
        ):
            item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
            item_ty_ani_info.into_ani_boxed(target, env, f"(*{cpp_value})", ani_spec)
            target.writelns(
                f"{ani_result} = {ani_spec};",
            )


class FixedArrayTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[ArrayType]):
    def __init__(self, am: AnalysisManager, t: ArrayType) -> None:
        super().__init__(am, t)
        self.am = am
        self.t = t
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        self.ani_type = item_ty_ani_info.ani_type.fixedarray
        self.type_desc = f"Lescompat/FixedArray;"

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        sts_type = item_ty_ani_info.sts_type_in(target)
        return f"FixedArray<{sts_type}>"

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        ani_size = f"{cpp_result}_size"
        cpp_buffer = f"{cpp_result}_buffer"
        target.writelns(
            f"size_t {ani_size};",
            f"{env}->FixedArray_GetLength({ani_value}, &{ani_size});",
            f"{item_ty_cpp_info.as_owner}* {cpp_buffer} = reinterpret_cast<{item_ty_cpp_info.as_owner}*>(malloc({ani_size} * sizeof({item_ty_cpp_info.as_owner})));",
        )
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        item_ty_ani_info.from_ani_fixedarray(
            target, env, ani_size, ani_value, cpp_buffer
        )
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_result}({cpp_buffer}, {ani_size});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        cpp_size = f"{ani_result}_size"
        target.writelns(
            f"size_t {cpp_size} = {cpp_value}.size();",
        )
        item_ty_ani_info.into_ani_fixedarray(
            target, env, cpp_size, f"{cpp_value}.data()", ani_result
        )


class ArrayTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[ArrayType]):
    def __init__(self, am: AnalysisManager, t: ArrayType) -> None:
        super().__init__(am, t)
        self.am = am
        self.t = t
        self.ani_type = ANI_ARRAY
        self.type_desc = f"Lescompat/Array;"

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        sts_type = item_ty_ani_info.sts_type_in(target)
        return f"Array<{sts_type}>"

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        ani_size = f"{cpp_result}_size"
        cpp_buffer = f"{cpp_result}_buffer"
        ani_item = f"{cpp_buffer}_ani_item"
        cpp_item = f"{cpp_buffer}_cpp_item"
        cpp_ctr = f"{cpp_buffer}_i"
        target.writelns(
            f"size_t {ani_size};",
            f"{env}->Array_GetLength({ani_value}, &{ani_size});",
            f"{item_ty_cpp_info.as_owner}* {cpp_buffer} = reinterpret_cast<{item_ty_cpp_info.as_owner}*>(malloc({ani_size} * sizeof({item_ty_cpp_info.as_owner})));",
        )
        with target.indented(
            f"for (size_t {cpp_ctr} = 0; {cpp_ctr} < {ani_size}; {cpp_ctr}++) {{",
            f"}}",
        ):
            target.writelns(
                f"ani_object {ani_item};",
                f"{env}->Array_Get_Ref({ani_value}, {cpp_ctr}, reinterpret_cast<ani_ref*>(&{ani_item}));",  # TODO: Array
            )
            item_ty_ani_info.from_ani_boxed(target, env, ani_item, cpp_item)
            target.writelns(
                f"new (&{cpp_buffer}[{cpp_ctr}]) {item_ty_ani_info.cpp_info.as_owner}(std::move({cpp_item}));",
            )
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_result}({cpp_buffer}, {ani_size});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        cpp_size = f"{ani_result}_size"
        ani_item = f"{ani_result}_item"
        ani_undefined = f"{ani_result}_undef"
        cpp_ctr = f"{ani_result}_i"
        target.writelns(
            f"size_t {cpp_size} = {cpp_value}.size();",
            f"ani_array_ref {ani_result};",  # TODO: Array
            f"ani_ref {ani_undefined};",
            f"{env}->GetUndefined(&{ani_undefined});",
            f'{env}->Array_New_Ref(TH_ANI_FIND_CLASS({env}, "Lstd/core/Object;"), {cpp_size}, {ani_undefined}, &{ani_result});',  # TODO: Array
        )
        with target.indented(
            f"for (size_t {cpp_ctr} = 0; {cpp_ctr} < {cpp_size}; {cpp_ctr}++) {{",
            f"}}",
        ):
            item_ty_ani_info.into_ani_boxed(
                target, env, f"{cpp_value}[{cpp_ctr}]", ani_item
            )
            target.writelns(
                f"{env}->Array_Set_Ref({ani_result}, {cpp_ctr}, {ani_item});",  # TODO: Array
            )


class ArrayBufferTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[ArrayType]):
    def __init__(self, am: AnalysisManager, t: ArrayType) -> None:
        super().__init__(am, t)
        self.am = am
        self.t = t
        if not isinstance(t.item_ty, ScalarType) or t.item_ty.kind not in (
            ScalarKind.I8,
            ScalarKind.U8,
        ):
            raise_adhoc_error(
                am,
                "@arraybuffer only supports Array<i8> or Array<i8>",
                t.ty_ref.loc,
            )
        self.ani_type = ANI_ARRAYBUFFER
        self.type_desc = "Lescompat/ArrayBuffer;"

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        return "ArrayBuffer"

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        ani_data = f"{cpp_result}_data"
        ani_length = f"{cpp_result}_length"
        target.writelns(
            f"char* {ani_data} = nullptr;",
            f"size_t {ani_length} = 0;",
            f"{env}->ArrayBuffer_GetInfo({ani_value}, reinterpret_cast<void**>(&{ani_data}), &{ani_length});",
            f"{self.cpp_info.as_param} {cpp_result}(reinterpret_cast<{item_ty_cpp_info.as_owner}*>({ani_data}), {ani_length});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        ani_data = f"{ani_result}_data"
        target.writelns(
            f"char* {ani_data} = nullptr;",
            f"ani_arraybuffer {ani_result};",
            f"{env}->CreateArrayBuffer({cpp_value}.size(), reinterpret_cast<void**>(&{ani_data}), &{ani_result});",
            f"memcpy({ani_data}, {cpp_value}.data(), {cpp_value}.size());",
        )


class TypedArrayTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[ArrayType]):
    def __init__(self, am: AnalysisManager, t: ArrayType):
        super().__init__(am, t)
        self.am = am
        self.t = t
        if (
            not isinstance(t.item_ty, ScalarType)
            or (
                sts_type := {
                    ScalarKind.F32: "Float32Array",
                    ScalarKind.F64: "Float64Array",
                    ScalarKind.I8: "Int8Array",
                    ScalarKind.I16: "Int16Array",
                    ScalarKind.I32: "Int32Array",
                    ScalarKind.I64: "BigInt64Array",
                    ScalarKind.U8: "Uint8Array",
                    ScalarKind.U16: "Uint16Array",
                    ScalarKind.U32: "Uint32Array",
                    ScalarKind.U64: "BigUint64Array",
                }.get(t.item_ty.kind)
            )
            is None
        ):
            raise_adhoc_error(
                am,
                f"@typedarray does not supports Array<{t.item_ty.ty_ref.text}>",
                t.ty_ref.loc,
            )
            self.sts_type = "TypedArray"
        else:
            self.sts_type = sts_type
        self.ani_type = ANI_OBJECT
        self.type_desc = f"Lescompat/{self.sts_type};"

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        return self.sts_type

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        ani_byte_length = f"{cpp_result}_bylen"
        ani_byte_offset = f"{cpp_result}_byoff"
        ani_arrbuf = f"{cpp_result}_arrbuf"
        ani_data = f"{cpp_result}_data"
        ani_length = f"{cpp_result}_length"
        target.writelns(
            f"ani_double {ani_byte_length};",
            f"ani_double {ani_byte_offset};",
            f"ani_arraybuffer {ani_arrbuf};",
            f'{env}->Object_GetPropertyByName_Double({ani_value}, "byteLength", &{ani_byte_length});',
            f'{env}->Object_GetPropertyByName_Double({ani_value}, "byteOffset", &{ani_byte_offset});',
            f'{env}->Object_GetPropertyByName_Ref({ani_value}, "buffer", reinterpret_cast<ani_ref*>(&{ani_arrbuf}));',
            f"char* {ani_data} = nullptr;",
            f"size_t {ani_length} = 0;",
            f"{env}->ArrayBuffer_GetInfo({ani_arrbuf}, reinterpret_cast<void**>(&{ani_data}), &{ani_length});",
            f"{self.cpp_info.as_param} {cpp_result}(reinterpret_cast<{item_ty_cpp_info.as_owner}*>({ani_data} + (size_t){ani_byte_offset}), (size_t){ani_byte_length} / (sizeof({item_ty_cpp_info.as_owner}) / sizeof(char)));",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        ani_data = f"{ani_result}_data"
        ani_arrbuf = f"{ani_result}_arrbuf"
        ani_byte_length = f"{ani_result}_bylen"
        ani_byte_offset = f"{ani_result}_byoff"
        target.writelns(
            f"char* {ani_data} = nullptr;",
            f"ani_arraybuffer {ani_arrbuf};",
            f"{env}->CreateArrayBuffer({cpp_value}.size() * (sizeof({item_ty_cpp_info.as_owner}) / sizeof(char)), reinterpret_cast<void**>(&{ani_data}), &{ani_arrbuf});",
            f"memcpy({ani_data}, {cpp_value}.data(), {cpp_value}.size() * (sizeof({item_ty_cpp_info.as_owner}) / sizeof(char)));",
            f"ani_ref {ani_byte_length};",
            f"{env}->GetUndefined(&{ani_byte_length});",
            f"ani_ref {ani_byte_offset};",
            f"{env}->GetUndefined(&{ani_byte_offset});",
            f"ani_object {ani_result};",
            f'{env}->Object_New(TH_ANI_FIND_CLASS({env}, "{self.type_desc}"), TH_ANI_FIND_CLASS_METHOD({env}, "{self.type_desc}", "<ctor>", "Lescompat/ArrayBuffer;Lstd/core/Double;Lstd/core/Double;:V"), &{ani_result}, {ani_arrbuf}, {ani_byte_length}, {ani_byte_offset});',
        )


class BigIntTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[ArrayType]):
    def __init__(self, am: AnalysisManager, t: ArrayType) -> None:
        super().__init__(am, t)
        self.am = am
        self.t = t
        if not isinstance(t.item_ty, ScalarType) or t.item_ty.kind not in (
            ScalarKind.I8,
            ScalarKind.I16,
            ScalarKind.I32,
            ScalarKind.I64,
            ScalarKind.U8,
            ScalarKind.U16,
            ScalarKind.U32,
            ScalarKind.U64,
        ):
            raise_adhoc_error(
                am,
                f"@bigint does not supports Array<{t.item_ty.ty_ref.text}>",
                t.ty_ref.loc,
            )
        self.ani_type = ANI_OBJECT
        self.type_desc = "Lescompat/BigInt;"

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        return "BigInt"

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        pkg_ani_info = PackageANIInfo.get(self.am, self.t.ty_ref.parent_pkg)
        ani_arrbuf = f"{cpp_result}_arrbuf"
        ani_data = f"{cpp_result}_data"
        ani_length = f"{cpp_result}_length"
        target.writelns(
            f"ani_arraybuffer {ani_arrbuf};",
            f'{env}->Function_Call_Ref(TH_ANI_FIND_MODULE_FUNCTION({env}, "{pkg_ani_info.ns.module.impl_desc}", "__fromBigIntToArrayBuffer", nullptr), reinterpret_cast<ani_ref*>(&{ani_arrbuf}), {ani_value}, sizeof({item_ty_cpp_info.as_owner}) / sizeof(char));'
            f"char* {ani_data} = nullptr;",
            f"size_t {ani_length} = 0;",
            f"{env}->ArrayBuffer_GetInfo({ani_arrbuf}, reinterpret_cast<void**>(&{ani_data}), &{ani_length});",
            f"{self.cpp_info.as_param} {cpp_result}(reinterpret_cast<{item_ty_cpp_info.as_owner}*>({ani_data}), {ani_length} / (sizeof({item_ty_cpp_info.as_owner}) / sizeof(char)));",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        pkg_ani_info = PackageANIInfo.get(self.am, self.t.ty_ref.parent_pkg)
        ani_data = f"{ani_result}_data"
        ani_arrbuf = f"{ani_result}_arrbuf"
        target.writelns(
            f"char* {ani_data} = nullptr;",
            f"ani_arraybuffer {ani_arrbuf};",
            f"{env}->CreateArrayBuffer({cpp_value}.size() * (sizeof({item_ty_cpp_info.as_owner}) / sizeof(char)), reinterpret_cast<void**>(&{ani_data}), &{ani_arrbuf});",
            f"memcpy({ani_data}, {cpp_value}.data(), {cpp_value}.size() * (sizeof({item_ty_cpp_info.as_owner}) / sizeof(char)));",
            f"ani_object {ani_result};",
            f'{env}->Function_Call_Ref(TH_ANI_FIND_MODULE_FUNCTION({env}, "{pkg_ani_info.ns.module.impl_desc}", "__fromArrayBufferToBigInt", nullptr), reinterpret_cast<ani_ref*>(&{ani_result}), {ani_arrbuf});',
        )


class RecordTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[MapType]):
    def __init__(self, am: AnalysisManager, t: MapType) -> None:
        super().__init__(am, t)
        self.am = am
        self.t = t
        self.ani_type = ANI_OBJECT
        self.type_desc = "Lescompat/Record;"

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        key_ty_ani_info = TypeANIInfo.get(self.am, self.t.key_ty)
        val_ty_ani_info = TypeANIInfo.get(self.am, self.t.val_ty)
        key_sts_type = key_ty_ani_info.sts_type_in(target)
        val_sts_type = val_ty_ani_info.sts_type_in(target)
        return f"Record<{key_sts_type}, {val_sts_type}>"

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        ani_iter = f"{cpp_result}_iter"
        ani_item = f"{cpp_result}_item"
        ani_next = f"{cpp_result}_next"
        ani_done = f"{cpp_result}_done"
        ani_key = f"{cpp_result}_ani_key"
        ani_val = f"{cpp_result}_ani_val"
        cpp_key = f"{cpp_result}_cpp_key"
        cpp_val = f"{cpp_result}_cpp_val"
        key_ty_ani_info = TypeANIInfo.get(self.am, self.t.key_ty)
        val_ty_ani_info = TypeANIInfo.get(self.am, self.t.val_ty)
        target.writelns(
            f"ani_ref {ani_iter};",
            f'{env}->Object_CallMethod_Ref({ani_value}, TH_ANI_FIND_CLASS_METHOD({env}, "Lescompat/Record;", "$_iterator", nullptr), &{ani_iter});',
            f"{self.cpp_info.as_owner} {cpp_result};",
        )
        with target.indented(
            f"while (true) {{",
            f"}}",
        ):
            target.writelns(
                f"ani_ref {ani_next};",
                f"ani_boolean {ani_done};",
                f'{env}->Object_CallMethod_Ref(static_cast<ani_object>({ani_iter}), TH_ANI_FIND_CLASS_METHOD({env}, "Lescompat/MapIterator;", "next", nullptr), &{ani_next});',
                f'{env}->Object_GetField_Boolean(static_cast<ani_object>({ani_next}), TH_ANI_FIND_CLASS_FIELD({env}, "Lescompat/IteratorResult;", "done"), &{ani_done});',
            )
            with target.indented(
                f"if ({ani_done}) {{;",
                f"}};",
            ):
                target.writelns(
                    f"break;",
                )
            target.writelns(
                f"ani_ref {ani_item};",
                f'{env}->Object_GetField_Ref(static_cast<ani_object>({ani_next}),  TH_ANI_FIND_CLASS_FIELD({env}, "Lescompat/IteratorResult;", "value"), &{ani_item});',
                f"ani_ref {ani_key};",
                f"{env}->TupleValue_GetItem_Ref(static_cast<ani_tuple_value>({ani_item}), 0, &{ani_key});",
                f"ani_ref {ani_val};",
                f"{env}->TupleValue_GetItem_Ref(static_cast<ani_tuple_value>({ani_item}), 1, &{ani_val});",
            )
            key_ty_ani_info.from_ani_boxed(target, env, ani_key, cpp_key)
            val_ty_ani_info.from_ani_boxed(target, env, ani_val, cpp_val)
            target.writelns(
                f"{cpp_result}.emplace({cpp_key}, {cpp_val});",
            )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        key_ty_ani_info = TypeANIInfo.get(self.am, self.t.key_ty)
        val_ty_ani_info = TypeANIInfo.get(self.am, self.t.val_ty)
        cpp_key = f"{ani_result}_cpp_key"
        cpp_val = f"{ani_result}_cpp_val"
        ani_key = f"{ani_result}_ani_key"
        ani_val = f"{ani_result}_ani_val"
        target.writelns(
            f"ani_object {ani_result};",
            f'{env}->Object_New(TH_ANI_FIND_CLASS({env}, "{self.type_desc}"), TH_ANI_FIND_CLASS_METHOD({env}, "{self.type_desc}", "<ctor>", nullptr), &{ani_result});',
        )
        with target.indented(
            f"for (const auto& [{cpp_key}, {cpp_val}] : {cpp_value}) {{",
            f"}}",
        ):
            key_ty_ani_info.into_ani_boxed(target, env, cpp_key, ani_key)
            val_ty_ani_info.into_ani_boxed(target, env, cpp_val, ani_val)
            target.writelns(
                f'{env}->Object_CallMethod_Void({ani_result}, TH_ANI_FIND_CLASS_METHOD({env}, "Lescompat/Record;", "$_set", nullptr), {ani_key}, {ani_val});',
            )


class MapTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[MapType]):
    def __init__(self, am: AnalysisManager, t: MapType) -> None:
        super().__init__(am, t)
        self.am = am
        self.t = t
        self.ani_type = ANI_OBJECT
        self.type_desc = "Lescompat/Map;"
        raise_adhoc_error(
            am,
            f"Map is not supported yet, "
            f"if you want to use TS Record type, "
            f"please use `@record Map<{self.t.key_ty.ty_ref.text}, {self.t.val_ty.ty_ref.text}>`",
            self.t.ty_ref.loc,
        )

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        key_ty_ani_info = TypeANIInfo.get(self.am, self.t.key_ty)
        val_ty_ani_info = TypeANIInfo.get(self.am, self.t.val_ty)
        key_sts_type = key_ty_ani_info.sts_type_in(target)
        val_sts_type = val_ty_ani_info.sts_type_in(target)
        return f"Map<{key_sts_type}, {val_sts_type}>"

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_result};",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        target.writelns(
            f"ani_object {ani_result} = {{}};",
        )


class CallbackTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[CallbackType]):
    def __init__(self, am: AnalysisManager, t: CallbackType) -> None:
        super().__init__(am, t)
        self.am = am
        self.t = t
        self.ani_type = ANI_FN_OBJECT
        self.type_desc = f"Lstd/core/Function{len(t.params_ty)};"

    @override
    def sts_type_in(self, target: StsWriter) -> str:
        params_ty_sts = []
        for index, param_ty in enumerate(self.t.params_ty):
            param_ty_sts_info = TypeANIInfo.get(self.am, param_ty)
            prm_sts_type = param_ty_sts_info.sts_type_in(target)
            params_ty_sts.append(f"arg_{index}: {prm_sts_type}")
        params_ty_sts_str = ", ".join(params_ty_sts)
        if return_ty := self.t.return_ty:
            return_ty_sts_info = TypeANIInfo.get(self.am, return_ty)
            ret_sts_type = return_ty_sts_info.sts_type_in(target)
            return_ty_sts = ret_sts_type
        else:
            return_ty_sts = "void"
        return f"(({params_ty_sts_str}) => {return_ty_sts})"

    @override
    def from_ani(
        self,
        target: CSourceWriter,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        cpp_impl_class = f"{cpp_result}_cpp_impl_t"
        with target.indented(
            f"struct {cpp_impl_class} : ::taihe::dref_guard {{",
            f"}};",
        ):
            target.writelns(
                f"{cpp_impl_class}(ani_env* env, ani_ref val) : ::taihe::dref_guard(env, val) {{}}",
            )
            inner_cpp_params = []
            inner_ani_args = []
            inner_cpp_args = []
            for index, param_ty in enumerate(self.t.params_ty):
                inner_cpp_arg = f"cpp_arg_{index}"
                inner_ani_arg = f"ani_arg_{index}"
                param_ty_cpp_info = TypeCppInfo.get(self.am, param_ty)
                inner_cpp_params.append(f"{param_ty_cpp_info.as_param} {inner_cpp_arg}")
                inner_ani_args.append(inner_ani_arg)
                inner_cpp_args.append(inner_cpp_arg)
            cpp_params_str = ", ".join(inner_cpp_params)
            if return_ty := self.t.return_ty:
                return_ty_cpp_info = TypeCppInfo.get(self.am, return_ty)
                return_ty_as_owner = return_ty_cpp_info.as_owner
            else:
                return_ty_as_owner = "void"
            with target.indented(
                f"{return_ty_as_owner} operator()({cpp_params_str}) {{",
                f"}}",
            ):
                target.writelns(
                    f"::taihe::env_guard guard;",
                    f"ani_env *env = guard.get_env();",
                )
                for inner_ani_arg, inner_cpp_arg, param_ty in zip(
                    inner_ani_args, inner_cpp_args, self.t.params_ty, strict=True
                ):
                    param_ty_ani_info = TypeANIInfo.get(self.am, param_ty)
                    param_ty_ani_info.into_ani_boxed(
                        target, "env", inner_cpp_arg, inner_ani_arg
                    )
                inner_ani_args_str = ", ".join(inner_ani_args)
                if return_ty := self.t.return_ty:
                    inner_ani_res = "ani_result"
                    inner_cpp_res = "cpp_result"
                    target.writelns(
                        f"ani_ref ani_argv[] = {{{inner_ani_args_str}}};",
                        f"ani_ref {inner_ani_res};",
                        f"env->FunctionalObject_Call(static_cast<ani_fn_object>(this->ref), {len(self.t.params_ty)}, ani_argv, &{inner_ani_res});",
                    )
                    return_ty_ani_info = TypeANIInfo.get(self.am, return_ty)
                    return_ty_ani_info.from_ani_boxed(
                        target, "env", inner_ani_res, inner_cpp_res
                    )
                    target.writelns(
                        f"return {inner_cpp_res};",
                    )
                else:
                    inner_ani_res = "ani_result"
                    target.writelns(
                        f"ani_ref ani_argv[] = {{{inner_ani_args_str}}};",
                        f"ani_ref {inner_ani_res};",
                        f"env->FunctionalObject_Call(static_cast<ani_fn_object>(this->ref), {len(self.t.params_ty)}, ani_argv, &{inner_ani_res});",
                        f"return;",
                    )
            with target.indented(
                f"uintptr_t getGlobalReference() const {{",
                f"}}",
            ):
                target.writelns(
                    f"return reinterpret_cast<uintptr_t>(this->ref);",
                )
        target.writelns(
            f"{self.cpp_info.as_owner} {cpp_result} = ::taihe::make_holder<{cpp_impl_class}, {self.cpp_info.as_owner}, ::taihe::platform::ani::AniObject>({env}, {ani_value});",
        )

    @override
    def into_ani(
        self,
        target: CSourceWriter,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        # TODO: Callback into ani
        target.writelns(
            f"ani_fn_object {ani_result} = {{}};",
        )


class TypeANIInfo(TypeVisitor[AbstractTypeANIInfo]):
    def __init__(self, am: AnalysisManager):
        self.am = am

    @staticmethod
    def get(am: AnalysisManager, t: Type) -> AbstractTypeANIInfo:
        return TypeANIInfo(am).handle_type(t)

    @override
    def visit_enum_type(self, t: EnumType) -> AbstractTypeANIInfo:
        return EnumTypeANIInfo.get(self.am, t)

    @override
    def visit_union_type(self, t: UnionType) -> AbstractTypeANIInfo:
        return UnionTypeANIInfo.get(self.am, t)

    @override
    def visit_struct_type(self, t: StructType) -> AbstractTypeANIInfo:
        return StructTypeANIInfo.get(self.am, t)

    @override
    def visit_iface_type(self, t: IfaceType) -> AbstractTypeANIInfo:
        return IfaceTypeANIInfo.get(self.am, t)

    @override
    def visit_scalar_type(self, t: ScalarType) -> AbstractTypeANIInfo:
        return ScalarTypeANIInfo.get(self.am, t)

    @override
    def visit_string_type(self, t: StringType) -> AbstractTypeANIInfo:
        return StringTypeANIInfo.get(self.am, t)

    @override
    def visit_array_type(self, t: ArrayType) -> AbstractTypeANIInfo:
        if t.ty_ref.attrs.get("bigint"):
            return BigIntTypeANIInfo.get(self.am, t)
        if t.ty_ref.attrs.get("typedarray"):
            return TypedArrayTypeANIInfo.get(self.am, t)
        if t.ty_ref.attrs.get("arraybuffer"):
            return ArrayBufferTypeANIInfo.get(self.am, t)
        if t.ty_ref.attrs.get("fixedarray"):
            return FixedArrayTypeANIInfo.get(self.am, t)
        return ArrayTypeANIInfo.get(self.am, t)

    @override
    def visit_optional_type(self, t: OptionalType) -> AbstractTypeANIInfo:
        return OptionalTypeANIInfo.get(self.am, t)

    @override
    def visit_opaque_type(self, t: OpaqueType) -> AbstractTypeANIInfo:
        return OpaqueTypeANIInfo.get(self.am, t)

    @override
    def visit_map_type(self, t: MapType) -> AbstractTypeANIInfo:
        if t.ty_ref.attrs.get("record"):
            return RecordTypeANIInfo.get(self.am, t)
        return MapTypeANIInfo.get(self.am, t)

    @override
    def visit_callback_type(self, t: CallbackType) -> AbstractTypeANIInfo:
        return CallbackTypeANIInfo.get(self.am, t)