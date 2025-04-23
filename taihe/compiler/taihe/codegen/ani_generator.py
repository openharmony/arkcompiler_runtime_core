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
from typing import TYPE_CHECKING, Literal

from typing_extensions import override

from taihe.codegen.abi_generator import (
    IfaceABIInfo,
)
from taihe.codegen.cpp_generator import (
    EnumCppInfo,
    IfaceCppInfo,
    IfaceMethodCppInfo,
    StructCppInfo,
    TypeCppInfo,
    UnionCppInfo,
)
from taihe.codegen.cpp_user_generator import (
    GlobFuncCppUserInfo,
    PackageCppUserInfo,
)
from taihe.codegen.mangle import DeclKind, encode
from taihe.semantics.declarations import (
    AttrItemDecl,
    EnumDecl,
    GlobFuncDecl,
    IfaceDecl,
    IfaceMethodDecl,
    PackageDecl,
    PackageGroup,
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
from taihe.utils.outputs import COutputBuffer, OutputManager, STSOutputBuffer
from taihe.utils.sources import SourceLocation

if TYPE_CHECKING:
    from taihe.semantics.declarations import (
        ParamDecl,
    )


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
    def array(self) -> "ANIArrayType":
        pass
        return self.base.inner_array


@dataclass(repr=False)
class ANIArrayType(ANIType):
    def __hash__(self) -> int:
        return hash(self.hint)


@dataclass(repr=False)
class ANIBaseType(ANIType):
    inner_array: "ANIArrayType | None"

    def __init__(self, hint: str):
        super().__init__(hint, self)
        self.inner_array = None

    def __hash__(self) -> int:
        return hash(self.hint)


ANI_REF = ANIBaseType(hint="ref")
ANI_ARRAY_REF = ANIArrayType(hint="array_ref", base=ANI_REF)
ANI_REF.inner_array = ANI_ARRAY_REF

ANI_BOOLEAN = ANIBaseType(hint="boolean")
ANI_ARRAY_BOOLEAN = ANIArrayType(hint="array_boolean", base=ANI_REF)
ANI_BOOLEAN.inner_array = ANI_ARRAY_BOOLEAN

ANI_FLOAT = ANIBaseType(hint="float")
ANI_ARRAY_FLOAT = ANIArrayType(hint="array_float", base=ANI_REF)
ANI_FLOAT.inner_array = ANI_ARRAY_FLOAT

ANI_DOUBLE = ANIBaseType(hint="double")
ANI_ARRAY_DOUBLE = ANIArrayType(hint="array_double", base=ANI_REF)
ANI_DOUBLE.inner_array = ANI_ARRAY_DOUBLE

ANI_BYTE = ANIBaseType(hint="byte")
ANI_ARRAY_BYTE = ANIArrayType(hint="array_byte", base=ANI_REF)
ANI_BYTE.inner_array = ANI_ARRAY_BYTE

ANI_SHORT = ANIBaseType(hint="short")
ANI_ARRAY_SHORT = ANIArrayType(hint="array_short", base=ANI_REF)
ANI_SHORT.inner_array = ANI_ARRAY_SHORT

ANI_INT = ANIBaseType(hint="int")
ANI_ARRAY_INT = ANIArrayType(hint="array_int", base=ANI_REF)
ANI_INT.inner_array = ANI_ARRAY_INT

ANI_LONG = ANIBaseType(hint="long")
ANI_ARRAY_LONG = ANIArrayType(hint="array_long", base=ANI_REF)
ANI_LONG.inner_array = ANI_ARRAY_LONG

ANI_OBJECT = ANIType(hint="object", base=ANI_REF)
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


ANI_CLASS = ANIScope("class", ANI_METHOD)
ANI_MODULE = ANIScope("module", ANI_FUNCTION)
ANI_NAMESPACE = ANIScope("namespace", ANI_FUNCTION)


@dataclass
class ANINativeFuncInfo:
    sts_native_name: str
    mangled_name: str


@dataclass
class ANIRegisterInfo:
    impl_desc: str
    member_infos: list[ANINativeFuncInfo]
    parent_scope: ANIScope


class PackageANIInfo(AbstractAnalysis[PackageDecl]):
    def __init__(self, am: AnalysisManager, p: PackageDecl) -> None:
        super().__init__(am, p)
        self.am = am

        self.header = f"{p.name}.ani.hpp"
        self.source = f"{p.name}.ani.cpp"

        self.cpp_ns = "::".join(p.segments)

        if (namespace_attr := p.get_last_attr("namespace")) and check_attr_args(
            am, namespace_attr, "ss*"
        ):
            self.module_name, *sts_ns_parts = namespace_attr.args
            self.sts_ns_parts = []
            for sts_ns_part in sts_ns_parts:
                self.sts_ns_parts.extend(sts_ns_part.split("."))
        else:
            self.module_name = p.name
            self.sts_ns_parts = []

        self.ani_path = "/".join(self.module_name.split(".") + self.sts_ns_parts)
        self.impl_desc = f"L{self.ani_path};"

        self.injected_codes: list[str] = []
        for injected in p.get_all_attrs("sts_inject"):
            if check_attr_args(am, injected, "s"):
                (code,) = injected.args
                self.injected_codes.append(code)

        self.module_injected_codes: list[str] = []
        for module_injected in p.get_all_attrs("sts_inject_into_module"):
            if check_attr_args(am, module_injected, "s"):
                (code,) = module_injected.args
                self.module_injected_codes.append(code)

    @property
    def scope(self):
        return ANI_NAMESPACE if len(self.sts_ns_parts) > 0 else ANI_MODULE

    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer, sts_name: str):
        pkg_ani_info = PackageANIInfo.get(self.am, pkg)
        if pkg_ani_info.module_name == self.module_name:
            length = len(pkg_ani_info.sts_ns_parts)
            if self.sts_ns_parts[:length] == pkg_ani_info.sts_ns_parts:
                relative_name = ".".join(self.sts_ns_parts[length:] + [sts_name])
                return relative_name
        # name mangling
        import_name = "_" + "".join(c if c.isalnum() else "_" for c in self.module_name)
        target.import_module(import_name, self.module_name)
        relative_name = ".".join([import_name, *self.sts_ns_parts, sts_name])
        return relative_name


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

        if (
            self.resolve_ctor()
            or self.resolve_static()
            and (self.resolve_getter() or self.resolve_setter())
        ):
            pass
        elif self.resolve_on_off() or self.resolve_normal():
            self.resolve_async()
            self.resolve_promise()

        self.sts_params: list[ParamDecl] = []
        for param in f.params:
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
        self.sts_func_name = func_name  # pyre-ignore
        self.on_off_type = type_name
        return True

    def resolve_normal(self) -> bool:
        if overload_attr := self.f.get_last_attr("overload"):
            if not check_attr_args(self.am, overload_attr, "s"):
                return True
            (func_name,) = overload_attr.args
        else:
            if self.f.parent_pkg.get_last_attr("sts_keep_name"):
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

    def call_native_with(self, sts_args: list[str]) -> str:
        sts_native_args = sts_args
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
        self.ani_method_name = method_name  # pyre-ignore
        self.sts_method_name = method_name  # pyre-ignore
        self.on_off_type = type_name
        return True

    def resolve_normal(self) -> bool:
        if overload_attr := self.f.get_last_attr("overload"):
            if not check_attr_args(self.am, overload_attr, "s"):
                return True
            (method_name,) = overload_attr.args
        else:
            if self.f.parent_pkg.get_last_attr("sts_keep_name"):
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

    def call_native_with(self, this: str, sts_args: list[str]) -> str:
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

        self.pkg_ani_info = PackageANIInfo.get(am, d.parent_pkg)
        self.sts_type_name = d.name
        self.type_desc = f"L{self.pkg_ani_info.ani_path}/{self.sts_type_name};"

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

    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer):
        return self.pkg_ani_info.sts_type_in(pkg, target, self.sts_type_name)


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
        segments = [*d.parent_pkg.segments, d.name]
        self.from_ani_func_name = encode(segments, DeclKind.FROM_ANI)
        self.into_ani_func_name = encode(segments, DeclKind.INTO_ANI)
        self.decl_header = f"{d.parent_pkg.name}.{d.name}.ani.0.h"
        self.impl_header = f"{d.parent_pkg.name}.{d.name}.ani.1.h"

        self.pkg_ani_info = PackageANIInfo.get(am, d.parent_pkg)
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

    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer):
        return self.pkg_ani_info.sts_type_in(pkg, target, self.sts_type_name)


