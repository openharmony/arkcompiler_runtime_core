#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#

from __future__ import annotations

import logging
import shutil
from abc import abstractmethod, ABC
from functools import cached_property
from pathlib import Path
from typing import List, Any

from runner.logger import Log
from runner.options.config import Config
from runner.options.options_jit import JitOptions
from runner.path_utils import is_sudo_user, chown2user
from runner.plugins.ets.ets_suites import EtsSuites
from runner.plugins.ets.ets_test_dir import EtsTestDir
from runner.plugins.ets.ets_utils import ETSUtils
from runner.plugins.ets.preparation_step import FrontendFuncTestPreparationStep, \
                                                TestPreparationStep, \
                                                CtsTestPreparationStep, \
                                                FuncTestPreparationStep, \
                                                ESCheckedTestPreparationStep, \
                                                JitStep, \
                                                CopyStep, \
                                                CustomGeneratorTestPreparationStep
from runner.plugins.ets.runtime_default_ets_test_dir import RuntimeDefaultEtsTestDir
from runner.plugins.work_dir import WorkDir

_LOGGER = logging.getLogger("runner.plugins.ets.ets_test_suite")


class EtsTestSuite(ABC):
    def __init__(self, config: Config, work_dir: WorkDir, suite_name: str, default_list_root: str) -> None:
        self.__suite_name = suite_name
        self.__work_dir = work_dir
        self.__default_list_root = default_list_root
        self._list_root = config.general.list_root

        self.config = config

        self._preparation_steps: List[TestPreparationStep] = []
        self._jit: JitOptions = config.ark.jit
        self._is_jit = config.ark.jit.enable and config.ark.jit.num_repeats > 0

    @property
    def test_root(self) -> Path:
        return self.__work_dir.gen

    @property
    def default_list_root_suite_name(self) -> Path:
        return Path(self.__default_list_root, self.__suite_name)

    @staticmethod
    def get_class(ets_suite_name: str) -> Any:
        name_to_class = {
            EtsSuites.FUNC.value: FuncEtsTestSuite,
            EtsSuites.CTS.value: CtsEtsTestSuite,
            EtsSuites.RUNTIME.value: RuntimeEtsTestSuite,
            EtsSuites.GCSTRESS.value: GCStressEtsTestSuite,
            EtsSuites.ESCHECKED.value: ESCheckedEtsTestSuite,
            EtsSuites.CUSTOM.value: CustomEtsTestSuite,
            EtsSuites.TS_SUBSET.value: TsSubsetEtsTestSuite,
            EtsSuites.SDK.value: SdkEtsTestSuite,
        }
        cls = name_to_class.get(ets_suite_name)
        if cls is None:
            Log.exception_and_raise(_LOGGER, "Cannot find TestSuite class for " + ets_suite_name)
        return cls

    @cached_property
    def name(self) -> str:
        return self.__suite_name

    @cached_property
    def list_root(self) -> Path:
        return Path(self._list_root) if self._list_root else self.default_list_root_suite_name

    @abstractmethod
    def set_preparation_steps(self) -> None:
        pass

    def process(self, force_generate: bool) -> None:
        util = ETSUtils()
        if not force_generate and util.are_tests_generated(self.test_root):
            Log.all(_LOGGER, f"Reused earlier generated tests from {self.test_root}")
            return
        Log.all(_LOGGER, "Generated folder : " + str(self.test_root))

        if self.test_root.exists():
            Log.all(_LOGGER, f"INFO: {str(self.test_root.absolute())} already exist. WILL BE CLEANED")
            shutil.rmtree(self.test_root)

        tests: List[str] = []
        for step in self._preparation_steps:
            tests = step.transform(force_generate)

        util.create_report(self.test_root, tests)
        if is_sudo_user():
            chown2user(self.test_root)
        if len(tests) == 0:
            Log.exception_and_raise(_LOGGER, "Failed generating and updating tests for ets templates or stdlib")


class RuntimeEtsTestSuite(EtsTestSuite):
    def __init__(self, config: Config, work_dir: WorkDir, default_list_root: str):
        super().__init__(config, work_dir, EtsSuites.RUNTIME.value, default_list_root)
        self.__default_test_dir = RuntimeDefaultEtsTestDir(config.general.static_core_root, config.general.test_root)
        self.set_preparation_steps()

    def set_preparation_steps(self) -> None:
        self._preparation_steps.append(CopyStep(
            test_source_path=self.__default_test_dir.root,
            test_gen_path=self.test_root,
            config=self.config
        ))
        if self._is_jit:
            self._preparation_steps.append(JitStep(
                test_source_path=self.test_root,
                test_gen_path=self.test_root,
                config=self.config,
                num_repeats=self._jit.num_repeats
            ))


class GCStressEtsTestSuite(EtsTestSuite):
    def __init__(self, config: Config, work_dir: WorkDir, default_list_root: str):
        super().__init__(config, work_dir, EtsSuites.GCSTRESS.value, default_list_root)
        self._ets_test_dir = EtsTestDir(config.general.static_core_root, config.general.test_root)
        self.set_preparation_steps()

    def set_preparation_steps(self) -> None:
        self._preparation_steps.append(CopyStep(
            test_source_path=self._ets_test_dir.gc_stress,
            test_gen_path=self.test_root,
            config=self.config
        ))
        if self._is_jit:
            self._preparation_steps.append(JitStep(
                test_source_path=self.test_root,
                test_gen_path=self.test_root,
                config=self.config,
                num_repeats=self._jit.num_repeats
            ))


