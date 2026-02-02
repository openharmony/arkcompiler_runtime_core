#!/usr/bin/env python3
# -- coding: utf-8 --
#
# Copyright (c) 2026 Huawei Device Co., Ltd.
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

from collections.abc import Callable
from typing import TYPE_CHECKING, Any, Generic, TypeVar, cast

from runner.extensions.flows.itest_flow import ITestFlow

if TYPE_CHECKING:
    from runner.extensions.suites.test_suite_registry import ITestSuite

T = TypeVar("T", "ITestFlow", "ITestSuite")
FactoryClass = Callable[..., T]  # type: ignore[explicit-any]


class ConfigRegistry(Generic[T]):
    def __init__(self) -> None:
        self._map: dict[str, FactoryClass] = {}

    def register(self, name: str) -> Callable[[FactoryClass], FactoryClass]:
        def wrapper(f: FactoryClass) -> FactoryClass:
            if name in self._map:
                raise ValueError(f"Workflow {name} is already registered")
            self._map[name] = f
            return f
        return wrapper

    def create(self, name: str, *args: Any, **kwargs: Any) -> T:  # type: ignore[explicit-any]
        try:
            workflow = self._map[name]
        except KeyError as e:
            known = ", ".join(sorted(self._map))
            raise KeyError(f"Unknown test_suite/workflow-kind '{name}'. Known: {known}") from e
        return cast(T, workflow(*args, **kwargs))


workflow_registry: ConfigRegistry[ITestFlow] = ConfigRegistry()