class StructFieldANIInfo(AbstractAnalysis[StructFieldDecl]):
    def __init__(self, am: AnalysisManager, d: StructFieldDecl) -> None:
        super().__init__(am, d)
        self.readonly = d.get_last_attr("readonly") is not None


class StructANIInfo(AbstractAnalysis[StructDecl]):
    def __init__(self, am: AnalysisManager, d: StructDecl) -> None:
        super().__init__(am, d)
        segments = [*d.parent_pkg.segments, d.name]
        self.from_ani_func_name = encode(segments, DeclKind.FROM_ANI)
        self.into_ani_func_name = encode(segments, DeclKind.INTO_ANI)
        self.decl_header = f"{d.parent_pkg.name}.{d.name}.ani.0.h"
        self.impl_header = f"{d.parent_pkg.name}.{d.name}.ani.1.h"

        self.pkg_ani_info = PackageANIInfo.get(am, d.parent_pkg)
        self.sts_type_name = d.name
        if d.get_last_attr("class"):
            self.sts_impl_name = f"{d.name}"
        else:
            self.sts_impl_name = f"{d.name}_inner"
        self.type_desc = f"L{self.pkg_ani_info.ani_path}/{self.sts_type_name};"
        self.impl_desc = f"L{self.pkg_ani_info.ani_path}/{self.sts_impl_name};"

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
        self.sts_parents: list[StructFieldDecl] = []
        self.sts_final_fields: list[list[StructFieldDecl]] = []
        for field in d.fields:
            try:
                if not field.get_last_attr("extends"):
                    raise RuntimeError
                ty = field.ty_ref.resolved_ty
                if not isinstance(ty, StructType):
                    raise_adhoc_error(
                        am,
                        "struct cannot extend non-struct type",
                        field.loc,
                    )
                    raise RuntimeError
                parent_ani_info = StructANIInfo.get(am, ty.ty_decl)
                if parent_ani_info.is_class():
                    raise_adhoc_error(
                        am,
                        "struct cannot extend an @class struct",
                        field.loc,
                    )
                    raise RuntimeError
                self.sts_parents.append(field)
                self.sts_final_fields.extend(
                    [field, *parts] for parts in parent_ani_info.sts_final_fields
                )
            except RuntimeError:
                self.sts_fields.append(field)
                self.sts_final_fields.append([field])

    def is_class(self):
        return self.sts_type_name == self.sts_impl_name

    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer):
        return self.pkg_ani_info.sts_type_in(pkg, target, self.sts_type_name)