class TsSubsetEtsTestSuite(EtsTestSuite):
    def __init__(self, config: Config, work_dir: WorkDir, default_list_root: str):
        super().__init__(config, work_dir, EtsSuites.TS_SUBSET.value, default_list_root)
        self._ets_test_dir = EtsTestDir(config.general.static_core_root, config.general.test_root)
        self.set_preparation_steps()

    def set_preparation_steps(self) -> None:
        self._preparation_steps.append(CopyStep(
            test_source_path=self._ets_test_dir.ets_ts_subset,
            test_gen_path=self.test_root,
            config=self.config
        ))
        if self._is_jit:
            self._preparation_steps.append(JitStep(
                test_source_path=self.test_root,
                test_gen_path=self.test_root,
                config=self.config,
                num_repeats=self._jit.num_repeats
            ))


class CustomEtsTestSuite(EtsTestSuite):
    def __init__(self, config: Config, work_dir: WorkDir, _: str):
        super().__init__(config, work_dir, config.custom.suite_name, config.custom.list_root)
        self._list_root = config.custom.list_root
        self.set_preparation_steps()

    def set_preparation_steps(self) -> None:
        if self.config.custom.generator is not None:
            self._preparation_steps.append(CustomGeneratorTestPreparationStep(
                test_source_path=self.config.custom.test_root,
                test_gen_path=self.test_root,
                config=self.config,
                extension="ets"
            ))
            copy_source_path = self.test_root
        else:
            copy_source_path = self.config.custom.test_root
        self._preparation_steps.append(CopyStep(
            test_source_path=copy_source_path,
            test_gen_path=self.test_root,
            config=self.config
        ))
        if self._is_jit:
            self._preparation_steps.append(JitStep(
                test_source_path=self.test_root,
                test_gen_path=self.test_root,
                config=self.config,
                num_repeats=self._jit.num_repeats
            ))


class CtsEtsTestSuite(EtsTestSuite):
    def __init__(self, config: Config, work_dir: WorkDir, default_list_root: str):
        super().__init__(config, work_dir, EtsSuites.CTS.value, default_list_root)
        self._ets_test_dir = EtsTestDir(config.general.static_core_root, config.general.test_root)
        self.set_preparation_steps()

    def set_preparation_steps(self) -> None:
        self._preparation_steps.append(CtsTestPreparationStep(
            test_source_path=self._ets_test_dir.ets_templates,
            test_gen_path=self.test_root,
            config=self.config
        ))
        if self._is_jit:
            self._preparation_steps.append(JitStep(
                test_source_path=self.test_root,
                test_gen_path=self.test_root,
                config=self.config,
                num_repeats=self._jit.num_repeats
            ))


class FuncEtsTestSuite(EtsTestSuite):
    def __init__(self, config: Config, work_dir: WorkDir, default_list_root: str):
        super().__init__(config, work_dir, EtsSuites.FUNC.value, default_list_root)
        self._ets_test_dir = EtsTestDir(config.general.static_core_root, config.general.test_root)
        self.set_preparation_steps()

    def set_preparation_steps(self) -> None:
        self._preparation_steps.append(FuncTestPreparationStep(
            test_source_path=self._ets_test_dir.stdlib_templates,
            test_gen_path=self.test_root,
            config=self.config
        ))
        self._preparation_steps.append(FrontendFuncTestPreparationStep(
            test_source_path=self._ets_test_dir.ets_func_tests_templates,
            test_gen_path=self.test_root,
            config=self.config
        ))
        self._preparation_steps.append(CopyStep(
            test_source_path=self._ets_test_dir.ets_func_tests,
            test_gen_path=self.test_root,
            config=self.config
        ))
        if self._is_jit:
            self._preparation_steps.append(JitStep(
                test_source_path=self.test_root,
                test_gen_path=self.test_root,
                config=self.config,
                num_repeats=self._jit.num_repeats
            ))


class ESCheckedEtsTestSuite(EtsTestSuite):
    def __init__(self, config: Config, work_dir: WorkDir, default_list_root: str):
        super().__init__(config, work_dir, EtsSuites.ESCHECKED.value, default_list_root)
        self._ets_test_dir = EtsTestDir(config.general.static_core_root, config.general.test_root)
        self.set_preparation_steps()

    def set_preparation_steps(self) -> None:
        self._preparation_steps.append(ESCheckedTestPreparationStep(
            test_source_path=self._ets_test_dir.ets_es_checked,
            test_gen_path=self.test_root,
            config=self.config
        ))
        self._preparation_steps.append(CopyStep(
            test_source_path=self._ets_test_dir.ets_es_checked,
            test_gen_path=self.test_root,
            config=self.config
        ))
        if self._is_jit:
            self._preparation_steps.append(JitStep(
                test_source_path=self.test_root,
                test_gen_path=self.test_root,
                config=self.config,
                num_repeats=self._jit.num_repeats
            ))


class SdkEtsTestSuite(EtsTestSuite):
    def __init__(self, config: Config, work_dir: WorkDir, default_list_root: str):
        super().__init__(config, work_dir, EtsSuites.SDK.value, default_list_root)
        self._ets_test_dir = EtsTestDir(config.general.static_core_root, config.general.test_root)
        self.set_preparation_steps()

    def set_preparation_steps(self) -> None:
        self._preparation_steps.append(CopyStep(
            test_source_path=self._ets_test_dir.ets_sdk,
            test_gen_path=self.test_root,
            config=self.config
        ))
        if self._is_jit:
            self._preparation_steps.append(JitStep(
                test_source_path=self.test_root,
                test_gen_path=self.test_root,
                config=self.config,
                num_repeats=self._jit.num_repeats
            ))
