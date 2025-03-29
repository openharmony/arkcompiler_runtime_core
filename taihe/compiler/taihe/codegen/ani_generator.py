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
from typing import TYPE_CHECKING

from typing_extensions import override

from taihe.codegen.abi_generator import (
    IfaceABIInfo,
)
from taihe.codegen.cpp_generator import (
    CallbackTypeCppInfo,
    EnumCppInfo,
    GlobFuncCppInfo,
    IfaceCppInfo,
    IfaceMethodCppInfo,
    PackageCppInfo,
    StructCppInfo,
    TypeCppInfo,
    UnionCppInfo,
)
from taihe.codegen.mangle import DeclKind, encode
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
from taihe.semantics.types import (
    BOOL,
    F32,
    F64,
    I8,
    I16,
    I32,
    I64,
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
    StringType,
    StructType,
    Type,
    UnionType,
)
from taihe.semantics.visitor import TypeVisitor
from taihe.utils.analyses import AbstractAnalysis, AnalysisManager
from taihe.utils.outputs import COutputBuffer, OutputManager, STSOutputBuffer

if TYPE_CHECKING:
    from taihe.semantics.declarations import (
        ParamDecl,
        StructFieldDecl,
        UnionFieldDecl,
    )


class ANIType:
    hint: str
    base: "ANIBaseType"

    def __init__(self, hint: str, base: "ANIBaseType"):
        self.hint = hint
        self.base = base

    def __repr__(self) -> str:
        return f"ani_{self.hint}"

    @property
    def suffix(self) -> str:
        return self.base.hint[0].upper() + self.base.hint[1:]

    @property
    def array(self) -> "ANIArrayType":
        assert self.base.inner_array
        return self.base.inner_array


class ANIArrayType(ANIType):
    pass


class ANIBaseType(ANIType):
    inner_array: "ANIArrayType | None"

    def __init__(self, hint: str):
        super().__init__(hint, self)
        self.inner_array = None


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
ANI_ENUM_ITEM = ANIType(hint="enum_item", base=ANI_REF)
ANI_STRING = ANIType(hint="string", base=ANI_REF)
ANI_ARRAYBUFFER = ANIType(hint="arraybuffer", base=ANI_REF)


class PackageANIInfo(AbstractAnalysis[PackageDecl]):
    def __init__(self, am: AnalysisManager, p: PackageDecl) -> None:
        super().__init__(am, p)
        self.am = am

        self.header = f"{p.name}.ani.hpp"
        self.source = f"{p.name}.ani.cpp"

        self.cpp_ns = "::".join(p.segments)

        if namespace_attr := p.get_attr_item("namespace"):
            self.module, *sts_ns_parts = namespace_attr.args
            self.sts_ns_parts = []
            for sts_ns_part in sts_ns_parts:
                self.sts_ns_parts.extend(sts_ns_part.split("."))
        else:
            self.module = p.name
            self.sts_ns_parts = []

        self.imported_mod_name = "_" + "_".join(self.module.split("."))

        self.ani_path = "/".join(self.module.split(".") + self.sts_ns_parts)
        if self.sts_ns_parts:
            self.impl_desc = f"L{self.ani_path};"
        else:
            self.impl_desc = f"L{self.ani_path}/ETSGLOBAL;"

        self.injected_codes: list[str] = []
        for injected in p.get_attr_list("sts_inject"):
            (code,) = injected.args
            self.injected_codes.append(code)

    def is_namespace(self):
        return len(self.sts_ns_parts) > 0

    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer, sts_name: str):
        pkg_ani_info = PackageANIInfo.get(self.am, pkg)
        if pkg_ani_info.module == self.module:
            self_sts_ns_parts = iter(self.sts_ns_parts)
            for else_sts_ns_part in pkg_ani_info.sts_ns_parts:
                try:
                    if next(self_sts_ns_parts) != else_sts_ns_part:
                        break
                except Exception:
                    break
            else:
                relative_name = ".".join([*self_sts_ns_parts, sts_name])
                return relative_name
        target.imports([(self.module, self.imported_mod_name)])
        relative_name = ".".join([self.imported_mod_name, *self.sts_ns_parts, sts_name])
        return relative_name


class GlobFuncANIInfo(AbstractAnalysis[GlobFuncDecl]):
    def __init__(self, am: AnalysisManager, f: GlobFuncDecl) -> None:
        super().__init__(am, f)
        self.f = f

        p = f.node_parent
        assert p
        segments = [*p.segments, f.name]
        self.mangled_name = encode(segments, DeclKind.ANI_FUNC)

        self.sts_native_name = f"{f.name}_inner"

        self.sts_func_name = None
        self.on_off_type = None

        self.sts_static_scope = None
        self.sts_ctor_scope = None

        self.sts_async_name = None
        self.sts_promise_name = None

        if overload_attr := f.get_attr_item("overload"):
            (self.sts_func_name,) = overload_attr.args

        elif on_off_attr := f.get_attr_item("on_off"):
            assert f.name.startswith(
                ("on", "off")
            ), f'{f.loc}: @on_off method name must start with "on" or "off"'

            if f.name.startswith("on"):
                if not on_off_attr.args:
                    type_name = f.name[2:]
                    type_name = type_name[0].lower() + type_name[1:]
                else:
                    (type_name,) = on_off_attr.args
                self.on_off_type = ("on", type_name)

            if f.name.startswith("off"):
                if not on_off_attr.args:
                    type_name = f.name[3:]
                    type_name = type_name[0].lower() + type_name[1:]
                else:
                    (type_name,) = on_off_attr.args
                self.on_off_type = ("off", type_name)

        else:
            self.sts_func_name = f.name

        if static_attr := f.get_attr_item("static"):
            (self.sts_static_scope,) = static_attr.args

        if ctor_attr := f.get_attr_item("ctor"):
            (self.sts_ctor_scope,) = ctor_attr.args

        if sts_async_attr := f.get_attr_item("gen_async"):
            (self.sts_async_name,) = sts_async_attr.args

        if sts_promise_attr := f.get_attr_item("gen_promise"):
            (self.sts_promise_name,) = sts_promise_attr.args

        self.sts_real_params: list[ParamDecl] = []
        for param in f.params:
            self.sts_real_params.append(param)

    def call_native_with(self, sts_real_args: list[str]) -> str:
        sts_native_args = sts_real_args
        sts_native_args_str = ", ".join(sts_native_args)
        return f"{self.sts_native_name}({sts_native_args_str})"


class IfaceMethodANIInfo(AbstractAnalysis[IfaceMethodDecl]):
    def __init__(self, am: AnalysisManager, f: IfaceMethodDecl) -> None:
        super().__init__(am, f)
        self.f = f

        d = f.node_parent
        assert d
        p = d.node_parent
        assert p
        segments = [*p.segments, d.name, f.name]
        self.mangled_name = encode(segments, DeclKind.ANI_FUNC)

        self.sts_native_name = f"{f.name}_inner"

        self.sts_method_name = None
        self.get_name = None
        self.set_name = None
        self.on_off_type = None

        self.ani_method_name = None

        self.sts_async_name = None
        self.sts_promise_name = None

        if overload_attr := f.get_attr_item("overload"):
            (self.sts_method_name,) = overload_attr.args
            (self.ani_method_name,) = overload_attr.args

        elif on_off_attr := f.get_attr_item("on_off"):
            assert f.name.startswith(
                ("on", "off")
            ), f'{f.loc}: @on_off method name must start with "on" or "off"'

            if f.name.startswith("on"):
                if not on_off_attr.args:
                    type_name = f.name[2:]
                    type_name = type_name[0].lower() + type_name[1:]
                else:
                    (type_name,) = on_off_attr.args
                self.on_off_type = ("on", type_name)
                self.ani_method_name = "on"

            if f.name.startswith("off"):
                if not on_off_attr.args:
                    type_name = f.name[3:]
                    type_name = type_name[0].lower() + type_name[1:]
                else:
                    (type_name,) = on_off_attr.args
                self.on_off_type = ("off", type_name)
                self.ani_method_name = "off"

        elif get_attr := f.get_attr_item("get"):
            assert len(f.params) == 0, f"{f.loc}: @get method should take no parameters"
            assert f.return_ty_ref, f"{f.loc}: @get method cannot return void"

            if not get_attr.args:
                assert f.name.startswith(
                    "get"
                ), f'{f.loc}: @get method name must start with "get" if the property name is omitted'
                get_name = f.name[3:]
                self.get_name = get_name[0].lower() + get_name[1:]
            else:
                (self.get_name,) = get_attr.args
            self.ani_method_name = f"<get>{self.get_name}"

        elif set_attr := f.get_attr_item("set"):
            assert len(f.params) == 1, f"{f.loc}: @set method should have one parameter"
            assert f.return_ty_ref is None, f"{f.loc}: @set method should return void"

            if not set_attr.args:
                assert f.name.startswith(
                    "set"
                ), f'{f.loc}: @set method name must start with "set" if the property name is omitted'
                set_name = f.name[3:]
                self.set_name = set_name[0].lower() + set_name[1:]
            else:
                (self.set_name,) = set_attr.args
            self.ani_method_name = f"<set>{self.set_name}"

        else:
            self.sts_method_name = f.name
            self.ani_method_name = f.name

        if sts_async_attr := f.get_attr_item("gen_async"):
            (self.sts_async_name,) = sts_async_attr.args

        if sts_promise_attr := f.get_attr_item("gen_promise"):
            (self.sts_promise_name,) = sts_promise_attr.args

        self.sts_real_params: list[ParamDecl] = []
        for param in f.params:
            if param.get_attr_item("sts_this"):
                continue
            self.sts_real_params.append(param)

    def call_native_with(self, this: str, sts_real_args: list[str]) -> str:
        arg = iter(sts_real_args)
        sts_native_args: list[str] = []
        for param in self.f.params:
            if param.get_attr_item("sts_this"):
                sts_native_args.append(this)
                continue
            sts_native_args.append(next(arg))
        sts_native_args_str = ", ".join(sts_native_args)
        return f"{this}.{self.sts_native_name}({sts_native_args_str})"