class IfaceANIInfo(AbstractAnalysis[IfaceDecl]):
    def __init__(self, am: AnalysisManager, d: IfaceDecl) -> None:
        super().__init__(am, d)
        segments = [*d.parent_pkg.segments, d.name]
        self.from_ani_func_name = encode(segments, DeclKind.FROM_ANI)
        self.into_ani_func_name = encode(segments, DeclKind.INTO_ANI)
        self.decl_header = f"{d.parent_pkg.name}.{d.name}.ani.0.h"
        self.impl_header = f"{d.parent_pkg.name}.{d.name}.ani.1.h"

        self.pkg_ani_info = PackageANIInfo.get(am, d.parent_pkg)
        self.sts_type_name = d.name
        if d.get_last_attr("class"):
            self.sts_impl_name = f"{d.name}"
        else:
            self.sts_impl_name = f"{d.name}_inner"
        self.type_desc = f"L{self.pkg_ani_info.ani_path}/{self.sts_type_name};"
        self.impl_desc = f"L{self.pkg_ani_info.ani_path}/{self.sts_impl_name};"

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

        for parent in d.parents:
            ty = parent.ty_ref.resolved_ty
            pass
            parent_ani_info = IfaceANIInfo.get(am, ty.ty_decl)
            if parent_ani_info.is_class():
                raise_adhoc_error(
                    am,
                    "interface cannot extend an @class interface",
                    parent.loc,
                )

    def is_class(self):
        return self.sts_type_name == self.sts_impl_name

    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer):
        return self.pkg_ani_info.sts_type_in(pkg, target, self.sts_type_name)


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
    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer) -> str:
        pass

    @abstractmethod
    def _from_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        pass

    def from_ani(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        with target.indent_manager.offset(offset):
            self._from_ani_impl(target, env, ani_value, cpp_result)

    @abstractmethod
    def _into_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        pass

    def into_ani(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        with target.indent_manager.offset(offset):
            self._into_ani_impl(target, env, cpp_value, ani_result)

    def _from_ani_array_impl(
        self,
        target: COutputBuffer,
        env: str,
        ani_size: str,
        ani_array_value: str,
        cpp_array_buffer: str,
    ):
        if self.ani_type.base == ANI_REF:
            ani_value = f"{cpp_array_buffer}_ani_item"
            cpp_result = f"{cpp_array_buffer}_cpp_item"
            cpp_i = f"{cpp_array_buffer}_i"
            target.writeln(
                f"for (size_t {cpp_i} = 0; {cpp_i} < {ani_size}; {cpp_i}++) {{",
                f"    {self.ani_type} {ani_value};",
                f"    {env}->Array_Get_Ref({ani_array_value}, {cpp_i}, reinterpret_cast<ani_ref*>(&{ani_value}));",
            )
            self.from_ani(target, 4, env, ani_value, cpp_result)
            target.writeln(
                f"    new (&{cpp_array_buffer}[{cpp_i}]) {self.cpp_info.as_owner}(std::move({cpp_result}));",
                f"}}",
            )
        else:
            target.writeln(
                f"{env}->Array_GetRegion_{self.ani_type.suffix}({ani_array_value}, 0, {ani_size}, reinterpret_cast<{self.ani_type}*>({cpp_array_buffer}));",
            )

    def from_ani_array(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        ani_size: str,
        ani_array_value: str,
        cpp_array_buffer: str,
    ):
        with target.indent_manager.offset(offset):
            self._from_ani_array_impl(
                target, env, ani_size, ani_array_value, cpp_array_buffer
            )

    def _into_ani_array_impl(
        self,
        target: COutputBuffer,
        env: str,
        cpp_size: str,
        cpp_array_value: str,
        ani_array_result: str,
    ):
        if self.ani_type.base == ANI_REF:
            ani_class = f"{ani_array_result}_cls"
            ani_result = f"{ani_array_result}_item"
            ani_undefined = f"{ani_array_result}_undef"
            cpp_i = f"{ani_array_result}_i"
            target.writeln(
                f"ani_array_ref {ani_array_result};",
                f"ani_class {ani_class};",
                f'{env}->FindClass("{self.type_desc}", &{ani_class});',
                f"ani_ref {ani_undefined};",
                f"{env}->GetUndefined(&{ani_undefined});",
                f"{env}->Array_New_Ref({ani_class}, {cpp_size}, {ani_undefined}, &{ani_array_result});",
                f"for (size_t {cpp_i} = 0; {cpp_i} < {cpp_size}; {cpp_i}++) {{",
            )
            self.into_ani(target, 4, env, f"{cpp_array_value}[{cpp_i}]", ani_result)
            target.writeln(
                f"    {env}->Array_Set_Ref({ani_array_result}, {cpp_i}, {ani_result});",
                f"}}",
            )
        else:
            target.writeln(
                f"{self.ani_type.array} {ani_array_result};",
                f"{env}->Array_New_{self.ani_type.suffix}({cpp_size}, &{ani_array_result});",
                f"{env}->Array_SetRegion_{self.ani_type.suffix}({ani_array_result}, 0, {cpp_size}, reinterpret_cast<{self.ani_type} const*>({cpp_array_value}));",
            )

    def into_ani_array(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        cpp_size: str,
        cpp_array_value: str,
        ani_array_result: str,
    ):
        with target.indent_manager.offset(offset):
            self._into_ani_array_impl(
                target, env, cpp_size, cpp_array_value, ani_array_result
            )

    def _into_ani_boxed_impl(
        self,
        target: COutputBuffer,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        if self.ani_type.base == ANI_REF:
            self.into_ani(target, 0, env, cpp_value, ani_result)
        else:
            ani_class = f"{ani_result}_cls"
            ani_ctor = f"{ani_result}_ctor"
            ani_value = f"{ani_result}_ani"
            target.writeln(
                f"ani_class {ani_class};",
                f'{env}->FindClass("Lstd/core/{self.ani_type.suffix};", &{ani_class});',
                f"ani_method {ani_ctor};",
                f'{env}->Class_FindMethod({ani_class}, "<ctor>", "{self.type_desc}:V", &{ani_ctor});',
                f"ani_object {ani_result};",
            )
            self.into_ani(target, 0, env, cpp_value, ani_value)
            target.writeln(
                f"{env}->Object_New({ani_class}, {ani_ctor}, &{ani_result}, {ani_value});",
            )

    def into_ani_boxed(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        with target.indent_manager.offset(offset):
            self._into_ani_boxed_impl(target, env, cpp_value, ani_result)

    def _from_ani_boxed_impl(
        self,
        target: COutputBuffer,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        if self.ani_type.base == ANI_REF:
            self.from_ani(
                target,
                0,
                env,
                f"static_cast<{self.ani_type}>({ani_value})",
                cpp_result,
            )
        else:
            ani_class = f"{cpp_result}_cls"
            ani_getter = f"{cpp_result}_get"
            ani_result = f"{cpp_result}_ani"
            target.writeln(
                f"ani_class {ani_class};",
                f'{env}->FindClass("Lstd/core/{self.ani_type.suffix};", &{ani_class});',
                f"ani_method {ani_getter};",
                f'{env}->Class_FindMethod({ani_class}, "unboxed", nullptr, &{ani_getter});',
                f"{self.ani_type} {ani_result};",
                f"{env}->Object_CallMethod_{self.ani_type.suffix}((ani_object){ani_value}, {ani_getter}, &{ani_result});",
            )
            self.from_ani(target, 0, env, ani_result, cpp_result)

    def from_ani_boxed(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        with target.indent_manager.offset(offset):
            self._from_ani_boxed_impl(target, env, ani_value, cpp_result)


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
    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer) -> str:
        enum_ani_info = EnumANIInfo.get(self.am, self.t.ty_decl)
        return enum_ani_info.sts_type_in(pkg, target)

    @override
    def _from_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        ani_index = f"{cpp_result}_idx"
        enum_cpp_info = EnumCppInfo.get(self.am, self.t.ty_decl)
        target.writeln(
            f"ani_size {ani_index};",
            f"{env}->EnumItem_GetIndex({ani_value}, &{ani_index});",
            f"{enum_cpp_info.full_name} {cpp_result}(({enum_cpp_info.full_name}::key_t){ani_index});",
        )

    @override
    def _into_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        ani_class = f"{ani_result}_cls"
        target.writeln(
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
    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer) -> str:
        struct_ani_info = StructANIInfo.get(self.am, self.t.ty_decl)
        return struct_ani_info.sts_type_in(pkg, target)

    @override
    def _from_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        struct_ani_info = StructANIInfo.get(self.am, self.t.ty_decl)
        target.include(struct_ani_info.impl_header)
        target.writeln(
            f"{self.cpp_info.as_owner} {cpp_result} = {struct_ani_info.from_ani_func_name}({env}, {ani_value});",
        )

    @override
    def _into_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        struct_ani_info = StructANIInfo.get(self.am, self.t.ty_decl)
        target.include(struct_ani_info.impl_header)
        target.writeln(
            f"ani_object {ani_result} = {struct_ani_info.into_ani_func_name}({env}, {cpp_value});",
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
    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer) -> str:
        union_ani_info = UnionANIInfo.get(self.am, self.t.ty_decl)
        return union_ani_info.sts_type_in(pkg, target)

    @override
    def _from_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        union_ani_info = UnionANIInfo.get(self.am, self.t.ty_decl)
        target.include(union_ani_info.impl_header)
        target.writeln(
            f"{self.cpp_info.as_owner} {cpp_result} = {union_ani_info.from_ani_func_name}({env}, {ani_value});",
        )

    @override
    def _into_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        union_ani_info = UnionANIInfo.get(self.am, self.t.ty_decl)
        target.include(union_ani_info.impl_header)
        target.writeln(
            f"ani_ref {ani_result} = {union_ani_info.into_ani_func_name}({env}, {cpp_value});",
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
    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer) -> str:
        iface_ani_info = IfaceANIInfo.get(self.am, self.t.ty_decl)
        return iface_ani_info.sts_type_in(pkg, target)

    @override
    def _from_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        iface_ani_info = IfaceANIInfo.get(self.am, self.t.ty_decl)
        target.include(iface_ani_info.impl_header)
        target.writeln(
            f"{self.cpp_info.as_owner} {cpp_result} = {iface_ani_info.from_ani_func_name}({env}, {ani_value});",
        )

    @override
    def _into_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        iface_ani_info = IfaceANIInfo.get(self.am, self.t.ty_decl)
        target.include(iface_ani_info.impl_header)
        target.writeln(
            f"ani_object {ani_result} = {iface_ani_info.into_ani_func_name}({env}, {cpp_value});",
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
    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer) -> str:
        return self.sts_type

    @override
    def _from_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        target.writeln(
            f"{self.cpp_info.as_owner} {cpp_result} = ({self.cpp_info.as_owner}){ani_value};",
        )

    @override
    def _into_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        target.writeln(
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
    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer) -> str:
        return self.sts_type

    @override
    def _from_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        target.writeln(
            f"{self.cpp_info.as_owner} {cpp_result} = ({self.cpp_info.as_owner}){ani_value};",
        )

    @override
    def _into_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        target.writeln(
            f"{self.ani_type} {ani_result} = ({self.ani_type}){cpp_value};",
        )


class StringTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[StringType]):
    def __init__(self, am: AnalysisManager, t: StringType):
        super().__init__(am, t)
        self.ani_type = ANI_STRING
        self.type_desc = "Lstd/core/String;"

    @override
    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer) -> str:
        return "string"

    @override
    def _from_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        ani_length = f"{cpp_result}_len"
        cpp_tstr = f"{cpp_result}_tstr"
        cpp_buffer = f"{cpp_result}_buf"
        target.writeln(
            f"ani_size {ani_length};",
            f"{env}->String_GetUTF8Size({ani_value}, &{ani_length});",
            f"TString {cpp_tstr};",
            f"char* {cpp_buffer} = tstr_initialize(&{cpp_tstr}, {ani_length} + 1);",
            f"{env}->String_GetUTF8({ani_value}, {cpp_buffer}, {ani_length} + 1, &{ani_length});",
            f"{cpp_buffer}[{ani_length}] = '\\0';",
            f"{cpp_tstr}.length = {ani_length};",
            f"taihe::string {cpp_result} = taihe::string({cpp_tstr});",
        )

    @override
    def _into_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        target.writeln(
            f"ani_string {ani_result};",
            f"{env}->String_NewUTF8({cpp_value}.c_str(), {cpp_value}.size(), &{ani_result});",
        )


class ArrayTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[ArrayType]):
    def __init__(self, am: AnalysisManager, t: ArrayType) -> None:
        super().__init__(am, t)
        self.am = am
        self.t = t
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        self.ani_type = item_ty_ani_info.ani_type.array
        self.type_desc = f"[{item_ty_ani_info.type_desc}"
        # TODO: remove this check
        if isinstance(self.t.item_ty, ScalarType) and self.t.item_ty.kind in (
            ScalarKind.U8,
            ScalarKind.U16,
            ScalarKind.U32,
            ScalarKind.U64,
        ):
            raise_adhoc_error(
                am,
                f"Array of unsigned integer types is not supported, "
                f"if you want to use ArrayBuffer, please use @arraybuffer Array<{self.t.item_ty.ty_ref.text}>, "
                f"if you want to use TypedArray, please use @typedarray Array<{self.t.item_ty.ty_ref.text}>",
                self.t.ty_ref.loc,
            )

    @override
    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer) -> str:
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        sts_type = item_ty_ani_info.sts_type_in(pkg, target)
        return f"({sts_type}[])"

    @override
    def _from_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        ani_size = f"{cpp_result}_size"
        cpp_buffer = f"{cpp_result}_buffer"
        target.writeln(
            f"size_t {ani_size};",
            f"{env}->Array_GetLength({ani_value}, &{ani_size});",
            f"{item_ty_cpp_info.as_owner}* {cpp_buffer} = reinterpret_cast<{item_ty_cpp_info.as_owner}*>(malloc({ani_size} * sizeof({item_ty_cpp_info.as_owner})));",
        )
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        item_ty_ani_info.from_ani_array(target, 0, env, ani_size, ani_value, cpp_buffer)
        target.writeln(
            f"{self.cpp_info.as_owner} {cpp_result}({cpp_buffer}, {ani_size});",
        )

    @override
    def _into_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        cpp_size = f"{ani_result}_size"
        target.writeln(
            f"size_t {cpp_size} = {cpp_value}.size();",
        )
        item_ty_ani_info.into_ani_array(
            target, 0, env, cpp_size, f"{cpp_value}.data()", ani_result
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
    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer) -> str:
        return "ArrayBuffer"

    @override
    def _from_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        ani_data = f"{cpp_result}_data"
        ani_length = f"{cpp_result}_length"
        target.writeln(
            f"char* {ani_data} = nullptr;",
            f"size_t {ani_length} = 0;",
            f"{env}->ArrayBuffer_GetInfo(reinterpret_cast<ani_arraybuffer>({ani_value}), reinterpret_cast<void**>(&{ani_data}), &{ani_length});",
            f"{self.cpp_info.as_param} {cpp_result}(reinterpret_cast<{item_ty_cpp_info.as_owner}*>({ani_data}), {ani_length});",
        )

    @override
    def _into_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        ani_data = f"{ani_result}_data"
        target.writeln(
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
    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer) -> str:
        return self.sts_type

    @override
    def _from_ani_impl(
        self,
        target: COutputBuffer,
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
        target.writeln(
            f"ani_double {ani_byte_length};",
            f"ani_double {ani_byte_offset};",
            f"ani_arraybuffer {ani_arrbuf};",
            f'{env}->Object_GetPropertyByName_Double({ani_value}, "byteLength", &{ani_byte_length});',
            f'{env}->Object_GetPropertyByName_Double({ani_value}, "byteOffset", &{ani_byte_offset});',
            f'{env}->Object_GetPropertyByName_Ref({ani_value}, "buffer", reinterpret_cast<ani_ref*>(&{ani_arrbuf}));',
            f"char* {ani_data} = nullptr;",
            f"size_t {ani_length} = 0;",
            f"{env}->ArrayBuffer_GetInfo(reinterpret_cast<ani_arraybuffer>({ani_arrbuf}), reinterpret_cast<void**>(&{ani_data}), &{ani_length});",
            f"{self.cpp_info.as_param} {cpp_result}(reinterpret_cast<{item_ty_cpp_info.as_owner}*>({ani_data} + (size_t){ani_byte_offset}), (size_t){ani_byte_length} / (sizeof({item_ty_cpp_info.as_owner}) / sizeof(char)));",
        )

    @override
    def _into_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        ani_data = f"{ani_result}_data"
        ani_arrbuf = f"{ani_result}_arrbuf"
        ani_class = f"{ani_result}_cls"
        ani_method = f"{ani_result}_ctor"
        ani_byte_length = f"{ani_result}_bylen"
        ani_byte_offset = f"{ani_result}_byoff"
        target.writeln(
            f"char* {ani_data} = nullptr;",
            f"ani_arraybuffer {ani_arrbuf};",
            f"{env}->CreateArrayBuffer({cpp_value}.size() * (sizeof({item_ty_cpp_info.as_owner}) / sizeof(char)), reinterpret_cast<void**>(&{ani_data}), &{ani_arrbuf});",
            f"memcpy({ani_data}, {cpp_value}.data(), {cpp_value}.size() * (sizeof({item_ty_cpp_info.as_owner}) / sizeof(char)));",
            f"ani_class {ani_class};",
            f'{env}->FindClass("{self.type_desc}", &{ani_class});',
            f"ani_method {ani_method};",
            f'{env}->Class_FindMethod({ani_class}, "<ctor>", "Lescompat/Buffer;Lstd/core/Double;Lstd/core/Double;:V", &{ani_method});',
            f"ani_ref {ani_byte_length};",
            f"{env}->GetUndefined(&{ani_byte_length});",
            f"ani_ref {ani_byte_offset};",
            f"{env}->GetUndefined(&{ani_byte_offset});",
            f"ani_object {ani_result};",
            f"{env}->Object_New({ani_class}, {ani_method}, &{ani_result}, {ani_arrbuf}, {ani_byte_length}, {ani_byte_offset});",
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
    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer) -> str:
        return "BigInt"

    @override
    def _from_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        pkg_ani_info = PackageANIInfo.get(self.am, self.t.ty_ref.parent_pkg)
        parent_scope = pkg_ani_info.scope
        ani_scope = f"{cpp_result}_scope"
        ani_cast = f"{cpp_result}_cast"
        ani_arrbuf = f"{cpp_result}_arrbuf"
        ani_data = f"{cpp_result}_data"
        ani_length = f"{cpp_result}_length"
        target.writeln(
            f"{parent_scope} {ani_scope};",
            f'{env}->Find{parent_scope.suffix}("{pkg_ani_info.impl_desc}", &{ani_scope});',
            f"ani_function {ani_cast};",
            f'{env}->{parent_scope.suffix}_FindFunction({ani_scope}, "__fromBigIntToArrayBuffer", nullptr, &{ani_cast});',
            f"ani_arraybuffer {ani_arrbuf};",
            f"{env}->Function_Call_Ref({ani_cast}, reinterpret_cast<ani_ref*>(&{ani_arrbuf}), {ani_value}, sizeof({item_ty_cpp_info.as_owner}) / sizeof(char));"
            f"char* {ani_data} = nullptr;",
            f"size_t {ani_length} = 0;",
            f"{env}->ArrayBuffer_GetInfo(reinterpret_cast<ani_arraybuffer>({ani_arrbuf}), reinterpret_cast<void**>(&{ani_data}), &{ani_length});",
            f"{self.cpp_info.as_param} {cpp_result}(reinterpret_cast<{item_ty_cpp_info.as_owner}*>({ani_data}), {ani_length} / (sizeof({item_ty_cpp_info.as_owner}) / sizeof(char)));",
        )

    @override
    def _into_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        pkg_ani_info = PackageANIInfo.get(self.am, self.t.ty_ref.parent_pkg)
        parent_scope = pkg_ani_info.scope
        ani_data = f"{ani_result}_data"
        ani_arrbuf = f"{ani_result}_arrbuf"
        ani_scope = f"{ani_result}_scope"
        ani_cast = f"{ani_result}_cast"
        target.writeln(
            f"char* {ani_data} = nullptr;",
            f"ani_arraybuffer {ani_arrbuf};",
            f"{env}->CreateArrayBuffer({cpp_value}.size() * (sizeof({item_ty_cpp_info.as_owner}) / sizeof(char)), reinterpret_cast<void**>(&{ani_data}), &{ani_arrbuf});",
            f"memcpy({ani_data}, {cpp_value}.data(), {cpp_value}.size() * (sizeof({item_ty_cpp_info.as_owner}) / sizeof(char)));",
            f"{parent_scope} {ani_scope};",
            f'{env}->Find{parent_scope.suffix}("{pkg_ani_info.impl_desc}", &{ani_scope});',
            f"ani_function {ani_cast};",
            f'{env}->{parent_scope.suffix}_FindFunction({ani_scope}, "__fromArrayBufferToBigInt", nullptr, &{ani_cast});',
            f"ani_object {ani_result};",
            f"{env}->Function_Call_Ref({ani_cast}, reinterpret_cast<ani_ref*>(&{ani_result}), {ani_arrbuf});",
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
    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer) -> str:
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        sts_type = item_ty_ani_info.sts_type_in(pkg, target)
        return f"({sts_type} | undefined)"

    @override
    def _from_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        ani_is_undefined = f"{cpp_result}_flag"
        cpp_pointer = f"{cpp_result}_ptr"
        cpp_spec = f"{cpp_result}_spec"
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        target.writeln(
            f"ani_boolean {ani_is_undefined};",
            f"{item_ty_cpp_info.as_owner}* {cpp_pointer} = nullptr;",
            f"{env}->Reference_IsUndefined({ani_value}, &{ani_is_undefined});",
            f"if (!{ani_is_undefined}) {{",
        )
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        item_ty_ani_info.from_ani_boxed(target, 4, env, ani_value, cpp_spec)
        target.writeln(
            f"    {cpp_pointer} = new {item_ty_cpp_info.as_owner}(std::move({cpp_spec}));",
            f"}};",
            f"{self.cpp_info.as_owner} {cpp_result}({cpp_pointer});",
        )

    @override
    def _into_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        ani_spec = f"{ani_result}_spec"
        target.writeln(
            f"ani_ref {ani_result};",
            f"if (!{cpp_value}) {{",
            f"    {env}->GetUndefined(&{ani_result});",
            f"}} else {{",
        )
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        item_ty_ani_info.into_ani_boxed(target, 4, env, f"(*{cpp_value})", ani_spec)
        target.writeln(
            f"    {ani_result} = {ani_spec};",
            f"}}",
        )


class RecordTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[MapType]):
    def __init__(self, am: AnalysisManager, t: MapType) -> None:
        super().__init__(am, t)
        self.am = am
        self.t = t
        self.ani_type = ANI_OBJECT
        self.type_desc = "Lescompat/Record;"

    @override
    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer) -> str:
        key_ty_ani_info = TypeANIInfo.get(self.am, self.t.key_ty)
        val_ty_ani_info = TypeANIInfo.get(self.am, self.t.val_ty)
        key_sts_type = key_ty_ani_info.sts_type_in(pkg, target)
        val_sts_type = val_ty_ani_info.sts_type_in(pkg, target)
        return f"Record<{key_sts_type}, {val_sts_type}>"

    @override
    def _from_ani_impl(
        self,
        target: COutputBuffer,
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
        target.writeln(
            f"ani_ref {ani_iter};",
            f'{env}->Object_CallMethodByName_Ref({ani_value}, "$_iterator", nullptr, &{ani_iter});',
            f"{self.cpp_info.as_owner} {cpp_result};",
            f"while (true) {{",
            f"    ani_ref {ani_next};",
            f"    ani_boolean {ani_done};",
            f'    {env}->Object_CallMethodByName_Ref(static_cast<ani_object>({ani_iter}), "next", nullptr, &{ani_next});',
            f'    {env}->Object_GetFieldByName_Boolean(static_cast<ani_object>({ani_next}), "done", &{ani_done});',
            f"    if ({ani_done}) {{;",
            f"        break;",
            f"    }};",
            f"    ani_ref {ani_item};",
            f'    {env}->Object_GetFieldByName_Ref(static_cast<ani_object>({ani_next}), "value", &{ani_item});',
            f"    ani_ref {ani_key};",
            f"    {env}->TupleValue_GetItem_Ref(static_cast<ani_tuple_value>({ani_item}), 0, &{ani_key});",
            f"    ani_ref {ani_val};",
            f"    {env}->TupleValue_GetItem_Ref(static_cast<ani_tuple_value>({ani_item}), 1, &{ani_val});",
        )
        key_ty_ani_info.from_ani_boxed(target, 4, env, ani_key, cpp_key)
        val_ty_ani_info.from_ani_boxed(target, 4, env, ani_val, cpp_val)
        target.writeln(
            f"    {cpp_result}.emplace({cpp_key}, {cpp_val});",
            f"}}",
        )

    @override
    def _into_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        key_ty_ani_info = TypeANIInfo.get(self.am, self.t.key_ty)
        val_ty_ani_info = TypeANIInfo.get(self.am, self.t.val_ty)
        ani_class = f"{ani_result}_class"
        ani_method = f"{ani_result}_ctor"
        cpp_key = f"{ani_result}_cpp_key"
        cpp_val = f"{ani_result}_cpp_val"
        ani_key = f"{ani_result}_ani_key"
        ani_val = f"{ani_result}_ani_val"
        target.writeln(
            f"ani_class {ani_class};",
            f'{env}->FindClass("{self.type_desc}", &{ani_class});',
            f"ani_method {ani_method};",
            f'{env}->Class_FindMethod({ani_class}, "<ctor>", nullptr, &{ani_method});',
            f"ani_object {ani_result};",
            f"{env}->Object_New({ani_class}, {ani_method}, &{ani_result});",
            f"for (const auto& [{cpp_key}, {cpp_val}] : {cpp_value}) {{",
        )
        key_ty_ani_info.into_ani_boxed(target, 4, env, cpp_key, ani_key)
        val_ty_ani_info.into_ani_boxed(target, 4, env, cpp_val, ani_val)
        target.writeln(
            f'    {env}->Object_CallMethodByName_Void({ani_result}, "$_set", nullptr, {ani_key}, {ani_val});',
            f"}}",
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
            f"please use @record Map<{self.t.key_ty.ty_ref.text}, {self.t.val_ty.ty_ref.text}>",
            self.t.ty_ref.loc,
        )

    @override
    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer) -> str:
        key_ty_ani_info = TypeANIInfo.get(self.am, self.t.key_ty)
        val_ty_ani_info = TypeANIInfo.get(self.am, self.t.val_ty)
        key_sts_type = key_ty_ani_info.sts_type_in(pkg, target)
        val_sts_type = val_ty_ani_info.sts_type_in(pkg, target)
        return f"Map<{key_sts_type}, {val_sts_type}>"

    @override
    def _from_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        target.writeln(
            f"{self.cpp_info.as_owner} {cpp_result};",
        )

    @override
    def _into_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        target.writeln(
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
    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer) -> str:
        params_ty_sts = []
        for index, param_ty in enumerate(self.t.params_ty):
            param_ty_sts_info = TypeANIInfo.get(self.am, param_ty)
            prm_sts_type = param_ty_sts_info.sts_type_in(pkg, target)
            params_ty_sts.append(f"arg_{index}: {prm_sts_type}")
        params_ty_sts_str = ", ".join(params_ty_sts)
        if return_ty := self.t.return_ty:
            return_ty_sts_info = TypeANIInfo.get(self.am, return_ty)
            ret_sts_type = return_ty_sts_info.sts_type_in(pkg, target)
            return_ty_sts = ret_sts_type
        else:
            return_ty_sts = "void"
        return f"(({params_ty_sts_str}) => {return_ty_sts})"

    @override
    def _from_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        cpp_impl_class = f"{cpp_result}_cpp_impl_t"
        target.writeln(
            f"struct {cpp_impl_class} {{",
            f"    ani_ref ref;",
            f"    {cpp_impl_class}(ani_env* env, ani_fn_object obj) {{",
            f"        env->GlobalReference_Create(obj, &this->ref);",
            f"    }}",
            f"    ~{cpp_impl_class}() {{",
            f"        ani_env *env = ::taihe::get_env();",
            f"        env->GlobalReference_Delete(this->ref);",
            f"    }}",
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
        target.writeln(
            f"    {return_ty_as_owner} operator()({cpp_params_str}) {{",
            f"        ani_env *env = ::taihe::get_env();",
        )
        for inner_ani_arg, inner_cpp_arg, param_ty in zip(
            inner_ani_args, inner_cpp_args, self.t.params_ty, strict=True
        ):
            param_ty_ani_info = TypeANIInfo.get(self.am, param_ty)
            param_ty_ani_info.into_ani_boxed(
                target, 8, "env", inner_cpp_arg, inner_ani_arg
            )
        inner_ani_args_str = ", ".join(inner_ani_args)
        if return_ty := self.t.return_ty:
            inner_ani_res = "ani_result"
            inner_cpp_res = "cpp_result"
            target.writeln(
                f"        ani_ref ani_argv[] = {{{inner_ani_args_str}}};",
                f"        ani_ref {inner_ani_res};",
                f"        env->FunctionalObject_Call(static_cast<ani_fn_object>(this->ref), {len(self.t.params_ty)}, ani_argv, &{inner_ani_res});",
            )
            return_ty_ani_info = TypeANIInfo.get(self.am, return_ty)
            return_ty_ani_info.from_ani_boxed(
                target, 8, "env", inner_ani_res, inner_cpp_res
            )
            target.writeln(
                f"        return {inner_cpp_res};",
            )
        else:
            inner_ani_res = "ani_result"
            target.writeln(
                f"        ani_ref ani_argv[] = {{{inner_ani_args_str}}};",
                f"        ani_ref {inner_ani_res};",
                f"        env->FunctionalObject_Call(static_cast<ani_fn_object>(this->ref), {len(self.t.params_ty)}, ani_argv, &{inner_ani_res});",
                f"        return;",
            )
        target.writeln(
            f"    }}",
        )
        target.writeln(
            f"}};",
            f"{self.cpp_info.as_owner} {cpp_result} = {self.cpp_info.as_owner}::from<{cpp_impl_class}>({env}, {ani_value});",
        )

    @override
    def _into_ani_impl(
        self,
        target: COutputBuffer,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        # TODO: Callback into ani
        target.writeln(
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
        constructor_target.writeln(
            f"ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result) {{",
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
                parent_scope=ANI_CLASS,
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
            f"    ::taihe::set_env(env);",
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
            f"    ::taihe::set_env(env);",
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
            f"    ::taihe::set_env(env);",
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
            f"            ani_env *env = ::taihe::get_env();",
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
                    f"            ani_env *env = ::taihe::get_env();",
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
            f"    return taihe::make_holder<cpp_impl_t, {iface_cpp_info.as_owner}>(env, ani_obj);",
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
            f'    env->Class_FindMethod(ani_obj_cls, "<ctor>", nullptr, &ani_obj_ctor);',
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
                    f"taihe::static_tag<{path_cpp_info.full_name}::tag_t::{part.name}>"
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
                    f"taihe::static_tag<{path_cpp_info.full_name}::tag_t::{part.name}>"
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