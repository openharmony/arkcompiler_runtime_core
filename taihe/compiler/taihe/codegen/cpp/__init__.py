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

from dataclasses import dataclass
from typing import ClassVar

from taihe.driver.backend import Backend, BackendConfig
from taihe.driver.contexts import CompilerInstance


@dataclass
class CppCommonHeadersBackendConfig(BackendConfig):
    NAME = "cpp-common"
    DEPS: ClassVar = ["abi-header"]

    def construct(self, instance: CompilerInstance) -> Backend:
        from taihe.codegen.cpp.gen_common import CppHeadersGenerator

        return CppHeadersGenerator(instance)


@dataclass
class CppAuthorBackendConfig(BackendConfig):
    NAME = "cpp-author"
    DEPS: ClassVar = ["cpp-common", "abi-source"]

    def construct(self, instance: CompilerInstance) -> Backend:
        from taihe.codegen.cpp.gen_impl import (
            CppImplHeadersGenerator,
            CppImplSourcesGenerator,
        )

        # TODO: unify CppImpl{Headers,Sources}Generator
        class CppImplBackendImpl(Backend):
            def __init__(self, ci: CompilerInstance):
                super().__init__(ci)
                self._ci = ci

            def generate(self):
                tm = self._ci.target_manager
                am = self._ci.analysis_manager
                pg = self._ci.package_group
                CppImplSourcesGenerator(tm, am).generate(pg)
                CppImplHeadersGenerator(tm, am).generate(pg)

        return CppImplBackendImpl(instance)


@dataclass
class CppUserHeadersBackendConfig(BackendConfig):
    NAME = "cpp-user"
    DEPS: ClassVar = ["cpp-common"]

    def construct(self, instance: CompilerInstance) -> Backend:
        from taihe.codegen.cpp.gen_user import CppUserHeadersGenerator

        return CppUserHeadersGenerator(instance)