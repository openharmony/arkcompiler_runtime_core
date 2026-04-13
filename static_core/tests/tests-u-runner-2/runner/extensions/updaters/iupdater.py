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
#

from abc import ABC, abstractmethod
from pathlib import Path

from runner.common_exceptions import InvalidConfiguration
from runner.options.config import Config
from runner.types.step_report import StepReport
from runner.utils import get_class_by_name


def get_updater_class(clazz: str) -> type['IUpdater']:
    class_obj = get_class_by_name(clazz)
    if not issubclass(class_obj, IUpdater):
        raise InvalidConfiguration(
            f"Updater class '{clazz}' not found. "
            f"Check value of 'updater' parameter")
    return class_obj


class IUpdater(ABC):
    """Abstract base class for test result updaters.
    
    Process option `--update-expected` for the specified test-suite.
    Each test suite can specify one updater per collection.
    
    Implementations of this interface handle updating expected test output
    files based on actual test execution results, allowing test baselines
    to be refreshed when test behavior changes intentionally.
    """

    def __init__(self, config: Config) -> None:
        self._config: Config = config

    @abstractmethod
    def process(self, test_file: Path, data: list[StepReport], use_metadata: bool = False) -> None:
        """Process and update expected outputs for a test file.
        
        This method must be implemented by concrete updater classes to handle
        the logic of updating expected output files based on actual test results.
        
        Args:
            test_file: Path to the test file being processed
            data: List of StepReport objects containing actual test results and execution information
            use_metadata: Whether to include metadata in the update process
        """