class EnumANIInfo(AbstractAnalysis[EnumDecl]):
    def __init__(self, am: AnalysisManager, d: EnumDecl) -> None:
        super().__init__(am, d)
        p = d.node_parent
        assert p

        self.pkg_ani_info = PackageANIInfo.get(am, p)
        self.sts_type_name = d.name
        self.type_desc = f"L{self.pkg_ani_info.ani_path}/{self.sts_type_name};"

        self.const = d.get_attr_item("const") is not None

    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer):
        return self.pkg_ani_info.sts_type_in(pkg, target, self.sts_type_name)


class UnionANIInfo(AbstractAnalysis[UnionDecl]):
    def __init__(self, am: AnalysisManager, d: UnionDecl) -> None:
        super().__init__(am, d)
        p = d.node_parent
        assert p
        segments = [*p.segments, d.name]
        self.from_ani_func_name = encode(segments, DeclKind.FROM_ANI)
        self.into_ani_func_name = encode(segments, DeclKind.INTO_ANI)
        self.decl_header = f"{p.name}.{d.name}.ani.0.h"
        self.impl_header = f"{p.name}.{d.name}.ani.1.h"

        self.pkg_ani_info = PackageANIInfo.get(am, p)
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


class StructANIInfo(AbstractAnalysis[StructDecl]):
    def __init__(self, am: AnalysisManager, d: StructDecl) -> None:
        super().__init__(am, d)
        p = d.node_parent
        assert p
        segments = [*p.segments, d.name]
        self.from_ani_func_name = encode(segments, DeclKind.FROM_ANI)
        self.into_ani_func_name = encode(segments, DeclKind.INTO_ANI)
        self.decl_header = f"{p.name}.{d.name}.ani.0.h"
        self.impl_header = f"{p.name}.{d.name}.ani.1.h"

        self.pkg_ani_info = PackageANIInfo.get(am, p)
        self.sts_type_name = d.name
        if d.get_attr_item("class"):
            self.sts_impl_name = f"{d.name}"
        else:
            self.sts_impl_name = f"{d.name}_inner"
        self.type_desc = f"L{self.pkg_ani_info.ani_path}/{self.sts_type_name};"
        self.impl_desc = f"L{self.pkg_ani_info.ani_path}/{self.sts_impl_name};"

        self.sts_fields: list[StructFieldDecl] = []
        self.sts_parents: list[StructFieldDecl] = []
        self.sts_final_fields: list[list[StructFieldDecl]] = []
        for field in d.fields:
            if field.get_attr_item("extends"):
                ty = field.ty_ref.resolved_ty
                assert isinstance(
                    ty, StructType
                ), f"{field.loc}: @extends expects a struct type field"
                parent_ani_info = StructANIInfo.get(am, ty.ty_decl)
                assert (
                    not parent_ani_info.is_class()
                ), f"{field.loc}: cannot extend an @class struct"
                self.sts_parents.append(field)
                self.sts_final_fields.extend(
                    [field, *parts] for parts in parent_ani_info.sts_final_fields
                )
            else:
                self.sts_fields.append(field)
                self.sts_final_fields.append([field])

    def is_class(self):
        return self.sts_type_name == self.sts_impl_name

    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer):
        return self.pkg_ani_info.sts_type_in(pkg, target, self.sts_type_name)


class IfaceANIInfo(AbstractAnalysis[IfaceDecl]):
    def __init__(self, am: AnalysisManager, d: IfaceDecl) -> None:
        super().__init__(am, d)
        p = d.node_parent
        assert p
        segments = [*p.segments, d.name]
        self.from_ani_func_name = encode(segments, DeclKind.FROM_ANI)
        self.into_ani_func_name = encode(segments, DeclKind.INTO_ANI)
        self.decl_header = f"{p.name}.{d.name}.ani.0.h"
        self.impl_header = f"{p.name}.{d.name}.ani.1.h"

        self.pkg_ani_info = PackageANIInfo.get(am, p)
        self.sts_type_name = d.name
        if d.get_attr_item("class"):
            self.sts_impl_name = f"{d.name}"
        else:
            self.sts_impl_name = f"{d.name}_inner"
        self.type_desc = f"L{self.pkg_ani_info.ani_path}/{self.sts_type_name};"
        self.impl_desc = f"L{self.pkg_ani_info.ani_path}/{self.sts_impl_name};"

        self.iface_injected_codes: list[str] = []
        for iface_injected in d.get_attr_list("sts_inject_into_interface"):
            (code,) = iface_injected.args
            self.iface_injected_codes.append(code)
        self.class_injected_codes: list[str] = []
        for class_injected in d.get_attr_list("sts_inject_into_class"):
            (code,) = class_injected.args
            self.class_injected_codes.append(code)

        for parent in d.parents:
            ty = parent.ty_ref.resolved_ty
            assert isinstance(ty, IfaceType)
            parent_ani_info = IfaceANIInfo.get(am, ty.ty_decl)
            assert (
                not parent_ani_info.is_class()
            ), f"{parent.loc}: cannot extend an @class interface"

    def is_class(self):
        return self.sts_type_name == self.sts_impl_name

    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer):
        return self.pkg_ani_info.sts_type_in(pkg, target, self.sts_type_name)


