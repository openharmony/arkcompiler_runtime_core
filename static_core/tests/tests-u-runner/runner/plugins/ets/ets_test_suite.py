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
from runner.plugins.ets.ets_test_dir import EtsTestDir
from runner.plugins.ets.ets_utils import ETSUtils
from runner.plugins.ets.preparation_step import TestPreparationStep, CtsTestPreparationStep, \
    FuncTestPreparationStep, JitStep, CopyStep
from runner.plugins.ets.runtime_default_ets_test_dir import RuntimeDefaultEtsTestDir
from runner.plugins.work_dir import WorkDir

_LOGGER = logging.getLogger("runner.plugins.ets.ets_test_suite")


class EtsTestSuite(ABC):
    def __init__(self, config: Config, work_dir: WorkDir, suite_name: str) -> None:
        self.__suite_name = suite_name
        self.__work_dir = work_dir
        self._list_root = config.general.list_root

        self.config = config

        self._preparation_steps: List[TestPreparationStep] = []
        self._jit: JitOptions = config.ark.jit
        self._is_jit = config.ark.jit.enable and config.ark.jit.num_repeats > 0

    @staticmethod
    def get_class(ets_suite_name: str) -> Any:
        name_to_class = {
            "ets-func-tests": FuncEtsTestSuite,
            "ets-cts": CtsEtsTestSuite,
            "ets-runtime": RuntimeEtsTestSuite,
        }
        return name_to_class[ets_suite_name]

    @cached_property
    def name(self) -> str:
        return self.__suite_name

    @property
    def test_root(self) -> Path:
        return self.__work_dir.gen

    @property
    @abstractmethod
    def list_root(self) -> Path:
        pass

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
        if len(tests) == 0:
            Log.exception_and_raise(_LOGGER, "Failed generating and updating tests for ets templates or stdlib")


class RuntimeEtsTestSuite(EtsTestSuite):
    # pylint: disable=unused-argument
    def __init__(self, panda_source_root: str, config: Config, work_dir: WorkDir):
        super().__init__(config, work_dir, "ets-runtime")
        self.__default_test_dir = RuntimeDefaultEtsTestDir(panda_source_root, config.general.test_root)
        self.set_preparation_steps()

    @cached_property
    def list_root(self) -> Path:
        return Path(self._list_root) if self._list_root else self.__default_test_dir.es2panda_test

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


class CtsEtsTestSuite(EtsTestSuite):
    def __init__(self, panda_source_root: str, config: Config, work_dir: WorkDir):
        super().__init__(config, work_dir, "ets-cts")
        self._ets_test_dir = EtsTestDir(panda_source_root, config.general.test_root)
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

    @cached_property
    def list_root(self) -> Path:
        return Path(self._list_root) if self._list_root else Path(__file__).parent


class FuncEtsTestSuite(EtsTestSuite):
    def __init__(self, panda_source_root: str, config: Config, work_dir: WorkDir):
        super().__init__(config, work_dir, "ets-func-tests")
        self._ets_test_dir = EtsTestDir(panda_source_root, config.general.test_root)
        self.set_preparation_steps()

    def set_preparation_steps(self) -> None:
        self._preparation_steps.append(FuncTestPreparationStep(
            test_source_path=self._ets_test_dir.stdlib_templates,
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

    @cached_property
    def list_root(self) -> Path:
        return Path(self._list_root) if self._list_root else Path(__file__).parent
