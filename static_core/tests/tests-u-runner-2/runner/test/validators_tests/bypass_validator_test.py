#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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

from typing import cast
from unittest import TestCase
from unittest.mock import MagicMock

from runner.extensions.validators.bypass_validator import BypassValidator
from runner.options.options_step import StepKind
from runner.options.options_step_utils import StepFields
from runner.suites.test_standard_flow import TestStandardFlow


class BypassValidatorTest(TestCase):
    """Tests for BypassValidator - a validator that always returns passed=True."""

    @staticmethod
    def _make_step_fields(step_kind: StepKind = StepKind.COMPILER) -> StepFields:
        return StepFields(step_kind=step_kind.value, step_name="test_step")

    def test_always_passes_return_code_0(self) -> None:
        """BypassValidator returns passed=True when the step return code is 0."""
        mock_test = MagicMock(spec=TestStandardFlow)
        validator = BypassValidator()
        step_fields = self._make_step_fields(StepKind.COMPILER)

        validate_fns = validator.get_validator(StepKind.COMPILER.value)
        self.assertIsNotNone(validate_fns)

        for fn in cast(list, validate_fns):
            result = fn(mock_test, step_fields, "some output", "", 0)
            self.assertTrue(result.passed)

    def test_always_passes_return_code_nonzero(self) -> None:
        """BypassValidator returns passed=True even when the step return code is non-zero."""
        mock_test = MagicMock(spec=TestStandardFlow)
        validator = BypassValidator()
        step_fields = self._make_step_fields(StepKind.RUNTIME)

        validate_fn = validator.get_validator(StepKind.RUNTIME.value)
        self.assertIsNotNone(validate_fn)

        for fn in validate_fn:  # type: ignore[union-attr]
            result = fn(mock_test, step_fields, "", "some error", 1)
            self.assertTrue(result.passed)

    def test_registered_for_all_step_kinds(self) -> None:
        """BypassValidator registers a handler for every StepKind value."""
        validator = BypassValidator()
        for step_kind in StepKind.values():
            fns = validator.get_validator(step_kind)
            self.assertIsNotNone(fns, f"No validator registered for StepKind '{step_kind}'")
            self.assertGreater(len(cast(list, fns)), 0, f"Empty validator list for StepKind '{step_kind}'")
