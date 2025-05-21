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
class AbiHeaderBackendConfig(BackendConfig):
    NAME = "abi-header"

    def construct(self, instance: CompilerInstance) -> Backend:
        from taihe.codegen.abi.gen_abi import ABIHeadersGenerator

        return ABIHeadersGenerator(instance)


@dataclass
class AbiSourcesBackendConfig(BackendConfig):
    NAME = "abi-source"
    DEPS: ClassVar = ["abi-header"]

    def construct(self, instance: CompilerInstance) -> Backend:
        from taihe.codegen.abi.gen_abi import ABISourcesGenerator

        return ABISourcesGenerator(instance)


@dataclass
class CAuthorBackendConfig(BackendConfig):
    NAME = "c-author"
    DEPS: ClassVar = ["abi-source"]

    def construct(self, instance: CompilerInstance) -> Backend:
        from taihe.codegen.abi.gen_impl import (
            CImplHeadersGenerator,
            CImplSourcesGenerator,
        )

        # TODO: unify CImpl{Headers,Sources}Generator
        class CImplBackendImpl(Backend):
            def __init__(self, ci: CompilerInstance):
                super().__init__(ci)
                self._ci = ci

            def generate(self):
                oc = self._ci.output_config
                am = self._ci.analysis_manager
                pg = self._ci.package_group
                CImplSourcesGenerator(oc, am).generate(pg)
                CImplHeadersGenerator(oc, am).generate(pg)

        return CImplBackendImpl(instance)