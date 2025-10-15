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
    IfaceMethodAbiInfo,
    PackageAbiInfo,
    TypeAbiInfo,
)
from taihe.codegen.abi.writer import CHeaderWriter, CSourceWriter
from taihe.codegen.cpp.analyses import (
    GlobFuncCppImplInfo,
    IfaceCppImplInfo,
    IfaceCppInfo,
    IfaceMethodCppImplInfo,
    IfaceMethodCppInfo,
    PackageCppImplInfo,
    TypeCppInfo,
    from_abi,
    into_abi,
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
            for iface in pkg.interfaces:
                self.gen_iface_file(iface)

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

    def gen_iface_file(self, iface: IfaceDecl):
        iface_cpp_info = IfaceCppInfo.get(self.am, iface)
        iface_cpp_impl_info = IfaceCppImplInfo.get(self.am, iface)
        with CHeaderWriter(
            self.om,
            f"include/{iface_cpp_impl_info.header}",
            FileKind.CPP_HEADER,
        ) as iface_cpp_impl_target:
            iface_cpp_impl_target.add_include("taihe/common.hpp")
            iface_cpp_impl_target.add_include(iface_cpp_info.impl_header)
            for method in iface.methods:
                for param in method.params:
                    param_ty_cpp_info = TypeCppInfo.get(self.am, param.ty)
                    iface_cpp_impl_target.add_include(*param_ty_cpp_info.impl_headers)
                if isinstance(return_ty := method.return_ty, NonVoidType):
                    return_ty_cpp_info = TypeCppInfo.get(self.am, return_ty)
                    iface_cpp_impl_target.add_include(*return_ty_cpp_info.impl_headers)
                self.gen_method(iface, method, iface_cpp_impl_target)

    def gen_func(
        self,
        func: GlobFuncDecl,
        pkg_cpp_impl_target: CHeaderWriter,
    ):
        func_abi_info = GlobFuncAbiInfo.get(self.am, func)
        func_cpp_impl_info = GlobFuncCppImplInfo.get(self.am, func)
        func_impl = "CPP_FUNC_IMPL"
        params_abi = []
        args_cpp = []
        for param in func.params:
            param_ty_cpp_info = TypeCppInfo.get(self.am, param.ty)
            param_ty_abi_info = TypeAbiInfo.get(self.am, param.ty)
            params_abi.append(f"{param_ty_abi_info.as_param} {param.name}")
            args_cpp.append(from_abi(param_ty_cpp_info.as_param, param.name))
        params_abi_str = ", ".join(params_abi)
        args_cpp_str = ", ".join(args_cpp)
        result_cpp = f"{func_impl}({args_cpp_str})"
        if isinstance(return_ty := func.return_ty, NonVoidType):
            return_ty_cpp_info = TypeCppInfo.get(self.am, return_ty)
            return_ty_abi_info = TypeAbiInfo.get(self.am, return_ty)
            return_ty_abi_name = return_ty_abi_info.as_owner
            result_abi = into_abi(return_ty_cpp_info.as_owner, result_cpp)
        else:
            return_ty_abi_name = "void"
            result_abi = result_cpp
        pkg_cpp_impl_target.writelns(
            f"#define {func_cpp_impl_info.macro}({func_impl}) \\",
            f"    {return_ty_abi_name} {func_abi_info.impl_name}({params_abi_str}) {{ \\",
            f"        return {result_abi}; \\",
            f"    }}",
        )

    def gen_method(
        self,
        iface: IfaceDecl,
        method: IfaceMethodDecl,
        iface_cpp_impl_target: CHeaderWriter,
    ):
        method_abi_info = IfaceMethodAbiInfo.get(self.am, method)
        method_cpp_impl_info = IfaceMethodCppImplInfo.get(self.am, method)
        method_impl = "CPP_METHOD_IMPL"
        params_abi = []
        args_cpp = []
        iface_cpp_info = IfaceCppInfo.get(self.am, iface)
        iface_abi_info = IfaceAbiInfo.get(self.am, iface)
        params_abi.append(f"{iface_abi_info.as_param} tobj")
        args_cpp.append(from_abi(iface_cpp_info.as_param, "tobj"))
        for param in method.params:
            param_ty_cpp_info = TypeCppInfo.get(self.am, param.ty)
            param_ty_abi_info = TypeAbiInfo.get(self.am, param.ty)
            params_abi.append(f"{param_ty_abi_info.as_param} {param.name}")
            args_cpp.append(from_abi(param_ty_cpp_info.as_param, param.name))
        params_abi_str = ", ".join(params_abi)
        args_cpp_str = ", ".join(args_cpp)
        result_cpp = f"{method_impl}({args_cpp_str})"
        if isinstance(return_ty := method.return_ty, NonVoidType):
            return_ty_cpp_info = TypeCppInfo.get(self.am, return_ty)
            return_ty_abi_info = TypeAbiInfo.get(self.am, return_ty)
            return_ty_abi_name = return_ty_abi_info.as_owner
            result_abi = into_abi(return_ty_cpp_info.as_owner, result_cpp)
        else:
            return_ty_abi_name = "void"
            result_abi = result_cpp
        iface_cpp_impl_target.writelns(
            f"#define {method_cpp_impl_info.macro}({method_impl}) \\",
            f"    {return_ty_abi_name} {method_abi_info.impl_name}({params_abi_str}) {{ \\",
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

    def gen_using_namespace(
        self,
        pkg_cpp_impl_target: CSourceWriter,
        namespace: str,
    ):
        pkg_cpp_impl_target.writelns(
            f"using namespace {namespace};",
        )
        self.using_namespaces.append(namespace)

    def generate(self, pg: PackageGroup):
        for pkg in pg.packages:
            self.gen_package_file(pkg)
            for iface in pkg.interfaces:
                self.gen_iface_file(iface)
        for pkg in pg.packages:
            for iface in pkg.interfaces:
                self.gen_iface_template_header(iface)
                self.gen_iface_template_source(iface)

    def gen_package_file(self, pkg: PackageDecl):
        pkg_cpp_impl_info = PackageCppImplInfo.get(self.am, pkg)
        with CSourceWriter(
            self.om,
            f"temp/{pkg_cpp_impl_info.source}",
            FileKind.TEMPLATE,
        ) as pkg_cpp_impl_target:
            self.using_namespaces = []
            pkg_cpp_impl_target.add_include(pkg_cpp_impl_info.header)
            pkg_cpp_impl_target.add_include("stdexcept")
            pkg_cpp_impl_target.newline()
            with pkg_cpp_impl_target.indented(
                f"namespace {{",
                f"}}  // namespace",
                indent="",
            ):
                pkg_cpp_impl_target.writelns(
                    f"// To be implemented.",
                )
                for func in pkg.functions:
                    pkg_cpp_impl_target.newline()
                    self.gen_func_impl(func, pkg_cpp_impl_target)
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

    def gen_iface_file(self, iface: IfaceDecl):
        iface_cpp_impl_info = IfaceCppImplInfo.get(self.am, iface)
        with CSourceWriter(
            self.om,
            f"temp/{iface_cpp_impl_info.source}",
            FileKind.TEMPLATE,
        ) as iface_cpp_impl_target:
            self.using_namespaces = []
            iface_cpp_impl_target.add_include(iface_cpp_impl_info.header)
            iface_cpp_impl_target.add_include("stdexcept")
            iface_cpp_impl_target.newline()
            with iface_cpp_impl_target.indented(
                f"namespace {{",
                f"}}  // namespace",
                indent="",
            ):
                iface_cpp_impl_target.writelns(
                    f"// To be implemented.",
                )
                for method in iface.methods:
                    iface_cpp_impl_target.newline()
                    self.gen_method_impl(iface, method, iface_cpp_impl_target)
            iface_cpp_impl_target.newline()
            iface_cpp_impl_target.writelns(
                "// Since these macros are auto-generate, lint will cause false positive.",
                "// NOLINTBEGIN",
            )
            for method in iface.methods:
                self.gen_method_macro(method, iface_cpp_impl_target)
            iface_cpp_impl_target.writelns(
                "// NOLINTEND",
            )
            self.using_namespaces = []

    def gen_func_impl(
        self,
        func: GlobFuncDecl,
        pkg_cpp_impl_target: CSourceWriter,
    ):
        func_cpp_impl_info = GlobFuncCppImplInfo.get(self.am, func)
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
            f"{return_ty_cpp_name} {func_cpp_impl_info.function}({params_cpp_str}) {{",
            f"}}",
        ):
            if isinstance(return_ty := func.return_ty, IfaceType):
                ret_cpp_impl_info = IfaceCppImplInfo.get(self.am, return_ty.decl)
                pkg_cpp_impl_target.add_include(ret_cpp_impl_info.template_header)
                pkg_cpp_impl_target.writelns(
                    f"// The parameters in the make_holder function should be of the same type",
                    f"// as the parameters in the constructor of the actual implementation class.",
                    f"return {self.make_holder}<{ret_cpp_impl_info.template_class}, {return_ty_cpp_name}>();",
                )
            else:
                pkg_cpp_impl_target.writelns(
                    f'TH_THROW({self.runtime_error}, "not implemented");',
                )

    def gen_func_macro(
        self,
        func: GlobFuncDecl,
        pkg_cpp_impl_target: CSourceWriter,
    ):
        func_cpp_impl_info = GlobFuncCppImplInfo.get(self.am, func)
        pkg_cpp_impl_target.writelns(
            f"{func_cpp_impl_info.macro}({func_cpp_impl_info.function});",
        )

    def gen_method_impl(
        self,
        iface: IfaceDecl,
        method: IfaceMethodDecl,
        iface_cpp_impl_target: CSourceWriter,
    ):
        method_cpp_impl_info = IfaceMethodCppImplInfo.get(self.am, method)
        params_cpp = []
        iface_cpp_info = IfaceCppInfo.get(self.am, iface)
        params_cpp.append(f"{self.mask(iface_cpp_info.as_param)} tobj")
        for param in method.params:
            param_ty_cpp_info = TypeCppInfo.get(self.am, param.ty)
            params_cpp.append(f"{self.mask(param_ty_cpp_info.as_param)} {param.name}")
        params_cpp_str = ", ".join(params_cpp)
        if isinstance(return_ty := method.return_ty, NonVoidType):
            return_ty_cpp_info = TypeCppInfo.get(self.am, return_ty)
            return_ty_cpp_name = self.mask(return_ty_cpp_info.as_owner)
        else:
            return_ty_cpp_name = "void"
        with iface_cpp_impl_target.indented(
            f"{return_ty_cpp_name} {method_cpp_impl_info.function}({params_cpp_str}) {{",
            f"}}",
        ):
            if isinstance(return_ty := method.return_ty, IfaceType):
                ret_cpp_impl_info = IfaceCppImplInfo.get(self.am, return_ty.decl)
                iface_cpp_impl_target.add_include(ret_cpp_impl_info.template_header)
                iface_cpp_impl_target.writelns(
                    f"// The parameters in the make_holder function should be of the same type",
                    f"// as the parameters in the constructor of the actual implementation class.",
                    f"return {self.make_holder}<{ret_cpp_impl_info.template_class}, {return_ty_cpp_name}>();",
                )
            else:
                iface_cpp_impl_target.writelns(
                    f'TH_THROW({self.runtime_error}, "not implemented");',
                )

    def gen_method_macro(
        self,
        method: IfaceMethodDecl,
        iface_cpp_impl_target: CSourceWriter,
    ):
        method_cpp_impl_info = IfaceMethodCppImplInfo.get(self.am, method)
        iface_cpp_impl_target.writelns(
            f"{method_cpp_impl_info.macro}({method_cpp_impl_info.function});",
        )

    def gen_iface_template_header(self, iface: IfaceDecl):
        iface_abi_info = IfaceAbiInfo.get(self.am, iface)
        iface_cpp_impl_info = IfaceCppImplInfo.get(self.am, iface)
        with CHeaderWriter(
            self.om,
            f"temp/{iface_cpp_impl_info.template_header}",
            FileKind.TEMPLATE,
        ) as cls_header_target:
            for ancestor in iface_abi_info.ancestor_dict:
                for method in ancestor.methods:
                    for param in method.params:
                        param_ty_cpp_info = TypeCppInfo.get(self.am, param.ty)
                        cls_header_target.add_include(*param_ty_cpp_info.impl_headers)
                    if isinstance(return_ty := method.return_ty, NonVoidType):
                        return_ty_cpp_info = TypeCppInfo.get(self.am, return_ty)
                        cls_header_target.add_include(*return_ty_cpp_info.impl_headers)
            cls_header_target.newline()
            self.gen_iface_template_class(iface, cls_header_target)

    def gen_iface_template_class(
        self,
        iface: IfaceDecl,
        cls_header_target: CSourceWriter,
    ):
        iface_abi_info = IfaceAbiInfo.get(self.am, iface)
        iface_cpp_impl_info = IfaceCppImplInfo.get(self.am, iface)
        with cls_header_target.indented(
            f"class {iface_cpp_impl_info.template_class} {{",
            f"}};",
        ):
            cls_header_target.writelns(
                f"public:",
                f"// You can add member variables and constructor here.",
            )
            for ancestor in iface_abi_info.ancestor_dict:
                for method in ancestor.methods:
                    self.gen_iface_method_decl(iface, method, cls_header_target)

    def gen_iface_template_source(self, iface: IfaceDecl):
        iface_abi_info = IfaceAbiInfo.get(self.am, iface)
        iface_cpp_impl_info = IfaceCppImplInfo.get(self.am, iface)
        with CSourceWriter(
            self.om,
            f"temp/{iface_cpp_impl_info.template_source}",
            FileKind.TEMPLATE,
        ) as cls_source_target:
            cls_source_target.add_include(iface_cpp_impl_info.template_header)
            for ancestor in iface_abi_info.ancestor_dict:
                for method in ancestor.methods:
                    cls_source_target.newline()
                    self.gen_iface_method_impl(iface, method, cls_source_target)

    def gen_iface_method_decl(
        self,
        iface: IfaceDecl,
        method: IfaceMethodDecl,
        cls_header_target: CSourceWriter,
    ):
        method_cpp_info = IfaceMethodCppInfo.get(self.am, method)
        params_cpp = []
        for param in method.params:
            param_ty_cpp_info = TypeCppInfo.get(self.am, param.ty)
            params_cpp.append(f"{self.mask(param_ty_cpp_info.as_param)} {param.name}")
        params_cpp_str = ", ".join(params_cpp)
        if isinstance(return_ty := method.return_ty, NonVoidType):
            return_ty_cpp_info = TypeCppInfo.get(self.am, return_ty)
            return_ty_cpp_name = self.mask(return_ty_cpp_info.as_owner)
        else:
            return_ty_cpp_name = "void"
        cls_header_target.writelns(
            f"{return_ty_cpp_name} {method_cpp_info.call_name}({params_cpp_str});",
        )

    def gen_iface_method_impl(
        self,
        iface: IfaceDecl,
        method: IfaceMethodDecl,
        cls_source_target: CSourceWriter,
    ):
        iface_cpp_impl_info = IfaceCppImplInfo.get(self.am, iface)
        method_cpp_info = IfaceMethodCppInfo.get(self.am, method)
        params_cpp = []
        for param in method.params:
            param_ty_cpp_info = TypeCppInfo.get(self.am, param.ty)
            params_cpp.append(f"{self.mask(param_ty_cpp_info.as_param)} {param.name}")
        params_cpp_str = ", ".join(params_cpp)
        if isinstance(return_ty := method.return_ty, NonVoidType):
            return_ty_cpp_info = TypeCppInfo.get(self.am, return_ty)
            return_ty_cpp_name = self.mask(return_ty_cpp_info.as_owner)
        else:
            return_ty_cpp_name = "void"
        with cls_source_target.indented(
            f"{return_ty_cpp_name} {iface_cpp_impl_info.template_class}::{method_cpp_info.impl_name}({params_cpp_str}) {{",
            f"}}",
        ):
            if isinstance(return_ty := method.return_ty, IfaceType):
                ret_cpp_impl_info = IfaceCppImplInfo.get(self.am, return_ty.decl)
                cls_source_target.add_include(ret_cpp_impl_info.template_header)
                cls_source_target.writelns(
                    f"// The parameters in the make_holder function should be of the same type",
                    f"// as the parameters in the constructor of the actual implementation class.",
                    f"return {self.make_holder}<{ret_cpp_impl_info.template_class}, {return_ty_cpp_name}>();",
                )
            else:
                cls_source_target.writelns(
                    f'TH_THROW({self.runtime_error}, "not implemented");',
                )