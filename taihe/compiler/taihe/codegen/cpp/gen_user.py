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
    GlobFuncABIInfo,
    PackageABIInfo,
)
from taihe.codegen.abi.writer import CHeaderWriter
from taihe.codegen.cpp.analyses import (
    GlobFuncCppUserInfo,
    PackageCppInfo,
    PackageCppUserInfo,
    TypeCppInfo,
)
from taihe.semantics.declarations import (
    GlobFuncDecl,
    PackageDecl,
    PackageGroup,
)
from taihe.utils.analyses import AnalysisManager
from taihe.utils.outputs import FileKind, OutputManager


class CppUserHeadersGenerator:
    def __init__(self, om: OutputManager, am: AnalysisManager):
        self.om = om
        self.am = am

    def generate(self, pg: PackageGroup):
        for pkg in pg.packages:
            self.gen_package_file(pkg)

    def gen_package_file(self, pkg: PackageDecl):
        pkg_abi_info = PackageABIInfo.get(self.am, pkg)
        pkg_cpp_info = PackageCppInfo.get(self.am, pkg)
        pkg_cpp_user_info = PackageCppUserInfo.get(self.am, pkg)
        with CHeaderWriter(
            self.om,
            f"include/{pkg_cpp_user_info.header}",
            FileKind.CPP_HEADER,
        ) as pkg_cpp_target:
            # types
            pkg_cpp_target.add_include(pkg_cpp_info.header)
            # functions
            pkg_cpp_target.add_include("taihe/common.hpp")
            pkg_cpp_target.add_include(pkg_abi_info.header)
            for func in pkg.functions:
                for param in func.params:
                    type_cpp_info = TypeCppInfo.get(self.am, param.ty_ref.resolved_ty)
                    pkg_cpp_target.add_include(*type_cpp_info.impl_headers)
                if return_ty_ref := func.return_ty_ref:
                    type_cpp_info = TypeCppInfo.get(self.am, return_ty_ref.resolved_ty)
                    pkg_cpp_target.add_include(*type_cpp_info.impl_headers)
                self.gen_func(func, pkg_cpp_target)

    def gen_func(
        self,
        func: GlobFuncDecl,
        pkg_cpp_target: CHeaderWriter,
    ):
        func_abi_info = GlobFuncABIInfo.get(self.am, func)
        func_cpp_user_info = GlobFuncCppUserInfo.get(self.am, func)
        params_cpp = []
        args_into_abi = []
        for param in func.params:
            type_cpp_info = TypeCppInfo.get(self.am, param.ty_ref.resolved_ty)
            params_cpp.append(f"{type_cpp_info.as_param} {param.name}")
            args_into_abi.append(type_cpp_info.pass_into_abi(param.name))
        params_cpp_str = ", ".join(params_cpp)
        args_into_abi_str = ", ".join(args_into_abi)
        abi_result = f"{func_abi_info.mangled_name}({args_into_abi_str})"
        if return_ty_ref := func.return_ty_ref:
            type_cpp_info = TypeCppInfo.get(self.am, return_ty_ref.resolved_ty)
            cpp_return_ty_name = type_cpp_info.as_owner
            cpp_result = type_cpp_info.return_from_abi(abi_result)
        else:
            cpp_return_ty_name = "void"
            cpp_result = abi_result
        with pkg_cpp_target.indented(
            f"namespace {func_cpp_user_info.namespace} {{",
            f"}}",
            indent="",
        ):
            with pkg_cpp_target.indented(
                f"inline {cpp_return_ty_name} {func_cpp_user_info.call_name}({params_cpp_str}) {{",
                f"}}",
            ):
                pkg_cpp_target.writelns(
                    f"return {cpp_result};",
                )