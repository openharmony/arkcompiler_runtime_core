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
class AniBridgeBackendConfig(BackendConfig):
    NAME = "ani-bridge"
    DEPS: ClassVar = ["cpp-user"]

    keep_name: bool = False
    """Use the original function name (instead of "camelCase") in exported ArkTS sources."""

    def construct(self, instance: CompilerInstance) -> Backend:
        from taihe.codegen.ani.attributes import all_attr_types
        from taihe.codegen.ani.gen_ani import ANICodeGenerator
        from taihe.codegen.ani.gen_sts import STSCodeGenerator

        instance.attribute_manager.register(*all_attr_types)

        # TODO: unify {ANI,STS}CodeGenerator
        class AniBridgeBackendImpl(Backend):
            def __init__(self, ci: "CompilerInstance"):
                super().__init__(ci)
                self._ci = ci

            def generate(self):
                om = self._ci.output_manager
                am = self._ci.analysis_manager
                pg = self._ci.package_group
                ANICodeGenerator(om, am).generate(pg)
                STSCodeGenerator(om, am).generate(pg)

        return AniBridgeBackendImpl(instance)