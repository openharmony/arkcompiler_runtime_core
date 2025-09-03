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

import re

from taihe.codegen.abi.analyses import (
    GlobFuncAbiInfo,
    IfaceAbiInfo,
    PackageAbiInfo,
    TypeAbiInfo,
)
from taihe.codegen.abi.writer import CHeaderWriter, CSourceWriter
from taihe.codegen.cpp.analyses import (
    GlobFuncCppImplInfo,
    IfaceMethodCppInfo,
    PackageCppImplInfo,
    PackageCppInfo,
    TypeCppInfo,
)
from taihe.semantics.declarations import (
    GlobFuncDecl,
    IfaceDecl,
    IfaceMethodDecl,
    PackageDecl,
    PackageGroup,
)
from taihe.semantics.types import IfaceType, NonVoidType
from taihe.utils.analyses import AnalysisManager
from taihe.utils.outputs import FileKind, OutputManager


class CppImplHeadersGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager):
        self.om = om
        self.am = am

    def generate(self, pg: PackageGroup):
        for pkg in pg.packages:
            self.gen_package_file(pkg)

    def gen_package_file(self, pkg: PackageDecl):
        pkg_abi_info = PackageAbiInfo.get(self.am, pkg)
        pkg_cpp_impl_info = PackageCppImplInfo.get(self.am, pkg)
        with CHeaderWriter(
            self.om,
            f"include/{pkg_cpp_impl_info.header}",
            FileKind.CPP_HEADER,
        ) as pkg_cpp_impl_target:
            pkg_cpp_impl_target.add_include("taihe/common.hpp")
            pkg_cpp_impl_target.add_include(pkg_abi_info.header)
            for func in pkg.functions:
                for param in func.params:
                    param_ty_cpp_info = TypeCppInfo.get(self.am, param.ty)
                    pkg_cpp_impl_target.add_include(*param_ty_cpp_info.impl_headers)
                if isinstance(return_ty := func.return_ty, NonVoidType):
                    return_ty_cpp_info = TypeCppInfo.get(self.am, return_ty)
                    pkg_cpp_impl_target.add_include(*return_ty_cpp_info.impl_headers)
                self.gen_func(func, pkg_cpp_impl_target)

    def gen_func(
        self,
        func: GlobFuncDecl,
        pkg_cpp_impl_target: CHeaderWriter,
    ):
        func_abi_info = GlobFuncAbiInfo.get(self.am, func)
        func_cpp_impl_info = GlobFuncCppImplInfo.get(self.am, func)
        func_impl = "CPP_FUNC_IMPL"
        args_cpp = []
        params_abi = []
        for param in func.params:
            param_ty_cpp_info = TypeCppInfo.get(self.am, param.ty)
            param_ty_abi_info = TypeAbiInfo.get(self.am, param.ty)
            args_cpp.append(param_ty_cpp_info.pass_from_abi(param.name))
            params_abi.append(f"{param_ty_abi_info.as_param} {param.name}")
        args_cpp_str = ", ".join(args_cpp)
        params_abi_str = ", ".join(params_abi)
        result_cpp = f"{func_impl}({args_cpp_str})"
        if isinstance(return_ty := func.return_ty, NonVoidType):
            return_ty_cpp_info = TypeCppInfo.get(self.am, return_ty)
            return_ty_abi_info = TypeAbiInfo.get(self.am, return_ty)
            return_ty_abi_name = return_ty_abi_info.as_owner
            result_abi = return_ty_cpp_info.return_into_abi(result_cpp)
        else:
            return_ty_abi_name = "void"
            result_abi = result_cpp
        pkg_cpp_impl_target.writelns(
            f"#define {func_cpp_impl_info.macro}({func_impl}) \\",
            f"    {return_ty_abi_name} {func_abi_info.mangled_name}({params_abi_str}) {{ \\",
            f"        return {result_abi}; \\",
            f"    }}",
        )


class CppImplSourcesGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager):
        self.om = om
        self.am = am
        self.using_namespaces: list[str] = []

    @property
    def make_holder(self):
        return self.mask("taihe::make_holder")

    @property
    def runtime_error(self):
        return self.mask("std::runtime_error")

    def mask(self, cpp_type: str):
        pattern = r"(::)?([A-Za-z_][A-Za-z_0-9]*::)*[A-Za-z_][A-Za-z_0-9]*"

        def replace_ns(match):
            matched = match.group(0)
            for ns in self.using_namespaces:
                ns = ns + "::"
                if matched.startswith(ns):
                    return matched[len(ns) :]
                ns = "::" + ns
                if matched.startswith(ns):
                    return matched[len(ns) :]
            return matched

        return re.sub(pattern, replace_ns, cpp_type)

    def generate(self, pg: PackageGroup):
        for pkg in pg.packages:
            self.gen_package_file(pkg)

    def gen_package_file(self, pkg: PackageDecl):
        pkg_cpp_info = PackageCppInfo.get(self.am, pkg)
        pkg_cpp_impl_info = PackageCppImplInfo.get(self.am, pkg)
        with CSourceWriter(
            self.om,
            f"temp/{pkg_cpp_impl_info.source}",
            FileKind.TEMPLATE,
        ) as pkg_cpp_impl_target:
            pkg_cpp_impl_target.add_include(pkg_cpp_info.header)
            pkg_cpp_impl_target.add_include(pkg_cpp_impl_info.header)
            pkg_cpp_impl_target.add_include("taihe/runtime.hpp")
            pkg_cpp_impl_target.add_include("stdexcept")
            pkg_cpp_impl_target.newline()
            self.using_namespaces = []
            pkg_cpp_impl_target.newline()
            self.gen_anonymous_namespace_block(pkg, pkg_cpp_impl_target)
            pkg_cpp_impl_target.newline()
            pkg_cpp_impl_target.writelns(
                "// Since these macros are auto-generate, lint will cause false positive.",
                "// NOLINTBEGIN",
            )
            for func in pkg.functions:
                self.gen_func_macro(func, pkg_cpp_impl_target)
            pkg_cpp_impl_target.writelns(
                "// NOLINTEND",
            )
            self.using_namespaces = []

    def gen_using_namespace(
        self,
        pkg_cpp_impl_target: CSourceWriter,
        namespace: str,
    ):
        pkg_cpp_impl_target.writelns(
            f"using namespace {namespace};",
        )
        self.using_namespaces.append(namespace)

    def gen_anonymous_namespace_block(
        self,
        pkg: PackageDecl,
        pkg_cpp_impl_target: CSourceWriter,
    ):
        with pkg_cpp_impl_target.indented(
            f"namespace {{",
            f"}}  // namespace",
            indent="",
        ):
            pkg_cpp_impl_target.writelns(
                f"// To be implemented.",
            )
            for iface in pkg.interfaces:
                pkg_cpp_impl_target.newline()
                self.gen_iface(iface, pkg_cpp_impl_target)
            for func in pkg.functions:
                pkg_cpp_impl_target.newline()
                self.gen_func_impl(func, pkg_cpp_impl_target)

    def gen_iface(
        self,
        iface: IfaceDecl,
        pkg_cpp_impl_target: CSourceWriter,
    ):
        iface_abi_info = IfaceAbiInfo.get(self.am, iface)
        impl_name = f"{iface.name}Impl"
        with pkg_cpp_impl_target.indented(
            f"class {impl_name} {{",
            f"}};",
        ):
            pkg_cpp_impl_target.writelns(
                f"public:",
            )
            with pkg_cpp_impl_target.indented(
                f"{impl_name}() {{",
                f"}}",
            ):
                pkg_cpp_impl_target.writelns(
                    f"// Don't forget to implement the constructor.",
                )
            for ancestor in iface_abi_info.ancestor_dict:
                for func in ancestor.methods:
                    pkg_cpp_impl_target.newline()
                    self.gen_method_impl(func, pkg_cpp_impl_target)

    def gen_method_impl(
        self,
        func: IfaceMethodDecl,
        pkg_cpp_impl_target: CSourceWriter,
    ):
        method_cpp_info = IfaceMethodCppInfo.get(self.am, func)
        func_cpp_impl_name = method_cpp_info.impl_name
        params_cpp = []
        for param in func.params:
            param_ty_cpp_info = TypeCppInfo.get(self.am, param.ty)
            params_cpp.append(f"{self.mask(param_ty_cpp_info.as_param)} {param.name}")
        params_cpp_str = ", ".join(params_cpp)
        if isinstance(return_ty := func.return_ty, NonVoidType):
            return_ty_cpp_info = TypeCppInfo.get(self.am, return_ty)
            return_ty_cpp_name = self.mask(return_ty_cpp_info.as_owner)
        else:
            return_ty_cpp_name = "void"
        with pkg_cpp_impl_target.indented(
            f"{return_ty_cpp_name} {func_cpp_impl_name}({params_cpp_str}) {{",
            f"}}",
        ):
            if isinstance(return_ty := func.return_ty, IfaceType):
                impl_name = f"{return_ty.decl.name}Impl"
                pkg_cpp_impl_target.writelns(
                    f"// The parameters in the make_holder function should be of the same type",
                    f"// as the parameters in the constructor of the actual implementation class.",
                    f"return {self.make_holder}<{impl_name}, {return_ty_cpp_name}>();",
                )
            else:
                pkg_cpp_impl_target.writelns(
                    f'TH_THROW({self.runtime_error}, "{func_cpp_impl_name} not implemented");',
                )

    def gen_func_impl(
        self,
        func: GlobFuncDecl,
        pkg_cpp_impl_target: CSourceWriter,
    ):
        func_cpp_impl_name = func.name
        params_cpp = []
        for param in func.params:
            param_ty_cpp_info = TypeCppInfo.get(self.am, param.ty)
            params_cpp.append(f"{self.mask(param_ty_cpp_info.as_param)} {param.name}")
        params_cpp_str = ", ".join(params_cpp)
        if isinstance(return_ty := func.return_ty, NonVoidType):
            return_ty_cpp_info = TypeCppInfo.get(self.am, return_ty)
            return_ty_cpp_name = self.mask(return_ty_cpp_info.as_owner)
        else:
            return_ty_cpp_name = "void"
        with pkg_cpp_impl_target.indented(
            f"{return_ty_cpp_name} {func_cpp_impl_name}({params_cpp_str}) {{",
            f"}}",
        ):
            if isinstance(return_ty := func.return_ty, IfaceType):
                impl_name = f"{return_ty.decl.name}Impl"
                pkg_cpp_impl_target.writelns(
                    f"// The parameters in the make_holder function should be of the same type",
                    f"// as the parameters in the constructor of the actual implementation class.",
                    f"return {self.make_holder}<{impl_name}, {return_ty_cpp_name}>();",
                )
            else:
                pkg_cpp_impl_target.writelns(
                    f'TH_THROW({self.runtime_error}, "{func_cpp_impl_name} not implemented");',
                )

    def gen_func_macro(
        self,
        func: GlobFuncDecl,
        pkg_cpp_impl_target: CSourceWriter,
    ):
        func_cpp_impl_info = GlobFuncCppImplInfo.get(self.am, func)
        func_cpp_impl_name = f"{func.name}"
        pkg_cpp_impl_target.writelns(
            f"{func_cpp_impl_info.macro}({func_cpp_impl_name});",
        )