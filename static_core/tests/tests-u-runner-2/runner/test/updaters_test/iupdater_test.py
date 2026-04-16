#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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

from pathlib import Path
from unittest import TestCase
from unittest.mock import MagicMock

from runner.common_exceptions import InvalidConfiguration
from runner.extensions.updaters.iupdater import IUpdater, get_updater_class
from runner.options.config import Config
from runner.types.step_report import StepReport


class ConcreteUpdater(IUpdater):
    """Minimal concrete implementation of IUpdater for testing."""

    def __init__(self, config: Config) -> None:
        super().__init__(config)
        self.processed: list[tuple[Path, list[StepReport], bool]] = []

    def process(self, test_file: Path, data: list[StepReport], use_metadata: bool = False) -> None:
        self.processed.append((test_file, data, use_metadata))


class IUpdaterTest(TestCase):

    @staticmethod
    def _make_config() -> MagicMock:
        return MagicMock(spec=Config)

    def test_get_updater_class_returns_correct_class(self) -> None:
        """
        get_updater_class should return the class object when a valid
        fully-qualified name of an IUpdater subclass is provided.
        """
        fqn = "runner.extensions.updaters.parser.parser_updater.ParserUpdater"
        cls = get_updater_class(fqn)
        self.assertTrue(issubclass(cls, IUpdater))

    def test_get_updater_class_raises_for_non_updater_class(self) -> None:
        """
        get_updater_class should raise InvalidConfiguration when the
        resolved class is not a subclass of IUpdater.
        """
        # BaseValidator is a real class but is NOT an IUpdater subclass
        fqn = "runner.extensions.validators.base_validator.BaseValidator"
        with self.assertRaises(InvalidConfiguration):
            get_updater_class(fqn)

    def test_get_updater_class_raises_for_unknown_class(self) -> None:
        """
        get_updater_class should raise an ModuleNotFoundError exception when the class name
        cannot be resolved (module or class does not exist).
        """
        fqn = "runner.extensions.updaters.nonexistent_module.NonExistentUpdater"
        with self.assertRaises(ModuleNotFoundError):
            get_updater_class(fqn)

    def test_concrete_updater_stores_config(self) -> None:
        """
        IUpdater.__init__ should store the config object so that
        subclasses can access it via self._config.
        """
        config = self._make_config()
        updater = ConcreteUpdater(config)
        self.assertIs(updater._config, config)  # pylint: disable=protected-access

    def test_concrete_updater_process_called_with_correct_args(self) -> None:
        """
        Calling process() on a concrete updater should forward all
        arguments unchanged.
        """
        config = self._make_config()
        updater = ConcreteUpdater(config)

        test_file = Path("/some/path/test.ets")
        report = StepReport(name="step", command_line="cmd")
        data = [report]

        updater.process(test_file, data, use_metadata=True)

        self.assertEqual(1, len(updater.processed))
        recorded_file, recorded_data, recorded_meta = updater.processed[0]
        self.assertEqual(test_file, recorded_file)
        self.assertIs(data, recorded_data)
        self.assertTrue(recorded_meta)

    def test_concrete_updater_process_default_use_metadata_is_false(self) -> None:
        """
        The default value of use_metadata in process() should be False.
        """
        config = self._make_config()
        updater = ConcreteUpdater(config)

        updater.process(Path("/tmp/test.ets"), [])

        _, _, recorded_meta = updater.processed[0]
        self.assertFalse(recorded_meta)

    def test_concrete_updater_process_called_multiple_times(self) -> None:
        """
        process() can be called multiple times and each call is recorded
        independently.
        """
        config = self._make_config()
        updater = ConcreteUpdater(config)

        files = [Path(f"/tmp/test{i}.ets") for i in range(3)]
        for f in files:
            updater.process(f, [])

        self.assertEqual(3, len(updater.processed))
        for i, (recorded_file, _, _) in enumerate(updater.processed):
            self.assertEqual(files[i], recorded_file)