class AbstractTypeANIInfo(metaclass=ABCMeta):
    ani_type: ANIType
    type_desc: str

    def __init__(self, am: AnalysisManager, t: Type):
        self.cpp_info = TypeCppInfo.get(am, t)

    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer) -> str:
        raise NotImplementedError

    def from_ani(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        raise NotImplementedError(f"from class {self.__class__.__name__}")

    def into_ani(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        raise NotImplementedError(f"from class {self.__class__.__name__}")

    def from_ani_array(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        size: str,
        ani_array_value: str,
        cpp_array_buffer: str,
    ):
        if self.ani_type.base == ANI_REF:
            ani_value = f"{cpp_array_buffer}_ani_item"
            cpp_result = f"{cpp_array_buffer}_cpp_item"
            i = f"{cpp_array_buffer}_i"
            target.write(
                f"{' ' * offset}for (size_t {i} = 0; {i} < {size}; {i}++) {{\n"
                f"{' ' * offset}    {self.ani_type} {ani_value};\n"
                f"{' ' * offset}    {env}->Array_Get_Ref({ani_array_value}, {i}, reinterpret_cast<ani_ref*>(&{ani_value}));\n"
            )
            self.from_ani(target, offset + 4, env, ani_value, cpp_result)
            target.write(
                f"{' ' * offset}    new (&{cpp_array_buffer}[{i}]) {self.cpp_info.as_owner}(std::move({cpp_result}));\n"
                f"{' ' * offset}}}\n"
            )
        else:
            target.write(
                f"{' ' * offset}{env}->Array_GetRegion_{self.ani_type.suffix}({ani_array_value}, 0, {size}, reinterpret_cast<{self.ani_type}*>({cpp_array_buffer}));\n"
            )

    def into_ani_array(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        size: str,
        cpp_array_value: str,
        ani_array_result: str,
    ):
        if self.ani_type.base == ANI_REF:
            ani_class = f"{ani_array_result}_cls"
            ani_result = f"{ani_array_result}_ani_item"
            i = f"{ani_array_result}_i"
            target.write(
                f"{' ' * offset}ani_array_ref {ani_array_result};\n"
                f"{' ' * offset}ani_class {ani_class};\n"
                f"{' ' * offset}{env}->FindClass(\"{self.type_desc}\", &{ani_class});\n"
                f"{' ' * offset}ani_ref undefined;\n"
                f"{' ' * offset}{env}->GetUndefined(&undefined);\n"
                f"{' ' * offset}{env}->Array_New_Ref({ani_class}, {size}, undefined, &{ani_array_result});\n"
                f"{' ' * offset}for (size_t {i} = 0; {i} < {size}; {i}++) {{\n"
            )
            self.into_ani(
                target, offset + 4, env, f"{cpp_array_value}[{i}]", ani_result
            )
            target.write(
                f"{' ' * offset}    {env}->Array_Set_Ref({ani_array_result}, {i}, {ani_result});\n"
                f"{' ' * offset}}}\n"
            )
        else:
            target.write(
                f"{' ' * offset}{self.ani_type.array} {ani_array_result};\n"
                f"{' ' * offset}{env}->Array_New_{self.ani_type.suffix}({size}, &{ani_array_result});\n"
                f"{' ' * offset}{env}->Array_SetRegion_{self.ani_type.suffix}({ani_array_result}, 0, {size}, reinterpret_cast<{self.ani_type} const*>({cpp_array_value}));\n"
            )

    @property
    def type_desc_boxed(self) -> str:
        if self.ani_type.base == ANI_REF:
            return self.type_desc
        else:
            return f"Lstd/core/{self.ani_type.suffix};"

    def into_ani_boxed(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        if self.ani_type.base == ANI_REF:
            self.into_ani(target, offset, env, cpp_value, ani_result)
        else:
            ani_class = f"{ani_result}_cls"
            ani_ctor = f"{ani_result}_ctor"
            ani_value = f"{ani_result}_ani"
            target.write(
                f"{' ' * offset}ani_class {ani_class};\n"
                f"{' ' * offset}{env}->FindClass(\"Lstd/core/{self.ani_type.suffix};\", &{ani_class});\n"
                f"{' ' * offset}ani_method {ani_ctor};\n"
                f"{' ' * offset}{env}->Class_FindMethod({ani_class}, \"<ctor>\", \"{self.type_desc}:V\", &{ani_ctor});\n"
                f"{' ' * offset}ani_object {ani_result};\n"
            )
            self.into_ani(target, offset, env, cpp_value, ani_value)
            target.write(
                f"{' ' * offset}{env}->Object_New({ani_class}, {ani_ctor}, &{ani_result}, {ani_value});\n"
            )

    def from_ani_boxed(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        if self.ani_type.base == ANI_REF:
            self.from_ani(
                target,
                offset,
                env,
                f"static_cast<{self.ani_type}>({ani_value})",
                cpp_result,
            )
        else:
            ani_class = f"{cpp_result}_cls"
            ani_getter = f"{cpp_result}_get"
            ani_result = f"{cpp_result}_ani"
            target.write(
                f"{' ' * offset}ani_class {ani_class};\n"
                f"{' ' * offset}{env}->FindClass(\"Lstd/core/{self.ani_type.suffix};\", &{ani_class});\n"
                f"{' ' * offset}ani_method {ani_getter};\n"
                f"{' ' * offset}{env}->Class_FindMethod({ani_class}, \"unboxed\", nullptr, &{ani_getter});\n"
                f"{' ' * offset}{self.ani_type} {ani_result};\n"
                f"{' ' * offset}{env}->Object_CallMethod_{self.ani_type.suffix}((ani_object){ani_value}, {ani_getter}, &{ani_result});\n"
            )
            self.from_ani(target, offset, env, ani_result, cpp_result)


class EnumTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[EnumType]):
    def __init__(self, am: AnalysisManager, t: EnumType):
        super().__init__(am, t)
        enum_ani_info = EnumANIInfo.get(am, t.ty_decl)
        self.ani_type = ANI_ENUM_ITEM
        self.type_desc = enum_ani_info.type_desc
        self.am = am
        self.t = t

    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer) -> str:
        enum_ani_info = EnumANIInfo.get(self.am, self.t.ty_decl)
        return enum_ani_info.sts_type_in(pkg, target)

    @override
    def from_ani(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        ani_index = f"{cpp_result}_ani"
        enum_cpp_info = EnumCppInfo.get(self.am, self.t.ty_decl)
        target.write(
            f"{' ' * offset}ani_size {ani_index};\n"
            f"{' ' * offset}{env}->EnumItem_GetIndex({ani_value}, &{ani_index});\n"
            f"{' ' * offset}{enum_cpp_info.full_name} {cpp_result}(({enum_cpp_info.full_name}::key_t){ani_index});\n"
        )

    @override
    def into_ani(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        cls = f"{ani_result}_cls"
        target.write(
            f"{' ' * offset}ani_enum {cls};\n"
            f"{' ' * offset}{env}->FindEnum(\"{self.type_desc}\", &{cls});\n"
            f"{' ' * offset}ani_enum_item {ani_result};\n"
            f"{' ' * offset}{env}->Enum_GetEnumItemByIndex({cls}, (ani_size){cpp_value}.get_key(), &{ani_result});\n"
        )


class StructTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[StructType]):
    def __init__(self, am: AnalysisManager, t: StructType):
        super().__init__(am, t)
        struct_ani_info = StructANIInfo.get(am, t.ty_decl)
        self.ani_type = ANI_OBJECT
        self.type_desc = struct_ani_info.type_desc
        self.am = am
        self.t = t

    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer) -> str:
        struct_ani_info = StructANIInfo.get(self.am, self.t.ty_decl)
        return struct_ani_info.sts_type_in(pkg, target)

    @override
    def from_ani(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        struct_ani_info = StructANIInfo.get(self.am, self.t.ty_decl)
        target.include(struct_ani_info.impl_header)
        target.write(
            f"{' ' * offset}{self.cpp_info.as_owner} {cpp_result} = {struct_ani_info.from_ani_func_name}({env}, {ani_value});\n"
        )

    @override
    def into_ani(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        struct_ani_info = StructANIInfo.get(self.am, self.t.ty_decl)
        target.include(struct_ani_info.impl_header)
        target.write(
            f"{' ' * offset}ani_object {ani_result} = {struct_ani_info.into_ani_func_name}({env}, {cpp_value});\n"
        )


class UnionTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[UnionType]):
    def __init__(self, am: AnalysisManager, t: UnionType):
        super().__init__(am, t)
        union_ani_info = UnionANIInfo.get(am, t.ty_decl)
        self.ani_type = ANI_REF
        self.type_desc = union_ani_info.type_desc
        self.am = am
        self.t = t

    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer) -> str:
        union_ani_info = UnionANIInfo.get(self.am, self.t.ty_decl)
        return union_ani_info.sts_type_in(pkg, target)

    @override
    def from_ani(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        union_ani_info = UnionANIInfo.get(self.am, self.t.ty_decl)
        target.include(union_ani_info.impl_header)
        target.write(
            f"{' ' * offset}{self.cpp_info.as_owner} {cpp_result} = {union_ani_info.from_ani_func_name}({env}, {ani_value});\n"
        )

    @override
    def into_ani(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        union_ani_info = UnionANIInfo.get(self.am, self.t.ty_decl)
        target.include(union_ani_info.impl_header)
        target.write(
            f"{' ' * offset}ani_ref {ani_result} = {union_ani_info.into_ani_func_name}({env}, {cpp_value});\n"
        )


class IfaceTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[IfaceType]):
    def __init__(self, am: AnalysisManager, t: IfaceType):
        super().__init__(am, t)
        iface_ani_info = IfaceANIInfo.get(am, t.ty_decl)
        self.ani_type = ANI_OBJECT
        self.type_desc = iface_ani_info.type_desc
        self.am = am
        self.t = t

    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer) -> str:
        iface_ani_info = IfaceANIInfo.get(self.am, self.t.ty_decl)
        return iface_ani_info.sts_type_in(pkg, target)

    @override
    def from_ani(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        iface_ani_info = IfaceANIInfo.get(self.am, self.t.ty_decl)
        target.include(iface_ani_info.impl_header)
        target.write(
            f"{' ' * offset}{self.cpp_info.as_owner} {cpp_result} = {iface_ani_info.from_ani_func_name}({env}, {ani_value});\n"
        )

    @override
    def into_ani(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        iface_ani_info = IfaceANIInfo.get(self.am, self.t.ty_decl)
        target.include(iface_ani_info.impl_header)
        target.write(
            f"{' ' * offset}ani_object {ani_result} = {iface_ani_info.into_ani_func_name}({env}, {cpp_value});\n"
        )


class ScalarTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[ScalarType]):
    def __init__(self, am: AnalysisManager, t: ScalarType):
        super().__init__(am, t)
        sts_type, ani_type, type_desc = {
            BOOL: ("boolean", ANI_BOOLEAN, "Z"),
            F32: ("float", ANI_FLOAT, "F"),
            F64: ("double", ANI_DOUBLE, "D"),
            I8: ("byte", ANI_BYTE, "B"),
            I16: ("short", ANI_SHORT, "S"),
            I32: ("int", ANI_INT, "I"),
            I64: ("long", ANI_LONG, "J"),
            U8: ("byte", ANI_BYTE, "B"),
            U16: ("short", ANI_SHORT, "S"),
            U32: ("int", ANI_INT, "I"),
            U64: ("long", ANI_LONG, "J"),
        }[t]
        self.sts_type = sts_type
        self.ani_type = ani_type
        self.type_desc = type_desc

    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer) -> str:
        return self.sts_type

    @override
    def from_ani(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        target.write(
            f"{' ' * offset}{self.cpp_info.as_owner} {cpp_result} = ({self.cpp_info.as_owner}){ani_value};\n"
        )

    @override
    def into_ani(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        target.write(
            f"{' ' * offset}{self.ani_type} {ani_result} = ({self.cpp_info.as_owner}){cpp_value};\n"
        )


class OpaqueTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[OpaqueType]):
    def __init__(self, am: AnalysisManager, t: OpaqueType) -> None:
        super().__init__(am, t)
        self.ani_type = ANI_REF
        self.type_desc = "Lstd/core/Object;"

    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer) -> str:
        return "NullishType"

    @override
    def from_ani(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        target.write(
            f"{' ' * offset}{self.cpp_info.as_owner} {cpp_result} = ({self.cpp_info.as_owner}){ani_value};\n"
        )

    @override
    def into_ani(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        target.write(
            f"{' ' * offset}{self.ani_type} {ani_result} = ({self.ani_type}){cpp_value};\n"
        )


class StringTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[StringType]):
    def __init__(self, am: AnalysisManager, t: StringType):
        super().__init__(am, t)
        self.ani_type = ANI_STRING
        self.type_desc = "Lstd/core/String;"

    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer) -> str:
        return "string"

    @override
    def from_ani(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        length = f"{cpp_result}_len"
        tstr = f"{cpp_result}_tstr"
        buffer = f"{cpp_result}_buf"
        target.write(
            f"{' ' * offset}ani_size {length};\n"
            f"{' ' * offset}{env}->String_GetUTF8Size({ani_value}, &{length});\n"
            f"{' ' * offset}TString {tstr};\n"
            f"{' ' * offset}char* {buffer} = tstr_initialize(&{tstr}, {length} + 1);\n"
            f"{' ' * offset}{env}->String_GetUTF8({ani_value}, {buffer}, {length} + 1, &{length});\n"
            f"{' ' * offset}{buffer}[{length}] = '\\0';\n"
            f"{' ' * offset}{tstr}.length = {length};\n"
            f"{' ' * offset}taihe::core::string {cpp_result} = taihe::core::string({tstr});\n"
        )

    @override
    def into_ani(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        target.write(
            f"{' ' * offset}ani_string {ani_result};\n"
            f"{' ' * offset}{env}->String_NewUTF8({cpp_value}.c_str(), {cpp_value}.size(), &{ani_result});\n"
        )


class ArrayTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[ArrayType]):
    def __init__(self, am: AnalysisManager, t: ArrayType) -> None:
        super().__init__(am, t)
        item_ty_ani_info = TypeANIInfo.get(am, t.item_ty)
        self.ani_type = item_ty_ani_info.ani_type.array
        self.type_desc = f"[{item_ty_ani_info.type_desc}"
        self.am = am
        self.t = t

    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer) -> str:
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        sts_type = item_ty_ani_info.sts_type_in(pkg, target)
        return f"({sts_type}[])"

    @override
    def from_ani(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        cpp_info = TypeCppInfo.get(self.am, self.t)
        size = f"{cpp_result}_size"
        buffer = f"{cpp_result}_buffer"
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        target.write(
            f"{' ' * offset}size_t {size};\n"
            f"{' ' * offset}{env}->Array_GetLength({ani_value}, &{size});\n"
            f"{' ' * offset}{item_ty_cpp_info.as_owner}* {buffer} = ({item_ty_cpp_info.as_owner}*)malloc({size} * sizeof({item_ty_cpp_info.as_owner}));\n"
        )
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        item_ty_ani_info.from_ani_array(target, offset, env, size, ani_value, buffer)
        target.write(
            f"{' ' * offset}{cpp_info.as_owner} {cpp_result}({buffer}, {size});\n"
        )

    @override
    def into_ani(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        size = f"{ani_result}_size"
        target.write(f"{' ' * offset}size_t {size} = {cpp_value}.size();\n")
        item_ty_ani_info.into_ani_array(
            target, offset, env, size, f"{cpp_value}.data()", ani_result
        )


class ArrayBufferTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[ArrayType]):
    def __init__(self, am: AnalysisManager, t: ArrayType) -> None:
        super().__init__(am, t)
        self.ani_type = ANI_ARRAYBUFFER
        self.type_desc = "Lescompat/ArrayBuffer;"

    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer) -> str:
        return "ArrayBuffer"

    @override
    def from_ani(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        data_ptr = f"{cpp_result}_data"
        length = f"{cpp_result}_length"
        target.write(
            f"{' ' * offset}void* {data_ptr} = nullptr;\n"
            f"{' ' * offset}size_t {length} = 0;\n"
            f"{' ' * offset}{env}->ArrayBuffer_GetInfo(reinterpret_cast<ani_arraybuffer>({ani_value}), &{data_ptr}, &{length});\n"
        )
        target.write(
            f"{' ' * offset}taihe::core::array_view<uint8_t> {cpp_result}(reinterpret_cast<uint8_t*>({data_ptr}), {length});\n"
        )

    @override
    def into_ani(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        data_ptr = f"{ani_result}_data"
        target.write(
            f"{' ' * offset}void* {data_ptr} = nullptr;\n"
            f"{' ' * offset}ani_arraybuffer {ani_result};\n"
            f"{' ' * offset}{env}->CreateArrayBuffer({cpp_value}.size(), &{data_ptr}, &{ani_result});\n"
        )
        target.write(
            f"{' ' * offset}memcpy({data_ptr}, {cpp_value}.data(), {cpp_value}.size());\n"
        )


class OptionalTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[OptionalType]):
    def __init__(self, am: AnalysisManager, t: OptionalType) -> None:
        super().__init__(am, t)
        item_ty_ani_info = TypeANIInfo.get(am, t.item_ty)
        self.ani_type = ANI_REF
        self.type_desc = item_ty_ani_info.type_desc_boxed
        self.am = am
        self.t = t

    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer) -> str:
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        sts_type = item_ty_ani_info.sts_type_in(pkg, target)
        return f"({sts_type} | undefined)"

    @override
    def from_ani(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        ani_is_undefined = f"{cpp_result}_flag"
        cpp_pointer = f"{cpp_result}_ptr"
        cpp_spec = f"{cpp_result}_spec"
        cpp_info = TypeCppInfo.get(self.am, self.t)
        item_ty_cpp_info = TypeCppInfo.get(self.am, self.t.item_ty)
        target.write(
            f"{' ' * offset}ani_boolean {ani_is_undefined};\n"
            f"{' ' * offset}{item_ty_cpp_info.as_owner}* {cpp_pointer} = nullptr;\n"
            f"{' ' * offset}{env}->Reference_IsUndefined({ani_value}, &{ani_is_undefined});\n"
            f"{' ' * offset}if (!{ani_is_undefined}) {{\n"
        )
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        item_ty_ani_info.from_ani_boxed(target, offset + 4, env, ani_value, cpp_spec)
        target.write(
            f"{' ' * offset}    {cpp_pointer} = new {item_ty_cpp_info.as_owner}(std::move({cpp_spec}));\n"
            f"{' ' * offset}}};\n"
            f"{' ' * offset}{cpp_info.as_owner} {cpp_result}({cpp_pointer});\n"
        )

    @override
    def into_ani(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        ani_result_spec = f"{ani_result}_spec"
        target.write(
            f"{' ' * offset}ani_ref {ani_result};\n"
            f"{' ' * offset}if (!{cpp_value}) {{\n"
            f"{' ' * offset}    {env}->GetUndefined(&{ani_result});\n"
            f"{' ' * offset}}} else {{\n"
        )
        item_ty_ani_info = TypeANIInfo.get(self.am, self.t.item_ty)
        item_ty_ani_info.into_ani_boxed(
            target, offset + 4, env, f"(*{cpp_value})", ani_result_spec
        )
        target.write(
            f"{' ' * offset}    {ani_result} = {ani_result_spec};\n"
            f"{' ' * offset}}}\n"
        )


class MapTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[MapType]):
    def __init__(self, am: AnalysisManager, t: MapType) -> None:
        super().__init__(am, t)
        self.ani_type = ANI_OBJECT
        self.type_desc = "Lescompat/Record;"
        self.am = am
        self.t = t

    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer) -> str:
        key_ty_ani_info = TypeANIInfo.get(self.am, self.t.key_ty)
        val_ty_ani_info = TypeANIInfo.get(self.am, self.t.val_ty)
        key_sts_type = key_ty_ani_info.sts_type_in(pkg, target)
        val_sts_type = val_ty_ani_info.sts_type_in(pkg, target)
        return f"Record<{key_sts_type}, {val_sts_type}>"

    @override
    def from_ani(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        cpp_info = TypeCppInfo.get(self.am, self.t)
        ani_iter = f"{cpp_result}_ani_iter"
        ani_item = f"{cpp_result}_ani_item"
        ani_key = f"{cpp_result}_ani_key"
        ani_val = f"{cpp_result}_ani_val"
        cpp_key = f"{cpp_result}_cpp_key"
        cpp_val = f"{cpp_result}_cpp_val"
        key_ty_ani_info = TypeANIInfo.get(self.am, self.t.key_ty)
        val_ty_ani_info = TypeANIInfo.get(self.am, self.t.val_ty)
        target.write(
            f"{' ' * offset}ani_ref {ani_iter};\n"
            f"{' ' * offset}{env}->Object_CallMethodByName_Ref({ani_value}, \"$_iterator\", nullptr, &{ani_iter});\n"
            f"{' ' * offset}{cpp_info.as_owner} {cpp_result};\n"
            f"{' ' * offset}while (true) {{\n"
            f"{' ' * offset}    ani_ref next;\n"
            f"{' ' * offset}    ani_boolean done;\n"
            f"{' ' * offset}    {env}->Object_CallMethodByName_Ref(static_cast<ani_object>({ani_iter}), \"next\", nullptr, &next);\n"
            f"{' ' * offset}    {env}->Object_GetFieldByName_Boolean(static_cast<ani_object>(next), \"done\", &done);\n"
            f"{' ' * offset}    if (done) {{;\n"
            f"{' ' * offset}        break;\n"
            f"{' ' * offset}    }};\n"
            f"{' ' * offset}    ani_ref {ani_item};\n"
            f"{' ' * offset}    {env}->Object_GetFieldByName_Ref(static_cast<ani_object>(next), \"value\", &{ani_item});\n"
            f"{' ' * offset}    ani_ref {ani_key};\n"
            f"{' ' * offset}    {env}->TupleValue_GetItem_Ref(static_cast<ani_tuple_value>({ani_item}), 0, &{ani_key});\n"
            f"{' ' * offset}    ani_ref {ani_val};\n"
            f"{' ' * offset}    {env}->TupleValue_GetItem_Ref(static_cast<ani_tuple_value>({ani_item}), 1, &{ani_val});\n"
        )
        key_ty_ani_info.from_ani_boxed(target, offset + 4, env, ani_key, cpp_key)
        val_ty_ani_info.from_ani_boxed(target, offset + 4, env, ani_val, cpp_val)
        target.write(
            f"{' ' * offset}    {cpp_result}.emplace({cpp_key}, {cpp_val});\n"
            f"{' ' * offset}}}\n"
        )

    @override
    def into_ani(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        key_ty_ani_info = TypeANIInfo.get(self.am, self.t.key_ty)
        val_ty_ani_info = TypeANIInfo.get(self.am, self.t.val_ty)
        ani_class = f"{ani_result}_class"
        ani_method = f"{ani_result}_method"
        cpp_key = f"{ani_result}_cpp_key"
        cpp_val = f"{ani_result}_cpp_val"
        ani_key = f"{ani_result}_ani_key"
        ani_val = f"{ani_result}_ani_val"
        target.write(
            f"{' ' * offset}ani_class {ani_class};\n"
            f"{' ' * offset}{env}->FindClass(\"{self.type_desc}\", &{ani_class});\n"
            f"{' ' * offset}ani_method {ani_method};\n"
            f"{' ' * offset}{env}->Class_FindMethod({ani_class}, \"<ctor>\", nullptr, &{ani_method});\n"
            f"{' ' * offset}ani_object {ani_result};\n"
            f"{' ' * offset}{env}->Object_New({ani_class}, {ani_method}, &{ani_result});\n"
            f"{' ' * offset}for (const auto& [{cpp_key}, {cpp_val}] : {cpp_value}) {{\n"
        )
        key_ty_ani_info.into_ani_boxed(target, offset + 4, env, cpp_key, ani_key)
        val_ty_ani_info.into_ani_boxed(target, offset + 4, env, cpp_val, ani_val)
        target.write(
            f"{' ' * offset}    env->Object_CallMethodByName_Void({ani_result}, \"$_set\", nullptr, {ani_key}, {ani_val});\n"
            f"{' ' * offset}}}\n"
        )


class CallbackTypeANIInfo(AbstractTypeANIInfo, AbstractAnalysis[CallbackType]):
    def __init__(self, am: AnalysisManager, t: CallbackType) -> None:
        super().__init__(am, t)
        self.ani_type = ANI_OBJECT
        self.type_desc = f"Lstd/core/Function{len(t.params_ty)};"
        self.am = am
        self.t = t

    def sts_type_in(self, pkg: PackageDecl, target: STSOutputBuffer) -> str:
        params_ty_sts = []
        for index, param_ty in enumerate(self.t.params_ty):
            param_ty_sts_info = TypeANIInfo.get(self.am, param_ty)
            prm_sts_type = param_ty_sts_info.sts_type_in(pkg, target)
            params_ty_sts.append(f"arg_{index}: {prm_sts_type}")
        params_ty_sts_str = ", ".join(params_ty_sts)
        if self.t.return_ty:
            return_ty_sts_info = TypeANIInfo.get(self.am, self.t.return_ty)
            ret_sts_type = return_ty_sts_info.sts_type_in(pkg, target)
            return_ty_sts = ret_sts_type
        else:
            return_ty_sts = "void"
        return f"(({params_ty_sts_str}) => {return_ty_sts})"

    @override
    def from_ani(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        ani_value: str,
        cpp_result: str,
    ):
        cb_cpp_info = CallbackTypeCppInfo.get(self.am, self.t)
        params_ty_as_param = []
        for index, param_ty in enumerate(self.t.params_ty):
            cpp_arg = f"cpp_arg_{index}"
            param_ty_cpp_info = TypeCppInfo.get(self.am, param_ty)
            params_ty_as_param.append(f"{param_ty_cpp_info.as_param} {cpp_arg}")
        params_str = ", ".join(params_ty_as_param)
        if self.t.return_ty:
            return_ty_cpp_info = TypeCppInfo.get(self.am, self.t.return_ty)
            return_ty_as_owner = return_ty_cpp_info.as_owner
        else:
            return_ty_as_owner = "void"
        cpp_impl_cb_name = f"{cpp_result}_cpp_impl_cb"
        target.write(
            f"{' ' * offset}struct {cpp_impl_cb_name} {{\n"
            f"{' ' * offset}    ani_env* env;\n"
            f"{' ' * offset}    ani_object ref;\n"
            f"{' ' * offset}    {cpp_impl_cb_name}(ani_env* env, ani_object obj): env(env) {{\n"
            f"{' ' * offset}        env->GlobalReference_Create(obj, reinterpret_cast<ani_ref*>(&ref));\n"
            f"{' ' * offset}    }}\n"
            f"{' ' * offset}    ~{cpp_impl_cb_name}() {{\n"
            f"{' ' * offset}        env->GlobalReference_Delete(ref);\n"
            f"{' ' * offset}    }}\n"
            f"{' ' * offset}    {return_ty_as_owner} operator()({params_str}) {{\n"
        )
        cpp_args_boxed = []
        for index, param_ty in enumerate(self.t.params_ty):
            param_ty_ani_info = TypeANIInfo.get(self.am, param_ty)
            cpp_arg = f"cpp_arg_{index}"
            ani_arg = f"ani_arg_{index}"
            param_ty_ani_info.into_ani_boxed(target, offset + 8, env, cpp_arg, ani_arg)
            cpp_args_boxed.append(ani_arg)
        cpp_args_boxed_str = ", ".join(cpp_args_boxed)
        target.write(
            f"{' ' * offset}        ani_ref ani_argv[] = {{{cpp_args_boxed_str}}};\n"
            f"{' ' * offset}        ani_ref ani_result;\n"
            f"{' ' * offset}        this->env->FunctionalObject_Call(static_cast<ani_fn_object>(this->ref), {len(self.t.params_ty)}, ani_argv, &ani_result);\n"
        )
        if self.t.return_ty:
            return_ty_ani_info = TypeANIInfo.get(self.am, self.t.return_ty)
            return_ty_ani_info.from_ani_boxed(
                target, offset + 8, "env", "ani_result", "cpp_result"
            )
            target.write(f"{' ' * offset}        return cpp_result;\n")
        else:
            target.write(f"{' ' * offset}        return;\n")
        target.write(
            f"{' ' * offset}    }}\n"
            f"{' ' * offset}}};\n"
            f"{' ' * offset}auto {cpp_result} = {cb_cpp_info.as_owner}::from<{cpp_impl_cb_name}>({env}, {ani_value});\n"
        )

    @override
    def into_ani(
        self,
        target: COutputBuffer,
        offset: int,
        env: str,
        cpp_value: str,
        ani_result: str,
    ):
        # TODO: Callback into ani
        target.write(f"{' ' * offset}ani_object {ani_result};\n")


class TypeANIInfo(TypeVisitor[AbstractTypeANIInfo]):
    def __init__(self, am: AnalysisManager):
        self.am = am

    @staticmethod
    def get(am: AnalysisManager, t: Type | None) -> AbstractTypeANIInfo:
        assert t is not None
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
        if t.item_ty == U8:
            return ArrayBufferTypeANIInfo.get(self.am, t)
        else:
            return ArrayTypeANIInfo.get(self.am, t)

    @override
    def visit_optional_type(self, t: OptionalType) -> AbstractTypeANIInfo:
        return OptionalTypeANIInfo.get(self.am, t)

    @override
    def visit_opaque_type(self, t: OpaqueType) -> AbstractTypeANIInfo:
        return OpaqueTypeANIInfo.get(self.am, t)

    @override
    def visit_map_type(self, t: MapType) -> AbstractTypeANIInfo:
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
        constructor_target.write(
            f"ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result) {{\n"
            f"    ani_env *env;\n"
            f"    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {{\n"
            f"        return ANI_ERROR;\n"
            f"    }}\n"
        )
        for pkg in pg.packages:
            pkg_ani_info = PackageANIInfo.get(self.am, pkg)
            constructor_target.include(pkg_ani_info.header)
            constructor_target.write(
                f"    if (ANI_OK != {pkg_ani_info.cpp_ns}::ANIRegister(env)) {{\n"
                f'        std::cerr << "{pkg_ani_info.cpp_ns}" << std::endl;\n'
                f"        return ANI_ERROR;\n"
                f"    }}\n"
            )
        constructor_target.write(
            f"    *result = ANI_VERSION_1;\n" f"    return ANI_OK;\n" f"}}\n"
        )

    def gen_package(self, pkg: PackageDecl):
        for iface in pkg.interfaces:
            self.gen_iface_file(iface)
        for struct in pkg.structs:
            self.gen_struct_file(struct)
        for union in pkg.unions:
            self.gen_union_file(union)
        pkg_ani_info = PackageANIInfo.get(self.am, pkg)
        pkg_cpp_info = PackageCppInfo.get(self.am, pkg)
        self.gen_package_header(pkg, pkg_ani_info, pkg_cpp_info)
        self.gen_package_source(pkg, pkg_ani_info, pkg_cpp_info)

    def gen_package_header(
        self,
        pkg: PackageDecl,
        pkg_ani_info: PackageANIInfo,
        pkg_cpp_info: PackageCppInfo,
    ):
        pkg_ani_header_target = COutputBuffer.create(
            self.tm, f"include/{pkg_ani_info.header}", True
        )
        pkg_ani_header_target.include("core/runtime.hpp")
        pkg_ani_header_target.write(
            f"namespace {pkg_ani_info.cpp_ns} {{\n"
            f"ani_status ANIRegister(ani_env *env);\n"
            f"}}\n"
        )

    def gen_package_source(
        self,
        pkg: PackageDecl,
        pkg_ani_info: PackageANIInfo,
        pkg_cpp_info: PackageCppInfo,
    ):
        pkg_ani_source_target = COutputBuffer.create(
            self.tm, f"src/{pkg_ani_info.source}", False
        )
        pkg_ani_source_target.include(pkg_cpp_info.header)
        pkg_ani_source_target.include(pkg_ani_info.header)
        # generate functions
        for func in pkg.functions:
            self.gen_func(func, pkg_ani_source_target, pkg_ani_info.is_namespace())
        for iface in pkg.interfaces:
            for method in iface.methods:
                self.gen_method(iface, method, pkg_ani_source_target)

            iface_abi_info = IfaceABIInfo.get(self.am, iface)
            for ancestor in iface_abi_info.ancestor_dict:
                if ancestor == iface:
                    continue
                for method in ancestor.methods:
                    p = iface.node_parent
                    assert p
                    segments = [*p.segments, iface.name, method.name]
                    mangled_name = encode(segments, DeclKind.ANI_FUNC)
                    self.gen_method(
                        iface, method, pkg_ani_source_target, ancestor, mangled_name
                    )

        # register infos
        register_infos: list[tuple[str, list[tuple[str, str]], bool]] = []
        impl_desc = pkg_ani_info.impl_desc
        func_infos = []
        for func in pkg.functions:
            glob_func_info = GlobFuncANIInfo.get(self.am, func)
            sts_native_name = glob_func_info.sts_native_name
            mangled_name = glob_func_info.mangled_name
            func_infos.append((sts_native_name, mangled_name))
        register_infos.append((impl_desc, func_infos, pkg_ani_info.is_namespace()))
        for iface in pkg.interfaces:
            iface_ani_info = IfaceANIInfo.get(self.am, iface)
            impl_desc = iface_ani_info.impl_desc
            func_infos = []
            for method in iface.methods:
                method_ani_info = IfaceMethodANIInfo.get(self.am, method)
                sts_native_name = method_ani_info.sts_native_name
                mangled_name = method_ani_info.mangled_name
                func_infos.append((sts_native_name, mangled_name))

            iface_abi_info = IfaceABIInfo.get(self.am, iface)
            for ancestor in iface_abi_info.ancestor_dict:
                if ancestor == iface:
                    continue
                for method in ancestor.methods:
                    method_ani_info = IfaceMethodANIInfo.get(self.am, method)
                    sts_native_name = method_ani_info.sts_native_name
                    p = iface.node_parent
                    assert p
                    segments = [*p.segments, iface.name, method.name]
                    mangled_name = encode(segments, DeclKind.ANI_FUNC)
                    func_infos.append((sts_native_name, mangled_name))

            register_infos.append((impl_desc, func_infos, False))
        pkg_ani_source_target.write(
            f"namespace {pkg_ani_info.cpp_ns} {{\n"
            f"ani_status ANIRegister(ani_env *env) {{\n"
        )
        for impl_desc, func_infos, is_namespace in register_infos:
            if is_namespace:
                ani_type = "ani_namespace"
                ani_find_func = "FindNamespace"
                ani_bind_func = "Namespace_BindNativeFunctions"
            else:
                ani_type = "ani_class"
                ani_find_func = "FindClass"
                ani_bind_func = "Class_BindNativeMethods"
            pkg_ani_source_target.write(
                f"    {{\n"
                f"        {ani_type} ani_env;\n"
                f'        if (ANI_OK != env->{ani_find_func}("{impl_desc}", &ani_env)) {{\n'
                f"            return ANI_ERROR;\n"
                f"        }}\n"
                f"        ani_native_function methods[] = {{\n"
            )
            for sts_native_name, mangled_name in func_infos:
                pkg_ani_source_target.write(
                    f'            {{"{sts_native_name}", nullptr, reinterpret_cast<void*>({mangled_name})}},\n'
                )

            pkg_ani_source_target.write(
                f"        }};\n"
                f"        if (ANI_OK != env->{ani_bind_func}(ani_env, methods, sizeof(methods) / sizeof(ani_native_function))) {{\n"
                f"            return ANI_ERROR;\n"
                f"        }}\n"
                f"    }}\n"
            )
        pkg_ani_source_target.write(f"    return ANI_OK;\n" f"}}\n" f"}}\n")

    def gen_func(
        self,
        func: GlobFuncDecl,
        pkg_ani_source_target: COutputBuffer,
        is_namespace: bool,
    ):
        func_ani_info = GlobFuncANIInfo.get(self.am, func)
        func_cpp_info = GlobFuncCppInfo.get(self.am, func)
        params_ani = []
        params_ani.append("[[maybe_unused]] ani_env *env")
        if not is_namespace:
            params_ani.append("[[maybe_unused]] ani_object object")
        ani_param_names = []
        args_cpp = []
        for param in func.params:
            type_ani_info = TypeANIInfo.get(self.am, param.ty_ref.resolved_ty)
            ani_param_name = f"ani_arg_{param.name}"
            cpp_arg_name = f"cpp_arg_{param.name}"
            params_ani.append(f"{type_ani_info.ani_type} {ani_param_name}")
            ani_param_names.append(ani_param_name)
            args_cpp.append(cpp_arg_name)
        params_ani_str = ", ".join(params_ani)
        if return_ty_ref := func.return_ty_ref:
            type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
            ani_return_ty_name = type_ani_info.ani_type
        else:
            ani_return_ty_name = "void"
        pkg_ani_source_target.write(
            f"static {ani_return_ty_name} {func_ani_info.mangled_name}({params_ani_str}) {{\n"
            f"    taihe::core::set_env(env);\n"
        )
        for param, ani_param_name, cpp_arg_name in zip(
            func.params, ani_param_names, args_cpp, strict=True
        ):
            type_ani_info = TypeANIInfo.get(self.am, param.ty_ref.resolved_ty)
            type_ani_info.from_ani(
                pkg_ani_source_target, 4, "env", ani_param_name, cpp_arg_name
            )
        args_cpp_str = ", ".join(args_cpp)
        if return_ty_ref := func.return_ty_ref:
            type_cpp_info = TypeCppInfo.get(self.am, return_ty_ref.resolved_ty)
            cpp_return_ty_name = type_cpp_info.as_owner
            cpp_result = "cpp_result"
            ani_result = "ani_result"
            pkg_ani_source_target.write(
                f"    {cpp_return_ty_name} {cpp_result} = {func_cpp_info.full_name}({args_cpp_str});\n"
            )
            type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
            type_ani_info.into_ani(
                pkg_ani_source_target, 4, "env", cpp_result, ani_result
            )
            pkg_ani_source_target.write(f"    return {ani_result};\n")
        else:
            pkg_ani_source_target.write(
                f"    {func_cpp_info.full_name}({args_cpp_str});\n" f"    return;\n"
            )
        pkg_ani_source_target.write(f"}}\n")

    def gen_method(
        self,
        iface: IfaceDecl,
        method: IfaceMethodDecl,
        pkg_ani_source_target: COutputBuffer,
        ancestor: IfaceDecl | None = None,
        mangled_name: str | None = None,
    ):
        method_ani_info = IfaceMethodANIInfo.get(self.am, method)
        method_cpp_info = IfaceMethodCppInfo.get(self.am, method)
        iface_cpp_info = IfaceCppInfo.get(self.am, iface)
        iface_abi_info = IfaceABIInfo.get(self.am, iface)
        params_ani = []
        params_ani.append("[[maybe_unused]] ani_env *env")
        params_ani.append("[[maybe_unused]] ani_object object")
        ani_param_names = []
        args_cpp = []
        for param in method.params:
            type_ani_info = TypeANIInfo.get(self.am, param.ty_ref.resolved_ty)
            ani_param_name = f"ani_arg_{param.name}"
            cpp_arg_name = f"cpp_arg_{param.name}"
            params_ani.append(f"{type_ani_info.ani_type} {ani_param_name}")
            ani_param_names.append(ani_param_name)
            args_cpp.append(cpp_arg_name)
        params_ani_str = ", ".join(params_ani)
        if return_ty_ref := method.return_ty_ref:
            type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
            ani_return_ty_name = type_ani_info.ani_type
        else:
            ani_return_ty_name = "void"
        pkg_ani_source_target.write(
            f"static {ani_return_ty_name} {mangled_name or method_ani_info.mangled_name}({params_ani_str}) {{\n"
            f"    taihe::core::set_env(env);\n"
        )
        for param, ani_param_name, cpp_arg_name in zip(
            method.params, ani_param_names, args_cpp, strict=False
        ):
            type_ani_info = TypeANIInfo.get(self.am, param.ty_ref.resolved_ty)
            type_ani_info.from_ani(
                pkg_ani_source_target, 4, "env", ani_param_name, cpp_arg_name
            )
        args_cpp_str = ", ".join(args_cpp)
        pkg_ani_source_target.write(
            f"    ani_long ani_data_ptr;\n"
            f'    env->Object_GetPropertyByName_Long(object, "_data_ptr", reinterpret_cast<ani_long*>(&ani_data_ptr));\n'
            f"    ani_long ani_vtbl_ptr;\n"
            f'    env->Object_GetPropertyByName_Long(object, "_vtbl_ptr", reinterpret_cast<ani_long*>(&ani_vtbl_ptr));\n'
            f"    DataBlockHead* cpp_data_ptr = reinterpret_cast<DataBlockHead*>(ani_data_ptr);\n"
            f"    {iface_abi_info.vtable}* cpp_vtbl_ptr = reinterpret_cast<{iface_abi_info.vtable}*>(ani_vtbl_ptr);\n"
            f"    {iface_cpp_info.full_weak_name} cpp_iface = {iface_cpp_info.full_weak_name}({{cpp_vtbl_ptr, cpp_data_ptr}});\n"
        )
        if ancestor:
            ancestor_cpp_info = IfaceCppInfo.get(self.am, ancestor)
            cpp_iface_str = f"{ancestor_cpp_info.as_param}(cpp_iface)"
        else:
            cpp_iface_str = "cpp_iface"
        if return_ty_ref := method.return_ty_ref:
            type_cpp_info = TypeCppInfo.get(self.am, return_ty_ref.resolved_ty)
            cpp_return_ty_name = type_cpp_info.as_owner
            cpp_result = "cpp_result"
            ani_result = "ani_result"
            pkg_ani_source_target.write(
                f"    {cpp_return_ty_name} {cpp_result} = {cpp_iface_str}->{method_cpp_info.call_name}({args_cpp_str});\n"
            )
            type_ani_info = TypeANIInfo.get(self.am, return_ty_ref.resolved_ty)
            type_ani_info.into_ani(
                pkg_ani_source_target, 4, "env", cpp_result, ani_result
            )
            pkg_ani_source_target.write(f"    return {ani_result};\n")
        else:
            pkg_ani_source_target.write(
                f"    {cpp_iface_str}->{method_cpp_info.call_name}({args_cpp_str});\n"
                f"    return;\n"
            )
        pkg_ani_source_target.write(f"}}\n")

    def gen_iface_file(
        self,
        iface: IfaceDecl,
    ):
        iface_abi_info = IfaceABIInfo.get(self.am, iface)
        iface_cpp_info = IfaceCppInfo.get(self.am, iface)
        iface_ani_info = IfaceANIInfo.get(self.am, iface)
        # declaration
        iface_ani_decl_target = COutputBuffer.create(
            self.tm, f"include/{iface_ani_info.decl_header}", True
        )
        iface_ani_decl_target.include("ani.h")
        iface_ani_decl_target.include(iface_cpp_info.decl_header)
        iface_ani_decl_target.write(
            f"{iface_cpp_info.as_owner} {iface_ani_info.from_ani_func_name}(ani_env* env, ani_object ani_obj);\n"
            f"ani_object {iface_ani_info.into_ani_func_name}(ani_env* env, {iface_cpp_info.as_owner} cpp_obj);\n"
        )
        # implementation
        iface_ani_impl_target = COutputBuffer.create(
            self.tm, f"include/{iface_ani_info.impl_header}", True
        )
        iface_ani_impl_target.include(iface_ani_info.decl_header)
        iface_ani_impl_target.include(iface_cpp_info.impl_header)
        # from ani
        iface_ani_impl_target.write(
            f"inline {iface_cpp_info.as_owner} {iface_ani_info.from_ani_func_name}(ani_env* env, ani_object ani_obj) {{\n"
            f"    struct cpp_impl_t {{\n"
            f"        ani_env* env;\n"
            f"        ani_object ref;\n"
            f"        cpp_impl_t(ani_env* env, ani_object obj) : env(env) {{\n"
            f"            env->GlobalReference_Create(obj, reinterpret_cast<ani_ref*>(&ref));\n"
            f"        }}\n"
            f"        ~cpp_impl_t() {{\n"
            f"            env->GlobalReference_Delete(ref);\n"
            f"        }}\n"
        )
        for ancestor in iface_abi_info.ancestor_dict:
            for method in ancestor.methods:
                method_cpp_info = IfaceMethodCppInfo.get(self.am, method)
                method_ani_info = IfaceMethodANIInfo.get(self.am, method)
                cpp_param_pairs = []
                cpp_param_names = []
                ani_arg_names = []
                for param in method.params:
                    cpp_param_name = f"cpp_arg_{param.name}"
                    ani_arg_name = f"ani_arg_{param.name}"
                    type_cpp_info = TypeCppInfo.get(self.am, param.ty_ref.resolved_ty)
                    cpp_param_pairs.append(f"{type_cpp_info.as_param} {cpp_param_name}")
                    cpp_param_names.append(cpp_param_name)
                    ani_arg_names.append(ani_arg_name)
                params_cpp_str = ", ".join(cpp_param_pairs)
                if method.return_ty_ref:
                    type_cpp_info = TypeCppInfo.get(
                        self.am, method.return_ty_ref.resolved_ty
                    )
                    cpp_return_ty_name = type_cpp_info.as_owner
                else:
                    cpp_return_ty_name = "void"
                iface_ani_impl_target.write(
                    f"        {cpp_return_ty_name} {method_cpp_info.impl_name}({params_cpp_str}) {{\n"
                )
                args_ani = []
                for param, cpp_param_name, ani_arg_name in zip(
                    method.params, cpp_param_names, ani_arg_names, strict=False
                ):
                    type_ani_info = TypeANIInfo.get(self.am, param.ty_ref.resolved_ty)
                    type_ani_info.into_ani(
                        iface_ani_impl_target, 8, "env", cpp_param_name, ani_arg_name
                    )
                    args_ani.append(ani_arg_name)
                args_ani_trailing = "".join(", " + arg_ani for arg_ani in args_ani)
                ani_result = "ani_result"
                cpp_result = "cpp_result"
                if method.return_ty_ref:
                    type_ani_info = TypeANIInfo.get(
                        self.am, method.return_ty_ref.resolved_ty
                    )
                    iface_ani_impl_target.write(
                        f"            {type_ani_info.ani_type} {ani_result};\n"
                        f'            this->env->Object_CallMethodByName_{type_ani_info.ani_type.suffix}(this->ref, "{method_ani_info.ani_method_name}", nullptr, reinterpret_cast<{type_ani_info.ani_type.base}*>(&{ani_result}){args_ani_trailing});\n'
                    )
                    type_ani_info.from_ani(
                        iface_ani_impl_target, 8, "env", ani_result, cpp_result
                    )
                    iface_ani_impl_target.write(f"            return {cpp_result};\n")
                else:
                    iface_ani_impl_target.write(
                        f'            this->env->Object_CallMethodByName_Void(this->ref, "{method_ani_info.ani_method_name}", nullptr{args_ani_trailing});\n'
                    )
                iface_ani_impl_target.write(f"        }}\n")
        iface_ani_impl_target.write(
            f"    }};\n"
            f"    return taihe::core::make_holder<cpp_impl_t, {iface_cpp_info.as_owner}>(env, ani_obj);\n"
            f"}}\n"
        )
        # into ani
        iface_ani_impl_target.write(
            f"inline ani_object {iface_ani_info.into_ani_func_name}(ani_env* env, {iface_cpp_info.as_owner} cpp_obj) {{\n"
            f"    ani_long ani_vtbl_ptr = reinterpret_cast<ani_long>(cpp_obj.m_handle.vtbl_ptr);\n"
            f"    ani_long ani_data_ptr = reinterpret_cast<ani_long>(cpp_obj.m_handle.data_ptr);\n"
            f"    cpp_obj.m_handle.data_ptr = nullptr;\n"
            f"    ani_class ani_obj_cls;\n"
            f'    env->FindClass("{iface_ani_info.impl_desc}", &ani_obj_cls);\n'
            f"    ani_method ani_obj_ctor;\n"
            f'    env->Class_FindMethod(ani_obj_cls, "<ctor>", nullptr, &ani_obj_ctor);\n'
            f"    ani_object ani_obj;\n"
            f"    env->Object_New(ani_obj_cls, ani_obj_ctor, &ani_obj, ani_vtbl_ptr, ani_data_ptr);\n"
            f"    return ani_obj;\n"
            f"}}\n"
        )

    def gen_struct_file(
        self,
        struct: StructDecl,
    ):
        struct_cpp_info = StructCppInfo.get(self.am, struct)
        struct_ani_info = StructANIInfo.get(self.am, struct)
        # declaration
        struct_ani_decl_target = COutputBuffer.create(
            self.tm, f"include/{struct_ani_info.decl_header}", True
        )
        struct_ani_decl_target.include("ani.h")
        struct_ani_decl_target.include(struct_cpp_info.decl_header)
        struct_ani_decl_target.write(
            f"{struct_cpp_info.as_owner} {struct_ani_info.from_ani_func_name}(ani_env* env, ani_object ani_obj);\n"
            f"ani_object {struct_ani_info.into_ani_func_name}(ani_env* env, {struct_cpp_info.as_param} cpp_obj);\n"
        )
        # implementation
        struct_ani_impl_target = COutputBuffer.create(
            self.tm, f"include/{struct_ani_info.impl_header}", True
        )
        struct_ani_impl_target.include(struct_ani_info.decl_header)
        struct_ani_impl_target.include(struct_cpp_info.impl_header)
        # from ani
        struct_ani_impl_target.write(
            f"inline {struct_cpp_info.as_owner} {struct_ani_info.from_ani_func_name}(ani_env* env, ani_object ani_obj) {{\n"
        )
        cpp_field_results = []
        for parts in struct_ani_info.sts_final_fields:
            final = parts[-1]
            type_ani_info = TypeANIInfo.get(self.am, final.ty_ref.resolved_ty)
            ani_field_value = f"ani_field_{final.name}"
            cpp_field_result = f"cpp_field_{final.name}"
            struct_ani_impl_target.write(
                f"    {type_ani_info.ani_type} {ani_field_value};\n"
                f'    env->Object_GetPropertyByName_{type_ani_info.ani_type.suffix}(ani_obj, "{final.name}", reinterpret_cast<{type_ani_info.ani_type.base}*>(&{ani_field_value}));\n'
            )
            type_ani_info.from_ani(
                struct_ani_impl_target, 4, "env", ani_field_value, cpp_field_result
            )
            cpp_field_results.append(cpp_field_result)
        cpp_moved_fields_str = ", ".join(
            f"std::move({cpp_field_result})" for cpp_field_result in cpp_field_results
        )
        struct_ani_impl_target.write(
            f"    return {struct_cpp_info.as_owner}{{{cpp_moved_fields_str}}};\n"
            f"}}\n"
        )
        # into ani
        ani_field_results = []
        struct_ani_impl_target.write(
            f"inline ani_object {struct_ani_info.into_ani_func_name}(ani_env* env, {struct_cpp_info.as_param} cpp_obj) {{\n"
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
        struct_ani_impl_target.write(
            f"    ani_class ani_obj_cls;\n"
            f'    env->FindClass("{struct_ani_info.impl_desc}", &ani_obj_cls);\n'
            f"    ani_method ani_obj_ctor;\n"
            f'    env->Class_FindMethod(ani_obj_cls, "<ctor>", nullptr, &ani_obj_ctor);\n'
            f"    ani_object ani_obj;\n"
            f"    env->Object_New(ani_obj_cls, ani_obj_ctor, &ani_obj{ani_field_results_trailing});\n"
            f"    return ani_obj;\n"
            f"}}\n"
        )

    def gen_union_file(
        self,
        union: UnionDecl,
    ):
        union_cpp_info = UnionCppInfo.get(self.am, union)
        union_ani_info = UnionANIInfo.get(self.am, union)
        union_ani_decl_target = COutputBuffer.create(
            self.tm, f"include/{union_ani_info.decl_header}", True
        )
        union_ani_decl_target.include("ani.h")
        union_ani_decl_target.include(union_cpp_info.decl_header)
        union_ani_decl_target.write(
            f"{union_cpp_info.as_owner} {union_ani_info.from_ani_func_name}(ani_env* env, ani_ref ani_obj);\n"
            f"ani_ref {union_ani_info.into_ani_func_name}(ani_env* env, {union_cpp_info.as_param} cpp_obj);\n"
        )
        # implementation
        union_ani_impl_target = COutputBuffer.create(
            self.tm, f"include/{union_ani_info.impl_header}", True
        )
        union_ani_impl_target.include(union_ani_info.decl_header)
        union_ani_impl_target.include(union_cpp_info.impl_header)
        # from ani
        union_ani_impl_target.write(
            f"inline {union_cpp_info.as_owner} {union_ani_info.from_ani_func_name}(ani_env* env, ani_ref ani_value) {{\n"
        )
        # `Reference_IsUndefined` should be called before `Object_InstanceOf`
        for parts in union_ani_info.sts_final_fields:
            final = parts[-1]
            if final.ty_ref is None:
                static_tags = []
                for part in parts:
                    assert part.node_parent
                    path_cpp_info = UnionCppInfo.get(self.am, part.node_parent)
                    static_tags.append(
                        f"taihe::core::static_tag<{path_cpp_info.full_name}::tag_t::{part.name}>"
                    )
                static_tags_str = ", ".join(static_tags)
                full_name = "_".join(part.name for part in parts)
                is_field = f"ani_is_{full_name}"
                union_ani_impl_target.write(
                    f"    ani_boolean {is_field};\n"
                    f"    env->Reference_IsUndefined(ani_value, &{is_field});\n"
                    f"    if ({is_field}) {{\n"
                    f"        return {union_cpp_info.full_name}({static_tags_str});\n"
                    f"    }}\n"
                )
        for parts in union_ani_info.sts_final_fields:
            final = parts[-1]
            if final.ty_ref is not None:
                static_tags = []
                for part in parts:
                    assert part.node_parent
                    path_cpp_info = UnionCppInfo.get(self.am, part.node_parent)
                    static_tags.append(
                        f"taihe::core::static_tag<{path_cpp_info.full_name}::tag_t::{part.name}>"
                    )
                static_tags_str = ", ".join(static_tags)
                type_ani_info = TypeANIInfo.get(
                    self.am, final.ty_ref.resolved_ty  # pyre-ignore
                )
                full_name = "_".join(part.name for part in parts)
                is_field = f"ani_is_{full_name}"
                filed_class = f"ani_cls_{full_name}"
                union_ani_impl_target.write(
                    f"    ani_class {filed_class};\n"
                    f'    env->FindClass("{type_ani_info.type_desc_boxed}", &{filed_class});\n'
                    f"    ani_boolean {is_field};\n"
                    f"    env->Object_InstanceOf((ani_object)ani_value, {filed_class}, &{is_field});\n"
                    f"    if ({is_field}) {{\n"
                )
                cpp_result_spec = f"cpp_field_{full_name}"
                type_ani_info.from_ani_boxed(
                    union_ani_impl_target,
                    8,
                    "env",
                    "ani_value",
                    cpp_result_spec,
                )
                union_ani_impl_target.write(
                    f"        return {union_cpp_info.full_name}({static_tags_str}, std::move({cpp_result_spec}));\n"
                    f"    }}\n"
                )
        union_ani_impl_target.write(f"    __builtin_unreachable();\n" f"}}\n")
        # into ani
        union_ani_impl_target.write(
            f"inline ani_ref {union_ani_info.into_ani_func_name}(ani_env* env, {union_cpp_info.as_param} cpp_value) {{\n"
            f"    ani_ref ani_value;\n"
            f"    switch (cpp_value.get_tag()) {{\n"
        )
        for field in union.fields:
            union_ani_impl_target.write(
                f"    case {union_cpp_info.full_name}::tag_t::{field.name}: {{\n"
            )
            if field.ty_ref is None:
                union_ani_impl_target.write(f"        env->GetUndefined(&ani_value);\n")
            else:
                ani_result_spec = f"ani_field_{field.name}"
                type_ani_info = TypeANIInfo.get(self.am, field.ty_ref.resolved_ty)
                type_ani_info.into_ani_boxed(
                    union_ani_impl_target,
                    8,
                    "env",
                    f"cpp_value.get_{field.name}_ref()",
                    ani_result_spec,
                )
                union_ani_impl_target.write(f"        ani_value = {ani_result_spec};\n")
            union_ani_impl_target.write(f"        break;\n" f"    }}\n")
        union_ani_impl_target.write(f"    }}\n")
        union_ani_impl_target.write(f"    return ani_value;\n" f"}}\n")
